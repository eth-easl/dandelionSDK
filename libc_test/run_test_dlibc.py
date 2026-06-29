#!/usr/bin/env python3
"""Python-based test runner for dlibc.

Replaces the shell-based runtest helper while preserving the same interface:
    runtest [-t timeout] [-w wrap] cmd [args...]
"""

from __future__ import annotations

import getopt
import os
import resource
import shutil
import signal
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

STACK_LIMIT_BYTES = 100 * 1024
INTERNAL_EXIT_CODE = 255


class UsageError(Exception):
    """Raised when the runner invocation is invalid."""


def usage(prog: str) -> None:
    print(f"usage: {prog} [-t timeoutsec] [-w wrapcmd] cmd [args..]", file=sys.stderr)


def parse_args(argv: list[str]) -> tuple[str, str, list[str]]:
    timeout = 5
    wrap = ""
    try:
        options, remaining = getopt.getopt(argv, "w:t:")
    except getopt.GetoptError:
        usage(Path(sys.argv[0]).name)
        raise UsageError("invalid arguments")

    for opt, value in options:
        if opt == "-w":
            wrap = value
        elif opt == "-t":
            try:
                timeout = int(value)
            except ValueError:
                timeout = 0

    if not remaining:
        usage(Path(sys.argv[0]).name)
        raise UsageError("missing test command")

    return timeout, wrap, remaining


def resolve_test_bin(test_bin: str) -> str:
    # check if the test_bin is an absolute path
    if os.path.isabs(test_bin):
        return test_bin
    return os.path.join(os.getcwd(), test_bin)


def artifact_subpath(test_rel: str) -> Path:
    return Path(test_rel.lstrip("/"))


def set_stack_limit() -> None:
    try:
        soft_limit, hard_limit = resource.getrlimit(resource.RLIMIT_STACK)
        if STACK_LIMIT_BYTES <= hard_limit:
            resource.setrlimit(resource.RLIMIT_STACK, (STACK_LIMIT_BYTES, STACK_LIMIT_BYTES))
        elif STACK_LIMIT_BYTES <= soft_limit:
            resource.setrlimit(resource.RLIMIT_STACK, (STACK_LIMIT_BYTES, hard_limit))
    except (OSError, ValueError):
        # Match the C helper's "best effort" behavior: exec should still proceed.
        pass


def write_meta(
    meta_file: Path,
    test_bin: str,
    test_rel: str,
    timeout: int,
    wrap: str,
    exit_code: int,
    artifact_dir: Path,
    tmpdir: str,
) -> None:
    meta_file.write_text(
        "\n".join(
            [
                f"test_bin={test_bin}",
                f"test_rel={test_rel}",
                f"timeout={timeout}",
                f"wrap={wrap}",
                f"exit_code={exit_code}",
                f"artifact_dir={artifact_dir}",
                f"tmpdir={tmpdir}",
            ]
        )
        + "\n",
        encoding="utf-8",
    )


def copy_stdio_file(src: Path, dst: Path) -> None:
    if src.is_file():
        shutil.copyfile(src, dst)


def replay_file(path: Path, stream) -> None:
    if not path.exists():
        return

    with path.open("rb") as handle:
        shutil.copyfileobj(handle, stream.buffer)
    stream.flush()


def replay_outputs(
    tmpdir_path: Path,
    artifact_dir: Path,
    host_stdout: Path,
    host_stderr: Path,
    dandelion_stdout: Path,
    dandelion_stderr: Path,
) -> None:
    output_sets_dir = tmpdir_path / "output_sets"
    if output_sets_dir.is_dir():
        shutil.copytree(output_sets_dir, artifact_dir / "output_sets")

    copy_stdio_file(output_sets_dir / "stdio" / "stdout", dandelion_stdout)
    copy_stdio_file(output_sets_dir / "stdio" / "stderr", dandelion_stderr)

    # replay the host stdout and stderr to the console
    # this is useful for debugging the test runner itself
    replay_file(host_stdout, sys.stdout)
    replay_file(host_stderr, sys.stderr)


def format_signal_failure(return_code: int) -> str:
    signal_number = -return_code
    try:
        signal_name = signal.strsignal(signal_number)
    except (AttributeError, ValueError):
        signal_name = None
    if signal_name:
        return f"[signal {signal_name}]"
    return "[unknown]"


def report_internal(test_bin: str, message: str) -> int:
    print(f"{test_bin} {message}", file=sys.stderr)
    print(f"FAIL {test_bin} [internal]")
    return INTERNAL_EXIT_CODE


def main(argv: list[str]) -> int:
    runner_name = Path(argv[0]).name if argv else "run_test_dlibc.py"
    try:
        timeout, wrap, remaining = parse_args(argv[1:])
    except UsageError as exc:
        return report_internal(runner_name, str(exc))

    # resolve the test binary to an absolute path
    test_bin = resolve_test_bin(remaining[0])
    test_args = remaining[1:]

    script_dir = Path(__file__).resolve().parent
    artifact_root = Path(
        os.environ.get("DLIBC_STDIO_ARTIFACTS", str(script_dir / "logs" / "test_stdio"))
    )
    # create root folder for artifacts
    artifact_root.mkdir(parents=True, exist_ok=True)

    test_rel = test_bin
    script_dir_prefix = str(script_dir) + os.sep
    if test_bin.startswith(script_dir_prefix):
        test_rel = test_bin[len(script_dir_prefix) :]

    artifact_dir = artifact_root / artifact_subpath(test_rel)
    # remove existing artifacts
    shutil.rmtree(artifact_dir, ignore_errors=True)
    artifact_dir.mkdir(parents=True, exist_ok=True)

    tmpdir = tempfile.mkdtemp()
    tmpdir_path = Path(tmpdir)
    (tmpdir_path / "input_sets").mkdir(parents=True, exist_ok=True)
    (tmpdir_path / "output_sets" / "stdio").mkdir(parents=True, exist_ok=True)

    host_stdout = artifact_dir / "host.stdout"
    host_stderr = artifact_dir / "host.stderr"
    meta_file = artifact_dir / "meta.txt"
    dandelion_stdout = artifact_dir / "dandelion.stdout"
    dandelion_stderr = artifact_dir / "dandelion.stderr"

    command = [*shlex.split(wrap), test_bin, *test_args] if wrap else [test_bin, *test_args]

    rc = 0
    timed_out = False
    try:
        with host_stdout.open("wb") as stdout_handle, host_stderr.open("wb") as stderr_handle:
            try:
                process = subprocess.Popen(
                    command,
                    cwd=tmpdir,
                    stdout=stdout_handle,
                    stderr=stderr_handle,
                    preexec_fn=set_stack_limit,
                )
            except OSError as exc:
                rc = 1
                print(f"{command[0]} exec failed: {exc.strerror}", file=stderr_handle)
            else:
                try:
                    rc = process.wait(timeout=timeout)
                except subprocess.TimeoutExpired:
                    timed_out = True
                    process.kill()
                    rc = process.wait()

        write_meta(meta_file, test_bin, test_rel, timeout, wrap, rc, artifact_dir, tmpdir)
        replay_outputs(tmpdir_path, artifact_dir, host_stdout, host_stderr, dandelion_stdout, dandelion_stderr)
    except Exception as exc:
        try:
            write_meta(meta_file, test_bin, test_rel, timeout, wrap, INTERNAL_EXIT_CODE, artifact_dir, tmpdir)
        except Exception:
            pass
        return report_internal(test_bin, f"internal runner failure: {exc}")
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)

    if timed_out:
        print(f"FAIL {test_bin} [timed out]")
        return 1
    if rc == 0:
        return 0
    if rc > 0:
        print(f"FAIL {test_bin} [status {rc}]")
        return 1
    if rc < 0:
        print(f"FAIL {test_bin} {format_signal_failure(rc)}")
        return 1

    print(f"FAIL {test_bin} [unknown]")
    return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

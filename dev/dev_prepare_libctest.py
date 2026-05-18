#!/usr/bin/env python3
"""Prepare the generated libc-test worktree from upstream plus local overlay."""

from __future__ import annotations

import argparse
import hashlib
import os
import shutil
import subprocess
import sys
from pathlib import Path


OVERLAY_PATHS = [
    "upstream-ref.txt",
    "libc-test.patch",
    ".dockerignore",
    "config.mak.dlibc",
    "gen_csv.py",
    "run_test_dlibc.py",
    "check_csv.py",
    "scripts/analyze_results.ipynb",
    "scripts/stub_inventory/libc_extension_manual_stubbed_functions.csv",
    "scripts/stub_inventory/newlib_shim_manual_stubbed_functions.csv",
    "scripts/stub_inventory/stubbed_functions.csv",
]


def parse_args() -> argparse.Namespace:
    script_dir = Path(__file__).resolve().parent
    libctest_dir_default = (script_dir / "../libc_test").resolve()
    state_dir_default = script_dir / ".dlibc-dev"
    sdk_build_default = Path(os.environ.get("SDK_BUILD", state_dir_default / "sdk-build"))
    worktree_default = Path(os.environ.get("LIBCTEST_WORKTREE", sdk_build_default / "libc-test"))

    parser = argparse.ArgumentParser(
        description=(
            "Prepare the generated libc-test worktree under SDK_BUILD from upstream plus "
            "the local patch."
        )
    )
    parser.add_argument(
        "--libctest-dir",
        default=os.environ.get("LIBCTEST_DIR", str(libctest_dir_default)),
        help="Path to the tracked libc_test shim directory.",
    )
    parser.add_argument(
        "--sdk-build",
        default=str(sdk_build_default),
        help="SDK build directory containing the generated libc-test checkout.",
    )
    parser.add_argument(
        "--worktree",
        default=str(worktree_default),
        help="Path to the generated libc-test worktree.",
    )
    parser.add_argument(
        "--upstream-url",
        default="https://repo.or.cz/libc-test.git",
        help="Upstream libc-test Git URL.",
    )
    parser.add_argument(
        "--reset",
        action="store_true",
        help="Reset the generated worktree before reapplying the patch.",
    )
    parser.add_argument(
        "--fetch-missing",
        action="store_true",
        help="Fetch from origin if the pinned upstream ref is missing locally.",
    )
    return parser.parse_args()


def run_cmd(*args: str, cwd: Path | None = None, capture_output: bool = False) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=cwd,
        check=True,
        text=True,
        capture_output=capture_output,
    )


def overlay_state(base: Path) -> str:
    digest = hashlib.sha256()
    for rel_path in OVERLAY_PATHS:
        path = base / rel_path
        digest.update(rel_path.encode("utf-8"))
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    return digest.hexdigest()


def copy_overlay(libctest_dir: Path, worktree: Path) -> None:
    shutil.copy2(libctest_dir / ".dockerignore", worktree / ".dockerignore")
    shutil.copy2(libctest_dir / "config.mak.dlibc", worktree / "config.mak.dlibc")
    shutil.copy2(libctest_dir / "gen_csv.py", worktree / "gen_csv.py")
    shutil.copy2(libctest_dir / "run_test_dlibc.py", worktree / "run_test_dlibc.py")
    shutil.copy2(libctest_dir / "check_csv.py", worktree / "check_csv.py")

    stub_dir = worktree / "scripts/stub_inventory"
    stub_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(libctest_dir / "scripts/analyze_results.ipynb", worktree / "scripts/analyze_results.ipynb")
    for csv_path in (libctest_dir / "scripts/stub_inventory").glob("*.csv"):
        shutil.copy2(csv_path, stub_dir / csv_path.name)


def ensure_worktree(worktree: Path, upstream_url: str) -> None:
    if (worktree / ".git").is_dir():
        return
    shutil.rmtree(worktree, ignore_errors=True)
    run_cmd("git", "clone", upstream_url, str(worktree))


def ensure_upstream_ref(worktree: Path, upstream_ref: str, fetch_missing: bool) -> None:
    result = subprocess.run(
        ["git", "-C", str(worktree), "cat-file", "-e", f"{upstream_ref}^{{commit}}"],
        text=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    if result.returncode == 0:
        return
    if not fetch_missing:
        raise SystemExit(
            f"Pinned upstream ref {upstream_ref} is not available in {worktree}.\n"
            "Rerun with --fetch-missing to fetch it from origin."
        )
    run_cmd("git", "-C", str(worktree), "fetch", "origin")


def main() -> int:
    args = parse_args()
    libctest_dir = Path(args.libctest_dir).resolve()
    sdk_build = Path(args.sdk_build).resolve()
    worktree = Path(args.worktree).resolve()
    upstream_ref_file = libctest_dir / "upstream-ref.txt"
    patch_file = libctest_dir / "libc-test.patch"
    stamp_file = worktree / ".dandelion-prepared"

    if not upstream_ref_file.is_file():
        raise SystemExit(f"Pinned upstream ref file not found: {upstream_ref_file}")
    if not patch_file.is_file():
        raise SystemExit(f"Patch file not found: {patch_file}")

    upstream_ref = upstream_ref_file.read_text(encoding="utf-8").strip()
    expected_state = overlay_state(libctest_dir)

    sdk_build.mkdir(parents=True, exist_ok=True)
    ensure_worktree(worktree, args.upstream_url)
    ensure_upstream_ref(worktree, upstream_ref, args.fetch_missing)

    current_state = stamp_file.read_text(encoding="utf-8").strip() if stamp_file.is_file() else ""

    if args.reset or current_state != expected_state:
        run_cmd("git", "-C", str(worktree), "reset", "--hard", upstream_ref)
        run_cmd("git", "-C", str(worktree), "clean", "-fdx")
        run_cmd("git", "-C", str(worktree), "apply", str(patch_file))
        copy_overlay(libctest_dir, worktree)
        stamp_file.write_text(f"{expected_state}\n", encoding="utf-8")

    (libctest_dir / "logs").mkdir(parents=True, exist_ok=True)
    (libctest_dir / "report").mkdir(parents=True, exist_ok=True)

    print(f"Prepared libc-test worktree at {worktree}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        sys.stderr.write(f"Command failed with exit code {exc.returncode}: {' '.join(exc.cmd)}\n")
        raise SystemExit(exc.returncode)

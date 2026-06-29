#!/usr/bin/env python3
"""Generate a detailed test results CSV.

Usage:
    gen_csv.py [build_dir]

Output columns:
    test,result,failure_stage,evidence_file
"""

from __future__ import annotations

import csv
import os
import sys
from pathlib import Path


RUNTIME_EXCLUDED_SUFFIXES = (".o.err", ".lo.err", ".ld.err", ".so.err")
BUILD_SUFFIXES = (".o.err", ".lo.err", ".ld.err", ".so.err")
STAGE_INFO = {
    ".ld.err": ("link", 4),
    ".so.err": ("shared_link", 3),
    ".o.err": ("compile", 2),
    ".lo.err": ("shared_compile", 1),
}


def path_has_common(path: str) -> bool:
    return "common" in Path(path).parts


def iter_matching_files(build_dir: str, suffixes: tuple[str, ...]) -> list[str]:
    matches: list[str] = []
    for root, _, files in os.walk(build_dir):
        for filename in files:
            full_path = os.path.join(root, filename)
            # exlucde common directory as it contains no tests
            if path_has_common(full_path):
                continue
            # check for suffixes of selected build stages
            if filename.endswith(suffixes):
                matches.append(full_path)
    matches.sort()
    return matches


def artifact_exists(err_path: str) -> bool:
    if err_path.endswith(".ld.err"):
        return os.path.exists(err_path[: -len(".ld.err")] + ".exe")
    if err_path.endswith(".so.err"):
        return os.path.exists(err_path[: -len(".so.err")] + ".so")
    if err_path.endswith(".o.err"):
        return os.path.exists(err_path[: -len(".o.err")] + ".o")
    if err_path.endswith(".lo.err"):
        return os.path.exists(err_path[: -len(".lo.err")] + ".lo")
    return False


def contains_fail_line(path: str) -> bool:
    with open(path, "r", encoding="utf-8", errors="replace") as handle:
        return any(line.startswith("FAIL ") for line in handle)


def relative_test_name(path: str, build_dir: str, suffix: str) -> str:
    without_suffix = path[: -len(suffix)]
    return os.path.relpath(without_suffix, build_dir)


def collect_runtime_rows(build_dir: str) -> tuple[set[str], list[tuple[str, str, str, str]]]:
    ran_tests: set[str] = set()
    rows: list[tuple[str, str, str, str]] = []

    for err_path in iter_matching_files(build_dir, (".err",)):
        if err_path.endswith(RUNTIME_EXCLUDED_SUFFIXES):
            continue

        name = relative_test_name(err_path, build_dir, ".err")
        ran_tests.add(name)

        if contains_fail_line(err_path):
            rows.append((name, "FAIL", "runtime", err_path))
        else:
            rows.append((name, "PASS", "pass", err_path))

    return ran_tests, rows


def collect_build_rows(build_dir: str, ran_tests: set[str]) -> list[tuple[str, str, str, str]]:
    best_by_test: dict[str, tuple[int, str, str]] = {}

    for err_path in iter_matching_files(build_dir, BUILD_SUFFIXES):
        if os.path.getsize(err_path) == 0:
            continue
        if artifact_exists(err_path):
            continue

        suffix = next((item for item in BUILD_SUFFIXES if err_path.endswith(item)), None)
        if suffix is None:
            continue

        name = relative_test_name(err_path, build_dir, suffix)
        if name in ran_tests:
            continue

        stage, priority = STAGE_INFO[suffix]
        current = best_by_test.get(name)
        if current is None or priority > current[0]:
            best_by_test[name] = (priority, stage, err_path)

    rows = [
        (name, "FAIL", stage, evidence)
        for name, (_, stage, evidence) in best_by_test.items()
    ]
    return rows


def main(argv: list[str]) -> int:
    # set build directory or default to src
    build_dir = argv[1] if len(argv) > 1 else "src"

    runtime_tests, rows = collect_runtime_rows(build_dir)
    rows.extend(collect_build_rows(build_dir, runtime_tests))
    rows.sort(key=lambda row: row[0])

    writer = csv.writer(sys.stdout, lineterminator="\n")
    writer.writerow(("test", "result", "failure_stage", "evidence_file"))
    writer.writerows(rows)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

#!/usr/bin/env python3
"""Validate a libc-test CSV summary and print a concise result summary."""

from __future__ import annotations

import csv
import sys
from pathlib import Path


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("Usage: check_csv.py <csv-path>", file=sys.stderr)
        return 1

    csv_path = Path(argv[1])
    if not csv_path.is_file():
        print(f"Missing libc-test CSV report: {csv_path}", file=sys.stderr)
        return 1

    with csv_path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    failures = [row for row in rows if row.get("result") != "PASS"]
    passes = sum(1 for row in rows if row.get("result") == "PASS")

    print(f"libc-test summary: passes={passes} failures={len(failures)} total={len(rows)}")

    if failures:
        print("First failing rows:", file=sys.stderr)
        for row in failures[:20]:
            print(
                f"{row.get('test', '')},{row.get('failure_stage', '')},{row.get('evidence_file', '')}",
                file=sys.stderr,
            )
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

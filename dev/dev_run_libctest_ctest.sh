#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

usage() {
  cat <<'EOF'
Usage: ./dev_run_libctest_ctest.sh <sdk-src> <sdk-build> <sdk-install> <libctest-dir> <libctest-worktree>

Run the full libc-test suite for CTest and fail if any test case fails.
EOF
}

if [[ $# -ne 5 ]]; then
  usage >&2
  exit 1
fi

SDK_SRC="$1"
SDK_BUILD="$2"
SDK_INSTALL="$3"
LIBCTEST_DIR="$4"
LIBCTEST_WORKTREE="$5"
LOG_NAME="ctest.log"
CSV_NAME="ctest.csv"

if [[ ! -f "$SDK_BUILD/CMakeCache.txt" ]]; then
  echo "SDK build directory is not configured: $SDK_BUILD" >&2
  exit 1
fi

mkdir -p "$LIBCTEST_DIR/logs"

cmake --install "$SDK_BUILD"

if [[ ! -x "$SDK_INSTALL/create-compiler.sh" ]]; then
  echo "Missing compiler wrapper generator: $SDK_INSTALL/create-compiler.sh" >&2
  exit 1
fi

"$SDK_INSTALL/create-compiler.sh" -c clang-20

env \
  SDK_SRC="$SDK_SRC" \
  SDK_BUILD="$SDK_BUILD" \
  SDK_INSTALL="$SDK_INSTALL" \
  LIBCTEST_DIR="$LIBCTEST_DIR" \
  LIBCTEST_WORKTREE="$LIBCTEST_WORKTREE" \
  "$SCRIPT_DIR/dev_rebuild.sh" test --log "$LOG_NAME" --csv "$CSV_NAME"

python3 - "$LIBCTEST_DIR/logs/$CSV_NAME" <<'PY'
import csv
import sys
from pathlib import Path

csv_path = Path(sys.argv[1])
if not csv_path.is_file():
    print(f"Missing libc-test CSV report: {csv_path}", file=sys.stderr)
    raise SystemExit(1)

with csv_path.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))

failures = [row for row in rows if row.get("result") != "PASS"]
passes = sum(1 for row in rows if row.get("result") == "PASS")

print(f"libc-test summary: passes={passes} failures={len(failures)} total={len(rows)}")

if failures:
    print("First failing rows:", file=sys.stderr)
    for row in failures[:20]:
        print(
            f"{row.get('test','')},{row.get('failure_stage','')},{row.get('evidence_file','')}",
            file=sys.stderr,
        )
    raise SystemExit(1)
PY

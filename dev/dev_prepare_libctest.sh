#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LIBCTEST_DIR="${LIBCTEST_DIR:-$(cd "$SCRIPT_DIR/../libc_test" && pwd)}"
ROOT_DIR_DEFAULT="$(cd "$LIBCTEST_DIR/.." && pwd)"
STATE_DIR_DEFAULT="${SCRIPT_DIR}/.dlibc-dev"
SDK_BUILD="${SDK_BUILD:-$STATE_DIR_DEFAULT/sdk-build}"
LIBCTEST_WORKTREE="${LIBCTEST_WORKTREE:-$SDK_BUILD/libc-test}"
UPSTREAM_URL="https://repo.or.cz/libc-test.git"
UPSTREAM_REF_FILE="$LIBCTEST_DIR/upstream-ref.txt"
PATCH_FILE="$LIBCTEST_DIR/libc-test.patch"
STAMP_FILE="$LIBCTEST_WORKTREE/.dandelion-prepared"
RESET_TREE=false
FETCH_MISSING=false

usage() {
  cat <<'EOF'
Usage: ./dev_prepare_libctest.sh [options]

Prepare the generated libc-test worktree under SDK_BUILD from upstream plus
the local patch.

Options:
  --reset         Reset the generated worktree before reapplying the patch
  --fetch-missing Fetch from origin if the pinned upstream ref is missing locally
  -h, --help      Show help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --reset)
      RESET_TREE=true
      shift
      ;;
    --fetch-missing)
      FETCH_MISSING=true
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ ! -f "$UPSTREAM_REF_FILE" ]]; then
  echo "Pinned upstream ref file not found: $UPSTREAM_REF_FILE" >&2
  exit 1
fi
if [[ ! -f "$PATCH_FILE" ]]; then
  echo "Patch file not found: $PATCH_FILE" >&2
  exit 1
fi

UPSTREAM_REF="$(tr -d '[:space:]' < "$UPSTREAM_REF_FILE")"

overlay_state() {
  python3 - "$LIBCTEST_DIR" <<'PY'
from pathlib import Path
import hashlib
import sys

base = Path(sys.argv[1])
paths = [
    "upstream-ref.txt",
    "libc-test.patch",
    ".dockerignore",
    "config.mak.dlibc",
    "gen_csv.py",
    "run_test_dlibc.py",
    "scripts/analyze_results.ipynb",
    "scripts/stub_inventory/libc_extension_manual_stubbed_functions.csv",
    "scripts/stub_inventory/newlib_shim_manual_stubbed_functions.csv",
    "scripts/stub_inventory/stubbed_functions.csv",
]

digest = hashlib.sha256()
for rel in paths:
    digest.update(rel.encode("utf-8"))
    digest.update(b"\0")
    digest.update((base / rel).read_bytes())
    digest.update(b"\0")

print(digest.hexdigest())
PY
}

copy_overlay() {
  cp "$LIBCTEST_DIR/.dockerignore" "$LIBCTEST_WORKTREE/.dockerignore"
  cp "$LIBCTEST_DIR/config.mak.dlibc" "$LIBCTEST_WORKTREE/config.mak.dlibc"
  cp "$LIBCTEST_DIR/gen_csv.py" "$LIBCTEST_WORKTREE/gen_csv.py"
  cp "$LIBCTEST_DIR/run_test_dlibc.py" "$LIBCTEST_WORKTREE/run_test_dlibc.py"
  mkdir -p "$LIBCTEST_WORKTREE/scripts/stub_inventory"
  cp "$LIBCTEST_DIR/scripts/analyze_results.ipynb" "$LIBCTEST_WORKTREE/scripts/analyze_results.ipynb"
  cp "$LIBCTEST_DIR/scripts/stub_inventory/"*.csv "$LIBCTEST_WORKTREE/scripts/stub_inventory/"
}

EXPECTED_STATE="$(overlay_state)"

if [[ ! -d "$LIBCTEST_WORKTREE/.git" ]]; then
  rm -rf "$LIBCTEST_WORKTREE"
  git clone "$UPSTREAM_URL" "$LIBCTEST_WORKTREE"
fi

if ! git -C "$LIBCTEST_WORKTREE" cat-file -e "${UPSTREAM_REF}^{commit}" 2>/dev/null; then
  if [[ "$FETCH_MISSING" == true ]]; then
    git -C "$LIBCTEST_WORKTREE" fetch origin
  else
    echo "Pinned upstream ref $UPSTREAM_REF is not available in $LIBCTEST_WORKTREE." >&2
    echo "Rerun with --fetch-missing to fetch it from origin." >&2
    exit 1
  fi
fi

CURRENT_STATE=""
if [[ -f "$STAMP_FILE" ]]; then
  CURRENT_STATE="$(tr -d '[:space:]' < "$STAMP_FILE")"
fi

if [[ "$RESET_TREE" == true || "$CURRENT_STATE" != "$EXPECTED_STATE" ]]; then
  git -C "$LIBCTEST_WORKTREE" reset --hard "$UPSTREAM_REF"
  git -C "$LIBCTEST_WORKTREE" clean -fdx
  git -C "$LIBCTEST_WORKTREE" apply "$PATCH_FILE"
  copy_overlay
  printf '%s\n' "$EXPECTED_STATE" > "$STAMP_FILE"
fi

mkdir -p "$LIBCTEST_DIR/logs" "$LIBCTEST_DIR/report"

echo "Prepared libc-test worktree at $LIBCTEST_WORKTREE"

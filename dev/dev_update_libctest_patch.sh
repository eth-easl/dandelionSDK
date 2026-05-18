#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LIBCTEST_DIR="${LIBCTEST_DIR:-$(cd "$SCRIPT_DIR/../libc_test" && pwd)}"
STATE_DIR_DEFAULT="${SCRIPT_DIR}/.dlibc-dev"
SDK_BUILD="${SDK_BUILD:-$STATE_DIR_DEFAULT/sdk-build}"
LIBCTEST_WORKTREE="${LIBCTEST_WORKTREE:-$SDK_BUILD/libc-test}"
PATCH_FILE="$LIBCTEST_DIR/libc-test.patch"

if [[ ! -d "$LIBCTEST_WORKTREE/.git" ]]; then
  echo "Generated libc-test worktree not found: $LIBCTEST_WORKTREE" >&2
  exit 1
fi

if [[ ! -f "$PATCH_FILE" ]]; then
  echo "Patch file not found: $PATCH_FILE" >&2
  exit 1
fi

git -C "$LIBCTEST_WORKTREE" diff -- . > "$PATCH_FILE"
echo "Wrote patch to $PATCH_FILE"

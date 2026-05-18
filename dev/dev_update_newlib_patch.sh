#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LIBCTEST_DIR="${LIBCTEST_DIR:-$(cd "$SCRIPT_DIR/../libc_test" && pwd)}"
ROOT_DIR_DEFAULT="$(cd "$LIBCTEST_DIR/.." && pwd)"
STATE_DIR_DEFAULT="${SCRIPT_DIR}/.dlibc-dev"
SDK_SRC="${SDK_SRC:-$ROOT_DIR_DEFAULT}"
SDK_BUILD="${SDK_BUILD:-$STATE_DIR_DEFAULT/sdk-build}"
NEWLIB_SRC_DIR="${SDK_BUILD}/newlib-cygwin/src/newlib"
PATCH_FILE="${SDK_SRC}/newlib_shim/newlib-cygwin-3.5.3.patch"
EXCLUDE_PATHS=(
  ':(exclude)configure'
  ':(exclude)*/configure'
  ':(exclude)Makefile.in'
  ':(exclude)*/Makefile.in'
  ':(exclude)aclocal.m4'
  ':(exclude)*/aclocal.m4'
  ':(exclude)config.h.in'
  ':(exclude)*/config.h.in'
  ':(exclude)autom4te.cache'
  ':(exclude)*/autom4te.cache'
  ':(exclude)libtool'
  ':(exclude)*/libtool'
  ':(exclude)config.status'
  ':(exclude)*/config.status'
  ':(exclude)Makefile'
  ':(exclude)*/Makefile'
  ':(exclude)stamp-h1'
  ':(exclude)*/stamp-h1'
)

if [[ ! -d "$NEWLIB_SRC_DIR/.git" ]]; then
  echo "Cached newlib checkout not found: $NEWLIB_SRC_DIR" >&2
  exit 1
fi

if [[ ! -d "$SDK_SRC/newlib_shim" ]]; then
  echo "SDK shim directory not found: $SDK_SRC/newlib_shim" >&2
  exit 1
fi

git -C "$NEWLIB_SRC_DIR" diff -- . "${EXCLUDE_PATHS[@]}" > "$PATCH_FILE"
echo "Wrote patch to $PATCH_FILE"

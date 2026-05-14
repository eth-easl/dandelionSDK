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
PATCH_PATHS=(
  config.sub
  newlib/Makefile.am
  newlib/configure.host
  newlib/libc/acinclude.m4
  newlib/libc/include/sys/config.h
  newlib/libc/include/sys/stat.h
  newlib/libc/include/sys/resource.h
  newlib/libc/include/sys/unistd.h
  newlib/libc/include/sys/_pthreadtypes.h
  newlib/libc/include/sys/features.h
  newlib/libc/sys/Makefile.inc
  newlib/libc/include/pthread.h
  newlib/libc/include/glob.h
  newlib/libc/include/sys/_default_fcntl.h
  newlib/libc/include/fcntl.h
  newlib/libc/include/complex.h
  newlib/libc/include/regex.h
  newlib/libc/include/search.h
  newlib/libc/include/signal.h
  newlib/libc/include/sys/signal.h
  newlib/libc/include/sys/wait.h
  newlib/libc/include/stdlib.h
  newlib/libc/include/limits.h
  newlib/libc/locale/locale.c
  newlib/libc/include/stdio.h
  newlib/libc/include/machine/setjmp.h
  newlib/libc/search/tfind.c
  newlib/libc/sys/dandelion/time.c
  newlib/libc/include/time.h
  newlib/libc/include/tgmath.h
  newlib/libm/ld/s_nextafterl.c
  newlib/libm/ld/s_nexttoward.c
  newlib/libm/ld/s_nexttowardf.c
)

if [[ ! -d "$NEWLIB_SRC_DIR/.git" ]]; then
  echo "Cached newlib checkout not found: $NEWLIB_SRC_DIR" >&2
  exit 1
fi

if [[ ! -d "$SDK_SRC/newlib_shim" ]]; then
  echo "SDK shim directory not found: $SDK_SRC/newlib_shim" >&2
  exit 1
fi

git -C "$NEWLIB_SRC_DIR" diff -- "${PATCH_PATHS[@]}" > "$PATCH_FILE"
echo "Wrote patch to $PATCH_FILE"

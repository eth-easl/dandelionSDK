#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

usage() {
  cat <<'EOF'
Usage: ./dev_rebuild.sh [all|sdk|test] [options]

Commands:
  all    Rebuild/install dandelionSDK, then build+run libc-test (default)
  sdk    Rebuild/install dandelionSDK only
  test   Build+run libc-test only (uses already-installed SDK)

Options:
  --arch ARCH         Target architecture (default: x86_64)
  --platform NAME     Dandelion platform (default: debug)
  --build-type TYPE   CMake build type (default: Release)
  --jobs N            Parallel jobs (default: nproc)
  --clean             Run clean steps before building
  --refresh-newlib-patch
                       Reapply the cached newlib patch without recloning
  --reset-newlib-tree  Reset the cached newlib checkout before reapplying
                       the patch (useful after patch edits)
  --log FILE          Write build/test logs to logs/FILE
  --csv FILE          Write CSV output to logs/FILE (instead of stdout)
  -h, --help          Show help

Environment defaults:
  SDK_SRC       (default: sibling dandelionSDK dir, or dev_shell-provided mount)
  SDK_BUILD     (default: sibling sdk-build dir, or dev_shell-provided mount)
  SDK_INSTALL   (default: sibling dandelion_sdk dir, or dev_shell-provided mount)
  LIBCTEST_DIR  (default: script directory, or dev_shell-provided mount)
EOF
}

COMMAND="all"
if [[ $# -gt 0 ]]; then
  case "$1" in
    all|sdk|test)
      COMMAND="$1"
      shift
      ;;
  esac
fi

ARCH="${TARGET_ARCH:-x86_64}"
PLATFORM="${DANDELION_PLATFORM:-debug}"
BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
JOBS="$(nproc)"
CLEAN=false
REFRESH_NEWLIB_PATCH=false
RESET_NEWLIB_TREE=false
LOG_FILE=""
CSV_FILE=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --arch)
      ARCH="$2"
      shift 2
      ;;
    --platform)
      PLATFORM="$2"
      shift 2
      ;;
    --build-type)
      BUILD_TYPE="$2"
      shift 2
      ;;
    --jobs)
      JOBS="$2"
      shift 2
      ;;
    --clean)
      CLEAN=true
      shift
      ;;
    --refresh-newlib-patch)
      REFRESH_NEWLIB_PATCH=true
      shift
      ;;
    --reset-newlib-tree)
      RESET_NEWLIB_TREE=true
      REFRESH_NEWLIB_PATCH=true
      shift
      ;;
    --log)
      LOG_FILE="$2"
      shift 2
      ;;
    --csv)
      CSV_FILE="$2"
      shift 2
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

LIBCTEST_DIR="${LIBCTEST_DIR:-$SCRIPT_DIR}"
ROOT_DIR_DEFAULT="$(cd "$LIBCTEST_DIR/.." && pwd)"
SDK_SRC="${SDK_SRC:-$ROOT_DIR_DEFAULT/dandelionSDK}"
SDK_BUILD="${SDK_BUILD:-$ROOT_DIR_DEFAULT/sdk-build}"
SDK_INSTALL="${SDK_INSTALL:-$ROOT_DIR_DEFAULT/dandelion_sdk}"
LOG_DIR="${LIBCTEST_DIR}/logs"
SDK_CC="${SDK_INSTALL}/${ARCH}-unknown-dandelion-clang"
NEWLIB_SRC_DIR="${SDK_BUILD}/newlib-cygwin/src/newlib"
NEWLIB_BUILD_DIR="${SDK_BUILD}/newlib-cygwin/src/newlib-build"
NEWLIB_PATCH_STAMP="${NEWLIB_BUILD_DIR}/patch_applied"
NEWLIB_EP_STAMP_DIR="${SDK_BUILD}/newlib-cygwin/src/newlib-stamp"
NEWLIB_COMPLETE_STAMP="${SDK_BUILD}/CMakeFiles/newlib-complete"

if [[ ! -d "$SDK_SRC" ]]; then
  echo "SDK source directory not found: $SDK_SRC" >&2
  exit 1
fi
if [[ ! -f "$SDK_SRC/CMakeLists.txt" ]]; then
  echo "SDK source is not dandelionSDK repo root: $SDK_SRC" >&2
  echo "Expected file missing: $SDK_SRC/CMakeLists.txt" >&2
  exit 1
fi
if [[ ! -d "$LIBCTEST_DIR" ]]; then
  echo "libc-test directory not found: $LIBCTEST_DIR" >&2
  exit 1
fi

mkdir -p "$SDK_BUILD" "$SDK_INSTALL"

normalize_output_path() {
  local file_name
  file_name="$(basename "$1")"
  printf '%s/%s\n' "$LOG_DIR" "$file_name"
}

mkdir -p "$LOG_DIR"

if [[ -n "$LOG_FILE" ]]; then
  LOG_FILE="$(normalize_output_path "$LOG_FILE")"
fi

if [[ -n "$CSV_FILE" ]]; then
  CSV_FILE="$(normalize_output_path "$CSV_FILE")"
fi

if [[ -n "$LOG_FILE" ]]; then
  : >"$LOG_FILE"
fi

run_cmd() {
  if [[ -n "$LOG_FILE" ]]; then
    "$@" >>"$LOG_FILE" 2>&1
  else
    "$@"
  fi
}

refresh_newlib_patch() {
  if [[ ! -d "$NEWLIB_SRC_DIR/.git" ]]; then
    echo "==> Cached newlib checkout not found at $NEWLIB_SRC_DIR; skipping patch refresh"
    return
  fi

  if [[ "$RESET_NEWLIB_TREE" == true ]]; then
    echo "==> Resetting cached newlib checkout"
    run_cmd git -C "$NEWLIB_SRC_DIR" reset --hard HEAD
    run_cmd git -C "$NEWLIB_SRC_DIR" clean -fd
  fi

  echo "==> Clearing newlib patch stamp"
  run_cmd rm -f "$NEWLIB_PATCH_STAMP"

  echo "==> Invalidating newlib external-project stamps"
  run_cmd rm -f \
    "$NEWLIB_EP_STAMP_DIR/newlib-patch_disconnected" \
    "$NEWLIB_EP_STAMP_DIR/newlib-configure" \
    "$NEWLIB_EP_STAMP_DIR/newlib-build" \
    "$NEWLIB_EP_STAMP_DIR/newlib-install" \
    "$NEWLIB_EP_STAMP_DIR/newlib-done" \
    "$NEWLIB_COMPLETE_STAMP"
}

rebuild_sdk() {
  if [[ "$REFRESH_NEWLIB_PATCH" == true ]]; then
    refresh_newlib_patch
  fi

  echo "==> Configuring dandelionSDK in $SDK_BUILD"
  run_cmd cmake -S "$SDK_SRC" -B "$SDK_BUILD" \
    -DCMAKE_C_COMPILER=clang-20 \
    -DCMAKE_CXX_COMPILER=clang++-20 \
    -DDANDELION_PLATFORM="$PLATFORM" \
    -DARCHITECTURE="$ARCH" \
    -DNEWLIB=ON \
    -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$SDK_INSTALL"

  echo "==> Building required SDK targets"
  # Build only core SDK targets needed for install; skip sample binaries in
  # dandelionSDK/test_programs that can fail on cross-arch Docker hosts.
  run_cmd cmake --build "$SDK_BUILD" \
    --target dandelion_system dandelion_runtime dandelion_file_system newlib llvmproject \
    -j"$JOBS"

  echo "==> Installing dandelionSDK"
  run_cmd cmake --install "$SDK_BUILD"

  echo "==> Creating wrapped compiler"
  run_cmd "$SDK_INSTALL/create-compiler.sh" -c clang-20
  # On the Docker workflow the copied clang wrapper can lose its execute bit
  # on the bind-mounted install dir, which later breaks libc-test with
  # "Permission denied" when invoking $SDK_CC.
  run_cmd chmod u+x "$SDK_CC"
  if [[ ! -x "$SDK_CC" ]]; then
    echo "Wrapped compiler is not executable after creation: $SDK_CC" >&2
    exit 1
  fi
}

emit_csv() {
  if [[ -n "$CSV_FILE" ]]; then
    python3 "$LIBCTEST_DIR/gen_csv.py" >"$CSV_FILE"
    echo "CSV written to $CSV_FILE"
  else
    python3 "$LIBCTEST_DIR/gen_csv.py"
  fi
}

run_tests() {
  if [[ ! -x "$SDK_CC" ]]; then
    echo "Expected compiler wrapper not found: $SDK_CC" >&2
    echo "Run: ./dev_rebuild.sh sdk --arch $ARCH" >&2
    exit 1
  fi

  cd "$LIBCTEST_DIR"
  cp config.mak.dlibc config.mak
  mkdir -p input_sets output_sets

  if [[ "$CLEAN" == true ]]; then
    run_cmd make cleanall
  fi

  echo "==> Building and running libc-test"
  if ! run_cmd make -k -j"$JOBS" CC="$SDK_CC"; then
    true
  fi
  emit_csv
}

case "$COMMAND" in
  sdk)
    rebuild_sdk
    ;;
  test)
    run_tests
    ;;
  all)
    rebuild_sdk
    run_tests
    ;;
esac

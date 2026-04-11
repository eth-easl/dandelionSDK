#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
IMAGE_NAME="${IMAGE_NAME:-dev_dandelion_sdk}"
CONTAINER_NAME="${CONTAINER_NAME:-dev_dandelion_sdk_container}"
SDK_DIR_DEFAULT="${DANDELION_SDK_DIR:-$SCRIPT_DIR/../}"
STATE_DIR_DEFAULT="${DLIBC_DEV_STATE_DIR:-$SCRIPT_DIR/.dlibc-dev}"
CONTAINER_ROOT_DEFAULT="${DLIBC_CONTAINER_ROOT:-/work}"
LIBCTEST_MOUNT_DEFAULT="${LIBCTEST_MOUNT:-$CONTAINER_ROOT_DEFAULT/libc-test}"
SDK_MOUNT_DEFAULT="${SDK_MOUNT:-$CONTAINER_ROOT_DEFAULT/dandelionSDK}"
SDK_BUILD_MOUNT_DEFAULT="${SDK_BUILD_MOUNT:-$CONTAINER_ROOT_DEFAULT/sdk-build}"
SDK_INSTALL_MOUNT_DEFAULT="${SDK_INSTALL_MOUNT:-$CONTAINER_ROOT_DEFAULT/dandelion_sdk}"

SDK_DIR="$SDK_DIR_DEFAULT"
STATE_DIR="$STATE_DIR_DEFAULT"
LIBCTEST_MOUNT="$LIBCTEST_MOUNT_DEFAULT"
SDK_MOUNT="$SDK_MOUNT_DEFAULT"
SDK_BUILD_MOUNT="$SDK_BUILD_MOUNT_DEFAULT"
SDK_INSTALL_MOUNT="$SDK_INSTALL_MOUNT_DEFAULT"
ENSURE_ONLY=false
REBUILD_IMAGE=false
RECREATE_CONTAINER=false

require_docker() {
  if ! command -v docker >/dev/null 2>&1; then
    echo "Error: docker command not found in PATH" >&2
    exit 1
  fi
}

abspath_existing_dir() {
  local dir="$1"
  (
    cd "$dir" >/dev/null 2>&1 && pwd -P
  )
}

abspath_parent_child() {
  local path="$1"
  local parent
  local child
  parent="$(dirname "$path")"
  child="$(basename "$path")"
  (
    mkdir -p "$parent" >/dev/null 2>&1
    cd "$parent" >/dev/null 2>&1 && printf '%s/%s\n' "$(pwd -P)" "$child"
  )
}

usage() {
  cat <<EOF
Usage: $(basename "$0") [options]

Start/reuse an interactive dev container for incremental dandelionSDK + libc-test work.

Options:
  --sdk DIR            Path to dandelionSDK source (default: .. (parent folder))
  --state DIR          Host directory for persistent build/install state (default: .dlibc-dev)
  --libctest-mount DIR Container path for the libc-test repo (default: /work/libc-test)
  --sdk-mount DIR      Container path for the dandelionSDK repo (default: /work/dandelionSDK)
  --sdk-build-mount DIR
                       Container path for the SDK build dir (default: /work/sdk-build)
  --sdk-install-mount DIR
                       Container path for the SDK install dir (default: /work/dandelion_sdk)
  --container NAME     Docker container name (default: dev_dandelion_sdk_container)
  --image NAME         Docker image name (default: dev_dandelion_sdk)
  --rebuild-image      Force rebuild of the Docker image
  --recreate-container Remove and recreate the container before use
  --ensure-only        Ensure container is running and exit (no interactive shell)
  -h, --help           Show this help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --sdk)
      SDK_DIR="$2"
      shift 2
      ;;
    --state)
      STATE_DIR="$2"
      shift 2
      ;;
    --libctest-mount)
      LIBCTEST_MOUNT="$2"
      shift 2
      ;;
    --sdk-mount)
      SDK_MOUNT="$2"
      shift 2
      ;;
    --sdk-build-mount)
      SDK_BUILD_MOUNT="$2"
      shift 2
      ;;
    --sdk-install-mount)
      SDK_INSTALL_MOUNT="$2"
      shift 2
      ;;
    --container)
      CONTAINER_NAME="$2"
      shift 2
      ;;
    --image)
      IMAGE_NAME="$2"
      shift 2
      ;;
    --rebuild-image)
      REBUILD_IMAGE=true
      shift
      ;;
    --recreate-container)
      RECREATE_CONTAINER=true
      shift
      ;;
    --ensure-only)
      ENSURE_ONLY=true
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

require_docker

if [[ ! -d "$SDK_DIR" ]]; then
  echo "Error: SDK directory not found: $SDK_DIR" >&2
  exit 1
fi
if [[ ! -f "$SDK_DIR/CMakeLists.txt" ]]; then
  echo "Error: SDK directory does not look like dandelionSDK root: $SDK_DIR" >&2
  echo "Expected file missing: $SDK_DIR/CMakeLists.txt" >&2
  exit 1
fi

SCRIPT_DIR="$(abspath_existing_dir "$SCRIPT_DIR")"
SDK_DIR="$(abspath_existing_dir "$SDK_DIR")"
STATE_DIR="$(abspath_parent_child "$STATE_DIR")"

for path_var in LIBCTEST_MOUNT SDK_MOUNT SDK_BUILD_MOUNT SDK_INSTALL_MOUNT; do
  path_value="${!path_var}"
  if [[ "$path_value" != /* ]]; then
    echo "Error: $path_var must be an absolute container path: $path_value" >&2
    exit 1
  fi
done

mkdir -p "$STATE_DIR/sdk-build" "$STATE_DIR/dandelion_sdk"

image_exists() {
  docker image inspect "$IMAGE_NAME" >/dev/null 2>&1
}

if [[ "$REBUILD_IMAGE" == true ]] || ! image_exists; then
  echo "Building image '$IMAGE_NAME'..."
  docker buildx build --load \
    -f "$SCRIPT_DIR/Dockerfile.dlibc" \
    -t "$IMAGE_NAME" \
    "$SCRIPT_DIR"
fi

container_exists() {
  docker container inspect "$CONTAINER_NAME" >/dev/null 2>&1
}

container_running() {
  [[ "$(docker inspect -f '{{.State.Running}}' "$CONTAINER_NAME")" == "true" ]]
}

container_image_matches() {
  local image_id
  local container_image_id

  image_id="$(docker image inspect -f '{{.Id}}' "$IMAGE_NAME")"
  container_image_id="$(docker inspect -f '{{.Image}}' "$CONTAINER_NAME")"
  [[ "$image_id" == "$container_image_id" ]]
}

if container_exists && [[ "$RECREATE_CONTAINER" == true ]]; then
  echo "Removing existing container '$CONTAINER_NAME'..."
  docker rm -f "$CONTAINER_NAME" >/dev/null
fi

if container_exists && ! container_image_matches; then
  echo "Existing container '$CONTAINER_NAME' uses an outdated image; recreating it..."
  docker rm -f "$CONTAINER_NAME" >/dev/null
fi

if container_exists; then
  if ! container_running; then
    echo "Starting existing container '$CONTAINER_NAME'..."
    docker start "$CONTAINER_NAME" >/dev/null
  fi
else
  echo "Creating container '$CONTAINER_NAME'..."
  docker run -d \
    --name "$CONTAINER_NAME" \
    -e SDK_SRC="$SDK_MOUNT" \
    -e SDK_BUILD="$SDK_BUILD_MOUNT" \
    -e SDK_INSTALL="$SDK_INSTALL_MOUNT" \
    -e LIBCTEST_DIR="$LIBCTEST_MOUNT" \
    -v "$SCRIPT_DIR:$LIBCTEST_MOUNT" \
    -v "$SDK_DIR:$SDK_MOUNT" \
    -v "$STATE_DIR/sdk-build:$SDK_BUILD_MOUNT" \
    -v "$STATE_DIR/dandelion_sdk:$SDK_INSTALL_MOUNT" \
    -w "$LIBCTEST_MOUNT" \
    "$IMAGE_NAME" \
    sleep infinity >/dev/null
fi

echo "Container '$CONTAINER_NAME' is ready."
echo "libc-test:   $SCRIPT_DIR -> $LIBCTEST_MOUNT"
echo "SDK mount:   $SDK_DIR -> $SDK_MOUNT"
echo "State mount: $STATE_DIR/sdk-build -> $SDK_BUILD_MOUNT"
echo "State mount: $STATE_DIR/dandelion_sdk -> $SDK_INSTALL_MOUNT"
echo "Inside container, use:"
echo "  cd $LIBCTEST_MOUNT"
echo "  ./dev_rebuild.sh all         # incremental SDK rebuild + libc-test run + CSV"
echo "  ./dev_rebuild.sh sdk         # incremental SDK rebuild only"
echo "  ./dev_rebuild.sh test        # rerun libc-test against installed SDK"

if [[ "$ENSURE_ONLY" == true ]]; then
  exit 0
fi

docker_exec_args=(
  -e SDK_SRC="$SDK_MOUNT"
  -e SDK_BUILD="$SDK_BUILD_MOUNT"
  -e SDK_INSTALL="$SDK_INSTALL_MOUNT"
  -e LIBCTEST_DIR="$LIBCTEST_MOUNT"
)

if [[ -t 0 && -t 1 ]]; then
  docker_exec_args+=(-it)
else
  docker_exec_args+=(-i)
fi

exec docker exec \
  "${docker_exec_args[@]}" \
  "$CONTAINER_NAME" bash

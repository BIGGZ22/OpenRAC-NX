#!/usr/bin/env bash
# Run a command inside the rac2-build container (see the Dockerfile next to this file), with the
# repository mounted at the same path it has on the host. Uses a local image, else the published
# one, else builds it.
#
#   bash tools/docker/run.sh tools/compiler/build.sh
#   bash tools/docker/run.sh python3 tools/compiler/sdk/build.py
#   bash tools/docker/run.sh python3 tools/build.py all
set -euo pipefail
repo=$(cd "$(dirname "$0")/../.." && pwd)
CLI="${CONTAINER_CLI:-$(command -v podman >/dev/null 2>&1 && echo podman || echo docker)}"
GHCR_IMAGE="${RAC2_BUILD_IMAGE:-ghcr.io/openrac/rac2-build:latest}"
LOCAL_IMAGE="rac2-build:latest"
if $CLI image inspect "$LOCAL_IMAGE" >/dev/null 2>&1; then
  image="$LOCAL_IMAGE"
elif $CLI image inspect "$GHCR_IMAGE" >/dev/null 2>&1 || $CLI pull "$GHCR_IMAGE"; then
  image="$GHCR_IMAGE"
else
  echo "Building $LOCAL_IMAGE (once; about 15 minutes)..."
  $CLI build --platform linux/386 -t "$LOCAL_IMAGE" -f "$repo/tools/docker/Dockerfile" "$repo"
  image="$LOCAL_IMAGE"
fi
exec $CLI run --rm --platform linux/386 -v "$repo:$repo" -w "$repo" -e WORK=/tmp/rac2-compiler \
  "$image" bash -c 'wineserver -p >/dev/null 2>&1 || true; exec "$@"' bash "$@"

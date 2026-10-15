#!/usr/bin/env bash
# Start, stop or inspect the Ghidra MCP container (docs/CONTAINERS.md):
# headless Ghidra with PS2 EmotionEngine support and an MCP server, for
# LLM agents. The retail executable is mounted read-only from baserom/.
#
#   bash tools/docker/ghidra_mcp.sh start    # first start imports and analyses
#   bash tools/docker/ghidra_mcp.sh logs
#   bash tools/docker/ghidra_mcp.sh stop
set -euo pipefail
repo=$(cd "$(dirname "$0")/../.." && pwd)

CONTAINER_CLI="${CONTAINER_CLI:-$(command -v podman || command -v docker || true)}"
[ -n "$CONTAINER_CLI" ] || { echo "Error: neither podman nor docker found in PATH" >&2; exit 1; }

NAME="${GHIDRA_MCP_CONTAINER:-ghidra-ps2-mcp}"
IMAGE="${GHIDRA_MCP_IMAGE:-ghcr.io/lynder063/rac1-ghidra-ps2-mcp:latest}"
TOKEN="${GHIDRA_MCP_AUTH_TOKEN:-rac1-local}"
PROJECTS="${GHIDRA_PROJECTS_DIR:-$HOME/.local/share/rac1-ghidra}"

case "${1:-start}" in
start)
  [ -f "$repo/baserom/SCES_509.16" ] || { echo "Error: baserom/SCES_509.16 missing (README step 2)" >&2; exit 1; }
  mkdir -p "$PROJECTS"
  "$CONTAINER_CLI" rm -f "$NAME" >/dev/null 2>&1 || true
  sec=(); [ "$(basename "$CONTAINER_CLI")" = podman ] && sec=(--security-opt label=disable)
  "$CONTAINER_CLI" run -d --name "$NAME" ${sec[@]+"${sec[@]}"} \
    -p 127.0.0.1:8089:8089 -p 127.0.0.1:8081:8081 \
    -v "$PROJECTS:/projects" -v "$repo/baserom:/baserom:ro" \
    -e PS2_BINARY=/baserom/SCES_509.16 \
    -e GHIDRA_MCP_AUTH_TOKEN="$TOKEN" \
    -e JAVA_OPTS="${JAVA_OPTS:--Xmx4g -XX:+UseG1GC}" \
    "$IMAGE" >/dev/null
  echo "$NAME started: REST http://127.0.0.1:8089  MCP http://127.0.0.1:8081/mcp  token $TOKEN"
  echo "First start imports and analyses the executable; follow it with: $0 logs"
  ;;
stop) "$CONTAINER_CLI" stop -t 5 "$NAME" ;;
logs) exec "$CONTAINER_CLI" logs -f "$NAME" ;;
status) "$CONTAINER_CLI" ps -a --filter "name=^${NAME}\$" ;;
*) echo "usage: $0 start|stop|logs|status" >&2; exit 2 ;;
esac

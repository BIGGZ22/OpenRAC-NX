#!/bin/bash
# Entrypoint for Ghidra MCP with PS2 EmotionEngine decompiler support
set -e

PORT=${GHIDRA_MCP_PORT:-8089}
MCP_PORT=${GHIDRA_MCP_BRIDGE_PORT:-8081}
BIND_ADDRESS=${GHIDRA_MCP_BIND_ADDRESS:-"0.0.0.0"}
JAVA_OPTS=${JAVA_OPTS:-"-Xmx4g -XX:+UseG1GC"}
GHIDRA_HOME=${GHIDRA_HOME:-"/opt/ghidra"}

# Auth token: if not set, generate a default secure token
if [ -z "${GHIDRA_MCP_AUTH_TOKEN}" ]; then
    export GHIDRA_MCP_AUTH_TOKEN=$(head -c 32 /dev/urandom | od -An -tx1 | tr -d ' \n')
fi

echo "=========================================================="
echo "  Ghidra MCP Server with PS2 (EmotionEngine R5900) Support"
echo "=========================================================="
echo "  Ghidra Home:      ${GHIDRA_HOME}"
echo "  REST API Port:    ${PORT} (Bind: ${BIND_ADDRESS})"
echo "  MCP Bridge Port:  ${MCP_PORT} (/mcp)"
echo "  Auth Token:       ${GHIDRA_MCP_AUTH_TOKEN}"
echo "=========================================================="

# Check Ghidra installation
if [ ! -d "${GHIDRA_HOME}" ]; then
    echo "Error: Ghidra not found at ${GHIDRA_HOME}"
    exit 1
fi

# Build CLASSPATH with all Ghidra Framework, Feature, Processors, and Extensions
CLASSPATH="/app/GhidraMCP.jar"
for jar in ${GHIDRA_HOME}/Ghidra/Framework/*/lib/*.jar; do
    [ -f "$jar" ] && CLASSPATH="${CLASSPATH}:${jar}"
done
for jar in ${GHIDRA_HOME}/Ghidra/Features/*/lib/*.jar; do
    [ -f "$jar" ] && CLASSPATH="${CLASSPATH}:${jar}"
done
for jar in ${GHIDRA_HOME}/Ghidra/Processors/*/lib/*.jar; do
    [ -f "$jar" ] && CLASSPATH="${CLASSPATH}:${jar}"
done
for jar in ${GHIDRA_HOME}/Ghidra/Extensions/*/lib/*.jar; do
    [ -f "$jar" ] && CLASSPATH="${CLASSPATH}:${jar}"
done
if [ -d "/app/lib" ]; then
    for jar in /app/lib/*.jar; do
        [ -f "$jar" ] && CLASSPATH="${CLASSPATH}:${jar}"
    done
fi

mkdir -p /projects /data

PROJECT_NAME=${PROJECT_NAME:-"rac1"}
PROJECT_DIR=${PROJECT_DIR:-"/projects"}

# Auto-import PS2 binary if provided and project doesn't exist
if [ -n "${PS2_BINARY}" ] && [ -f "${PS2_BINARY}" ]; then
    PROGRAM_BASENAME=$(basename "${PS2_BINARY}")
    GPR_FILE="${PROJECT_DIR}/${PROJECT_NAME}.gpr"
    
    if [ ! -f "${GPR_FILE}" ] || [ "${FORCE_REIMPORT:-0}" = "1" ]; then
        echo "==> Importing PS2 binary ${PS2_BINARY} as r5900:LE:32:default into ${PROJECT_NAME}..."
        "${GHIDRA_HOME}/support/analyzeHeadless" \
            "${PROJECT_DIR}" "${PROJECT_NAME}" \
            -import "${PS2_BINARY}" \
            -processor "r5900:LE:32:default" \
            -overwrite || echo "Warning: analyzeHeadless returned non-zero, continuing..."
        echo "==> PS2 binary import complete."
    fi
    
    if [ -z "${PROJECT_PATH}" ] && [ -z "${PROGRAM_FILE}" ]; then
        PROJECT_PATH="${PROJECT_DIR}/${PROJECT_NAME}.gpr"
        PROGRAM_NAME="/${PROGRAM_BASENAME}"
    fi
fi

# Build arguments for GhidraMCPHeadlessServer
ARGS="--port ${PORT} --bind ${BIND_ADDRESS}"

if [ -n "${PROGRAM_FILE}" ] && [ -f "${PROGRAM_FILE}" ]; then
    ARGS="${ARGS} --file ${PROGRAM_FILE}"
fi

if [ -n "${PROJECT_PATH}" ]; then
    ARGS="${ARGS} --project ${PROJECT_PATH}"
    if [ -n "${PROGRAM_NAME}" ]; then
        ARGS="${ARGS} --program ${PROGRAM_NAME}"
    fi
fi

# Append any extra command line arguments
if [ "$#" -gt 0 ]; then
    ARGS="${ARGS} $@"
fi

# Track child PIDs
GHIDRA_PID=""
BRIDGE_PID=""

cleanup() {
    echo ""
    echo "Shutting down Ghidra MCP services..."
    [ -n "$BRIDGE_PID" ] && kill -TERM "$BRIDGE_PID" 2>/dev/null || true
    [ -n "$GHIDRA_PID" ] && kill -TERM "$GHIDRA_PID" 2>/dev/null || true
    wait 2>/dev/null || true
    echo "Done."
    exit 0
}

trap cleanup SIGTERM SIGINT

echo "==> Starting GhidraMCP Headless Server..."
java \
    ${JAVA_OPTS} \
    -Dghidra.home="${GHIDRA_HOME}" \
    -Dapplication.name=GhidraMCP \
    -classpath "${CLASSPATH}" \
    com.xebyte.headless.GhidraMCPHeadlessServer \
    ${ARGS} &
GHIDRA_PID=$!

# Wait for Ghidra REST API to become ready
echo "==> Waiting for Ghidra REST server on port ${PORT}..."
MAX_WAIT=60
WAITED=0
while [ $WAITED -lt $MAX_WAIT ]; do
    if curl -sf -H "Authorization: Bearer ${GHIDRA_MCP_AUTH_TOKEN}" "http://127.0.0.1:${PORT}/check_connection" >/dev/null 2>&1 || \
       curl -sf "http://127.0.0.1:${PORT}/check_connection" >/dev/null 2>&1; then
        echo "==> Ghidra REST server is UP and responding on port ${PORT}!"
        break
    fi
    sleep 1
    WAITED=$((WAITED + 1))
done

if [ $WAITED -ge $MAX_WAIT ]; then
    echo "Warning: Ghidra REST server did not respond within ${MAX_WAIT}s. Starting bridge anyway..."
fi

# Check if stdio mode was requested
if [ "${1:-}" = "stdio" ] || [ "${MODE:-}" = "stdio" ]; then
    echo "==> Starting Ghidra MCP Bridge in STDIO mode..."
    export GHIDRA_MCP_URL="http://127.0.0.1:${PORT}"
    exec bridge-mcp-ghidra --transport stdio
fi

# Start Python MCP Bridge (streamable-http)
echo "==> Starting Ghidra MCP Bridge on port ${MCP_PORT}..."
export GHIDRA_MCP_URL="http://127.0.0.1:${PORT}"
bridge-mcp-ghidra --transport streamable-http --mcp-host 0.0.0.0 --mcp-port "${MCP_PORT}" &
BRIDGE_PID=$!

echo "=========================================================="
echo "  Ghidra MCP is fully READY!"
echo "  - REST API:   http://localhost:${PORT}/"
echo "  - MCP Server: http://localhost:${MCP_PORT}/mcp"
echo "  - Auth Token: ${GHIDRA_MCP_AUTH_TOKEN}"
echo "=========================================================="

# Wait for both processes
wait -n "$GHIDRA_PID" "$BRIDGE_PID"
cleanup

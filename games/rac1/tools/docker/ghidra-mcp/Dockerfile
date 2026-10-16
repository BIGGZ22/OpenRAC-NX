FROM docker.io/library/eclipse-temurin:21-jdk

LABEL org.opencontainers.image.title="Ghidra MCP Server with PS2 EmotionEngine Decompiler" \
      org.opencontainers.image.description="Ghidra 12.1.3 headless server with bethington/ghidra-mcp v7 and chaoticgd/ghidra-emotionengine-reloaded PS2 R5900 support" \
      org.opencontainers.image.source="https://github.com/lynder063/rac1-decomp"

ARG GHIDRA_VERSION=12.1.3
ARG GHIDRA_DATE=20260817
ARG EE_VERSION=v2.1.37
ARG GHIDRA_MCP_VERSION=v7.0.0-rc.1

ENV GHIDRA_HOME=/opt/ghidra
ENV PATH="${GHIDRA_HOME}:${PATH}"
ENV PYTHONUNBUFFERED=1

# Install runtime packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    wget \
    unzip \
    python3 \
    python3-pip \
    procps \
    git \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# 1. Download and install Ghidra
WORKDIR /opt
RUN echo "Downloading Ghidra ${GHIDRA_VERSION}..." \
 && curl -sSL \
    "https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_${GHIDRA_VERSION}_build/ghidra_${GHIDRA_VERSION}_PUBLIC_${GHIDRA_DATE}.zip" \
    -o ghidra.zip \
 && unzip -q ghidra.zip \
 && rm ghidra.zip \
 && mv ghidra_* ghidra

# 2. Install Jython extension (shipped in Ghidra)
RUN set -eux; \
    EXT_DIR="/opt/ghidra/Extensions/Ghidra"; \
    JYTHON_ZIP="$(ls "$EXT_DIR"/*_Jython.zip 2>/dev/null | head -n1)"; \
    if [ -n "$JYTHON_ZIP" ]; then \
        mkdir -p /opt/ghidra/Ghidra/Extensions; \
        unzip -q "$JYTHON_ZIP" -d /opt/ghidra/Ghidra/Extensions/; \
        echo "Installed Jython extension"; \
    fi

# 3. Install Ghidra EmotionEngine Reloaded extension (PS2 R5900 Support)
RUN set -eux; \
    echo "Downloading PS2 EmotionEngine Reloaded extension ${EE_VERSION}..." \
 && curl -sSL \
    "https://github.com/chaoticgd/ghidra-emotionengine-reloaded/releases/download/${EE_VERSION}/ghidra_12.1.3_PUBLIC_20260825_ghidra-emotionengine-reloaded.zip" \
    -o /tmp/ee.zip \
 && unzip -q /tmp/ee.zip -d /opt/ghidra/Ghidra/Extensions/ \
 && rm -f /tmp/ee.zip \
 && echo "PS2 EmotionEngine extension installed successfully."

# 4. Install GhidraMCP extension & standalone headless JAR
RUN set -eux; \
    mkdir -p /app /data /projects /baserom; \
    echo "Downloading GhidraMCP extension ${GHIDRA_MCP_VERSION}..." \
 && curl -sSL \
    "https://github.com/bethington/ghidra-mcp/releases/download/${GHIDRA_MCP_VERSION}/GhidraMCP-7.0.0-rc.1.zip" \
    -o /tmp/ghidra_mcp.zip \
 && unzip -q /tmp/ghidra_mcp.zip -d /opt/ghidra/Ghidra/Extensions/ \
 && cp /opt/ghidra/Ghidra/Extensions/GhidraMCP/lib/GhidraMCP-*.jar /app/GhidraMCP.jar \
 && rm -f /tmp/ghidra_mcp.zip \
 && echo "GhidraMCP jar and extension installed successfully."

# 5. Install Python MCP Bridge
RUN pip3 install --no-cache-dir --break-system-packages \
    "mcp>=1.28.1,<2.0.0" \
    "pydantic>=2.0.0" \
    "https://github.com/bethington/ghidra-mcp/releases/download/${GHIDRA_MCP_VERSION}/ghidra_mcp_bridge-7.0.0-py3-none-any.whl"

# 6. Copy Entrypoint script
COPY tools/docker/ghidra-mcp/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

WORKDIR /app
EXPOSE 8089 8081

ENTRYPOINT ["/app/entrypoint.sh"]

# Containers

Two container images serve this repository. Both work with Podman or
Docker, and both are published on GitHub Container Registry, so nothing has
to be built locally.

| Image | Purpose | Source |
|---|---|---|
| `ghcr.io/lynder063/rac1-build` | The build: SN ProDG under 32-bit Wine, plus the Python tools | `tools/docker/Dockerfile` |
| `ghcr.io/lynder063/rac1-ghidra-ps2-mcp` | Headless Ghidra with PS2 EmotionEngine support, exposed to LLM agents over MCP | `tools/docker/ghidra-mcp/` |

Neither image contains the game. Both read your own `baserom/SCES_509.16`
from a mount (README step 2), and the build image also needs `toolchain/`
(README step 4).

## rac1-build

Every build and decompilation tool runs through it on Linux and macOS:

```sh
bash tools/docker/run.sh bash tools/setup_asm.sh
bash tools/docker/run.sh bash tools/build_sn.sh
bash tools/docker/run.sh python3 tools/try_func.py func_XXXXXXXX cand.c --diff
```

`run.sh` mounts the repository at the same path it has on the host (so
paths in logs line up), works in it, and removes the container when the
command exits. It picks the image in this order:

1. a locally built `localhost/rac1-build:latest` (Podman) or
   `rac1-build:latest` (Docker);
2. `ghcr.io/lynder063/rac1-build:latest` if already pulled;
3. a pull from GHCR;
4. if the pull fails, a local build from `tools/docker/Dockerfile`
   (about 15 minutes).

Overrides, as environment variables:

| Variable | Effect |
|---|---|
| `CONTAINER_CLI` | `podman` or `docker`; default: Podman if installed |
| `RAC1_BUILD_IMAGE` | image to pull instead of `ghcr.io/lynder063/rac1-build:latest` |

The image is `linux/386` (32-bit Wine needs it); on x86-64 hosts it runs
natively, on ARM Macs through emulation. Podman gets
`--security-opt label=disable` so SELinux hosts (Fedora) can read the
mount.

To rebuild the image yourself, for example after changing
`requirements.txt`:

```sh
podman build --platform linux/386 -t localhost/rac1-build:latest -f tools/docker/Dockerfile .
```

A local image takes precedence over the published one; remove it
(`podman rmi localhost/rac1-build:latest`) to go back. On pushes to
`main` that touch `tools/docker/` or `requirements.txt`,
`.github/workflows/docker-publish.yml` rebuilds and publishes the GHCR
image.

## rac1-ghidra-ps2-mcp

Ghidra 12.1.3, run headless, with
[ghidra-emotionengine-reloaded](https://github.com/chaoticgd/ghidra-emotionengine-reloaded)
for the R5900 and [GhidraMCP](https://github.com/bethington/ghidra-mcp)
serving the program over a REST API and an MCP server. It is meant for
LLM agents: they ask it for a function's decompilation or disassembly
while writing C. The project has no scripts for importing into a desktop
Ghidra; use Ghidra's own importer if you want one.

Ghidra output is a reference, like m2c's, and is never pasted into
`src/`.

### A level's own code

The project holds only the executable, so a level's own code (the part
that replaces `main` when the level loads, docs/OVERLAYS.md) is not in it,
and the headless server cannot import files. `tools/ghidra_level.py`
fills that gap:

```sh
python3 tools/ghidra_level.py 18                    # every stub still INCLUDE_ASM in src/overlays/l18_*/
python3 tools/ghidra_level.py 18 func_L18_002DD8A8  # just these
```

It builds a flat image of the level's memory (the resident executable
below 0x15F000, then the level's records at their load addresses),
imports it into a scratch project inside the running container with
`analyzeHeadless` (the container's own project is untouched), creates a
function at every address this repository names, and writes the
decompiler's C to `build-sn/ghidra/lNN/out/`. Nothing it writes is
tracked.

### Start and stop

```sh
bash tools/docker/ghidra_mcp.sh start     # or: stop, logs, status
```

On the first start the container imports `SCES_509.16` as
`r5900:LE:32:default` and runs Ghidra's auto-analysis. That takes several
minutes; `bash tools/docker/ghidra_mcp.sh logs` shows progress, and the
server is ready when it prints `Ghidra MCP is fully READY!`. The analysed
project is kept on the host, so later starts take seconds.

| Port (host, `127.0.0.1` only) | Service |
|---|---|
| 8089 | REST API |
| 8081 | MCP server, streamable HTTP, at `/mcp` |

| Variable | Default | Effect |
|---|---|---|
| `GHIDRA_MCP_AUTH_TOKEN` | `rac1-local` | bearer token for both services |
| `GHIDRA_PROJECTS_DIR` | `~/.local/share/rac1-ghidra` | where the analysed project is stored |
| `GHIDRA_MCP_IMAGE` | `ghcr.io/lynder063/rac1-ghidra-ps2-mcp:latest` | image to run |
| `GHIDRA_MCP_CONTAINER` | `ghidra-ps2-mcp` | container name |
| `JAVA_OPTS` | `-Xmx4g -XX:+UseG1GC` | JVM options; raise `-Xmx` if analysis runs out of memory |
| `CONTAINER_CLI` | Podman if installed | `podman` or `docker` |

To redo the import and analysis, delete the projects directory, or start
the container with `-e FORCE_REIMPORT=1`.

Build the image locally with:

```sh
podman build -t localhost/rac1-ghidra-ps2-mcp:latest -f tools/docker/ghidra-mcp/Dockerfile .
GHIDRA_MCP_IMAGE=localhost/rac1-ghidra-ps2-mcp:latest bash tools/docker/ghidra_mcp.sh start
```

### Connecting an agent

Over HTTP, for clients that support streamable-HTTP MCP servers:

```json
{
  "mcpServers": {
    "ghidra-ps2": {
      "type": "http",
      "url": "http://127.0.0.1:8081/mcp",
      "headers": { "Authorization": "Bearer rac1-local" }
    }
  }
}
```

Over stdio, for clients that only start MCP servers as processes. This
runs the bridge inside the running container:

```json
{
  "mcpServers": {
    "ghidra-ps2": {
      "command": "podman",
      "args": ["exec", "-i", "-e", "GHIDRA_MCP_AUTH_TOKEN=rac1-local",
               "ghidra-ps2-mcp", "bridge-mcp-ghidra", "--transport", "stdio"]
    }
  }
}
```

For Claude Code the file is `.mcp.json` at the repository root (keep it
out of commits if your token is private). Replace `rac1-local` with your
token if you set one.

### REST API

Agents without MCP, and scripts, can use the REST API directly. Addresses
are the retail ones, as in `func_XXXXXXXX`:

```sh
T=rac1-local
curl -H "Authorization: Bearer $T" "http://127.0.0.1:8089/check_connection"
curl -H "Authorization: Bearer $T" "http://127.0.0.1:8089/decompile_function?address=0x001FB530"
curl -H "Authorization: Bearer $T" "http://127.0.0.1:8089/disassemble_function?address=0x001FB530"
curl -H "Authorization: Bearer $T" "http://127.0.0.1:8089/get_function_by_address?address=0x001FB530"
curl -H "Authorization: Bearer $T" "http://127.0.0.1:8089/list_functions?offset=0&limit=100"
```

`decompile_function` returns JSON whose `decompiled` field holds the C.
Ghidra lists EABI parameters with the float ones first, so its parameter
order is not the source order: take that from the registers in the
retail assembly.

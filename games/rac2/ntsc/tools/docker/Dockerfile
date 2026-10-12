# Build environment for hosts that cannot run the toolchain directly (macOS, Linux, Windows):
# the compilers are 32-bit Linux programs built here from source, and SN ProDG's assembler and
# linker are 32-bit Windows programs. Both run natively in a 32-bit (linux/386) Debian with
# classic 32-bit Wine, as rac1-decomp's image does. On Apple Silicon the container runs under
# QEMU user-mode emulation: Rosetta cannot execute 32-bit x86 code.
#
#   docker build --platform linux/386 -t rac2-build -f tools/docker/Dockerfile .
#   bash tools/docker/run.sh tools/compiler/build.sh
#   bash tools/docker/run.sh python3 tools/build.py all
FROM --platform=linux/386 debian:bookworm

LABEL org.opencontainers.image.source="https://github.com/OpenRAC/rac2-gc-decomp"
LABEL org.opencontainers.image.description="Build environment for the Ratchet & Clank: Going Commando PS2 matching decompilation (32-bit Wine, GNU EE 2.9 build tools)"

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      wine make git curl patch ca-certificates flex m4 texinfo \
      gcc g++ libc6-dev python3 python3-pip python3-venv python3-dev \
 && rm -rf /var/lib/apt/lists/*

# Pinned like requirements.txt, so the disassembly regenerates byte-identically.
COPY requirements.txt /tmp/requirements.txt
RUN python3 -m venv /opt/venv \
 && /opt/venv/bin/pip install --no-cache-dir -r /tmp/requirements.txt \
 && ln -s /opt/venv/bin/python /usr/local/bin/python

ENV PATH=/opt/venv/bin:$PATH \
    WINEDEBUG=-all \
    WINEPREFIX=/opt/wineprefix
RUN wineboot -i >/dev/null 2>&1; wineserver -w; chmod -R a+rwX /opt/wineprefix

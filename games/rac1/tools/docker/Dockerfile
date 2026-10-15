# Build environment for hosts that cannot run the SN ProDG toolchain directly
# (macOS, Linux). The toolchain is 32-bit Windows programs, so this image is
# 32-bit (linux/386) Debian with classic 32-bit Wine. On Apple Silicon the
# container runs under QEMU user-mode emulation: Rosetta cannot execute 32-bit
# x86 code, so Wine's WoW64 mode in an amd64 container does not work there.
#
#   docker build --platform linux/386 -t rac1-build -f tools/docker/Dockerfile .
#   bash tools/docker/run.sh bash tools/build_sn.sh
FROM --platform=linux/386 debian:bookworm

LABEL org.opencontainers.image.source="https://github.com/OpenRAC/rac1-decomp"
LABEL org.opencontainers.image.description="Build environment for Ratchet & Clank PS2 matching decompilation (32-bit Wine + ProDG)"

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      wine make git ca-certificates \
      python3 python3-pip python3-venv python3-dev gcc g++ cmake \
 && rm -rf /var/lib/apt/lists/*

# Pinned like requirements.txt, so asm/ regenerates byte-identically.
COPY requirements.txt /tmp/requirements.txt
RUN python3 -m venv /opt/venv \
 && /opt/venv/bin/pip install --no-cache-dir -r /tmp/requirements.txt \
 && ln -s /opt/venv/bin/python /usr/local/bin/python

# asm-differ (tools/diff.sh) and m2c (tools/m2c.py) prerequisites. Levenshtein
# has no 32-bit wheel, so it compiles here (hence g++ and cmake): slow, once.
RUN /opt/venv/bin/pip install --no-cache-dir \
      colorama watchdog levenshtein cxxfilt graphviz pycparser

ENV PATH=/opt/venv/bin:$PATH \
    WINEDEBUG=-all \
    WINEPREFIX=/opt/wineprefix
RUN wineboot -i >/dev/null 2>&1; wineserver -w; chmod -R a+rwX /opt/wineprefix

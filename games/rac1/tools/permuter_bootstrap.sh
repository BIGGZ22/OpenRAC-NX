#!/usr/bin/env bash
# One-time setup for tools/permuter_setup.py and tools/permuter_run.sh:
# clones decomp-permuter and fetches the pieces it needs that our build
# container doesn't already carry, all under tools/ext/ (gitignored, see
# .gitignore's "Third-party decomp tools" entry -- same place tools/m2c.py
# keeps its own clone).
#
# tools/ext/ is not wiped between container runs: the container is started
# with --rm (tools/docker/run.sh), but the repo is bind-mounted, so anything
# written under the repo -- including here -- lands on the host and persists.
# Only the running container itself (its apt state, /opt/venv, etc.) is
# thrown away each time, which is why this script vendors everything it
# fetches into the repo tree instead of just `pip install`/`apt install`ing
# it into the container.
#
#   bash tools/docker/run.sh bash tools/permuter_bootstrap.sh
#
# Re-run any time tools/ext/ is deleted. Needs network access from inside
# the container.
set -euo pipefail
cd "$(dirname "$0")/.."

if [ ! -e tools/ext/decomp-permuter/permuter.py ]; then
  echo "Cloning decomp-permuter..."
  git clone --depth 1 https://github.com/simonlindholm/decomp-permuter.git tools/ext/decomp-permuter
fi

# decomp-permuter's only real dependency beyond what's already in the image
# (Levenshtein, pycparser -- see tools/docker/Dockerfile) is `toml`, to read
# settings.toml. Installed with --target into the repo, not the container's
# venv, so it survives past this container.
if [ ! -d tools/ext/decomp-permuter/.pydeps/toml ]; then
  echo "Fetching toml (for decomp-permuter)..."
  pip install --no-cache-dir --target tools/ext/decomp-permuter/.pydeps toml
fi

# decomp-permuter's scorer needs an objdump that can at least decode plain
# MIPS (R5900/EE-specific opcodes fall back to raw words on both sides of a
# comparison, which is fine for scoring). None of the SN toolchain's own
# tools do this job, and there is no mips-linux-gnu-objdump in the image,
# so fetch the Debian package's payload directly -- not `apt-get install`,
# which would vanish with the container.
if [ ! -x tools/ext/mips-binutils/usr/bin/mips-linux-gnu-objdump ]; then
  echo "Fetching mips-linux-gnu-objdump..."
  apt-get update -qq
  ( cd /tmp && rm -f binutils-mips-linux-gnu_*.deb && apt-get download binutils-mips-linux-gnu )
  mkdir -p tools/ext/mips-binutils
  dpkg-deb -x /tmp/binutils-mips-linux-gnu_*.deb tools/ext/mips-binutils
fi

echo "permuter bootstrap done: tools/ext/decomp-permuter, tools/ext/mips-binutils"

#!/usr/bin/env bash
# Runs decomp-permuter (tools/ext/decomp-permuter, set up by
# tools/permuter_bootstrap.sh) with its vendored Python and objdump
# dependencies on PATH, so its own subprocess calls (compile.sh, objdump)
# find them without anything installed system-wide or left behind when the
# build container exits.
#
#   bash tools/docker/run.sh bash tools/permuter_run.sh build-sn/permuter/func_X -j4 --stop-on-zero
#
# Must run inside the build container: compile.sh, via
# tools/permuter_compile.py, needs Wine and the SN toolchain.
set -euo pipefail
cd "$(dirname "$0")/.."
export PYTHONPATH="$PWD/tools/ext/decomp-permuter/.pydeps${PYTHONPATH:+:$PYTHONPATH}"
export PATH="$PWD/tools/ext/mips-binutils/usr/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/tools/ext/mips-binutils/usr/lib/i386-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec python3 tools/ext/decomp-permuter/permuter.py "$@"

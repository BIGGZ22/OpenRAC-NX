#!/bin/sh
# Build, link, and asm-diff one function against retail.
#   sh tools/diff.sh func_XXXXXXXX [extra asm-differ args]
#
# Refuses to diff after a failed compile: a failed make leaves the previous
# .o behind, whose INCLUDE_ASM stubs still hold retail's bytes, so the diff
# would show a fictional match (this trap has bitten 15+ times).
set -u
. tools/toolchain.sh
[ $# -ge 1 ] || { echo "usage: sh tools/diff.sh func_XXXXXXXX [args]"; exit 2; }
rm -rf build-sn/core build-sn/libgcc
rm -rf build-sn/game
make_sn >build-sn/diff_make.log 2>&1
rc=$?
if [ $rc -ne 0 ]; then
    tail -15 build-sn/diff_make.log
    echo "*** make failed (exit $rc) -- NOT diffing ***"; exit 1
fi
bash rac1.ld.sh >/dev/null || exit 1
sn "$TC/ee-ld.exe" -T build-sn/rac1.ld build-sn/bss_equs.o -o build-sn/rac1.elf || {
    echo "*** link failed -- NOT diffing ***"; exit 1; }
python tools/gen_differ_files.py >/dev/null || exit 1
fn=$1; shift
python tools/ext/asm-differ/diff.py --no-pager -s "$fn" "$@"

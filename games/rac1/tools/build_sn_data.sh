#!/usr/bin/env bash
# Assembles the standalone data/rodata .s files (not part of any C file)
# through the SN Systems ProDG assembler. See docs/TOOLCHAIN.md.
set -euo pipefail
cd "$(dirname "$0")/.."
. tools/toolchain.sh

AS="$TC/ee-as.exe"
mkdir -p build-sn

for f in asm/data/*.s; do
  name=$(basename "$f")
  out="build-sn/${name%.s}.o"
  sn "$AS" -I include-sn -I include -o "$out" "$f"
  echo "assembled $out"
done

# core_rdata minus the read-only data compiled objects now provide
# themselves (config/core_rodata.txt: __divdi3's static __clz_tab, the
# literals of compiled SDK functions). rac1.ld.sh links the pieces with
# each object's .rodata in its hole. See tools/split_data_s.py.
rm -f build-sn/core_rdata_*.s build-sn/core_rdata_*.o
python tools/split_data_s.py asm/data/core_rdata.rodata.s build-sn/core_rdata $(python tools/core_rodata.py labels)
for s in build-sn/core_rdata_*.s; do
  sn "$AS" -I include-sn -I include -o "${s%.s}.o" "$s"
  echo "assembled ${s%.s}.o"
done

# data minus the jump tables that compiled game functions now bring
# themselves (tools/jump_tables.py). rac1.ld.sh links the pieces with each
# table in its hole.
rm -f build-sn/data_*.s build-sn/data_*.o
python tools/split_data_s.py asm/data/data.data.s build-sn/data $(python tools/jump_tables.py labels)
for s in build-sn/data_*.s; do
  sn "$AS" -I include-sn -I include -o "${s%.s}.o" "$s"
  echo "assembled ${s%.s}.o"
done

# bss padding objects -- see rac1.ld.sh for why these are real loaded
# sections rather than NOLOAD.
# core_bss (0x154200-0x15ED80) is split around 0x1597EC, where libgcc's
# __main keeps its static `initialized` (see rac1.ld.sh).
printf '.section .core_bss_pad, "wa"\n.skip 0x55ec\n' > build-sn/core_bss_pad_1.s
printf '.section .core_bss_pad, "wa"\n.skip 0x5590\n' > build-sn/core_bss_pad_2.s
printf '.section .bss_pad, "wa"\n.skip 0x4200\n' > build-sn/bss_pad.s
sn "$AS" -o build-sn/core_bss_pad_1.o build-sn/core_bss_pad_1.s
sn "$AS" -o build-sn/core_bss_pad_2.o build-sn/core_bss_pad_2.s
sn "$AS" -o build-sn/bss_pad.o build-sn/bss_pad.s
echo "assembled build-sn/core_bss_pad_1.o build-sn/core_bss_pad_2.o build-sn/bss_pad.o"

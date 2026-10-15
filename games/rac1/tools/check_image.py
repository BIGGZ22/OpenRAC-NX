#!/usr/bin/env python3
"""
Compare the whole linked image with retail, not just the functions.

sweep_matches.py audits every decompiled function and check_layout.py
checks every function's address, but neither looks at the data segments.
Denormal floats that the assembler read back wrong went unnoticed that
way: 66 data words differed from retail (see fix_denormal_floats.py).

This compares every loaded section of build-sn/rac1.elf with the retail
executable. A byte may differ only inside a function that is compiled
from C (a known near-miss); anywhere else -- data, or a stub's retail
bytes -- it is an error.

Usage: python tools/check_image.py
"""
import sys
from pathlib import Path

from elftools.elf.elffile import ELFFile

sys.path.insert(0, str(Path(__file__).resolve().parent))
from sweep_matches import FUNC_DEF, STUB  # noqa: E402
from libgcc_units import FUNCTIONS as LIBGCC_FUNCTIONS, SEGMENT_SOURCES  # noqa: E402

BASEROM = "baserom/SCES_509.16"
LINKED_ELF = "build-sn/rac1.elf"
# The -G sections parked past the image (rac1.ld.sh).
PARKED = 0x2400000


def compiled_functions() -> set[str]:
    names = set(LIBGCC_FUNCTIONS)
    for srcs in SEGMENT_SOURCES.values():
        for src in srcs:
            text = Path(src).read_text(errors="replace")
            stubs = set(STUB.findall(text))
            names |= {n for n in FUNC_DEF.findall(text) if n not in stubs}
    return names


def main() -> int:
    raw = Path(BASEROM).read_bytes()
    with open(BASEROM, "rb") as f:
        seg = next(s for s in ELFFile(f).iter_segments() if s["p_type"] == "PT_LOAD")
        delta = seg["p_vaddr"] - seg["p_offset"]

    elf = ELFFile(open(LINKED_ELF, "rb"))
    names = compiled_functions()
    spans = sorted((s["st_value"], s["st_value"] + s["st_size"])
                   for s in elf.get_section_by_name(".symtab").iter_symbols()
                   if s.name in names and s["st_size"])

    def in_compiled(addr: int) -> bool:
        return any(a <= addr < b for a, b in spans)

    bad, near = [], 0
    for sec in elf.iter_sections():
        if sec["sh_type"] != "SHT_PROGBITS" or not sec["sh_addr"] or sec["sh_addr"] >= PARKED:
            continue
        base, data = sec["sh_addr"], sec.data()
        want = raw[base - delta:base - delta + len(data)]
        for i in (i for i in range(len(data)) if data[i] != want[i]):
            if in_compiled(base + i):
                near += 1
            else:
                bad.append((sec.name, base + i))

    if bad:
        print(f"*** {len(bad)} byte(s) differ from retail outside any compiled function:")
        for name, addr in bad[:10]:
            print(f"    {name} 0x{addr:08X}")
        return 1
    print(f"image matches retail outside the decompiled near-misses ({near} bytes differ inside them)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

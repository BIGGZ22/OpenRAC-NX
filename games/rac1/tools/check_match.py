#!/usr/bin/env python3
"""
Compares bytes against the retail baserom, byte for byte.

IMPORTANT: once any function's compiled size differs from the original
(even by one instruction), every function *after* it in the same object
shifts in a flat section-vs-section comparison, making that comparison
meaningless past the first size-changing function -- it looks like a
massive regression when it's really just alignment. Per-function checks
(using each function's own retail address + size, from its `nonmatching`
header in asm/) are the only way to verify a specific function once any
earlier one in its object doesn't match exactly.

Usage:
  Whole section (only meaningful before any decompiled function's size
  has drifted from retail -- i.e. right after a fresh `splat split`, or
  once everything in a section actually matches):
    python tools/check_match.py section build-sn/core_text.o core.text

  One function, by symbol name, against a freshly linked build-sn/rac1.elf
  (recommended -- this is the position-independent check: it looks up
  where OUR build put the symbol, reads that many bytes from there, and
  compares against retail's bytes at retail's own address for the same
  symbol, so it works correctly even when an earlier function in the same
  object has drifted in size):
    python tools/check_match.py symbol func_001138A8 0xC

  One function, by its retail vram address + size only, no build-sn/rac1.elf
  needed (just prints retail's bytes -- useful before you've even compiled
  anything yet, or to sanity-check what you're aiming for):
    python tools/check_match.py func 0x112380 0x28
"""
import re
import sys
from elftools.elf.elffile import ELFFile

BASEROM = "baserom/SCES_509.16"
LINKED_ELF = "build-sn/rac1.elf"


def diff_report(orig: bytes, ours: bytes, label: str) -> None:
    n = min(len(orig), len(ours))
    mismatches = sum(1 for i in range(n) if orig[i] != ours[i])
    print(f"{label}: sizes retail={len(orig)} ours={len(ours)}")
    print(f"  byte mismatches: {mismatches}/{n} ({100 * mismatches / max(n, 1):.2f}%)")
    if mismatches:
        for i in range(n):
            if orig[i] != ours[i]:
                print(f"  first diff at offset {i:#x}:")
                print(f"    retail {orig[max(0, i - 4):i + 12].hex()}")
                print(f"    ours   {ours[max(0, i - 4):i + 12].hex()}")
                break


def check_section(obj_path: str, section_name: str) -> None:
    with open(BASEROM, "rb") as f:
        orig = ELFFile(f).get_section_by_name(section_name).data()
    with open(obj_path, "rb") as f:
        ours = ELFFile(f).get_section_by_name(".text").data()
    diff_report(orig, ours, f"{obj_path} vs retail '{section_name}'")


def check_func(vram: int, size: int) -> None:
    """
    Reads the retail bytes directly from the baserom's raw file offset.
    Works out the file offset from the ELF's own program header (the
    core segment: vram 0x100080 <-> rom offset 0x1000, a constant
    0xFF080 delta -- see config/splat.yaml) rather than requiring a
    freshly linked build-sn/rac1.elf, so this works even mid-edit before
    relinking.
    """
    with open(BASEROM, "rb") as f:
        elf = ELFFile(f)
        seg = next(s for s in elf.iter_segments() if s["p_type"] == "PT_LOAD")
        delta = seg["p_vaddr"] - seg["p_offset"]
        f.seek(vram - delta)
        orig = f.read(size)
    print(f"NOTE: reads retail bytes directly by address, not from build-sn/rac1.elf.")
    print(f"Compare 'ours' bytes yourself (e.g. objdump -s on the built object) --")
    print(f"this only prints what retail has at {vram:#x}:")
    print(f"  {orig.hex()}")


def check_symbol(name: str, size: int) -> None:
    """
    Position-independent check: looks up `name`'s address in a freshly
    linked build-sn/rac1.elf, reads `size` bytes from there, and compares
    against retail's bytes at retail's own address for the same symbol
    (parsed straight from the name -- func_XXXXXXXX/D_XXXXXXXX both
    encode their retail vram, splat's own naming convention). This is
    correct even when an earlier function in the same object has a
    different size than retail and everything after it has shifted --
    it never assumes our build's layout matches retail's.
    """
    m = re.match(r"^(?:func|D)_([0-9A-Fa-f]{8})$", name)
    if not m:
        print(f"'{name}' doesn't match func_XXXXXXXX/D_XXXXXXXX -- can't infer its retail address.")
        raise SystemExit(1)
    retail_vram = int(m.group(1), 16)

    with open(LINKED_ELF, "rb") as f:
        elf = ELFFile(f)
        symtab = elf.get_section_by_name(".symtab")
        sym = next((s for s in symtab.iter_symbols() if s.name == name), None)
        if sym is None:
            print(f"'{name}' not found in {LINKED_ELF}'s symbol table.")
            raise SystemExit(1)
        our_vram = sym["st_value"]
        our_size = sym["st_size"]
        sec = elf.get_section(sym["st_shndx"])
        ours = sec.data()[our_vram - sec["sh_addr"]: our_vram - sec["sh_addr"] + size]

    # A function that is CORRECT in its first `size` bytes but LONGER than
    # retail used to report a clean 0/N "match", because only `size` bytes
    # were ever compared -- the surplus fell outside the window entirely.
    # That produced at least three false "match" records (func_0011AE1C,
    # func_001F9B90, func_001F9B98), so size disagreement is now a loud
    # failure in its own right. `st_size` comes from the `.size` directive
    # that labels.inc's glabel/endlabel macros emit, so it is our real
    # compiled extent, not an assumption.
    if our_size and our_size != size:
        verdict = "LONGER THAN RETAIL" if our_size > size else "SHORTER THAN RETAIL"
        print(f"{name}: *** SIZE MISMATCH -- NOT A MATCH ***")
        print(f"  retail={size} (0x{size:X}) bytes, ours={our_size} (0x{our_size:X}) bytes -- {verdict}")
        print(f"  The byte comparison below only covers retail's {size} bytes, so it can")
        print(f"  still read 0/{size}. That is NOT a match while the sizes differ.")

    with open(BASEROM, "rb") as f:
        belf = ELFFile(f)
        seg = next(s for s in belf.iter_segments() if s["p_type"] == "PT_LOAD")
        delta = seg["p_vaddr"] - seg["p_offset"]
        f.seek(retail_vram - delta)
        orig = f.read(size)

    diff_report(orig, ours, f"{name}: our_vram={our_vram:#x} retail_vram={retail_vram:#x}")


def main() -> None:
    if len(sys.argv) < 2:
        print(__doc__)
        raise SystemExit(1)
    mode = sys.argv[1]
    if mode == "section":
        check_section(sys.argv[2], sys.argv[3])
    elif mode == "func":
        check_func(int(sys.argv[2], 16), int(sys.argv[3], 16))
    elif mode == "symbol":
        check_symbol(sys.argv[2], int(sys.argv[3], 16))
    else:
        print(__doc__)
        raise SystemExit(1)


if __name__ == "__main__":
    main()

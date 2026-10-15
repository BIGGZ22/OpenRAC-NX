#!/usr/bin/env python3
"""
Sony's prebuilt libgcc.a as a reference for the libgcc modules.

Every EE compiler directory in the toolchain mirrors ships the same
libgcc.a, and its members are the code retail linked: _divdi3.o,
_moddi3.o, _udivdi3.o, _umoddi3.o and most functions of dp-bit.o and
fp-bit.o match retail byte for byte (relocated fields masked). That turns
"which source and flags reproduce this module?" into a local comparison
against one object, with no link and no baserom needed for the compare
itself.

Usage:
  python tools/libgcc_ref.py retail
      For every function in the archive, where it matches retail exactly
      (searching the libgcc region), or its closest miss.
  python tools/libgcc_ref.py diff OBJECT SYMBOL [MEMBER]
      Disassemble SYMBOL from OBJECT (e.g. a module you just compiled)
      and from the archive, and show the differing instructions. MEMBER
      defaults to the archive member that defines SYMBOL.

A function that matches here and still differs in the linked image is a
layout or link problem, not a source problem.
"""
import difflib
import io
import sys
from pathlib import Path

import rabbitizer as rz
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

ARCHIVE = Path("toolchain/sn-prodg-24/local/sce/ee/gcc/lib/gcc-lib/ee/2.9-ee-991111/libgcc.a")
BASEROM = "baserom/SCES_509.16"
REGION = (0x11DF10, 0x120900)   # libgcc, plus the single-precision object after it


def members(path=ARCHIVE):
    """(name, bytes) for every ELF member of a GNU ar archive."""
    data = path.read_bytes()
    assert data[:8] == b"!<arch>\n", f"{path} is not an ar archive"
    pos, names = 8, None
    while pos < len(data):
        hdr = data[pos:pos + 60]
        pos += 60
        name, size = hdr[:16].decode().strip(), int(hdr[48:58].decode().strip())
        body = data[pos:pos + size]
        pos += size + (size & 1)
        if name == "//":
            names = body
            continue
        if name in ("/", "/SYM64/"):
            continue
        if name.startswith("/") and names is not None:
            off = int(name[1:])
            name = names[off:names.index(b"/", off)].decode()
        if body[:4] == b"\x7fELF":
            yield name.rstrip("/"), body


def functions(obj: bytes):
    """[(symbol, offset, size)], the .text bytes, and the set of byte offsets
    covered by relocations (their contents depend on the link)."""
    elf = ELFFile(io.BytesIO(obj))
    text = elf.get_section_by_name(".text")
    if text is None:
        return [], b"", set()
    mask = set()
    for sec in elf.iter_sections():
        if isinstance(sec, RelocationSection) and sec.name == ".rel.text":
            for rel in sec.iter_relocations():
                o = rel["r_offset"]
                half = rel["r_info_type"] in (5, 6, 7)   # HI16, LO16, GPREL16
                mask |= {o, o + 1} if half else {o, o + 1, o + 2, o + 3}
    tidx = next(i for i, s in enumerate(elf.iter_sections()) if s.name == ".text")
    syms = sorted((s.name, s["st_value"], s["st_size"])
                  for s in elf.get_section_by_name(".symtab").iter_symbols()
                  if s["st_shndx"] == tidx and s["st_info"]["type"] == "STT_FUNC")
    return syms, text.data(), mask


def disassemble(code: bytes):
    return [rz.Instruction(int.from_bytes(code[i:i + 4], "little"),
                           category=rz.InstrCategory.R5900).disassemble()
            for i in range(0, len(code), 4)]


def cmd_retail():
    raw = Path(BASEROM).read_bytes()
    elf = ELFFile(open(BASEROM, "rb"))
    seg = next(s for s in elf.iter_segments() if s["p_type"] == "PT_LOAD")
    delta = seg["p_vaddr"] - seg["p_offset"]
    lo, hi = REGION
    for member, body in members():
        syms, text, mask = functions(body)
        for name, off, size in syms:
            if not size:
                continue
            best = None
            for a in range(lo, hi - size + 1, 4):
                orig = raw[a - delta:a - delta + size]
                d = sum(1 for i in range(size)
                        if off + i not in mask and text[off + i] != orig[i])
                if best is None or d < best[1]:
                    best = (a, d)
                    if not d:
                        break
            verdict = "EXACT" if best[1] == 0 else f"closest, {best[1]} bytes differ"
            print(f"{member:16s} {name:22s} {size:5d}  0x{best[0]:06X}  {verdict}")


def cmd_diff(obj_path, symbol, member=None):
    def pick(syms, text):
        name, off, size = next(s for s in syms if s[0] == symbol)
        return disassemble(text[off:off + size])
    ours = pick(*functions(Path(obj_path).read_bytes())[:2])
    for name, body in members():
        syms, text, _ = functions(body)
        if (member is None or name == member) and any(s[0] == symbol for s in syms):
            ref = pick(syms, text)
            break
    else:
        sys.exit(f"{symbol} not found in {ARCHIVE}")
    print(f"{symbol}: ours {len(ours)} words, Sony's {len(ref)} words")
    same = True
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, ours, ref, autojunk=False).get_opcodes():
        if tag == "equal":
            continue
        same = False
        print(f"--- {tag} ours[{i1}:{i2}] sony[{j1}:{j2}]")
        for k in range(max(i2 - i1, j2 - j1)):
            a = ours[i1 + k] if i1 + k < i2 else ""
            b = ref[j1 + k] if j1 + k < j2 else ""
            print(f"   {a:44s} | {b}")
    if same:
        print("identical instructions (relocated fields are compared as encoded)")


if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1] == "retail":
        cmd_retail()
    elif len(sys.argv) >= 4 and sys.argv[1] == "diff":
        cmd_diff(*sys.argv[2:5])
    else:
        sys.exit(__doc__)

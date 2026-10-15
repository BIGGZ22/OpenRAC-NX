#!/usr/bin/env python3
"""
For every decompiled function that still mismatches, print each differing
32-bit word as retail-vs-ours, lightly decoded.

This exists because the useful unit of analysis is not "N bytes differ"
but "which instruction, and how". Two functions whose residual decodes to
the same instruction pair almost always share one cause and one fix -- see
func_001EC270/func_001EC780, where both residuals were a swapped `addu`
and both were fixed by giving the table a real element type.

Usage: python tools/diff_words.py [func_NAME ...]
"""
import re
import subprocess
import sys
from pathlib import Path
from elftools.elf.elffile import ELFFile

BASEROM = "baserom/SCES_509.16"
LINKED = "build-sn/rac1.elf"

R = ["$0","$at","$v0","$v1","$a0","$a1","$a2","$a3","$t0","$t1","$t2","$t3",
     "$t4","$t5","$t6","$t7","$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
     "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"]
SPECIAL = {0x20:"add",0x21:"addu",0x22:"sub",0x23:"subu",0x24:"and",0x25:"or",
           0x26:"xor",0x27:"nor",0x2A:"slt",0x2B:"sltu",0x2D:"daddu",0x2F:"dsubu",
           0x00:"sll",0x02:"srl",0x03:"sra",0x04:"sllv",0x06:"srlv",0x07:"srav",
           0x08:"jr",0x09:"jalr",0x10:"mfhi",0x12:"mflo",0x18:"mult",0x19:"multu",
           0x1A:"div",0x1B:"divu",0x0A:"movz",0x0B:"movn",0x38:"dsll",
           0x3A:"dsrl",0x3B:"dsra",0x3C:"dsll32",0x3E:"dsrl32",0x3F:"dsra32"}
OPS = {0x08:"addi",0x09:"addiu",0x0A:"slti",0x0B:"sltiu",0x0C:"andi",0x0D:"ori",
       0x0E:"xori",0x0F:"lui",0x04:"beq",0x05:"bne",0x06:"blez",0x07:"bgtz",
       0x20:"lb",0x21:"lh",0x23:"lw",0x24:"lbu",0x25:"lhu",0x28:"sb",0x29:"sh",
       0x2B:"sw",0x02:"j",0x03:"jal",0x37:"ld",0x3F:"sd",0x1E:"lq",0x1F:"sq",
       0x31:"lwc1",0x39:"swc1"}


def decode(w):
    op, rs, rt, rd = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
    sa, fn, imm = (w >> 6) & 31, w & 63, w & 0xFFFF
    s = imm - 0x10000 if imm & 0x8000 else imm
    if op == 0:
        m = SPECIAL.get(fn, "spec:%#x" % fn)
        if m in ("sll", "srl", "sra", "dsll", "dsra", "dsrl", "dsll32", "dsra32", "dsrl32"):
            return "nop" if w == 0 else "%s %s,%s,%d" % (m, R[rd], R[rt], sa)
        if m in ("jr", "mfhi", "mflo"):
            return "%s %s" % (m, R[rs] if m == "jr" else R[rd])
        if m in ("mult", "multu", "div", "divu"):
            return "%s %s,%s,%s" % (m, R[rd], R[rs], R[rt])
        return "%s %s,%s,%s" % (m, R[rd], R[rs], R[rt])
    m = OPS.get(op, "op:%#x" % op)
    if m in ("j", "jal"):
        return "%s %#x" % (m, (w & 0x3FFFFFF) << 2)
    if m == "lui":
        return "lui %s,%#x" % (R[rt], imm)
    if m in ("beq", "bne"):
        return "%s %s,%s,%+d" % (m, R[rs], R[rt], (s + 1) * 4)
    if op in (0x20,0x21,0x23,0x24,0x25,0x28,0x29,0x2B,0x37,0x3F,0x1E,0x1F,0x31,0x39):
        return "%s %s,%d(%s)" % (m, R[rt], s, R[rs])
    return "%s %s,%s,%d" % (m, R[rt], R[rs], s)


def sizes():
    out = {}
    for p in Path("asm/nonmatchings").rglob("*.s"):
        t = p.read_text(errors="ignore")
        m = re.search(r"nonmatching\s+(\w+),\s*(0x[0-9A-Fa-f]+)", t)
        if m:
            out[m.group(1)] = int(m.group(2), 16)
    return out


def main():
    want = set(sys.argv[1:])
    sz = sizes()
    bf = open(BASEROM, "rb")
    belf = ELFFile(bf)
    seg = next(x for x in belf.iter_segments() if x["p_type"] == "PT_LOAD")
    delta = seg["p_vaddr"] - seg["p_offset"]

    of = open(LINKED, "rb")
    oelf = ELFFile(of)
    osyms = {s.name: s for s in oelf.get_section_by_name(".symtab").iter_symbols()}

    def retail(vram, n):
        bf.seek(vram - delta)
        return bf.read(n)

    def ours(name, n):
        s = osyms.get(name)
        if s is None:
            return None
        sec = oelf.get_section(s["st_shndx"])
        off = s["st_value"] - sec["sh_addr"]
        return sec.data()[off:off + n]

    for n in sorted(sz):
        if want and n not in want:
            continue
        if not re.match(r"^func_[0-9A-Fa-f]{8}$", n) or n not in osyms:
            continue
        k = sz[n]
        a, b = retail(int(n[5:], 16), k), ours(n, k)
        if b is None or len(b) != k or a == b:
            continue
        diffs = [i for i in range(0, k, 4) if a[i:i + 4] != b[i:i + 4]]
        print("%s  %d/%d words differ  (size %#x)" % (n, len(diffs), k // 4, k))
        for i in diffs:
            wa = int.from_bytes(a[i:i + 4], "little")
            wb = int.from_bytes(b[i:i + 4], "little")
            print("    +%#05x  retail %-30s ours %s" % (i, decode(wa), decode(wb)))
        print()


if __name__ == "__main__":
    main()

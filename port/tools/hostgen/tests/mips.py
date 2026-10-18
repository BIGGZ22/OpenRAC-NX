# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""A small R5900 assembler for hostgen's tests: it writes assembly files the way the
decompilations' setup does (splat/spimdisasm: an address and the instruction's word in a
comment before each instruction), from test programs written here. Only what the tests use.

    text = assemble("func_00100000", 0x00100000, '''
        addiu $2, $0, 5
    loop:
        bne $4, $0, loop
        addiu $4, $4, -1
        jr $31
        nop
    ''')
"""

from __future__ import annotations

import re

SPECIAL = {"sll": 0x00, "srl": 0x02, "sra": 0x03, "jr": 0x08, "jalr": 0x09, "movz": 0x0A, "movn": 0x0B,
           "syscall": 0x0C, "break": 0x0D, "mfhi": 0x10, "mflo": 0x12, "mult": 0x18, "div": 0x1A, "addu": 0x21,
           "subu": 0x23, "and": 0x24, "or": 0x25, "xor": 0x26, "nor": 0x27, "slt": 0x2A, "sltu": 0x2B,
           "daddu": 0x2D, "dsubu": 0x2F, "dsll": 0x38, "dsrl": 0x3A, "dsra": 0x3B, "dsll32": 0x3C,
           "dsrl32": 0x3E, "dsra32": 0x3F}
IMM = {"addi": 0x08, "addiu": 0x09, "slti": 0x0A, "sltiu": 0x0B, "andi": 0x0C, "ori": 0x0D, "xori": 0x0E,
       "daddiu": 0x19}
MEM = {"lb": 0x20, "lh": 0x21, "lwl": 0x22, "lw": 0x23, "lbu": 0x24, "lhu": 0x25, "lwr": 0x26, "lwu": 0x27,
       "sb": 0x28, "sh": 0x29, "swl": 0x2A, "sw": 0x2B, "swr": 0x2E, "lq": 0x1E, "sq": 0x1F, "ld": 0x37,
       "sd": 0x3F, "lwc1": 0x31, "swc1": 0x39, "lqc2": 0x36, "sqc2": 0x3E}
BRANCH2 = {"beq": 0x04, "bne": 0x05, "beql": 0x14, "bnel": 0x15}
BRANCH1 = {"blez": (0x06, 0), "bgtz": (0x07, 0), "blezl": (0x16, 0), "bgtzl": (0x17, 0),
           "bltz": (0x01, 0), "bgez": (0x01, 1), "bltzl": (0x01, 2), "bgezl": (0x01, 3)}
FPU3 = {"add.s": 0x00, "sub.s": 0x01, "mul.s": 0x02, "div.s": 0x03, "max.s": 0x28, "min.s": 0x29}
FPU2 = {"abs.s": 0x05, "mov.s": 0x06, "neg.s": 0x07, "cvt.w.s": 0x24}
FCMP = {"c.eq.s": 0x32, "c.lt.s": 0x34, "c.le.s": 0x36}
MMI = {"pextlw": (0x08, 0x12), "pcpyld": (0x09, 0x0E), "pcpyud": (0x29, 0x0E), "paddw": (0x08, 0x00),
       "por": (0x29, 0x12), "pand": (0x09, 0x12)}
VU_BC = {"vaddbc": 0x00, "vsubbc": 0x04, "vmulbc": 0x18}
VU_3 = {"vadd": 0x28, "vmul": 0x2A, "vsub": 0x2C}


def reg(text: str) -> int:
    text = text.strip()
    m = re.fullmatch(r"\$(?:f|vf)?(\d+)", text)
    if not m:
        raise ValueError(f"not a register: {text}")
    return int(m.group(1))


def mem(text: str) -> tuple[int, int]:
    m = re.fullmatch(r"\s*(-?(?:0x[0-9A-Fa-f]+|\d+)|%lo\([^)]*\))\((\$\d+)\)\s*", text)
    off = m.group(1)
    return (0 if off.startswith("%") else int(off, 0)), reg(m.group(2))


def dest(text: str) -> int:
    return sum({"x": 8, "y": 4, "z": 2, "w": 1}[c] for c in text)


def encode(op: str, args: list[str], addr: int, labels: dict[str, int], symbols: dict[str, int]) -> int:
    def target(name: str) -> int:
        return labels[name] if name in labels else symbols[name]

    def rel(name: str) -> int:
        return ((target(name) - (addr + 4)) >> 2) & 0xFFFF

    base, _, suffix = op.partition(".")
    if op == "nop":
        return 0
    if op in ("sll", "srl", "sra", "dsll", "dsrl", "dsra", "dsll32", "dsrl32", "dsra32"):
        return reg(args[1]) << 16 | reg(args[0]) << 11 | int(args[2], 0) << 6 | SPECIAL[op]
    if op == "jr":
        return reg(args[0]) << 21 | SPECIAL[op]
    if op == "jalr":
        return reg(args[1]) << 21 | reg(args[0]) << 11 | SPECIAL[op]
    if op in ("mfhi", "mflo"):
        return reg(args[0]) << 11 | SPECIAL[op]
    if op in ("mult", "div") and len(args) == 2:
        return reg(args[0]) << 21 | reg(args[1]) << 16 | SPECIAL[op]
    if op == "syscall":
        return SPECIAL[op]
    if op in SPECIAL:
        return reg(args[1]) << 21 | reg(args[2]) << 16 | reg(args[0]) << 11 | SPECIAL[op]
    if op in IMM:
        m = re.fullmatch(r"%lo\(([\w.]+)\)", args[2].strip())
        value = symbols[m.group(1)] if m else int(args[2], 0)
        return IMM[op] << 26 | reg(args[1]) << 21 | reg(args[0]) << 16 | (value & 0xFFFF)
    if op == "lui":
        m = re.fullmatch(r"%hi\(([\w.]+)\)", args[1].strip())
        value = (symbols[m.group(1)] + 0x8000) >> 16 if m else int(args[1], 0)
        return 0x0F << 26 | reg(args[0]) << 16 | (value & 0xFFFF)
    if op in MEM:
        off, rs = mem(args[1])
        m = re.match(r"\s*%lo\(([\w.]+)\)", args[1])
        if m:
            off = symbols[m.group(1)] & 0xFFFF
        return MEM[op] << 26 | rs << 21 | reg(args[0]) << 16 | (off & 0xFFFF)
    if op in BRANCH2:
        return BRANCH2[op] << 26 | reg(args[0]) << 21 | reg(args[1]) << 16 | rel(args[2])
    if op in BRANCH1:
        o, rt = BRANCH1[op]
        return o << 26 | reg(args[0]) << 21 | rt << 16 | rel(args[1])
    if op in ("j", "jal"):
        return (2 if op == "j" else 3) << 26 | ((target(args[0]) >> 2) & 0x3FFFFFF)
    if op in ("bc1f", "bc1t", "bc1fl", "bc1tl"):
        return 0x11 << 26 | 8 << 21 | ["bc1f", "bc1t", "bc1fl", "bc1tl"].index(op) << 16 | rel(args[0])
    if op in FPU3:
        return 0x11 << 26 | 0x10 << 21 | reg(args[2]) << 16 | reg(args[1]) << 11 | reg(args[0]) << 6 | FPU3[op]
    if op in FPU2:
        return 0x11 << 26 | 0x10 << 21 | reg(args[1]) << 11 | reg(args[0]) << 6 | FPU2[op]
    if op == "cvt.s.w":
        return 0x11 << 26 | 0x14 << 21 | reg(args[1]) << 11 | reg(args[0]) << 6 | 0x20
    if op in FCMP:
        return 0x11 << 26 | 0x10 << 21 | reg(args[1]) << 16 | reg(args[0]) << 11 | FCMP[op]
    if op in ("mtc1", "mfc1"):
        return 0x11 << 26 | (4 if op == "mtc1" else 0) << 21 | reg(args[0]) << 16 | reg(args[1]) << 11
    if op in MMI:
        funct, sa = MMI[op]
        return 0x1C << 26 | reg(args[1]) << 21 | reg(args[2]) << 16 | reg(args[0]) << 11 | sa << 6 | funct
    if op in ("qmfc2", "qmtc2"):
        return 0x12 << 26 | (1 if op == "qmfc2" else 5) << 21 | reg(args[0]) << 16 | reg(args[1]) << 11
    if base in VU_BC or base in VU_3:
        # vaddbc.xyzw $vf1, $vf2, $vf3x / vadd.xyz $vf1, $vf2, $vf3
        d = dest(suffix)
        if base in VU_BC:
            ft, bc = args[2].strip()[:-1], "xyzw".index(args[2].strip()[-1])
            funct = VU_BC[base] + bc
        else:
            ft, funct = args[2], VU_3[base]
        return 0x12 << 26 | 1 << 25 | d << 21 | reg(ft) << 16 | reg(args[1]) << 11 | reg(args[0]) << 6 | funct
    raise ValueError(f"the test assembler has no {op}")


def assemble(name: str, start: int, source: str, symbols: dict[str, int] | None = None,
             tables: dict[str, list[str]] | None = None) -> str:
    """A spimdisasm-style file for one function. Lines are instructions or `label:`; jal and j
    take a label or a name in symbols; %hi(name) / %lo(name) take a name in symbols, written
    into the instruction text as the setup writes them. tables: jump tables written after the
    function (`dlabel jtbl_...` and `.word .Lxxxxxxxx`), each a list of labels."""
    symbols = dict(symbols or {})
    lines = [l.split("#")[0].strip() for l in source.strip().splitlines()]
    lines = [l for l in lines if l]
    labels: dict[str, int] = {}
    addr = start
    for l in lines:
        if l.endswith(":"):
            labels[l[:-1]] = addr
        else:
            addr += 4
    out = [".set noat", ".align 3", f"glabel {name}"]
    addr = start
    for l in lines:
        if l.endswith(":"):
            out.append(f"  .L{labels[l[:-1]]:08X}:")
            continue
        op, _, rest = l.partition(" ")
        args = [a.strip() for a in re.split(r",(?![^(]*\))", rest)] if rest.strip() else []
        word = encode(op, args, addr, labels, symbols)
        text = rest
        for lab, at in labels.items():
            text = re.sub(r"\b" + re.escape(lab) + r"\b", f".L{at:08X}", text)
        out.append(f"    /* {addr - start:06X} {addr:08X} {word.to_bytes(4, 'little').hex().upper()} */  {op:<10} {text}")
        addr += 4
    out.append(f"endlabel {name}")
    for tname, entries in (tables or {}).items():
        out += [".section .rodata", f"dlabel {tname}"]
        at = symbols[tname]
        for e in entries:
            out.append(f"    /* {at:06X} {at:08X} 00000000 */ .word .L{labels[e]:08X}")
            at += 4
        out += [f"enddlabel {tname}", ".section .text"]
    return "\n".join(out) + "\n"


def label_addresses(start: int, source: str) -> dict[str, int]:
    labels = {}
    addr = start
    for l in (x.split("#")[0].strip() for x in source.strip().splitlines()):
        if not l:
            continue
        if l.endswith(":"):
            labels[l[:-1]] = addr
        else:
            addr += 4
    return labels

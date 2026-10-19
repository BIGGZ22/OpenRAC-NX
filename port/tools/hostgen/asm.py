# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""A decompilation's assembly, read: every function's instructions by address.

The decompilations keep the functions they have no C for as assembly files that
each game's setup writes from the player's own executable (splat and
spimdisasm: rac1/pal's asm/, the NTSC decomp's config/us/...). None of it is in
git, so this is optional: where the files are not there, nothing is read and
hostgen keeps its stubs.

Each instruction line carries its address and its word,

    /* 11EA18 0021DA98 1400033C */  lui        $3, %hi(D_0013D5CA)

and the word is what is decoded (decode): the text only says where functions
and labels start (glabel, alabel, jlabel, .Lxxxxxxxx:), and which immediates are
halves of a symbol's address (%hi / %lo), which the translation relocates like
hostgen's C does its globals. Jump tables written beside a function (dlabel
jtbl_..., .word .Lxxxxxxxx) are read too.
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field
from pathlib import Path

_INSN = re.compile(r"^\s*/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s*\*/\s*(\S+)\s*(.*)$")
_LABEL = re.compile(r"^\s*(?:glabel|alabel|jlabel|dlabel)\s+(\S+)")
_LOCAL = re.compile(r"^\s*(\.L[0-9A-Fa-f_]+):")
_END = re.compile(r"^\s*(?:endlabel|enddlabel)\s+(\S+)")
_SECTION = re.compile(r"^\s*\.section\s+(\S+)")
_RELOC = re.compile(r"%(hi|lo)\(([A-Za-z_.$][\w.$]*)\s*(?:\+\s*(0x[0-9A-Fa-f]+|\d+))?\)")


@dataclass
class Line:
    """One instruction as the file has it."""
    addr: int
    word: int
    text: str                    # mnemonic and operands as printed
    reloc: tuple | None = None   # ("hi" | "lo", symbol, addend)


@dataclass
class Block:
    """A function as the file has it: from its glabel to its endlabel."""
    name: str
    path: Path
    lines: list[Line] = field(default_factory=list)
    labels: dict[int, str] = field(default_factory=dict)      # address -> name
    entries: dict[str, int] = field(default_factory=dict)     # glabel/alabel name -> address
    tables: dict[str, list[int]] = field(default_factory=dict)  # jump table name -> targets
    jlabels: set[int] = field(default_factory=set)  # addresses marked as jump table targets

    def index_of(self, addr: int) -> int | None:
        if not self.lines:
            return None
        i = (addr - self.lines[0].addr) // 4
        if 0 <= i < len(self.lines) and self.lines[i].addr == addr:
            return i
        return None


def word_of(text: str) -> int:
    """The instruction word from the file's bytes (little-endian, as in memory)."""
    return int.from_bytes(bytes.fromhex(text), "little")


def parse_file(path: Path) -> list[Block]:
    blocks: list[Block] = []
    cur: Block | None = None
    section = ".text"
    table: list[int] | None = None
    pending: list[str] = []          # labels waiting for the next instruction
    jpending: set[str] = set()       # those of them that are jump table targets (jlabel)
    label_addr: dict[str, int] = {}
    try:
        text = path.read_text(errors="replace")
    except OSError:
        return []
    table_refs: list[tuple[Block, str, list[str]]] = []
    for raw in text.splitlines():
        m = _SECTION.match(raw)
        if m:
            section = m.group(1)
            table = None
            continue
        m = _END.match(raw)
        if m:
            if cur is not None and m.group(1) == cur.name:
                cur = None
            table = None
            continue
        m = _LABEL.match(raw)
        if m:
            name = m.group(1)
            kind = raw.split()[0]
            # A function spimdisasm took for data (its words written as .word under a dlabel, in
            # .text, in the file named for the function): code all the same, as the catalogue says.
            code_dlabel = (kind == "dlabel" and section.startswith(".text") and name == path.stem
                           and re.match(r"(?:func|FUN)_", name) is not None)
            if section.startswith(".text") and (kind == "glabel" or code_dlabel):
                cur = Block(name, path)
                blocks.append(cur)
                pending = [name]
            elif kind == "dlabel" or not section.startswith(".text"):
                if name.startswith("jtbl") or kind == "dlabel":
                    table = []
                    owner = cur if cur is not None else (blocks[-1] if blocks else None)
                    if owner is not None:
                        table_refs.append((owner, name, []))
            else:
                pending.append(name)
                if kind == "jlabel":
                    jpending.add(name)
            continue
        m = _LOCAL.match(raw)
        if m and section.startswith(".text"):
            pending.append(m.group(1))
            continue
        m = _INSN.match(raw)
        if not m:
            continue
        addr, word, op, ops = int(m.group(1), 16), word_of(m.group(2)), m.group(3), m.group(4)
        # A .word inside a function in .text is an instruction spimdisasm wrote as its word (a
        # branch to another file's label); elsewhere it is data, perhaps a jump table's entry.
        in_code = section.startswith(".text") and cur is not None and table is None
        if not in_code or (op == ".word" and not ops.strip().startswith("0x")):
            if table is not None and table_refs and op == ".word":
                table_refs[-1][2].append(ops.split("/*")[0].strip())
            continue
        if cur is None:
            continue
        ops = ops.split("/*")[0].strip()
        reloc = None
        r = _RELOC.search(ops)
        if r:
            reloc = (r.group(1), r.group(2), int(r.group(3), 0) if r.group(3) else 0)
        cur.lines.append(Line(addr, word, f"{op} {ops}".strip(), reloc))
        for name in pending:
            label_addr[name] = addr
            if name in jpending:
                cur.jlabels.add(addr)
            if name.startswith(".L"):
                cur.labels[addr] = name
            else:
                cur.entries[name] = addr
        pending = []
    for owner, name, words in table_refs:
        targets = [label_addr[w] for w in words if w in label_addr]
        if targets:
            owner.tables[name] = targets
    return [b for b in blocks if b.lines]


@dataclass
class Index:
    """Every instruction the assembly has, by program and address, and every function's entry.

    A program is the executable (-1) or a level's (its number): the levels' programs share
    addresses. The code of one program is one map of addresses, whatever files it came in, so
    a function that branches into code its neighbour's file has (splat split a routine in two)
    still has all of it."""
    blocks: list[Block] = field(default_factory=list)
    by_name: dict[str, tuple[int, int]] = field(default_factory=dict)     # name -> (program, address)
    by_place: dict[tuple[int, int], str] = field(default_factory=dict)    # (program, address) -> name
    code: dict[tuple[int, int], tuple[Line, Block]] = field(default_factory=dict)
    called: set[tuple[int, int]] = field(default_factory=set)   # (program, address) a jal or j goes to

    def find(self, names: list[str], places: list[tuple[int, int]], own: bool = False) -> tuple[int, int] | None:
        """Where a function starts, (program, address): by one of its names, else by its place.
        `own`: the place is the decompilation's own for the function (its INCLUDE_ASM stub), so
        code there with no label is its code (read from the retail program)."""
        for n in names:
            if n in self.by_name:
                return self.by_name[n]
        for p in places:
            if p in self.by_place:
                return p
        # A second entry the files do not label: an address inside another function's code that
        # the decompilation calls by a name of its own, and the program's code calls or jumps to
        # (the EE runs the code from there). A name whose address only another version's
        # numbering gives is not one.
        for p in places:
            if (own or p in self.called) and self.code.get(p) is not None:
                return p
        return None


def read_index(dirs: list[Path], overlay_of) -> Index:
    """Reads every .s file under dirs (missing ones are skipped). overlay_of(name) gives the
    program a function name belongs to (-1 for the executable, a level's number)."""
    idx = Index()
    for d in dirs:
        if not d.is_dir():
            continue
        for path in sorted(d.rglob("*.s")):
            for b in parse_file(path):
                idx.blocks.append(b)
                ov = overlay_of(b.name)
                for line in b.lines:
                    idx.code.setdefault((ov, line.addr), (line, b))
                    if line.word >> 26 in (2, 3):
                        idx.called.add((ov, (line.addr & 0xF0000000) | ((line.word & 0x3FFFFFF) << 2)))
                for name, addr in b.entries.items():
                    if b.index_of(addr) is None:
                        continue
                    idx.by_name.setdefault(name, (ov, addr))
                    idx.by_place.setdefault((ov, addr), name)
    return idx


# ---- Decoding ----------------------------------------------------------------

@dataclass
class Insn:
    op: str          # the operation, by its R5900 name (dots for FPU formats: add.s)
    addr: int
    word: int
    rs: int = 0
    rt: int = 0
    rd: int = 0
    sa: int = 0
    imm: int = 0     # sign-extended 16-bit immediate (zero-extended for the logical ones)
    target: int = 0  # j / jal / branch target address
    fs: int = 0
    ft: int = 0
    fd: int = 0
    dest: int = 0    # VU field mask: x 8, y 4, z 2, w 1
    bc: int = 0      # VU broadcast field, or the fsf/ftf of div/sqrt
    ftf: int = 0
    reloc: tuple | None = None

    @property
    def is_branch(self) -> bool:
        return self.op in BRANCHES

    @property
    def likely(self) -> bool:
        return self.op in LIKELY


BRANCHES = {"beq", "bne", "blez", "bgtz", "bltz", "bgez", "beql", "bnel", "blezl", "bgtzl", "bltzl", "bgezl",
            "bltzal", "bgezal", "bltzall", "bgezall", "bc1f", "bc1t", "bc1fl", "bc1tl", "bc0f", "bc0t",
            "bc0fl", "bc0tl", "bc2f", "bc2t", "bc2fl", "bc2tl", "j", "jal", "jr", "jalr"}
LIKELY = {"beql", "bnel", "blezl", "bgtzl", "bltzl", "bgezl", "bltzall", "bgezall", "bc1fl", "bc1tl",
          "bc0fl", "bc0tl", "bc2fl", "bc2tl"}

_SPECIAL = {
    0x00: "sll", 0x02: "srl", 0x03: "sra", 0x04: "sllv", 0x06: "srlv", 0x07: "srav", 0x08: "jr", 0x09: "jalr",
    0x0A: "movz", 0x0B: "movn", 0x0C: "syscall", 0x0D: "break", 0x0F: "sync", 0x10: "mfhi", 0x11: "mthi",
    0x12: "mflo", 0x13: "mtlo", 0x14: "dsllv", 0x16: "dsrlv", 0x17: "dsrav", 0x18: "mult", 0x19: "multu",
    0x1A: "div", 0x1B: "divu", 0x20: "add", 0x21: "addu", 0x22: "sub", 0x23: "subu", 0x24: "and", 0x25: "or",
    0x26: "xor", 0x27: "nor", 0x28: "mfsa", 0x29: "mtsa", 0x2A: "slt", 0x2B: "sltu", 0x2C: "dadd",
    0x2D: "daddu", 0x2E: "dsub", 0x2F: "dsubu", 0x30: "tge", 0x31: "tgeu", 0x32: "tlt", 0x33: "tltu",
    0x34: "teq", 0x36: "tne", 0x38: "dsll", 0x3A: "dsrl", 0x3B: "dsra", 0x3C: "dsll32", 0x3E: "dsrl32",
    0x3F: "dsra32",
}
_REGIMM = {0x00: "bltz", 0x01: "bgez", 0x02: "bltzl", 0x03: "bgezl", 0x08: "tgei", 0x09: "tgeiu", 0x0A: "tlti",
           0x0B: "tltiu", 0x0C: "teqi", 0x0E: "tnei", 0x10: "bltzal", 0x11: "bgezal", 0x12: "bltzall",
           0x13: "bgezall", 0x18: "mtsab", 0x19: "mtsah"}
_PRIMARY = {
    0x02: "j", 0x03: "jal", 0x04: "beq", 0x05: "bne", 0x06: "blez", 0x07: "bgtz", 0x08: "addi", 0x09: "addiu",
    0x0A: "slti", 0x0B: "sltiu", 0x0C: "andi", 0x0D: "ori", 0x0E: "xori", 0x0F: "lui", 0x14: "beql",
    0x15: "bnel", 0x16: "blezl", 0x17: "bgtzl", 0x18: "daddi", 0x19: "daddiu", 0x1A: "ldl", 0x1B: "ldr",
    0x1E: "lq", 0x1F: "sq", 0x20: "lb", 0x21: "lh", 0x22: "lwl", 0x23: "lw", 0x24: "lbu", 0x25: "lhu",
    0x26: "lwr", 0x27: "lwu", 0x28: "sb", 0x29: "sh", 0x2A: "swl", 0x2B: "sw", 0x2C: "sdl", 0x2D: "sdr",
    0x2E: "swr", 0x2F: "cache", 0x31: "lwc1", 0x33: "pref", 0x36: "lqc2", 0x37: "ld", 0x39: "swc1",
    0x3E: "sqc2", 0x3F: "sd",
}
_COP1_S = {
    0x00: "add.s", 0x01: "sub.s", 0x02: "mul.s", 0x03: "div.s", 0x04: "sqrt.s", 0x05: "abs.s", 0x06: "mov.s",
    0x07: "neg.s", 0x16: "rsqrt.s", 0x18: "adda.s", 0x19: "suba.s", 0x1A: "mula.s", 0x1C: "madd.s",
    0x1D: "msub.s", 0x1E: "madda.s", 0x1F: "msuba.s", 0x24: "cvt.w.s", 0x28: "max.s", 0x29: "min.s",
    0x30: "c.f.s", 0x32: "c.eq.s", 0x34: "c.lt.s", 0x36: "c.le.s",
}
_MMI = {0x00: "madd", 0x01: "maddu", 0x04: "plzcw", 0x10: "mfhi1", 0x11: "mthi1", 0x12: "mflo1", 0x13: "mtlo1",
        0x18: "mult1", 0x19: "multu1", 0x1A: "div1", 0x1B: "divu1", 0x20: "madd1", 0x21: "maddu1",
        0x30: "pmfhl", 0x31: "pmthl", 0x34: "psllh", 0x36: "psrlh", 0x37: "psrah", 0x3C: "psllw",
        0x3E: "psrlw", 0x3F: "psraw"}
_MMI0 = {0x00: "paddw", 0x01: "psubw", 0x02: "pcgtw", 0x03: "pmaxw", 0x04: "paddh", 0x05: "psubh", 0x06: "pcgth",
         0x07: "pmaxh", 0x08: "paddb", 0x09: "psubb", 0x0A: "pcgtb", 0x10: "paddsw", 0x11: "psubsw",
         0x12: "pextlw", 0x13: "ppacw", 0x14: "paddsh", 0x15: "psubsh", 0x16: "pextlh", 0x17: "ppach",
         0x18: "paddsb", 0x19: "psubsb", 0x1A: "pextlb", 0x1B: "ppacb", 0x1E: "pext5", 0x1F: "ppac5"}
_MMI1 = {0x01: "pabsw", 0x02: "pceqw", 0x03: "pminw", 0x04: "padsbh", 0x05: "pabsh", 0x06: "pceqh", 0x07: "pminh",
         0x0A: "pceqb", 0x10: "padduw", 0x11: "psubuw", 0x12: "pextuw", 0x14: "padduh", 0x15: "psubuh",
         0x16: "pextuh", 0x18: "paddub", 0x19: "psubub", 0x1A: "pextub", 0x1B: "qfsrv"}
_MMI2 = {0x00: "pmaddw", 0x02: "psllvw", 0x03: "psrlvw", 0x04: "pmsubw", 0x08: "pmfhi", 0x09: "pmflo",
         0x0A: "pinth", 0x0C: "pmultw", 0x0D: "pdivw", 0x0E: "pcpyld", 0x10: "pmaddh", 0x11: "phmadh",
         0x12: "pand", 0x13: "pxor", 0x14: "pmsubh", 0x15: "phmsbh", 0x1A: "pexeh", 0x1B: "prevh",
         0x1C: "pmulth", 0x1D: "pdivbw", 0x1E: "pexew", 0x1F: "prot3w"}
_MMI3 = {0x00: "pmadduw", 0x03: "psravw", 0x08: "pmthi", 0x09: "pmtlo", 0x0A: "pinteh", 0x0C: "pmultuw",
         0x0D: "pdivuw", 0x0E: "pcpyud", 0x12: "por", 0x13: "pnor", 0x1A: "pexch", 0x1B: "pcpyh", 0x1E: "pexcw"}
# VU0 macro instructions (COP2 with the CO bit): the first table by funct, the second by
# (fd << 2 | funct & 3) when funct is 0x3C..0x3F.
_BC = "xyzw"
_VU1 = {}
for _i, _base in enumerate(["vadd", "vsub", "vmadd", "vmsub", "vmax", "vmini", "vmul"]):
    for _b in range(4):
        _VU1[_i * 4 + _b] = f"{_base}bc"
_VU1.update({0x1C: "vmulq", 0x1D: "vmaxi", 0x1E: "vmuli", 0x1F: "vminii", 0x20: "vaddq", 0x21: "vmaddq",
             0x22: "vaddi", 0x23: "vmaddi", 0x24: "vsubq", 0x25: "vmsubq", 0x26: "vsubi", 0x27: "vmsubi",
             0x28: "vadd", 0x29: "vmadd", 0x2A: "vmul", 0x2B: "vmax", 0x2C: "vsub", 0x2D: "vmsub",
             0x2E: "vopmsub", 0x2F: "vmini", 0x30: "viadd", 0x31: "visub", 0x32: "viaddi", 0x34: "viand",
             0x35: "vior", 0x38: "vcallms", 0x39: "vcallmsr"})
_VU2 = {}
for _i, _base in enumerate(["vaddabc", "vsubabc", "vmaddabc", "vmsubabc"]):
    for _b in range(4):
        _VU2[_i * 4 + _b] = _base
for _b in range(4):
    _VU2[0x18 + _b] = "vmulabc"
_VU2.update({0x10: "vitof0", 0x11: "vitof4", 0x12: "vitof12", 0x13: "vitof15", 0x14: "vftoi0", 0x15: "vftoi4",
             0x16: "vftoi12", 0x17: "vftoi15", 0x1C: "vmulaq", 0x1D: "vabs", 0x1E: "vmulai", 0x1F: "vclipw",
             0x20: "vaddaq", 0x21: "vmaddaq", 0x22: "vaddai", 0x23: "vmaddai", 0x24: "vsubaq", 0x25: "vmsubaq",
             0x26: "vsubai", 0x27: "vmsubai", 0x28: "vadda", 0x29: "vmadda", 0x2A: "vmula", 0x2C: "vsuba",
             0x2D: "vmsuba", 0x2E: "vopmula", 0x2F: "vnop", 0x30: "vmove", 0x31: "vmr32", 0x34: "vlqi",
             0x35: "vsqi", 0x36: "vlqd", 0x37: "vsqd", 0x38: "vdiv", 0x39: "vsqrt", 0x3A: "vrsqrt",
             0x3B: "vwaitq", 0x3C: "vmtir", 0x3D: "vmfir", 0x3E: "vilwr", 0x3F: "viswr", 0x40: "vrnext",
             0x41: "vrget", 0x42: "vrinit", 0x43: "vrxor"})


def _sext16(v: int) -> int:
    return v - 0x10000 if v & 0x8000 else v


def decode(word: int, addr: int, reloc=None) -> Insn:
    """One R5900 instruction. An encoding this does not know is op "?"."""
    op = word >> 26
    rs, rt, rd, sa, funct = (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31, (word >> 6) & 31, word & 63
    imm = _sext16(word & 0xFFFF)
    i = Insn("?", addr, word, rs=rs, rt=rt, rd=rd, sa=sa, imm=imm, reloc=reloc)
    branch_target = (addr + 4 + (imm << 2)) & 0xFFFFFFFF
    if op == 0:
        i.op = _SPECIAL.get(funct, "?")
        if word == 0:
            i.op = "nop"
    elif op == 1:
        i.op = _REGIMM.get(rt, "?")
        i.target = branch_target
    elif op in (2, 3):
        i.op = _PRIMARY[op]
        i.target = ((addr + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2)
    elif op == 0x10:
        if rs == 0:
            i.op = "mfc0"
        elif rs == 4:
            i.op = "mtc0"
        elif rs == 8:
            i.op = ["bc0f", "bc0t", "bc0fl", "bc0tl"][rt & 3]
            i.target = branch_target
        elif rs == 0x10 and funct == 0x18:
            i.op = "eret"
        elif rs == 0x10 and funct == 0x38:
            i.op = "ei"
        elif rs == 0x10 and funct == 0x39:
            i.op = "di"
        else:
            i.op = "cop0"
    elif op == 0x11:
        i.ft, i.fs, i.fd = rt, rd, sa
        if rs == 0:
            i.op = "mfc1"
        elif rs == 2:
            i.op = "cfc1"
        elif rs == 4:
            i.op = "mtc1"
        elif rs == 6:
            i.op = "ctc1"
        elif rs == 8:
            i.op = ["bc1f", "bc1t", "bc1fl", "bc1tl"][rt & 3]
            i.target = branch_target
        elif rs == 0x10:
            i.op = _COP1_S.get(funct, "?")
        elif rs == 0x14 and funct == 0x20:
            i.op = "cvt.s.w"
    elif op == 0x12:
        i.ft, i.fs, i.fd = rt, rd, sa
        i.dest = (word >> 21) & 15
        if rs & 0x10:
            if funct >= 0x3C:
                i.op = _VU2.get((sa << 2) | (funct & 3), "?")
            else:
                i.op = _VU1.get(funct, "?")
            i.bc = funct & 3
            if i.op in ("vdiv", "vsqrt", "vrsqrt", "vmtir"):
                i.bc = (word >> 21) & 3          # fsf
                i.ftf = (word >> 23) & 3
            if i.op in ("viaddi",):
                i.imm = sa - 32 if sa & 0x10 else sa
        elif rs == 1:
            i.op = "qmfc2"
        elif rs == 2:
            i.op = "cfc2"
        elif rs == 5:
            i.op = "qmtc2"
        elif rs == 6:
            i.op = "ctc2"
        elif rs == 8:
            i.op = ["bc2f", "bc2t", "bc2fl", "bc2tl"][rt & 3]
            i.target = branch_target
    elif op == 0x1C:
        if funct == 0x08:
            i.op = _MMI0.get(sa, "?")
        elif funct == 0x28:
            i.op = _MMI1.get(sa, "?")
        elif funct == 0x09:
            i.op = _MMI2.get(sa, "?")
        elif funct == 0x29:
            i.op = _MMI3.get(sa, "?")
        else:
            i.op = _MMI.get(funct, "?")
        if i.op == "pmfhl":
            i.op = ["pmfhl.lw", "pmfhl.uw", "pmfhl.slw", "pmfhl.lh", "pmfhl.sh"][sa] if sa < 5 else "?"
    else:
        i.op = _PRIMARY.get(op, "?")
        if i.op in ("andi", "ori", "xori"):
            i.imm = word & 0xFFFF
        if i.op in ("beq", "bne", "blez", "bgtz", "beql", "bnel", "blezl", "bgtzl"):
            i.target = branch_target
        if i.op in ("lwc1", "swc1"):
            i.ft = rt
        if i.op in ("lqc2", "sqc2"):
            i.ft = rt
    if i.op in ("bc1f", "bc1t", "bc1fl", "bc1tl", "bc0f", "bc0t", "bc0fl", "bc0tl", "bc2f", "bc2t", "bc2fl", "bc2tl"):
        i.target = branch_target
    return i

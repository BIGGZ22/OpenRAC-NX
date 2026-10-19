#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Checks the port's own C for a version (port/game/<id>/hand, hand.tsv's rows)
against that version's retail code: every function the C calls and every
global address it reaches should be one the retail function's instructions
reach, and the other way round.

  xcheck.py report GAME_DIR --gen GEN [--asm-source DIR] [-v] [--all] [NAME ...]
  xcheck.py check  GAME_DIR --gen GEN [--asm-source DIR]

`report` prints each row that differs (with NAME, only those rows; -v also
what each side reaches). `check` is the gate: it exits 1 on a difference
allow.tsv does not explain, or on a row of allow.tsv nothing matches any more,
and 77 (skipped) where there is no build or no retail program to read.

GAME_DIR is port/game/<id>: its hostgen.json names the decompilation, the
retail program, $gp, the levels' low end and hand.tsv. GEN is its generated
folder (port/build/<preset>/games/<id>/gen): functions.c there says which
function hostgen binds at each place of each program, so a call is compared
by the function it reaches, under whatever spelling. The retail words come
from the executable and the level dumps the decompilation's setup placed
(hostgen.json "asm" "retail", read by hostgen's retail.py); `--asm-source`
(or OPENRAC_<ID>_ASM_SOURCE, as for the build) names the checkout that holds
them when the decompilation does not (a worktree).

The C side is what Clang makes of each hand file, prepared as hostgen prepares
it (prep.py: the EE's 64-bit long first of all), as LLVM IR at -O1 for the
console's 32-bit little-endian MIPS: every address the C forms from a global
and a constant (a field of a struct laid over a global, a cast and an offset,
an array element) is folded into one constant there, and calls are the
functions' linker names (their `__asm__` labels). A function a hand file
defines that is no row of its own is a call where hostgen binds it at a place,
and is read through where it is no retail function and the code does not call
it (a wrapper of the port's own, func_001F9688 for FUN_001f96f8).

The code side follows which registers hold an address through the function's
blocks (what every way into a block agrees on; a moved pointer as a place in
its array), into a tail it jumps or falls into and past a catalogued end it
branches over.

What is compared, at the function's own place (the level its name says, the
level whose globals its C names, or the executable):

- calls: jal and j targets outside the function, and code addresses formed
  with a lui pair (callbacks); against the functions the C calls or takes the
  address of. A split_places.tsv variant counts as its function (hostgen takes
  the caller's own jal for it), and an inline trap (teq, a divide's break) as a
  call to the routine that only traps;
- globals: addresses formed with a lui pair or from $gp, and those of loads,
  stores and addiu through a register holding one; against the globals and
  constant offsets from them the C reaches, and address literals it casts to
  a pointer. Left out as the C may write them as literals: reads of the
  literal pools, read-only data and tables in the code. Taken as reached: a
  base the code forms only to reach what the C reaches through it; an element
  within 0x1000 bytes of an array the other side indexes; one inside an array,
  struct or block copy the C declares with its size; the words of a quadword
  either side moves whole. These are where the check is coarse: an element
  off inside an indexed array passes;
- every other place of the function (the levels' copies): call by call
  against the home copy; a copy that calls something else counts where
  hostgen would call the C's one name there (lower.py takes the copy's own jal
  where that name has no place in the level, is folded or split, or is a hook).

Rows hostgen does not place (hand.tsv rows the decompilation has its own C
for, by GEN/report.json) are listed and not compared.

allow.tsv beside hand.tsv lists what differs on purpose:
`row<TAB>kind<TAB>item<TAB>reason`, kind one of c-call, code-call, c-data,
code-data, level-call (item `Lnn:name`), no-code; item `*` for every item of
that kind; row `*` for every row (a helper the C writes inline everywhere).
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

HERE = Path(__file__).resolve().parent
HOSTGEN = HERE.parent / "hostgen"
sys.path.insert(0, str(HOSTGEN))

ARRAY = 0x1000          # how far past an indexed array's start its elements are taken to reach
DATA_TOP = 0x400000     # the games' globals sit below this; a value formed above it is a constant
KINDS = ("c-call", "code-call", "c-data", "code-data", "level-call", "no-code")
LOADS_STORES = {"lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "lq", "lwl", "lwr", "ldl", "ldr", "sb", "sh", "sw",
                "sd", "sq", "swl", "swr", "sdl", "sdr", "lwc1", "swc1", "lqc2", "sqc2"}
WIDTH = {"ld": 8, "sd": 8, "ldl": 8, "ldr": 8, "sdl": 8, "sdr": 8, "lq": 16, "sq": 16, "lqc2": 16, "sqc2": 16}
TRAPS = {"teq", "tne", "tge", "tgeu", "tlt", "tltu", "teqi", "tnei", "tgei", "tgeiu", "tlti", "tltiu"}   # not break: the compiler's divide checks
READ_ONLY = {"core.rdata", "core.lit", ".lit", ".rodata", ".rdata", "lit"}
ENTRY = re.compile(r'\{0x([0-9A-Fa-f]+)u, \(openrac_host_fn\)\w+, "(\w+)"\}')
TABLE = re.compile(r"static const openrac_fn_entry k_(exe|overlay_(\d\d))\[\]")


@dataclass
class Refs:
    calls: set = field(default_factory=set)       # function names (as functions.c binds them)
    data: set = field(default_factory=set)        # global addresses
    constants: set = field(default_factory=set)   # read-only data the code reads
    derived: dict = field(default_factory=dict)   # a base the code formed -> what it reached through it
    unknown: set = field(default_factory=set)     # the C's names that resolve to nothing
    host: set = field(default_factory=set)        # the port's own functions the C calls
    wide: set = field(default_factory=set)        # bytes the code moves as a doubleword or quadword
    sequence: list = field(default_factory=list)  # the code's calls in their order
    indexed: set = field(default_factory=set)     # arrays reached with an index (a loop, a variable)
    breaks: bool = False                          # the code can trap (a teq, a divide's break)
    where: dict = field(default_factory=dict)     # an address -> the instructions that reach it
    arrays: dict = field(default_factory=dict)    # a C array the function reaches -> its declared bytes
    helpers: dict = field(default_factory=dict)   # a hand-file function the C calls -> what it reaches

    def update(self, other: "Refs") -> None:
        for k in ("calls", "data", "constants", "unknown", "host", "indexed", "arrays", "helpers"):
            getattr(self, k).update(getattr(other, k))


# ---- the game: names, places, code ----------------------------------------------------------------

class Game:
    def __init__(self, game_dir: Path, gen: Path, asm_source: Path | None):
        self.dir = game_dir
        self.cfg = json.loads((game_dir / "hostgen.json").read_text())
        root = game_dir.resolve().parents[2]
        self.root = root
        self.source = (root / self.cfg["source"]).resolve()
        self.asm_source = asm_source.resolve() if asm_source else self.source
        asm_cfg = self.cfg.get("asm", {})
        self.gp = int(str(asm_cfg.get("gp", "0")), 0)
        self.low = self.cfg["relocate"][0]
        self.code_names = [re.compile(p) for p in self.cfg["names"]["code"]]
        self.data_names = [re.compile(p) for p in self.cfg["names"]["data"]]
        self.hand_table = game_dir / self.cfg["hand"]
        self._functions(gen / "functions.c")
        self._symbols()
        self._sizes()
        self._aliases()
        self.retail = None
        r = asm_cfg.get("retail", {})
        exe = self.asm_source / r.get("exe", "")
        dump = self.asm_source / r.get("dump", "") if r.get("dump") else None
        if r.get("exe") and exe.is_file():
            import retail
            self.retail = retail.Retail.from_files(exe, dump if dump and dump.is_dir() else None, self.low)

    def _functions(self, path: Path) -> None:
        self.table: dict[int, dict[int, str]] = {}      # program (-1 exe) -> address -> name
        self.places: dict[str, list] = {}              # name -> [(program, address)]
        prog = None
        for line in path.read_text().splitlines():
            m = TABLE.search(line)
            if m:
                prog = -1 if m.group(1) == "exe" else int(m.group(2))
                self.table[prog] = {}
                continue
            m = ENTRY.search(line)
            if m and prog is not None:
                a = int(m.group(1), 16)
                self.table[prog].setdefault(a, []).append(m.group(2))
                self.places.setdefault(m.group(2), []).append((prog, a))

    def _aliases(self) -> None:
        # Names that share a place are one function (the catalogue's FUN_001f9bf8 and the C's
        # func_L00_001FF500 at its level-0 place): each stands for one of them.
        parent: dict[str, str] = {}

        def find(x):
            while parent.get(x, x) != x:
                x = parent[x]
            return x

        def union(x, y):
            a, b = find(x), find(y)
            if a != b:
                keep, drop = sorted((a, b), key=lambda n: (not n.startswith("FUN_"), n))
                parent[drop] = keep
        for at in self.table.values():
            for names in at.values():
                for other in names[1:]:
                    union(names[0], other)
        # The catalogue's places of one function (a host function's name in the executable, the
        # decompilation's C at its level places) are that function too.
        split = set()
        split_pairs = []
        self.split_names = set()
        sp = self.dir / self.cfg.get("split_places", "split_places.tsv")
        if self.cfg.get("split_places") and sp.is_file():
            for line in sp.read_text().splitlines():
                f = line.split("\t")
                if not line.startswith("#") and len(f) >= 3:
                    self.split_names.update((f[0], f[1]))
                    split_pairs.append((f[1], f[0]))
                    split.update((int(x.split(":")[0]), int(x.split(":")[1], 16)) for x in f[2].split(",") if ":" in x)
        for name, places in self.catalogue.items():
            for prog, addr in places:
                if (prog, addr) in split:
                    continue    # split_places.tsv gives this place a function of its own
                for other in self.table.get(prog, {}).get(addr, []):
                    union(name, other)
        self.alias = {n: find(n) for n in self.places}
        self.split_names = {self.alias.get(n, n) for n in self.split_names}
        # A variant split_places.tsv makes is its function's for a call: hostgen takes the caller's
        # own jal for a call to the function (lower.py), which reaches the variant there.
        self.split_parent = {self.alias.get(v, v): self.alias.get(p, p) for v, p in split_pairs}
        self.group: dict[str, list] = {}               # a function -> every place of its names
        for n, pl in self.places.items():
            self.group.setdefault(self.alias[n], []).extend(pl)
        for n in self.group:
            self.group[n] = sorted(set(self.group[n]))

    def trap_functions(self) -> set:
        """The executable's functions that only raise a trap (teq $0, $0 and return)."""
        if getattr(self, "_traps", None) is None:
            import asm
            self._traps = set()
            for (prog, addr), size in self.sizes.items():
                if prog == -1 and size <= 16 and self.part(-1, addr) is not None:
                    p = self.part(-1, addr)
                    if p.code and p.holds(addr, size):
                        ops = [asm.decode(p.word(addr + 4 * k), addr + 4 * k).op for k in range(size // 4)]
                        if ops and ops[0] in TRAPS and "jr" in ops:
                            f = self.function_at(-1, addr)
                            if f:
                                self._traps.add(f)
        return self._traps

    def retail_start(self, name: str) -> bool:
        """Whether a name the hand files define is a function of the retail program (one the
        catalogue or the executable's function list has), not a wrapper of the port's own at an
        address no function starts at."""
        r = self.resolve(name)
        if r is None or r[0] != "call":
            return False
        return any(pl in self.sizes for pl in self.group.get(r[1], []))

    def per_copy(self, name: str, prog: int) -> bool:
        """Whether hostgen takes the calling copy's own jal for a C call to NAME made while PROG
        is loaded (lower.py): NAME has no place there (a variant of the level's own is called), or
        it is one the catalogue folded (two places in one program, a split_places.tsv row), or a
        level hook stub."""
        places = self.group.get(name, [])
        progs = [p for p, _ in places]
        if prog not in progs:
            return True
        if len(progs) != len(set(progs)) or name in self.split_names:
            return True
        return name in {self.alias.get(h, h) for h in self.cfg.get("level_hooks", [])}

    def _symbols(self) -> None:
        self.symbols: dict[str, int] = {}
        for rel in self.cfg.get("symbols", []):
            p = self.source / rel
            if p.is_file():
                for m in re.finditer(r"^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", p.read_text(), re.M):
                    self.symbols.setdefault(m.group(1), int(m.group(2), 16))

    def _sizes(self) -> None:
        self.sizes: dict[tuple[int, int], int] = {}
        self.catalogue: dict[str, list] = {}           # functions.tsv: name -> its places
        p = self.source / self.cfg.get("places", "")
        if p.is_file():
            for line in p.read_text().splitlines():
                f = line.split("\t")
                if line.startswith("#") or len(f) < 6 or not f[2].isdigit():
                    continue
                size = int(f[2])
                m = re.search(r"([0-9A-Fa-f]{8})$", f[0])
                if m and f[1] == "exe":
                    self.sizes[(-1, int(m.group(1), 16))] = size
                if m and f[1] == "exe":
                    self.catalogue.setdefault(f[0], []).append((-1, int(m.group(1), 16)))
                for pl in f[5].split(","):
                    if ":" in pl:
                        lv, a = pl.split(":")
                        self.sizes.setdefault((int(lv), int(a, 16)), size)
                        self.catalogue.setdefault(f[0], []).append((int(lv), int(a, 16)))
        g = self.source / "config" / "ghidra" / "ghidra_functions.csv"
        if g.is_file():
            for line in g.read_text().splitlines()[1:]:
                f = line.split(",")
                if len(f) >= 3 and f[1].startswith("0x"):
                    self.sizes.setdefault((-1, int(f[1], 16)), int(f[2], 16) - int(f[1], 16))

    # -- names

    def function_at(self, prog: int, addr: int) -> str | None:
        """The function functions.c binds at ADDR while PROG is loaded."""
        names = self.table.get(prog if prog >= 0 and addr >= self.low else -1, {}).get(addr)
        return self.alias[names[0]] if names else None

    def place_of_name(self, sym: str) -> tuple[int, int] | None:
        for pattern in self.code_names:
            m = pattern.match(sym)
            if m:
                ov = m.groupdict().get("overlay")
                return (int(ov) if ov is not None else -1, int(m.group("addr"), 16))
        return None

    def data_of_name(self, sym: str) -> int | None:
        if sym in self.symbols:
            return self.symbols[sym]
        for pattern in self.data_names + self.code_names:
            m = pattern.match(sym)
            if m:
                return int(m.group("addr"), 16)
        return None

    def resolve(self, sym: str):
        """('call', name) for a function, ('data', address) for a global, None for neither."""
        if sym in self.places:
            return ("call", self.alias[sym])
        pl = self.place_of_name(sym)
        if pl is not None:
            f = self.function_at(*pl)
            if f is None and pl[0] < 0:
                f = self.function_at(0, pl[1])
            if f is not None:
                return ("call", f)
        for pattern in self.data_names:
            m = pattern.match(sym)
            if m and m.group(0).startswith(("D_", "DAT_")):
                return ("data", int(m.group("addr"), 16))   # a global's name, whatever is there
        a = self.data_of_name(sym)
        if a is not None:
            # A named symbol (the decompilation's symbol tables): a function where one starts.
            f = self.table.get(-1, {}).get(a)
            return ("call", self.alias[f[0]]) if f else ("data", a)
        return None

    def resolve_row(self, name: str):
        """A hand.tsv name: as resolve, or through the decompilation's label for it."""
        r = self.resolve(name)
        if r is None:
            label = self.decomp_labels().get(name)
            if label and label != name:
                r = self.resolve(label)
        return r

    def decomp_labels(self) -> dict[str, str]:
        """The decompilation's names for functions it declares with a label (fast_vec_normalize is
        FUN_001f9bf8): how hand.tsv can name a row."""
        if getattr(self, "_labels", None) is None:
            self._labels = {}
            pat = re.compile(r"\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*__asm__\s*\(\s*\"(\w+)\"\s*\)")
            for path in sorted((self.source / "src").rglob("*.[ch]")):
                for m in pat.finditer(path.read_text(errors="replace")):
                    if any(p.match(m.group(2)) for p in self.code_names):
                        self._labels.setdefault(m.group(1), m.group(2))
        return self._labels

    def home(self, name: str) -> tuple[int, int] | None:
        """The place a row's function is checked at: the one its name says, else its first."""
        places = self.group.get(name, [])
        pl = self.place_of_name(name)
        if pl is not None:
            for p in places:
                if p == pl or (pl[0] < 0 and p[0] < 0 and p[1] == pl[1]):
                    return p
            if self.function_at(*pl) is not None:
                return pl
        if not places:
            return None
        exe = [p for p in places if p[0] < 0]
        return exe[0] if exe else sorted(places)[0]

    # -- code

    def size(self, prog: int, addr: int) -> int:
        """The catalogue's size (or the executable's function list's), cut at the next function
        hostgen knows (the decompilation splits what that list has as one); a routine that runs
        on past it is read on (code_refs)."""
        if not hasattr(self, "_starts"):
            self._starts = {p: sorted(t) for p, t in self.table.items()}
        import bisect
        starts = self._starts.get(prog if prog >= 0 and addr >= self.low else -1, [])
        i = bisect.bisect_right(starts, addr)
        nxt = starts[i] - addr if i < len(starts) else None
        s = self.sizes.get((prog, addr)) or (self.sizes.get((-1, addr)) if prog < 0 else None)
        if s:
            return min(s, nxt) if nxt else s
        return nxt or 4

    def part(self, prog: int, addr: int):
        if self.retail is None:
            return None
        for p in self.retail.program(prog):
            if p.holds(addr):
                return p
        return None

    def code_refs(self, prog: int, start: int, depth: int = 0) -> Refs | None:
        """What the retail function at START in PROG reaches.

        Which registers hold an address is followed through the function's branches (the values
        every way into a block agrees on), so a base kept in a saved register is followed to its
        fields and a value from another way is not taken for it; a call keeps only the registers
        the callee saves ($s0-$s7, $gp, $sp, $fp)."""
        import asm
        from asmc import rw
        part = self.part(prog, start)
        if part is None or not part.code:
            return None
        end = min(start + self.size(prog, start), part.base + len(part.data))
        n = (end - start) // 4
        insns = [asm.decode(part.word(start + 4 * k), start + 4 * k) for k in range(n)]
        writes = [rw(i)[1] for i in insns]
        # Blocks: a leader at the start, at each branch target inside, after each delay slot.
        leaders = {0}
        beyond = set()          # branch targets past the catalogued end (it cut the routine short)
        for k, ins in enumerate(insns):
            if ins.is_branch and ins.op not in ("jal", "jalr"):
                leaders.add(k + 2)
                if ins.op != "jr" and start <= ins.target < end:
                    leaders.add((ins.target - start) // 4)
                elif ins.op not in ("jr", "j") and end <= ins.target < end + 0x1000:
                    beyond.add(ins.target)  # (a branch is never a call, even where a function starts)
        leaders = sorted(x for x in leaders if 0 <= x < n)
        blocks = []
        block_of = {}
        for b, first in enumerate(leaders):
            blocks.append((first, leaders[b + 1] if b + 1 < len(leaders) else n))
            block_of[first] = b

        def successors(b):
            first, last = blocks[b]
            k = last - 2
            ins = insns[k] if k >= first else None
            out = []
            if ins is not None and ins.is_branch and ins.op not in ("jal", "jalr"):
                uncond = ins.op == "j" or (ins.op == "beq" and ins.rs == ins.rt == 0)
                if ins.op != "jr" and start <= ins.target < end:
                    out.append(block_of.get((ins.target - start) // 4))
                if not uncond and ins.op not in ("jr", "jalr"):
                    out.append(block_of.get(last))
            elif last < n:
                out.append(block_of.get(last))
            return [s for s in out if s is not None]

        saved = {16, 17, 18, 19, 20, 21, 22, 23, 28, 29, 30}
        refs: list[tuple[int, int | None, int]] = []   # (address, the base it came from, bytes)
        calls: set[int] = set()
        sequence: list[int] = []
        indexed: set[int] = set()
        traps: list[int] = []
        breaks: list[int] = []
        tail_jumps: set[int] = set()
        where: dict[int, list] = {}

        def meet(a, b):
            """What two ways into a block agree a register holds: the same value, or a place
            in the same array (a pointer a loop moves on)."""
            if a == b:
                return a
            if a and b and a[0] in ("full", "idx") and b[0] in ("full", "idx") and a[2] == b[2]:
                return ("idx", None, a[2])
            return None

        def likely_end(b):
            first, last = blocks[b]
            return last - 2 >= first and insns[last - 2].likely

        def run(state: dict, b: int, record: bool) -> tuple[dict, dict]:
            """The registers after the block, and (for a branch-likely's way on, whose delay slot
            does not run) before its last instruction."""
            st = dict(state)
            first, last = blocks[b]
            clobber_after = None
            before = st
            for k in range(first, last):
                if k == last - 1 and likely_end(b):
                    before = dict(st)
                ins = insns[k]
                op = ins.op
                new = []
                if op in TRAPS and record:
                    traps.append(k)
                if op == "break" and record:
                    breaks.append(k)
                if op in ("jal", "j") and record and not start <= ins.target < end:
                    calls.add(ins.target)
                    sequence.append(ins.target)
                    if op == "j":
                        tail_jumps.add(ins.target)
                if op == "lui":
                    new.append((ins.rt, ("hi", (ins.word & 0xFFFF) << 16, None)))
                elif op in LOADS_STORES or op in ("addiu", "daddiu", "addi", "daddi", "ori"):
                    imm = ins.imm if op != "ori" else (ins.word & 0xFFFF)
                    v = base = None
                    have = st.get(ins.rs)
                    if have and have[0] == "hi":
                        v = (have[1] | imm) if op == "ori" else have[1] + imm
                    elif have and have[0] == "full" and op != "ori":
                        v, base = have[1] + imm, have[2]
                    elif ins.rs == 28 and op != "ori" and self.gp:
                        v = self.gp + imm
                    elif have and have[0] == "idx":
                        # Somewhere in an array (a pointer a loop moves on): the array is indexed.
                        if record:
                            indexed.add(have[2])
                        if op in ("addiu", "daddiu", "addi", "daddi"):
                            new.append((ins.rt, have))
                    if v is not None:
                        v &= 0xFFFFFFFF
                        if record and (op != "ori" or self.function_at(prog, v)):
                            # An unaligned half (lwl, sdr) reaches the aligned word it is part of.
                            at = v & ~3 if op in ("lwl", "lwr", "swl", "swr") else \
                                v & ~7 if op in ("ldl", "ldr", "sdl", "sdr") else v
                            refs.append((at, base, WIDTH.get(op, 1)))
                            where.setdefault(at, []).append(ins.addr)
                        if op in ("addiu", "daddiu", "addi", "daddi", "ori"):
                            new.append((ins.rt, ("full", v, base if base is not None else v)))
                elif op in ("addu", "daddu", "or") and ins.rd and 0 in (ins.rs, ins.rt):
                    src = ins.rs or ins.rt          # a move: the address goes with it
                    if src in st:
                        new.append((ins.rd, st[src]))
                elif op in ("addu", "daddu", "add", "dadd", "subu", "dsubu") and record:
                    # An address plus a register: an array the code indexes.
                    for r in (ins.rs, ins.rt):
                        if st.get(r) and st[r][0] == "full":
                            indexed.add(st[r][1])
                for r in writes[k]:
                    st.pop(r, None)
                for r, v in new:
                    if r:
                        st[r] = v
                if clobber_after == k:
                    for r in [r for r in st if r not in saved]:
                        del st[r]
                if op in ("jal", "jalr"):
                    clobber_after = k + 1
            if clobber_after is not None and clobber_after >= last:
                for r in [r for r in st if r not in saved]:
                    del st[r]
            return st, (before if likely_end(b) else st)

        def edge(p, s):
            after, before = outs[p]
            return before if likely_end(p) and s == block_of.get(blocks[p][1]) and s != taken.get(p) else after

        succ = {b: successors(b) for b in range(len(blocks))}
        taken = {}
        for b, (first, last) in enumerate(blocks):
            if last - 2 >= first and insns[last - 2].is_branch and start <= insns[last - 2].target < end:
                taken[b] = block_of.get((insns[last - 2].target - start) // 4)
        preds: dict[int, list] = {b: [] for b in range(len(blocks))}
        for b in range(len(blocks)):
            for s in succ[b]:
                preds[s].append(b)
        # A jr through a register other than $ra is a jump table: its cases are the blocks
        # nothing else leads to.
        orphans = [b for b in range(1, len(blocks)) if not preds[b]]
        for b, (first, last) in enumerate(blocks):
            k = last - 2
            if k >= first and insns[k].op == "jr" and insns[k].rs != 31:
                succ[b] = succ[b] + orphans
                for o in orphans:
                    preds[o].append(b)
        outs: dict[int, dict] = {}
        entry: dict[int, dict] = {}
        work = [0]
        budget = 64 * len(blocks) + 64
        while work and budget:
            budget -= 1
            b = work.pop()
            have = [edge(p, b) for p in preds[b] if p in outs]
            if b == 0:
                state = {}
            elif not have:
                continue
            else:
                state = dict(have[0])
                for o in have[1:]:
                    state = {r: meet(v, o.get(r)) for r, v in state.items() if meet(v, o.get(r))}
            if b in outs and entry.get(b) == state:
                continue
            entry[b] = state
            out = run(state, b, False)
            if outs.get(b) != out:
                outs[b] = out
                work.extend(succ[b])
            elif b not in outs:
                outs[b] = out
        for b in range(len(blocks)):
            # A block no way reaches (a jump table's case): read with nothing known.
            run(entry.get(b, {}), b, True)

        out = Refs()
        derived: dict[int, set] = {}
        tails = sorted(beyond) if depth < 3 else []
        # Code that runs on past its catalogued end (a routine falling into a tail it shares with
        # another): read on there too.
        body = list(insns)
        while body and body[-1].word == 0:
            body.pop()          # the padding after a routine
        ends = body[-2:]
        if depth < 3 and ends and not any(i.op in ("jr", "j") or i.op in TRAPS or i.op == "beq" and i.rs == i.rt == 0
                                          for i in ends) and self.part(prog, end) is part:
            tails.append(end)
        tails = sorted(set(tails))
        for t in calls:
            f = self.function_at(prog, t)
            if f is None and t in tail_jumps and depth < 3:
                tails.append(t)         # a jump into a tail another function shares: read on there
                continue
            out.calls.add(f if f is not None else f"?{t:08X}")
        out.sequence = [self.function_at(prog, t) or f"?{t:08X}" for t in sequence]
        if traps and not (end - start <= 16):
            # A trap in line (a teq, a divide's break): the C calls the routine that only traps.
            out.calls.update(self.trap_functions())
        out.breaks = bool(breaks or traps)
        out.where = where
        for v, base, width in refs:
            f = self.function_at(prog, v)
            if f is not None:
                out.calls.add(f)
                continue
            if not 0x100000 <= v < DATA_TOP:
                continue
            p = self.part(prog, v)
            if p is not None and p.code and start <= v < end:
                continue
            if p is not None and p.holds(v, 8) and all(start <= p.word(v + d) < end for d in (0, 4)):
                continue        # a jump table of this function's (the C's switch)
            # Read-only data, and tables kept in the code (a constant row a routine loads): constants.
            (out.constants if p is not None and (p.name in READ_ONLY or p.code) else out.data).add(v)
            if width > 4:
                out.wide.update(range(v, v + width))
            if base is not None and base != v:
                derived.setdefault(base, set()).add(v)
        out.derived = {b: s for b, s in derived.items() if b in out.data}
        out.indexed = {v for v in indexed if 0x100000 <= v < DATA_TOP}
        for t in tails:
            more = self.code_refs(prog, t, depth + 1)
            if more is not None:
                out.update(more)
                out.data |= more.data
                out.derived.update(more.derived)
                out.wide |= more.wide
                out.breaks = out.breaks or more.breaks
                out.sequence = [x for x in out.sequence if x != f"?{t:08X}"] + more.sequence
        return out


# ---- the C side: Clang's IR of each hand file -----------------------------------------------------

GLOBAL_REF = r'@(?:"((?:[^"\\]|\\.)*)"|([\w.$]+))'
DEFINE = re.compile(r"^define\s[^@]*" + GLOBAL_REF + r"\(", re.M)
DECLARE = re.compile(r"^declare\s[^@]*" + GLOBAL_REF + r"\(", re.M)
GEP_I8 = re.compile(r"getelementptr (?:inbounds )?(?:nuw )?(?:nusw )?\(i8, ptr " + GLOBAL_REF + r", i(?:32|64) (-?\d+)\)")
GEP_VAR = re.compile(r"getelementptr [^\n]*?ptr " + GLOBAL_REF + r"(?:, i(?:32|64) -?\d+)*, i(?:32|64) %")
GEP_I8_VAR = re.compile(r"getelementptr [^\n]*?ptr getelementptr (?:inbounds )?(?:nuw )?(?:nusw )?\(i8, ptr "
                        + GLOBAL_REF + r", i(?:32|64) (-?\d+)\)(?:, i(?:32|64) -?\d+)*, i(?:32|64) %")
UNSIZED = re.compile(r"(\bextern\b[^;{}()=]*?\b\w+\s*(?:__asm__\s*\([^)]*\)\s*)?)\[\s*\](?=[^;{}]*;)")
UNSIZED_ELEMENTS = 0x10000
MEMOP = re.compile(r"@llvm\.mem(?:cpy|move|set)[\w.]*\((.*), i(?:32|64) (\d+), i1 \w+\)")
INTTOPTR = re.compile(r"inttoptr \(i(?:32|64) (-?\d+) to ptr\)")
REF = re.compile(GLOBAL_REF)
GLOBAL_ARRAY = re.compile(r"^" + GLOBAL_REF + r" = (?:[\w ]*?)global (\[[^=\n]*\]|%[\w.]+),", re.M)
TYPE_DEF = re.compile(r"^(%[\w.]+) = type (<?\{.*\}>?)$", re.M)
IR_SCALAR = {"i8": 1, "i16": 2, "i32": 4, "i64": 8, "i128": 16, "float": 4, "double": 8, "ptr": 4}


def _ir_layout(t: str, types: dict, depth: int = 0) -> tuple[int, int] | None:
    """(bytes, alignment) of an IR type: scalars, arrays, vectors, named and literal structs."""
    t = t.strip()
    if depth > 16:
        return None
    if t in IR_SCALAR:
        return IR_SCALAR[t], IR_SCALAR[t]
    m = re.fullmatch(r"\[(\d+) x (.+)\]", t) or re.fullmatch(r"<(\d+) x (.+)>", t)
    if m:
        inner = _ir_layout(m.group(2), types, depth + 1)
        if inner is None:
            return None
        n = int(m.group(1)) * inner[0]
        return (n, n if t.startswith("<") else inner[1])
    if t in types:
        return _ir_layout(types[t], types, depth + 1)
    packed = t.startswith("<{")
    m = re.fullmatch(r"<?\{(.*)\}>?", t)
    if m:
        size, align = 0, 1
        for part in _split_types(m.group(1)):
            lay = _ir_layout(part, types, depth + 1)
            if lay is None:
                return None
            if not packed:
                size = (size + lay[1] - 1) // lay[1] * lay[1]
                align = max(align, lay[1])
            size += lay[0]
        if not packed:
            size = (size + align - 1) // align * align
        return size, (1 if packed else align)
    return None


def _split_types(body: str) -> list[str]:
    parts, depth, cur = [], 0, ""
    for ch in body:
        if ch in "[{<":
            depth += 1
        elif ch in "]}>":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur)
            cur = ""
        else:
            cur += ch
    if cur.strip():
        parts.append(cur)
    return parts


def _ir_name(m, base=1) -> str:
    q, plain = m.group(base), m.group(base + 1)
    name = q if q is not None else plain
    return name[3:] if name.startswith("\\01") else name


@dataclass
class Module:
    path: Path
    error: str | None = None
    bodies: dict = field(default_factory=dict)      # function defined -> its IR
    internal: set = field(default_factory=set)      # of those, the file's own (static)
    functions: set = field(default_factory=set)     # every function named (declared or defined)
    sizes: dict = field(default_factory=dict)       # a global array's name -> its declared bytes


def compile_ir(path: Path, flags: list[str], clang: str) -> Module:
    """Clang's IR of one hand file, prepared as hostgen prepares the C it reads (prep.py: the EE's
    64-bit long above all, which lays a struct of longs out as the console does)."""
    import prep
    import tempfile
    mod = Module(path)
    with tempfile.TemporaryDirectory(prefix="xcheck") as tmp:
        src = Path(tmp) / path.name
        # An extern array of no size ([]) is one of none to the optimiser, which then takes every
        # access past its start for undefined and drops the code around it: give it room.
        text = UNSIZED.sub(lambda m: m.group(1) + "[UNSIZED_ELEMENTS]", prep.prepare(path.read_text(errors="replace")))
        src.write_text(f"#define UNSIZED_ELEMENTS {UNSIZED_ELEMENTS}\n" + text)
        cmd = [clang, *flags, "-I", str(path.parent), "-O1", "-fno-inline-functions", "-ffreestanding",
               "-fno-builtin", "-S", "-emit-llvm", "-o", "-", str(src)]
        r = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
        if r.returncode and "too large" in r.stderr:
            src.write_text(text.replace("[UNSIZED_ELEMENTS]", "[]"))
            r = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
        if r.returncode:
            # Hand C is read after its decompilation unit's own declarations; alone it may want
            # the decompilation's basic types.
            r = subprocess.run(cmd[:1] + ["-include", "types.h"] + cmd[1:], capture_output=True, text=True,
                               errors="replace")
    if r.returncode:
        mod.error = next((l for l in r.stderr.splitlines() if "error:" in l), r.stderr.strip()[:200])
        return mod
    ir = r.stdout
    types = {m.group(1): m.group(2) for m in TYPE_DEF.finditer(ir)}
    for m in GLOBAL_ARRAY.finditer(ir):
        lay = _ir_layout(m.group(3), types)
        if lay and lay[0] > 1 and not re.match(rf"\[{UNSIZED_ELEMENTS} x ", m.group(3)):
            mod.sizes[_ir_name(m)] = lay[0]
    for m in DECLARE.finditer(ir):
        mod.functions.add(_ir_name(m))
    for m in DEFINE.finditer(ir):
        name = _ir_name(m)
        mod.functions.add(name)
        end = ir.find("\n}\n", m.end())
        mod.bodies[name] = ir[m.start():end + 2]
        head = ir[m.start():m.end()]
        if re.match(r"define\s+(?:dso_local\s+)?(?:internal|private)\b", head):
            mod.internal.add(name)
    return mod


class Checker:
    def __init__(self, game: Game, prepared: Path | None = None):
        self.game = game
        self.rows: list[tuple[str, str]] = []
        for line in game.hand_table.read_text().splitlines():
            if line.startswith("#") or "\t" not in line:
                continue
            name, rel = line.split("\t")[:2]
            self.rows.append((name, rel))
        self.row_names = {n for n, _ in self.rows}
        import clangast
        cfg = game.cfg
        # The decompilation's headers as hostgen prepared them (GEN/prep), else as they are.
        root = prepared if prepared is not None and prepared.is_dir() else game.source
        includes = [root / i for i in cfg.get("includes", [])] + [HOSTGEN / "include"]
        flags = [f for f in clangast.base_flags(includes, cfg.get("defines", [])) if f != "-fsyntax-only"]
        clang = clangast.clang_binary()
        from concurrent.futures import ThreadPoolExecutor
        rels = sorted({rel for _, rel in self.rows})
        with ThreadPoolExecutor(max_workers=min(8, os.cpu_count() or 1)) as pool:
            mods = pool.map(lambda rel: compile_ir(game.hand_table.parent / rel, flags, clang), rels)
        self.modules: dict[str, Module] = dict(zip(rels, mods))
        # Functions the hand files define (with what they are), for following a helper.
        self.defined: dict[str, tuple[str, Module]] = {}
        for rel, mod in self.modules.items():
            for name in mod.bodies:
                if name not in mod.internal:
                    self.defined.setdefault(name, (rel, mod))
        self._memo: dict[tuple[str, str], Refs] = {}
        self._row_defs = {d for d in (self.definition(n, rel) for n, rel in self.rows) if d}
        self.verbose = False

    def canon(self, name: str) -> str | None:
        r = self.game.resolve_row(name)
        return r[1] if r and r[0] == "call" else None

    def definition(self, name: str, rel: str) -> str | None:
        """The IR function of a row: by name, or the one bound at the row's place."""
        mod = self.modules[rel]
        if name in mod.bodies:
            return name
        target = self.canon(name)
        if target is not None:
            for d in mod.bodies:
                if d not in mod.internal and self.canon(d) == target:
                    return d
        public = [d for d in mod.bodies if d not in mod.internal]
        return public[0] if len(public) == 1 else None

    def c_refs(self, rel: str, fn: str, stack=()) -> Refs:
        key = (rel, fn)
        if key in self._memo:
            return self._memo[key]
        mod = self.modules[rel]
        body = mod.bodies[fn]
        body = body[body.index("{"):]
        out = Refs()
        g = self.game
        in_gep = set()          # where a folded offset from a global starts
        for m in GEP_I8.finditer(body):
            r = g.resolve(_ir_name(m))
            if r is not None and r[0] == "data" and 0x100000 <= r[1] < DATA_TOP:
                out.data.add((r[1] + int(m.group(3))) & 0xFFFFFFFF)
                if _ir_name(m) in mod.sizes:
                    out.arrays[r[1]] = max(out.arrays.get(r[1], 0), mod.sizes[_ir_name(m)])
                in_gep.add(m.start())
        var_base = set()        # where a global is the base of an element reached with an index
        for m in GEP_VAR.finditer(body):
            r = g.resolve(_ir_name(m))
            if r is not None and r[0] == "data":
                out.indexed.add(r[1])
                var_base.add(m.start(1) - 2 if m.group(1) is not None else m.start(2) - 1)
        for m in GEP_I8_VAR.finditer(body):
            r = g.resolve(_ir_name(m))
            if r is not None and r[0] == "data":
                out.indexed.add((r[1] + int(m.group(3))) & 0xFFFFFFFF)
        # A block copy or fill (a struct assigned, an array cleared): its bytes are reached.
        for m in MEMOP.finditer(body):
            args, size = m.group(1), int(m.group(2))
            for g8 in GEP_I8.finditer(args):
                r = g.resolve(_ir_name(g8))
                if r is not None and r[0] == "data":
                    base = (r[1] + int(g8.group(3))) & 0xFFFFFFFF
                    out.arrays[base] = max(out.arrays.get(base, 0), size)
            for ref in re.finditer(r"ptr (?:noundef |nonnull |align \d+ |dereferenceable\(\d+\) )*" + GLOBAL_REF, args):
                r = g.resolve(_ir_name(ref))
                if r is not None and r[0] == "data":
                    out.arrays[r[1]] = max(out.arrays.get(r[1], 0), size)
        for m in INTTOPTR.finditer(body):
            v = int(m.group(1)) & 0xFFFFFFFF
            if 0x100000 <= v < DATA_TOP:
                f = g.function_at(-1, v) if v < g.low else None
                (out.calls.add(f) if f else out.data.add(v))
        for m in REF.finditer(body):
            name = _ir_name(m)
            if name.startswith("llvm.") or "." in name or name == fn:
                # Intrinsics, string literals, Clang's own tables, a function's static locals.
                continue
            if any(m.start() - k in in_gep for k in range(20, 60)):
                continue                    # a global inside a folded offset: counted with it above
            if m.start() in var_base:
                continue                    # an array's base, indexed: its elements are not known
            # The hand files' own functions that are no rows: followed into.
            if name in mod.internal and name in mod.bodies:
                if (rel, name) not in stack:
                    out.update(self.c_refs(rel, name, stack + (key,)))
                continue
            if name in self.defined and name not in self.row_names and name not in self._row_defs:
                # A function the hand files define that is no row (a wrapper, a helper): a call to
                # it where hostgen binds it at a place, and what it reaches kept for the comparison,
                # which reads through it where the code does not call that place.
                hrel, _ = self.defined[name]
                r = g.resolve(name)
                if (hrel, name) not in stack:
                    inner = self.c_refs(hrel, name, stack + (key,))
                    if r is not None and r[0] == "call":
                        out.calls.add(r[1])
                        out.helpers[r[1]] = inner
                        out.helpers.update(inner.helpers)
                    else:
                        out.update(inner)
                continue
            r = g.resolve(name)
            if r is None:
                (out.host if name in mod.functions else out.unknown).add(name)
                continue
            if r[0] == "call":
                out.calls.add(r[1])
            elif 0x100000 <= r[1] < DATA_TOP:
                out.data.add(r[1])
            if r[0] == "data" and name in mod.sizes:
                out.arrays[r[1]] = max(out.arrays.get(r[1], 0), mod.sizes[name])
        self._memo[key] = out
        return out

    def through_helpers(self, c: Refs, code: Refs) -> Refs:
        """The C's references with each call to a hand-file helper that is no retail function and
        that the code does not call (a wrapper of the port's own at an address no function starts
        at, func_001F9688 for FUN_001f96f8) replaced by what it reaches."""
        out = Refs()
        out.update(c)
        out.data = set(c.data)
        out.arrays = dict(c.arrays)
        done = set()
        while True:
            fam = {self.game.split_parent.get(x, x) for x in code.calls}
            todo = [h for h in out.calls if h in c.helpers and h not in done and not self.game.retail_start(h)
                    and self.game.split_parent.get(h, h) not in fam]
            if not todo:
                return out
            for h in todo:
                done.add(h)
                out.calls.discard(h)
                inner = c.helpers[h]
                out.update(inner)
                out.data |= inner.data
                out.arrays.update(inner.arrays)

    def check_row(self, name: str, rel: str) -> list[tuple[str, str]]:
        """[(kind, item)] of what differs for one row."""
        g = self.game
        mod = self.modules[rel]
        if mod.error:
            return [("no-code", f"{rel} does not compile alone: {mod.error}")]
        fn = self.definition(name, rel)
        if fn is None:
            return [("no-code", "no definition in " + rel)]
        canon = self.canon(fn) or self.canon(name)
        home = g.home(canon) if canon else None
        if home is None:
            return [("no-code", "no place for " + name)]
        # C written with one level's globals (D_L00_...) for a function the executable and the
        # levels both carry is that level's copy: it is checked there.
        body = self.modules[rel].bodies[fn]
        levels = {int(x) for x in re.findall(r"@\"?D_L(\d\d)_", body)}
        if home[0] < 0 and len(levels) == 1:
            lv = levels.pop()
            there = [p for p in g.group.get(canon, []) if p[0] == lv]
            if there:
                home = there[0]
        code = g.code_refs(*home)
        if code is None:
            return [("no-code", f"no retail code at {home[1]:08X}")]
        c = self.c_refs(rel, fn)
        c = self.through_helpers(c, code)
        if self.verbose:
            fmt = lambda xs: " ".join(x if isinstance(x, str) else f"{x:08X}" for x in sorted(xs, key=str))
            print(f"  {name}: {fn} in {rel}, checked at {home[0]}:{home[1]:08X}")
            print(f"    C calls: {fmt(c.calls)}\n    C host:  {fmt(c.host)}\n    C data:  {fmt(c.data)}")
            print(f"    code calls: {fmt(code.calls)}\n    code data:  {fmt(code.data)}\n"
                  f"    code constants: {fmt(code.constants)}")
            for v in sorted((code.data | code.constants) - c.data):
                print(f"    {v:08X} at " + " ".join(f"{a:08X}" for a in code.where.get(v, [])))
        out = []
        fam = lambda xs: {g.split_parent.get(x, x) for x in xs}
        c_calls, code_calls = fam(c.calls), fam(code.calls)
        for f in sorted(c_calls - code_calls - {canon}):
            if f in g.trap_functions() and code.breaks:
                continue        # the C's trap where the code's divide check breaks
            out.append(("c-call", f))
        for f in sorted(code_calls - c_calls - {canon}):
            out.append(("code-call", f))
        # A quadword the code moves whole covers the C's words in it, and the other way round.
        quads = {v & ~15 for v in c.data}
        within = lambda v, bases: any(b <= v < b + ARRAY for b in bases)
        for v in sorted(c.data - code.data - code.constants - code.wide):
            if within(v, code.indexed):
                continue        # an element of an array the code indexes
            out.append(("c-data", f"0x{v:08X}"))
        for v in sorted(code.data - c.data):
            if (v & ~15) in quads and (v & ~15) in c.data:
                continue
            if code.derived.get(v, set()) & c.data or any(within(d, c.indexed) for d in code.derived.get(v, ())):
                continue        # a base the code forms to reach what the C reaches through it
            if within(v, c.indexed):
                continue        # an element of an array the C indexes (a loop the code unrolled)
            if any(b <= v < b + n for b, n in c.arrays.items()):
                continue        # inside an array the C declares with its size and reaches
            out.append(("code-data", f"0x{v:08X}"))
        for s in sorted(c.unknown):
            out.append(("c-data", s))
        # The other copies (the levels' programs): call by call against the home copy. Where a copy
        # calls something else, the C's one name is right only if hostgen takes the copy's own
        # call for it (g.per_copy).
        seen = set()
        for prog, addr in g.group.get(canon, []):
            if (prog, addr) == home:
                continue
            lv = g.code_refs(prog, addr)
            if lv is None:
                continue
            tag = f"L{prog:02d}" if prog >= 0 else "exe"
            if len(lv.sequence) == len(code.sequence):
                pairs = [(h, x) for h, x in zip(code.sequence, lv.sequence) if h != x]
            else:
                pairs = [(None, x) for x in sorted(lv.calls - code.calls)]
            for h, x in pairs:
                if x == canon or (h is not None and g.per_copy(h, prog)) or (tag, x) in seen:
                    continue
                if h is None and x in c.calls:
                    continue
                seen.add((tag, x))
                out.append(("level-call", f"{tag}:{x}"))
        return out


def read_allow(path: Path) -> list[tuple[str, str, str, str]]:
    rows = []
    if path.is_file():
        for line in path.read_text().splitlines():
            if not line.strip() or line.startswith("#"):
                continue
            f = line.split("\t")
            if len(f) < 4 or f[1] not in KINDS or not f[3].strip():
                raise SystemExit(f"{path}: a row needs name, kind ({', '.join(KINDS)}), item and a reason: {line!r}")
            rows.append((f[0], f[1], f[2], f[3]))
    return rows


def allowed(allow, name, kind, item) -> tuple | None:
    for a in allow:
        if a[0] in (name, "*") and a[1] == kind and (a[2] == "*" or a[2] == item):
            return a
    return None


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mode", choices=("report", "check"))
    ap.add_argument("game_dir", type=Path)
    ap.add_argument("--gen", type=Path, required=True)
    ap.add_argument("--asm-source", type=Path, default=None)
    ap.add_argument("--allow", type=Path, default=None)
    ap.add_argument("--all", action="store_true", help="report: also the differences allow.tsv explains")
    ap.add_argument("-v", "--verbose", action="store_true", help="report: what each side reaches")
    ap.add_argument("names", nargs="*")
    a = ap.parse_args(argv)
    asm_source = a.asm_source
    if asm_source is None:
        env = os.environ.get(f"OPENRAC_{a.game_dir.resolve().name.upper().replace('-', '_')}_ASM_SOURCE")
        asm_source = Path(env) if env else None
    if not (a.gen / "functions.c").is_file():
        print(f"xcheck: no {a.gen / 'functions.c'}: build the game first", file=sys.stderr)
        return 77 if a.mode == "check" else 1
    game = Game(a.game_dir, a.gen, asm_source)
    if game.retail is None:
        print("xcheck: no retail program to read (run the decompilation's setup, or pass --asm-source); skipped",
              file=sys.stderr)
        return 77 if a.mode == "check" else 1
    ck = Checker(game, a.gen / "prep")
    ck.verbose = a.verbose
    allow = read_allow(a.allow or game.hand_table.parent / "allow.tsv")
    used = set()
    bad = 0
    want = {n.lower() for n in a.names}
    # hostgen's report: the rows whose C it placed (a row replaces a function the decompilation has
    # as assembly; where it has its own C now, the row is not read and there is nothing to check).
    placed = None
    report = a.gen / "report.json"
    if report.is_file():
        placed = set(json.loads(report.read_text()).get("candidates_used", []))
    unused = []
    for name, rel in ck.rows:
        if want and name.lower() not in want and Path(rel).stem.lower() not in want:
            continue
        if placed is not None and name not in placed:
            unused.append(name)
            continue
        shown = []
        for kind, item in ck.check_row(name, rel):
            hit = allowed(allow, name, kind, item)
            if hit:
                used.add(hit)
                if a.all:
                    shown.append(f"({kind} {item})")
                continue
            shown.append(f"{kind} {item}")
            bad += 1
        if shown or (a.mode == "report" and want):
            print(f"{name} [{rel}]: " + ("; ".join(shown) if shown else "agrees"))
    if unused:
        print(f"xcheck: {len(unused)} row(s) hostgen does not read (the decompilation has its own C for "
              f"them; the row can go): {' '.join(unused)}", file=sys.stderr)
    if not want:
        for row in allow:
            if row not in used and (placed is None or row[0] == "*" or row[0] in placed):
                print(f"allow.tsv: nothing differs any more for: {row[0]} {row[1]} {row[2]}")
                bad += 1
    print(f"xcheck: {len(ck.rows) if not want else len(want)} rows, {bad} unexplained difference(s)",
          file=sys.stderr)
    return 1 if (a.mode == "check" and bad) else 0


if __name__ == "__main__":
    sys.exit(main())

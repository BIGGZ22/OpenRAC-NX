# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The retail program's instructions, read from the player's own executable and level programs.

asmc.py writes C for the functions with no C from their instructions. The decompilations keep
those as assembly files their setup writes (asm.py reads them), which only a machine that ran the
decompilation's setup has. This reads the same instructions from the program itself, wherever the
port is built from the player's disc:

- the executable (an ELF: its code sections, and its data for jump tables), and
- each level's program (the records ParseBin loads over the executable from `relocate`'s low end
  up: lit, data, the class tables, text), from level dumps (level_NN/manifest.json and its
  records, as rac1-decomp's tools/overlays.py writes them) or from the disc image itself (the
  level's overlay section, through the editor's own reader, editor/disc.py and formats.py).

What the assembly's text says beyond the words, this works out the way spimdisasm does:

- where functions start: every function hostgen knows, by its places;
- which immediates are halves of an address: a lui and the addiu, ori, load or store that
  completes it on the same register before that register is written again. An address in the
  program's code is the function there (and an offset into it); one in game RAM is a global,
  named as the decompilations name them (D_<addr>, D_Lnn_<addr> for a level's own data from
  `relocate`'s low end up), so hostgen relocates it as it does the C's;
- jump tables: a jr through a register loaded from such an address after an addu with an index,
  with the words there that are addresses in the same function.

Nothing is written or kept: the bytes are read when the port is built, and only C comes out.
"""
from __future__ import annotations

import bisect
import json
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import asm

RAM = (0x00100000, 0x02000000)
# A Ratchet & Clank level program's records in their order, as the level dumps name them.
RECORD_NAMES = ("lit", "bss", "data", "vtbl", "camvtbl", "sndvtbl", "text")
LOADS_STORES = {"lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "lq", "lwl", "lwr", "ldl", "ldr", "sb", "sh", "sw",
                "sd", "sq", "swl", "swr", "sdl", "sdr", "lwc1", "swc1", "lqc2", "sqc2"}


@dataclass
class Part:
    name: str
    base: int
    data: bytes
    code: bool

    def holds(self, addr: int, size: int = 4) -> bool:
        return self.base <= addr and addr + size <= self.base + len(self.data)

    def word(self, addr: int) -> int:
        return struct.unpack_from("<I", self.data, addr - self.base)[0]


class Retail:
    """The executable's sections and each level's records."""

    def __init__(self, exe: bytes | None, levels: dict[int, list[Part]], low: int, label: str = "retail"):
        self.exe: list[Part] = elf_parts(exe) if exe else []
        self.entry = struct.unpack_from("<I", exe, 0x18)[0] if exe else None
        self.levels = levels
        self.low = low
        self.label = label

    @classmethod
    def from_files(cls, exe: Path, dump: Path | None, low: int) -> "Retail":
        return cls(exe.read_bytes(), read_dump(dump) if dump else {}, low, exe.name)

    @classmethod
    def from_image(cls, image: Path, low: int) -> "Retail":
        exe, levels = read_image(image)
        return cls(exe, levels, low, image.name)

    def program(self, level: int) -> list[Part]:
        """Everything in game memory while program `level` (-1 the executable) is loaded."""
        if level < 0:
            return self.exe
        out = [Part(p.name, p.base, p.data[:max(0, self.low - p.base)], p.code) for p in self.exe
               if p.base < self.low]
        return out + self.levels.get(level, [])

    def code_parts(self, home: int) -> list[Part]:
        """The code of program `home` itself (a level's own text, not the executable's under it)."""
        return [p for p in (self.exe if home < 0 else self.levels.get(home, [])) if p.code]

    def called(self) -> set[tuple[int, int]]:
        """(program, address) every jal and j of each program's own code goes to, and every code
        address it forms with a lui and an addiu or ori (a callback it hands on)."""
        out = set()
        for home in [-1] + sorted(self.levels):
            parts = self.code_parts(home)
            in_code = lambda t: any(q.holds(t) for q in parts) or any(q.holds(t) for q in self.code_parts(-1))
            for p in parts:
                n = len(p.data) // 4
                hi: dict[int, tuple[int, int]] = {}
                for k, w in enumerate(struct.unpack_from(f"<{n}I", p.data)):
                    op = w >> 26
                    t = None
                    if op in (2, 3):
                        t = ((p.base + 4 * k) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
                    elif op == 0x0F:
                        hi[(w >> 16) & 31] = ((w & 0xFFFF) << 16, k)
                    elif op in (0x09, 0x0D) and ((w >> 21) & 31) in hi and k - hi[(w >> 21) & 31][1] <= 8:
                        value = hi[(w >> 21) & 31][0]
                        imm = w & 0xFFFF
                        t = (value | imm) if op == 0x0D else (value + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF
                        if not in_code(t):
                            t = None
                    if t is not None:
                        # Below `low` is the executable's code whatever program is loaded.
                        out.add((home if t >= self.low else -1, t))
        return out

    def word(self, level: int, addr: int) -> int | None:
        for p in self.program(level):
            if p.holds(addr):
                return p.word(addr)
        return None


def elf_parts(b: bytes) -> list[Part]:
    """An EE executable's loaded sections (its code, and its data), or its segments without them."""
    out = []
    shoff, = struct.unpack_from("<I", b, 0x20)
    shnum, shstrndx = struct.unpack_from("<HH", b, 0x30)
    if shnum and shstrndx < shnum and shoff + 40 * shnum <= len(b):
        stro = struct.unpack_from("<I", b, shoff + 40 * shstrndx + 16)[0]
        for i in range(shnum):
            nm, ty, fl, ad, of, sz = struct.unpack_from("<6I", b, shoff + 40 * i)
            name = b[stro + nm:b.index(b"\0", stro + nm)].decode(errors="replace")
            # Loaded (SHF_ALLOC), with contents (not NOBITS); the VU microcode is not EE code.
            if ad and sz and ty != 8 and fl & 2 and name != ".vutext":
                out.append(Part(name, ad, b[of:of + sz], bool(fl & 4)))
    if not out:
        phoff, = struct.unpack_from("<I", b, 0x1C)
        for i in range(struct.unpack_from("<H", b, 0x2C)[0]):
            kind, off, va, _pa, size = struct.unpack_from("<5I", b, phoff + 32 * i)
            if kind == 1:
                out.append(Part("load", va, b[off:off + size], True))
    return out


def read_dump(dump: Path) -> dict[int, list[Part]]:
    levels = {}
    for d in sorted(dump.glob("level_*")):
        man = d / "manifest.json"
        if not man.is_file():
            continue
        m = json.loads(man.read_text())
        parts = []
        for r in m["records"]:
            f = d / f"{r['name']}.bin"
            if r.get("type") == 1 and f.is_file():
                parts.append(Part(r["name"], r["address"], f.read_bytes(), r["name"] == "text"))
        levels[int(m["level"])] = parts
    return levels


def read_image(image: Path) -> tuple[bytes, dict[int, list[Part]]]:
    """The executable and every level's program from a disc image, through the editor's reader."""
    editor = Path(__file__).resolve().parents[3] / "editor"
    if str(editor) not in sys.path:
        sys.path.insert(0, str(editor))
    from disc import Disc  # noqa: E402  (the editor's ISO and table-of-contents reader)
    from formats import overlay_sections, unpack  # noqa: E402
    from level import decoded  # noqa: E402
    levels: dict[int, list[Part]] = {}
    with Disc(image) as disc:
        exe = disc.executable()
        for info in disc.survey()["levels"]:
            r = info["ranges"]["data"]
            # The level data header's first pair is the overlay: read that much, not the level.
            offset, size = unpack("<ii", disc.sectors(r["lba"], 1), 0)
            if not size or offset < 0 or offset + size > r["sectors"] * 2048:
                continue
            overlay = decoded(disc.read(r["lba"] * 2048 + offset, size))
            sections = overlay_sections(overlay)
            parts = []
            recs = sections["sections"]
            names = RECORD_NAMES if len(recs) == len(RECORD_NAMES) else [f"record{k}" for k in range(len(recs))]
            for name, rec in zip(names, recs):
                if rec["type"] != 1:
                    continue
                body = overlay[rec["offset"]:rec["offset"] + rec["bytes"]]
                code = rec["address"] <= sections["entry_point"] < rec["address"] + rec["bytes"]
                parts.append(Part("text" if code else name, rec["address"], body, code))
            levels[int(info["id"])] = parts
    return exe, levels


# ---- The instructions, as asm.Index has them ---------------------------------------------------

class RetailCode:
    """(program, address) -> (asm.Line, asm.Block), for every word of a program's code: the
    mapping asm.Index.code is, read from the retail bytes when first asked for."""

    def __init__(self, retail: Retail, starts: dict[int, dict[int, str]], low: int):
        self.retail = retail
        self.low = low
        self.starts = starts                      # program -> {address: name}
        self._sorted = {h: sorted(m) for h, m in starts.items()}
        self._blocks: dict[tuple[int, int], asm.Block] = {}
        self._pairs: dict[tuple[int, int], tuple[dict, dict]] = {}
        self._lines: dict[tuple[int, int], tuple[asm.Line, asm.Block]] = {}

    def __contains__(self, key) -> bool:
        return self.get(key) is not None

    def get(self, key, default=None):
        hit = self._lines.get(key)
        if hit is not None:
            return hit
        home, addr = key
        if addr & 3:
            return default
        part = next((p for p in self.retail.code_parts(home) if p.holds(addr)), None)
        if part is None:
            return default
        starts = self._sorted.get(home, [])
        i = bisect.bisect_right(starts, addr) - 1
        start = starts[i] if i >= 0 and starts[i] >= part.base else part.base
        end = starts[i + 1] if i + 1 < len(starts) else part.base + len(part.data)
        end = min(end, part.base + len(part.data))
        self._block(home, part, start, end)
        return self._lines.get(key, default)

    def _block(self, home: int, part: Part, start: int, end: int) -> None:
        if (home, start) in self._blocks:
            return
        name = self.starts.get(home, {}).get(start, f"code_{start:08X}")
        block = asm.Block(name, Path(f"<{self.retail.label}>"))
        words = [part.word(a) for a in range(start, end, 4)]
        # The pairs over the whole of the program's code at once: a catalogue's function boundary
        # may fall between a lui and its other half (a function it cut in two).
        key = (home, part.base)
        if key not in self._pairs:
            n = len(part.data) // 4
            self._pairs[key] = self.pairs(home, part.base, list(struct.unpack_from(f"<{n}I", part.data)))
        all_relocs, all_tables = self._pairs[key]
        relocs = {a: r for a, r in all_relocs.items() if start <= a < end}
        tables = {a: t for a, t in all_tables.items() if start <= a < end}
        for k, w in enumerate(words):
            a = start + 4 * k
            line = asm.Line(a, w, "", relocs.get(a))
            block.lines.append(line)
            self._lines[(home, a)] = (line, block)
        block.entries[name] = start
        for jr, table in tables.items():
            targets = []
            for k in range(1024):
                t = self.retail.word(home, table + 4 * k)
                if t is None or t & 3 or not start <= t < end:
                    break
                targets.append(t)
            if targets:
                block.tables[self._table_name(home, table)] = targets
        self._blocks[(home, start)] = block

    def _table_name(self, home: int, address: int) -> str:
        return f"jtbl_L{home:02d}_{address:08X}" if home >= 0 and address >= self.low else f"jtbl_{address:08X}"

    def _symbol(self, home: int, address: int, table: bool) -> tuple[str, int] | None:
        """The symbol a %hi/%lo pair names: the function (and offset) for a code address, the jump
        table, or the global the decompilations would name there."""
        if table:
            return self._table_name(home, address), 0
        # A function there: a level's own from `low` up, the executable's below it (or in its own
        # program). An address inside a function's code is data kept there (a table), as
        # spimdisasm names it.
        h = home if home < 0 or address >= self.low else -1
        if address in self.starts.get(h, {}):
            return self.starts[h][address], 0
        if not RAM[0] <= address < RAM[1]:
            return None
        if home >= 0 and address >= self.low:
            return f"D_L{home:02d}_{address:08X}", 0
        return f"D_{address:08X}", 0

    def pairs(self, home: int, start: int, words: list[int]):
        """The %hi/%lo relocations of a run of code's words, and its jump tables (jr address ->
        table address)."""
        from asmc import rw   # (asmc imports this module)
        hi: dict[int, tuple[int, int]] = {}       # register -> (high half, lui address)
        full: dict[int, int] = {}                 # register -> an address a pair completed in it
        idx: dict[int, tuple] = {}                # register -> (high half, lui address) or (address,) plus an index
        loaded: dict[int, int] = {}               # register -> the table a word was loaded from
        tables: dict[int, int] = {}
        uses: dict[int, list] = {}
        leave_after = None                        # the delay slot of a jr or j: nothing goes on past it
        insns = [asm.decode(w, start + 4 * k) for k, w in enumerate(words)]
        writes = [rw(i)[1] for i in insns]
        for k, w in enumerate(words):
            pc = start + 4 * k
            if leave_after is not None and pc > leave_after:
                hi.clear(); full.clear(); idx.clear(); loaded.clear()
                leave_after = None
            ins = insns[k]
            op = ins.op
            if op in ("jr", "j"):
                leave_after = pc + 4
            new = []
            if op == "lui":
                new.append((hi, ins.rt, ((w & 0xFFFF) << 16, pc)))
            elif op in LOADS_STORES or op in ("addiu", "daddiu", "ori"):
                imm = ins.imm if op != "ori" else (w & 0xFFFF)
                if ins.rs in hi:
                    value, at = hi[ins.rs]
                    s = (value | imm) if op == "ori" else (value + imm) & 0xFFFFFFFF
                    uses.setdefault(at, []).append(s)
                    if op in ("addiu", "daddiu", "ori"):
                        new.append((full, ins.rt, s))
                elif ins.rs in idx and op == "lw":
                    base = idx[ins.rs]
                    if len(base) == 2:              # lui, addu index, lw %lo(table)
                        s = (base[0] + imm) & 0xFFFFFFFF
                        uses.setdefault(base[1], []).append(s)
                    else:                           # lui, addiu %lo(table), addu index, lw 0
                        s = (base[0] + imm) & 0xFFFFFFFF
                    new.append((loaded, ins.rt, s))
            elif op in ("addu", "daddu") and ins.rd:
                for maps, wrap in ((hi, lambda v: v), (full, lambda v: (v,))):
                    if (ins.rs in maps) != (ins.rt in maps):
                        new.append((idx, ins.rd, wrap(maps[ins.rs if ins.rs in maps else ins.rt])))
                        break
            if op == "jr" and ins.rs != 31 and ins.rs in loaded:
                tables[pc] = loaded[ins.rs]
            for r in writes[k]:
                for m in (hi, full, idx, loaded):
                    m.pop(r, None)
            for m, r, v in new:
                m[r] = v
        # A lui whose other half the straight run did not reach (it sits in a branch's delay slot,
        # the half at the branch's target): follow the branches a short way.
        for k, w in enumerate(words):
            if w >> 26 == 0x0F and start + 4 * k not in uses:
                s = self._follow(insns, writes, k, (w >> 16) & 31, (w & 0xFFFF) << 16)
                if s is not None:
                    uses[start + 4 * k] = [s]
        relocs = {}
        table_addrs = set(tables.values())
        for at, items in uses.items():
            s = items[0]
            sym = self._symbol(home, s, s in table_addrs)
            if sym is not None:
                relocs[at] = ("hi", sym[0], sym[1])
        return relocs, tables


    @staticmethod
    def _follow(insns: list, writes: list, k: int, reg: int, value: int, limit: int = 48) -> int | None:
        """The address the lui at insns[k] forms, along the code's branches: the first addiu, ori,
        load or store through `reg` before something writes it, a jr or j ends the way."""
        start = insns[0].addr

        def formed(d):
            if d.rs == reg and (d.op in LOADS_STORES or d.op in ("addiu", "daddiu", "ori")):
                return (value | (d.word & 0xFFFF)) if d.op == "ori" else (value + d.imm) & 0xFFFFFFFF
            return None

        todo = [(k + 1, 0)]
        if k > 0:
            before = insns[k - 1]
            if before.is_branch and before.op not in ("jr", "jalr", "j", "jal"):
                todo.append(((before.target - start) // 4, 0))     # the lui is that branch's delay slot
        seen = set()
        while todo:
            i, n = todo.pop()
            if i in seen or not 0 <= i < len(insns) or n > limit:
                continue
            seen.add(i)
            ins = insns[i]
            s = formed(ins)
            if s is not None:
                return s
            if reg in writes[i]:
                continue
            if ins.op in ("jr", "j", "jal", "jalr"):
                if i + 1 < len(insns):         # its delay slot still runs
                    s = formed(insns[i + 1])
                    if s is not None:
                        return s
                continue
            if ins.is_branch:
                todo.append(((ins.target - start) // 4, n + 1))
                if ins.op not in ("beq",) or ins.rs != ins.rt:
                    todo.append((i + 1, n + 1))
                continue
            todo.append((i + 1, n + 1))
        return None


class CombinedCode:
    """The assembly files' instructions first (their text has the decompilation's own symbols),
    the retail bytes for everything else."""

    def __init__(self, files: dict, retail: RetailCode):
        self.files = files
        self.retail = retail

    def __contains__(self, key) -> bool:
        return self.get(key) is not None

    def get(self, key, default=None):
        hit = self.files.get(key)
        if hit is not None:
            return hit
        return self.retail.get(key, default)


def starts_of(program) -> dict[int, dict[int, str]]:
    """Every function hostgen knows, by program and address: the one with C (or the host's) where
    several names share a place."""
    out: dict[int, dict[int, str]] = {}
    for sym in sorted(program.functions):
        for ov, addr in program.code_places(sym):
            have = out.setdefault(ov, {}).get(addr)
            if have is None or (program.functions.get(have) is not None and program.functions[have].unit is None
                                and program.functions[sym].unit is not None):
                out[ov][addr] = sym
    return out

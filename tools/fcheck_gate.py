#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
The gate for backfilled (functional) C: a function counts as done only when
fcheck says SAME on real calls of the retail game; one fcheck cannot reach is
"unchecked", never done (docs/workflow/BACKFILL.md).

  /opt/homebrew/bin/python3 tools/fcheck_gate.py run                   PAL: check every row of games/rac1/nonmatching/functional.tsv
  /opt/homebrew/bin/python3 tools/fcheck_gate.py run --only func_001F7E98 ...
  /opt/homebrew/bin/python3 tools/fcheck_gate.py report [--strict]     summary of the verdicts already in the tables
  /opt/homebrew/bin/python3 tools/fcheck_gate.py us                    US: record "unchecked (no US oracle)" in port/game/rac1-ntsc/hand/fcheck.tsv

`run` reads the backfill table and drives rac1-decomp's tools/fcheck.py (test-only: it runs the retail
code in the interpreter build of OpenRAC, openrac-fcheck, and compares what
our C leaves in memory and returns). For each row it copies the C into
rac1-decomp's build-sn/fcheck/gate/ (fcheck wants the file under its own
tree and the table's copy is the one that ships), captures call states for
the functions that have none yet in one headless run, then checks the rest in
parallel. A function the boot run never calls, and every level function, is
captured and checked in the levels as well ("Levels" below, and
tools/fcheck_level_build.py). Nothing in rac1-decomp is edited besides its
git-ignored build-sn/.

Verdicts, written to OpenRAC-owned files keyed by function name (the backfill
tables themselves, synced from rac1-decomp or read by hostgen, are never
touched): port/game/rac1-pal/fcheck.tsv for functional.tsv and
port/game/rac1-ntsc/hand/fcheck.tsv for hand.tsv. Columns:

  status  same            fcheck compared real calls and everything matched
          differs         fcheck found a difference (the log says where)
          unchecked       fcheck could not compare (compile failure, every
                          state skipped, no US oracle); the note says why
          no-states       no capture run calls it (title, menu and every level;
                          a level function: its own level)
          not-executable  neither func_XXXXXXXX nor func_LNN_XXXXXXXX (or
                          --no-levels for a level function)
  date    the day it was run (YYYY-MM-DD)
  fcheck  rac1-decomp's commit that fcheck ran from (+dirty if tools/fcheck.py
          had local edits)
  csum    first 12 hex digits of the sha1 of the C file, so a later edit shows
          the verdict as stale (report lists those)
  note    free text: differing lines, the reason for unchecked

Environment: OPENRAC_ROOT is the OpenRAC checkout that has build/runtime/
openrac-fcheck and openrac-boot built (default ~/Projects/RAC/OpenRAC);
OPENRAC_DECOMP is rac1-decomp (default ~/Projects/RAC/rac1-decomp).
Headless only: fcheck opens no window.

  python3 -m unittest tools/test_fcheck_gate.py
"""
from __future__ import annotations

import argparse
import concurrent.futures
import datetime
import hashlib
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PAL_TABLE = ROOT / "games/rac1/nonmatching/functional.tsv"
US_TABLE = ROOT / "port/game/rac1-ntsc/hand/hand.tsv"
# The port's own C for PAL: hostgen places a hand row before a candidate of the same name, so a
# functional.tsv row that is also here is not the C the port runs (its verdict says so).
PAL_HAND = ROOT / "port/game/rac1-pal/hand/hand.tsv"
PAL_VERDICTS = ROOT / "port/game/rac1-pal/fcheck.tsv"
# Functions whose DIFFERS is known not to be a difference of the C (name<TAB>reason), recorded by hand.
PAL_ARTEFACTS = ROOT / "port/game/rac1-pal/fcheck_artefacts.tsv"
US_VERDICTS = ROOT / "port/game/rac1-ntsc/hand/fcheck.tsv"
PYTHON = "/opt/homebrew/bin/python3"
HEADER = "name\tstatus\tdate\tfcheck\tcsum\tnote"
HEAD_COMMENT = ("# fcheck verdicts for the rows of {table} (tools/fcheck_gate.py writes this; "
                "docs/workflow/BACKFILL.md, \"The gate\"). Only status same is done.")
STATUSES = ("same", "differs", "unchecked", "no-states", "not-executable")
EXECUTABLE = re.compile(r"func_[0-9A-F]{8}")
NO_US_ORACLE = "no US oracle (fcheck has no interpreter build for the NTSC-U disc)"


# --- The tables -------------------------------------------------------------------
#
# The backfill tables (functional.tsv, hand.tsv) are read, never written: the
# first is synced from rac1-decomp. The verdicts live in OpenRAC-owned files
# beside them, keyed by function name.

class Backfill:
    """A backfill table: name -> file (two columns, # comments)."""

    def __init__(self, path: Path):
        self.path = path
        self.files: dict[str, str] = {}
        for line in path.read_text().splitlines():
            if line.strip() and not line.startswith("#"):
                c = line.split("\t")
                if len(c) > 1:
                    self.files[c[0].strip()] = c[1].strip()

    def source(self, name: str) -> Path:
        """The C file of a row: relative to the table, or (PAL) to the decompilation."""
        f = self.files[name]
        here = self.path.parent / f
        return here if here.is_file() else self.path.parent.parent / f


class Row:
    """One verdict: name, status, date, fcheck commit, csum, note."""

    def __init__(self, name: str, status: str = "", date: str = "", fcheck: str = "",
                 csum: str = "", note: str = ""):
        self.name, self.status, self.date = name, status, date
        self.fcheck, self.csum, self.note = fcheck, csum, note

    @classmethod
    def parse(cls, line: str) -> "Row":
        c = line.rstrip("\n").split("\t")
        c += [""] * (6 - len(c))
        return cls(*[x.strip() for x in c[:6]])

    def format(self) -> str:
        return "\t".join([self.name, self.status, self.date, self.fcheck, self.csum, self.note.replace("\t", " ")]).rstrip("\t")


class Verdicts:
    """The verdict file for one backfill table, keyed by function name."""

    def __init__(self, path: Path, backfill: Backfill):
        self.path, self.backfill = path, backfill
        self.rows: dict[str, Row] = {}
        if path.is_file():
            for line in path.read_text().splitlines():
                if line.strip() and not line.startswith("#"):
                    r = Row.parse(line)
                    self.rows[r.name] = r

    def row(self, name: str) -> Row:
        return self.rows.setdefault(name, Row(name))

    def ordered(self) -> list[Row]:
        """One row per backfill row, in its order (a verdict without a row is dropped)."""
        return [self.rows.get(n) or Row(n) for n in self.backfill.files]

    def write(self) -> None:
        out = [HEAD_COMMENT.format(table=self.backfill.path.name), "# " + HEADER]
        out += [r.format() for r in self.ordered() if r.status]
        self.path.write_text("\n".join(out) + "\n")


def csum(path: Path) -> str:
    return hashlib.sha1(path.read_bytes()).hexdigest()[:12] if path.is_file() else ""


# --- Running fcheck ---------------------------------------------------------------

def decomp_root() -> Path:
    return Path(os.environ.get("OPENRAC_DECOMP", str(Path.home() / "Projects/RAC/rac1-decomp")))


def fcheck_commit(decomp: Path) -> str:
    def git(*a: str) -> str:
        return subprocess.run(["git", "-C", str(decomp), *a], capture_output=True, text=True).stdout.strip()
    rev = git("rev-parse", "--short=8", "HEAD")
    return rev + ("+dirty" if git("status", "--porcelain", "--", "tools/fcheck.py") else "")


def fcheck_env() -> dict:
    env = dict(os.environ)
    env.setdefault("OPENRAC_ROOT", str(Path.home() / "Projects/RAC/OpenRAC"))
    return env


def classify(rc: int, output: str) -> tuple[str, str]:
    """fcheck's exit code and text -> (status, note). 0 means SAME; 3 the
    function was never called in the capture run; 4 every state was skipped;
    2 the candidate did not build or place; anything else is a difference."""
    lines = [l for l in output.strip().splitlines() if l.strip()]
    if rc == 0:
        m = re.search(r"SAME on (\d+) real calls", output)
        return "same", f"{m.group(1)} calls" if m else ""
    if rc == 3:
        return "no-states", "never called in the capture run (title, menu, level 0)"
    if rc == 4:
        return "unchecked", "every captured state was skipped (nothing compared)"
    if rc == 2:
        return "unchecked", "did not build or place: " + (lines[-1] if lines else "?")[:160]
    diff = [l for l in lines if "SAME" not in l and not l.startswith("compared")]
    return "differs", "; ".join(diff[:2])[:200] or f"exit {rc}"


def artefacts(path: Path = PAL_ARTEFACTS) -> dict[str, str]:
    """name -> why a DIFFERS on it is not the C's doing (a # line is a comment)."""
    out: dict[str, str] = {}
    if path.is_file():
        for line in path.read_text().splitlines():
            if line.strip() and not line.startswith("#"):
                c = line.split("\t", 1)
                out[c[0].strip()] = c[1].strip() if len(c) > 1 else ""
    return out


def waive(name: str, status: str, note: str, known: dict[str, str]) -> tuple[str, str]:
    """A DIFFERS on a function recorded as an artefact is "unchecked", never "same": the note
    says why, then what fcheck printed. Anything else passes through."""
    if status == "differs" and name in known:
        return "unchecked", f"not the C's difference: {known[name]} (fcheck: {note})"[:400]
    return status, note


# --- The test copy of a candidate ----------------------------------------------------
#
# fcheck compiles a candidate inside its source file in rac1-decomp, in place of the
# function's INCLUDE_ASM line or its C. The port compiles the same C file on its own, so what
# the decomp's file already declares can clash with it (a prototype in callees.h, a typedef the
# file has from its own copy of the function). The test copy gets these changes, and only it:
# the file that ships is never touched.

# The host library's functions the port's C calls, as the console's FPU computes them (the
# retail code uses VU0 or FPU instructions there; a call of sqrtf has no retail address).
# memcpy and memset are the game's own (port/game/rac1-pal/libraries.tsv).
PRELUDE = {
    "__gate_sqrtf": 'static __inline__ float __gate_sqrtf(float x) { float r; __asm__ ("sqrt.s %0, %1" : "=f" (r) : "f" (x)); return r; }',
    "__gate_fabsf": 'static __inline__ float __gate_fabsf(float x) { float r; __asm__ ("abs.s %0, %1" : "=f" (r) : "f" (x)); return r; }',
    "func_00115248": "extern void *func_00115248(void *, const void *, unsigned int);",
    "func_001153FC": "extern void *func_001153FC(void *, int, unsigned int);",
}
LIBM = {"sqrtf": "__gate_sqrtf", "__builtin_sqrtf": "__gate_sqrtf", "openrac_sqrt": "__gate_sqrtf",
        "fabsf": "__gate_fabsf", "__builtin_fabsf": "__gate_fabsf",
        "memcpy": "func_00115248", "__builtin_memcpy": "func_00115248",
        "memset": "func_001153FC", "__builtin_memset": "func_001153FC"}
STATIC_FN = re.compile(r"^(static\s+)(?!inline\b|__inline__\b)(?=[\w \t\*]+?\b(\w+)\s*\([^;{]*\)\s*(?:\{|$))", re.M)
# A C99 hexadecimal float (0x1.fffffep127f), which GCC 2.95 does not read.
HEX_FLOAT = re.compile(r"\b0[xX]([0-9a-fA-F]*\.?[0-9a-fA-F]*[pP][+-]?\d+)([fFlL]?)")


def decimal_float(m: re.Match) -> str:
    """A hexadecimal float literal as a decimal one of the same value (17 digits round-trip a
    double; a float suffix rounds it the way the hex literal rounds)."""
    return f"{float.fromhex('0x' + m.group(1)):.17g}" + m.group(2)
CLASH = re.compile(r"(?:conflicting types for|redefinition of) `(?:struct |union |enum )?(\w+)'")


def clashes(log: str) -> set[str]:
    """The names a compile of the test copy failed on as declared twice, differently."""
    return set(CLASH.findall(log))


def prepare(text: str, name: str, renames: set[str] = frozenset()) -> str:
    """The test copy of a candidate: the float library as the console computes it; a static
    helper made inline (fcheck places one function, and a helper has no retail address); each
    name in RENAMES given a name of its own for the copy (NAME__gate: an address-named symbol
    still resolves to the same retail address, fcheck reads the address from the name), and the
    function itself, if it is one of them, defined under that name and exported under its own
    with its size. Every macro is undone at the end, so the rest of the source file is as it was."""
    used = [k for k in LIBM if re.search(rf"\b{k}\b", text)]
    # The copy's own definitions stand for these: the candidate's declarations of them go
    # (their types differ between files: size_t, unsigned long). EE GCC's long is 64 bits and
    # its long long 128, so the host's 64-bit long long is the console's long.
    text = re.sub(rf"^[ \t]*(?:extern[ \t]+)?(?:const[ \t]+)?(?:void|float|double|int|unsigned|char|long)\b"
                  rf"[\w \t\*]*?\b(?:{'|'.join(LIBM)})[ \t]*\([^;{{)]*\)[ \t]*;[ \t]*$", "", text, flags=re.M)
    text = re.sub(r"\blong[ \t]+long\b", "long", text)
    text = HEX_FLOAT.sub(decimal_float, text)
    body = STATIC_FN.sub(lambda m: m.group(1) + "__inline__ " if m.group(2) != name else m.group(0), text)
    head, tail = [], []
    used = [k for k in used if LIBM[k] != name]
    if used:
        head.append("/* fcheck gate (test copy only): the host library's functions the C calls, as the console has them. */")
        head += [PRELUDE[t] for t in dict.fromkeys(LIBM[k] for k in used)]
        for k in used:
            head.append(f"#define {k} {LIBM[k]}")
            tail.append(f"#undef {k}")
    for x in sorted(renames):
        head.append(f"#define {x} {x}__gate")
        tail.append(f"#undef {x}")
    if name in renames:
        # The old assembler writes a .set alias as undefined: a label of its own, aligned the way
        # the compiler aligns a function, right where the renamed definition starts.
        label = (f'__asm__(".text\\n\\t.align 4\\n\\t.globl {name}\\n\\t.type {name},@function\\n'
                 f'{name}:\\n\\t.previous");\n')
        m = re.search(rf"^(?!extern\b)[A-Za-z_][^;\n]*\b{name}\s*\([^;]*?\)\s*\{{", body, re.M)
        if m:
            body = body[:m.start()] + label + body[m.start():]
        tail.append(f'__asm__(".text\\n\\t.size {name}, . - {name}\\n\\t.previous");')
    return "\n".join(head + [body.rstrip("\n")] + tail) + "\n"


def run_fcheck(decomp: Path, name: str, src: Path, log_dir: Path, attempt, tag: str = "") -> tuple[str, str]:
    """Write the test candidate, run ATTEMPT(candidate) -> (exit code, text, compile log path);
    when the compile fails on names declared twice, rename those and try again. TAG tells apart
    the copies of one function checked from several levels at once."""
    cand = decomp / "build-sn/fcheck/gate" / f"{name}{tag}.c"
    cand.parent.mkdir(parents=True, exist_ok=True)
    renames: set[str] = set()
    text, rc, retried = "", 2, False
    for _ in range(5):
        cand.write_text(prepare(src.read_text(errors="replace"), name, renames))
        rc, text, clog = attempt(cand)
        log = clog.read_text(errors="replace") if rc == 2 and clog.is_file() else ""
        new = clashes(log) - renames
        if not new:
            # The assembler under Wine now and then fails to open an included file that is
            # there ("Can't open ... No error") when the machine is busy: once more.
            if "Can't open" in log and not retried:
                retried = True
                continue
            break
        renames |= new
    log_dir.mkdir(parents=True, exist_ok=True)
    (log_dir / f"{name}{tag}.log").write_text(text + (f"\n(test copy renamed: {' '.join(sorted(renames))})\n" if renames else ""))
    return classify(rc, text)


NO_STUB = "neither an INCLUDE_ASM stub nor a C definition"


def run_one(decomp: Path, name: str, src: Path, log_dir: Path) -> tuple[str, str]:
    """fcheck itself on the states of the boot executable's run (title, menu, level 0). A
    function no source file of rac1-decomp has (a libgcc routine, a second entry inside another
    function) is built in a scratch source by the gate's own placement instead."""
    def attempt(cand: Path):
        clog = decomp / "build-sn/fcheck" / name / "log.txt"
        if clog.exists():
            clog.unlink()
        r = subprocess.run([PYTHON, "tools/fcheck.py", name, str(cand.relative_to(decomp))],
                           cwd=decomp, capture_output=True, text=True, env=fcheck_env())
        if r.returncode == 2 and NO_STUB in r.stdout + r.stderr:
            states = sorted((decomp / "build-sn/fcheck/states").glob(f"{name}.*.snap"))
            return placed_check(decomp, name, cand, "exe", states)
        return r.returncode, r.stdout + r.stderr, clog
    return run_fcheck(decomp, name, src, log_dir, attempt)


def capture_missing(decomp: Path, names: list[str]) -> None:
    """One headless capture run for every function with neither states nor a
    "never called" marker (fcheck's own `capture` form)."""
    states = decomp / "build-sn/fcheck/states"
    todo = [n for n in names if not list(states.glob(f"{n}.*.snap")) and not (states / f"{n}.none").exists()]
    if not todo:
        return
    lst = decomp / "build-sn/fcheck/gate_capture_list.txt"
    lst.write_text("\n".join(todo) + "\n")
    print(f"capturing call states for {len(todo)} functions (one headless run) ...", flush=True)
    subprocess.run([PYTHON, "tools/fcheck.py", "capture", str(lst.relative_to(decomp))],
                   cwd=decomp, capture_output=True, text=True, env=fcheck_env())


# --- Levels ------------------------------------------------------------------------
#
# fcheck takes the states of one run (title, menu, level 0) at the executable's addresses. In
# a level, the game's code above 0x15F000 is the level's overlay: the level's own functions
# (func_LNN_) and the executable's functions linked into it at places of their own
# (rac1-decomp's config/overlays/functions.tsv gives each its place in each level). So the gate
# captures states in every level itself, with openrac-boot's --capture at those places (the
# CRC of the level's own bytes there, so only that level's code is taken), into
# build-sn/fcheck/gate/lstates/LNN/, and checks a candidate from them with the symbols resolved
# the way the level resolves them (tools/fcheck_level_build.py, run in rac1-decomp's
# container). A state is the whole machine, the level's code and data with it, so retail runs
# from it as it ran in the game.

LEVELS = tuple(range(19))
RESIDENT_MAX = 0x15F000
LEVEL_FN = re.compile(r"func_L(\d{2})_([0-9A-F]{8})")
# Most levels a function is checked in (the first ones whose run called it).
CHECK_LEVELS = 3
BUILDER = ROOT / "tools/fcheck_level_build.py"
BUILD_TIMEOUT = 1800


def fcheck_module(decomp: Path):
    """rac1-decomp's tools/fcheck.py as a module (its paths, presses and return-type reading)."""
    import importlib.util
    os.environ.setdefault("OPENRAC_ROOT", str(Path.home() / "Projects/RAC/OpenRAC"))
    spec = importlib.util.spec_from_file_location("rac1_fcheck", decomp / "tools/fcheck.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


class Places:
    """Where a function's code is, in the boot executable and in each level."""

    def __init__(self, decomp: Path):
        self.decomp = decomp
        self.catalogue: dict[str, tuple[int, list[tuple[int, int]]]] = {}
        cat = decomp / "config/overlays/functions.tsv"
        for line in cat.read_text().splitlines() if cat.is_file() else []:
            c = line.split("\t")
            if line.startswith("#") or len(c) < 6:
                continue
            self.catalogue[c[0]] = (int(c[2]), [(int(p[:2]), int(p[3:], 16)) for p in c[5].split(",")])
        self._text: dict[int, tuple[int, bytes]] = {}
        self._exe: tuple[int, bytes] | None = None

    def text(self, level: int) -> tuple[int, bytes]:
        if level not in self._text:
            import json
            d = self.decomp / f"baserom/overlays/level_{level:02d}"
            rec = next(r for r in json.loads((d / "manifest.json").read_text())["records"] if r["name"] == "text")
            self._text[level] = (rec["address"], (d / "text.bin").read_bytes())
        return self._text[level]

    def exe(self, address: int, n: int) -> bytes:
        if self._exe is None:
            import struct
            raw = (self.decomp / "baserom/SCES_509.16").read_bytes()
            phoff = struct.unpack_from("<I", raw, 0x1C)[0]
            _t, poff, pvaddr = struct.unpack_from("<III", raw, phoff)
            self._exe = (pvaddr - poff, raw)
        delta, raw = self._exe
        return raw[address - delta:address - delta + n]

    def exe_size(self, name: str) -> int:
        asm = next(self.decomp.glob(f"asm/nonmatchings/*/{name}.s"), None)
        m = re.search(r"^\s*(?:nonmatching|\.size)\s+\w+,\s*(0x[0-9A-Fa-f]+|\d+)", asm.read_text(), re.M) if asm else None
        return int(m.group(1), 0) if m else 16

    def targets(self, name: str, level: int | None) -> list[tuple[int, int, int]]:
        """(address, bytes compared, their CRC-32) of each place of NAME in LEVEL (None: the boot
        executable's run), or [] when it is not there."""
        import zlib

        def at(address: int, code: bytes) -> tuple[int, int, int]:
            k = min(len(code), 256)
            return address, k, zlib.crc32(code[:k])

        m = LEVEL_FN.fullmatch(name)
        if m:
            if level is None or int(m.group(1)) != level:
                return []
            a = int(m.group(2), 16)
            base, text = self.text(level)
            size = self.catalogue.get(name, (64, []))[0]
            return [at(a, text[a - base:a - base + size])]
        a = int(name[5:], 16)
        if level is None or a < RESIDENT_MAX:
            return [at(a, self.exe(a, self.exe_size(name)))]
        size, places = self.catalogue.get(name, (0, []))
        base, text = self.text(level)
        exe = self.exe(a, size)
        return [at(p, text[p - base:p - base + size]) for lv, p in places
                if lv == level and same_code(exe, text[p - base:p - base + size],
                                             lambda x: self.exe(x, 4), lambda x: self.level_word(level, x))]

    def level_word(self, level: int, address: int) -> bytes | None:
        """The four bytes a level loads at ADDRESS (its lit, data and bss records), or None."""
        if ("rec", level) not in self._text:
            import json
            d = self.decomp / f"baserom/overlays/level_{level:02d}"
            recs = []
            for r in json.loads((d / "manifest.json").read_text())["records"]:
                f = d / f"{r['name']}.bin"
                recs.append((r["address"], r["bytes"], f.read_bytes() if f.is_file() and r["type"] != 8 else None))
            self._text[("rec", level)] = recs
        for start, n, blob in self._text[("rec", level)]:
            if start <= address < start + n:
                return bytes(4) if blob is None else blob[address - start:address - start + 4]
        return None


def same_code(a: bytes, b: bytes, word_a=None, word_b=None) -> bool:
    """Whether two copies of a function are the same code but for where their calls and jumps
    go (the linker's) and where a $gp-relative constant sits, when it holds the same value
    (WORD_A, WORD_B read four bytes at an address of each). The catalogue folds routines by a
    fingerprint that leaves out offsets and constants, so one name can stand for a level's
    routine that loads another constant (func_001F9878's places); a state taken there is not a
    call of the function."""
    if len(a) != len(b) or len(a) % 4:
        return False
    sext = lambda v: v - 0x10000 if v & 0x8000 else v
    for i in range(0, len(a), 4):
        x, y = int.from_bytes(a[i:i + 4], "little"), int.from_bytes(b[i:i + 4], "little")
        if x == y or (x >> 26 == y >> 26 and x >> 26 in (2, 3)):
            continue
        # The same load (lw, lwc1) of the same register from $gp, of the same value.
        if word_a and x >> 16 == y >> 16 and x >> 26 in (0x23, 0x31) and (x >> 21) & 31 == 28:
            va, vb = word_a(GP + sext(x & 0xFFFF)), word_b(GP + sext(y & 0xFFFF))
            if va is not None and va == vb:
                continue
        return False
    return True


GP = 0x166D00


def lstates(decomp: Path, level: int | None) -> Path:
    if level is None:
        return decomp / "build-sn/fcheck/states"
    return decomp / f"build-sn/fcheck/gate/lstates/L{level:02d}"


def has_states(decomp: Path, name: str, level: int | None) -> list[Path]:
    return sorted(lstates(decomp, level).glob(f"{name}.*.snap"))


def play(frames: int, start: int = 1500) -> list[str]:
    """Button presses (openrac-boot --press FRAME:BUTTONS:FIELDS) that play a level once it is in:
    walk each way with the direction buttons, jump and double jump, swing the wrench, fire,
    strafe with R1 and L1, now and then quick select, the pause menu and the map, and back. Most
    of what a level's code does happens only when Ratchet moves and acts; a run that stands
    still calls a third of it."""
    out, f, i = [], start, 0
    while f < frames - 300:
        out += [f"{f}:{(0x10, 0x20, 0x40, 0x80)[i % 4]:x}:200", f"{f + 20}:4000:6", f"{f + 60}:8000:6",
                f"{f + 100}:2000:10", f"{f + 140}:4000:6", f"{f + 145}:4000:6"]
        if i % 3 == 1:
            out += [f"{f + 170}:800:8", f"{f + 185}:400:8"]
        if i % 4 == 2:
            out += [f"{f + 190}:1000:6", f"{f + 230}:1000:6"]
        if i % 5 == 3:
            out += [f"{f + 190}:8:6", f"{f + 260}:8:6"]
        if i % 7 == 5:
            out += [f"{f + 190}:1:6", f"{f + 260}:1:6"]
        f, i = f + 300, i + 1
    return out


def capture_level(decomp: Path, places: Places, names: list[str], level: int | None,
                  frames: int, presses: list[str]) -> tuple[int, int]:
    """One headless run of the retail game into LEVEL (None: title, menu and level 0, as fcheck
    plays it), taking up to three states at each listed function's places there. A function the
    run never called gets a "none" marker for that level."""
    fc = fcheck_module(decomp)
    out = lstates(decomp, level)
    out.mkdir(parents=True, exist_ok=True)
    todo = [n for n in names if places.targets(n, level) and not has_states(decomp, n, level)
            and not (out / f"{n}.none").exists()]
    if not todo:
        return 0, 0
    tag = "title" if level is None else f"L{level:02d}"
    lst = decomp / f"build-sn/fcheck/gate/capture_{tag}.txt"
    lst.write_text("".join(f"{a:08x} {k} {crc:08x} {n} 3\n" for n in todo for a, k, crc in places.targets(n, level)))
    cmd = [str(fc.BOOT), str(fc.DISC), "--hooks", str(fc.HOOKS), "--no-card", "--frames", str(frames),
           "--capture", str(lst), str(out)]
    for p in fc.PRESSES + presses + (play(frames) if level is not None else []):
        cmd += ["--press", p]
    if level is not None:
        cmd += ["--write", f"600:15EE84:{level:x}:400"]
    log = decomp / f"build-sn/fcheck/gate/logs/capture_{tag}.log"
    log.parent.mkdir(parents=True, exist_ok=True)
    with open(log, "w") as f:
        subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT, env=fcheck_env())
    got = 0
    for n in todo:
        if has_states(decomp, n, level):
            got += 1
        else:
            (out / f"{n}.none").write_text(f"not called in the capture run of {tag}\n")
    return got, len(todo)


def capture_levels(decomp: Path, names: list[str], levels, frames: int, presses: list[str], jobs: int) -> None:
    places = Places(decomp)
    print(f"capturing call states in {len(levels)} runs ({jobs} at once) ...", flush=True)
    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        futs = {pool.submit(capture_level, decomp, places, names, lv, frames, presses): lv for lv in levels}
        for f in concurrent.futures.as_completed(futs):
            lv = futs[f]
            got, asked = f.result()
            print(f"  {'title' if lv is None else f'level {lv:2d}'}: {got} of {asked} functions called", flush=True)


def placed_check(decomp: Path, name: str, cand: Path, level, states: list[Path]) -> tuple[int, str, Path]:
    """Build CAND for LEVEL ("exe": the boot executable's addresses) with
    tools/fcheck_level_build.py in rac1-decomp's container, then run openrac-fcheck from
    STATES as fcheck does. Returns fcheck's exit code, its text and the compile log."""
    fc = fcheck_module(decomp)
    tag = level if level == "exe" else f"L{int(level):02d}"
    work = decomp / f"build-sn/fcheck/gate/work/{name}.{tag}"
    work.mkdir(parents=True, exist_ok=True)
    out = work / "placed.json"
    clog = work / "log.txt"
    for p in (out, clog):
        if p.exists():
            p.unlink()
    if not states:
        return 3, f"{name}: NOT CALLED in the capture run of {tag}", clog
    builder = decomp / "build-sn/fcheck/gate/fcheck_level_build.py"
    if not builder.is_file() or builder.read_bytes() != BUILDER.read_bytes():
        shutil.copyfile(BUILDER, builder)
    rel = lambda p: str(Path(p).resolve().relative_to(decomp.resolve()))
    try:
        r = subprocess.run(["bash", "tools/docker/run.sh", "python3", rel(builder), name, rel(cand), rel(out),
                            str(level)], cwd=decomp, capture_output=True, text=True, timeout=BUILD_TIMEOUT)
    except subprocess.TimeoutExpired:
        # The compiler under Wine has hung once in a busy machine; its container is left to
        # docker's --rm when it is stopped (`docker ps`).
        return 2, f"the build did not finish within {BUILD_TIMEOUT} s", clog
    if r.returncode or not out.exists():
        return 2, "\n".join((r.stdout + r.stderr).strip().splitlines()[-25:]), clog
    import json
    placed = json.loads(out.read_text())
    cmd = [str(fc.FCHECK), "--entry", f"{placed['entry']:x}", "--returns", fc.returns(name, cand.read_text(errors="replace"))]
    for f, a in placed["segments"]:
        cmd += ["--code", f"{work / f}@{a:x}"]
    cmd += [str(s) for s in states]
    r = subprocess.run(cmd, capture_output=True, text=True)
    text = r.stdout.replace(str(states[0].parent) + "/", "").strip() or r.stderr.strip()
    if r.returncode == 0:
        m = re.search(r"compared (\d+) of", r.stdout)
        text += f"\n{name}: SAME on {m.group(1) if m else '?'} real calls"
    return r.returncode, text, clog


def run_levels(decomp: Path, name: str, src: Path, log_dir: Path) -> list[tuple[str, str, str]]:
    """NAME checked from the states of the first CHECK_LEVELS levels whose run called it:
    [(level, status, note)]."""
    m = LEVEL_FN.fullmatch(name)
    levels = [int(m.group(1))] if m else list(LEVELS)
    out = []
    for lv in levels:
        states = has_states(decomp, name, lv)
        if not states:
            continue
        rc_text = run_fcheck(decomp, name, src, log_dir,
                             lambda cand, lv=lv, states=states: placed_check(decomp, name, cand, lv, states),
                             tag=f".L{lv:02d}")
        out.append((f"level {lv:02d}", *rc_text))
        if len(out) >= CHECK_LEVELS:
            break
    return out


def combine(name: str, title: tuple[str, str] | None, levels: list[tuple[str, str, str]]) -> tuple[str, str]:
    """One verdict from the boot run's and the levels': a difference anywhere is one; else SAME
    where anything compared; else why nothing could."""
    runs = ([("title", *title)] if title else []) + levels
    for where, s, note in runs:
        if s == "differs":
            return "differs", f"{where}: {note}"
    same = [(w, n) for w, s, n in runs if s == "same"]
    if same:
        return "same", ", ".join(f"{n} ({w})" for w, n in same)
    for where, s, note in runs:
        if s == "unchecked":
            return "unchecked", f"{where}: {note}"
    if LEVEL_FN.fullmatch(name):
        return "no-states", f"never called in level {name[6:8]}'s capture run"
    return "no-states", "never called in the capture runs (title, menu and every level)"


def summary(rows: list[Row]) -> str:
    counts = {s: 0 for s in STATUSES}
    counts[""] = 0
    for r in rows:
        counts[r.status if r.status in STATUSES else ""] += 1
    parts = [f"{counts[s]} {s}" for s in STATUSES] + ([f"{counts['']} no verdict"] if counts[""] else [])
    return f"{len(rows)} rows: " + ", ".join(parts)


def check_row(decomp: Path, name: str, src: Path, log_dir: Path, levels: bool) -> tuple[str, str]:
    """A row's verdict: an executable function from the boot run first (fcheck itself), then,
    when that run never called it, from the levels'; a level's function from its own level."""
    title = None
    if EXECUTABLE.fullmatch(name):
        title = run_one(decomp, name, src, log_dir)
        if title[0] != "no-states" or not levels:
            return title
    elif not levels:
        return "not-executable", "level/overlay function: run with --levels to check it from its level's states"
    return combine(name, None if title and title[0] == "no-states" else title, run_levels(decomp, name, src, log_dir))


def cmd_run(args: argparse.Namespace) -> int:
    decomp = decomp_root()
    back = Backfill(PAL_TABLE)
    table = Verdicts(PAL_VERDICTS, back)
    commit = fcheck_commit(decomp)
    today = datetime.date.today().isoformat()
    # --try NAME=FILE: check other C for a row (a fix before it lands); the table is not written.
    trial = {k: Path(v).resolve() for k, v in (x.split("=", 1) for x in args.trial)}
    if trial:
        args.dry_run = True
        args.only = sorted(set(args.only or []) | set(trial))
    names = [n for n in back.files if not args.only or n in args.only]
    work: list[Row] = []
    for n in names:
        r = table.row(n)
        src = trial.get(n) or back.source(n)
        r.csum, r.date, r.fcheck = csum(src), today, commit
        if not EXECUTABLE.fullmatch(n) and not LEVEL_FN.fullmatch(n):
            r.status, r.note = "not-executable", "neither an executable nor a level function (func_XXXXXXXX, func_LNN_XXXXXXXX)"
        elif not src.is_file():
            r.status, r.note = "unchecked", f"file {back.files[n]} is missing"
        else:
            work.append(r)
    known = artefacts()
    hand = Backfill(PAL_HAND) if PAL_HAND.is_file() else Backfill.__new__(Backfill)
    hand.files = getattr(hand, "files", {})
    exe = [r.name for r in work if EXECUTABLE.fullmatch(r.name)]
    capture_missing(decomp, exe)
    # What fcheck's own capture cannot take (no assembly of its own: a second entry point).
    capture_levels(decomp, [n for n in exe if not has_states(decomp, n, None)
                            and not (lstates(decomp, None) / f"{n}.none").exists()], [None],
                   args.frames, args.press, 1)
    if args.levels:
        lv_names = [r.name for r in work if LEVEL_FN.fullmatch(r.name)
                    or not has_states(decomp, r.name, None)]
        capture_levels(decomp, lv_names, list(LEVELS), args.frames, args.press, args.capture_jobs)
    log_dir = decomp / "build-sn/fcheck/gate/logs"
    print(f"checking {len(work)} functions with {args.jobs} workers ...", flush=True)
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        futs = {pool.submit(check_row, decomp, r.name, trial.get(r.name) or back.source(r.name), log_dir, args.levels): r for r in work}
        for i, f in enumerate(concurrent.futures.as_completed(futs), 1):
            r = futs[f]
            try:
                r.status, r.note = waive(r.name, *f.result(), known)
            except Exception as e:  # a crash of the harness is not a verdict on the C
                r.status, r.note = "unchecked", f"gate error: {e}"[:160]
            if r.name in hand.files:
                r.note = f"{r.note}; the port runs hand/{hand.files[r.name]} instead (hostgen's hand row comes first)"
            print(f"[{i}/{len(work)}] {r.name}: {r.status} {r.note}", flush=True)
    if not args.dry_run:
        table.write()
    print(summary(table.ordered()))
    listing(table.ordered())
    return 0


def listing(rows: list[Row]) -> None:
    for status in ("differs", "unchecked", "no-states"):
        names = [r for r in rows if r.status == status]
        if names:
            print(f"\n{status} ({len(names)}):")
            for r in names:
                print(f"  {r.name}  {r.note}")


def cmd_us(args: argparse.Namespace) -> int:
    back = Backfill(US_TABLE)
    table = Verdicts(US_VERDICTS, back)
    today = datetime.date.today().isoformat()
    for n in back.files:
        r = table.row(n)
        r.status, r.date, r.fcheck, r.csum, r.note = "unchecked", today, "", csum(back.source(n)), NO_US_ORACLE
    if not args.dry_run:
        table.write()
    print(summary(table.ordered()))
    return 0


def cmd_report(args: argparse.Namespace) -> int:
    """Join each backfill table with its verdict file."""
    bad = 0
    for tab, ver in ((PAL_TABLE, PAL_VERDICTS), (US_TABLE, US_VERDICTS)):
        back = Backfill(tab)
        rows = Verdicts(ver, back).ordered()
        print(f"{tab.relative_to(ROOT)} + {ver.relative_to(ROOT)}: {summary(rows)}")
        stale = [r for r in rows if r.csum and r.csum != csum(back.source(r.name))]
        for r in stale:
            print(f"  stale (file changed since the verdict): {r.name}")
        for r in rows:
            if r.status == "differs":
                print(f"  differs: {r.name}  {r.note}")
        bad += sum(1 for r in rows if r.status in ("", "differs")) + len(stale)
    return 1 if args.strict and bad else 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("run", help="PAL: run fcheck on every row and write the verdicts")
    p.add_argument("--only", nargs="*", help="check these functions only")
    p.add_argument("--try", dest="trial", action="append", default=[], metavar="NAME=FILE",
                   help="check FILE as NAME's C instead of the table's (implies --dry-run)")
    p.add_argument("-j", "--jobs", type=int, default=max(2, (os.cpu_count() or 4) // 2))
    p.add_argument("--dry-run", action="store_true", help="do not write the table")
    p.add_argument("--no-levels", dest="levels", action="store_false",
                   help="the boot run's states only (no level captures, level functions not checked)")
    p.add_argument("--frames", type=int, default=7000, help="frames of each capture run (default 7000)")
    p.add_argument("--press", action="append", default=[], metavar="FRAME:BUTTONS[:FRAMES]",
                   help="a button press added to every capture run, after fcheck's own (openrac-boot --press)")
    p.add_argument("--capture-jobs", type=int, default=6, help="capture runs at once (default 6)")
    p.set_defaults(fn=cmd_run)
    p = sub.add_parser("us", help="US: record unchecked (no US oracle)")
    p.add_argument("--dry-run", action="store_true")
    p.set_defaults(fn=cmd_us)
    p = sub.add_parser("report", help="summarise the verdicts in both tables")
    p.add_argument("--strict", action="store_true", help="exit 1 while any row has no verdict, differs, or is stale")
    p.set_defaults(fn=cmd_report)
    args = ap.parse_args()
    return args.fn(args)


if __name__ == "__main__":
    sys.exit(main())

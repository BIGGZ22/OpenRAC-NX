#!/usr/bin/env python3
"""
Try C for one function without touching src/: compile it in a scratch copy
of its source file through the same per-segment pipeline Makefile.sn uses,
and compare the function against retail.

  python tools/try_func.py func_XXXXXXXX candidate.c         # verdict
  python tools/try_func.py func_XXXXXXXX candidate.c --diff  # + differing words
  python tools/try_func.py func_XXXXXXXX c1.c c2.c c3.c      # one verdict each

candidate.c holds the function definition, plus any extern declarations it
needs that the source file does not already have; it replaces the
function's INCLUDE_ASM line, or, for a function already written in C (a
near-miss being refined), its current definition. Work happens in
build-sn/try/<func>/.

For an executable function the comparison fills every relocated field (a
call target, a %hi/%lo half, a $gp offset) with retail's address of its
symbol (config/symbol_addrs.txt, or the address in the name), so a wrong
symbol or addend shows; only fields it cannot resolve, references into the
object's own .rodata or .data, stay masked. It needs no link, so it is
still a filter: a function that passes here has to pass the real build
(bash tools/build_sn.sh) before it counts. Level functions go through
tools/overlay_check.py, which places and resolves everything.

Verdicts: EXACT (or EXACT (n bytes masked)), BYTES n/size (same size, n
bytes differ), SIZE ours/retail (a size mismatch: never keep one), COMPILE
(see log.txt).

Every run is logged to build-sn/try/<func>/runs.log. If that folder holds a
BUDGET file (a number, written by tools/wave.py), runs stop once that many
have been logged. --no-budget skips both, for tools that re-check results.
--arm=NAME works in build-sn/try/<func>/NAME/ instead, with its own
runs.log and BUDGET: two agents trying the same function side by side (a
model comparison) each build in their own folder.
"""
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from libgcc_units import SEGMENT_SOURCES, ee29_sources  # noqa: E402
from toolchain import sn  # noqa: E402
import file_cflags  # noqa: E402
import overlay_check  # noqa: E402

import rabbitizer as rz  # noqa: E402
from elftools.elf.elffile import ELFFile  # noqa: E402
from elftools.elf.relocation import RelocationSection  # noqa: E402

CC = "toolchain/sn-prodg-24/local/sce/ee/gcc/bin/ee-gcc2953.exe"
# Sony SDK sources built with the SDK's 2.9-ee (Makefile.sn's EE29_CORE: the
# objects marked `ee29` in config/core_text.objects).
CC29 = "toolchain/sn-prodg-24/local/sce/ee/gcc/bin/ee-gcc.exe"
EE29_INC = "-Itoolchain/sn-prodg-24/local/sce/ee/gcc/lib/gcc-lib/ee/2.9-ee-991111/include"
EE29_SOURCES = ee29_sources()
CFLAGS = ["-O2", "-G2", "-Iinclude", "-Wa,-I,."] + os.environ.get("TRY_CFLAGS", "").split()  # extra flags for experiments
BASEROM = "baserom/SCES_509.16"
STUB = re.compile(r'^\s*(?:INCLUDE_ASM|ASM_FUNC)\([^)]*\b(func_[0-9A-Fa-f]{8})\)')
SIZE = re.compile(r"nonmatching\s+(func_[0-9A-Fa-f]{8}),\s*(0x[0-9A-Fa-f]+)")


DEF = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b(func_[0-9A-Fa-f]{8})(?:_r)?\s*\(")

# Overlay functions (docs/OVERLAYS.md): func_LNN_XXXXXXXX, checked through
# overlay_check instead of the masked compare() below. Their sources live
# under src/overlays/ (any subdirectory), not in SEGMENT_SOURCES.
OVERLAY_NAME = re.compile(r"^func_L\d{2}_[0-9A-Fa-f]{8}$")
OVERLAY_STUB = re.compile(r"^\s*INCLUDE_ASM\([^)]*\b(func_L\d{2}_[0-9A-Fa-f]{8})\)")
OVERLAY_DEF = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b(func_L\d{2}_[0-9A-Fa-f]{8})(?:_r)?\s*\(")


def find_overlay_stub(name):
    """Like find_stub, but over every src/overlays/**/*.c file."""
    for src in sorted(Path("src/overlays").rglob("*.c")):
        lines = src.read_text(errors="replace").splitlines()
        for i, line in enumerate(lines):
            m = OVERLAY_STUB.match(line)
            if m and m.group(1) == name:
                return "text", src, i, i
        for i, line in enumerate(lines):
            m = OVERLAY_DEF.match(line)
            if m and m.group(1) == name and not line.rstrip().endswith(";"):
                depth, seen = 0, False
                for j in range(i, len(lines)):
                    depth += lines[j].count("{") - lines[j].count("}")
                    seen = seen or "{" in lines[j]
                    if seen and depth == 0:
                        return "text", src, i, j
    sys.exit(f"{name}: neither an INCLUDE_ASM stub nor a C definition under src/overlays/")


def find_stub(name):
    """(segment, source, first line, last line) of NAME's INCLUDE_ASM line,
    or of its C definition when it is already decompiled."""
    if OVERLAY_NAME.match(name):
        return find_overlay_stub(name)
    for seg, srcs in SEGMENT_SOURCES.items():
        for src in srcs:
            lines = Path(src).read_text(errors="replace").splitlines()
            for i, line in enumerate(lines):
                m = STUB.match(line)
                if m and m.group(1) == name:
                    return seg, Path(src), i, i
            for i, line in enumerate(lines):
                m = DEF.match(line)
                if m and m.group(1) == name and not line.rstrip().endswith(";"):
                    depth, seen = 0, False
                    for j in range(i, len(lines)):
                        depth += lines[j].count("{") - lines[j].count("}")
                        seen = seen or "{" in lines[j]
                        if seen and depth == 0:
                            return seg, Path(src), i, j
    sys.exit(f"{name}: neither an INCLUDE_ASM stub nor a C definition in src/")


def run(cmd, log):
    r = subprocess.run(cmd, capture_output=True, text=True)
    log.write(" ".join(cmd) + "\n" + r.stdout + r.stderr)
    return r.returncode == 0


def build(name, seg, src, first, last, candidate, work):
    """The Makefile.sn recipe for the segment, on a scratch copy."""
    lines = src.read_text(errors="replace").splitlines()
    lines[first:last + 1] = [candidate.rstrip("\n")]
    work.mkdir(parents=True, exist_ok=True)
    c = work / "src.c"
    c.write_text("\n".join(lines) + "\n")
    obj = work / "obj.o"
    obj.unlink(missing_ok=True)
    # The source file's own extra flags (config/file_cflags.txt), as Makefile.sn passes them.
    # No step between the compiler and the assembler changes what the compiler wrote, apart
    # from the assembler behaviours listed in docs/BUILD_FIDELITY.md.
    extra = file_cflags.flags_for(src)
    with open(work / "log.txt", "w") as log:
        s = [work / f"{n}.s" for n in "abcd"]
        if str(src) in EE29_SOURCES:
            if not run(sn(CC29, *CFLAGS, *extra, EE29_INC, "-S", "-o", str(s[0]), str(c)), log):
                return None
            shutil.copy(s[0], s[3])
            if not run([sys.executable, "tools/fix_volatile_slot.py", str(s[3]), str(s[3])], log):
                return None
            if not run([sys.executable, "tools/check_macro_slots.py", str(s[3])], log):
                return None
            if not run(sn(CC, *CFLAGS, "-c", str(s[3]), "-o", str(obj)), log):
                return None
            return obj
        if not run(sn(CC, *CFLAGS, *extra, "-S", "-o", str(s[0]), str(c)), log):
            return None
        shutil.copy(s[0], s[2])
        if src.name in ("989snd.c", "wad.c"):
            if not run([sys.executable, "tools/fix_macro_load_delay.py", str(s[2]), str(s[2])], log):
                return None
        if seg == "text":
            if not run([sys.executable, "tools/fix_jump_tables.py", str(s[2]), str(s[2])], log):
                return None
            if not run([sys.executable, "tools/ps2eeas_dli.py", str(s[2]), str(s[2])], log):
                return None
        if not run([sys.executable, "tools/check_macro_slots.py", str(s[2])], log):
            return None
        if seg == "text":
            if not run([sys.executable, "tools/fix_orphan_hi.py", str(s[2]), str(s[2])], log):
                return None
            first = work / "c.o"
            if not run(sn(CC, *CFLAGS, "-c", str(s[2]), "-o", str(first)), log):
                return None
            if not run([sys.executable, "tools/ps2eeas_nops.py", str(s[2]), str(first), str(s[3])], log):
                return None
        elif src.name in ("989snd.c", "wad.c"):
            first = work / "c.o"
            if not run(sn(CC, *CFLAGS, "-c", str(s[2]), "-o", str(first)), log):
                return None
            if not run([sys.executable, "tools/ps2eeas_nops.py", str(s[2]), str(first), str(s[3])], log):
                return None
        else:
            shutil.copy(s[2], s[3])
        if not run(sn(CC, *CFLAGS, "-c", str(s[3]), "-o", str(obj)), log):
            return None
    return obj


GP = 0x166D00
_ADDRS = None


def symbol_address(name):
    """A symbol's retail address: config/symbol_addrs.txt, then the address a
    func_/D_/jtbl_ name carries (an alias may add a _suffix), then _gp."""
    global _ADDRS
    if _ADDRS is None:
        _ADDRS = {m.group(1): int(m.group(2), 16) for m in re.finditer(
            r"^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", Path("config/symbol_addrs.txt").read_text(), re.M)}
    if name in _ADDRS:
        return _ADDRS[name]
    if name == "_gp":
        return GP
    m = re.match(r"^(?:D|func|jtbl)_([0-9A-Fa-f]{8})(?:_\w+)?$", name)
    return int(m.group(1), 16) if m else None


def resolve_relocations(elf, text, text_base):
    """The object's relocated words filled in with retail's addresses, and the
    byte offsets of the fields it cannot resolve (references into the object's
    own .rodata or .data, or an unpaired %hi). Masking every relocated field
    instead hid real differences: two $gp stores swapped with each other differ
    only in their GPREL16 offsets (found by rac3-uya-decomp, whose try_func.py
    this follows). TEXT_BASE is where the object's .text sits in retail."""
    word = lambda o: int.from_bytes(text[o:o + 4], "little")
    sext = lambda v: v - 0x10000 if v & 0x8000 else v
    symtab = list(elf.get_section_by_name(".symtab").iter_symbols())
    relocs = sorted((r for s in elf.iter_sections()
                     if isinstance(s, RelocationSection) and s.name == ".rel.text"
                     for r in s.iter_relocations()), key=lambda r: r["r_offset"])
    resolved, masked, pending = {}, set(), []

    def mask(o, t):
        masked.update((o, o + 1) if t in (5, 6, 7) else range(o, o + 4))

    for r in relocs:
        o, t = r["r_offset"], r["r_info_type"]
        sym = symtab[r["r_info_sym"]]
        if sym["st_info"]["type"] == "STT_SECTION":
            secname = elf.get_section(sym["st_shndx"]).name if isinstance(sym["st_shndx"], int) else ""
            addr = text_base if secname == ".text" else None
        else:
            addr = symbol_address(sym.name)
        key = sym.name or f"section{sym['st_shndx']}"
        ins = word(o)
        if addr is None or t not in (2, 4, 5, 6, 7):
            mask(o, t)
            continue
        if t == 2:      # R_MIPS_32
            resolved[o] = (addr + ins) & 0xFFFFFFFF
        elif t == 4:    # R_MIPS_26: the target's address, with the in-place addend
            resolved[o] = (ins & 0xFC000000) | (((addr + ((ins & 0x3FFFFFF) << 2)) >> 2) & 0x3FFFFFF)
        elif t == 5:    # R_MIPS_HI16: resolved at the LO16 that follows it
            pending.append((o, key, ins))
        elif t == 6:    # R_MIPS_LO16
            lo = sext(ins & 0xFFFF)
            for ho, _, hins in [p for p in pending if p[1] == key]:
                full = addr + ((hins & 0xFFFF) << 16) + lo
                resolved[ho] = (hins & 0xFFFF0000) | (((full + 0x8000) >> 16) & 0xFFFF)
            pending = [p for p in pending if p[1] != key]
            resolved[o] = (ins & 0xFFFF0000) | ((addr + lo) & 0xFFFF)
        else:           # R_MIPS_GPREL16
            resolved[o] = (ins & 0xFFFF0000) | ((addr + sext(ins & 0xFFFF) - GP) & 0xFFFF)
    for ho, _, _ in pending:
        mask(ho, 5)
    return resolved, masked


def compare(name, seg, obj, show):
    rsize = int(SIZE.search(Path(f"asm/nonmatchings/{seg}/{name}.s").read_text()).group(2), 16)
    raw = Path(BASEROM).read_bytes()
    relf = ELFFile(open(BASEROM, "rb"))
    load = next(s for s in relf.iter_segments() if s["p_type"] == "PT_LOAD")
    delta = load["p_vaddr"] - load["p_offset"]
    elf = ELFFile(open(obj, "rb"))
    text = elf.get_section_by_name(".text").data()
    sym = next((s for s in elf.get_section_by_name(".symtab").iter_symbols() if s.name == name), None)
    if sym is None or not sym["st_size"]:
        return f"NOSYM {name} not defined by the candidate"
    off, osize = sym["st_value"], sym["st_size"]
    vram = int(name[5:], 16)
    resolved, masked = resolve_relocations(elf, text, vram - off)
    filled = bytearray(text)
    for o, w in resolved.items():
        filled[o:o + 4] = w.to_bytes(4, "little")
    relocated = {b: True for b in masked}
    ours = bytes(filled[off:off + osize])
    orig = raw[vram - delta:vram - delta + rsize]
    if osize != rsize:
        verdict = f"SIZE ours {osize} / retail {rsize}"
    else:
        diff = sum(1 for i in range(rsize) if not relocated.get(off + i) and ours[i] != orig[i])
        hidden = sum(1 for i in range(rsize) if relocated.get(off + i))
        verdict = (f"EXACT ({hidden} bytes masked)" if hidden else "EXACT") if diff == 0 else f"BYTES {diff}/{rsize}"
    if show and not verdict.startswith("EXACT"):
        n = max(osize, rsize)
        for i in range(0, n, 4):
            a = int.from_bytes(ours[i:i + 4], "little") if i < osize else None
            b = int.from_bytes(orig[i:i + 4], "little") if i < rsize else None
            # Differences only inside relocated fields are not differences.
            real = a is None or b is None or any(
                ours[i + k] != orig[i + k] and not relocated.get(off + i + k) for k in range(4))
            if a != b and real:
                da = rz.Instruction(a, vram=vram + i, category=rz.InstrCategory.R5900).disassemble() if a is not None else "-"
                db = rz.Instruction(b, vram=vram + i, category=rz.InstrCategory.R5900).disassemble() if b is not None else "-"
                print(f"  +{i:4x}  ours {da:40s} retail {db}")
    return verdict


def budget_check(work, count):
    """Stop a worker that has used its BUDGET of runs.

    Returns (runs so far, the budget or None)."""
    runs = work / "runs.log"
    used = len(runs.read_text().splitlines()) if runs.exists() else 0
    budget = work / "BUDGET"
    limit = int(budget.read_text().split()[0]) if budget.exists() else None
    if limit is not None and used + count > limit:
        sys.exit(f"BUDGET: {used} of {limit} runs used for {work.name}. "
                 "Write RESULT.md and NOTES.md now and stop.")
    return used, limit


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) < 2:
        sys.exit(__doc__)
    name, cands = args[0], args[1:]
    counted = "--no-budget" not in sys.argv
    arm = next((a.split("=", 1)[1] for a in sys.argv if a.startswith("--arm=")), "")
    work = Path("build-sn/try") / name / arm if arm else Path("build-sn/try") / name
    used, limit = budget_check(work, len(cands)) if counted else (0, None)
    seg, src, first, last = find_stub(name)
    is_overlay = bool(OVERLAY_NAME.match(name))
    failed = False
    for cand in cands:
        # Several candidates: label each line, and keep going past failures.
        label = f"{Path(cand).name:10s} " if len(cands) > 1 else ""
        obj = build(name, seg, src, first, last, Path(cand).read_text(), work)
        if obj is None:
            verdict = "COMPILE failed"
        elif is_overlay:
            verdict = overlay_check.check(obj, name, '--diff' in sys.argv)
        else:
            verdict = compare(name, seg, obj, '--diff' in sys.argv)
        if counted:
            used += 1
            with open(work / "runs.log", "a") as runs:
                runs.write(f"{Path(cand).name} {verdict}\n")
        tally = f"   [run {used} of {limit}]" if limit else ""
        if obj is None:
            log = (work / "log.txt").read_text(errors="replace")
            errs = [l for l in log.splitlines() if "error" in l.lower() or "undeclared" in l or "parse" in l]
            print(f"{label}{name}: COMPILE failed ({src}){tally}")
            for l in errs[:8]:
                print("   ", l)
            failed = True
            continue
        print(f"{label}{name}: {verdict}   ({src}){tally}")
    if failed:
        sys.exit(1)


if __name__ == "__main__":
    main()

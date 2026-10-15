#!/usr/bin/env python3
"""
Does a candidate do what the retail function does? A check by behaviour, for
the native port: the candidate need not match a byte, only leave the same
memory, result and registers as retail from real calls.

  /opt/homebrew/bin/python3 tools/fcheck.py FUNC CANDIDATE.c [--ulps N]
  /opt/homebrew/bin/python3 tools/fcheck.py capture LIST.txt [--frames N] [--level N]

The first form builds the candidate in its file as tools/try_func.py does
(through Docker, the retail compiler), places it in memory that the game
does not use (0x000A0000, the kernel's area on the console) with its own
.rodata and .data, resolves every relocation against retail's addresses, and
runs OpenRAC's `openrac-fcheck` on the call states captured for FUNC: from
each, retail runs in the interpreter, then the candidate, and what they left
is compared (memory but the dead stack, the scratchpad, the result by the
candidate's return type, the registers a function keeps, and the order of
device accesses and system calls). It prints SAME or the differences.

States come from a run of the retail game (`openrac-boot --capture`): the
`capture` form takes a list of function names, one per line, plays the PAL
disc headless into level 0 (or `--level N`) and writes up to three states
per function to build-sn/fcheck/states/. A function the run never calls has
no states and cannot be checked this way. The first form captures FUNC alone
when it has no states yet (about a minute).

Executable functions only for now (func_XXXXXXXX). Needs OpenRAC built
(cmake --build build/runtime in ~/Projects/OpenRAC) and the PAL disc image.
"""
from __future__ import annotations

import json
import re
import subprocess
import sys
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OPENRAC = Path.home() / "Projects/OpenRAC"
BOOT = OPENRAC / "build/runtime/openrac-boot"
FCHECK = OPENRAC / "build/runtime/openrac-fcheck"
DISC = OPENRAC / "baserom/SCES_509.16.iso"
HOOKS = OPENRAC / "runtime/games/SCES_509.16.hooks"
WORK = ROOT / "build-sn/fcheck"
STATES = WORK / "states"
# OpenRAC dropped runtime/ with its emulator; the hooks file is kept beside the states.
if not HOOKS.exists():
    HOOKS = WORK / "SCES_509.16.hooks"
BASE = 0x000A0000          # where the candidate goes: below the game, unused by it
LIMIT = 0x000F0000
# Into level 0 with no memory card: cross past the warning, start, new game.
PRESSES = ["100:4000:5", "450:8:5", "520:4000:5", "620:4000:5"]


# --- In Docker: build and place -------------------------------------------------

def build(name: str, candidate: Path, out: Path) -> None:
    sys.path.insert(0, str(ROOT / "tools"))
    import try_func
    from elftools.elf.elffile import ELFFile
    from elftools.elf.relocation import RelocationSection

    seg, src, first, last = try_func.find_stub(name)
    obj = try_func.build(name, seg, src, first, last, candidate.read_text(errors="replace"), out.parent)
    if obj is None:
        sys.exit("COMPILE failed (see the compiler's messages above)")
    elf = ELFFile(open(obj, "rb"))
    symtab = elf.get_section_by_name(".symtab")
    syms = list(symtab.iter_symbols())
    me = next((s for s in syms if s.name == name and s["st_size"]), None)
    if me is None:
        sys.exit(f"LINK {name} is not defined by the candidate")
    text_idx = me["st_shndx"]
    off, size = me["st_value"], me["st_size"]

    # Lay out: our function, then every data section of the file.
    place = {}          # section index -> address of its start
    blobs = []          # (address, bytes)
    text = bytearray(elf.get_section(text_idx).data())
    place[text_idx] = BASE - off
    at = (BASE + size + 15) & ~15
    for idx, sec in enumerate(elf.iter_sections()):
        if sec.name in (".rodata", ".data", ".sdata", ".lit4", ".lit8", ".bss", ".sbss") and sec["sh_size"]:
            place[idx] = at
            data = bytearray(sec["sh_size"]) if sec["sh_type"] == "SHT_NOBITS" else bytearray(sec.data())
            blobs.append([at, data, sec.name, idx])
            at = (at + len(data) + 15) & ~15
    if at > LIMIT:
        sys.exit("LINK the candidate's file has more data than fits below 0x000F0000")

    funcs = [s for s in syms if s["st_shndx"] == text_idx and s["st_size"] and s["st_info"]["type"] == "STT_FUNC"]

    def text_address(toff: int) -> int:
        """Offset in our .text: inside our function, the placed copy; inside
        another function of the file, that function's retail address."""
        if off <= toff < off + size:
            return BASE + toff - off
        for f in funcs:
            if f["st_value"] <= toff < f["st_value"] + f["st_size"]:
                a = try_func.symbol_address(f.name)
                if a is None:
                    raise SystemExit(f"LINK {f.name} (same file) has no retail address")
                return a + toff - f["st_value"]
        raise SystemExit(f"LINK a reference into .text at {toff:#x} that no function owns")

    def symbol(sym) -> tuple[int, bool]:
        """A relocation's symbol: its address, and whether the addend is a
        .text offset (section symbol of .text)."""
        if sym["st_info"]["type"] == "STT_SECTION":
            idx = sym["st_shndx"]
            if idx == text_idx:
                return 0, True
            if idx in place:
                return place[idx], False
            raise SystemExit(f"LINK a reference to section {elf.get_section(idx).name}, not placed")
        a = try_func.symbol_address(sym.name)
        if a is not None:
            return a, False
        if isinstance(sym["st_shndx"], int) and sym["st_shndx"] in place:
            if sym["st_shndx"] == text_idx:
                return text_address(sym["st_value"]), False
            return place[sym["st_shndx"]] + sym["st_value"], False
        raise SystemExit(f"LINK {sym.name} has no retail address")

    sext = lambda v: v - 0x10000 if v & 0x8000 else v

    def apply(sec_idx: int, buf: bytearray, lo_bound: int, hi_bound: int, base_of) -> None:
        relname = ".rel" + elf.get_section(sec_idx).name
        rel = elf.get_section_by_name(relname)
        if rel is None:
            return
        pending = []
        orig = bytes(buf)
        for r in sorted(rel.iter_relocations(), key=lambda r: r["r_offset"]):
            o, t = r["r_offset"], r["r_info_type"]
            if not (lo_bound <= o < hi_bound):
                continue
            ins = int.from_bytes(buf[o:o + 4], "little")
            addr, is_text = symbol(syms[r["r_info_sym"]])
            if t == 2:          # R_MIPS_32
                v = text_address(ins) if is_text else addr + ins
                buf[o:o + 4] = (v & 0xFFFFFFFF).to_bytes(4, "little")
            elif t == 4:        # R_MIPS_26
                target = ((ins & 0x3FFFFFF) << 2)
                v = text_address(target) if is_text else addr + target
                if (v ^ base_of(o)) & 0xF0000000:
                    raise SystemExit("LINK a jump out of its 256 MB region")
                buf[o:o + 4] = ((ins & 0xFC000000) | ((v >> 2) & 0x3FFFFFF)).to_bytes(4, "little")
            elif t == 5:        # R_MIPS_HI16, resolved with its LO16
                pending.append((o, r["r_info_sym"], ins))
            elif t == 6:        # R_MIPS_LO16
                lo = sext(ins & 0xFFFF)
                for ho, hs, hins in [p for p in pending if p[1] == r["r_info_sym"]]:
                    full = ((hins & 0xFFFF) << 16) + lo
                    full = text_address(full) if is_text else addr + full
                    buf[ho:ho + 4] = ((hins & 0xFFFF0000) | (((full + 0x8000) >> 16) & 0xFFFF)).to_bytes(4, "little")
                pending = [p for p in pending if p[1] != r["r_info_sym"]]
                v = text_address(lo) if is_text else addr + lo
                buf[o:o + 4] = ((ins & 0xFFFF0000) | (v & 0xFFFF)).to_bytes(4, "little")
            elif t == 7:        # R_MIPS_GPREL16
                v = addr + sext(ins & 0xFFFF) - try_func.GP
                if not -0x8000 <= v < 0x8000:
                    raise SystemExit("LINK small data of the candidate's own file (static in .sdata): "
                                     "give it a retail D_ address or make it not small data")
                buf[o:o + 4] = ((ins & 0xFFFF0000) | (v & 0xFFFF)).to_bytes(4, "little")
            else:
                raise SystemExit(f"LINK relocation type {t} not handled")
        # A %hi whose %lo came earlier (the scheduler moved the %lo up, or one %lo serves a loop):
        # paired with the nearest %lo of the same symbol.
        for ho, hs, hins in pending:
            los = [r for r in rel.iter_relocations() if r["r_info_type"] == 6 and r["r_info_sym"] == hs
                   and lo_bound <= r["r_offset"] < hi_bound]
            if not los:
                raise SystemExit("LINK a %hi without its %lo")
            lo_r = min(los, key=lambda r: abs(r["r_offset"] - ho))
            lo_ins = int.from_bytes(orig[lo_r["r_offset"]:lo_r["r_offset"] + 4], "little")
            addr, is_text = symbol(syms[hs])
            full = ((hins & 0xFFFF) << 16) + sext(lo_ins & 0xFFFF)
            full = text_address(full) if is_text else addr + full
            buf[ho:ho + 4] = ((hins & 0xFFFF0000) | (((full + 0x8000) >> 16) & 0xFFFF)).to_bytes(4, "little")

    apply(text_idx, text, off, off + size, lambda o: BASE + o - off)
    for blob in blobs:
        apply(blob[3], blob[1], 0, len(blob[1]), lambda o, b=blob: b[0] + o)

    segments = [[BASE, text[off:off + size]]] + [[b[0], b[1]] for b in blobs]
    files = []
    for i, (a, data) in enumerate(segments):
        p = out.parent / f"{out.stem}.{i}.bin"
        p.write_bytes(bytes(data))
        files.append([p.name, a])
    out.write_text(json.dumps({"entry": BASE, "segments": files}))


# --- On the host -----------------------------------------------------------------

def retail(name: str) -> tuple[int, bytes]:
    m = re.fullmatch(r"func_([0-9A-F]{8})", name)
    if not m:
        sys.exit(f"{name}: only executable functions (func_XXXXXXXX) for now")
    address = int(m.group(1), 16)
    asm = next(ROOT.glob(f"asm/nonmatchings/*/{name}.s"), None)
    if asm is None:
        sys.exit(f"{name}: no assembly in asm/nonmatchings")
    m2 = re.search(r"^\s*(?:nonmatching|\.size)\s+\w+,\s*(0x[0-9A-Fa-f]+|\d+)", asm.read_text(), re.M)
    size = int(m2.group(1), 0) if m2 else 0
    raw = (ROOT / "baserom/SCES_509.16").read_bytes()
    # The executable's single load segment: file offset = address - delta.
    import struct
    phoff = struct.unpack_from("<I", raw, 0x1C)[0]
    _type, poff, pvaddr = struct.unpack_from("<III", raw, phoff)
    delta = pvaddr - poff
    if not size:
        size = 64
    return address, raw[address - delta:address - delta + size]


def capture(names: list[str], frames: int, level: int | None) -> None:
    STATES.mkdir(parents=True, exist_ok=True)
    lines = []
    for n in names:
        try:
            address, code = retail(n)
        except SystemExit as e:
            print(e)
            continue
        k = min(len(code), 256)
        lines.append(f"{address:08x} {k} {zlib.crc32(code[:k]):08x} {n} 3")
    lst = WORK / "capture_list.txt"
    lst.write_text("\n".join(lines) + "\n")
    cmd = [str(BOOT), str(DISC), "--hooks", str(HOOKS), "--no-card", "--frames", str(frames),
           "--capture", str(lst), str(STATES)]
    for p in PRESSES:
        cmd += ["--press", p]
    if level is not None:
        cmd += ["--write", f"600:15EE84:{level:x}:400"]
    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for n in names:
        got = len(list(STATES.glob(f"{n}.*.snap")))
        if not got:
            (STATES / f"{n}.none").write_text("not called in the capture run\n")
        print(f"{n}: {got} states" + ("" if got else " (not called in this run)"))


def returns(name: str, text: str) -> str:
    m = re.search(r"^[ \t]*(?:static\s+|inline\s+)*([\w \t\*]+?)[ \t\*]*\b" + name + r"\s*\(", text, re.M)
    if not m:
        return "void"
    t = m.group(0)
    if "*" in t.split(name)[0]:
        return "int"
    t = m.group(1).split()
    t = [w for w in t if w not in ("const", "volatile", "extern", "signed", "unsigned", "register")]
    w = " ".join(t)
    if w in ("void",):
        return "void"
    if w in ("float", "f32"):
        return "float"
    if w in ("long", "long long", "s64", "u64", "long int", "u_long"):
        return "long"
    if w.startswith("struct") or w.startswith("union"):
        return "void"
    return "int"


def check(name: str, candidate: Path, ulps: int = 0) -> int:
    WORK.mkdir(parents=True, exist_ok=True)
    states = sorted(STATES.glob(f"{name}.*.snap"))
    if not states and (STATES / f"{name}.none").exists():
        print(f"{name}: NOT CALLED in the capture run (title, menu, level 0); cannot check by behaviour. "
              "Use tools/equiv.py instead.")
        return 3
    if not states:
        print(f"{name}: no call states yet, capturing (about a minute)")
        capture([name], 2600, None)
        states = sorted(STATES.glob(f"{name}.*.snap"))
        if not states:
            print(f"{name}: NOT CALLED in the capture run (title, menu, level 0); cannot check by behaviour")
            return 3
    work = WORK / name
    work.mkdir(parents=True, exist_ok=True)
    out = work / "placed.json"
    if out.exists():
        out.unlink()
    rel = lambda p: str(Path(p).resolve().relative_to(ROOT))
    r = subprocess.run(["bash", "tools/docker/run.sh", "python3", "tools/fcheck.py", "--build", name,
                        rel(candidate), rel(out)], cwd=ROOT, capture_output=True, text=True)
    if r.returncode or not out.exists():
        msg = (r.stdout + r.stderr).strip().splitlines()
        print("\n".join(msg[-25:]))
        return 2
    placed = json.loads(out.read_text())
    cmd = [str(FCHECK), "--entry", f"{placed['entry']:x}", "--returns", returns(name, candidate.read_text(errors="replace"))]
    if ulps:
        cmd += ["--ulps", str(ulps)]
    for f, a in placed["segments"]:
        cmd += ["--code", f"{work / f}@{a:x}"]
    cmd += [str(s) for s in states]
    r = subprocess.run(cmd, capture_output=True, text=True)
    text = r.stdout.replace(str(STATES) + "/", "")
    print(text.strip() or r.stderr.strip())
    if r.returncode == 0:
        m = re.search(r"compared (\d+) of", r.stdout)
        print(f"{name}: SAME on {m.group(1) if m else '?'} real calls")
    elif r.returncode == 4:
        print(f"{name}: NOTHING COMPARED (every state skipped); check it statically with tools/equiv.py")
    return r.returncode


def main() -> None:
    a = sys.argv[1:]
    if a and a[0] == "--build":
        build(a[1], Path(a[2]), Path(a[3]))
        return
    if a and a[0] == "capture":
        frames = int(a[a.index("--frames") + 1]) if "--frames" in a else 2600
        level = int(a[a.index("--level") + 1]) if "--level" in a else None
        names = [l.split()[0] for l in Path(a[1]).read_text().splitlines() if l.strip() and not l.startswith("#")]
        capture(names, frames, level)
        return
    ulps = 0
    if "--ulps" in a:
        i = a.index("--ulps")
        ulps = int(a[i + 1])
        del a[i:i + 2]
    if len(a) != 2:
        sys.exit(__doc__)
    sys.exit(check(a[0], Path(a[1]), ulps))


if __name__ == "__main__":
    main()

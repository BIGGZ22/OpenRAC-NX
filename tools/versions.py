#!/usr/bin/env python3
"""
Can one source tree build two versions of a game? This places one version's compiled functions
at their places in the other version's executable and checks that they come out as its bytes:
every instruction equal except the fields relocations fill in, and every symbol needing a
single value. The values found are the address map the second version would link with
(functions and data alike), written to build/versions/FROM--TO.tsv.

  python3 tools/versions.py check rac1/pal rac1/ntsc --build ~/Projects/RAC/rac1-decomp
  python3 tools/versions.py link rac1/pal rac1/ntsc --build ~/Projects/RAC/rac1-decomp

--build is a checkout of FROM's project after a full build: its config/core_text.objects and
config/text.objects list the objects. Only functions FROM's project has matched are placed (the
rest are still retail assembly), and only where tools/xmap.py pairs them one to one in the boot
executable (`xmap.py scan` both versions first). Resident boot code only: no level programs yet.
"""
import json
import struct
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import mips  # noqa: E402
import xmap  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/versions"
R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16, R_MIPS_LITERAL = 2, 4, 5, 6, 7, 8
SHT_SYMTAB, SHT_REL, STT_FUNC, STT_SECTION = 2, 9, 2, 3


def read_object(path: Path) -> dict:
    """{"sections": [{name, data}], "symbols": [{name, value, size, section, type}],
    "relocations": {section index: [(offset, type, symbol index)]}} of a relocatable ELF32."""
    data = Path(path).read_bytes()
    shoff, = struct.unpack_from("<I", data, 0x20)
    entsize, count, names = struct.unpack_from("<3H", data, 0x2E)
    headers = [struct.unpack_from("<10I", data, shoff + i * entsize) for i in range(count)]

    def string(table: int, at: int) -> str:
        start = headers[table][4] + at
        return data[start:data.index(b"\0", start)].decode()
    sections = [{"name": string(names, h[0]), "type": h[1], "link": h[6], "info": h[7],
                 "data": data[h[4]:h[4] + h[5]] if h[1] != 8 else b""} for h in headers]
    symbols, relocations = [], {}
    for s in sections:
        if s["type"] == SHT_SYMTAB:
            for at in range(0, len(s["data"]), 16):
                name, value, size, info, _, shndx = struct.unpack_from("<IIIBBH", s["data"], at)
                symbols.append({"name": string(s["link"], name), "value": value, "size": size,
                                "section": shndx, "type": info & 15})
        elif s["type"] == SHT_REL:
            relocations[s["info"]] = [(o, i & 0xFF, i >> 8) for o, i in
                                      (struct.unpack_from("<II", s["data"], at) for at in range(0, len(s["data"]), 8))]
    return {"sections": sections, "symbols": symbols, "relocations": relocations}


def sext(x: int) -> int:
    return x - 0x10000 if x & 0x8000 else x


def derive(code: bytes, relocs: list, address: int, retail: bytes, gp: int) -> tuple[dict, str | None]:
    """The symbol values that make CODE, placed at ADDRESS, read as RETAIL: {symbol: value}, or
    why it cannot. RELOCS are (offset in CODE, type, symbol name), REL style (addends in place)."""
    if len(retail) != len(code):
        return {}, "outside the executable"
    touched = {o for o, _, _ in relocs}
    for at in range(0, len(code), 4):
        if at not in touched and code[at:at + 4] != retail[at:at + 4]:
            return {}, "instruction differs"
    word = lambda b, at: struct.unpack_from("<I", b, at)[0]
    values = {}

    def agree(symbol: str, value: int) -> str | None:
        return None if values.setdefault(symbol, value & 0xFFFFFFFF) == value & 0xFFFFFFFF else "a symbol needs two values"
    relocs = sorted(relocs)
    for n, (at, kind, symbol) in enumerate(relocs):
        mine, theirs = word(code, at), word(retail, at)
        if kind == R_MIPS_26:
            if (mine ^ theirs) & 0xFC000000:
                return {}, "jump differs"
            err = agree(symbol, ((address & 0xF0000000) | (theirs & 0x3FFFFFF) << 2) - ((mine & 0x3FFFFFF) << 2))
        elif kind == R_MIPS_HI16:
            lo = next((r for r in relocs[n + 1:] if r[1] == R_MIPS_LO16 and r[2] == symbol), None)
            if lo is None:
                return {}, "%hi without %lo"
            if (mine ^ theirs) & 0xFFFF0000:
                return {}, "%hi differs"
            target = ((theirs & 0xFFFF) << 16) + sext(word(retail, lo[0]) & 0xFFFF)
            err = agree(symbol, target - (((mine & 0xFFFF) << 16) + sext(word(code, lo[0]) & 0xFFFF)))
        elif kind in (R_MIPS_GPREL16, R_MIPS_LITERAL):
            if (mine ^ theirs) & 0xFFFF0000:
                return {}, "gp access differs"
            err = agree(symbol, gp + sext(theirs & 0xFFFF) - sext(mine & 0xFFFF))
        elif kind == R_MIPS_32:
            err = agree(symbol, theirs - mine)
        elif kind == R_MIPS_LO16:
            err = None                     # checked below, once the %hi has given the value
        else:
            return {}, f"relocation type {kind}"
        if err:
            return {}, err
    for at, kind, symbol in relocs:
        if kind == R_MIPS_LO16:
            mine, theirs = word(code, at), word(retail, at)
            if symbol not in values:
                return {}, "%lo without %hi"
            if (mine ^ theirs) & 0xFFFF0000 or (values[symbol] + sext(mine & 0xFFFF) - theirs) & 0xFFFF:
                return {}, "%lo differs"
    return values, None


def place(obj: dict, obj_path: str, names: list, funcs: list, sym: dict, address: int, retail: bytes,
          gp: int) -> tuple[dict, str | None]:
    """derive() for function SYM of OBJ placed at ADDRESS, where RETAIL is the other version's code."""
    section = obj["sections"][sym["section"]]
    start, end = sym["value"], sym["value"] + sym["size"]
    code = bytearray(section["data"][start:end])
    relocs = []
    for o, kind, i in obj["relocations"].get(sym["section"], []):
        if not start <= o < end:
            continue
        name = names[i]
        if kind == R_MIPS_26 and obj["symbols"][i]["type"] == STT_SECTION and obj["symbols"][i]["section"] == sym["section"]:
            # A call within the object goes to a function of it: name that function, so a
            # version whose object grew or shrank elsewhere still places the call.
            w, = struct.unpack_from("<I", code, o - start)
            to = (w & 0x3FFFFFF) << 2
            callee = next((f for f in funcs if f["value"] <= to < f["value"] + max(f["size"], 1)), None)
            if callee is not None:
                struct.pack_into("<I", code, o - start, (w & 0xFC000000) | (to - callee["value"]) >> 2)
                name = callee["name"]
        relocs.append((o - start, kind, name))
    return derive(bytes(code), relocs, address, retail, gp)


def pairs(src_key: str, dst_key: str) -> dict[int, int]:
    """FROM's boot address -> TO's, for functions with the same code once in each boot."""
    src, dst = (json.loads((xmap.INDEX / f"{xmap.slug(k)}.json").read_text()) for k in (src_key, dst_key))
    out = {}
    for fp, f in src["functions"].items():
        g = dst["functions"].get(fp)
        if not g:
            continue
        a = [p[1] for p in f["places"] if p[0] == "boot"]
        b = [p[1] for p in g["places"] if p[0] == "boot"]
        if len(a) == 1 and len(b) == 1:
            out[a[0]] = b[0]
    return out


def boot_of(key: str) -> tuple[Path, int]:
    """The version's boot executable and its $gp (from .reginfo)."""
    path, _fmt = xmap.program_files(xmap.versions()[key])["boot"]
    path = ROOT / path
    b = path.read_bytes()
    shoff, = struct.unpack_from("<I", b, 0x20)
    entsize, count, names = struct.unpack_from("<3H", b, 0x2E)
    strtab = struct.unpack_from("<6I", b, shoff + names * entsize)[4]
    for i in range(count):
        name, _kind, _flags, _addr, off, _size = struct.unpack_from("<6I", b, shoff + i * entsize)
        if b[strtab + name:b.index(b"\0", strtab + name)] == b".reginfo":    # SHT_MIPS_REGINFO
            return path, struct.unpack_from("<I", b, off + 20)[0]
    sys.exit(f"{path}: no .reginfo")


def check(src_key: str, dst_key: str, build: Path, boot_only: bool = False, every: bool = False) -> tuple:
    pal_to = pairs(src_key, dst_key)
    matched = {m["name"] for m in json.loads((xmap.INDEX / f"{xmap.slug(src_key)}.json").read_text())["matched"].values()
               if m["program"] == "boot"}
    boot, gp = boot_of(dst_key)
    sections = [(a, d) for a, d, _ in mips.elf_sections(boot).values()]

    def retail(address: int, size: int) -> bytes:
        return next((d[address - a:address - a + size] for a, d in sections if a <= address < a + len(d)), b"")

    objects = [line.split()[0] for name in ("core_text.objects", "text.objects")
               for line in (build / "config" / name).read_text().splitlines() if line.strip() and not line.startswith("#")]
    values, owner, outcome, conflicts, placed = {}, {}, Counter(), [], {}
    for obj_path in objects:
        obj = read_object(build / obj_path)
        names = [s["name"] if s["type"] != STT_SECTION else f"§{obj_path}:{obj['sections'][s['section']]['name']}"
                 for s in obj["symbols"]]
        funcs = [s for s in obj["symbols"] if s["type"] == STT_FUNC]
        for sym in obj["symbols"]:
            if sym["type"] != STT_FUNC or not (every or sym["name"] in matched):
                continue
            m = xmap.NAME.match(sym["name"])
            address = pal_to.get(int(m.group(2), 16)) if m else None
            if address is None:
                outcome["not paired one to one"] += 1
                continue
            found, why = place(obj, obj_path, names, funcs, sym, address, retail(address, sym["size"]), gp)
            if why:
                outcome[why] += 1
                continue
            clash = [s for s, v in found.items() if values.get(s, v) != v]
            if clash:
                outcome["disagrees with another function"] += 1
                conflicts.append((sym["name"], clash[0], owner[clash[0]]))
                continue
            for s, v in found.items():
                values.setdefault(s, v)
                owner.setdefault(s, sym["name"])
            placed[sym["name"]] = address
            outcome["same bytes"] += 1
    # A function's name says where it is in FROM: in TO it has to be where xmap pairs it.
    wrong = []
    for s, v in values.items():
        m = xmap.NAME.match(s)
        if m and not m.group(1) and s.startswith("func_"):
            want = pal_to.get(int(m.group(2), 16))
            if want is not None and want != v:
                wrong.append(s)
    OUT.mkdir(parents=True, exist_ok=True)
    out = OUT / f"{xmap.slug(src_key)}--{xmap.slug(dst_key)}.tsv"
    out.write_text(f"# {src_key} symbol -> its value in {dst_key} (tools/versions.py)\n" + "".join(
        f"{s}\t{v:08X}\n" for s, v in sorted(values.items()) if not s.startswith("§")))
    total = sum(outcome.values())
    print(f"{src_key} -> {dst_key}: {total} {'' if every else 'matched '}boot functions tried")
    for why, n in outcome.most_common():
        print(f"  {n:5}  {why}")
    data = sum(1 for s in values if not s.startswith(("§", "func_")))
    print(f"symbols: {len(values)} with one value each ({data} data or other names); "
          f"{len(wrong)} functions not where xmap puts them; map written to {out.relative_to(ROOT)}")
    for name, symbol, other in conflicts[:10]:
        print(f"  {name}: {symbol} disagrees with {other}")
    for s in wrong[:10]:
        print(f"  {s}: {values[s]:08X}, xmap says {pal_to[int(s[5:], 16)]:08X}")
    if not boot_only:
        levels(src_key, dst_key, build, gp, values)
    return values, placed, objects, gp


def levels(src_key: str, dst_key: str, build: Path, gp: int, boot_values: dict) -> None:
    """The same check for level code. Each level is a program of its own, with its own addresses,
    and shared level code sits in several of them: a function is placed in every level where both
    versions have it exactly once, and each level gets its own map. A reference from level code to
    the boot executable has to agree with the boot's map."""
    src, dst = (json.loads((xmap.INDEX / f"{xmap.slug(k)}.json").read_text()) for k in (src_key, dst_key))
    fp_of = {m["name"]: fp for fp, m in src["matched"].items() if m["program"] != "boot"}
    files = xmap.program_files(xmap.versions()[dst_key])
    texts = {}

    def retail(program: str, address: int, size: int) -> bytes:
        if program not in texts:
            path, fmt = files[program]
            texts[program] = xmap.load(path, fmt).get(".text", (0, b""))
        base, data = texts[program]
        return data[address - base:address - base + size] if base <= address < base + len(data) else b""

    def one(f: dict | None, program: str) -> int | None:
        places = [a for p, a in (f or {"places": []})["places"] if p == program]
        return places[0] if len(places) == 1 else None

    # Levels load over part of the boot's memory and carry their own copies of what was there:
    # only boot symbols below the lowest level code are the boot's in a level too.
    low = min(xmap.load(*files[p]).get(".text", (1 << 32, b""))[0] for p in files if p.startswith("level:"))
    boot_values = {k: v for k, v in boot_values.items() if v < low}
    objects = sorted(p for d in ["shared", *[d.name for d in (build / "build-sn/overlays/report").glob("l??_*")]]
                     for p in (build / "build-sn/overlays/report" / d).glob("*/obj.o"))
    values, owner, outcome, placements = {}, {}, Counter(), 0
    for path in objects:
        obj = read_object(path)
        rel = path.relative_to(build).as_posix()
        names = [s["name"] if s["type"] != STT_SECTION else f"§{rel}:{obj['sections'][s['section']]['name']}"
                 for s in obj["symbols"]]
        funcs = [s for s in obj["symbols"] if s["type"] == STT_FUNC]
        for sym in funcs:
            fp = fp_of.get(sym["name"])
            if fp is None or fp not in src["functions"]:
                continue
            programs = {p for p, _ in src["functions"][fp]["places"]}
            placed_any = False
            for program in sorted(programs):
                at, there = one(src["functions"][fp], program), one(dst["functions"].get(fp), program)
                if at is None or there is None:
                    continue
                placed_any = True
                placements += 1
                found, why = place(obj, rel, names, funcs, sym, there, retail(program, there, sym["size"]), gp)
                if not why:
                    why = next(("disagrees with the boot's map below the levels" for s, v in found.items()
                                if s in boot_values and boot_values[s] != v), None)
                if not why:
                    clash = [k for k, v in found.items() if values.get((program, k), v) != v]
                    why = "disagrees with another function" if clash else None
                if why:
                    outcome[why] += 1
                    continue
                for k, v in found.items():
                    values.setdefault((program, k), v)
                    owner.setdefault((program, k), sym["name"])
                outcome["same bytes"] += 1
            if not placed_any:
                outcome["functions not paired one to one in any level"] += 1
    out = OUT / f"{xmap.slug(src_key)}--{xmap.slug(dst_key)}.levels.tsv"
    out.write_text(f"# {src_key} level symbol -> its value in {dst_key}'s level program (tools/versions.py)\n" + "".join(
        f"{p}\t{k}\t{v:08X}\n" for (p, k), v in sorted(values.items()) if not k.startswith("§")))
    tried = placements
    print(f"levels: {len(fp_of)} matched level functions, {tried} placements tried")
    for why, n in outcome.most_common():
        print(f"  {n:5}  {why}")
    print(f"symbols: {sum(1 for p, k in values if not k.startswith('§'))} (level, symbol) values; "
          f"map written to {out.relative_to(ROOT)}")


def link(src_key: str, dst_key: str, build: Path) -> None:
    """Link FROM's whole boot objects into TO's layout with SN's own linker and compare.

    An object goes in when every function in it placed cleanly and at the same offsets in TO (the
    file is one unit in TO too); its read-only data goes where check() found it, and every other
    symbol gets its value from the map. The objects are linked by FROM's toolchain, run by FROM's
    checkout (rac1-decomp's tools/docker/run.sh), into build-sn/versions/, and each object's
    linked bytes are compared with TO's retail executable."""
    # Every function of an object, its retail assembly too: the file is the unit that is linked.
    values, placed, objects, gp = check(src_key, dst_key, build, boot_only=True, every=True)
    boot, _gp = boot_of(dst_key)
    sections = [(a, d) for a, d, _ in mips.elf_sections(boot).values()]
    retail = lambda address, size: next((d[address - a:address - a + size] for a, d in sections
                                         if a <= address < a + len(d)), b"")
    work = build / "build-sn/versions"
    work.mkdir(parents=True, exist_ok=True)
    units, skipped = [], Counter()
    for obj_path in objects:
        obj = read_object(build / obj_path)
        text = next((i for i, sec in enumerate(obj["sections"]) if sec["name"] == ".text"), None)
        funcs = [f for f in obj["symbols"] if f["type"] == STT_FUNC and f["section"] == text and f["size"]]
        if not funcs:
            continue
        bases = {placed[f["name"]] - f["value"] if f["name"] in placed else None for f in funcs}
        if None in bases or len(bases) != 1:
            skipped["not every function placed, or not at the same offsets"] += 1
            continue
        data = {}
        for i, rel in obj["relocations"].items():
            for _o, _k, sym in rel:
                s = obj["symbols"][sym]
                if s["type"] == STT_SECTION and s["section"] != text:
                    name = obj["sections"][s["section"]]["name"]
                    data[name] = values.get(f"§{obj_path}:{name}")
        if None in data.values():
            skipped["read-only data with no place found"] += 1
            continue
        units.append((obj_path, bases.pop(), len(obj["sections"][text]["data"]), data))
    script = ["SECTIONS", "{", f"  _gp = 0x{gp:08X};"]
    script += [f"  PROVIDE({k} = 0x{v:08X});" for k, v in sorted(values.items()) if not k.startswith("§")]
    for n, (obj_path, base, _size, data) in enumerate(units):
        script.append(f"  .t{n} 0x{base:08X} : {{ {obj_path}(.text) }}")
        script += [f"  .d{n}_{k} 0x{a:08X} : {{ {obj_path}({sec}) }}" for k, (sec, a) in enumerate(sorted(data.items()))]
    script += ["  /DISCARD/ : { *(*) }", "}"]
    (work / "link.ld").write_text("\n".join(script) + "\n")
    (work / "objects.txt").write_text("".join(f"{u[0]}\n" for u in units))
    import subprocess
    cmd = ('. tools/toolchain.sh && sn "$TC/ee-ld.exe" -T build-sn/versions/link.ld '
           '$(cat build-sn/versions/objects.txt) -o build-sn/versions/link.elf')
    run = subprocess.run(["bash", "tools/docker/run.sh", "bash", "-c", cmd], cwd=build, capture_output=True, text=True)
    if run.returncode:
        sys.exit("the link failed:\n" + (run.stdout + run.stderr)[-3000:])
    out = {name: (a, d) for name, (a, d, _f) in mips.elf_sections(work / "link.elf").items()}
    same = differ = 0
    size_same = 0
    for n, (obj_path, base, size, data) in enumerate(units):
        pieces = [(f".t{n}", base)] + [(f".d{n}_{k}", a) for k, (_s, a) in enumerate(sorted(data.items()))]
        ok = all(name in out and out[name][1] == retail(a, len(out[name][1])) for name, a in pieces if name in out or name.startswith(".t"))
        same, differ = same + ok, differ + (not ok)
        size_same += size if ok else 0
        if not ok:
            print(f"  differs: {obj_path}")
    print(f"linked {len(units)} whole objects with SN's ee-ld at their {dst_key} addresses: "
          f"{same} identical to retail ({size_same:,} bytes of code), {differ} not")
    for why, n in skipped.most_common():
        print(f"  {n:5}  objects left out: {why}")


def main() -> None:
    args = sys.argv[1:]
    if len(args) == 5 and args[0] in ("check", "link") and args[3] == "--build":
        (check if args[0] == "check" else link)(args[1], args[2], Path(args[4]).expanduser())
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()

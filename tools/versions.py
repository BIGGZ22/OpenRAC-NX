#!/usr/bin/env python3
"""
Can one source tree build two versions of a game? This places one version's compiled functions
at their places in the other version's executable and checks that they come out as its bytes:
every instruction equal except the fields relocations fill in, and every symbol needing a
single value. The values found are the address map the second version would link with
(functions and data alike), written to build/versions/FROM--TO.tsv.

  python3 tools/versions.py check rac1/pal rac1/ntsc --build ~/Projects/RAC/rac1-decomp
  python3 tools/versions.py link rac1/pal rac1/ntsc --build ~/Projects/RAC/rac1-decomp
  python3 tools/versions.py build rac1/pal rac1/ntsc --build ~/Projects/RAC/rac1-decomp

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
    found, why = derive(bytes(code), relocs, address, retail, gp)
    if why is None:
        found["\0code"], found["\0relocs"] = bytes(code), relocs     # for build(): the piece as placed
    return found, why


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
    values, owner, outcome, conflicts, placed, pieces = {}, {}, Counter(), [], {}, {}
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
            piece = (found.pop("\0code"), found.pop("\0relocs"))
            clash = [s for s, v in found.items() if values.get(s, v) != v]
            if clash:
                outcome["disagrees with another function"] += 1
                conflicts.append((sym["name"], clash[0], owner[clash[0]]))
                continue
            for s, v in found.items():
                values.setdefault(s, v)
                owner.setdefault(s, sym["name"])
            placed[sym["name"]] = address
            pieces[sym["name"]] = piece
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
    check.pieces = pieces
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
                found.pop("\0code", None), found.pop("\0relocs", None)
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


def write_object(path: Path, data: bytes, defined: dict[str, int], relocs: list, align: int) -> None:
    """A relocatable ELF32 (little-endian MIPS) with DATA as its .text: DEFINED symbols at their
    offsets, RELOCS as (offset, type, symbol name) in REL form (addends in place)."""
    names = list(defined) + sorted({n for _, _, n in relocs} - set(defined))
    strtab, at = b"\0", {}
    for n in names:
        at[n] = len(strtab)
        strtab += n.encode() + b"\0"
    symtab = bytes(16) + b"".join(
        struct.pack("<IIIBBH", at[n], defined.get(n, 0), 0, 0x10 | (STT_FUNC if n in defined else 0), 0,
                    1 if n in defined else 0) for n in names)
    index = {n: i + 1 for i, n in enumerate(names)}
    rel = b"".join(struct.pack("<II", o, index[n] << 8 | kind) for o, kind, n in relocs)
    shstr = b"\0.text\0.rel.text\0.symtab\0.strtab\0.shstrtab\0"
    body = [data, rel, symtab, strtab, shstr]
    offsets, off = [], 52
    for b in body:
        off = (off + 3) & ~3
        offsets.append(off)
        off += len(b)
    shoff = (off + 3) & ~3
    headers = [bytes(40),
               struct.pack("<10I", 1, 1, 6, 0, offsets[0], len(data), 0, 0, align, 0),
               struct.pack("<10I", 7, SHT_REL, 0, 0, offsets[1], len(rel), 3, 1, 4, 8),
               struct.pack("<10I", 17, SHT_SYMTAB, 0, 0, offsets[2], len(symtab), 4, 1, 4, 16),
               struct.pack("<10I", 25, 3, 0, 0, offsets[3], len(strtab), 0, 0, 1, 0),
               struct.pack("<10I", 33, 3, 0, 0, offsets[4], len(shstr), 0, 0, 1, 0)]
    out = bytearray(struct.pack("<4s5B7x", b"\x7fELF", 1, 1, 1, 0, 0))
    out += struct.pack("<HHIIIIIHHHHHH", 1, 8, 1, 0, 0, shoff, 0x20924001, 52, 0, 0, 40, len(headers), 5)
    for o, b in zip(offsets, body):
        out += bytes(o - len(out)) + b
    out += bytes(shoff - len(out)) + b"".join(headers)
    path.write_bytes(bytes(out))


def paired(relocs: list) -> list:
    """RELOCS with each %hi moved to just before its first %lo, as GNU as orders them: SN's ld
    pairs a %hi with the relocation that follows it."""
    rest = sorted(relocs)
    out = []
    while rest:
        r = rest.pop(0)
        if r[1] == R_MIPS_HI16:
            j = next((k for k, x in enumerate(rest) if x[1] == R_MIPS_LO16 and x[2] == r[2]), None)
            if j is not None:
                # the %lo goes first in line, the relocations between keep their order after it
                out += [r, rest.pop(j)]
                continue
            # a second %hi sharing a %lo already placed: it goes just before that %lo
            k = next((k for k in range(len(out) - 1, -1, -1) if out[k][1] == R_MIPS_LO16 and out[k][2] == r[2]), None)
            if k is not None:
                out.insert(k, r)
                continue
        out.append(r)
    return out


def build(src_key: str, dst_key: str, build_dir: Path) -> None:
    """TO's boot executable linked from FROM's compiled functions, placed one by one.

    Every function check() places goes in as a piece of its own (its bytes and relocations, cut
    from FROM's object, calls within the object resolved to the function they reach) at its
    address in TO; everything else stays TO's own: its retail bytes, standing in for its assembly.
    SN's ee-ld links it all with the address map as the symbols' values, and every section of the
    result is compared with TO's executable."""
    values, placed, _objects, gp = check(src_key, dst_key, build_dir, boot_only=True)
    pieces = check.pieces
    boot, _gp = boot_of(dst_key)
    b = boot.read_bytes()
    shoff, = struct.unpack_from("<I", b, 0x20)
    entsize, count, names_at = struct.unpack_from("<3H", b, 0x2E)
    strtab = struct.unpack_from("<6I", b, shoff + names_at * entsize)[4]
    retail = []           # (name, address, bytes) of every loaded section with contents
    for i in range(count):
        name, kind, flags, addr, off, size = struct.unpack_from("<6I", b, shoff + i * entsize)
        if kind in (1, 8) and addr and flags & 2 and size:      # SHT_PROGBITS or SHT_NOBITS, SHF_ALLOC
            retail.append((b[strtab + name:b.index(b"\0", strtab + name)].decode(), addr,
                           b[off:off + size] if kind == 1 else None, size))
    work = build_dir / "build-sn/vb"
    if work.exists():
        for f in work.iterdir():
            f.unlink()
    work.mkdir(parents=True, exist_ok=True)
    alias = {}             # a section symbol's name in the map -> a name the linker script can hold
    sym = lambda n: alias.setdefault(n, f"__section_{len(alias)}") if n.startswith("§") else n
    skipped = Counter()
    script = ['OUTPUT_FORMAT("elf32-littlemips")', "SECTIONS", "{", f"  _gp = 0x{gp:08X};"]
    files = 0

    def emit(data: bytes, defined: dict, relocs: list, align: int) -> str:
        nonlocal files
        files += 1
        path = work / f"{files}.o"
        write_object(path, data, defined, relocs, align)
        return f"build-sn/vb/{files}.o(.text)"
    provide = {**{k: v for k, v in values.items() if not k.startswith("§")}}
    # Symbols are defined inside the output section that holds their address, relative to its
    # start: SN's ld computes %hi wrongly against absolute symbols (PROVIDE outside a section).
    inside = {n: sorted((v - addr, k) for k, v in provide.items() if addr <= v < addr + size)
              for n, (_name, addr, _data, size) in enumerate(retail)}
    for n, (name, addr, data, size) in enumerate(retail):
        start = [f"    {sym(k) if k.startswith('§') else k} = . + 0x{o:X};" for o, k in inside[n]]
        if data is None:                       # .bss: no bytes, only the symbols in it
            script.append(f"  .s{n} 0x{addr:08X} (NOLOAD) : {{")
            script += start + [f"    . = . + 0x{size:X};", "  }"]
            continue
        here = sorted((placed[f], f) for f in pieces if addr <= placed[f] < addr + len(data))
        inputs, at = [], addr
        for where, f in here:
            code, relocs = pieces[f]
            if any(kind == R_MIPS_LITERAL for _, kind, _ in relocs):
                skipped["uses a literal-pool relocation (left as retail)"] += 1
                continue
            if where > at:
                inputs.append(emit(data[at - addr:where - addr], {}, [], 1))
            inputs.append(emit(code, {f: 0}, paired([(o, kind, sym(t)) for o, kind, t in relocs]), 4))
            at = where + len(code)
        if at < addr + len(data):
            inputs.append(emit(data[at - addr:], {}, [], 1))
        script.append(f"  .s{n} 0x{addr:08X} : {{")
        script += start + [f"    {i}" for i in inputs]
        script.append("  }")
    placed_in = {k for ks in inside.values() for _o, k in ks}
    rest = {k: v for k, v in provide.items() if k not in placed_in}
    rest.update({alias[k]: v for k, v in values.items() if k in alias})
    script[4:4] = [f"  PROVIDE({k} = 0x{v:08X});" for k, v in sorted(rest.items())]
    script += ["  /DISCARD/ : { *(*) }", "}"]
    (work / "link.ld").write_text("\n".join(script) + "\n")
    import subprocess
    run = subprocess.run(["bash", "tools/docker/run.sh", "bash", "-c",
                          '. tools/toolchain.sh && sn "$TC/ee-ld.exe" -T build-sn/vb/link.ld -o build-sn/vb/out.elf'],
                         cwd=build_dir, capture_output=True, text=True)
    if run.returncode:
        sys.exit("the link failed:\n" + (run.stdout + run.stderr)[-3000:])
    out = {a: d for _n, (a, d, _f) in mips.elf_sections(work / "out.elf").items()}
    retail = [(name, addr, data) for name, addr, data, _size in retail if data is not None]
    same = [name for name, addr, data in retail if out.get(addr) == data]
    differ = [(name, addr, data) for name, addr, data in retail if out.get(addr) != data]
    c_bytes = sum(len(pieces[f][0]) for f in pieces)
    print(f"{dst_key} boot linked from {len(pieces)} placed {src_key} functions ({c_bytes:,} bytes) "
          f"and its own bytes elsewhere: {len(same)} of {len(retail)} sections identical to retail")
    for why, k in skipped.most_common():
        print(f"  {k:5}  {why}")
    for name, addr, data in differ:
        got = out.get(addr, b"")
        first = next((i for i in range(min(len(got), len(data))) if got[i] != data[i]), min(len(got), len(data)))
        print(f"  differs: {name} at {addr:#x}, first at +{first:#x} ({len(got)} bytes linked, {len(data)} retail)")


def main() -> None:
    args = sys.argv[1:]
    if len(args) == 5 and args[0] in ("check", "link", "build") and args[3] == "--build":
        {"check": check, "link": link, "build": build}[args[0]](args[1], args[2], Path(args[4]).expanduser())
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()

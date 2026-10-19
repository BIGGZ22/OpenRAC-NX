#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Every function with no C, classified: does the port need C for it?

    python3 port/tools/hostgen/no_c.py --game port/game/rac1-pal/hostgen.json \\
        --gen port/build/release/games/rac1-pal/gen --source games/rac1 \\
        --exe games/rac1/baserom/SCES_509.16 --dump games/rac1/baserom/overlays \\
        --out port/game/rac1-pal/no_c.tsv

hostgen's report lists the functions the decompilation has no C for (its stubs) and those
it wrote C for from their assembly (asmc.py). This sorts each into one of three classes,
from the retail program itself, not from the C:

    a  needs no C: a linker remnant or alignment padding; a piece of a function with C that
       the catalogue named apart (only its owner's branches and jumps reach it); another
       spelling of a function with C (calls go there); or nothing can run it (no call, jump
       or address of it in the code or data of any program, the executable's or a level's)
    b  replaced by the host on purpose: what reaches it is only the port's own versions of
       its callers (libraries.tsv host rows, hostgen.json host_functions), or it is one
    c  needs C: something the port runs can call it (a call or a jump from code with C, its
       address in a table or formed in code)

References are read from the retail bytes of every program (the executable, and each level's
records laid over it; --image reads them from the disc image, as retail.py does for hostgen):
jal and j targets, branch targets, the code addresses lui pairs form (retail.py's pairing), and
every word of the data that holds a function's address (the moby class tables, callbacks,
menus). A word in code that equals an address is an instruction, not a reference, and is
ignored. hostgen's own call graph (report.json) adds what its C calls or takes the address of.

The table it writes has a row per function: name, class (a, b, c), kind, the C it has now
(none, or "translated" for asmc.py's), how the program can reach it, and the evidence. A row
whose evidence starts with "by hand:" is a judgement the tool cannot make (a data word that
only looks like an address, say): it is kept from the existing table as it is, and the
references it judged do not count for the functions whose code it is part of.
"""
from __future__ import annotations

import argparse
import bisect
import collections
import json
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import asm  # noqa: E402
import retail  # noqa: E402
from lower import HOST_MATH  # noqa: E402

# Names hostgen writes as the host C library's own when they have no definition (lower.py).
HOST_LIBC = set(HOST_MATH) | {"memcpy", "memmove", "memset"}

COLUMNS = ["name", "class", "kind", "c", "reachable", "evidence"]
BRANCH_OPS = {4, 5, 6, 7, 0x14, 0x15, 0x16, 0x17}
REGIMM_BRANCHES = {0, 1, 2, 3, 0x10, 0x11, 0x12, 0x13}
FILL = 0xCDCDCDCD


def read_tsv_names(path: Path, col: int = 0) -> list[str]:
    out = []
    if path.is_file():
        for line in path.read_text().splitlines():
            if line.strip() and not line.startswith("#"):
                out.append(line.split("\t")[col].strip())
    return out


def read_functions_c(path: Path) -> dict[int, dict[int, str]]:
    """functions.c: program (-1 the executable, n level n) -> {address: name}."""
    progs: dict[int, dict[int, str]] = {}
    cur = None
    for line in path.read_text().splitlines():
        m = re.match(r"static const openrac_fn_entry k_(exe|overlay_(\d+))\[\]", line)
        if m:
            cur = progs.setdefault(-1 if m.group(1) == "exe" else int(m.group(2)), {})
            continue
        m = re.match(r'\s*\{0x([0-9A-Fa-f]+)u, \(openrac_host_fn\)\w+, "(\w+)"\}', line)
        if m and cur is not None:
            cur.setdefault(int(m.group(1), 16), m.group(2))
    return progs


def scan(r: retail.Retail, level: int, starts: dict[int, dict[int, str]]) -> dict[int, list[tuple[str, int, str]]]:
    """address -> [(kind, from, part)]: what in program `level` refers to the address: its code's
    jal, j and branch targets and the code addresses its lui pairs form (retail.py's pairing), and
    every word of its data. A level's program is its own records: what the executable keeps under
    `low` is the executable's program's, whichever level is loaded."""
    refs: dict[int, list[tuple[str, int, str]]] = collections.defaultdict(list)
    code = retail.RetailCode(r, starts, r.low)
    for part in (r.exe if level < 0 else r.levels.get(level, [])):
        n = len(part.data) // 4
        if not n:
            continue
        words = struct.unpack_from(f"<{n}I", part.data)
        if not part.code:
            for k, x in enumerate(words):
                refs[x].append(("data", part.base + 4 * k, part.name))
            continue
        for k, x in enumerate(words):
            pc = part.base + 4 * k
            op = x >> 26
            if op in (2, 3):
                refs[(pc & 0xF0000000) | ((x & 0x3FFFFFF) << 2)].append(("jal" if op == 3 else "j", pc, part.name))
            elif op in BRANCH_OPS or (op == 1 and ((x >> 16) & 31) in REGIMM_BRANCHES) \
                    or (op == 0x11 and ((x >> 21) & 31) == 8):
                off = x & 0xFFFF
                if off & 0x8000:
                    off -= 0x10000
                refs[(pc + 4 + off * 4) & 0xFFFFFFFF].append(("branch", pc, part.name))
        # Code addresses formed with a lui and its other half (a callback handed on).
        relocs, _tables = code.pairs(level, part.base, list(words))
        where = {}
        for h in ([level, -1] if level >= 0 else [-1]):
            for a, name in starts.get(h, {}).items():
                where.setdefault(name, a)
        for at, (_kind, sym, addend) in relocs.items():
            if addend == 0 and sym in where:
                refs[where[sym]].append(("address", at, part.name))
    return refs


def configure(cfg_path: Path) -> dict:
    cfg = json.loads(cfg_path.read_text())
    cfg["_dir"] = cfg_path.parent
    return cfg


def host_names(cfg: dict) -> dict[str, str]:
    """What the port writes itself: name -> why (libraries.tsv host rows, host_functions)."""
    out = {n: "hostgen.json host_functions" for n in cfg.get("host_functions", [])}
    table = cfg.get("libraries")
    if table and (cfg["_dir"] / table).is_file():
        for line in (cfg["_dir"] / table).read_text().splitlines():
            if not line.strip() or line.startswith("#"):
                continue
            cols = line.split("\t")
            if len(cols) >= 4 and cols[3].strip() == "host":
                out[cols[0].strip()] = f"libraries.tsv host ({cols[1].strip()} {cols[2].strip()})"
    return out


def decomp_lists(source: Path) -> dict[str, set[str]]:
    """The decompilation's own classification, where it has one (rac1/pal)."""
    def names(path: Path) -> set[str]:
        return {n.split("/")[-1] for n in read_tsv_names(path)}
    joined = set()
    p = source / "config/overlays/joined.tsv"
    if p.is_file():
        for line in p.read_text().splitlines():
            if line.strip() and not line.startswith("#"):
                joined.update(c.strip() for c in line.split("\t")[1:2])
    return {
        "remnant": names(source / "config/linker_remnants.txt") | names(source / "config/overlays/linker_remnants.txt"),
        "handwritten": names(source / "config/handwritten_asm.txt"),
        "joined": joined,
    }


def keep_rows(path: Path) -> dict[str, dict]:
    rows = {}
    if path.is_file():
        lines = [l for l in path.read_text().splitlines() if l and not l.startswith("#")]
        for line in lines[1:] if lines and lines[0].startswith("name\t") else lines:
            cols = line.split("\t")
            if len(cols) == len(COLUMNS) and cols[5].startswith("by hand:"):
                rows[cols[0]] = dict(zip(COLUMNS, cols))
    return rows


def classify(cfg: dict, gen: Path, source: Path, asm_source: Path, memory: retail.Retail,
             kept: dict[str, dict]) -> list[dict]:
    report = json.loads((gen / "report.json").read_text())
    if "without_c_places" not in report:
        raise SystemExit("no_c: report.json has no without_c_places; build again with this hostgen")
    places = report["without_c_places"]
    translated = set(report.get("from_assembly", {}))
    unsupported = report.get("assembly_unsupported", {})
    no_asm = set(report.get("without_c_or_assembly", []))
    host = host_names(cfg)
    lists = decomp_lists(source)
    progs = read_functions_c(gen / "functions.c")
    stubs = {n for n, p in places.items() if n not in translated}
    low = memory.low

    def prog_map(level: int) -> dict[int, str]:
        if level < 0:
            return progs.get(-1, {})
        return {**{a: n for a, n in progs.get(-1, {}).items() if a < low}, **progs.get(level, {})}

    maps = {lv: prog_map(lv) for lv in sorted(set(progs) | set(memory.levels) | {-1})}
    starts = {lv: sorted(m) for lv, m in maps.items()}

    def owner(level: int, addr: int) -> str | None:
        s = starts[level]
        i = bisect.bisect_right(s, addr) - 1
        return maps[level][s[i]] if i >= 0 else None

    refs = {lv: scan(memory, lv, progs) for lv in maps}

    # The assembly: each function's instructions, for its size and its first word.
    a_cfg = cfg.get("asm") or {}
    index = asm.read_index([asm_source / d for d in a_cfg.get("dirs", [])], lambda n: _overlay_of(cfg, n))
    sizes: dict[str, int] = {b.name: 4 * len(b.lines) for b in index.blocks}

    calls_in_c = collections.defaultdict(set)
    for caller, callees in report.get("call_graph", {}).items():
        for callee in callees:
            calls_in_c[callee].add(caller)

    entry = memory.entry
    exe_map = maps.get(-1, {})
    if entry in exe_map and exe_map[entry] not in host:
        host[exe_map[entry]] = ("the executable's entry (crt0): the port's own start calls the game's main "
                                f"({cfg.get('entry', 'hostgen.json entry')}) itself")

    def status(name: str | None) -> str:
        if name is None:
            return "no function"
        if name in host:
            return "host"
        if name in translated:
            return "translated"
        if name in stubs:
            return "no C"
        return "C"

    rows = []
    info: dict[str, dict] = {}
    for name in sorted(places):
        p = places[name]
        size = sizes.get(name, 4)
        found = collections.defaultdict(set)      # kind -> {(level, from, owner)}
        first_words = set()
        for level, a in p["places"]:
            addr = int(a, 16)
            if level not in refs:
                continue
            w = memory.word(level, addr)
            if w is not None:
                first_words.add(w)
            # The executable's code and data under `low` refer to it whatever level is loaded.
            shared = refs[-1].get(addr, []) if level >= 0 and addr < low else []
            for kind, frm, part in list(refs[level].get(addr, ())) + shared:
                if addr <= frm < addr + max(size, 4):
                    continue          # its own code
                own = owner(level, frm) if kind != "data" else None
                found[kind].add((level, frm, own, part))
        info[name] = {"found": found, "first_words": first_words, "size": size}

    def callers_of(name: str) -> set:
        found = info[name]["found"]
        return {o for k in ("jal", "j", "address") for _lv, _frm, o, _p in found.get(k, ())} | calls_in_c.get(name, set())

    def behind_host(name: str) -> bool:
        cs = callers_of(name)
        return bool(cs) and all(status(c) == "host" for c in cs) and not info[name]["found"].get("data")

    def piece_root(name: str, seen: set) -> tuple[str, str] | None:
        """For a function reached only by the branches and jumps of other functions' code, the
        function with C (or the host's) whose code it is part of, and how."""
        if name in seen:
            return None
        seen = seen | {name}
        f = info.get(name)
        if f is None:
            return None
        found = f["found"]
        # A judgement kept by hand (an address-like word that is not one) is not a reference.
        if found.get("jal") or (found.get("address") or found.get("data")) and name not in kept \
                or calls_in_c.get(name):
            return None
        owners = {o for k in ("branch", "j") for _lv, _frm, o, _p in found.get(k, ())}
        if not owners:
            return None
        roots = set()
        for o in owners:
            st = status(o)
            if st in ("C", "host", "translated"):
                roots.add((o, st))
            elif st == "no C" and o in info:
                r = piece_root(o, seen)
                if r is None and behind_host(o):
                    r = (o, "host")
                if r is None:
                    return None
                roots.add(r)
            else:
                return None
        return sorted(roots)[0] if roots else None

    for name in sorted(places):
        p = places[name]
        f = info[name]
        found = f["found"]
        why = unsupported.get(name, "no assembly here" if name in no_asm else "")
        c_now = "translated" if name in translated else "none"
        levels = sorted({lv for lv, _ in p["places"]})
        where = _levels_text(levels)

        def describe(kind: str, limit: int = 3) -> str:
            items = sorted(found.get(kind, ()))
            owners = collections.OrderedDict()
            for lv, frm, own, part in items:
                key = own if kind != "data" else part
                owners.setdefault(key, set()).add(lv)
            bits = []
            for key, lvs in list(owners.items())[:limit]:
                if kind == "data":
                    lv0 = sorted(lvs)[0]
                    frm = min(frm for lv, frm, own, part in items if part == key and lv == lv0)
                    bits.append(f"{key} {frm:08X} ({_levels_text(sorted(lvs))})")
                else:
                    bits.append(f"{key} [{status(key)}] ({_levels_text(sorted(lvs))})")
            more = len(owners) - limit
            return ", ".join(bits) + (f" and {more} more" if more > 0 else "")

        reach_bits = []
        for kind, label in (("jal", "called by"), ("j", "jumped to by"), ("address", "address formed by"),
                            ("data", "address held in data at"), ("branch", "branched to by")):
            if found.get(kind):
                reach_bits.append(f"{label} {describe(kind)}")
        if calls_in_c.get(name):
            cs = sorted(calls_in_c[name])
            reach_bits.append("C calls it: " + ", ".join(cs[:3]) + (f" and {len(cs) - 3} more" if len(cs) > 3 else ""))
        reachable = "; ".join(reach_bits) if reach_bits else "nothing: no call, jump, branch or address of it"
        if not p["places"]:
            reachable = "; ".join(reach_bits) if reach_bits else "nothing: no C calls it, and it has no address"

        row = {"name": name, "c": c_now, "reachable": reachable}
        callers = callers_of(name)
        root = piece_root(name, set())
        hand_or_asm = "hand-written" if name in lists["handwritten"] else ""
        if "other_spelling" in p:
            row.update(cls="a", kind="other spelling",
                       evidence=f"the same function as {p['other_spelling']}, which has C: calls to this name go there")
        elif name in host:
            row.update(cls="b", kind="host", evidence=host[name])
        elif name in lists["remnant"] and not callers:
            row.update(cls="a", kind="linker remnant",
                       evidence="the decompilation's linker remnants list (what the linker left of a stripped function)")
        elif name in lists["joined"] and not callers and not found.get("data"):
            row.update(cls="a", kind="piece", evidence="config/overlays/joined.tsv: a piece of the function before it")
        elif not p["places"] and name in HOST_LIBC:
            row.update(cls="b", kind="host",
                       evidence="the C library's own name: hostgen writes calls to it as the host compiler's (lower.py)")
        elif not p["places"] and not calls_in_c.get(name):
            row.update(cls="a", kind="declared only",
                       evidence="declared by the decompilation, defined nowhere, and no C calls it or takes its address")
        elif not p["places"]:
            row.update(cls="c", kind="no address",
                       evidence="C calls it by a name with no address" + (f" ({why})" if why else ""))
        elif f["first_words"] == {FILL} and not callers and not found.get("data"):
            row.update(cls="a", kind="padding", evidence=f"its first word is the linker's 0xCDCDCDCD fill ({where})")
        elif root is not None:
            o, st = root
            row.update(cls="a" if st != "host" else "b", kind="piece",
                       evidence=f"only the branches and jumps of {o} [{st}] reach it: part of that function's code"
                                + (f"; {why}" if why else ""))
        elif not found and not callers:
            row.update(cls="a", kind="unreachable",
                       evidence=f"no call, jump, branch, address or data word refers to it in any program ({where})"
                                + (f"; {why}" if why else ""))
        elif behind_host(name):
            row.update(cls="b", kind="behind host",
                       evidence="only functions the port replaces reach it: " + ", ".join(sorted(callers)[:4])
                                + (f"; {why}" if why else ""))
        else:
            row.update(cls="c", kind="needs C", evidence=(hand_or_asm + "; " if hand_or_asm else "")
                       + (why or "translated from its assembly" if c_now == "translated" else why or "no assembly"))
        if name in kept:
            k = kept[name]
            row.update(cls=k["class"], kind=k["kind"], evidence=k["evidence"])
        rows.append(row)
    return rows


def _overlay_of(cfg: dict, name: str) -> int:
    pats = [re.compile(p) for p in (cfg.get("names", {}).get("code") or
                                    [r"^func_(?:L(?P<overlay>\d\d)_)?(?P<addr>[0-9A-Fa-f]{8})(?:_\w*)?$",
                                     r"^FUN_(?:L(?P<overlay>\d\d)_)?(?P<addr>[0-9A-Fa-f]{8})$"])]
    for p in pats:
        m = p.match(name)
        if m:
            o = m.groupdict().get("overlay")
            return int(o) if o is not None else -1
    return -1


def _levels_text(levels: list[int]) -> str:
    if not levels:
        return "nowhere"
    if levels == [-1]:
        return "the executable"
    lv = [x for x in levels if x >= 0]
    exe = "the executable, " if -1 in levels else ""
    if len(lv) == 19:
        return exe + "every level"
    return exe + ("level " if len(lv) == 1 else "levels ") + ",".join(str(x) for x in lv)


def write(rows: list[dict], out: Path, game: str) -> None:
    head = [f"# {game}: every function hostgen has no C for, or wrote C for from its assembly, and",
            "# whether the port needs C for it. Written by port/tools/hostgen/no_c.py (its docstring says",
            "# how); rows whose evidence starts with \"by hand:\" are kept as they are. class: a needs no C,",
            "# b replaced by the host on purpose, c needs C. c: the C it has now.",
            "\t".join(COLUMNS)]
    lines = ["\t".join([r["name"], r["cls"], r["kind"], r["c"], r["reachable"], r["evidence"]]) for r in rows]
    out.write_text("\n".join(head + lines) + "\n")


def summary(rows: list[dict]) -> str:
    c = collections.Counter((r["cls"], r["kind"], r["c"]) for r in rows)
    return "\n".join(f"{v:5} {k[0]} {k[1]} (C: {k[2]})" for k, v in sorted(c.items()))


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--game", type=Path, required=True)
    ap.add_argument("--gen", type=Path, required=True)
    ap.add_argument("--source", type=Path, required=True, help="the decompilation")
    ap.add_argument("--asm-source", type=Path, help="where its assembly is, if not in --source")
    ap.add_argument("--image", type=Path, help="the disc image (its executable and level programs)")
    ap.add_argument("--exe", type=Path, help="the retail executable, without --image")
    ap.add_argument("--dump", type=Path, help="with --exe: the level dumps (level_NN/manifest.json and records)")
    ap.add_argument("--out", type=Path, required=True)
    args = ap.parse_args(argv)
    cfg = configure(args.game)
    low = (cfg.get("relocate") or [0x15F000])[0]
    if args.image:
        memory = retail.Retail.from_image(args.image, low)
    elif args.exe and args.exe.is_file():
        memory = retail.Retail.from_files(args.exe, args.dump, low)
    else:
        print("no_c: give the disc image or the executable: without them every reference is unknown",
              file=sys.stderr)
        return 1
    rows = classify(cfg, args.gen, args.source, args.asm_source or args.source, memory, keep_rows(args.out))
    write(rows, args.out, cfg.get("title", cfg.get("id", "")))
    print(summary(rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())

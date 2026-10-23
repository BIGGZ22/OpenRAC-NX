#!/usr/bin/env python3
"""What to backfill first: the weak functions gameplay goes through, per level program.

    python3 port/tools/hostgen/priority.py --game port/game/rac1-pal/hostgen.json \\
        --gen port/build/release/games/rac1-pal/gen --dump games/rac1/baserom/overlays \\
        --exe games/rac1/baserom/SCES_509.16 --out build/priority/rac1-pal.md

It reads what hostgen wrote (report.json with its call_graph and call_site_mismatches,
functions.c, stubs.c) and the decompilation's tables, and builds a call graph for each level
program: hostgen's C calls (and addresses taken), plus, when the level dumps are given, the
retail code's own jal, j and lui/addiu targets in that level's copy of each function (which is
how the level hooks, folded copies and variants resolve) and every function a level's vtbl,
camvtbl, sndvtbl or data record points at (the moby class update and handler tables, the
camera and sound tables, menu tables). Roots: the game's roots (hostgen.json), each level's
entry, and those tables.

A weak function is one with no C, C hostgen could not translate, a call that leaves arguments
out (filled with 0), C that is not matched (a candidate or the port's own hand C, weighted by
its confidence line), or matched C its callers reach through a declaration that disagrees with
its definition. Each is ranked: reached in every level or on a hot path first (hero, camera,
collision, HUD, 2D draw, moby update and draw, sound, level load), then by the number of levels
that reach it, then by how weak it is. Static only: nothing is run.
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from collections import deque
from pathlib import Path

sys.path.insert(1, str(next(p for p in Path(__file__).resolve().parents if (p / "tools/openrac_common").is_dir()) / "tools"))
from openrac_common import paths  # noqa: E402

# How weak each kind is: higher goes first among functions reached as widely.
WEIGHT = {
    "no C": 6, "untranslatable": 6, "mislabelled declaration": 5.5, "unverifiable": 5, "functional: low": 4,
    "zero-filled args": 2.5, "functional: medium": 3, "candidate (no confidence line)": 2.5,
    "hand C": 2, "functional: high": 1.5, "call-site mismatch": 1.5,
}
# Hot paths: by the unit a function's source sits in (the executable's unit names, which the
# level programs keep for the shared code; a level's own "vendor" and "help" runs are catch-alls).
UNIT_TAGS = {
    "camera": "camera", "collproc": "collision", "hud": "hud", "drawquad": "2d draw",
    "framebuf": "2d draw", "draw": "draw", "mobyproc": "moby", "mobyfunc": "moby",
    "mobyutil": "moby", "sound": "sound", "music": "sound", "loaders": "level load",
    "bloaders": "level load", "initonce": "level load", "init": "level load",
    "stream": "level load", "pause": "pause", "menu": "pause", "vendor": "vendor",
}
CATCH_ALL = {"vendor", "help"}          # in a level's own files these say nothing
NAME_TAGS = [(re.compile(p), t) for p, t in [
    (r"[Hh]ero", "hero"), (r"[Cc]am(era)?[A-Z_]", "camera"), (r"^[Cc]oll|_coll", "collision"),
    (r"[Hh]ud|HUD", "hud"), (r"Font|Sprite|Quad|2D|Text", "2d draw"), (r"Moby", "moby"),
    (r"Sound|Snd|Music", "sound"), (r"Load|Level", "level load")]]
ROLE_TAGS = [("UpdateMoby", "moby"), ("Camera", "camera"), ("SoundFunc", "sound")]
TRICKY = {"mislabelled declaration", "unverifiable", "zero-filled args", "call-site mismatch", "untranslatable"}
HOT = {"hero", "camera", "collision", "hud", "2d draw", "draw", "moby", "sound", "level load"}
TAG_HOPS = 2                            # a tag reaches this many calls below where it is given


def read_json(path: Path):
    return json.loads(path.read_text())


def read_programs(functions_c: Path) -> dict[int, dict[int, str]]:
    """functions.c: program (-1 the executable, n level n) -> {address: name}; first one wins."""
    programs: dict[int, dict[int, str]] = {}
    cur = None
    for line in functions_c.read_text().splitlines():
        m = re.match(r"static const openrac_fn_entry k_(exe|overlay_(\d+))\[\]", line)
        if m:
            cur = programs.setdefault(-1 if m.group(1) == "exe" else int(m.group(2)), {})
            continue
        m = re.match(r'\s*\{0x([0-9A-Fa-f]+)u, \(openrac_host_fn\)\w+, "(\w+)"\}', line)
        if m and cur is not None:
            cur.setdefault(int(m.group(1), 16), m.group(2))
    return programs


def read_stubs(stubs_c: Path) -> set[str]:
    return set(re.findall(r'openrac_guest_missing\("(\w+)"\)', stubs_c.read_text()))


def read_table(path: Path) -> dict[str, str]:
    out = {}
    if path.is_file():
        for line in path.read_text().splitlines():
            if line.strip() and not line.startswith("#") and "\t" in line:
                name, file = line.split("\t")[:2]
                out[name] = file.strip()
    return out


def confidence(path: Path) -> str:
    """The kind of a candidate from its file's first comment."""
    try:
        head = path.read_text(errors="replace")[:600]
    except OSError:
        return "candidate (no confidence line)"
    first = head.split("*/")[0]
    if re.search(r"unverifiable|VU0 microprogram|vcallms", first, re.I):
        return "unverifiable"
    m = re.search(r"functional:\s*(high|medium|low)", first)
    return f"functional: {m.group(1)}" if m else "candidate (no confidence line)"


def read_units(src: Path) -> dict[str, str]:
    """name -> source unit stem (camera, vendor, ...) and whether it is a level's own file."""
    units = {}
    if not src.is_dir():
        return units
    for f in src.rglob("*.c"):
        stem = re.sub(r"_[0-9A-Fa-f]{8}$", "", f.stem)
        own = f.parent.name.startswith("l") and f.parent.parent.name == "overlays"
        key = f"{stem}@level" if own else stem
        text = f.read_text(errors="replace")
        for name in re.findall(r'INCLUDE_ASM\("[^"]*",\s*(\w+)\)', text):
            units.setdefault(name, key)
        for name in re.findall(r"^[A-Za-z_][\w \t\*]*?\b(func_\w+)\s*\([^;{]*\)\s*\{?\s*$", text, re.M):
            units.setdefault(name, key)
    return units


def mislabelled(prep: Path) -> dict[str, list[str]]:
    """Candidates whose declarations prep.py renamed wrongly: in `extern T a(..), b(..);` it
    renames a to a_hcN and puts a's assembler name after the last declarator, so every call to b
    goes to a. candidate name -> ["b -> a", ...], read from hostgen's prepared copies."""
    out: dict[str, list[str]] = {}
    if not prep.is_dir():
        return out
    for f in prep.rglob("*.c"):
        owner = None
        for line in f.read_text(errors="replace").splitlines():
            m = re.match(r"/\* hostgen: (\w+) from a candidate", line)
            if m:
                owner = m.group(1)
                continue
            m = re.match(r'\s*extern\s[^;]*?\b(\w+_hc\d+)\b(.*)__asm__\("(\w+)"\)[^;]*;', line)
            if not m or owner is None:
                continue
            depth, parts = 0, [""]
            for ch in m.group(2):
                depth += ch == "("
                depth -= ch == ")"
                if ch == "," and depth == 0:
                    parts.append("")
                else:
                    parts[-1] += ch
            if len(parts) > 1:
                last = re.match(r"[\s\*]*(\w+)", parts[-1])
                if last:
                    out.setdefault(owner, []).append(f"{last.group(1)} -> {m.group(3)}")
    return out


def skipped_names(source: Path, cfg: dict) -> set[str]:
    """Functions in the parts of the tree hostgen leaves out (hostgen.json "skip": src/libgcc,
    which the host compiler provides)."""
    out: set[str] = set()
    for d in cfg.get("skip", []):
        out |= set(read_units(source / d))
    return out


def read_readable(names_h: Path) -> dict[str, str]:
    out = {}
    if names_h.is_file():
        for readable, sym in re.findall(r"#define\s+(\w+)\s+(func_\w+)", names_h.read_text()):
            out[sym] = readable
    return out


def read_roles(path: Path) -> dict[str, list[tuple[str, set[int]]]]:
    """config/overlays/names.tsv: name -> [(role, levels)]."""
    out: dict[str, list] = {}
    if path.is_file():
        for line in path.read_text().splitlines():
            if line.startswith("#") or "\t" not in line:
                continue
            name, roles = line.split("\t", 1)
            for r in roles.strip().split(","):
                role, _, levels = r.partition(":")
                out.setdefault(name, []).append((role, {int(x) for x in levels.split("/") if x}))
    return out


class Code:
    """The retail bytes of one program: the executable's below `low`, a level's records above."""

    def __init__(self, exe: Path | None, level_dir: Path | None, low: int):
        self.parts: list[tuple[int, bytes]] = []
        self.records: dict[str, tuple[int, bytes]] = {}
        self.entry = None
        if level_dir is not None and (level_dir / "manifest.json").is_file():
            man = read_json(level_dir / "manifest.json")
            self.entry = man.get("entry_point")
            for r in man["records"]:
                f = level_dir / f"{r['name']}.bin"
                if f.is_file():
                    self.records[r["name"]] = (r["address"], f.read_bytes())
            if "text" in self.records:
                self.parts.append(self.records["text"])
        if exe is not None and exe.is_file():
            b = exe.read_bytes()
            phoff, = struct.unpack_from("<I", b, 0x1C)
            for i in range(struct.unpack_from("<H", b, 0x2C)[0]):
                kind, off, va, _pa, size = struct.unpack_from("<5I", b, phoff + 32 * i)
                if kind == 1:
                    seg = b[off:off + size]
                    if level_dir is not None:
                        seg = seg[:max(0, low - va)]       # the level's records lie over the rest
                    self.parts.append((va, seg))

    def words(self, start: int, end: int):
        for base, data in self.parts:
            if base <= start < base + len(data):
                stop = min(end, base + len(data))
                n = (stop - start) // 4
                return struct.unpack_from(f"<{n}I", data, start - base)
        return ()

    def table_pointers(self, record: str):
        if record not in self.records:
            return ()
        _, data = self.records[record]
        return struct.unpack_from(f"<{len(data) // 4}I", data)


def machine_edges(code: Code, program: dict[int, str], only: set[str] | None = None) -> dict[str, set[str]]:
    """name -> names its retail code in this program calls, jumps to or takes the address of
    (for the names in `only`, when given)."""
    starts = sorted(program)
    edges: dict[str, set[str]] = {}
    for i, a in enumerate(starts):
        if only is not None and program[a] not in only:
            continue
        end = starts[i + 1] if i + 1 < len(starts) else a + 0x1000
        end = min(end, a + 0x20000)
        w = code.words(a, end)
        if not w:
            continue
        out = edges.setdefault(program[a], set())
        hi: dict[int, int] = {}
        for k, x in enumerate(w):
            op = x >> 26
            pc = a + 4 * k
            if op in (2, 3):
                t = (pc & 0xF0000000) | ((x & 0x3FFFFFF) << 2)
                if t in program and t != a:
                    out.add(program[t])
            elif op == 0x0F:
                hi[(x >> 16) & 31] = (x & 0xFFFF) << 16
            elif op in (0x09, 0x0D) and (x >> 21) & 31 in hi:
                imm = x & 0xFFFF
                if op == 0x09 and imm & 0x8000:
                    imm -= 0x10000
                t = (hi[(x >> 21) & 31] + imm) & 0xFFFFFFFF
                if t in program and t != a:
                    out.add(program[t])
    return edges


def sizes_of(programs: dict[int, dict[int, str]]) -> dict[str, int]:
    """A function's size: the distance to the next function, in the first program that has it."""
    out: dict[str, int] = {}
    for level in sorted(programs):
        starts = sorted(programs[level])
        for a, b in zip(starts, starts[1:]):
            out.setdefault(programs[level][a], b - a)
    return out


def tier(weakness: list[str], size: int | None) -> str:
    """Who should take it: Haiku for small plain ones, Sonnet up to 1 KB or for one argument read
    from the assembly, Opus for large or hand-written (VU0) ones."""
    if "unverifiable" in weakness:
        return "Opus"
    if set(weakness) <= {"zero-filled args", "call-site mismatch", "mislabelled declaration"}:
        return "Sonnet"          # one call or one declaration to read against the assembly
    if (size or 0) > 1024:
        return "Opus"
    if (size or 0) > 300 or set(weakness) & TRICKY:
        return "Sonnet"
    return "Haiku"


def bfs(roots: list[tuple[str, int, str]], graph) -> dict[str, tuple[int, str]]:
    """name -> (depth, the root it was reached from), nearest first."""
    seen: dict[str, tuple[int, str]] = {}
    q = deque(roots)
    while q:
        name, depth, root = q.popleft()
        if name in seen:
            continue
        seen[name] = (depth, root)
        for c in sorted(graph(name)):
            if c not in seen:
                q.append((c, depth + 1, root))
    return seen


def analyse(cfg: dict, gen: Path, source: Path, dump: Path | None, exe: Path | None,
            frame: str | None = None) -> dict:
    report = read_json(gen / "report.json")
    if "call_graph" not in report:
        raise SystemExit("priority: report.json has no call_graph; build again with this hostgen")
    cgraph = {k: set(v) for k, v in report["call_graph"].items()}
    programs = read_programs(gen / "functions.c")
    stubs = read_stubs(gen / "stubs.c")
    cand_files = read_table(source / cfg["candidates"]) if cfg.get("candidates") else {}
    used = set(report.get("candidates_used", []))
    hand = read_table(Path(cfg["_dir"]) / cfg["hand"]) if cfg.get("hand") else {}
    host = set(cfg.get("host_functions", []))
    low = (cfg.get("relocate") or [0x15F000])[0]
    units = read_units(source / "src")
    readable = read_readable(source / "include/names.h")
    roles = read_roles(source / "config/overlays/names.tsv")
    roots = cfg.get("roots", [])
    frame = frame or (roots[-1] if roots else None)

    # The weak functions and why.
    weak: dict[str, list[str]] = {}

    def add(name, why):
        weak.setdefault(name, [])
        if why not in weak[name]:
            weak[name].append(why)

    # What the host compiler provides (src/libgcc): the port's C never calls it.
    skipped = skipped_names(source, cfg)
    for name in stubs:
        if name not in host and name not in skipped:
            add(name, "no C")
    for name, why in report.get("stubbed", {}).items():
        add(name, "untranslatable")
    for name in used - skipped - host:
        if name in hand:
            add(name, "hand C")
        elif name in cand_files:
            add(name, confidence(source / cand_files[name]))
    labels = mislabelled(gen / "prep")
    for name in labels:
        add(name, "mislabelled declaration")
    zero = report.get("arguments_filled_with_zero", {})
    for name in zero:
        if name not in host:
            add(name, "zero-filled args")
    mism: dict[str, list[str]] = {}
    for caller, items in report.get("call_site_mismatches", {}).items():
        for item in items:
            callee, _, what = item.partition(": ")
            parts = [w.strip() for w in what.split(";")]
            if all((m := re.match(r"(\d+) \w+ arguments for (\d+)$", w)) and int(m.group(1)) > int(m.group(2))
                   for w in parts):
                continue          # only extra arguments, which the callee never reads: harmless
            mism.setdefault(callee, []).append(f"{caller} ({what})")
    for callee in mism:
        if callee not in weak and callee not in host:
            add(callee, "call-site mismatch")

    # One graph per program: hostgen's calls, and the retail code's own.
    def tags_of(name: str, level: int | None) -> set[str]:
        t = set()
        unit = units.get(name, "")
        stem = unit.split("@")[0]
        if stem in UNIT_TAGS and not (unit.endswith("@level") and stem in CATCH_ALL):
            t.add(UNIT_TAGS[stem])
        r = readable.get(name, "")
        for rx, tag in NAME_TAGS:
            if r and rx.search(r):
                t.add(tag)
        for role, lv in roles.get(name, []):
            if level is None or level in lv:
                for prefix, tag in ROLE_TAGS:
                    if prefix in role:
                        t.add(tag)
        return t

    hooks = set(cfg.get("level_hooks", []))
    hook_callers = {n for n, cs in cgraph.items() if cs & hooks}
    reach: dict[str, dict[int, tuple[int, str]]] = {}
    frame_depth: dict[str, dict[int, int]] = {}
    tags: dict[str, set[str]] = {}
    unresolved: dict[int, int] = {}
    levels_total = sum(1 for p in programs if p >= 0)
    for level in sorted(programs):
        prog = dict(programs[level]) if level < 0 else {
            **{a: n for a, n in programs[-1].items() if a < low}, **programs[level]}
        level_dir = dump / f"level_{level:02d}" if dump is not None and level >= 0 else None
        code = Code(exe, level_dir if level_dir and level_dir.is_dir() else None, low) \
            if (exe or level_dir) else None
        # The retail code's calls only where hostgen's C cannot say where the call goes: a call to
        # a level hook (each level fills it), and functions with no C yet (what they will reach).
        # Elsewhere the C's own calls are what the port runs (the retail code also calls
        # compiler helpers the host does not need).
        medges = machine_edges(code, prog, hook_callers | stubs | set(report.get("stubbed", {}))) \
            if code is not None and code.parts else {}

        def graph(n, medges=medges):
            return cgraph.get(n, set()) | medges.get(n, set())

        rts = [(r, 0, r) for r in roots]
        if code is not None and code.entry in prog:
            rts.append((prog[code.entry], 1, "level entry"))
        table_roots = 0
        if code is not None:
            for rec, depth in (("vtbl", 3), ("camvtbl", 3), ("sndvtbl", 3), ("data", 4), ("lit", 4)):
                for w in code.table_pointers(rec):
                    if w in prog:
                        rts.append((prog[w], depth, rec))
                        table_roots += 1
        if not table_roots and level >= 0:
            # No dump: the roles names.tsv recorded for this level.
            rts += [(n, 3, "vtbl") for n, rl in roles.items() if any(level in lv for _, lv in rl)]
            unresolved[level] = 1
        seen = bfs(rts, graph)
        from_frame = bfs([(frame, 0, frame)], graph) if frame else {}
        for n, (d, root) in seen.items():
            reach.setdefault(n, {})[level] = (d, root)
            if n in from_frame:
                frame_depth.setdefault(n, {})[level] = from_frame[n][0]
        # Hot-path tags, given where they are and carried TAG_HOPS calls down.
        for n in seen:
            t = tags_of(n, level)
            if not t:
                continue
            q = deque([(n, 0)])
            done = set()
            while q:
                m, h = q.popleft()
                if m in done:
                    continue
                done.add(m)
                tags.setdefault(m, set()).update(t)
                if h < TAG_HOPS:
                    q.extend((c, h + 1) for c in graph(m))

    sizes = sizes_of(programs)
    rows = []
    for name, whys in weak.items():
        lv = sorted(level for level in reach.get(name, {}) if level >= 0)
        title = -1 in reach.get(name, {})
        if not lv and not title:
            continue
        hot = sorted(tags.get(name, set()) & HOT)
        depths = [d for level, d in frame_depth.get(name, {}).items() if level >= 0]
        via = sorted({r for level, (d, r) in reach[name].items()})
        score = max(WEIGHT.get(w, 1) for w in whys) + 0.25 * (len(whys) - 1)
        rows.append({
            "name": name, "weakness": whys, "levels": lv, "title": title, "hot": hot,
            "frame_depth": min(depths) if depths else None,
            "depth": min(d for d, _ in reach[name].values()), "via": via, "score": score,
            "readable": readable.get(name, ""), "unit": units.get(name, ""),
            "size": sizes.get(name), "tier": tier(whys, sizes.get(name)),
            "zero_args": zero.get(name, []), "mislabelled": labels.get(name, []), "mismatched_callers": mism.get(name, [])[:4],
        })
    rows.sort(key=lambda r: (-(len(r["levels"]) == levels_total or bool(r["hot"])), -len(r["levels"]),
                             -r["score"], r["frame_depth"] if r["frame_depth"] is not None else 99,
                             r["name"]))
    unreached = sorted(n for n in weak if n not in reach)
    short: dict[str, list[str]] = {}
    for caller, items in zero.items():
        if caller in reach:
            for item in items:
                short.setdefault(item, []).append(caller)
    return {"rows": rows, "unreached": unreached, "levels_without_dump": sorted(unresolved),
            "weak": len(weak), "short_by_callee": short, "levels_total": levels_total}


def levels_text(lv: list[int], title: bool, total: int) -> str:
    if lv and len(lv) == total:
        s = "all"
    elif not lv:
        s = "-"
    elif len(lv) > 6:
        s = f"{len(lv)}: " + ",".join(f"{x}" for x in lv)
    else:
        s = ",".join(f"{x}" for x in lv)
    return s + (" +title" if title else "")


def why_text(r: dict) -> str:
    parts = []
    if r["readable"]:
        parts.append(r["readable"])
    if r["unit"]:
        parts.append(f"in {r['unit'].replace('@level', ' (level file)')}")
    if r["frame_depth"] is not None:
        parts.append(f"{r['frame_depth']} calls below the frame loop")
    else:
        parts.append("reached via " + ", ".join(r["via"][:3]))
    if r["mislabelled"]:
        parts.append("calls go elsewhere: " + ", ".join(r["mislabelled"][:4]))
    if r["zero_args"]:
        parts.append("zero-filled: " + ", ".join(sorted(set(r["zero_args"]))[:3]))
    if r["mismatched_callers"]:
        parts.append("callers disagree: " + "; ".join(r["mismatched_callers"][:2]))
    return "; ".join(parts)


def markdown(result: dict, game: str, top: int | None = None) -> str:
    rows = result["rows"][:top] if top else result["rows"]
    out = [f"# What to backfill first: {game}", "",
           f"{len(result['rows'])} weak functions reached statically (of {result['weak']}); "
           f"{len(result['unreached'])} weak functions no level reaches. Generated by "
           "port/tools/hostgen/priority.py; static only.", "",
           "| # | Function | Weakness | Levels | Hot path | Size | Tier | Why |",
           "|---:|---|---|---|---|---:|---|---|"]
    for i, r in enumerate(rows, 1):
        out.append(f"| {i} | {r['name']} | {', '.join(r['weakness'])} | "
                   f"{levels_text(r['levels'], r['title'], result['levels_total'])} | {', '.join(r['hot']) or '-'} | "
                   f"{r['size'] or '?'} | {r['tier']} | {why_text(r)} |")
    if result["levels_without_dump"]:
        out += ["", "Levels read without their dump (roles from names.tsv only): "
                + ", ".join(map(str, result["levels_without_dump"]))]
    if result.get("short_by_callee"):
        out += ["", "## Calls short of arguments, by callee", "",
                "One declaration behind many rows above: check the argument against the assembly once.", "",
                "| Callee and argument | Callers reached |", "|---|---:|"]
        out += [f"| {k} | {len(v)} |" for k, v in
                sorted(result["short_by_callee"].items(), key=lambda kv: -len(kv[1]))]
    out += ["", "## Weak functions no level reaches", "",
            "Nothing the analysis follows calls them: dead code, or a table it cannot see.", "",
            ", ".join(result["unreached"]) or "none", ""]
    return "\n".join(out)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--game", type=Path, required=True, help="the game's hostgen.json")
    ap.add_argument("--gen", type=Path, required=True, help="hostgen's output (report.json, functions.c, stubs.c)")
    ap.add_argument("--source", type=Path, help="the decompilation (default: hostgen.json's source)")
    ap.add_argument("--dump", type=Path, help="the level dumps (level_NN/manifest.json and records)")
    ap.add_argument("--exe", type=Path, help="the executable, for its code below the level records")
    ap.add_argument("--frame", help="the per-frame loop (default: the last of hostgen.json's roots)")
    ap.add_argument("--out", type=Path, help="the Markdown report (default: print it)")
    ap.add_argument("--json", type=Path, help="the ranked rows as JSON as well")
    ap.add_argument("--top", type=int, help="only the first N rows")
    args = ap.parse_args(argv)
    cfg = read_json(args.game)
    cfg["_dir"] = str(args.game.parent)
    source = args.source or paths.repo_root(args.game) / cfg.get("source", ".")
    result = analyse(cfg, args.gen, source, args.dump, args.exe, args.frame)
    text = markdown(result, cfg.get("title", cfg.get("id", "")), args.top)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(result, indent=1) + "\n")
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text)
        print(f"priority: {len(result['rows'])} weak functions ranked; {args.out}")
    else:
        print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())

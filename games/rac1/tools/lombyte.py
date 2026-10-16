#!/usr/bin/env python3
"""
Finds a PAL function's counterpart in Lombyte, the matching decompilation
of the same game's US build (github.com/mateuszklysz/Lombyte, MIT;
docs/SIBLING_DECOMPS.md). A function Lombyte has matched is the best
starting point there is: same source, same compiler family.

  python3 tools/lombyte.py map              # rebuild the US-to-PAL map
  python3 tools/lombyte.py func_X [...]     # counterpart, status, C file
  python3 tools/lombyte.py NAME [...]       # a Lombyte name's PAL function
  python3 tools/lombyte.py todo             # matched there, not here

The map pairs functions through config/overlays/us_map.tsv when it
exists (tools/overlays.py us-map: every US function's PAL counterpart,
found from the code): a Lombyte function's US address, in the executable
or in level NN for FUN_LNN_xxxxxxxx, gives our function. What that
leaves, or everything without it, is paired by aligning both builds'
function-size sequences (runs of three or more equal sizes), so such a
pair has the same size in both. Level code is aligned level by level:
Lombyte's FUN_LNN_xxxxxxxx against every function
config/overlays/functions.tsv places in level NN (its shared code is
named after level 00, as ours is). Each pair records how it was found
("us_map" or "sizes"). It lives in build-sn/lombyte_ntsc_pal_map.json.
Lombyte is looked for in $LOMBYTE, else next to this tree inside OpenRAC
(games/rac1/ntsc), else ~/Projects/Lombyte.
"""
from __future__ import annotations
import difflib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
# Inside OpenRAC, Lombyte is this game's other region, games/rac1/ntsc, and
# OpenRAC keeps a copy of the report its CI publishes (tools/openrac.py progress --fetch).
OPENRAC = ROOT.parents[2] if (ROOT.parents[2] / "games/rac1/game.json").exists() else None
LOMBYTE = Path(os.environ.get("LOMBYTE") or (ROOT.parent / "ntsc" if OPENRAC else Path.home() / "Projects/Lombyte"))
MAP = ROOT / "build-sn/lombyte_ntsc_pal_map.json"
US_MAP = ROOT / "config/overlays/us_map.tsv"
US_EXE_DELTA = 0xFF080      # SCUS-97199: vram - file offset of its one loaded segment
LEVEL_NAME = re.compile(r"FUN_L(\d\d)_([0-9a-fA-F]{8})")


def their_report(lombyte: Path = LOMBYTE) -> dict | None:
    """Lombyte's progress report, or None. It was committed as
    progress/report.json until their #64; since then their CI publishes
    it as report.json on the `progress` branch, and their
    scripts/gen_progress_report.py writes build/progress/report.json; inside
    OpenRAC, progress/sources/rac1-ntsc.json holds a copy of the published one."""
    copies = [OPENRAC / "progress/sources/rac1-ntsc.json"] if OPENRAC else []
    for path in (lombyte / "progress/report.json", lombyte / "build/progress/report.json", *copies):
        if path.exists():
            return json.loads(path.read_text())
    for ref in ("origin/progress", "progress"):
        shown = subprocess.run(["git", "-C", str(lombyte), "show", f"{ref}:report.json"],
                               capture_output=True, text=True)
        if shown.returncode == 0:
            return json.loads(shown.stdout)
    return None


def their_units() -> list[dict]:
    report = their_report()
    if report is None:
        sys.exit(f"no Lombyte progress report in {LOMBYTE} (progress/report.json, "
                 "build/progress/report.json or its progress branch): fetch it, or run its "
                 "scripts/gen_progress_report.py")
    return report["units"]


def functions(units: list[dict], key) -> list[tuple[int, int, str, bool]]:
    rows = []
    for unit in units:
        # Both reports also hold level overlay code (Lombyte's level_NN/...
        # and shared/..., our overlays/...): those addresses are in the
        # levels' own address space, and would corrupt the alignment.
        cats = (unit.get("metadata") or {}).get("progress_categories", [])
        if "level_code" in cats or "overlays" in cats or unit["name"].split("/")[0] in ("shared", "overlays") \
                or unit["name"].startswith("level_"):
            continue
        for f in unit.get("functions", []):
            rows.append((key(f), int(f["size"]), f["name"], (f.get("fuzzy_match_percent") or 0) == 100))
    return sorted(r for r in rows if r[0] is not None)


def size_runs(theirs: list, ours: list):
    """(theirs[i], ours[j]) along runs of three or more equal sizes (the
    second field of each) in both sequences."""
    match = difflib.SequenceMatcher(None, [x[1] for x in theirs], [x[1] for x in ours], autojunk=False)
    for a, b, n in match.get_matching_blocks():
        if n >= 3:
            for k in range(n):
                yield theirs[a + k], ours[b + k]


def level_functions(units: list[dict]) -> dict[int, list[tuple[int, int, str, bool]]]:
    """level -> Lombyte's FUN_LNN_xxxxxxxx there: (US address, size, name, matched)."""
    theirs: dict[int, list] = {}
    for unit in units:
        for f in unit.get("functions", []):
            m = LEVEL_NAME.fullmatch(f["name"])
            if m:
                theirs.setdefault(int(m.group(1)), []).append(
                    (int(m.group(2), 16), int(f["size"]), f["name"], (f.get("fuzzy_match_percent") or 0) == 100))
    return {level: sorted(fns) for level, fns in theirs.items()}


def read_us_map() -> dict[tuple[str, int], str]:
    """(program, US address) -> our function, from config/overlays/us_map.tsv
    (tools/overlays.py us-map), or {} without it. Program is "boot" or
    "level_NN"."""
    if not US_MAP.exists():
        return {}
    rows = {}
    for line in US_MAP.read_text().splitlines():
        if not line.startswith("#"):
            program, addr, _size, pal, _method, _sim = line.split("\t")
            if pal != "-":
                rows[(program, int(addr, 16))] = pal
    return rows


def definition(ntsc: str) -> tuple[str, str] | None:
    """(file, C text) of Lombyte's definition of NTSC: from its first line
    to the closing brace at column 0, with the comment right above it."""
    for path in source_of(ntsc):
        lines = Path(path).read_text(errors="replace").splitlines()
        for i, line in enumerate(lines):
            if re.match(rf"(?!extern\b)[A-Za-z_][^;]*\b{re.escape(ntsc)}\b\s*\(", line) and not line.rstrip().endswith(";"):
                start = i
                while start > 0 and lines[start - 1].strip().startswith(("/*", "*", "//")):
                    start -= 1
                end = next((j for j in range(i, len(lines)) if lines[j].startswith("}")), None)
                if end is not None:
                    return path, "\n".join(lines[start:end + 1]) + "\n"
    return None


def build_map() -> list[dict]:
    """Pairs through config/overlays/us_map.tsv first, then by sizes (see the
    module docstring). One pair per PAL function; when two Lombyte functions
    reach the same one, the one Lombyte matched is kept."""
    sys.path.insert(0, str(ROOT / "tools"))
    import dossier
    units = their_units()
    report = json.loads((ROOT / "progress/report.json").read_text())["units"]
    exact = {f["name"]: (f.get("fuzzy_match_percent") or 0) == 100 for u in report for f in u.get("functions", [])}
    mapped = read_us_map()
    # Lombyte's report gives the executable's functions at their offset in
    # the file (its "virtual_address" of CheckStateRange, 0x112380, is 0x13300).
    # Take whichever reading puts more of them on a US function start.
    theirs = functions(units, lambda f: int(f.get("metadata", {}).get("virtual_address") or 0))
    if sum(("boot", a + US_EXE_DELTA) in mapped for a, *_ in theirs) >= sum(("boot", a) in mapped for a, *_ in theirs):
        theirs = [(a + US_EXE_DELTA, *rest) for a, *rest in theirs]
    ours = functions(report,
                     lambda f: int(f["name"][5:], 16) if re.fullmatch(r"func_[0-9A-Fa-f]{8}", f["name"]) else None)
    theirs_lv = level_functions(units)
    ours_lv: dict[int, list] = {}
    sizes = {name: size for _, size, name, _ in ours}
    for name, (_kind, size, _n, places) in dossier.load_overlay_catalogue().items():
        sizes.setdefault(name, size)
        for level, addr in places:
            ours_lv.setdefault(level, []).append((addr, size, name))
    pairs: dict[str, dict] = {}

    def add(pal: str, ntsc: str, ntsc_size: int, ntsc_exact: bool, how: str, overlay: bool) -> None:
        old = pairs.get(pal)
        if old and (old["ntsc_exact"] or not ntsc_exact):
            return
        pairs[pal] = {"pal": pal, "size": sizes.get(pal, 0), "ntsc": ntsc, "ntsc_size": ntsc_size,
                      "ntsc_exact": ntsc_exact, "pal_exact": exact.get(pal, False), "how": how,
                      **({"overlay": True} if overlay else {})}

    for addr, size, name, ok in theirs:
        if ("boot", addr) in mapped:
            add(mapped[("boot", addr)], name, size, ok, "us_map", False)
    for level, fns in theirs_lv.items():
        for addr, size, name, ok in fns:
            if (f"level_{level:02d}", addr) in mapped:
                add(mapped[(f"level_{level:02d}", addr)], name, size, ok, "us_map", True)
    paired = {p["ntsc"] for p in pairs.values()}
    for t, o in size_runs(theirs, ours):
        if t[2] not in paired and o[2] not in pairs:
            add(o[2], t[2], t[1], t[3], "sizes", False)
    for level in sorted(theirs_lv):
        for t, o in size_runs(theirs_lv[level], sorted(ours_lv.get(level, []))):
            if t[2] not in paired and o[2] not in pairs:
                add(o[2], t[2], t[1], t[3], "sizes", True)
    MAP.parent.mkdir(parents=True, exist_ok=True)
    MAP.write_text(json.dumps(list(pairs.values()), indent=1) + "\n")
    return list(pairs.values())


def load_map() -> list[dict]:
    return json.loads(MAP.read_text()) if MAP.exists() else build_map()


def source_of(name: str) -> list[str]:
    """Lombyte C files that define NAME (as a C name or an asm label):
    a column-0 line naming it before `(` and not ending in `;`."""
    pattern = re.compile(rf"(?m)^(?!extern\b)[A-Za-z_][^;\n]*\b{re.escape(name)}\b\s*\([^;\n]*$"
                         rf"|__asm__\(\"{re.escape(name)}\"\)")
    hits = []
    for path in (LOMBYTE / "src").rglob("*.c"):
        if pattern.search(path.read_text(errors="replace")):
            hits.append(str(path))
    return hits


def main() -> None:
    args = sys.argv[1:]
    if not args:
        sys.exit(__doc__)
    if not LOMBYTE.is_dir():
        sys.exit(f"no Lombyte checkout at {LOMBYTE} (set $LOMBYTE)")
    if args[0] == "map":
        pairs = build_map()
        todo = [p for p in pairs if p["ntsc_exact"] and not p["pal_exact"]]
        how = {h: sum(1 for p in pairs if p["how"] == h) for h in ("us_map", "sizes")}
        print(f"{len(pairs)} functions paired ({how['us_map']} through {US_MAP.relative_to(ROOT)}, "
              f"{how['sizes']} by sizes); {len(todo)} matched in Lombyte only "
              f"({sum(p['size'] for p in todo)} bytes). Written to {MAP.relative_to(ROOT)}.")
        return
    pairs = load_map()
    if args[0] == "todo":
        for p in sorted((p for p in pairs if p["ntsc_exact"] and not p["pal_exact"]), key=lambda p: p["size"]):
            there = p.get("ntsc_size", p["size"])
            print(f"{p['pal']}  {p['size']:>5}  {p['ntsc']}" + (f"  ({there} bytes there)" if there != p["size"] else ""))
        return
    by_pal = {p["pal"]: p for p in pairs}
    by_ntsc = {p["ntsc"]: p for p in pairs}
    for name in args:
        if not name.startswith("func_"):
            p = by_ntsc.get(name)
            print(f"{name}: " + (f"PAL {p['pal']} ({p['size']} bytes)" if p else "no PAL counterpart in the map"))
            continue
        p = by_pal.get(name)
        if not p:
            print(f"{name}: no Lombyte counterpart in the map")
            continue
        state = "matched in Lombyte" if p["ntsc_exact"] else "not matched in Lombyte either"
        files = source_of(p["ntsc"])
        there = p.get("ntsc_size", p["size"])
        print(f"{name}: Lombyte {p['ntsc']} ({p['size']} bytes" + (f", {there} there" if there != p["size"] else "")
              + f"), {state}")
        for f in files:
            print(f"  {f}")


if __name__ == "__main__":
    main()

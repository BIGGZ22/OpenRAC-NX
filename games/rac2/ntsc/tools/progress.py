#!/usr/bin/env python3
"""
How much of the game's code is C: progress/report.json, in objdiff's report format (the one
decomp.dev reads, as rac1-decomp's), and the table in README.md.

  python3 tools/progress.py           # after tools/build.py all
  python3 tools/progress.py --check   # CI: fail if src/ and the committed report disagree

Every distinct function counts once: the boot executable's functions, and each level function
of config/levels/functions.tsv that the boot does not also have (a function many levels share
is one function). A function is matched when its C linked into at least one program that came
out equal to the retail one (build/<program>/placed.json, written by tools/build.py).

Units are the files of src/, with their functions in address order; categories are the boot
executable and the level code. src/ holds only C that links (a near miss goes to nonmatching/), so
--check, which needs neither the toolchain nor the game, requires the functions src/ has as C to be
exactly the report's matched ones.
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import levels  # noqa: E402

REPORT = ROOT / "progress/report.json"
NAME = r"(?:func_(?:L\d\d_)?[0-9A-F]{8}|sceIpuRestartDMA|_sysbitFlush)"
DEFINITION = re.compile(rf"^(?:[A-Za-z_][^;=#]*?\b)?({NAME})\s*\(")
MARKER = re.compile(rf'^(?:INCLUDE_ASM|ASM_FUNC|LINKER_REMNANT)\("[^"]*",\s*({NAME})\);')


def source_functions() -> dict[str, list[tuple[str, bool]]]:
    """src/ file -> [(function, has C)] in file order."""
    out = {}
    for path in sorted((ROOT / "src").rglob("*.c")):
        found = []
        for line in path.read_text(errors="replace").splitlines():
            m = MARKER.match(line)
            if m:
                found.append((m.group(1), False))
                continue
            m = DEFINITION.match(line)
            if m and not line.rstrip().endswith(";") and not line.startswith(("extern", "typedef")):
                found.append((m.group(1), True))
        out[path.relative_to(ROOT).as_posix()] = found
    return out


def measures(fns: list[dict]) -> dict:
    total = sum(f["size"] for f in fns)
    matched = sum(f["size"] for f in fns if f["matched"])
    complete = total if fns and all(f["matched"] for f in fns) else 0
    pct = lambda a, b: 100.0 * a / b if b else 0.0
    return {"fuzzy_match_percent": pct(matched, total), "total_code": str(total),
            "matched_code": str(matched), "matched_code_percent": pct(matched, total),
            "total_functions": len(fns), "matched_functions": sum(1 for f in fns if f["matched"]),
            "matched_functions_percent": pct(sum(1 for f in fns if f["matched"]), len(fns)),
            "complete_code": str(complete), "complete_code_percent": pct(complete, total)}


def check() -> None:
    report = json.loads(REPORT.read_text())
    matched = {f["name"] for u in report["units"] for f in u["functions"] if f["fuzzy_match_percent"] == 100.0}
    in_c = {n for fns in source_functions().values() for n, c in fns if c}
    problems = sorted(in_c ^ matched)
    if problems:
        sys.exit(f"progress/report.json is out of date for {len(problems)} function(s), e.g. "
                 f"{', '.join(problems[:5])}: run tools/build.py all, then tools/progress.py")
    print(f"progress/report.json is current: {len(matched)} matched functions")


def main() -> None:
    if "--check" in sys.argv:
        check()
        return
    placed = set()
    programs = ["boot"] + [levels.level_dir(n) for n in sorted(levels.NAMES)]
    for program in programs:
        path = ROOT / "build" / program / "placed.json"
        if not path.is_file():
            sys.exit(f"no build of {program}: run tools/build.py all first")
        placed |= set(json.loads(path.read_text())["functions"])
    where = {n: (a, s) for n, a, s in levels.read_boot_functions()}
    for name, kind, size, _, places in levels.read_catalogue():
        if kind != "boot":
            where[name] = (places[0][1], size)
    units, in_c = [], set()
    for path, fns in source_functions().items():
        if not fns:
            continue
        rows = []
        base = where[fns[0][0]][0]
        for name, has_c in fns:
            address, size = where[name]
            if has_c:
                in_c.add(name)
            rows.append({"name": name, "size": size, "matched": has_c and name in placed, "address": address})
        category = "executable" if path.startswith(("src/core/", "src/game/")) else "level_code"
        m = measures(rows)
        units.append({"name": path.removeprefix("src/").removesuffix(".c"),
                      "measures": {**m, "total_units": 1, "complete_units": int(m["complete_code"] == m["total_code"])},
                      "functions": [{"name": r["name"], "size": str(r["size"]),
                                     "fuzzy_match_percent": 100.0 if r["matched"] else 0.0,
                                     "address": str(r["address"] - base),
                                     "metadata": {"virtual_address": str(r["address"])}} for r in rows],
                      "metadata": {"complete": m["complete_code"] == m["total_code"], "source_path": path,
                                   "progress_categories": [category]},
                      "_rows": rows, "_category": category})
    allrows = [r for u in units for r in u["_rows"]]
    cats = []
    for cid, cname in (("executable", "Boot executable"), ("level_code", "Level code")):
        rows = [r for u in units if u["_category"] == cid for r in u["_rows"]]
        cats.append({"id": cid, "name": cname, "measures": measures(rows)})
    total = measures(allrows)
    total.update({"total_units": len(units), "complete_units": sum(u["measures"]["complete_units"] for u in units)})
    for u in units:
        del u["_rows"], u["_category"]
    report = {"version": 2, "measures": total, "categories": cats, "units": units}
    if in_c - placed:
        print(f"{len(in_c - placed)} functions have C in src/ that does not link: move them to nonmatching/ "
              f"({', '.join(sorted(in_c - placed)[:5])})", file=sys.stderr)
    REPORT.parent.mkdir(exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=1) + "\n")

    # README table
    placements = sum(len(json.loads((ROOT / "build" / p / "placed.json").read_text())["functions"])
                     for p in programs[1:])
    row = lambda label, m: (f"| {label} | {m['matched_functions']:,} of {m['total_functions']:,} | "
                            f"{int(m['matched_code']):,} of {int(m['total_code']):,} bytes |")
    table = "\n".join(["| | Functions | Code |", "| --- | ---: | ---: |",
                       row("Boot executable", cats[0]["measures"]), row("Levels", cats[1]["measures"]),
                       row("**Total**", total)[:-2] + f" ({total['matched_code_percent']:.2f}%) |", "",
                       f"The 27 levels hold {sum(len(r[4]) for r in levels.read_catalogue()):,} function "
                       f"placements; C covers {placements:,} of them."])
    readme = ROOT / "README.md"
    text = readme.read_text()
    a, b = text.index("<!-- progress -->") + len("<!-- progress -->"), text.index("<!-- /progress -->")
    readme.write_text(text[:a] + "\n" + table + "\n" + text[b:])
    print(f"{total['matched_functions']} of {total['total_functions']} functions, {total['matched_code']} of "
          f"{total['total_code']} bytes ({total['matched_code_percent']:.2f}%)")


if __name__ == "__main__":
    main()

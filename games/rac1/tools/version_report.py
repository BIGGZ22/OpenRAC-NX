#!/usr/bin/env python3
"""
Progress report of another version, in objdiff's report format (version 2), for ByteMatch.

  python3 tools/version_report.py --version jp           # progress/jp/report.json
  python3 tools/version_report.py --check                # CI: every version's report is current
  python3 tools/version_report.py                        # every integrated version but the primary

Like the primary's report (tools/gen_progress_report.py), it needs neither the toolchain nor the
disc: it counts from committed tables. The version's functions are config/versions/<K>/functions.tsv
(tools/version_map.py: every distinct function once, as the primary's catalogue counts them), and
their verdicts config/versions/<K>/map.tsv (tools/version_check.py): a function is finished when
its C (shared, or the version's own) builds to this version's bytes (match), or it is original
assembly whose bytes are the primary's (asm), or a later piece of a joined function that matched;
everything else counts with the closeness the check measured (0 where the code differs in size).

Units and categories follow the primary's report: an executable function sits in the unit of its
counterpart there (code only this version has, in the unit of the function before it), the shared
level code is one unit and each level one unit named after it. Function names are the counterpart's
(the one C source both build from), or func_<TAG>_... for code only this version has.
"""
import argparse
import bisect
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import versions  # noqa: E402
import version_map  # noqa: E402
from levels import NUM_LEVELS, title as level_title, dirname as level_dirname  # noqa: E402

SHARED_TITLE = "Shared level code"
FINISHED = {"match", "asm", "joined"}


def read_map(v):
    out = {}
    p = v.config / "map.tsv"
    for line in p.read_text().splitlines():
        if line and not line.startswith("#"):
            n, s, st, place, z, c = line.split("\t")
            out[n] = (st, float(c))
    return out


def measures(items, units=0, complete_units=0, complete_code=0):
    total = sum(s for s, _, _ in items)
    matched = sum(s for s, _, e in items if e)
    nfun = len(items)
    nmatch = sum(1 for _, _, e in items if e)
    fuzzy = sum(s * p for s, p, _ in items) / total if total else 0.0
    m = {
        "fuzzy_match_percent": fuzzy,
        "total_code": str(total),
        "matched_code": str(matched),
        "matched_code_percent": 100.0 * matched / total if total else 0.0,
        "total_functions": nfun,
        "matched_functions": nmatch,
        "matched_functions_percent": 100.0 * nmatch / nfun if nfun else 0.0,
        "complete_code": str(complete_code),
        "complete_code_percent": 100.0 * complete_code / total if total else 0.0,
    }
    if units:
        m["total_units"] = units
        m["complete_units"] = complete_units
    return m


def generate(v):
    primary = json.loads(versions.PRIMARY.report.read_text())
    exe_unit = {}
    unit_meta = {}
    for u in primary["units"]:
        cats = u["metadata"]["progress_categories"]
        unit_meta[u["name"]] = u["metadata"]
        if "executable" in cats:
            for f in u["functions"]:
                exe_unit[f["name"]] = u["name"]
    verdict = read_map(v)
    entries = version_map.read(v)

    def score(e):
        st, close = verdict.get(e["name"], ("none", 0.0))
        if st in FINISHED:
            return 100.0, True
        return min(close, 99.99), False

    units = {}          # name -> {"meta", "rows": [(name, addr, size, pct, done)]}
    exe = sorted((e for e in entries if e["kind"] == "exe"), key=lambda e: dict(e["places"])["boot"])
    last = None
    for e in exe:
        addr = dict(e["places"])["boot"]
        uname = exe_unit.get(e["counterpart"] if e["name"] == e["counterpart"] else e["name"]) \
            or exe_unit.get(e["counterpart"] or "") or last or next(iter(exe_unit.values()))
        last = uname
        u = units.setdefault(uname, {"meta": unit_meta[uname], "rows": []})
        u["rows"].append((e["name"], addr, e["size"], *score(e)))
    for e in sorted((e for e in entries if e["kind"] != "exe"), key=lambda e: e["places"][0]):
        lv, addr = min(e["places"], key=lambda p: (p[0], p[1]))
        if e["kind"] == "shared":
            uname, meta = SHARED_TITLE, {"source_path": "src/overlays/shared",
                                         "progress_categories": ["level_code", "common"]}
        else:
            uname, meta = level_title(lv), {"source_path": f"src/overlays/{level_dirname(lv)}",
                                            "progress_categories": ["level_code", "levels", f"level_{lv:02d}"]}
        u = units.setdefault(uname, {"meta": meta, "rows": []})
        u["rows"].append((e["name"], addr, e["size"], *score(e)))

    order = [u["name"] for u in primary["units"]]
    names = sorted(units, key=lambda n: order.index(n) if n in order else len(order))
    out_units, cat_items, cat_complete = [], {}, {}
    all_items, complete_units, complete_code = [], 0, 0
    for uname in names:
        u = units[uname]
        rows = sorted(u["rows"], key=lambda r: r[1])
        start = rows[0][1]
        items = [(s, p, d) for _, _, s, p, d in rows]
        complete = all(d for _, _, d in items)
        code = sum(s for s, _, _ in items)
        cats = u["meta"]["progress_categories"]
        if complete:
            complete_units += 1
            complete_code += code
        all_items += items
        for c in cats:
            cat_items.setdefault(c, []).extend(items)
            cat_complete[c] = cat_complete.get(c, 0) + (code if complete else 0)
        out_units.append({
            "name": uname,
            "measures": measures(items, 1, int(complete), code if complete else 0),
            "functions": [{"name": n, "size": str(s), "fuzzy_match_percent": p, "address": str(a - start),
                           "metadata": {"virtual_address": str(a)}} for n, a, s, p, _ in rows],
            "metadata": {"complete": complete, "source_path": u["meta"]["source_path"],
                         "progress_categories": cats},
        })
    exe_items = cat_items.get("game", []) + cat_items.get("libgcc", [])
    categories = [
        {"id": "executable", "name": "Executable",
         "measures": measures(exe_items, complete_code=cat_complete.get("game", 0) + cat_complete.get("libgcc", 0))},
        {"id": "level_code", "name": "Level code",
         "measures": measures(cat_items.get("level_code", []), complete_code=cat_complete.get("level_code", 0))},
        {"id": "game", "name": "Executable game code",
         "measures": measures(cat_items.get("game", []), complete_code=cat_complete.get("game", 0))},
        {"id": "libgcc", "name": "libgcc",
         "measures": measures(cat_items.get("libgcc", []), complete_code=cat_complete.get("libgcc", 0))},
        {"id": "common", "name": SHARED_TITLE,
         "measures": measures(cat_items.get("common", []), complete_code=cat_complete.get("common", 0))},
        {"id": "levels", "name": "Level-specific code",
         "measures": measures(cat_items.get("levels", []), complete_code=cat_complete.get("levels", 0))},
    ] + [{"id": f"level_{i:02d}", "name": level_title(i),
          "measures": measures(cat_items.get(f"level_{i:02d}", []), complete_code=cat_complete.get(f"level_{i:02d}", 0))}
         for i in range(NUM_LEVELS)]
    return {"measures": measures(all_items, len(out_units), complete_units, complete_code),
            "units": out_units, "version": 2, "categories": categories}


def text(report):
    return json.dumps(report, indent=1) + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", action="append")
    ap.add_argument("--check", action="store_true")
    a = ap.parse_args()
    vs = [versions.get(k) for k in a.version] if a.version else [v for v in versions.INTEGRATED if not v.primary]
    bad = 0
    for v in vs:
        r = generate(v)
        m = r["measures"]
        line = (f"{v.id}: {m['matched_functions']}/{m['total_functions']} functions, "
                f"{m['matched_code_percent']:.2f}% code ({m['matched_code']}/{m['total_code']} bytes)")
        if a.check:
            if not v.report.exists() or v.report.read_text() != text(r):
                print(f"*** {v.report.relative_to(ROOT)} is out of date: python3 tools/version_report.py --version {v.key}")
                bad += 1
            else:
                print(f"report is current: {line}")
        else:
            v.report.parent.mkdir(parents=True, exist_ok=True)
            v.report.write_text(text(r))
            print(f"wrote {v.report.relative_to(ROOT)}: {line}")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()

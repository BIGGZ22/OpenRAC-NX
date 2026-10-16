#!/usr/bin/env python3
"""
Sorts every function that is not exact yet into one work queue each:

  sonnet   one function per task for the cheaper worker: a library function
           whose original source is on hand as an answer key
  opus     game code and near-misses, grouped by source file so a worker
           reads each file's context once
  blocked  a blocker tools/rank_candidates.py has confirmed: it needs new
           tooling, not another attempt

The routes follow what batches cost on 2026-09-24 (docs/DECOMP_PROGRESS.md):
Sonnet matched library code with a reference source for 60-100K tokens a
function but 0 of 15 game functions, while Opus matched game code for
37-79K. Sonnet is half Opus's price per token, so it only pays off where it
has an answer key.

Inputs:
  progress/report.json       exact, near-miss (partial) or untouched (0%)
  asm/nonmatchings/          stub bodies, for rank_candidates' blocker rules
  build-sn/try/*/RESULT.md   the last attempt's verdict, when there was one
  build-sn/mig/sdkmap_by_object.txt   local map of core functions to Sony
                             library symbols (optional; stub name comments
                             in src/ are used as well)
  build-sn/ref/              local reference sources: newlib, MSSG
                             mpeg2decode (optional)

  python3 tools/triage.py                 # summary
  python3 tools/triage.py sonnet          # one route's queue (sonnet, opus, blocked)
  python3 tools/triage.py --json          # every function, for scripts
"""
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

import rank_candidates  # noqa: E402  (needs tools/ on the path)

ROUTES = ("sonnet", "opus", "blocked")
SDK_MAP = ROOT / "build-sn/mig/sdkmap_by_object.txt"
REFERENCES = ROOT / "build-sn/ref"
# A name comment is only a name: `/* ParseBin */` or `/* ParseBin(void) */`, not prose.
NAME_COMMENT = re.compile(r"INCLUDE_ASM\([^)]*\b(func_[0-9A-Fa-f]{8})\);[^\S\n]*/\*[^\S\n]*([A-Za-z_]\w*)[^\S\n]*(?:\(|\*/)")


def report_functions() -> list[dict]:
    """Every function in the progress report that is not exact yet."""
    report = json.loads((ROOT / "progress/report.json").read_text())
    out = []
    for unit in report["units"]:
        for f in unit.get("functions", []):
            percent = f.get("fuzzy_match_percent", 0) or 0
            if percent < 100:
                out.append({"name": f["name"], "unit": unit["name"], "size": int(f["size"]),
                            "status": "near-miss" if percent else "stub",
                            "match_percent": round(percent, 1)})
    return out


def sdk_symbols() -> dict[str, tuple[str, str]]:
    """func name -> (library:object, symbol) from the local map and name comments."""
    names = {}
    if SDK_MAP.exists():
        for line in SDK_MAP.read_text().splitlines():
            parts = line.split()
            if len(parts) >= 4 and parts[0].startswith("func_") and ":" in parts[2]:
                names[parts[0]] = (parts[2], parts[3])
    for path in (ROOT / "src").rglob("*.c"):
        for name, symbol in NAME_COMMENT.findall(path.read_text(errors="replace")):
            names.setdefault(name, ("", symbol))
    return names


# A definition starts in column 0: newlib's `_DEFUN (name,` or a type and
# `name(` on one line that is not a prototype (ending in `;`).
DEFINITION = re.compile(r"(?m)^(?:_DEFUN[^\S\n]*\([^\S\n]*([A-Za-z_]\w*)[^\S\n]*,"
                        r"|(?:[A-Za-z_][\w*]*[^\S\n]+\**)*([A-Za-z_]\w*)[^\S\n]*\((?![^\n]*;[^\S\n]*$))")
# newlib's malloc defines mALLOc and friends, renamed by `#define mALLOc _malloc_r`.
ALIAS = re.compile(r"(?m)^#[^\S\n]*define[^\S\n]+([A-Za-z_]\w*)[^\S\n]+([A-Za-z_]\w*)[^\S\n]*$")


def reference_index() -> dict[str, str]:
    """symbol -> reference source file that defines it (C sources only)."""
    index = {}
    if not REFERENCES.is_dir():
        return index
    for path in sorted(REFERENCES.rglob("*.c")):
        text = path.read_text(errors="replace")
        defined = {a or b for a, b in DEFINITION.findall(text)}
        defined |= {target for name, target in ALIAS.findall(text) if name in defined}
        for symbol in defined:
            index.setdefault(symbol, str(path.relative_to(ROOT)))
    return index


def last_attempt(name: str) -> str:
    result = ROOT / "build-sn/try" / name / "RESULT.md"
    if not result.exists():
        return ""
    first = result.read_text(errors="replace").strip().splitlines()
    return first[0].strip()[:60] if first else ""


def triage() -> list[dict]:
    symbols, references = sdk_symbols(), reference_index()
    attempted = rank_candidates.already_attempted()
    rows = []
    for f in report_functions():
        library, symbol = symbols.get(f["name"], ("", ""))
        reference = references.get(symbol, "") if symbol else ""
        f.update(library=library, symbol=symbol, reference=reference,
                 last_attempt=last_attempt(f["name"]))
        if f["status"] == "near-miss":
            f.update(route="opus", reason=f"near-miss, {f['match_percent']}%")
        else:
            seg = "text" if f["unit"].startswith("game/") else "core_text"
            asm = ROOT / "asm/nonmatchings" / seg / f"{f['name']}.s"
            if not asm.exists():
                f.update(route="blocked", reason="no asm (run tools/setup_asm.sh)")
                rows.append(f)
                continue
            verdict, category, detail = rank_candidates.classify(
                f["name"], asm.read_text(errors="replace"), seg, f["size"])
            if verdict == "blocked":
                f.update(route="blocked", reason=category)
            else:
                notes = [n for n in (category if verdict == "risky" else "", detail,
                                     "attempted before" if f["name"] in attempted else "") if n]
                f.update(route="sonnet" if reference else "opus", reason="; ".join(notes))
        rows.append(f)
    return rows


def show_summary(rows: list[dict]) -> None:
    stubs = sum(r["status"] == "stub" for r in rows)
    print(f"{stubs} stubs and {len(rows) - stubs} near-misses are not exact yet.\n")
    print(f"{'route':8}  {'functions':>9}  {'bytes':>7}")
    for route in ROUTES:
        chosen = [r for r in rows if r["route"] == route]
        print(f"{route:8}  {len(chosen):>9}  {sum(r['size'] for r in chosen):>7}")
    opus = [r for r in rows if r["route"] == "opus"]
    kinds = Counter("near-miss" if r["status"] == "near-miss" else r["unit"].split("/")[0] + " stub"
                    for r in opus)
    tried = sum(bool(r["last_attempt"]) or "attempted before" in r["reason"] for r in opus)
    print(f"\nopus: {', '.join(f'{n} {kind}' for kind, n in kinds.most_common())}, "
          f"in {len({r['unit'] for r in opus})} source files; {tried} attempted before.")
    reasons = Counter(r["reason"] for r in rows if r["route"] == "blocked")
    print("blocked by: " + ", ".join(f"{reason} {n}" for reason, n in reasons.most_common()))
    if not SDK_MAP.exists():
        print(f"\nNo {SDK_MAP.relative_to(ROOT)}: library symbols come from stub comments only.")
    if not REFERENCES.is_dir():
        print(f"No {REFERENCES.relative_to(ROOT)}: no reference sources, so nothing routes to sonnet.")


def show_route(rows: list[dict], route: str) -> None:
    chosen = [r for r in rows if r["route"] == route]
    if route == "opus":
        by_unit = defaultdict(list)
        for r in chosen:
            by_unit[r["unit"]].append(r)
        for unit, group in sorted(by_unit.items(), key=lambda kv: -len(kv[1])):
            print(f"src/{unit}.c: {len(group)} functions, {sum(r['size'] for r in group)} bytes")
            for r in sorted(group, key=lambda r: r["size"]):
                tried = f"  [last: {r['last_attempt']}]" if r["last_attempt"] else ""
                print(f"    {r['name']}  {r['size']:>6}  {r['reason']}{tried}")
        return
    for r in sorted(chosen, key=lambda r: (r["reason"], r["size"]) if route == "blocked" else r["size"]):
        extra = (f"{r['symbol']} ({r['library']}) from {r['reference']}" if route == "sonnet"
                 else r["reason"])
        tried = f"  [last: {r['last_attempt']}]" if r["last_attempt"] else ""
        print(f"{r['name']}  {r['size']:>6}  {r['unit']:<18}  {extra}{tried}")


def main() -> None:
    args = sys.argv[1:]
    rows = triage()
    if "--json" in args:
        json.dump(rows, sys.stdout, indent=1)
        print()
    elif args and args[0] in ROUTES:
        show_route(rows, args[0])
    elif args:
        sys.exit(__doc__)
    else:
        show_summary(rows)


if __name__ == "__main__":
    main()

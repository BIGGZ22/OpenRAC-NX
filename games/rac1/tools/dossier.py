#!/usr/bin/env python3
"""
Collects what a worker needs to match a function into
build-sn/try/<func>/CONTEXT.md, so it doesn't have to search for it:

  - where the function lives, its size, compiler and triage notes;
  - how every function it calls, and every global it touches, is already
    declared (prefer the declaration in its own file: a second declaration
    with another type does not compile);
  - who calls it, with any prototype their callers already use;
  - matched functions in the same file that share the most calls and
    globals, to copy the style of;
  - earlier attempts: the last verdict, notes and best candidate.

  python3 tools/dossier.py func_X [func_Y ...]          # CONTEXT.md only
  python3 tools/dossier.py --m2c func_X [func_Y ...]    # + m2c.c sketch (Docker)

func_LNN_XXXXXXXX names (docs/OVERLAYS.md) are overlay functions: their
CONTEXT.md is written by overlay_write() instead, since they have no
progress-report row yet, and adds what an overlay worker needs instead of
a triage reason: which levels it's in, the src/overlays file it lands in
and its neighbours there, and its relative from config/overlays/families.tsv.
--m2c is not offered for them: tools/m2c.py only reads asm/nonmatchings/.

Scripts do this for free; every line here is a search a worker doesn't pay for.
"""
from __future__ import annotations
import json
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

import triage  # noqa: E402

GP_BASE = 0x166D00
# The optional (?:L\d{2}_)? makes every pattern below match both an
# executable name (func_XXXXXXXX / D_XXXXXXXX) and an overlay one
# (func_LNN_XXXXXXXX / D_LNN_XXXXXXXX), without changing what it matches
# for plain executable assembly.
SYMBOL = re.compile(r"\b((?:func|D)_(?:L\d{2}_)?[0-9A-Fa-f]{8})\b")
CALL = re.compile(r"\bj(?:al)?\s+(func_(?:L\d{2}_)?[0-9A-Fa-f]{8})\b")
DATA = re.compile(r"%(?:hi|lo)\((D_(?:L\d{2}_)?[0-9A-Fa-f]{8})\)")
GP = re.compile(r"(-?0x[0-9A-Fa-f]+)\(\$28\)")
ALIAS = re.compile(r'__asm__\("((?:func|D)_(?:L\d{2}_)?[0-9A-Fa-f]{8})"\)')

OVERLAY_NAME = re.compile(r"^func_L\d{2}_[0-9A-Fa-f]{8}$")
OVERLAY_DATA_L = re.compile(r"^D_L\d{2}_[0-9A-Fa-f]{8}$")
OVERLAY_STUB = re.compile(r"INCLUDE_ASM\([^)]*\b(func_L\d{2}_[0-9A-Fa-f]{8})\)")
OVERLAY_DEF = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b(func_L\d{2}_[0-9A-Fa-f]{8})(?:_r)?\s*\(")
ALL_LEVELS = 19  # docs/OVERLAYS.md: every level; a "shared" function may be in fewer.


def asm_path(name: str) -> Path | None:
    if OVERLAY_NAME.match(name):
        path = ROOT / "asm/overlays" / f"{name}.s"
        return path if path.exists() else None
    for seg in ("text", "core_text"):
        path = ROOT / "asm/nonmatchings" / seg / f"{name}.s"
        if path.exists():
            return path
    return None


def references(name: str) -> tuple[list[str], list[str]]:
    """Functions called and globals used, in order of first appearance."""
    path = asm_path(name)
    if path is None:
        return [], []
    text = re.sub(r"/\*.*?\*/", "", path.read_text(errors="replace"))
    calls = list(dict.fromkeys(c for c in CALL.findall(text) if c != name))
    data = list(DATA.findall(text))
    level = re.match(r"func_L(\d\d)_", name)
    for off in GP.findall(text):
        addr = (GP_BASE + int(off, 16)) & 0xFFFFFFFF
        # In level code, 0x15F000 and up is the level's own data (docs/OVERLAYS.md).
        data.append(f"D_L{level.group(1)}_{addr:08X}" if level and addr >= 0x15F000 else f"D_{addr:08X}")
    return calls, list(dict.fromkeys(data))


def declaration_index() -> dict[str, list[tuple[str, int, str, str]]]:
    """symbol -> [(file, line, text, kind)] for lines that declare or define it.

    A declaration starts in column 0; indented continuation lines are joined
    to it, so a prototype whose __asm__ alias sits on the next line counts.
    """
    index = defaultdict(list)
    files = sorted((ROOT / "src").rglob("*.c")) + sorted((ROOT / "include").rglob("*.h"))
    for path in files:
        rel = str(path.relative_to(ROOT))
        lines = path.read_text(errors="replace").splitlines()
        for i, line in enumerate(lines):
            if not line or line[0].isspace() or line.startswith(("/", "*", "#")) or "INCLUDE_ASM" in line:
                continue
            text, j = line.rstrip(), i + 1
            while (not text.endswith(";") and "{" not in text and j < min(i + 5, len(lines))
                   and lines[j][:1].isspace() and not lines[j].strip().startswith("{")):
                text += " " + lines[j].strip()
                j += 1
            aliased = ALIAS.findall(text)
            for symbol in set(SYMBOL.findall(text)) | set(aliased):
                declared = text.startswith("extern") or text.endswith(";") or symbol in aliased
                kind = "declared" if declared else ("defined" if "(" in text else "declared")
                index[symbol].append((rel, i + 1, " ".join(text.split())[:160], kind))
    return index


def callers(name: str) -> list[str]:
    pattern = re.compile(rf"\bjal\s+{name}\b")
    dirs = [ROOT / "asm/nonmatchings"]
    if (ROOT / "asm/overlays").is_dir():  # overlay code can call back into it too
        dirs.append(ROOT / "asm/overlays")
    return sorted(p.stem for d in dirs for p in d.rglob("*.s")
                  if p.stem != name and pattern.search(p.read_text(errors="replace")))


def alias_name(text: str, symbol: str) -> str | None:
    """The C name a declaration gives SYMBOL through __asm__("SYMBOL"), if any."""
    head = text.split(f'__asm__("{symbol}")')[0].rstrip() if f'__asm__("{symbol}")' in text else None
    if head is None:
        return None
    if head.endswith(")"):  # A function: skip back over its parameter list.
        depth, i = 0, len(head)
        for i in range(len(head) - 1, -1, -1):
            depth += {")": 1, "(": -1}.get(head[i], 0)
            if depth == 0:
                break
        head = head[:i].rstrip()
    head = re.sub(r"(\[[^\]]*\])+$", "", head).rstrip()
    m = re.search(r"(\w+)$", head)
    return m.group(1) if m and m.group(1) != symbol else None


def describe(symbol: str, source: str, index) -> str:
    entries = index.get(symbol, [])
    own = [e for e in entries if e[0] == source]
    chosen = (own or entries)[:3]
    if not chosen:
        if OVERLAY_DATA_L.match(symbol):
            return (f"- `{symbol}`: level data, no declaration anywhere yet -- declare it "
                    f"yourself (a plain `extern <type> {symbol};` in the candidate, sized "
                    "and typed from how it's used; nothing in include/ names it)")
        return f"- `{symbol}`: not declared anywhere yet"
    status = " (matched C)" if any(e[3] == "defined" for e in entries) else ""
    lines = [f"- `{symbol}`{status}:"]
    for path, line, text, _ in chosen:
        name = alias_name(text, symbol)
        lines.append(f"  `{text}` ({path}:{line})" + (f" -> write `{name}` in C" if name else ""))
    if len({alias_name(e[2], symbol) for e in own}) > 1:
        lines.append("  (this file names it more than once: use the name whose type fits the access)")
    if not own and entries:
        lines.append("  (declared in another file only: add an extern of the same type)")
    return "\n".join(lines)


def neighbours(name: str, unit: str, calls, data, exact_by_unit) -> list[str]:
    mine = set(calls) | set(data)
    scored = []
    for other, size in exact_by_unit.get(unit, []):
        c, d = references(other)
        shared = mine & (set(c) | set(d))
        if shared:
            scored.append((len(shared), other, size, sorted(shared)))
    scored.sort(key=lambda s: -s[0])
    return [f"- `{o}` ({size} bytes) shares {n}: {', '.join(shared[:6])}{' ...' if n > 6 else ''}"
            for n, o, size, shared in scored[:3]]


def attempts(name: str) -> list[str]:
    work = ROOT / "build-sn/try" / name
    lines = []
    result = work / "RESULT.md"
    if result.exists():
        head = result.read_text(errors="replace").strip().splitlines()[:2]
        lines.append(f"- Last verdict: {' | '.join(h.strip() for h in head)}")
    for extra in ("NOTES.md", "RESULT.prev.md"):
        if (work / extra).exists():
            lines.append(f"- Read `build-sn/try/{name}/{extra}` for what was tried.")
    candidates = sorted(p.name for p in work.glob("*.c") if p.name not in ("src.c", "m2c.c"))
    if candidates:
        lines.append(f"- {len(candidates)} earlier candidates in build-sn/try/{name}/.")
    return lines or ["- None."]


def write(names: list[str]) -> None:
    report = json.loads((ROOT / "progress/report.json").read_text())
    exact_by_unit = {u["name"]: [(f["name"], int(f["size"])) for f in u.get("functions", [])
                                 if (f.get("fuzzy_match_percent") or 0) == 100] for u in report["units"]}
    rows = {r["name"]: r for r in triage.triage()}
    roles = load_overlay_roles()
    classes = load_moby_classes()
    rerac = load_rerac_notes()
    known = load_names()
    strings_by_addr, strings_by_func = load_strings()
    index = declaration_index()
    for name in names:
        row = rows.get(name)
        if row is None:
            print(f"{name}: already exact or unknown; skipped")
            continue
        source = f"src/{row['unit']}.c"
        seg = "text" if row["unit"].startswith("game/") else "core_text"
        compiler = "Sony gcc 2.9-ee (ee29)" if "2.9-ee" in row["reason"] else (
            "SN gcc 2.95.3" if seg == "text" else "see config/core_text.objects")
        calls, data = references(name)
        status = (f"near-miss, {row['match_percent']}% (its C is already in {source})"
                  if row["status"] == "near-miss" else f"stub (INCLUDE_ASM in {source})")
        out = [f"# {name}", "",
               f"- Source: {source}, {status}",
               f"- Size: {row['size']} bytes; compiler: {compiler}",
               f"- Retail assembly: asm/nonmatchings/{seg}/{name}.s",
               f"- Triage: {row['reason'] or 'no known blocker'}"]
        if row["symbol"]:
            out.append(f"- Real name: {row['symbol']}" + (f" ({row['library']})" if row["library"] else ""))
        out += name_lines(known.get(name))
        out += role_lines(roles.get(name, []), classes)
        out += rerac_lines(rerac.get(name, []))
        if row["reference"]:
            out.append(f"- Original source: {row['reference']} (start from it, not from m2c)")
        sketch = ROOT / "build-sn/try" / name / "m2c.c"
        if sketch.exists() and "{" in sketch.read_text(errors="replace"):
            out.append(f"- m2c sketch: build-sn/try/{name}/m2c.c (a starting point, never matching as is)")
        elif sketch.exists():
            out.append(f"- m2c failed on this function (its message is in build-sn/try/{name}/m2c.c)")
        out += ["", "## Earlier attempts", *attempts(name)]
        out += ["", "## Calls", *(describe(c, source, index) for c in calls)] if calls else ["", "## Calls", "- None."]
        out += ["", "## Globals", *(describe(d, source, index) for d in data)] if data else ["", "## Globals", "- None."]
        out += string_lines(name, data, strings_by_addr, strings_by_func)
        found = callers(name)
        out += ["", "## Called from", f"- {', '.join(found) if found else 'no direct calls in asm'}"]
        prototypes = [e for e in index.get(name, []) if e[3] == "declared"]
        out += [f"  `{text}` ({path}:{line})" for path, line, text, _ in prototypes[:3]]
        similar = neighbours(name, row["unit"], calls, data, exact_by_unit)
        out += ["", "## Matched neighbours in the same file", *(similar or ["- None share calls or globals."])]
        work = ROOT / "build-sn/try" / name
        work.mkdir(parents=True, exist_ok=True)
        (work / "CONTEXT.md").write_text("\n".join(out) + "\n")
        print(f"{name}: build-sn/try/{name}/CONTEXT.md ({len(calls)} calls, {len(data)} globals)")


def load_overlay_catalogue() -> dict[str, tuple[str, int, int, list[tuple[int, int]]]]:
    """name -> (kind, size, levels count, [(level, address), ...]),
    from config/overlays/functions.tsv (docs/OVERLAYS.md, "Names")."""
    rows = {}
    path = ROOT / "config/overlays/functions.tsv"
    for line in path.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        name, kind, size, _fp, levels, places = line.split("\t")
        place_list = [(int(p[:2]), int(p[3:], 16)) for p in places.split(",")]
        rows[name] = (kind, int(size), int(levels), place_list)
    return rows


def load_overlay_families() -> dict[str, tuple[str, str, int, float]]:
    """name -> (relative, relative kind, relative size, similarity),
    from config/overlays/families.tsv (docs/OVERLAYS.md, "Relatives")."""
    rows = {}
    path = ROOT / "config/overlays/families.tsv"
    if not path.exists():
        return rows
    for line in path.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        name, _kind, _size, relative, rel_kind, rel_size, similarity = line.split("\t")
        rows[name] = (relative, rel_kind, int(rel_size), float(similarity))
    return rows


def load_overlay_roles() -> dict[str, list[str]]:
    """name -> ["UpdateMoby_809:00/01/14", ...], from config/overlays/names.tsv
    (docs/OVERLAYS.md, "Roles"): what the levels' dispatch records use it for."""
    path = ROOT / "config/overlays/names.tsv"
    if not path.exists():
        return {}
    rows = {}
    for line in path.read_text().splitlines():
        if line and not line.startswith("#"):
            name, roles = line.split("\t")
            rows[name] = roles.split(",")
    return rows


def load_names() -> dict[str, dict]:
    """symbol -> its row in config/names.tsv (docs/NAMES.md)."""
    path = ROOT / "config/names.tsv"
    if not path.exists():
        return {}
    rows = {}
    for line in path.read_text().splitlines():
        if line and not line.startswith("#"):
            sym, name, tier, source, evidence, alts = (line.split("\t") + [""] * 6)[:6]
            rows[sym] = {"name": name, "tier": tier, "source": source, "alts": alts}
    return rows


def name_lines(row: dict | None) -> list[str]:
    if not row:
        return []
    use = ("" if row["tier"] == "candidate"
           else f"; C can call it `{row['name']}` (include/names.h), but define it as the address name")
    alts = f"; also called {row['alts'].replace(',', ', ')}" if row["alts"] else ""
    return [f"- Name: {row['name']} ({row['tier']}, from {row['source']}{alts}){use}"]


def load_moby_classes() -> dict[int, str]:
    """oClass -> moby class name, from the OpenRAC editor's moby_classes.tsv."""
    path = ROOT.parents[2] / "editor/moby_classes.tsv"
    if not path.exists():
        return {}
    rows = {}
    for line in path.read_text().splitlines():
        if line and not line.startswith("#"):
            number, name = line.split("\t")[:2]
            rows[int(number)] = name
    return rows


CAMERA_ROLES = {"InitCamera": "init", "ActivateCamera": "activate", "UpdateCamera": "update", "ExitCamera": "exit"}


def describe_role(role: str, classes: dict[int, str]) -> str:
    """"UpdateMoby_726:01" -> "update function of moby class 726 (novalis_elevator) on level 01"."""
    what, _, levels = role.partition(":")
    kind, _, number = what.rpartition("_")
    lv = levels.split("/") if levels else []
    where = ("on every level" if len(lv) == ALL_LEVELS else f"on level {lv[0]}" if len(lv) == 1
             else f"on levels {', '.join(lv)}" if len(lv) <= 4 else f"on {len(lv)} levels")
    if kind == "UpdateMoby":
        name = classes.get(int(number)) if number.isdigit() else None
        return f"update function of moby class {number}" + (f" ({name})" if name else "") + f" {where}"
    if kind in CAMERA_ROLES:
        return f"{CAMERA_ROLES[kind]} function of camera {number} {where}"
    if kind == "SoundFunc":
        return f"sound function {number} {where}"
    return f"{what} {where}"


def role_lines(roles: list[str], classes: dict[int, str] | None = None) -> list[str]:
    if not roles:
        return []
    classes = load_moby_classes() if classes is None else classes
    shown = "; ".join(describe_role(r, classes) for r in roles[:3])
    more = f"; +{len(roles) - 3} more" if len(roles) > 3 else ""
    moby = (" An update function takes the moby in $a0 (a partial `Moby` struct is in src/game/mobyfunc.c)."
            if any(r.startswith("UpdateMoby_") for r in roles) else "")
    return [f"- Role, from the levels' dispatch records (config/overlays/names.tsv): {shown}{more}.{moby}"]


def load_rerac_notes() -> dict[str, list[tuple[str, str, str, str]]]:
    """name -> [(ReRAC's name, confidence, note, source doc)], from
    config/overlays/rerac_notes.tsv (docs/OVERLAYS.md, "ReRAC notes")."""
    path = ROOT / "config/overlays/rerac_notes.tsv"
    if not path.exists():
        return {}
    rows: dict[str, list] = {}
    for line in path.read_text().splitlines():
        if line and not line.startswith("#"):
            name, rerac, confidence, note, source = line.split("\t")[:5]
            rows.setdefault(name, []).append((rerac, confidence, note, source))
    return rows


def rerac_lines(notes: list[tuple[str, str, str, str]]) -> list[str]:
    """What ReRAC (ISC) says the function does: at most two of its entries."""
    out = []
    for rerac, confidence, note, source in notes[:2]:
        out.append(f"- ReRAC (ISC) calls it `{rerac}` ({confidence}, its {source})" + (f": {note}" if note else ""))
    if len(notes) > 2:
        out.append(f"  (+{len(notes) - 2} more ReRAC entries in config/overlays/rerac_notes.tsv)")
    if len({n[0] for n in notes}) > 1:
        out.append("  (ReRAC documents several copies of this code under different names: the one name here "
                   "covers every identical copy)")
    return out


def load_strings() -> tuple[dict, dict]:
    """Returns (strings_by_address, strings_by_function) from config/strings.json."""
    path = ROOT / "config/strings.json"
    if not path.exists():
        return {}, {}
    try:
        data = json.loads(path.read_text(errors="replace"))
        return data.get("strings_by_address", {}), data.get("strings_by_function", {})
    except Exception:
        return {}, {}


def string_lines(name: str, data: list[str], strings_by_addr: dict, strings_by_func: dict) -> list[str]:
    seen = set()
    items = []
    for item in strings_by_func.get(name, []):
        sym = item.get("symbol", "")
        addr = item.get("string_address", "")
        val = item.get("value", "")
        key = (sym, addr, val)
        if key not in seen:
            seen.add(key)
            items.append((sym, addr, val))

    for d in data:
        m = re.search(r"([0-9A-Fa-f]{8})$", d)
        if m:
            hex_addr = m.group(1).upper()
            if hex_addr in strings_by_addr:
                s = strings_by_addr[hex_addr]
                sym = s.get("symbol", d)
                addr = s.get("address", f"0x{hex_addr}")
                val = s.get("value", "")
                key = (sym, addr, val)
                if key not in seen:
                    seen.add(key)
                    items.append((sym, addr, val))

    if not items:
        return []
    return ["", "## Referenced Strings", *(f"- `{sym}` ({addr}): {repr(val)}" for sym, addr, val in items)]


def overlay_functions_in(path: Path) -> list[tuple[str, bool]]:
    """[(name, already matched C)] for every overlay function PATH defines
    or stubs, in file order -- "the functions next to it" for the worker."""
    out = []
    for line in path.read_text(errors="replace").splitlines():
        m = OVERLAY_STUB.search(line)
        if m:
            out.append((m.group(1), False))
            continue
        m = OVERLAY_DEF.match(line)
        if m and not line.rstrip().endswith(";"):
            out.append((m.group(1), True))
    return out


def overlay_file_index() -> dict[str, tuple[Path, list[tuple[str, bool]]]]:
    """name -> (its src/overlays file, that file's whole function list),
    over every src/overlays/**/*.c file. Built once per dossier run."""
    index: dict[str, tuple[Path, list[tuple[str, bool]]]] = {}
    for path in sorted((ROOT / "src/overlays").rglob("*.c")):
        fns = overlay_functions_in(path)
        for name, _is_c in fns:
            index[name] = (path, fns)
    return index


def overlay_write(names: list[str], out_dir: Path | None = None) -> None:
    """CONTEXT.md for overlay functions (docs/OVERLAYS.md): no progress-report
    row or triage reason exists for these yet, so this covers what an
    overlay worker needs instead -- which levels it's in, the file it lands
    in and its neighbours there, and its family relative. OUT_DIR replaces
    build-sn/try/ (to look at a dossier without touching a worker's folder)."""
    catalogue = load_overlay_catalogue()
    families = load_overlay_families()
    roles = load_overlay_roles()
    classes = load_moby_classes()
    rerac = load_rerac_notes()
    known = load_names()
    strings_by_addr, strings_by_func = load_strings()
    findex = overlay_file_index()
    index = declaration_index()
    for name in names:
        row = catalogue.get(name)
        if row is None:
            print(f"{name}: not in config/overlays/functions.tsv; skipped")
            continue
        kind, size, levels_count, places = row
        levels = sorted({lv for lv, _addr in places})
        asm = asm_path(name)
        out = [f"# {name}", "",
               f"- Kind: {kind}, {size} bytes, in {levels_count} level(s): "
               + ", ".join(f"{lv:02d}" for lv in levels),
               *name_lines(known.get(name)),
               *role_lines(roles.get(name, []), classes),
               *rerac_lines(rerac.get(name, [])),
               "- Retail assembly: asm/overlays/" + name + ".s"
               + ("" if asm else " (missing -- run tools/overlay_asm.py, or wait: it's being regenerated)"),
               "- No m2c sketch: tools/m2c.py only reads asm/nonmatchings/. Start from the "
               "relative below, a matched neighbour in this file, or the assembly directly.",
               "- The executable does not build src/overlays/: no full build to verify "
               "against here, only tools/try_func.py's stricter overlay check "
               "(docs/OVERLAYS.md's per-function EXACT, not the executable's masked one)."]
        entry = findex.get(name)
        source = str(entry[0].relative_to(ROOT)) if entry else ""
        if entry:
            path, fns = entry
            out.append(f"- Lands in: {source}")
            out.append("- Functions in this file, in order:")
            for fn, is_c in fns:
                mark = "matched C" if is_c else "INCLUDE_ASM stub"
                flag = "  <-- this one" if fn == name else ""
                out.append(f"  - `{fn}` ({mark}){flag}")
        else:
            out.append("- Not found under src/overlays/ yet (unexpected for a catalogued function)")

        calls, data = references(name)
        out += ["", "## Calls", *(describe(c, source, index) for c in calls)] if calls else ["", "## Calls", "- None."]
        out += ["", "## Globals", *(describe(d, source, index) for d in data)] if data else ["", "## Globals", "- None."]
        out += string_lines(name, data, strings_by_addr, strings_by_func)

        found = callers(name)
        out += ["", "## Called from", f"- {', '.join(found) if found else 'no direct calls found in asm'}"]

        fam = families.get(name)
        if fam:
            relative, rel_kind, rel_size, similarity = fam
            rel_entry = findex.get(relative)
            status = " (not under src/overlays/ yet)"
            if rel_entry:
                rel_path, rel_fns = rel_entry
                rel_is_c = next((c for n, c in rel_fns if n == relative), False)
                status = (f", already matched in {rel_path.relative_to(ROOT)}" if rel_is_c
                          else f", still a stub in {rel_path.relative_to(ROOT)}")
            out += ["", "## Relative (docs/OVERLAYS.md, \"Relatives\")",
                    f"- `{relative}` ({rel_kind}, {rel_size} bytes, {similarity:.0%} similar){status}",
                    "  Start from its C if matched C is available; either way it's a same-shape "
                    "sibling to check against, not a finished answer -- masking hides constants, "
                    "so two functions can share a fingerprint and still differ."]
        else:
            out += ["", "## Relative", "- None recorded in config/overlays/families.tsv."]

        out += ["", "## Earlier attempts", *attempts(name)]
        work = (out_dir or ROOT / "build-sn/try") / name
        work.mkdir(parents=True, exist_ok=True)
        (work / "CONTEXT.md").write_text("\n".join(out) + "\n")
        shown = work.relative_to(ROOT) if work.is_relative_to(ROOT) else work
        print(f"{name}: {shown}/CONTEXT.md ({len(calls)} calls, {len(data)} globals)")


def sketches(names: list[str]) -> None:
    """m2c sketches for all names in one container run."""
    loop = "; ".join(f"python tools/m2c.py {n} > build-sn/try/{n}/m2c.c 2>&1" for n in names)
    for n in names:
        (ROOT / "build-sn/try" / n).mkdir(parents=True, exist_ok=True)
    # m2c can exit non-zero and still leave a usable sketch, so check each file.
    subprocess.run(["bash", "tools/docker/run.sh", "sh", "-c", loop], cwd=ROOT,
                   stdout=subprocess.DEVNULL)
    for n in names:
        sketch = ROOT / "build-sn/try" / n / "m2c.c"
        if "{" not in sketch.read_text(errors="replace"):
            print(f"{n}: m2c produced no sketch; see build-sn/try/{n}/m2c.c")


def main() -> None:
    args = sys.argv[1:]
    names = [a for a in args if a.startswith("func_")]
    if not names:
        sys.exit(__doc__)
    overlay = [n for n in names if OVERLAY_NAME.match(n)]
    exe = [n for n in names if n not in overlay]
    if overlay and exe:
        sys.exit("dossier.py: mix of overlay and executable names; run them separately")
    if overlay:
        overlay_write(overlay)
        return
    if "--m2c" in args:
        sketches(names)
    write(names)


if __name__ == "__main__":
    main()

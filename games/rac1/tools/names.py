#!/usr/bin/env python3
"""
Readable names for this build's symbols, and the header that lets C use
them while the linker symbol stays func_<ADDR> / func_LNN_<ADDR> / D_<ADDR>
(docs/NAMES.md).

  python3 tools/names.py build     # sources -> config/names.tsv
  python3 tools/names.py header    # config/names.tsv -> include/names.h
  python3 tools/names.py apply     # use the header's names in src/ C bodies
  python3 tools/names.py check     # names.h is current and collision-free
  python3 tools/names.py ntsc      # the NTSC decomp's names (tools/ntsc_source.py) -> config/names_ntsc.tsv,
                                   #   config/modules.tsv, and merged into config/names.tsv
  python3 tools/names.py rename    # carry changed names into names.h and src/ (header, apply-defs, apply)
  python3 tools/names.py SYMBOL|NAME [...]   # look one up

config/names.tsv is the committed table; `build` regenerates it and needs
the other RaC1 projects checked out (paths from the environment, defaults
in parentheses):

  NTSC       the NTSC decompilation (~/Projects/NTSC)
  LOMBYTE   Lombyte, the US decompilation (games/rac1/ntsc inside OpenRAC, else ~/Projects/Lombyte)
  RERAC     ReRAC, the PC port, for its Ghidra name tables (~/Projects/rerac)

Missing sources are skipped with a warning; the rows they gave are then
dropped, so run `build` with all three present.

Every row is one symbol of ours with one chosen name, its tier, where the
name came from and the evidence that placed it on this PAL address:

  recovered    the original identifier: from config/symbol_names.txt,
               the NTSC decomp's symbols.txt as it stood before its 2026-09
               automated naming loop, or a symbol Lombyte recovered
  descriptive  a later, evidence-backed name: the NTSC decomp's 2026-09 names, a
               Lombyte proposal of high confidence, a ReRAC name it marks
               verified, the community PAL memory map (globals)
  candidate    everything weaker (Lombyte medium, ReRAC suggested or
               inferred, a name another symbol already took): listed for
               workers, never put into names.h

US addresses reach PAL two ways. The executable's are aligned by function
size against Lombyte's report (tools/lombyte.py's method, runs of four or
more equal sizes). Level code, and executable code the levels also hold,
goes through Lombyte's US overlay catalogue, which fingerprints functions
exactly as ours does: a fingerprint found once in each catalogue, with
the same size, is the same function at every place it lists. Where the
two methods cover the same executable function they must agree, or both
are dropped.
"""
from __future__ import annotations
import csv
import difflib
import json
import os
import re
import subprocess
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lombyte import LOMBYTE as LOMBYTE_TREE, their_report  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
TABLE = ROOT / "config/names.tsv"
LOCAL = ROOT / "config/names_local.tsv"
HEADER = ROOT / "include/names.h"
SYMBOL_NAMES = ROOT / "config/symbol_names.txt"
CATALOGUE = ROOT / "config/overlays/functions.tsv"
REPORT = ROOT / "progress/report.json"
HOME = Path.home() / "Projects"
NTSC = Path(os.environ.get("NTSC", HOME / "NTSC"))
LOMBYTE = LOMBYTE_TREE        # $LOMBYTE, games/rac1/ntsc inside OpenRAC, or ~/Projects/Lombyte
RERAC = Path(os.environ.get("RERAC", HOME / "rerac"))

TIERS = ("recovered", "descriptive", "candidate")
# the NTSC decomp's automated naming loop started in 2026-09; symbols.txt as of
# the last commit before it holds only the hand-made names.
NTSC_LOOP_START = "2026-09-01"
MINRUN = 4
SYMBOL = re.compile(r"^(func_(?:L\d{2}_)?[0-9A-F]{8}|D_(?:L\d{2}_)?[0-9A-F]{8})$")
PLACEHOLDER = re.compile(r"(?i)^(func|fun|sub|d|dat|lab)_|unk|_[0-9a-f]{6,8}$|^[0-9]")
IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

# Globals whose PAL address the community memory map ("Ratchet & Clank
# Series Addresses", RaC1 sheet, PS2 PAL column) gives, kept only where the
# address is the exact start of one of our D_ symbols and our code's
# accesses fit the described value. (symbol, name, the sheet's label)
MEMORY_MAP = [
    ("D_0013D490", "gSpecialItems", "Zoomerator (first of the special-item flags: Zoomerator, Raritanium, Codebot, Premium/Ultra Nanotech)"),
    ("D_0013D510", "gSkillPoints", "Take Aim (first of the 30 skill-point flags)"),
    ("D_0013D5CA", "gHaveHeliPack", "Heli-Pack owned"),
    ("D_0013D5DD", "gHaveMorphORay", "Morph-o-Ray owned"),
    ("D_0013D5E4", "gHaveMagneboots", "Magneboots owned"),
    ("D_0013D5E7", "gHaveHologuise", "Hologuise owned"),
    ("D_0013D5E9", "gHaveMapOMatic", "Map-o-matic owned"),
    ("D_0013D5EB", "gHavePersuader", "Persuader owned"),
    ("D_0013D618", "gGalaxyList", "Galaxy list: planets in the order they were discovered"),
    ("D_0013E629", "gGoldSuckCannon", "Suck Cannon gold"),
    ("D_0013F4D0", "gHeroPos", "X/Y/Z coordinate of Ratchet"),
    ("D_0015EE98", "gBolts", "Bolts"),
    ("D_0015EEA0", "gMaxNanotech", "Ratchet max nanotech"),
    ("D_0015EEB0", "gCheats", "Actors have oversized craniums (first of the 8 cheat flags)"),
    ("D_0015EEB4", "gCheatMirroredLevels", "Levels are mirrored (cheat flag 4)"),
    ("D_0015EEE8", "gStereo", "Stereo/Mono setting"),
    ("D_0015EF1C", "gHelpDeskVoice", "HelpDesk voice setting"),
    ("D_0015EF1D", "gHelpDeskText", "HelpDesk text setting"),
    ("D_0015EF40", "gSubtitles", "Subtitles setting"),
]


def warn(msg):
    print(f"names.py: {msg}", file=sys.stderr)


# ---------------------------------------------------------------- helpers

def c_name(mangled: str) -> str | None:
    """The C identifier for a (possibly GCC 2.x-mangled) name: the base
    name, or Class_method for a member function; None if there is none."""
    name = mangled.strip()
    m = re.match(r"^(.+?)__(\d+)([A-Za-z_]\w*)", name)
    if m and not name.startswith("__"):
        cls = m.group(3)[:int(m.group(2))]
        name = f"{cls}_{m.group(1)}"
    elif not name.startswith("__"):
        name = re.split(r"__(?=[FQ0-9])", name)[0]
    name = name.split("(")[0].replace("::", "_")
    if not IDENT.match(name) or PLACEHOLDER.search(name):
        return None
    return name


def exe_functions() -> list[tuple[int, int, str]]:
    """Our executable functions (address, size, func_X), from the report."""
    rows = []
    for unit in json.loads(REPORT.read_text())["units"]:
        if "level_code" in (unit.get("metadata") or {}).get("progress_categories", []):
            continue
        for f in unit.get("functions", []):
            if re.fullmatch(r"func_[0-9A-F]{8}", f["name"]):
                rows.append((int(f["name"][5:], 16), int(f["size"]), f["name"]))
    return sorted(rows)


def game_symbols() -> set[str]:
    """func_X in the report's game units (not core/SDK, not libgcc)."""
    out = set()
    for unit in json.loads(REPORT.read_text())["units"]:
        if unit["name"].startswith("game/"):
            out |= {f["name"] for f in unit.get("functions", [])}
    return out


def align(theirs, ours) -> dict[int, str]:
    """{their address: our name} for runs of MINRUN+ equal sizes.
    theirs: [(addr, size)], ours: [(addr, size, name)], both sorted."""
    sm = difflib.SequenceMatcher(None, [s for _, s in theirs], [s for _, s, _ in ours], autojunk=False)
    out = {}
    for a, b, n in sm.get_matching_blocks():
        if n >= MINRUN:
            for k in range(n):
                out[theirs[a + k][0]] = ours[b + k][2]
    return out


def ntsc_exe_map() -> dict[int, str]:
    """US executable address -> our func_X, through Lombyte's report."""
    report = their_report(LOMBYTE)
    # Its "virtual_address" is the ROM offset of splat's main segment;
    # the segment's start/vram in its splat config give the address.
    yaml = (LOMBYTE / "config/us/rnc1.us.yaml").read_text()
    start = int(re.search(r"(?m)^\s*start:\s*(0x[0-9A-Fa-f]+)", yaml).group(1), 16)
    vram = int(re.search(r"(?m)^\s*vram:\s*(0x[0-9A-Fa-f]+)", yaml).group(1), 16)
    theirs = []
    for unit in report["units"]:
        if unit["name"].split("/")[0] == "shared" or unit["name"].startswith("level_"):
            continue
        for f in unit.get("functions", []):
            va = int(f.get("metadata", {}).get("virtual_address") or 0)
            if va:
                theirs.append((va - start + vram, int(f["size"])))
    return align(sorted(set(theirs)), exe_functions())


def catalogue(path: Path) -> list[list[str]]:
    return [line.split("\t") for line in path.read_text().splitlines() if not line.startswith("#")]


def fingerprint_maps() -> tuple[dict[tuple[int, int], str], dict[int, str]]:
    """Lombyte's US overlay catalogue has the same masked fingerprints as
    ours (config/overlays/functions.tsv, tools/overlays.py's mask), so a
    function whose fingerprint occurs once in each, with the same size,
    is the same function. Returns ({(level, US address): our name} for
    every place, {US executable address: our func_X} for exe kinds)."""
    ours, theirs = defaultdict(list), defaultdict(list)
    for f in catalogue(CATALOGUE):
        ours[f[3]].append(f)
    for f in catalogue(LOMBYTE / "config/overlays/us/functions.tsv"):
        theirs[f[3]].append(f)
    places, exe = {}, {}
    for fp in set(ours) & set(theirs):
        if len(ours[fp]) != 1 or len(theirs[fp]) != 1:
            continue
        o, t = ours[fp][0], theirs[fp][0]
        if o[2] != t[2]:
            continue
        for pl in t[5].split(","):
            lv, a = pl.split(":")
            places[(int(lv), int(a, 16))] = o[0]
        if o[1] == "exe" and t[0].startswith("FUN_") and not t[0].startswith("FUN_L"):
            exe[int(t[0][4:], 16)] = o[0]
    return places, exe


def ntsc_symbols() -> tuple[dict[int, str], dict[int, str]]:
    """({NTSC address: name} as of before the naming loop, {... now})."""
    path = NTSC / "config/symbols.txt"
    parse = lambda text: {int(m.group(2), 16): m.group(1) for m in
                          re.finditer(r"(?m)^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;", text)}
    now = parse(path.read_text())
    rev = subprocess.run(["git", "-C", str(NTSC), "rev-list", "-1", f"--before={NTSC_LOOP_START}", "HEAD"],
                         capture_output=True, text=True).stdout.strip()
    old = {}
    if rev:
        text = subprocess.run(["git", "-C", str(NTSC), "show", f"{rev}:config/symbols.txt"],
                              capture_output=True, text=True).stdout
        old = parse(text)
    else:
        warn("NTSC history not available (shallow clone?): every NTSC name counts as descriptive")
    return old, now


# ---------------------------------------------------------------- build

def build() -> None:
    cands = defaultdict(list)   # symbol -> [(tier, priority, name, source, evidence)]

    def add(sym, name, tier, prio, source, evidence):
        cands[sym].append((TIERS.index(tier), prio, name, source, evidence))

    # 1. config/symbol_names.txt (the NTSC decomp's names already placed on PAL).
    for line in SYMBOL_NAMES.read_text().splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        head, _, evidence = line.partition("|")
        sym, mangled = head.split()[:2]
        name = c_name(mangled)
        if name:
            add(sym, name, "recovered", 0, "config/symbol_names.txt", evidence.strip())

    have = {"NTSC": (NTSC / "config/symbols.txt").exists(),
            "Lombyte": their_report(LOMBYTE) is not None,
            "ReRAC": (RERAC / "tools/ghidra/names/doc_names.csv").exists()}
    for k, ok in have.items():
        if not ok:
            warn(f"{k} not found; its names are left out")

    exe_map, ov_map = {}, {}
    if have["Lombyte"]:
        exe_map = ntsc_exe_map()
        ov_map, fp_exe = fingerprint_maps()
        clash = {a: (exe_map[a], n) for a, n in fp_exe.items() if a in exe_map and exe_map[a] != n}
        if clash:
            warn(f"size alignment and fingerprints disagree on {len(clash)} executable functions; "
                 "dropping both: " + ", ".join(f"US 0x{a:06X}" for a in sorted(clash)))
            for a in clash:
                del exe_map[a]
                del fp_exe[a]
        exe_map.update(fp_exe)

    # 2. NTSC symbols.txt, US executable addresses.
    if have["NTSC"] and exe_map:
        old, now = ntsc_symbols()
        for addr, mangled in now.items():
            sym, name = exe_map.get(addr), c_name(mangled)
            if sym and name:
                tier = "recovered" if old.get(addr) == mangled else "descriptive"
                era = "hand-named" if tier == "recovered" else "2026-09 naming loop"
                add(sym, name, tier, 1, "NTSC config/symbols.txt",
                    f"{mangled} at US 0x{addr:06X} ({era}); US/PAL size alignment")

    # 3. Lombyte: recovered symbols, then its semantic proposals.
    if have["Lombyte"]:
        rec = json.loads((LOMBYTE / "config/us/recovered_names.json").read_text())
        for s in rec["symbols"]:
            addr = int(s["address"], 16)
            sym, name = exe_map.get(addr), c_name(s["name"])
            if sym and name and s.get("match") in ("full", "fragment"):
                add(sym, name, "recovered", 2, "Lombyte recovered_names.json",
                    f"recovered symbol {s['name']} at US 0x{addr:06X} ({s['match']} match); US/PAL size alignment")
        for e in rec["rename_proposals"]["entries"]:
            if not e.get("proposed_name"):
                continue
            addr = int(e["address"], 16)
            sym, name = exe_map.get(addr), c_name(e["proposed_name"])
            if sym and name:
                tier = "descriptive" if e["confidence"] == "high" else "candidate"
                add(sym, name, tier, 2, "Lombyte recovered_names.json",
                    f"proposal ({e['confidence']}, {e['logical_group']}) at US 0x{addr:06X}; US/PAL size alignment")

    # 4. ReRAC: names its docs give (boot, and level programs).
    if have["ReRAC"]:
        with open(RERAC / "tools/ghidra/names/doc_names.csv", newline="") as f:
            for r in csv.DictReader(f):
                if r["kind"] != "fn":
                    continue
                addr = int(r["address"], 16)
                if r["program"] == "boot":
                    sym = exe_map.get(addr)
                    where = f"US boot 0x{addr:06X}"
                elif r["program"].startswith("level"):
                    lv = int(r["program"][5:])
                    sym = ov_map.get((lv, addr))
                    where = f"US level {lv:02d} 0x{addr:06X}"
                else:
                    continue
                name = c_name(r["name"])
                if sym and name:
                    tier = "descriptive" if r["confidence"] == "verified" else "candidate"
                    note = r["note"].split("|")[0].strip()
                    how = "fingerprint" if r["program"].startswith("level") else "US/PAL size alignment"
                    add(sym, name, tier, 3, "ReRAC doc_names.csv",
                        f"{r['confidence']} at {where} ({r['source_doc']}): {note}; {how}")

    # 5. Globals from the PAL memory map.
    for sym, name, label in MEMORY_MAP:
        add(sym, name, "descriptive", 0, "RaC1 PAL memory map", f"sheet row \"{label}\"")

    # Choose: best tier, then source priority; a name goes to one symbol.
    rows, taken = [], {}
    order = sorted(cands, key=lambda s: min(c[:2] for c in cands[s]))
    for sym in order:
        best = sorted(cands[sym])
        chosen = None
        for c in best:
            if c[2] not in taken:
                chosen = c
                break
        alts = sorted({c[2] for c in best} - {chosen[2] if chosen else None})
        if chosen is None:
            c = best[0]
            chosen = (TIERS.index("candidate"), c[1], c[2], c[3], c[4] + f"; name already taken by {taken[c[2]]}")
        elif chosen is not best[0]:
            chosen = (max(chosen[0], TIERS.index("candidate")),) + chosen[1:4] + \
                (chosen[4] + f"; preferred {best[0][2]} is taken by {taken[best[0][2]]}",)
        if chosen[0] < TIERS.index("candidate"):
            taken[chosen[2]] = sym
        rows.append((sym, chosen[2], TIERS[chosen[0]], chosen[3], chosen[4], ",".join(alts)))

    rows = [tuple(r.values()) for r in merge_ntsc(merge_local([dict(zip(COLUMNS, r)) for r in rows]))]
    rows.sort(key=lambda r: (r[0].startswith("D_"), r[0]))
    with open(TABLE, "w", newline="") as f:
        f.write("# Generated by tools/names.py build; see docs/NAMES.md. Columns:\n")
        f.write("# symbol\tname\ttier\tsource\tevidence\talternatives\n")
        for r in rows:
            f.write("\t".join(x.replace("\t", " ") for x in r) + "\n")
    c = Counter(r[2] for r in rows)
    print(f"wrote {TABLE.relative_to(ROOT)}: {len(rows)} symbols "
          f"({', '.join(f'{c[t]} {t}' for t in TIERS)})")


# ---------------------------------------------------------------- header

COLUMNS = ("symbol", "name", "tier", "source", "evidence", "alts")
LOCAL_SOURCE = "config/names_local.tsv"


def read_local() -> list[tuple[str, str, str, bool]]:
    """(symbol, name, evidence, override) rows of config/names_local.tsv:
    names this project gives from reading the code."""
    out = []
    if not LOCAL.exists():
        return out
    for line in LOCAL.read_text().splitlines():
        if line.startswith("#") or not line.strip():
            continue
        sym, name, evidence, flag = (line.split("\t") + ["", "", "", ""])[:4]
        if not SYMBOL.match(sym) or not IDENT.match(name):
            sys.exit(f"{LOCAL.name}: bad row: {line!r}")
        out.append((sym, name, evidence, flag.strip() == "override"))
    return out


def merge_local(rows: list[dict]) -> list[dict]:
    """ROWS with config/names_local.tsv applied: a local name fills a symbol
    that has none or only a candidate (tier descriptive); `override` also
    replaces a recovered/descriptive name that is wrong. The old name goes
    to the alternatives. A name another symbol holds is refused."""
    by_sym = {r["symbol"]: r for r in rows}
    holder = {r["name"]: r["symbol"] for r in rows if r["tier"] != "candidate"}
    for sym, name, evidence, override in read_local():
        old = by_sym.get(sym)
        if old and old["tier"] != "candidate" and not override:
            continue
        if holder.get(name, sym) != sym:
            warn(f"{LOCAL.name}: {name} for {sym} is taken by {holder[name]}")
            continue
        new = {"symbol": sym, "name": name, "tier": "descriptive", "source": LOCAL_SOURCE,
               "evidence": evidence, "alts": ""}
        if old:
            alts = [a for a in old["alts"].split(",") if a] + [old["name"]]
            new["alts"] = ",".join(dict.fromkeys(a for a in alts if a != name))
            if old["name"] in holder and holder[old["name"]] == sym:
                del holder[old["name"]]
            old.update(new)
        else:
            rows.append(new)
            by_sym[sym] = new
        holder[name] = sym
    return rows


def local() -> None:
    """Apply config/names_local.tsv to config/names.tsv without the
    external sources `build` needs. The NTSC decomp's adopted names stay on top."""
    rows = merge_ntsc(merge_local(unmerge_ntsc(read_table())))   # existing rows keep their order; new ones follow
    head = [l for l in TABLE.read_text().splitlines() if l.startswith("#")]
    with open(TABLE, "w", newline="") as f:
        f.write("\n".join(head) + "\n")
        for r in rows:
            f.write("\t".join(r[c].replace("\t", " ") for c in COLUMNS) + "\n")
    c = Counter(r["tier"] for r in rows)
    print(f"wrote {TABLE.relative_to(ROOT)}: {len(rows)} symbols "
          f"({', '.join(f'{c[t]} {t}' for t in TIERS)})")


def read_table() -> list[dict]:
    out = []
    for line in TABLE.read_text().splitlines():
        if line.startswith("#") or not line.strip():
            continue
        sym, name, tier, source, evidence, alts = (line.split("\t") + [""] * 6)[:6]
        out.append({"symbol": sym, "name": name, "tier": tier, "source": source,
                    "evidence": evidence, "alts": alts})
    return out


def strip_c(text: str) -> str:
    """TEXT without comments, string and character literals."""
    return re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'',
                  " ", text, flags=re.S)


def source_files():
    for d in ("src", "include"):
        for p in sorted((ROOT / d).rglob("*")):
            if p.suffix in (".c", ".h") and p != HEADER:
                yield p


def identifiers_in_use() -> set[str]:
    words = set()
    for p in source_files():
        words |= set(re.findall(r"\b[A-Za-z_]\w*\b", strip_c(p.read_text(errors="replace"))))
    return words


def header_rows(rows=None):
    """Rows names.h defines, and rows left out because the name is already
    an identifier in src/ or include/ (it would be silently renamed)."""
    rows = rows if rows is not None else read_table()
    game = game_symbols()
    # A name names.h already maps to the same symbol is in src/ because
    # `apply` put it there; only new names can clash with an identifier.
    current = dict(re.findall(r"(?m)^#define (\w+)\s+(\w+)$",
                              HEADER.read_text() if HEADER.exists() else ""))
    in_use = identifiers_in_use() - {n for n, s in current.items()
                                     if any(r["name"] == n and r["symbol"] == s for r in rows)}
    keep, clash = [], []
    for r in rows:
        if r["tier"] == "candidate":
            continue
        sym = r["symbol"]
        if sym.startswith("func_") and not sym.startswith("func_L") and sym not in game:
            continue   # core/SDK/libc: its own well-known names stay unaliased
        (clash if r["name"] in in_use else keep).append(r)
    return keep, clash


def render_header(keep) -> str:
    lines = [
        "/*",
        " * Readable names for this build's symbols. Generated by",
        " * tools/names.py header from config/names.tsv; do not edit.",
        " *",
        " * Each name is a macro for the address-named symbol the build and",
        " * every tool use (func_<ADDR>, func_LNN_<ADDR>, D_<ADDR>): code may",
        " * write either spelling and compiles to the same object. Definitions",
        " * and bodies use the readable name; declarations, __asm__ labels and",
        " * _r definitions keep the address name (tools read definitions",
        " * through tools/srcnames.py).",
        " * Tiers and sources: docs/NAMES.md.",
        " */",
        "#ifndef NAMES_H",
        "#define NAMES_H",
        "",
    ]
    width = max((len(r["name"]) for r in keep), default=0)
    for tier in ("recovered", "descriptive"):
        group = [r for r in keep if r["tier"] == tier]
        if not group:
            continue
        lines.append(f"/* {tier} */")
        for r in group:
            lines.append(f"#define {r['name']:<{width}} {r['symbol']}")
        lines.append("")
    lines.append("#endif /* NAMES_H */")
    return "\n".join(lines) + "\n"


def header() -> None:
    keep, clash = header_rows()
    HEADER.write_text(render_header(keep))
    print(f"wrote {HEADER.relative_to(ROOT)}: {len(keep)} names"
          + (f"; {len(clash)} left out, already an identifier in src/: "
             + ", ".join(sorted(r['name'] for r in clash)) if clash else ""))


def check() -> None:
    """names.h matches the table, and no name it defines is used as a
    different identifier anywhere in src/ or include/."""
    keep, clash = header_rows()
    current = HEADER.read_text() if HEADER.exists() else ""
    defined = dict(re.findall(r"(?m)^#define (\w+)\s+(\w+)$", current))
    wanted = {r["name"]: r["symbol"] for r in keep}
    in_header = {n: s for n, s in defined.items() if n != "NAMES_H"}
    bad = False
    for n, s in in_header.items():
        if wanted.get(n) != s and not any(r["name"] == n for r in clash):
            print(f"names.h defines {n} as {s}, the table says {wanted.get(n)}")
            bad = True
    missing = sorted(set(wanted) - set(in_header))
    if missing:
        print(f"names.h lacks {len(missing)} names from the table (run: tools/names.py header)")
        bad = True
    if bad:
        sys.exit(1)
    print(f"include/names.h is current: {len(in_header)} names")


# ---------------------------------------------------------------- apply

DECL_LINE = re.compile(r"^\s*(extern\b|#|INCLUDE_ASM|ASM_FUNC|LINKER_REMNANT|typedef\b)")


def apply() -> None:
    """In every src/ C file, write a named symbol by its readable name
    inside function bodies. Declarations, definitions, preprocessor lines,
    comments, strings and __asm__ labels keep the address name, so every
    tool that reads `func_X(` definitions or INCLUDE_ASM stubs is
    unaffected."""
    keep, _ = header_rows()
    name_of = {r["symbol"]: r["name"] for r in keep}
    token = re.compile(r"\b(" + "|".join(re.escape(s) for s in sorted(name_of, key=len, reverse=True)) + r")\b")
    changed_files = changed = 0
    for p in sorted((ROOT / "src").rglob("*.c")):
        text = p.read_text()
        out, n = rewrite_bodies(text, token, name_of)
        if n:
            if '#include "names.h"' not in out and "names.h" not in open(ROOT / "include/common.h").read():
                sys.exit("common.h does not include names.h")
            p.write_text(out)
            changed_files += 1
            changed += n
    print(f"{changed} uses renamed in {changed_files} files")


def rewrite_bodies(text: str, token, name_of) -> tuple[str, int]:
    """Replace TOKEN matches only inside brace depth >= 1 of top-level
    function bodies, outside comments/strings, on lines that are not
    declarations or preprocessor lines."""
    out, n, i, depth = [], 0, 0, 0
    L = len(text)
    line_start = 0
    while i < L:
        c = text[i]
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = L if j < 0 else j + 2
            out.append(text[i:j]); i = j; continue
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = L if j < 0 else j
            out.append(text[i:j]); i = j; continue
        if c in "\"'":
            j = i + 1
            while j < L and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1]); i = j + 1; continue
        if c == "\n":
            line_start = i + 1
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        m = token.match(text, i) if (c == "f" or c == "D") else None
        if m and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_")):
            line_end = text.find("\n", i)
            line = text[line_start:line_end if line_end >= 0 else L]
            if depth >= 1 and not DECL_LINE.match(line) and "__asm__" not in line:
                out.append(name_of[m.group(1)]); n += 1
            else:
                out.append(m.group(1))
            i = m.end(); continue
        out.append(c); i += 1
    return "".join(out), n


DEF_LINE = re.compile(r"^(?!extern\b)([A-Za-z_][^;\n]*?\b)(func_(?:L\d\d_)?[0-9A-Fa-f]{8})(\s*\()")


def apply_defs() -> None:
    """In every src/ C file, write a named function's definition with its
    readable name: `void func_001F4E08(void) {` becomes `void FadeToBlack(void) {`.
    names.h makes it the same symbol. `_r` definitions, lines with an
    __asm__ label and prototypes keep the address name. Tools that find
    definitions read source through tools/srcnames.py."""
    keep, _ = header_rows()
    name_of = {r["symbol"]: r["name"] for r in keep if r["symbol"].startswith("func_")}
    changed_files = changed = 0
    for p in sorted((ROOT / "src").rglob("*.c")):
        lines = p.read_text().split("\n")
        n = 0
        for i, line in enumerate(lines):
            m = DEF_LINE.match(line)
            if not m or m.group(2) not in name_of or "__asm__" in line:
                continue
            rest = line[m.end():]
            if line.rstrip().endswith(";") or DECL_LINE.match(line):
                continue
            # a definition: its body opens on this line or the next non-blank one
            nxt = next((l for l in lines[i + 1:] if l.strip()), "")
            if "{" not in rest and not nxt.lstrip().startswith("{") and not rest.rstrip().endswith(","):
                if not re.search(r"\)\s*$", rest) or nxt.strip().endswith(";"):
                    continue
            lines[i] = m.group(1) + name_of[m.group(2)] + m.group(3) + rest
            n += 1
        if n:
            p.write_text("\n".join(lines))
            changed_files += 1
            changed += n
    print(f"{changed} definitions renamed in {changed_files} files")


# ---------------------------------------------------------------- the NTSC decomp

# The NTSC decomp (Lombyte's project) is pinned to a commit (config/versions/us/ntsc_decomp.txt)
# and read from a cache outside the repository (tools/ntsc_source.py; `follow` bumps the pin).
# `ntsc` reads its names from that checkout at each follow; nothing of its code is copied here. Its C binds each name to its US address with an asm label
# (`void draw_hud(void) __asm__("FUN_001f5f18")`, `extern s32 bolts __asm__("D_0015EE98")`).
NTSC_TABLE = ROOT / "config/names_ntsc.tsv"
MODULES = ROOT / "config/modules.tsv"
NTSC_SOURCE = "the NTSC decomp"
NTSC_LABEL = re.compile(
    r"([A-Za-z_]\w*)\s*(?:\((?:[^;{}()]|\([^()]*\))*\)|\[[^\]]*\])*\s*"
    r"__asm__\s*\(\s*\"((?:FUN|func|D|DAT)_(?:L(\d\d)_)?([0-9A-Fa-f]{6,8}))\"\s*\)")
NTSC_ASM_STUB = re.compile(r"INCLUDE_ASM\(\s*\"[^\"]*\"\s*,\s*(?:FUN|func)_(?:L(\d\d)_)?([0-9A-Fa-f]{6,8})\s*\)")
NTSC_ADDR_DEF = re.compile(r"(?m)^[A-Za-z_][^;{}()\n]*?\b(?:FUN|func)_(?:L(\d\d)_)?([0-9A-Fa-f]{8})\s*\([^;{}]*\)\s*\{")
# a name that is a C type or keyword, or still an address placeholder, is not a name
NOT_A_NAME = re.compile(r"[0-9a-fA-F]{6,}|^(void|int|char|short|long|float|double|const|volatile|struct|union|"
                        r"enum|unsigned|signed|static|extern|inline|register|u8|s8|u16|s16|u32|s32|f32|u64|"
                        r"s64|u128|s128)$")
# statuses in config/names_ntsc.tsv
ADOPT, SAME, KEPT = "adopted", "same", "kept-original"
HELD = ("held-alias", "held-several-symbols", "held-several-names", "held-clash", "held-unreferenced",
        "held-library")


def ntsc_labels(root: Path) -> dict:
    """{(kind, level, US address): {"names": Counter, "defs": Counter, "files": {name: set}}}
    from the NTSC decomp's C and headers, and {(level, US address): {their .c file}} for the
    functions their C files hold, named or not (a definition or an INCLUDE_ASM stub)."""
    labels = defaultdict(lambda: {"names": Counter(), "defs": Counter(), "files": defaultdict(set)})
    homes = defaultdict(set)
    paths = sorted(root.glob("src/**/*.c")) + sorted(root.glob("src/**/*.h")) + sorted(root.glob("include/**/*.h"))
    for p in paths:
        rel = str(p.relative_to(root))
        text = re.sub(r"/\*.*?\*/|//[^\n]*", " ", p.read_text(errors="replace"), flags=re.S)
        for m in NTSC_LABEL.finditer(text):
            ident, label, lv, addr = m.groups()
            if NOT_A_NAME.search(ident) or ident in ("__asm__", "asm"):
                continue
            key = ("f" if label.startswith(("FUN", "func")) else "d", int(lv) if lv else None, int(addr, 16))
            labels[key]["names"][ident] += 1
            labels[key]["files"][ident].add(rel)
            if key[0] == "f" and p.suffix == ".c" and re.search(
                    r"\b" + re.escape(ident) + r"\s*\((?:[^;{}()]|\([^()]*\))*\)\s*\{", text):
                labels[key]["defs"][ident] += 1
                homes[key[1:]].add(rel)
        if p.suffix == ".c":
            for m in list(NTSC_ASM_STUB.finditer(text)) + list(NTSC_ADDR_DEF.finditer(text)):
                lv, addr = m.groups()
                homes[(int(lv) if lv else None, int(addr, 16))].add(rel)
    # a C file the splat config places at a ROM offset of the executable, named after its function
    yaml = (root / "config/us/rnc1.us.yaml").read_text()
    for m in re.finditer(r"-\s*\[\s*0x([0-9a-fA-F]+)\s*,\s*c\s*,\s*([\w/]+)\s*\]", yaml):
        off, path = int(m.group(1), 16), m.group(2)
        base = path.split("/")[-1]
        key = ("f", None, off - 0x1000 + 0x100080)
        homes[key[1:]].add(f"src/{path}.c")
        if re.match(r"(fun_)?[0-9a-f]{6,8}$", base) or NOT_A_NAME.search(base):
            continue
        labels[key]["names"][base] += 1
        labels[key]["defs"][base] += 1
        labels[key]["files"][base].add(f"src/{path}.c")
    return labels, homes


def ntsc_places(labels, homes):
    """Place the NTSC decomp's labels on our symbols: a function through the US function table
    (config/versions/us/functions.tsv, its counterpart), a datum through the US translation table
    of tools/version_check.py, kept only where code of both versions refers to exactly that
    address. Returns ([row], {our function: their file})."""
    import versions
    import version_check
    import version_map
    us = versions.get("us")
    place = {}
    for e in version_map.read(us):
        for sp, a in e["places"]:
            if e["counterpart"]:
                place[(None if sp == "boot" else sp, a)] = e["counterpart"]
    table = None
    rows = []
    for (kind, lv, addr), lab in sorted(labels.items(), key=lambda kv: (kv[0][0], kv[0][1] or 0, kv[0][2])):
        where = f"US {'boot' if lv is None else f'level {lv:02d}'} 0x{addr:08X}"
        if kind == "f":
            sym = place.get((lv, addr)) or (place.get((None, addr)) if lv is not None else None)
            how = "US function table counterpart"
        else:
            if table is None:
                table = version_check.Table(us)
            ctx = "boot" if lv is None else lv
            p, sym = table.inverse(ctx, addr), None
            if p is not None:
                keys = table.sorted_obs.get(table.space_of(ctx, p), ([], []))[0]
                k = __import__("bisect").bisect_left(keys, p)
                if k < len(keys) and keys[k] == p:
                    sym = f"D_{p:08X}" if lv is None or p < version_check.PAL_MAIN else f"D_L{lv:02d}_{p:08X}"
            how = "US/PAL address translation (code of both refers to it)"
        if not sym:
            continue
        defs = lab["defs"]
        pool = defs or lab["names"]
        name = max(pool, key=lambda n: (pool[n], -len(n), n))
        files = sorted(lab["files"][name])
        rows.append({"symbol": sym, "name": name, "alts": sorted(set(lab["names"]) - {name}),
                     "evidence": f"{name} at {where} ({files[0]}); {how}"})
    modules = {}
    for (lv, addr), files in homes.items():
        sym = place.get((lv, addr)) or (place.get((None, addr)) if lv is not None else None)
        if sym and len(files) == 1:
            modules.setdefault(sym, set()).add(next(iter(files)))
    return rows, {s: next(iter(f)) for s, f in modules.items() if len(f) == 1}


def referenced_data() -> set[str]:
    """D_ symbols our sources and assembly name."""
    out = set()
    for d in ("src", "include", "asm"):
        for p in (ROOT / d).rglob("*"):
            if p.is_file() and p.suffix in (".c", ".h", ".s", ".inc"):
                out |= set(re.findall(r"\bD_(?:L\d\d_)?[0-9A-F]{8}\b", p.read_text(errors="replace")))
    return out


def ntsc_statuses(placed, rows) -> list[dict]:
    """Each placed name with its status: adopted (it replaces a name from our own reading or
    another later source, or fills a symbol without one), same, kept-original (our name is the
    original identifier, recovered), or held back as a candidate (several names for one label,
    one name on several symbols or several names on one symbol, a name another symbol keeps, a
    datum nothing of ours refers to, a core/SDK/libc function, which keeps its library's name)."""
    by_sym = {r["symbol"]: r for r in rows}
    per_sym, per_name, aliased = defaultdict(set), defaultdict(set), set()
    for p in placed:
        per_sym[p["symbol"]].add(p["name"])
        per_name[p["name"]].add(p["symbol"])
        if p["alts"]:
            aliased.add(p["symbol"])
    data = referenced_data()
    game = game_symbols()
    out = []
    for p in placed:
        ours = by_sym.get(p["symbol"])
        p = dict(p, ours=ours["name"] if ours else "", our_tier=ours["tier"] if ours else "",
                 our_source=ours["source"] if ours else "", our_evidence=ours["evidence"] if ours else "")
        if ours and ours["tier"] == "recovered":
            p["status"] = SAME if ours["name"] == p["name"] else KEPT
        elif p["symbol"].startswith("func_") and not p["symbol"].startswith("func_L") and p["symbol"] not in game:
            p["status"] = "held-library"   # core/SDK/libc: the library's own name, never an alias
        elif p["symbol"] in aliased:
            p["status"] = "held-alias"   # one of its places goes by several names there
        elif len(per_name[p["name"]]) > 1:
            p["status"] = "held-several-symbols"
        elif len(per_sym[p["symbol"]]) > 1:
            p["status"] = "held-several-names"
        elif p["symbol"].startswith("D_") and p["symbol"] not in data:
            p["status"] = "held-unreferenced"
        else:
            p["status"] = SAME if ours and ours["name"] == p["name"] and ours["tier"] != "candidate" else ADOPT
        out.append(p)
    # A name another symbol keeps (recovered, or ours where the NTSC decomp names nothing) is held.
    adopted = {p["symbol"] for p in out if p["status"] in (ADOPT, SAME)}
    keeps = {r["name"]: r["symbol"] for r in rows if r["tier"] != "candidate" and r["symbol"] not in adopted}
    for p in out:
        if p["status"] == ADOPT and keeps.get(p["name"], p["symbol"]) != p["symbol"]:
            p["status"] = "held-clash"
            p["evidence"] += f"; {p['name']} is {keeps[p['name']]}'s name here"
    return out


def ntsc_order(entries) -> list[dict]:
    order = {s: i for i, s in enumerate((ADOPT, SAME, KEPT) + HELD)}
    return sorted(entries, key=lambda e: (order[e["status"]], e["symbol"].startswith("D_"), e["symbol"],
                                          e["evidence"]))


def write_ntsc_table(entries, rev) -> None:
    entries = ntsc_order(entries)
    with open(NTSC_TABLE, "w", newline="") as f:
        f.write(f"# Generated by tools/names.py ntsc from the NTSC decomp at {rev[:10]} (tools/ntsc_source.py); "
                "see docs/NAMES.md. Columns:\n")
        f.write("# symbol\tname\tstatus\tother_names\tevidence\tour_name\tour_tier\tour_source\tour_evidence\n")
        for e in entries:
            f.write("\t".join(x.replace("\t", " ") for x in (
                e["symbol"], e["name"], e["status"], ",".join(e["alts"]), e["evidence"],
                e["ours"], e["our_tier"], e["our_source"], e["our_evidence"])) + "\n")


def read_ntsc_table() -> list[dict]:
    out = []
    if not NTSC_TABLE.exists():
        return out
    for line in NTSC_TABLE.read_text().splitlines():
        if line.startswith("#") or not line.strip():
            continue
        sym, name, status, alts, evidence, ours, tier, source, our_ev = (line.split("\t") + [""] * 9)[:9]
        out.append({"symbol": sym, "name": name, "status": status, "alts": [a for a in alts.split(",") if a],
                    "evidence": evidence, "ours": ours, "our_tier": tier, "our_source": source,
                    "our_evidence": our_ev})
    return out


def merge_ntsc(rows: list[dict], entries=None) -> list[dict]:
    """ROWS with config/names_ntsc.tsv applied: an adopted name replaces the symbol's name
    (tier descriptive, the old one goes to the alternatives); a held or kept-original name is
    listed among the alternatives, or as a candidate where the symbol has no name."""
    by_sym = {r["symbol"]: r for r in rows}
    for e in ntsc_order(read_ntsc_table() if entries is None else entries):
        old = by_sym.get(e["symbol"])
        if e["status"] == ADOPT:
            new = {"symbol": e["symbol"], "name": e["name"], "tier": "descriptive", "source": NTSC_SOURCE,
                   "evidence": e["evidence"], "alts": ""}
            if old:
                alts = [a for a in old["alts"].split(",") if a] + [old["name"]]
                new["alts"] = ",".join(sorted(set(a for a in alts if a != e["name"])))
                old.update(new)
            else:
                rows.append(new)
                by_sym[e["symbol"]] = new
        elif e["status"] != SAME:
            if old:
                if e["name"] != old["name"]:
                    alts = [a for a in old["alts"].split(",") if a] + [e["name"]]
                    old["alts"] = ",".join(sorted(set(alts)))
            else:
                new = {"symbol": e["symbol"], "name": e["name"], "tier": "candidate", "source": NTSC_SOURCE,
                       "evidence": f"{e['status']}: {e['evidence']}", "alts": ",".join(e["alts"])}
                rows.append(new)
                by_sym[e["symbol"]] = new
    return rows


def unmerge_ntsc(rows: list[dict], entries=None) -> list[dict]:
    """ROWS as they were before merge_ntsc applied ENTRIES (default config/names_ntsc.tsv):
    each adopted name gives back the row it replaced, which the table records."""
    entries = read_ntsc_table() if entries is None else entries
    by_sym = defaultdict(list)
    for e in entries:
        by_sym[e["symbol"]].append(e)
    out = []
    for r in rows:
        es = by_sym.get(r["symbol"], [])
        listed = {e["name"] for e in es if e["status"] not in (ADOPT, SAME)}
        if r["source"] == NTSC_SOURCE:
            e = next((e for e in es if e["status"] == ADOPT and e["name"] == r["name"]), None)
            if not e or not e["ours"]:
                continue   # a name the import added, or a candidate it listed
            alts = [a for a in r["alts"].split(",") if a and a != e["ours"] and a not in listed]
            r = {"symbol": r["symbol"], "name": e["ours"], "tier": e["our_tier"], "source": e["our_source"],
                 "evidence": e["our_evidence"], "alts": ",".join(alts)}
        elif listed:
            r = dict(r, alts=",".join(a for a in r["alts"].split(",") if a and a not in listed))
        out.append(r)
    return out


def write_table(rows) -> None:
    head = [l for l in TABLE.read_text().splitlines() if l.startswith("#")]
    with open(TABLE, "w", newline="") as f:
        f.write("\n".join(head) + "\n")
        for r in rows:
            f.write("\t".join(r[c].replace("\t", " ") for c in COLUMNS) + "\n")


def write_modules(modules, rev) -> None:
    with open(MODULES, "w", newline="") as f:
        f.write(f"# Generated by tools/names.py ntsc from the NTSC decomp at {rev[:10]} (tools/ntsc_source.py). "
                "Metadata only: the file\n# of the NTSC decomp that holds each of our functions. Our "
                "source files stay as they are\n# (each is a compile unit with its own flags, "
                "config/file_cflags.txt). Columns:\n# symbol\tntsc_path\n")
        for sym in sorted(modules, key=lambda s: (s.startswith("func_L"), s)):
            f.write(f"{sym}\t{modules[sym]}\n")


def ntsc() -> None:
    """Read the NTSC decomp's names from its pinned checkout (tools/ntsc_source.py), write config/names_ntsc.tsv and
    config/modules.tsv, and merge the adopted names into config/names.tsv. Then run
    `rename`, which carries the new names into names.h and src/."""
    import ntsc_source
    sub = ntsc_source.path()
    if not (sub / "config/us/rnc1.us.yaml").exists():
        sys.exit(f"{sub} is not a checkout of the NTSC decomp (tools/ntsc_source.py)")
    rev = ntsc_source.revision(sub)
    labels, homes = ntsc_labels(sub)
    placed, modules = ntsc_places(labels, homes)
    # statuses are judged against the table as it is without the earlier import
    base = unmerge_ntsc(read_table())
    entries = ntsc_statuses(placed, base)
    write_ntsc_table(entries, rev)
    write_modules(modules, rev)
    write_table(merge_ntsc(base, entries))
    c = Counter(e["status"] for e in entries)
    print(f"wrote {NTSC_TABLE.relative_to(ROOT)}: {len(entries)} names placed exactly "
          f"({', '.join(f'{c[s]} {s}' for s in (ADOPT, SAME, KEPT) + HELD)})")
    print(f"wrote {MODULES.relative_to(ROOT)}: {len(modules)} functions")


def rename() -> None:
    """Carry a changed table into names.h and the sources: every use of a name names.h gives
    a symbol that now has another name (or none) goes back to the address name in code and to
    the new spelling in comments; then `header`, `apply-defs` and `apply` write the new names
    where the tools write names."""
    old = {n: s for n, s in re.findall(r"(?m)^#define (\w+)\s+(\w+)$",
                                       HEADER.read_text() if HEADER.exists() else "") if n != "NAMES_H"}
    rows = read_table()
    named = {r["symbol"]: r["name"] for r in rows if r["tier"] != "candidate"}
    gone = {n: s for n, s in old.items() if named.get(s) != n}
    if gone:
        token = re.compile(r"\b(" + "|".join(re.escape(n) for n in sorted(gone, key=len, reverse=True)) + r")\b")
        lit = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)
        files = changed = 0
        for d in ("src", "include", "nonmatching"):
            for p in sorted((ROOT / d).rglob("*")):
                if p.suffix not in (".c", ".h") or p == HEADER:
                    continue
                text = p.read_text()
                n = 0

                def code(seg):
                    nonlocal n
                    out, k = token.subn(lambda m: gone[m.group(1)], seg)
                    n += k
                    return out

                def comment(seg):
                    nonlocal n
                    out, k = token.subn(lambda m: named.get(gone[m.group(1)], gone[m.group(1)]), seg)
                    n += k
                    return out
                parts, last = [], 0
                for m in lit.finditer(text):
                    parts.append(code(text[last:m.start()]))
                    seg = m.group(0)
                    parts.append(comment(seg) if seg.startswith("/") else seg)
                    last = m.end()
                parts.append(code(text[last:]))
                out = "".join(parts)
                if out != text:
                    p.write_text(out)
                    files += 1
                    changed += n
        print(f"{len(gone)} names changed or dropped: {changed} uses rewritten in {files} files")
    header()
    apply_defs()
    apply()


# ---------------------------------------------------------------- lookup

def lookup(args) -> None:
    rows = read_table()
    for a in args:
        hits = [r for r in rows if a in (r["symbol"], r["name"]) or a in r["alts"].split(",")]
        if not hits:
            print(f"{a}: no name")
        for r in hits:
            print(f"{r['symbol']}  {r['name']}  [{r['tier']}] {r['source']}: {r['evidence']}"
                  + (f"  (also: {r['alts']})" if r["alts"] else ""))


def main() -> None:
    args = sys.argv[1:]
    if not args:
        sys.exit(__doc__)
    cmd = {"build": build, "header": header, "apply": apply, "apply-defs": apply_defs, "local": local,
           "check": check, "ntsc": ntsc, "rename": rename}.get(args[0])
    if cmd:
        cmd()
    else:
        lookup(args)


if __name__ == "__main__":
    main()

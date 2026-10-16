#!/usr/bin/env python3
"""
Readable names for this build's symbols, and the header that lets C use
them while the linker symbol stays func_<ADDR> / func_LNN_<ADDR> / D_<ADDR>
(docs/NAMES.md).

  python3 tools/names.py build     # sources -> config/names.tsv
  python3 tools/names.py header    # config/names.tsv -> include/names.h
  python3 tools/names.py apply     # use the header's names in src/ C bodies
  python3 tools/names.py check     # names.h is current and collision-free
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
        " * write either spelling and compiles to the same object. Declare and",
        " * define with the address name; use the readable one in bodies.",
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
    cmd = {"build": build, "header": header, "apply": apply, "check": check}.get(args[0])
    if cmd:
        cmd()
    else:
        lookup(args)


if __name__ == "__main__":
    main()

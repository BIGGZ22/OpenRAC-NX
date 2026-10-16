#!/usr/bin/env python3
"""
Progress report for decomp.dev, in objdiff's report format (version 2).

decomp.dev reads a `report.json` that CI uploads as the artifact
`SCES_509.16_report`. Our CI cannot build the game: the SN ProDG
toolchain and the retail executable may not be redistributed. So the
report is generated HERE, from a real from-scratch build, and committed
as progress/report.json. The workflow only validates and uploads it.

Match data is the same audit tools/sweep_matches.py performs: each
decompiled function is compared with retail on size and bytes. Handwritten
assembly and linker remnants are also compared byte for byte, then counted
as finished original-source work. The report holds names, addresses, sizes
and percentages. It holds no retail bytes.

Usage:
  python tools/gen_progress_report.py            from-scratch build, then write
  python tools/gen_progress_report.py --no-build use the current build-sn/rac1.elf
  python tools/gen_progress_report.py --check    CI: fail if the committed
                                                 report is out of date with src/

--check needs neither the toolchain nor the baserom. It re-derives which
functions have C source or classified original assembly and compares that
with the committed report, so forgetting to regenerate fails the PR.
"""
import os
import json
import re
import subprocess
import sys
from pathlib import Path

GIT_PATHS = [
    r"C:\Program Files\Git\usr\bin",
    r"C:\Program Files\Git\bin",
    r"C:\Program Files (x86)\Git\usr\bin",
    r"C:\Program Files (x86)\Git\bin",
]

def ensure_env():
    env = os.environ.copy()
    current_path = env.get("PATH", "")
    additions = [p for p in GIT_PATHS if os.path.isdir(p) and p not in current_path]
    if additions:
        env["PATH"] = ";".join(additions) + ";" + current_path
    return env

sys.path.insert(0, str(Path(__file__).resolve().parent))
from libgcc_units import (MODULES, FUNCTIONS as LIBGCC_FUNCTIONS, SEGMENT_SOURCES,
                          object_of)
from organize_asm import LISTS, entries
from toolchain import TC, make_sn, sn, start_wineserver
from levels import NUM_LEVELS, dirname as level_dirname, level_of_dir, title as level_title

SHARED_TITLE = "Shared level code"


def is_level_unit(unit: dict) -> bool:
    """A level-code unit (src/overlays/), by category, whatever its name."""
    return "level_code" in (unit.get("metadata") or {}).get("progress_categories", [])

REPORT = Path("progress/report.json")
BASEROM = "baserom/SCES_509.16"
LINKED_ELF = "build-sn/rac1.elf"

# Level code overlays (docs/OVERLAYS.md, "Plan" step 3): each file of
# src/overlays/shared/ is a unit, and each src/overlays/lNN_<planet>/ is one
# unit named after its level, alongside the executable's own units. The level directories and their labels come from
# tools/levels.py.
OVERLAYS_SRC = Path("src/overlays")
OVERLAYS_CATALOGUE = Path("config/overlays/functions.tsv")
OVERLAY_NAME = re.compile(r"^func_L(\d{2})_([0-9A-Fa-f]{8})$")
WORK_ROOT = Path("build-sn/overlays/report")

# Same patterns as tools/try_func.py's OVERLAY_STUB/OVERLAY_DEF, mirrored
# (not imported) so --check can classify src/overlays/ with only the
# standard library: try_func.py and overlay_check.py import rabbitizer and
# pyelftools at module scope for the toolchain-driven build/compare path,
# which CI (no toolchain, no baserom) never runs.
OVERLAY_STUB = re.compile(r'^\s*(?:INCLUDE_ASM|LINKER_REMNANT)\([^)]*\b(func_L\d{2}_[0-9A-Fa-f]{8})\)')
# Level-code linker remnants (docs/ASM_CLASSIFICATION.md): what the linker left of stripped functions
# (each one's last delay slot), kept as assembly and counted as finished, as the executable's are.
OVERLAY_REMNANT = re.compile(r'^\s*LINKER_REMNANT\("asm/overlays",\s*(func_L\d{2}_[0-9A-Fa-f]{8})\);', re.M)
OVERLAY_REMNANTS = Path("config/overlays/linker_remnants.txt")
OVERLAY_JOINED = Path("config/overlays/joined.tsv")
OVERLAY_DEF = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b(func_L\d{2}_[0-9A-Fa-f]{8})(?:_r)?\s*\(")

# Same patterns as tools/sweep_matches.py (see the comments there on why
# the definition regex is lazy and skips `extern`).
FUNC_DEF = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b(func_[0-9A-Fa-f]{8})(?:_r)?\s*\(", re.M)
STUB = re.compile(r"INCLUDE_ASM\([^)]*\b(func_[0-9A-Fa-f]{8})\)")
ORIGINAL_ASM = re.compile(
    r"\b(ASM_FUNC|LINKER_REMNANT)\(\"asm/(handwritten|remnants)/(core_text|text)\",\s*"
    r"(func_[0-9A-F]{8})\)")
NONMATCHING = re.compile(r"nonmatching\s+(func_[0-9A-Fa-f]{8}),\s*(0x[0-9A-Fa-f]+)")
WORD = re.compile(r"/\* [0-9A-F]+ [0-9A-F]{8} ([0-9A-F]{8}) \*/")
LINKER_FILL = "CDCDCDCD"


def retail_functions() -> dict[str, tuple[str, int, int]]:
    """name -> (segment, vram, size) for every function splat found, except
    runs of retail's linker fill (0xCDCDCDCD between objects), which splat
    also emits as "functions". Fill is not code: it cannot be decompiled,
    and the build reproduces it byte for byte, so counting it would only
    keep objects from ever reading as complete."""
    out = {}
    for seg in ("core_text", "text"):
        for p in sorted(Path(f"asm/nonmatchings/{seg}").glob("func_*.s")):
            text = p.read_text(errors="replace")
            m = NONMATCHING.search(text)
            if not m:
                continue
            words = WORD.findall(text.split("endlabel")[0])
            if words and all(w == LINKER_FILL for w in words):
                continue
            out[m.group(1)] = (seg, int(m.group(1)[5:], 16), int(m.group(2), 16))
    return out


def with_source() -> set[str]:
    """Functions that have real source: game C, or GCC's for libgcc."""
    names = set(LIBGCC_FUNCTIONS)
    for srcs in SEGMENT_SOURCES.values():
        for src in srcs:
            text = Path(src).read_text(errors="replace")
            stubs = set(STUB.findall(text))
            names |= {n for n in FUNC_DEF.findall(text) if n not in stubs}
    return names


def classified_asm() -> dict[str, str]:
    """Tracked original assembly, checked against its source include macro.

    This validation uses only tracked files, so --check works in CI without
    a baserom or generated asm directory.
    """
    expected = {}
    for manifest, folder in LISTS:
        for entry in entries(manifest):
            segment, name = entry.split("/")
            if name in expected:
                sys.exit(f"*** duplicate original assembly classification: {name}")
            expected[name] = (folder, segment)
    found = {}
    for sources in SEGMENT_SOURCES.values():
        for source in sources:
            for macro, folder, segment, name in ORIGINAL_ASM.findall(Path(source).read_text()):
                if macro != {"handwritten": "ASM_FUNC", "remnants": "LINKER_REMNANT"}[folder]:
                    sys.exit(f"*** {name}: wrong original assembly macro {macro}")
                if name in found:
                    sys.exit(f"*** duplicate original assembly include: {name}")
                found[name] = (folder, segment)
    if found != expected:
        missing = sorted(set(expected) - set(found))
        extra = sorted(set(found) - set(expected))
        wrong = sorted(n for n in set(found) & set(expected) if found[n] != expected[n])
        sys.exit(f"*** original assembly manifests disagree with src/: "
                 f"missing {missing}, extra {extra}, wrong path {wrong}")
    return {name: folder for name, (folder, _) in expected.items()}


def unit_of(name: str, seg: str, vram: int) -> tuple[str, str, str]:
    """(unit name, source path, progress category)."""
    for mod, src, fns, stubs in MODULES:
        if name in fns or name in stubs:
            return f"libgcc/{mod}", src, "libgcc"
    if seg == "text":
        name, src, _ = object_of("text", vram)
        return name, src, "game"
    name, src, _ = object_of("core_text", vram)
    return f"core/{name}", src, "game"


def overlay_catalogue() -> dict[str, tuple[str, int]]:
    """name -> (kind, size) for every row of config/overlays/functions.tsv
    (tools/overlays.py catalogue; docs/OVERLAYS.md, "Names"). A tiny local
    parser (pure stdlib, like tools/overlay_src.py's own copy) instead of
    tools/overlay_check.py's load_catalogue(), since importing that module
    pulls in rabbitizer/pyelftools that --check must not need."""
    out = {}
    for line in OVERLAYS_CATALOGUE.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        name, kind, size, _fp, _nlevels, _places = line.split("\t")
        out[name] = (kind, int(size))
    return out


def overlay_dirs() -> list[tuple[Path, list[str]]]:
    """(directory, progress_categories) for src/overlays/shared/ and every
    src/overlays/lNN_<planet>/ that exists. Any other file directly under
    src/overlays/ (e.g. a test file) is not part of either directory and is
    ignored."""
    out = []
    shared = OVERLAYS_SRC / "shared"
    if shared.is_dir():
        out.append((shared, ["level_code", "common"]))
    for i in range(NUM_LEVELS):
        d = OVERLAYS_SRC / level_dirname(i)
        if d.is_dir():
            out.append((d, ["level_code", "levels", f"level_{i:02d}"]))
    return out


def overlay_file_functions(path: Path) -> list[tuple[str, bool]]:
    """[(name, is_c), ...] for every func_LNN_XXXXXXXX in PATH, in file
    order: is_c is True for a C definition, False for an INCLUDE_ASM stub
    (mirrors tools/try_func.py's find_overlay_stub(), generalised to every
    name in the file instead of stopping at one)."""
    lines = path.read_text(errors="replace").splitlines()
    out = []
    i = 0
    while i < len(lines):
        line = lines[i]
        m = OVERLAY_STUB.match(line)
        if m:
            out.append((m.group(1), False))
            i += 1
            continue
        m = OVERLAY_DEF.match(line)
        if m and not line.rstrip().endswith(";"):
            depth, seen, j = 0, False, i
            while j < len(lines):
                depth += lines[j].count("{") - lines[j].count("}")
                seen = seen or "{" in lines[j]
                if seen and depth == 0:
                    break
                j += 1
            out.append((m.group(1), True))
            i = j + 1
            continue
        i += 1
    return out


def overlay_remnants() -> set[str]:
    """The level-code linker remnants: config/overlays/linker_remnants.txt, which has to agree
    with the LINKER_REMNANT lines of src/overlays/ and name only catalogue entries.
    Tracked files only, so --check can run it; tools/overlay_remnants.py --check holds the
    list to the rule, against the level dumps."""
    listed = [l.strip() for l in OVERLAY_REMNANTS.read_text().splitlines()
              if l.strip() and not l.startswith("#")] if OVERLAY_REMNANTS.exists() else []
    if len(set(listed)) != len(listed):
        sys.exit(f"*** {OVERLAY_REMNANTS}: a name is listed twice")
    marked = []
    for directory, _ in overlay_dirs():
        for path in sorted(directory.glob("*.c")):
            marked += OVERLAY_REMNANT.findall(path.read_text(errors="replace"))
    if sorted(marked) != sorted(listed):
        sys.exit(f"*** {OVERLAY_REMNANTS} disagrees with src/overlays/: "
                 f"listed only {sorted(set(listed) - set(marked))}, marked only {sorted(set(marked) - set(listed))}")
    catalogue = overlay_catalogue()
    wrong = sorted(n for n in listed if n not in catalogue)
    if wrong:
        sys.exit(f"*** {OVERLAY_REMNANTS}: not a catalogue entry: {wrong}")
    return set(listed)


def overlay_joined() -> dict[str, list[str]]:
    """owner -> the catalogue entries after it that are pieces of the same C function
    (config/overlays/joined.tsv): finished when the owner is, as that file says."""
    if not OVERLAY_JOINED.exists():
        return {}
    rows = [l.split("\t", 1) for l in OVERLAY_JOINED.read_text().splitlines() if l and not l.startswith("#")]
    return {owner: pieces.split() for owner, pieces in rows}


def overlay_file_map() -> dict[Path, list[tuple[str, bool]]]:
    """path -> [(name, is_c), ...] for every file under src/overlays/shared/
    or src/overlays/lNN_<planet>/."""
    return {path: overlay_file_functions(path)
            for directory, _ in overlay_dirs()
            for path in sorted(directory.glob("*.c"))}


def _build_and_check(path: Path, c_names: list[str]) -> tuple[Path, dict[str, float] | str]:
    """One file of overlay_match_results(): {name: 100.0/0.0}, or the build
    log's tail when it fails. Top-level so a process pool can run it."""
    import try_func
    import overlay_check
    lines = path.read_text(errors="replace").splitlines()
    candidate = "\n".join(lines) + "\n"
    work = WORK_ROOT / path.parent.name / path.stem    # several levels have a variants.c
    # Build the whole file exactly once, as it stands (splicing the
    # file's own text back over itself), through try_func's "text"
    # pipeline -- the same recipe a single-function candidate goes
    # through, just covering every line instead of one stub's range.
    obj = try_func.build(c_names[0], "text", path, 0, len(lines) - 1, candidate, work)
    if obj is None:
        return path, (work / "log.txt").read_text(errors="replace")[-3000:]
    return path, {n: 100.0 if overlay_check.check(obj, n) == "EXACT" else 0.0 for n in c_names}


def _score_staged(name: str, path: str) -> tuple[str, float]:
    """A staged near miss's fuzzy_match_percent (tools/nonmatching.py).
    Top-level so a process pool can run it."""
    import nonmatching
    return name, nonmatching.score(name, Path(path))


def overlay_match_results(file_map: dict) -> dict[str, float]:
    """name -> 100.0 or 0.0 for every overlay function in FILE_MAP
    (docs/OVERLAYS.md, "Plan" step 3): only a C-defined function can score
    100, and only when tools/overlay_check.py says EXACT for it, built
    exactly the way tools/try_func.py builds an overlay function (its
    build() for the "text" segment pipeline). Needs the toolchain, so this
    is only ever called from generate(), never from check(). Files build
    in parallel, one per CPU: each has its own work directory."""
    from concurrent.futures import ProcessPoolExecutor

    out = {}
    jobs = []
    remnants = overlay_remnants()
    for path, fns in file_map.items():
        for n, is_c in fns:
            if not is_c:
                out[n] = 100.0 if n in remnants else 0.0
        c_names = [n for n, is_c in fns if is_c]
        if c_names:
            jobs.append((path, c_names))
    start_wineserver()      # one server for the pool, or Wine processes race to start one
    with ProcessPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        results = list(pool.map(_build_and_check, *zip(*jobs))) if jobs else []
    failed = [(path, r) for path, r in results if isinstance(r, str)]
    if failed:
        for path, log in failed:
            print(f"*** {path}: failed to build for the progress report:\n{log}", file=sys.stderr)
        sys.exit(f"*** {len(failed)} file(s) failed to build -- NOT writing a report")
    for _path, r in results:
        out.update(r)
    # A joined function is one C function under its first name, checked against the bytes of
    # all its pieces: the stubs kept for the later names are finished when it is.
    for owner, pieces in overlay_joined().items():
        if out.get(owner) == 100.0:
            out.update({piece: 100.0 for piece in pieces if piece in out})
    # Near misses staged in nonmatching/ (docs/NONMATCHING.md): their share
    # of matching bytes as fuzzy_match_percent. They are never finished:
    # only an EXACT C definition in src/overlays/ counts as matched code.
    import nonmatching
    staged = [(n, str(p)) for n, p in nonmatching.staged().items() if out.get(n, 100.0) == 0.0]
    if staged:
        with ProcessPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
            for n, pct in pool.map(_score_staged, *zip(*staged)):
                out[n] = min(pct, 99.99)
    return out


def overlay_units(file_map: dict, fuzzy: dict) -> tuple[list[dict], list, dict, dict, int, int]:
    """(out_units, all_items, cat_items, cat_complete, complete_units,
    complete_code) for src/overlays/shared/ (one unit, "Shared level code") and
    src/overlays/lNN_<planet>/ (one unit per level, named after it): a unit's functions are its files' INCLUDE_ASM'd or defined
    func_LNN_XXXXXXXX names in address order (docs/OVERLAYS.md, "Plan" step 3).
    cat_items/cat_complete cover "level_code", "common", "levels" and "level_NN" -- a
    level file's items count under both "levels" and its own "level_NN"."""
    catalogue = overlay_catalogue()
    out_units = []
    all_items = []
    cat_items = {"level_code": [], "common": [], "levels": []}
    cat_complete = {"level_code": 0, "common": 0, "levels": 0}
    for i in range(NUM_LEVELS):
        cat_items[f"level_{i:02d}"] = []
        cat_complete[f"level_{i:02d}"] = 0
    complete_units = 0
    complete_code = 0

    # decomp.dev lays its treemap out in report order and draws no border
    # around a group, so a level is one unit (one box, its functions inside)
    # and so is the shared code.
    for directory, cats in overlay_dirs():
        shared = directory.name == "shared"
        groups = []     # [(unit name, source path, rows)]
        for path in sorted(directory.glob("*.c")):
            fns = file_map[path]
            if not fns:
                continue
            rows = []
            for name, is_c in fns:
                if name not in catalogue:
                    sys.exit(f"*** {path}: {name} is not in {OVERLAYS_CATALOGUE}")
                _kind, size = catalogue[name]
                m = OVERLAY_NAME.match(name)
                rows.append((name, int(m.group(2), 16), size, is_c))
            if groups:
                groups[0][2].extend(rows)
            else:
                groups.append((SHARED_TITLE if shared else level_title(level_of_dir(directory.name)),
                               directory.as_posix(), rows))
        for uname, source, rows in groups:
            rows.sort(key=lambda r: r[1])
            start = rows[0][1]
            items = [(size, fuzzy.get(name, 0.0), fuzzy.get(name, 0.0) == 100.0)
                     for name, _addr, size, _is_c in rows]
            complete = all(e for _, _, e in items)
            code = sum(s for s, _, _ in items)
            if complete:
                complete_units += 1
                complete_code += code
            all_items += items
            for cat in cats:
                cat_items[cat] += items
                if complete:
                    cat_complete[cat] += code
            out_units.append({
                "name": uname,
                "measures": measures(items, 1, int(complete), code if complete else 0),
                "functions": [{
                    "name": name,
                    "size": str(size),
                    "fuzzy_match_percent": fuzzy.get(name, 0.0),
                    "address": str(addr - start),
                    # objdiff's schema allows no other per-function metadata.
                    "metadata": {"virtual_address": str(addr)},
                } for name, addr, size, is_c in rows],
                "metadata": {
                    "complete": complete,
                    "source_path": source,
                    "progress_categories": cats,
                },
            })
    return out_units, all_items, cat_items, cat_complete, complete_units, complete_code


def overlay_c_functions() -> tuple[dict, set[str]]:
    """(file_map, names with real C) from src/overlays/ alone, cross-checked
    against config/overlays/functions.tsv (every name must be catalogued: a
    file can hold a function of the other kind too, when a branch into the
    next place forced a shared/level pair together across that boundary --
    docs/OVERLAYS.md, "Plan" step 2 -- so only catalogue membership is
    checked here, not which kind). Pure stdlib: used by --check, which has
    neither the toolchain nor the baserom."""
    catalogue = overlay_catalogue()
    file_map = overlay_file_map()
    have_c = set()
    for directory, _cats in overlay_dirs():
        for path in sorted(directory.glob("*.c")):
            for name, is_c in file_map[path]:
                if name not in catalogue:
                    sys.exit(f"*** {path}: {name} is not in {OVERLAYS_CATALOGUE}")
                if is_c:
                    have_c.add(name)
    return file_map, have_c


def build() -> None:
    """From-scratch build + link. Refuses on failure: a failed make leaves
    the previous .o behind, whose INCLUDE_ASM stubs still hold retail's
    bytes, and a report built on that would publish fictional matches."""
    for d in ("build-sn/core", "build-sn/libgcc", "build-sn/game"):
        for f in Path(d).rglob("*.o") if Path(d).is_dir() else []:
            f.unlink()
    for f in Path("build-sn/libgcc").glob("*.o") if Path("build-sn/libgcc").is_dir() else []:
        f.unlink()
    start_wineserver()
    steps = [
        [f"{TC}/make.exe", "-f", "Makefile.sn"],
        [sys.executable, "tools/gen_ld.py"],
        [f"{TC}/ee-ld.exe", "-T", "build-sn/rac1.ld", "build-sn/bss_equs.o", "-o", LINKED_ELF],
    ]
    for cmd in steps:
        r = subprocess.run(cmd, capture_output=True, text=True, env=ensure_env())
        if r.returncode != 0:
            sys.stdout.write(r.stdout[-3000:] + r.stderr[-3000:])
            sys.exit(f"*** {' '.join(cmd[:1])} failed (exit {r.returncode}) -- NOT writing a report")


def match_results(funcs: dict, decompiled: set[str]) -> dict[str, float]:
    """name -> fuzzy match percent for checked functions (100.0 = exact,
    size AND bytes). A size mismatch scores 0: it is not a near-miss, it
    breaks everything after it."""
    from elftools.elf.elffile import ELFFile

    raw = Path(BASEROM).read_bytes()
    with open(BASEROM, "rb") as f:
        seg = next(s for s in ELFFile(f).iter_segments() if s["p_type"] == "PT_LOAD")
        delta = seg["p_vaddr"] - seg["p_offset"]

    out = {}
    with open(LINKED_ELF, "rb") as f:
        elf = ELFFile(f)
        symtab = list(elf.get_section_by_name(".symtab").iter_symbols())
        syms = {s.name: s for s in symtab}
        # Linker-script aliases (libgcc's func_ names) carry st_size 0; the
        # real symbol at the same address (__adddf3, or the local
        # _fpadd_parts) has the size gcc emitted.
        sized_at = {}
        for s in symtab:
            if s["st_size"] and s["st_info"]["type"] == "STT_FUNC":
                sized_at.setdefault(s["st_value"], s["st_size"])
        secs = {i: (elf.get_section(i)["sh_addr"], elf.get_section(i).data())
                for i in range(elf.num_sections())}
        for name in sorted(decompiled):
            if name not in funcs or name not in syms:
                sys.exit(f"*** {name}: has source but no retail .s or no symbol")
            _, vram, size = funcs[name]
            sym = syms[name]
            # libgcc names are linker aliases (absolute symbols): read those
            # through the section that covers their address instead.
            idx = sym["st_shndx"]
            if not isinstance(idx, int):
                idx = next(i for i, (a, d) in secs.items()
                           if a and a <= vram < a + len(d))
            base, data = secs[idx]
            ours = data[vram - base: vram - base + size]
            orig = raw[vram - delta: vram - delta + size]
            osize = sym["st_size"] or sized_at.get(sym["st_value"])
            if sym["st_value"] != vram:
                sys.exit(f"*** {name} is at {sym['st_value']:#x}, not its retail address")
            if osize is None or osize != size:
                out[name] = 0.0
                continue
            same = sum(1 for a, b in zip(orig, ours) if a == b)
            out[name] = 100.0 * same / size
    return out


# Retail's initialised data (SCES_509.16's own section names). .bss and core.bss hold no bytes to
# compare, and the level programs' data is not measured (yet): the data measures cover the
# executable's data only.
RETAIL_DATA_SECTIONS = ("core.data", "core.rdata", "core.lit", ".lit", ".data",
                        "lvl.vtbl", "lvl.camvtbl", "lvl.sndvtbl")
EXE_LABEL = re.compile(r"^(?:D|jtbl)_([0-9A-F]{8})$")


def compiled_data() -> list[tuple[str, str, int]]:
    """(object, input section, retail address) for every piece of retail's data that a compiled
    object provides itself instead of the retail blobs in asm/data/: core_rdata's literals
    (config/core_rodata.txt) and the switch jump tables of decompiled game functions
    (tools/jump_tables.py). The link puts each in the hole cut for it at its first label, whose
    name is its retail address."""
    import core_rodata
    import jump_tables
    out = []
    for obj, labels in core_rodata.entries():
        out.append((obj, ".rodata", int(EXE_LABEL.match(labels[0]).group(1), 16)))
    for label, _func, obj in jump_tables.provided():
        m = EXE_LABEL.match(label)
        if m:                                      # level code's tables live in the levels
            out.append((obj, f".rodata.{label}", int(m.group(1), 16)))
    return out


def unit_of_object(obj: str) -> str:
    """The report unit an object's code is counted under (see unit_of)."""
    name = obj.rsplit("/", 1)[1][:-2]
    if obj.startswith("build-sn/libgcc/"):
        return "libgcc/" + name.removeprefix("l2_")
    if obj.startswith("build-sn/core/"):
        return f"core/{name}"
    return obj[len("build-sn/"):-2]


def data_results() -> tuple[int, list[tuple[str, int, bool]]]:
    """(retail's initialised data in bytes, [(unit, size, exact), ...] for each piece compiled
    objects provide). A piece is exact when the linked image holds retail's bytes at its retail
    address; the blobs leave a hole exactly there, so those bytes are the object's."""
    from elftools.elf.elffile import ELFFile

    with open(BASEROM, "rb") as f:
        retail = ELFFile(f)
        total = sum(retail.get_section_by_name(n)["sh_size"] for n in RETAIL_DATA_SECTIONS)
        seg = next(s for s in retail.iter_segments() if s["p_type"] == "PT_LOAD")
        delta = seg["p_vaddr"] - seg["p_offset"]
    raw = Path(BASEROM).read_bytes()
    with open(LINKED_ELF, "rb") as f:
        linked = [(s["sh_addr"], s.data()) for s in ELFFile(f).iter_sections()
                  if s["sh_addr"] and s["sh_type"] == "SHT_PROGBITS"]

    def ours(addr: int, size: int) -> bytes:
        for base, data in linked:
            if base <= addr and addr + size <= base + len(data):
                return data[addr - base: addr - base + size]
        sys.exit(f"*** no linked section holds {addr:#x}+{size:#x}")

    pieces = []
    for obj, section, addr in compiled_data():
        with open(obj, "rb") as f:
            sec = ELFFile(f).get_section_by_name(section)
            if sec is None:
                sys.exit(f"*** {obj} has no {section} section")
            size = sec["sh_size"]
        exact = ours(addr, size) == raw[addr - delta: addr - delta + size]
        pieces.append((unit_of_object(obj), size, exact))
    return total, pieces


def data_measures(total: int, pieces: list[tuple[str, int, bool]], complete: set[str]) -> dict:
    """total_data, matched_data and complete_data for the report's measures (uint64s as strings).
    Data still assembled from retail's dumps is not matched: only what compiled objects bring."""
    matched = sum(size for _u, size, exact in pieces if exact)
    done = sum(size for u, size, exact in pieces if exact and u in complete)
    return {
        "total_data": str(total),
        "matched_data": str(matched),
        "matched_data_percent": 100.0 * matched / total if total else 0.0,
        "complete_data": str(done),
        "complete_data_percent": 100.0 * done / total if total else 0.0,
    }


def measures(items: list[tuple[int, float, bool]], units: int = 0, complete_units: int = 0,
             complete_code: int = 0) -> dict:
    """items: (size, fuzzy%, finished). uint64 fields are strings, as protobuf
    JSON encodes them."""
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


def generate() -> dict:
    funcs = retail_functions()
    decompiled = with_source()
    original_asm = classified_asm()
    if decompiled & original_asm.keys():
        sys.exit(f"*** C source and original assembly overlap: {sorted(decompiled & original_asm.keys())}")
    fuzzy = match_results(funcs, decompiled | original_asm.keys())
    not_exact = sorted(name for name in original_asm if fuzzy[name] != 100.0)
    if not_exact:
        sys.exit(f"*** classified original assembly differs from retail: {not_exact}")

    units: dict[str, dict] = {}
    for name, (seg, vram, size) in sorted(funcs.items(), key=lambda kv: kv[1][1]):
        uname, src, cat = unit_of(name, seg, vram)
        u = units.setdefault(uname, {"src": src, "cat": cat, "start": vram, "fns": []})
        pct = fuzzy.get(name, 0.0)
        u["fns"].append((name, vram, size, pct, pct == 100.0))

    out_units = []
    all_items, cat_items = [], {"game": [], "libgcc": []}
    cat_complete = {"game": 0, "libgcc": 0}
    complete_units = 0
    complete_names = set()
    for uname, u in units.items():
        items = [(s, p, e) for _, _, s, p, e in u["fns"]]
        complete = all(e for _, _, e in items)
        code = sum(s for s, _, _ in items)
        if complete:
            complete_units += 1
            complete_names.add(uname)
            cat_complete[u["cat"]] += code
        all_items += items
        cat_items[u["cat"]] += items
        out_units.append({
            "name": uname,
            "measures": measures(items, 1, int(complete), code if complete else 0),
            "functions": [{
                "name": n,
                "size": str(s),
                "fuzzy_match_percent": p,
                "address": str(v - u["start"]),
                # objdiff's schema allows no other per-function metadata;
                # the asm classification lives in config/ (checked by --check).
                "metadata": {"virtual_address": str(v)},
            } for n, v, s, p, _ in u["fns"]],
            "metadata": {
                "complete": complete,
                "source_path": u["src"],
                "progress_categories": ["executable", u["cat"]],
            },
        })

    # Level code overlays (docs/OVERLAYS.md, "Plan" step 3): same report,
    # covering src/overlays/ too, so the totals below are of the whole
    # game's code. The executable's own units/categories above are
    # untouched by any of this.
    overlay_map, _have_c = overlay_c_functions()
    overlay_fuzzy = overlay_match_results(overlay_map)
    (overlay_out_units, overlay_items, overlay_cat_items, overlay_cat_complete,
     overlay_complete_units, overlay_complete_code) = overlay_units(overlay_map, overlay_fuzzy)

    # The executable's data: whole-image numbers, in the top-level measures and the
    # executable's category (the retail dumps can't be split by unit, so units carry none).
    data_total, data_pieces = data_results()
    data = data_measures(data_total, data_pieces, complete_names)

    # The two halves of the game's code, each as one number: the
    # executable (SCES_509.16) and all level code (common + level-specific).
    categories = [
        {"id": "executable", "name": "Executable",
         "measures": measures(cat_items["game"] + cat_items["libgcc"],
                              complete_code=cat_complete["game"] + cat_complete["libgcc"]) | data},
        {"id": "level_code", "name": "Level code",
         "measures": measures(overlay_cat_items["level_code"],
                              complete_code=overlay_cat_complete["level_code"])},
        {"id": "game", "name": "Executable game code",
         "measures": measures(cat_items["game"], complete_code=cat_complete["game"])},
        {"id": "libgcc", "name": "libgcc",
         "measures": measures(cat_items["libgcc"], complete_code=cat_complete["libgcc"])},
        {"id": "common", "name": SHARED_TITLE,
         "measures": measures(overlay_cat_items["common"], complete_code=overlay_cat_complete["common"])},
        {"id": "levels", "name": "Level-specific code",
         "measures": measures(overlay_cat_items["levels"], complete_code=overlay_cat_complete["levels"])},
    ] + [
        {"id": f"level_{i:02d}", "name": level_title(i),
         "measures": measures(overlay_cat_items[f"level_{i:02d}"],
                              complete_code=overlay_cat_complete[f"level_{i:02d}"])}
        for i in range(NUM_LEVELS)
    ]

    return {
        "measures": measures(all_items + overlay_items,
                             len(units) + len(overlay_out_units),
                             complete_units + overlay_complete_units,
                             sum(cat_complete.values()) + overlay_complete_code) | data,
        "units": out_units + overlay_out_units,
        "version": 2,
        "categories": categories,
    }


def check() -> None:
    # The build runs only documented assembler/linker steps, with flags per file (docs/BUILD_FIDELITY.md).
    import check_build_fidelity
    check_build_fidelity.main()
    if not REPORT.exists():
        sys.exit(f"*** {REPORT} missing -- run: python tools/gen_progress_report.py")
    report = json.loads(REPORT.read_text())
    # The executable's own staleness check (source vs. report), unchanged:
    # restricted to the executable's units so a level-code function that
    # becomes EXACT (checked separately below) can't look like a retail
    # function whose source disappeared.
    in_report = {f["name"] for u in report["units"] if not is_level_unit(u)
                 for f in u["functions"] if f.get("fuzzy_match_percent", 0) > 0}
    original_asm = classified_asm()
    have = with_source() | original_asm.keys()
    stale_new = sorted(have - in_report)
    stale_gone = sorted(in_report - have)
    if stale_new or stale_gone:
        print("progress/report.json is out of date with src/:")
        for n in stale_new:
            print(f"  has source, report says not decompiled: {n}")
        for n in stale_gone:
            print(f"  report says decompiled, no source any more: {n}")
        sys.exit("*** regenerate with: python tools/gen_progress_report.py")
    unfinished_asm = sorted(f["name"] for u in report["units"] for f in u["functions"]
                            if f["name"] in original_asm and f["fuzzy_match_percent"] != 100.0)
    if unfinished_asm:
        sys.exit(f"*** classified original assembly is not finished in the report: {unfinished_asm}")

    # Level code overlays (docs/OVERLAYS.md, "Plan" step 3): re-derive which
    # func_LNN_XXXXXXXX are C (not INCLUDE_ASM) from src/overlays/ and
    # config/overlays/functions.tsv alone -- no toolchain, no baserom -- and
    # compare that against the overlay functions the committed report counts
    # as matched (only exact C is ever committed). This cannot re-verify
    # EXACT-ness (that needs a real build), only that the report has not
    # gone stale about which functions have C.
    _overlay_map, have_c = overlay_c_functions()
    have_c |= overlay_remnants()        # classified original assembly counts as finished too
    level_names = overlay_catalogue()
    have_c |= {piece for owner, pieces in overlay_joined().items() if owner in have_c
               for piece in pieces if level_names.get(piece, ("exe",))[0] != "exe"}
    reported_c = {f["name"] for u in report["units"] for f in u["functions"]
                  if is_level_unit(u)
                  and f.get("fuzzy_match_percent", 0) == 100.0}
    stale_new_c = sorted(have_c - reported_c)
    stale_gone_c = sorted(reported_c - have_c)
    if stale_new_c or stale_gone_c:
        print("progress/report.json is out of date with src/overlays/:")
        for n in stale_new_c:
            print(f"  has C (or is a listed remnant or a joined piece), report doesn't mark it: {n}")
        for n in stale_gone_c:
            print(f"  report marks it as C, no C source any more: {n}")
        sys.exit("*** regenerate with: python tools/gen_progress_report.py")

    m = report["measures"]
    print(f"report is current: {m['matched_functions']}/{m['total_functions']} functions, "
          f"{m['matched_code_percent']:.2f}% code finished")


def main() -> None:
    args = sys.argv[1:]
    if "--check" in args:
        check()
        return
    if "--no-build" not in args:
        build()
    report = generate()
    REPORT.parent.mkdir(exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=1) + "\n", newline="\n")
    m = report["measures"]
    print(f"wrote {REPORT}: {m['matched_functions']}/{m['total_functions']} functions finished, "
          f"{m['matched_code_percent']:.2f}% code, {m['complete_units']}/{m['total_units']} units complete, "
          f"{m['matched_data_percent']:.2f}% of the executable's data")


if __name__ == "__main__":
    main()

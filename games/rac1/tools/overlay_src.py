#!/usr/bin/env python3
"""
src/overlays/...: C sources for every "shared" and "level" function in
config/overlays/functions.tsv, as INCLUDE_ASM stubs (docs/OVERLAYS.md,
"Plan" step 2).

  python3 tools/overlay_src.py

Needs config/overlays/functions.tsv (tools/overlays.py catalogue),
progress/report.json (the executable's units) and
baserom/overlays/level_NN/{manifest.json,text.bin} (tools/overlays.py
dump), to decode branch instructions when deciding whether a function must
stay glued to the one after it. asm/overlays/<name>.s does not need to
exist yet -- a stub is written regardless (tools/overlay_asm.py fills in
the .s files separately, including for the 317 functions with a jump
table that it currently skips).

Directory and grouping rules, from docs/OVERLAYS.md:

- A function's home directory is decided by its kind alone: "shared" goes
  to src/overlays/shared/, "level" to src/overlays/lNN_<planet>/ (its canonical
  level). Each directory accumulates its own run of files, independently
  of the other.
- Within one directory's stream (functions in canonical (level, address)
  order), a new file starts when the canonical level changes (shared
  only -- a level directory's own level never changes), when the
  *preceding unit* changes, or when the running file would pass about
  32 KB. The preceding unit is the executable unit (progress/report.json)
  of the nearest exe-kind function before this one in its canonical
  level, counting only exe functions of 32 bytes or more that occur once
  in that level as anchors ("start" if none precedes it).
- A function whose own bytes branch to the address right after its own
  end (decoded from baserom/overlays/level_NN/text.bin) -- i.e. into the
  next place the catalogue recorded in that level -- must be immediately
  followed by that place's function in the same file, even across the
  shared/level boundary above (a level function's file gains an
  out-of-place shared neighbour, or vice versa). When the next place is
  an exe-kind function (already compiled in src/game/, nothing to stub)
  or a repeat occurrence of a shared function whose real canonical home
  is a different, earlier level, the two cannot be kept together; this is
  counted and printed instead.

Each file is named <unit>_<first function's address, 8 hex digits>.c,
with '/' in a unit name (e.g. "movie/movie") written as '_'.

Re-running is safe: a file that isn't exactly the generated header, the
two #include lines and INCLUDE_ASM lines (i.e. one a human or a matcher
has started replacing stubs with real C in) is left untouched, with a
warning.
"""
import json
import re
import struct
import sys
from bisect import bisect_left
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from levels import dirname as level_dirname, level_of_dir, LEVELS

ROOT = Path(__file__).resolve().parent.parent
CATALOGUE = ROOT / "config/overlays/functions.tsv"
REPORT = ROOT / "progress/report.json"
DUMP = ROOT / "baserom/overlays"
OUT = ROOT / "src/overlays"

FILE_BUDGET = 32 * 1024          # "about 32 KB" of code per file
ANCHOR_MIN_SIZE = 32              # exe anchors must be at least this big

NAME_LEVEL_RE = re.compile(r"^func_L(\d{2})_([0-9A-Fa-f]{8})$")

HEADER_RE = re.compile(
    r"^/\* .*\*/\n"
    r'#include "common\.h"\n'
    r'#include "include_asm\.h"\n'
    r"\n"
    r"(INCLUDE_ASM\(\"asm/overlays\", func_L\d{2}_[0-9A-Fa-f]{8}\);\n)+$"
)


# ---------------------------------------------------------------- catalogue

def parse_catalogue():
    rows = []
    with CATALOGUE.open() as f:
        for line in f:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            name, kind, size, _fp, _nlevels, places = line.split("\t")
            place_list = []
            for p in places.split(","):
                lv, addr = p.split(":")
                place_list.append((int(lv), int(addr, 16)))
            rows.append({"name": name, "kind": kind, "size": int(size), "places": place_list})
    return rows


def canonical(row):
    """(level, address) a shared/level row's name already encodes."""
    m = NAME_LEVEL_RE.match(row["name"])
    if not m:
        sys.exit(f"{row['name']}: not a func_LNN_XXXXXXXX name")
    return int(m.group(1)), int(m.group(2), 16)


# --------------------------------------------------------- executable units

def load_unit_map():
    """exe function name -> its progress/report.json unit, "game/" stripped."""
    report = json.loads(REPORT.read_text())
    name_to_unit = {}
    for unit in report["units"]:
        if not unit["name"].startswith("game/"):
            continue
        unit_id = unit["name"][len("game/"):]
        for f in unit["functions"]:
            name_to_unit[f["name"]] = unit_id
    return name_to_unit


# ------------------------------------------------------------- level tiling

def build_level_tiling(rows):
    """
    level -> sorted [address], level -> {address: (name, kind)}: every
    place any row occupies in that level, i.e. the exact original function
    boundaries the catalogue split that level's text into.
    """
    owner = defaultdict(dict)
    for row in rows:
        for lv, addr in row["places"]:
            if addr in owner[lv]:
                sys.exit(f"level {lv:02d}: duplicate place {addr:08X} "
                          f"({owner[lv][addr][0]} and {row['name']})")
            owner[lv][addr] = (row["name"], row["kind"])
    addrs = {lv: sorted(o) for lv, o in owner.items()}
    return addrs, owner


def build_anchors(rows):
    """level -> sorted [(address, unit)] of exe-kind anchor functions."""
    name_to_unit = load_unit_map()
    per_level = defaultdict(list)
    for row in rows:
        if row["kind"] != "exe":
            continue
        by_level = defaultdict(list)
        for lv, addr in row["places"]:
            by_level[lv].append(addr)
        if row["size"] < ANCHOR_MIN_SIZE:
            continue
        unit = name_to_unit.get(row["name"])
        if unit is None:
            print(f"warning: {row['name']} (exe) has no unit in {REPORT.relative_to(ROOT)}",
                  file=sys.stderr)
            continue
        for lv, addr_list in by_level.items():
            if len(addr_list) == 1:
                per_level[lv].append((addr_list[0], unit))
    anchors = {}
    for lv, lst in per_level.items():
        lst.sort()
        anchors[lv] = ([a for a, _ in lst], [u for _, u in lst])
    return anchors


def preceding_unit(anchors, level, address):
    addrs, units = anchors.get(level, ([], []))
    i = bisect_left(addrs, address)
    if i == 0:
        return "start"
    return units[i - 1]


# ---------------------------------------------------------- branch decoding

def branch_target(word, addr):
    """PC-relative target of WORD if it is a branch, else None. (No jal/j:
    those are absolute and irrelevant to falling into the next place.)"""
    op = word >> 26
    imm = word & 0xFFFF
    simm = imm - 0x10000 if imm & 0x8000 else imm
    if op in (4, 5, 6, 7, 20, 21, 22, 23):           # beq/bne/blez/bgtz (+likely)
        return addr + 4 + (simm << 2)
    if op == 1:                                       # regimm: bltz/bgez family
        rt = (word >> 16) & 0x1F
        if rt in (0, 1, 2, 3, 16, 17, 18, 19):
            return addr + 4 + (simm << 2)
    if op in (0x11, 0x12):                             # cop1/cop2 branches
        fmt = (word >> 21) & 0x1F
        if fmt == 0x08:
            return addr + 4 + (simm << 2)
    return None


class Levels:
    """Cached baserom/overlays/level_NN/{manifest.json,text.bin}."""

    def __init__(self):
        self._text = {}

    def _load(self, lv):
        if lv not in self._text:
            d = DUMP / f"level_{lv:02d}"
            manifest = json.loads((d / "manifest.json").read_text())
            rec = next(r for r in manifest["records"] if r["name"] == "text")
            data = (d / "text.bin").read_bytes()
            self._text[lv] = (rec["address"], data)
        return self._text[lv]

    def branches_to(self, level, start, end, target):
        base, data = self._load(level)
        w0, w1 = (start - base) // 4, (end - base) // 4
        for wi in range(w0, w1):
            word = struct.unpack_from("<I", data, wi * 4)[0]
            if branch_target(word, base + wi * 4) == target:
                return True
        return False


# --------------------------------------------------- "branches into next"

def compute_forces_next(rows, by_name, level_addrs, level_owner, levels):
    """
    name -> the name of the next catalogued place in its canonical level,
    when this function's own bytes branch straight into it (so the two
    must land in the same file); plus counts of the cases where that next
    place cannot actually be kept together (another kind's canonical home
    is elsewhere, or it is an exe-kind function already compiled
    elsewhere).
    """
    forces = {}
    exceptions = {"exe_next": [], "noncanonical_next": []}
    for row in rows:
        if row["kind"] not in ("shared", "level"):
            continue
        level, addr = canonical(row)
        size = row["size"]
        addrs = level_addrs[level]
        i = bisect_left(addrs, addr)
        if i + 1 >= len(addrs):
            continue                       # last place in the level: no next
        next_start = addrs[i + 1]
        end = addr + size
        if end > next_start:
            sys.exit(f"{row['name']}: size runs past the next place "
                      f"({end:08X} > {next_start:08X})")
        if not levels.branches_to(level, addr, next_start, next_start):
            continue
        next_name, next_kind = level_owner[level][next_start]
        if next_kind == "exe":
            exceptions["exe_next"].append((row["name"], next_name))
            continue
        # next_kind is shared or level: is this occurrence its canonical one?
        next_row = by_name[next_name]
        n_level, n_addr = canonical(next_row)
        if (n_level, n_addr) != (level, next_start):
            exceptions["noncanonical_next"].append((row["name"], next_name))
            continue
        forces[row["name"]] = next_name
    return forces, exceptions


# ---------------------------------------------------------------- grouping

class FileGroup:
    __slots__ = ("directory", "unit_token", "first_addr", "rows", "size")

    def __init__(self, directory, unit_token, first_addr):
        self.directory = directory
        self.unit_token = unit_token
        self.first_addr = first_addr
        self.rows = []
        self.size = 0


def group_stream(stream, anchors, forces, level_of_stream_is_fixed, directory_of):
    """
    Split STREAM (rows already in canonical (level, address) order for one
    directory's worth of work) into FileGroup objects, honouring forced
    same-file pairs even where they defy the ordinary triggers. Returns
    (groups, kept_together_pairs, overridden_pairs).
    """
    groups = []
    cur = None
    cur_level = cur_unit = None
    pending_force = False
    kept_together = 0
    overridden = 0
    for row in stream:
        level, addr = canonical(row)
        unit = preceding_unit(anchors, level, addr)
        size = row["size"]
        if cur is None:
            start_new = True
        elif pending_force:
            start_new = False
        else:
            start_new = (level != cur_level or unit != cur_unit
                         or cur.size + size > FILE_BUDGET)
        if pending_force and cur is not None:
            would_split = (level != cur_level or unit != cur_unit
                           or cur.size + size > FILE_BUDGET)
            kept_together += 1
            if would_split:
                overridden += 1
        if start_new:
            if cur is not None:
                groups.append(cur)
            cur = FileGroup(directory_of(level), unit, addr)
            cur_level, cur_unit = level, unit
        cur.rows.append(row)
        cur.size += size
        pending_force = forces.get(row["name"]) is not None
    if cur is not None:
        groups.append(cur)
    return groups, kept_together, overridden


def splice_cross_kind_pairs(shared_groups, level_groups, forces, by_name, cross_pairs):
    """
    A forced pair whose two functions are of different kinds (shared vs
    level) cannot come out of group_stream() together, since each kind is
    grouped in its own independent stream. Move the second function into
    the first's file, immediately after it, and drop it from wherever its
    own stream put it.
    """
    all_groups = shared_groups + [g for gs in level_groups.values() for g in gs]
    loc = {}
    for g in all_groups:
        for idx, r in enumerate(g.rows):
            loc[r["name"]] = (g, idx)
    moved = 0
    for first_name, next_name in cross_pairs:
        if first_name not in loc or next_name not in loc:
            continue
        g1, i1 = loc[first_name]
        g2, i2 = loc[next_name]
        if g1 is g2:
            continue                       # already adjacent by luck
        row2 = g2.rows.pop(i2)
        g2.size -= row2["size"]
        g1.rows.insert(i1 + 1, row2)
        g1.size += row2["size"]
        # fix up indices after the removal/insertion for later lookups
        for j, r in enumerate(g1.rows):
            loc[r["name"]] = (g1, j)
        for j, r in enumerate(g2.rows):
            loc[r["name"]] = (g2, j)
        moved += 1
    return moved


# ------------------------------------------------------------------ output

def unit_token(unit):
    return "start" if unit == "start" else unit.replace("/", "_")


def file_path(group):
    return group.directory / f"{unit_token(group.unit_token)}_{group.first_addr:08X}.c"


def render(group):
    names = [r["name"] for r in group.rows]
    dirname = group.directory.name
    level = level_of_dir(dirname)
    kind_desc = "Shared code" if level is None else f"Level {level:02d} ({LEVELS[level][2]}) code"
    where = "before the first executable unit" if group.unit_token == "start" \
        else f"following {unit_token(group.unit_token)}"
    lines = [f"/* {kind_desc} {where}; generated by "
             f"tools/overlay_src.py, stubs replaced by C as functions are matched. */",
             '#include "common.h"',
             '#include "include_asm.h"',
             ""]
    lines += [f'INCLUDE_ASM("asm/overlays", {n});' for n in names]
    return "\n".join(lines) + "\n"


def write_groups(groups):
    written_files = 0
    written_funcs = 0
    written_bytes = 0
    skipped = []
    for g in groups:
        if not g.rows:
            continue
        path = file_path(g)
        content = render(g)
        if path.exists():
            existing = path.read_text()
            if existing == content:
                written_files += 1
                written_funcs += len(g.rows)
                written_bytes += g.size
                continue
            if not HEADER_RE.match(existing):
                skipped.append(str(path.relative_to(ROOT)))
                continue
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)
        written_files += 1
        written_funcs += len(g.rows)
        written_bytes += g.size
    return written_files, written_funcs, written_bytes, skipped


# -------------------------------------------------------------------- main

def main():
    rows = parse_catalogue()
    by_name = {r["name"]: r for r in rows}
    shared_level_rows = [r for r in rows if r["kind"] in ("shared", "level")]
    order_check = [canonical(r) for r in shared_level_rows]
    if order_check != sorted(order_check):
        sys.exit(f"{CATALOGUE.relative_to(ROOT)}: shared/level rows are not in canonical "
                  "(level, address) order")

    level_addrs, level_owner = build_level_tiling(rows)
    anchors = build_anchors(rows)
    levels = Levels()
    forces, exceptions = compute_forces_next(rows, by_name, level_addrs, level_owner, levels)

    shared_stream = [r for r in shared_level_rows if r["kind"] == "shared"]
    level_streams = defaultdict(list)
    for r in shared_level_rows:
        if r["kind"] == "level":
            lv, _ = canonical(r)
            level_streams[lv].append(r)

    cross_pairs = []
    same_kind_forces = {}
    for f_name, n_name in forces.items():
        if by_name[f_name]["kind"] == by_name[n_name]["kind"]:
            same_kind_forces[f_name] = n_name
        else:
            cross_pairs.append((f_name, n_name))

    shared_groups, kept_shared, over_shared = group_stream(
        shared_stream, anchors, same_kind_forces, False,
        lambda level: OUT / "shared")

    level_groups = {}
    kept_level = over_level = 0
    for lv, stream in level_streams.items():
        groups, kept, over = group_stream(
            stream, anchors, same_kind_forces, True,
            lambda level, lv=lv: OUT / level_dirname(lv))
        level_groups[lv] = groups
        kept_level += kept
        over_level += over

    moved = splice_cross_kind_pairs(shared_groups, level_groups, forces, by_name, cross_pairs)

    all_groups = shared_groups + [g for gs in level_groups.values() for g in gs]
    files, funcs, total_bytes, skipped = write_groups(all_groups)

    # ----------------------------------------------------------- verify

    seen = defaultdict(int)
    for g in all_groups:
        for r in g.rows:
            seen[r["name"]] += 1
    missing = [r["name"] for r in shared_level_rows if seen.get(r["name"], 0) != 1]
    extra_exe = [n for n in seen if by_name[n]["kind"] == "exe"]

    # ----------------------------------------------------------- summary

    per_dir = defaultdict(lambda: [0, 0])   # directory -> [files, functions]
    sizes = []
    for g in all_groups:
        if not g.rows:
            continue
        d = str(g.directory.relative_to(ROOT))
        per_dir[d][0] += 1
        per_dir[d][1] += len(g.rows)
        sizes.append(g.size)

    print(f"{len(shared_level_rows)} shared/level functions, {len(all_groups)} files:")
    for d in sorted(per_dir):
        nf, nfn = per_dir[d]
        print(f"  {d}: {nf} files, {nfn} functions")
    print(f"total code: {total_bytes} bytes across {files} written files "
          f"({len(skipped)} skipped as hand-edited)")
    if sizes:
        sizes.sort()
        n = len(sizes)
        print(f"file size distribution (bytes): min {sizes[0]}, median {sizes[n // 2]}, "
              f"max {sizes[-1]}, mean {sum(sizes) / n:.0f}")
    print(f"kept-together pairs: {kept_shared} in shared/ ({over_shared} needed to override "
          f"the ordinary split rules), {kept_level} in lNN_<planet>/ ({over_level} overridden), "
          f"{moved} spliced across the shared/level boundary")
    print(f"branch-into-next cases that could not be kept together: "
          f"{len(exceptions['exe_next'])} into an exe-kind function, "
          f"{len(exceptions['noncanonical_next'])} into a non-canonical repeat of a shared function")
    if skipped:
        print("skipped (not purely generated, left untouched):")
        for s in skipped:
            print(f"  {s}")
    if missing:
        print(f"VERIFY FAILED: {len(missing)} shared/level function(s) not in exactly one file: "
              + ", ".join(missing[:20]) + (", ..." if len(missing) > 20 else ""), file=sys.stderr)
    if extra_exe:
        print(f"VERIFY FAILED: {len(extra_exe)} exe-kind function(s) present in generated files: "
              + ", ".join(extra_exe[:20]), file=sys.stderr)
    if not missing and not extra_exe:
        print(f"verify OK: all {len(shared_level_rows)} shared/level functions appear in exactly "
              "one file, no exe-kind function does")
    if missing or extra_exe:
        sys.exit(1)


if __name__ == "__main__":
    main()

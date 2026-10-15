#!/usr/bin/env python3
"""
How much of each level's code overlay is the executable's own code, engine
code shared between levels, or code of a single level (docs/ASSETS.md,
"Code overlays").

  python3 tools/overlay_scan.py      # needs baserom/SCES_509.16{,.iso}

Every instruction is compared with its link-dependent fields masked (jump
targets, lui values, $gp offsets, non-stack memory offsets). Functions are
split at call targets, returns, tail calls followed by a frame opener, and
the executable's functions found in the overlay. Results go to
build-sn/overlays/scan.json.
"""
import json, struct, sys
from collections import defaultdict
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT.parents[2] / "editor"))  # the OpenRAC editor's readers
from disc import Disc
from level import decoded
from formats import overlay_sections, unpack, span

MEM = {0x1A, 0x1B, 0x1E, 0x1F, *range(0x20, 0x30), 0x31, 0x36, 0x37, 0x39, 0x3E, 0x3F}
def mask(w):
    op, rs = w >> 26, (w >> 21) & 31
    if op in (2, 3):
        return w & 0xFC000000
    if op == 0x0F or rs == 28 and op >= 8:
        return w & 0xFFFF0000
    if (op in MEM or op in (0x09, 0x0D, 0x19)) and rs != 29:
        return w & 0xFFFF0000
    return w
def words(b):
    return struct.unpack(f"<{len(b) // 4}I", b)
def masked(b):
    return struct.pack(f"<{len(b) // 4}I", *map(mask, words(b)))
def trim(b):
    while b.endswith(b"\0\0\0\0"):
        b = b[:-4]
    return b

elf = (ROOT / "baserom/SCES_509.16").read_bytes()
DELTA = 0xFF080
report = json.loads((ROOT / "progress/report.json").read_text())
exe = []
for u in report["units"]:
    if not u["name"].startswith("game/"):
        continue
    for f in u["functions"]:
        va, size = int(f["name"][5:], 16), int(f["size"])
        exe.append((f["name"], size, (f.get("fuzzy_match_percent") or 0) == 100,
                    masked(elf[va - DELTA: va - DELTA + size])))

def split(text, base, extra=()):
    w = words(text)
    starts = {0}
    for i, x in enumerate(w):
        if x >> 26 == 3:
            t = ((x & 0x3FFFFFF) << 2) | (base & 0xF0000000)
            if base <= t < base + len(text):
                starts.add((t - base) // 4)
        if (x == 0x03E00008 or x >> 26 == 2) and i + 2 < len(w):   # jr $ra, or a tail call j
            j = i + 2
            while j < len(w) and w[j] == 0:
                j += 1
            if x == 0x03E00008 or (j < len(w) and w[j] >> 16 == 0x27BD and w[j] & 0x8000):
                starts.add(j)
        if x >> 16 == 0x27BD and x & 0x8000 and i and w[i - 1] == 0:   # frame opener after padding
            starts.add(i)
    for a in extra:
        starts.add(a)
    s = sorted(x for x in starts if x < len(w))
    return [(a * 4, (b - a) * 4) for a, b in zip(s, s[1:] + [len(w)])]

levels = {}
with Disc(ROOT / "baserom/SCES_509.16.iso") as d:
    for info in d.survey()["levels"]:
        r = info["ranges"]
        data = d.sectors(r["data"]["lba"], r["data"]["sectors"])
        off, size = unpack("<ii", data, 0)
        raw = decoded(span(data, off, size))
        rec = max(overlay_sections(raw)["sections"], key=lambda x: x["bytes"])
        levels[info["id"]] = (rec["address"], raw[rec["offset"]: rec["offset"] + rec["bytes"]])

out = {"levels": {}}
total_exe = sum(s for _, s, _, _ in exe)
seen = defaultdict(set)        # fingerprint -> levels
fsize = {}
for lid, (base, text) in levels.items():
    m = masked(text)
    found, extra = [], set()
    for e in exe:
        i = m.find(e[3])
        while i >= 0 and i % 4:
            i = m.find(e[3], i + 1)
        if i >= 0:
            found.append(e)
            extra |= {i // 4, (i + e[1]) // 4}
    for off, size in split(text, base, extra):
        fp = trim(m[off:off + size])
        if fp:
            seen[fp].add(lid)
            fsize[fp] = len(fp)
    out["levels"][lid] = {"text": len(text), "exe_funcs_found": len(found),
                          "exe_bytes_found": sum(e[1] for e in found)}
exe_fps = {trim(e[3]) for e in exe}
shared = sum(fsize[f] for f, l in seen.items() if len(l) > 1 and f not in exe_fps)
unique = sum(fsize[f] for f, l in seen.items() if len(l) == 1 and f not in exe_fps)
in_exe = sum(fsize[f] for f in seen if f in exe_fps)
all_levels = sum(fsize[f] for f, l in seen.items() if len(l) == len(levels) and f not in exe_fps)
out["summary"] = {"exe_game_text": total_exe, "overlay_total": sum(len(t) for _, t in levels.values()),
                  "distinct_overlay_code_in_exe": in_exe, "distinct_shared_not_in_exe": shared,
                  "shared_by_all_levels_not_in_exe": all_levels,
                  "distinct_single_level": unique, "distinct_functions": len(seen)}
(ROOT / "build-sn/overlays").mkdir(parents=True, exist_ok=True)
(ROOT / "build-sn/overlays/scan.json").write_text(json.dumps(out, indent=1))
for lid, v in out["levels"].items():
    print(f"level {lid:2}: text {v['text']:8}  exe game funcs found {v['exe_funcs_found']:4} ({v['exe_bytes_found']} bytes)")
print(json.dumps(out["summary"], indent=1))
print("exe game functions:", len(exe), "bytes", total_exe)

#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Where one version of a game keeps what another version keeps: every function
and every data address of FROM with its address in TO, for carrying a port's
address lists (hostgen.json, libraries.tsv, host C) from the version that runs
to one that does not yet.

  python3 port/tools/versionmap/versionmap.py build rac1/pal rac1/ntsc --out FILE
  python3 port/tools/versionmap/versionmap.py lookup FILE ADDRESS ...

It reads the code your own discs give after each game's setup, the same files
tools/xmap.py scans (games/<game>/game.json, "xmap"). OPENRAC_CODE_ROOT names
the checkout that holds them when this one does not (a worktree).

Functions. Each program (the executable, each level) is cut into functions
the same way in both versions (tools/mips.py) and the two lists are aligned on
their strict fingerprints, which only mask address fields. A function with
the same fingerprint at the aligned place is `same`. Between two aligned runs,
when both versions have the same number of functions, they are paired in
order as `near`: the function changed between the versions and its C must be
read again before it is trusted.

Data. Two `same` functions form the same addresses in the same order
(mips.references), so the n-th address one forms is the other's n-th: that
gives the data map with no symbol table. An address no function forms by
itself (a field inside a record, reached with an offset) is `between` when its
neighbours on both sides moved by the same amount, and `guess` (the lower
neighbour's move) when they did not.

What is written holds addresses and kinds, never code.
"""
import argparse
import bisect
import difflib
import json
import os
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(1, str(next(p for p in Path(__file__).resolve().parents if (p / "tools/openrac_common").is_dir()) / "tools"))
from openrac_common import games, paths  # noqa: E402

import mips  # noqa: E402  tools/mips.py, on the path through the line above

ROOT = paths.repo_root(__file__)

CODE_ROOT = Path(os.environ.get("OPENRAC_CODE_ROOT", ROOT))
CODE_SECTIONS = ("core.text", ".text")


def version(key: str) -> dict:
    game, name = key.split("/")
    return games.load(game, ROOT)["versions"][name]


def programs(key: str) -> dict[str, dict[str, tuple[int, bytes]]]:
    """program -> section -> (address, bytes): the executable's code and data, each level's."""
    out = {}
    for entry in version(key)["xmap"]["code"]:
        found = sorted(CODE_ROOT.glob(entry["glob"])) if "glob" in entry else [CODE_ROOT / entry["path"]]
        for p in found:
            if not p.exists():
                continue
            if entry["format"] == "elf":
                out["boot"] = {n: (a, d) for n, (a, d, _f) in mips.elf_sections(p).items()}
            elif entry["format"] == "rac1-level":
                records = json.loads((p / "manifest.json").read_text())["records"]
                level = int(p.name.split("_")[-1])
                out[f"level:{level:02d}"] = {
                    ("." + r["name"]): (r["address"], (p / f"{r['name']}.bin").read_bytes())
                    for r in records if (p / f"{r['name']}.bin").exists()}
    return out


def gp_of(sections: dict) -> int | None:
    """$gp as the start-up code sets it: the first `lui gp` and the `addiu gp, gp` that completes it."""
    for name in CODE_SECTIONS:
        if name not in sections:
            continue
        ws = mips.words(sections[name][1])
        for i, w in enumerate(ws):
            if w >> 16 == 0x3C1C:
                for x in ws[i + 1:i + 8]:
                    if x >> 16 in (0x279C, 0x379C):
                        lo = x & 0xFFFF
                        if x >> 16 == 0x279C and lo & 0x8000:
                            lo -= 0x10000
                        return ((w & 0xFFFF) << 16) + lo
    return None


def functions(sections: dict) -> list[tuple[int, int, str, bytes]]:
    """(address, size, fingerprint, code) of every function of a program, in address order."""
    out = []
    for name in CODE_SECTIONS + (".text",):
        if name not in sections or any(name == n for n in ()):
            continue
        base, data = sections[name]
        for off, span in mips.split(data, base):
            size = mips.code_size(data[off:off + span])
            if size:
                code = data[off:off + size]
                out.append((base + off, size, mips.fingerprint(code), code))
    seen, uniq = set(), []
    for f in sorted(out):
        if f[0] not in seen:
            seen.add(f[0])
            uniq.append(f)
    return uniq


def align(a: list, b: list) -> list[tuple[int, int, str]]:
    """(index in A, index in B, kind) of the functions paired between two programs."""
    sm = difflib.SequenceMatcher(None, [f[2] for f in a], [f[2] for f in b], autojunk=False)
    out = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            out += [(i1 + k, j1 + k, "same") for k in range(i2 - i1)]
        elif tag == "replace" and i2 - i1 == j2 - j1:
            out += [(i1 + k, j1 + k, "near") for k in range(i2 - i1)]
    return out


def locate(src: dict, dst: dict, address: int, count: int = 12) -> int | None:
    """Where TO has the COUNT instructions FROM has at ADDRESS (addresses and constants masked),
    when that is one place: for an entry inside a function, which no cut pairs."""
    want = None
    for name, (base, data) in src.items():
        if name.endswith("text") and base <= address < base + len(data) - 4 * count:
            want = [mips.mask(w) for w in mips.words(data[address - base:address - base + 4 * count])]
    if want is None:
        return None
    hits = []
    for name, (base, data) in dst.items():
        if name.endswith("text"):
            ws = [mips.mask(w) for w in mips.words(data)]
            hits += [base + 4 * i for i in range(len(ws) - count) if ws[i] == want[0] and ws[i:i + count] == want]
    return hits[0] if len(hits) == 1 else None


def build(src_key: str, dst_key: str, entries: dict[str, list[int]] | None = None) -> dict:
    src, dst = programs(src_key), programs(dst_key)
    if "boot" not in src or "boot" not in dst:
        raise SystemExit("the executables are not there: run each game's setup, or set OPENRAC_CODE_ROOT")
    gps = gp_of(src["boot"]), gp_of(dst["boot"])
    out = {"from": src_key, "to": dst_key, "gp": list(gps), "programs": {}}
    for prog in sorted(src):
        if prog not in dst:
            continue
        a, b = functions(src[prog]), functions(dst[prog])
        pairs = align(a, b)
        funcs = {}
        data: dict[int, Counter] = defaultdict(Counter)
        calls: dict[int, Counter] = defaultdict(Counter)
        for i, j, kind in pairs:
            funcs[a[i][0]] = (b[j][0], kind, a[i][1], b[j][1])
            if kind != "same":
                continue
            ra = mips.references(a[i][3], a[i][0], gps[0])
            rb = mips.references(b[j][3], b[j][0], gps[1])
            if len(ra) != len(rb):
                continue
            for (ia, ka, xa), (ib, kb, xb) in zip(ra, rb):
                if ia != ib or ka != kb:
                    break
                (calls if ka == "call" else data)[xa][xb] += 1
        # A function only ever reached by a call both versions make is a pair too (code the cut missed).
        for xa, votes in calls.items():
            xb, n = votes.most_common(1)[0]
            if xa not in funcs and n == sum(votes.values()):
                funcs[xa] = (xb, "called", 0, 0)
        for xa in (entries or {}).get(prog, []):
            if xa not in funcs:
                xb = locate(src[prog], dst[prog], xa)
                if xb is not None:
                    funcs[xa] = (xb, "found", 0, 0)
        sure = {}
        for xa, votes in data.items():
            xb, n = votes.most_common(1)[0]
            if n == sum(votes.values()):
                sure[xa] = xb
        out["programs"][prog] = {
            "sections": {n: [s[0], len(s[1])] for n, s in src[prog].items()},
            "to_sections": {n: [s[0], len(s[1])] for n, s in dst[prog].items()},
            "functions": {f"{k:08X}": [f"{v[0]:08X}", v[1], v[2], v[3]] for k, v in sorted(funcs.items())},
            "data": {f"{k:08X}": f"{v:08X}" for k, v in sorted(sure.items())},
            "unpaired": [f"{f[0]:08X}" for n, f in enumerate(a) if f[0] not in funcs],
        }
    return out


class Map:
    """A built map, answering where FROM's addresses are in TO."""

    def __init__(self, path: Path):
        self.raw = json.loads(Path(path).read_text())
        self.programs = self.raw["programs"]
        self._data = {}
        for prog, p in self.programs.items():
            keys = sorted(int(k, 16) for k in p["data"])
            self._data[prog] = (keys, [int(p["data"][f"{k:08X}"], 16) for k in keys])

    def function(self, address: int, program: str = "boot") -> tuple[int, str] | None:
        hit = self.programs.get(program, {}).get("functions", {}).get(f"{address:08X}")
        return (int(hit[0], 16), hit[1]) if hit else None

    def data(self, address: int, program: str = "boot", reach: int = 0x4000) -> tuple[int, str] | None:
        keys, values = self._data.get(program, ([], []))
        at = bisect.bisect_right(keys, address)
        if at and keys[at - 1] == address:
            return values[at - 1], "exact"
        below = (keys[at - 1], values[at - 1]) if at else None
        above = (keys[at], values[at]) if at < len(keys) else None
        if below and above and below[1] - below[0] == above[1] - above[0]:
            return address + below[1] - below[0], "between"
        if below and address - below[0] <= reach:
            return address + below[1] - below[0], "guess"
        return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build")
    b.add_argument("src")
    b.add_argument("dst")
    b.add_argument("--out", type=Path, required=True)
    b.add_argument("--entries", type=Path, action="append", default=[],
                   help="a file of the port (C, TSV, JSON): every func_ address and level place (NN:ADDRESS) it "
                        "names is looked for by its instructions when the cut does not pair it")
    look = sub.add_parser("lookup")
    look.add_argument("map", type=Path)
    look.add_argument("addresses", nargs="+")
    look.add_argument("--program", default="boot")
    args = ap.parse_args()
    if args.cmd == "build":
        entries: dict[str, list[int]] = {}
        for path in args.entries:
            text = path.read_text()
            for level, address in re.findall(r"\bfunc_(?:L(\d\d)_)?([0-9A-F]{8})\b", text):
                entries.setdefault(f"level:{level}" if level else "boot", []).append(int(address, 16))
            for level, address in re.findall(r"\b(\d\d):([0-9A-F]{8})\b", text):      # a table's level places
                entries.setdefault(f"level:{level}", []).append(int(address, 16))
        m = build(args.src, args.dst, entries)
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(m, indent=0, sort_keys=True) + "\n")
        for prog, p in m["programs"].items():
            kinds = Counter(v[1] for v in p["functions"].values())
            print(f"{prog}: {kinds['same']} same, {kinds['near']} near, {kinds['called']} by call, {kinds['found']} found, "
                  f"{len(p['unpaired'])} unpaired; {len(p['data'])} data addresses")
        return 0
    m = Map(args.map)
    for text in args.addresses:
        x = int(text, 16)
        print(text, "function", m.function(x, args.program), "data", m.data(x, args.program))
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
The function map across the games: which code is the same in which programs,
and which functions one project has matched that another has not.

  python3 tools/xmap.py scan [GAME/VERSION ...]  index every function of a version's programs
  python3 tools/xmap.py report                   compare the indexed versions: shared/xmap/summary.json
                                                 and shared/xmap/README.md
  python3 tools/xmap.py ports FROM TO            functions FROM has matched and TO has not:
                                                 shared/xmap/ports/FROM--TO.tsv (e.g. rac1/pal rac1/ntsc)

Functions that changed a little between two versions share neither fingerprint.
For the resident programs (boot and frontend), `report` also pairs those: between
two functions that are the same in both versions and in the same order, the
functions in between are paired in order when their instructions are alike
(difflib's ratio over shape-masked instructions, at least 0.80). `ports` lists
them as `similar` with the ratio.

`scan` reads the code your own discs give after each game's setup (the paths
are in games/<game>/game.json under "xmap"; docs/engine/SHARED_CODE.md says
how to produce them) and writes build/xmap/, which is ignored. It cuts every
code section into functions the same way for every game (tools/mips.py), so
the versions compare like with like, and gives each function two fingerprints:

  strict  equal only for the same function at another address: only address
          fields are masked (mips.fingerprint)
  shape   equal for the same instructions with other constants and struct
          offsets (mips.shape): relatives, not copies

A function that repeats in several levels counts once. `report` and `ports`
need no disc: they read build/xmap/. What they write holds addresses, sizes,
names and hashes of masked code, never code itself.
"""
import json
import re
import sys
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import mips  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
GAMES = ROOT / "games"
INDEX = ROOT / "build/xmap"
OUT = ROOT / "shared/xmap"
CODE_SECTIONS = ("core.text", ".text", "net.text")
CLASSES = ("core", "net", "game", "level")          # where a function lives, most resident first
MIN_STRICT = 16          # bytes: below this (a bare return) a shared fingerprint says nothing
MIN_SHAPE = 48           # bytes: a shape match below this is too easily a coincidence
ALIKE = 0.80             # least similarity for two functions between the same anchors to be paired
RESIDENT = ("boot", "frontbin")
NAME = re.compile(r"^(?:func|FUN)_(?:L(\d+)_)?([0-9A-Fa-f]{8})$")


# --- Manifests ---------------------------------------------------------------

def versions() -> dict[str, dict]:
    """'game/version' -> its game.json entry, for versions with an "xmap" block."""
    out = {}
    for path in sorted(GAMES.glob("*/game.json")):
        game = json.loads(path.read_text())
        for name, v in game["versions"].items():
            if v.get("xmap"):
                out[f"{game['id']}/{name}"] = v
    return out


def slug(key: str) -> str:
    return key.replace("/", "-")


def level_id(path: Path, fmt: str) -> int:
    """A level's number from its directory: level_05 (rac1), 04_barlow (rac2), 16_kerwan (Wrench)."""
    name = path.name if fmt in ("rac1-level", "rac2-level") else path.parent.name
    return int(re.search(r"\d+", name).group())


def program_files(version: dict) -> dict[str, tuple[str, str]]:
    """program -> (path, format). A glob that matches several runs keeps the last of each program."""
    out = {}
    for entry in version["xmap"]["code"]:
        paths = sorted(ROOT.glob(entry["glob"])) if "glob" in entry else [ROOT / entry["path"]]
        for p in paths:
            if not p.exists():
                continue
            prog = f"level:{level_id(p, entry['format']):02d}" if entry["program"] == "level" else entry["program"]
            out[prog] = (str(p), entry["format"])
    return out


def load(path: str, fmt: str) -> dict[str, tuple[int, bytes]]:
    """section -> (address, bytes) of a program's code."""
    p = Path(path)
    if fmt == "elf":
        return {n: (a, d) for n, (a, d, _f) in mips.elf_sections(p).items() if n in CODE_SECTIONS}
    if fmt == "rac1-level":             # games/rac1/*/…/level_NN: text.bin and manifest.json
        text = next(r for r in json.loads((p / "manifest.json").read_text())["records"] if r["name"] == "text")
        return {".text": (text["address"], (p / "text.bin").read_bytes())}
    if fmt == "rac2-level":             # games/rac2/*/baserom/levels/NN_name (tools/levels.py dump)
        text = next(s for s in json.loads((p / "manifest.json").read_text())["sections"] if s["name"] == "text")
        return {".text": (text["address"], (p / "text.bin").read_bytes())}
    raise SystemExit(f"{path}: unknown code format {fmt}")


def klass(prog: str, section: str) -> str:
    if section == "core.text":
        return "core"
    if section == "net.text":
        return "net"
    return "level" if prog.startswith("level:") else "game"


# --- What each project has matched --------------------------------------------

def known(key: str, version: dict) -> dict[str, list[tuple[int, int, str, bool]]]:
    """program -> [(address, size, name, matched)] of the functions the project lists, as it
    cuts them, with whether it counts each as matched C."""
    cfg = version["xmap"].get("matched")
    out: dict[str, list] = {}
    if not cfg:
        return out
    base = GAMES / key

    def add(level, addr, size, name, done=True):
        prog = f"level:{int(level):02d}" if level is not None else cfg.get("program", "boot")
        out.setdefault(prog, []).append((addr, size, name, done))

    if cfg["kind"] == "objdiff":
        skip = set()
        for listing in cfg.get("not_c", []):            # handwritten assembly and remnants: finished, but not C to port
            skip |= set(re.findall(r"func_\w+", (ROOT / listing).read_text()))
        report = json.loads((ROOT / cfg["report"]).read_text())
        for unit in report["units"]:
            cats = (unit.get("metadata") or {}).get("progress_categories", [])
            for f in unit.get("functions", []):
                done = float(f.get("fuzzy_match_percent", 0)) >= 100 and f["name"] not in skip
                m = NAME.match(f["name"])
                if m:
                    add(m.group(1), int(m.group(2), 16), int(f["size"]), f["name"], done)
                elif "virtual_address" in (f.get("metadata") or {}):
                    level = next((c[6:] for c in cats if re.fullmatch(r"level_\d+", c)), None)
                    addr = int(f["metadata"]["virtual_address"])
                    add(level, addr + (cfg.get("boot_offset_delta", 0) if level is None else 0), int(f["size"]), f["name"], done)
    elif cfg["kind"] == "rac3":             # C blocks in src/frontbin; sizes from the project's report
        in_c = set()
        for src in sorted((base / "src/frontbin").glob("*.c")):
            in_c |= set(re.findall(r"localdecomp:start (func_[0-9A-F]{8})", src.read_text(errors="replace")))
        for unit in json.loads((base / "progress_report.json").read_text())["units"]:
            if unit["name"].startswith("frontbin/"):
                for f in unit.get("functions", []):
                    add(None, int(f["name"][5:], 16), int(f["size"]), f["name"], f["name"] in in_c)
    else:
        raise SystemExit(f"{key}: unknown matched kind {cfg['kind']}")
    return out


def aliases(version: dict) -> dict[str, str]:
    """A project's readable names for its address-named functions, when it keeps a table
    (xmap.matched.names: a TSV of symbol and name)."""
    path = (version["xmap"].get("matched") or {}).get("names")
    out = {}
    if path:
        for line in (ROOT / path).read_text().splitlines():
            cells = line.split("\t")
            if len(cells) > 1 and NAME.match(cells[0]) and cells[1] and cells[1] != cells[0]:
                out[cells[0]] = cells[1]
    return out


# --- Scan ------------------------------------------------------------------------

def scan_program(job: tuple) -> tuple:
    """One program: every function as (strict, shape, size, class, section, address), and the
    fingerprints of each function as the project itself cuts it there."""
    prog, path, fmt, wanted = job
    sections = load(path, fmt)
    functions, ordered = [], []
    for name, (base, data) in sections.items():
        for off, span in mips.split(data, base):
            size = mips.code_size(data[off:off + span])
            if size:
                code = data[off:off + size]
                fp = mips.fingerprint(code)
                functions.append((fp, mips.shape(code), size, klass(prog, name), name, base + off))
                if prog in RESIDENT:        # in address order, with shape-masked words, for the similarity pairing
                    ordered.append([name, base + off, size, fp, [mips.mask(w) for w in mips.words(code)]])
    listed = []
    for addr, size, fname, done in wanted:
        for name, (base, data) in sections.items():
            if size and base <= addr and addr + size <= base + len(data):
                code = data[addr - base:addr - base + size]
                listed.append((mips.fingerprint(code), mips.shape(code), fname, prog, addr, mips.code_size(code), done))
                break
    return prog, {n: [a, len(d)] for n, (a, d) in sections.items()}, functions, listed, ordered


def scan(keys: list[str]) -> None:
    INDEX.mkdir(parents=True, exist_ok=True)
    for key, version in versions().items():
        if keys and key not in keys:
            continue
        files = program_files(version)
        if not files:
            print(f"{key}: no code found (run its setup first; docs/engine/SHARED_CODE.md)")
            continue
        want, readable = known(key, version), aliases(version)
        jobs = [(prog, path, fmt, want.get(prog, [])) for prog, (path, fmt) in sorted(files.items())]
        functions, done, listed, programs, resident = {}, {}, {}, {}, {}
        with ProcessPoolExecutor() as pool:
            for prog, sections, found, own, ordered in pool.map(scan_program, jobs):
                if ordered:
                    resident[prog] = ordered
                programs[prog] = sections
                for fp, shape, size, cls, section, addr in found:
                    f = functions.setdefault(fp, {"size": size, "shape": shape, "class": cls, "count": 0, "places": []})
                    f["count"] += 1
                    if CLASSES.index(cls) < CLASSES.index(f["class"]):
                        f["class"] = cls
                    if len(f["places"]) < 3:
                        f["places"].append([prog, addr])
                for fp, shape, fname, prog, addr, size, ok in own:
                    entry = {"name": fname, "shape": shape, "program": prog, "address": addr, "size": size}
                    if fname in readable:
                        entry["alias"] = readable[fname]
                    listed.setdefault(fp, entry)
                    if ok:
                        done.setdefault(fp, entry)
        # "listed" is the version's code as its own project cuts it: used to tell whether a
        # function is present, never for the byte totals, which use the uniform cut above.
        index = {"version": key, "serial": version["serial"], "programs": programs, "functions": functions,
                 "listed": listed, "matched": done}
        (INDEX / f"{slug(key)}.json").write_text(json.dumps(index))
        (INDEX / f"{slug(key)}.resident.json").write_text(json.dumps(resident))     # masked code: local only
        code = sum(f["size"] for f in functions.values())
        print(f"{key}: {len(programs)} programs, {len(functions):,} distinct functions, {code:,} bytes; the project lists "
              f"{len(listed):,} distinct functions and has matched {len(done):,} of them ({sum(m['size'] for m in done.values()):,} bytes)")


# --- Report ----------------------------------------------------------------------

def indexes() -> dict[str, dict]:
    out = {}
    for key in versions():
        path = INDEX / f"{slug(key)}.json"
        if path.exists():
            out[key] = json.loads(path.read_text())
    if not out:
        sys.exit("nothing indexed: run `python3 tools/xmap.py scan` first")
    return out


def by_class(functions: dict, keep) -> dict:
    out = {c: [0, 0] for c in CLASSES}
    for fp, f in functions.items():
        if keep(fp, f):
            out[f["class"]][0] += 1
            out[f["class"]][1] += f["size"]
    return out


def total(counts: dict) -> list[int]:
    return [sum(v[0] for v in counts.values()), sum(v[1] for v in counts.values())]


def port_list(src: dict, dst: dict) -> list[dict]:
    """Functions SRC's project has matched that DST's code also has, unmatched there:
    the same function (strict), or a relative with other constants (shape)."""
    there = present(dst)
    dst_shapes = {}
    for fp, f in there.items():
        dst_shapes.setdefault(f["shape"], []).append(fp)
    dst_done_shapes = {m["shape"] for m in dst["matched"].values()}
    out = []
    for fp, m in src["matched"].items():
        if m["size"] < MIN_STRICT:
            continue
        if fp in there:
            if fp not in dst["matched"]:
                out.append({"kind": "same", "from": m, "to_fp": fp, "to": there[fp]})
        elif m["size"] >= MIN_SHAPE and m["shape"] in dst_shapes and m["shape"] not in dst_done_shapes:
            to_fp = dst_shapes[m["shape"]][0]
            if to_fp not in dst["matched"]:
                out.append({"kind": "shape", "from": m, "to_fp": to_fp, "to": there[to_fp]})
    return out


def increasing(pairs: list[tuple[int, int]]) -> list[tuple[int, int]]:
    """The longest run of (i, j) pairs, sorted by i, whose j also rises: anchors in the same order."""
    best, back, tails = [], [], []
    import bisect
    for n, (_i, j) in enumerate(pairs):
        at = bisect.bisect_left([pairs[t][1] for t in tails], j)
        back.append(tails[at - 1] if at else -1)
        if at == len(tails):
            tails.append(n)
        else:
            tails[at] = n
    n = tails[-1] if tails else -1
    while n >= 0:
        best.append(pairs[n])
        n = back[n]
    return best[::-1]


def alike(a: list, b: list) -> list[tuple[int, int, float]]:
    """Pair two runs of functions in order: (index in A, index in B, ratio) for the pairing with
    the highest total similarity among pairs of at least ALIKE."""
    import difflib
    if not a or not b or len(a) * len(b) > 400:
        return []
    ratio = [[0.0] * len(b) for _ in a]
    for i, x in enumerate(a):
        for j, y in enumerate(b):
            if 0.6 < x[2] / y[2] < 1.67:
                r = difflib.SequenceMatcher(None, x[4], y[4], autojunk=False).ratio()
                ratio[i][j] = r if r >= ALIKE else 0.0
    score = [[0.0] * (len(b) + 1) for _ in range(len(a) + 1)]
    for i in range(len(a) - 1, -1, -1):
        for j in range(len(b) - 1, -1, -1):
            score[i][j] = max(score[i + 1][j], score[i][j + 1], score[i + 1][j + 1] + ratio[i][j])
    out, i, j = [], 0, 0
    while i < len(a) and j < len(b):
        if ratio[i][j] and score[i][j] == score[i + 1][j + 1] + ratio[i][j]:
            out.append((i, j, ratio[i][j]))
            i, j = i + 1, j + 1
        elif score[i][j] == score[i + 1][j]:
            i += 1
        else:
            j += 1
    return out


def similar(src_key: str, dst_key: str) -> list[dict]:
    """Functions of SRC's resident programs that changed a little in DST: each sits between the
    same two unchanged functions in both, and is at least ALIKE similar."""
    files = [INDEX / f"{slug(k)}.resident.json" for k in (src_key, dst_key)]
    if not all(f.exists() for f in files):
        return []
    src, dst = (json.loads(f.read_text()) for f in files)
    out = []
    for prog in src:
        target = prog if prog in dst else "boot"
        for section in {f[0] for f in src[prog]}:
            a = [f for f in src[prog] if f[0] == section]
            b = [f for f in dst.get(target, []) if f[0] == section]
            if not a or not b:
                continue
            where = {}
            for j, f in enumerate(b):
                where.setdefault(f[3], []).append(j)
            unique = [(i, where[f[3]][0]) for i, f in enumerate(a)
                      if f[2] >= MIN_STRICT and len(where.get(f[3], [])) == 1]
            anchors = [(-1, -1)] + increasing(unique) + [(len(a), len(b))]
            for (i0, j0), (i1, j1) in zip(anchors, anchors[1:]):
                run_a, run_b = a[i0 + 1:i1], b[j0 + 1:j1]
                for i, j, r in alike(run_a, run_b):
                    x, y = run_a[i], run_b[j]
                    if x[3] != y[3] and x[2] >= MIN_STRICT:
                        out.append({"program": prog, "section": section, "address": x[1], "size": x[2], "fp": x[3],
                                    "to_program": target, "to_address": y[1], "to_size": y[2], "to_fp": y[3],
                                    "ratio": round(r, 3)})
    return out


def present(index: dict) -> dict:
    """fingerprint -> {shape, places, count} for every function of a version, cut uniformly
    or as its own project cuts it."""
    out = {fp: {"shape": m["shape"], "places": [[m["program"], m["address"]]], "count": 1}
           for fp, m in index["listed"].items()}
    out.update(index["functions"])
    return out


def report() -> None:
    idx = indexes()
    keys = list(idx)
    summary = {"min_strict_bytes": MIN_STRICT, "min_shape_bytes": MIN_SHAPE, "versions": {}, "pairs": {}}
    for key, index in idx.items():
        fs = index["functions"]
        summary["versions"][key] = {
            "serial": index["serial"], "programs": len(index["programs"]),
            "functions": by_class(fs, lambda fp, f: True),
            "matched": [len(index["matched"]), sum(m["size"] for m in index["matched"].values())],
        }
    shapes = {key: {f["shape"] for f in present(index).values()} for key, index in idx.items()}
    for a in keys:
        for b in keys:
            if a == b:
                continue
            fa, fb = idx[a]["functions"], present(idx[b])
            ports = port_list(idx[a], idx[b])
            near = similar(a, b)
            near_ports = [n for n in near if n["fp"] in idx[a]["matched"] and n["to_fp"] not in idx[b]["matched"]]
            summary["pairs"][f"{a} {b}"] = {
                "same": by_class(fa, lambda fp, f: f["size"] >= MIN_STRICT and fp in fb),
                "shape": by_class(fa, lambda fp, f: f["size"] >= MIN_SHAPE and fp not in fb and f["shape"] in shapes[b]),
                "ports_same": [sum(p["kind"] == "same" for p in ports), sum(p["from"]["size"] for p in ports if p["kind"] == "same")],
                "ports_shape": [sum(p["kind"] == "shape" for p in ports), sum(p["from"]["size"] for p in ports if p["kind"] == "shape")],
                "similar": [len(near), sum(n["size"] for n in near)],
                "ports_similar": [len(near_ports), sum(n["size"] for n in near_ports)],
            }
    # What each version could take from all the others together: functions matched in some
    # other project, present here and not matched here, each counted once.
    union = {}
    for key, index in idx.items():
        for fp, m in index["matched"].items():
            if m["size"] >= MIN_STRICT:
                union.setdefault(fp, []).append(key)
    for key, index in idx.items():
        there, open_here = present(index), {}
        for fp, where in union.items():
            if fp in there and fp not in index["matched"]:
                open_here[fp] = next(idx[w]["matched"][fp]["size"] for w in where)
        summary["versions"][key]["open_matched_elsewhere"] = [len(open_here), sum(open_here.values())]
    summary["matched_somewhere"] = [len(union), sum(idx[w[0]]["matched"][fp]["size"] for fp, w in union.items())]
    # Code every version shares.
    everywhere = set.intersection(*(set(present(i)) for i in idx.values()))
    first = idx[keys[0]]["functions"]
    summary["in_every_version"] = by_class(first, lambda fp, f: fp in everywhere and f["size"] >= MIN_STRICT)
    OUT.mkdir(parents=True, exist_ok=True)
    shared = function_table(idx)
    summary["in_two_or_more_games"] = shared
    (OUT / "summary.json").write_text(json.dumps(summary, indent=1) + "\n")
    (OUT / "README.md").write_text(render(summary, keys))
    print(render(summary, keys))


def function_table(idx: dict) -> list[int]:
    """shared/xmap/functions.tsv: every function found in two or more games, with where it is
    in each version, what that project calls it and whether it has matched it. Returns
    [functions, bytes]."""
    keys = list(idx)
    there = {k: present(idx[k]) for k in keys}
    games = {}
    for k in keys:
        for fp in there[k]:
            games.setdefault(fp, set()).add(k.split("/")[0])
    rows = []
    for fp, where in games.items():
        if len(where) < 2:
            continue
        first = next(k for k in keys if fp in there[k])
        size = idx[first]["functions"].get(fp, idx[first]["listed"].get(fp, {})).get("size", 0)
        if size < MIN_STRICT:
            continue
        cls = next((idx[k]["functions"][fp]["class"] for k in keys if fp in idx[k]["functions"]), "")
        name = next((m.get("alias") or m["name"] for k in keys for m in [idx[k]["listed"].get(fp)]
                     if m and not NAME.match(m.get("alias") or m["name"])), "")
        cells = []
        for k in keys:
            if fp not in there[k]:
                cells += ["", "", ""]
                continue
            prog, addr = there[k][fp]["places"][0]
            listed = idx[k]["listed"].get(fp)
            cells += [f"{prog}:{addr:08X}", listed["name"] if listed else "",
                      "matched" if fp in idx[k]["matched"] else "open"]
        rows.append((CLASSES.index(cls) if cls else 9, -size, [fp, str(size), cls, str(len(where)), name, *cells]))
    lines = ["# Functions found in two or more of the games, by identical code (only addresses differ).",
             "# Generated by tools/xmap.py report; do not edit. docs/engine/SHARED_CODE.md explains it.",
             "# name: a readable name some project has for it. Per version: where it is (program:address,",
             "# its first place when it repeats), what that project calls it, and whether it has matched it.",
             "fingerprint\tsize\tclass\tgames\tname\t" + "\t".join(f"{k}\t{k} name\t{k} state" for k in keys)]
    lines += ["\t".join(r[2]) for r in sorted(rows, key=lambda r: r[:2])]
    (OUT / "functions.tsv").write_text("\n".join(lines) + "\n")
    return [len(rows), sum(-r[1] for r in rows)]


def pct(part: int, whole: int) -> str:
    return f"{100.0 * part / whole:.1f}%" if whole else "–"


def render(s: dict, keys: list[str]) -> str:
    v = s["versions"]
    out = ["# Shared code across the games (generated)\n",
           "Generated by `python3 tools/xmap.py report` from the function indexes of each version;",
           "do not edit. [docs/engine/SHARED_CODE.md](../../docs/engine/SHARED_CODE.md) explains the",
           "method and what the numbers mean.\n",
           "## Each version's code\n",
           "Distinct functions: a function that repeats in several levels counts once.\n",
           "| Version | Programs | Distinct functions | Code bytes | Core | Network | Resident game | Level-only | Matched by its project |",
           "|---|---:|---:|---:|---:|---:|---:|---:|---:|"]
    for k in keys:
        f, m = v[k]["functions"], v[k]["matched"]
        n, b = total(f)
        out.append(f"| `{k}` ({v[k]['serial']}) | {v[k]['programs']} | {n:,} | {b:,} | " +
                   " | ".join(f"{f[c][1]:,}" for c in CLASSES) + f" | {m[0]:,} functions, {m[1]:,} ({pct(m[1], b)}) |")
    out += ["", "## The same function in two versions\n",
            f"Bytes of the row version's distinct functions (of {s['min_strict_bytes']} bytes or more) that the column",
            "version also contains: the same instructions with only addresses differing. In brackets,",
            f"more functions (of {s['min_shape_bytes']} bytes or more) with the same instructions but other constants or",
            "struct offsets.\n",
            "| | " + " | ".join(f"`{k}`" for k in keys) + " |", "|---|" + "---:|" * len(keys)]
    for a in keys:
        whole = total(v[a]["functions"])[1]
        cells = []
        for b in keys:
            if a == b:
                cells.append("–")
                continue
            p = s["pairs"][f"{a} {b}"]
            same, shape = total(p["same"])[1], total(p["shape"])[1]
            cells.append(f"{pct(same, whole)} (+{pct(shape, whole)})")
        out.append(f"| `{a}` | " + " | ".join(cells) + " |")
    out += ["", "### By where the code lives\n",
            "The same comparison as bytes, split by the row version's code class.\n",
            "| Row version | Column version | Core | Network | Resident game | Level-only | All |", "|---|---|---:|---:|---:|---:|---:|"]
    for a in keys:
        for b in keys:
            if a != b:
                p, f = s["pairs"][f"{a} {b}"]["same"], v[a]["functions"]
                out.append(f"| `{a}` | `{b}` | " + " | ".join(
                    f"{p[c][1]:,} ({pct(p[c][1], f[c][1])})" for c in CLASSES) + f" | {total(p)[1]:,} |")
    out += ["", "### Changed a little\n",
            f"Functions of the row version's resident programs (boot executable, frontend) that sit between the",
            f"same two unchanged functions in the column version and are at least {int(ALIKE * 100)}% alike there: count and",
            "bytes, on top of the tables above. Level code is not paired this way.\n",
            "| | " + " | ".join(f"`{k}`" for k in keys) + " |", "|---|" + "---:|" * len(keys)]
    for a in keys:
        out.append(f"| `{a}` | " + " | ".join(
            "–" if a == b else f"{s['pairs'][f'{a} {b}']['similar'][0]:,} / {s['pairs'][f'{a} {b}']['similar'][1]:,} B" for b in keys) + " |")
    e = s["in_every_version"]
    out += ["", f"Code present in every version above: {total(e)[0]:,} functions, {total(e)[1]:,} bytes "
            f"(core {e['core'][1]:,}, resident game {e['game'][1]:,}, level-only {e['level'][1]:,}).\n",
            f"[functions.tsv](functions.tsv) lists the {s['in_two_or_more_games'][0]:,} functions "
            f"({s['in_two_or_more_games'][1]:,} bytes) found in two or more games: where each is in every version,",
            "what each project calls it and whether it has matched it.\n",
            "## Port candidates\n",
            f"Across all projects, {s['matched_somewhere'][0]:,} distinct functions ({s['matched_somewhere'][1]:,} bytes) are matched in",
            "at least one. What each version could take from the others together, each function counted once:\n",
            "| Version | Functions matched elsewhere, present and open here | Bytes | Against what its project has matched |",
            "|---|---:|---:|---:|",
            *[f"| `{k}` | {v[k]['open_matched_elsewhere'][0]:,} | {v[k]['open_matched_elsewhere'][1]:,} | "
              f"{pct(v[k]['open_matched_elsewhere'][1], v[k]['matched'][1])} of {v[k]['matched'][1]:,} |" for k in keys],
            "",
            "Functions the row version's project has matched in C that the column version's code also",
            "contains and its project has not matched: count and bytes of the same function, and in",
            "brackets of relatives: other constants, or changed a little. `python3 tools/xmap.py ports FROM TO`",
            "lists them.\n",
            "| Matched in | " + " | ".join(f"Open in `{k}`" for k in keys) + " |", "|---|" + "---:|" * len(keys)]
    for a in keys:
        cells = []
        for b in keys:
            if a == b:
                cells.append("–")
                continue
            p = s["pairs"][f"{a} {b}"]
            more = [p["ports_shape"][0] + p["ports_similar"][0], p["ports_shape"][1] + p["ports_similar"][1]]
            cells.append(f"{p['ports_same'][0]:,} / {p['ports_same'][1]:,} B (+{more[0]:,} / {more[1]:,} B)")
        out.append(f"| `{a}` | " + " | ".join(cells) + " |")
    return "\n".join(out) + "\n"


def ports(src_key: str, dst_key: str) -> None:
    idx = indexes()
    for key in (src_key, dst_key):
        if key not in idx:
            sys.exit(f"{key} is not indexed; indexed: {', '.join(idx)}")
    rows = sorted(port_list(idx[src_key], idx[dst_key]), key=lambda p: (p["kind"] != "same", -p["from"]["size"]))
    seen = {p["from"]["name"] for p in rows}
    for n in similar(src_key, dst_key):
        m = idx[src_key]["matched"].get(n["fp"])
        if m and m["name"] not in seen and n["to_fp"] not in idx[dst_key]["matched"]:
            rows.append({"kind": "similar", "from": m, "ratio": n["ratio"],
                         "to": {"places": [[n["to_program"], n["to_address"]]], "count": 1}})
    path = OUT / "ports" / f"{slug(src_key)}--{slug(dst_key)}.tsv"
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [f"# Functions {src_key} has matched in C that {dst_key} also contains and has not matched.",
             "# Generated by tools/xmap.py ports; do not edit. kind: same = the same function at another",
             "# address; shape = the same instructions with other constants or struct offsets (check each);",
             "# similar = between the same unchanged neighbours and alike by the ratio given (a draft to adapt).",
             "# Addresses are hexadecimal; `places` counts where the function occurs in the target version.",
             "kind\tsize\tfrom_name\tfrom_program\tfrom_address\tto_program\tto_address\tplaces\tratio"]
    for p in rows:
        m, t = p["from"], p["to"]
        lines.append(f"{p['kind']}\t{m['size']}\t{m['name']}\t{m['program']}\t{m['address']:08X}\t"
                     f"{t['places'][0][0]}\t{t['places'][0][1]:08X}\t{t['count']}\t{p.get('ratio', '')}")
    path.write_text("\n".join(lines) + "\n")
    same = [p for p in rows if p["kind"] == "same"]
    print(f"{path.relative_to(ROOT)}: {len(same):,} the same ({sum(p['from']['size'] for p in same):,} bytes), "
          f"{len(rows) - len(same):,} relatives ({sum(p['from']['size'] for p in rows if p['kind'] != 'same'):,} bytes)")


def main() -> None:
    args = sys.argv[1:]
    if args[:1] == ["scan"]:
        scan(args[1:])
    elif args == ["report"]:
        report()
    elif args[:1] == ["ports"] and len(args) == 3:
        ports(args[1], args[2])
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()

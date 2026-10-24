#!/usr/bin/env python3
"""
Every function of another version, and its counterpart in the primary one (docs/VERSIONS.md).

  python3 tools/version_map.py --version us        # config/versions/us/functions.tsv
  python3 tools/version_map.py                     # every version other than the primary

The version's programs are cut and paired the way `tools/overlays.py us-map` does it
(docs/OVERLAYS.md, "US map"), from the dump (tools/versions.py dump) instead of a ReRAC
extraction: each code section (the executable's core.text and .text, each level's text record) is
cut by split() and at the primary version's functions found in it, so its functions are cut as
their counterparts are; then each function is paired with the primary's by identity (identical
instructions with address fields masked), in order between those anchors by similarity, and by
similarity anywhere in the program.

The level functions are then counted the way the primary's catalogue counts them
(config/overlays/functions.tsv, tools/overlays.py catalogue): one entry per distinct function
(its identity), whatever the number of places; one that is an executable function is that
function. Each entry:

  name         its name in this version's report and source: the primary's name of its
               counterpart, or func_<TAG>_XXXXXXXX / func_<TAG>_LNN_XXXXXXXX (its own address)
               for code only this version has
  kind         exe (the executable's), shared (in two or more levels) or level (in one)
  size         bytes
  fingerprint  its identity's hash (overlays.fingerprint)
  counterpart  the primary's function it was paired with, or -
  method       fingerprint, aligned or similar (how), or -
  similarity   1.00 when only constants differ; - without a counterpart
  same         yes when its identity is its counterpart's: the same C builds both
  places       boot:ADDR for the executable, NN:ADDR in level NN

Names, sizes, hashes and addresses only: no bytes.
"""
import argparse
import collections
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import overlays as ov  # noqa: E402
import versions  # noqa: E402

EXE_TEXT = ("core.text", ".text")


def primary_functions():
    """(exe: [(addr, size, name, bytes)], levels: {lid: [(addr, size, name, bytes)]},
    fps: {fingerprint: name} of the primary's distinct functions)."""
    exe = sorted((addr, size, name, b) for name, addr, size, b in ov.exe_functions(everything=True))
    texts = {lid: versions.level_text(versions.PRIMARY, lid) for lid in range(versions.LEVEL_COUNT)}
    in_level = collections.defaultdict(list)
    fps = {}
    for line in ov.CATALOGUE.read_text().splitlines():
        if not line.startswith("#"):
            name, _kind, _size, fp, _rest = line.split("\t", 4)
            fps[fp] = name
    for name, _kind, size, places in ov.read_catalogue():
        for lid, addr in places:
            base, data = texts[lid]
            in_level[lid].append((addr, size, name, data[addr - base:addr - base + size]))
    return exe, {k: sorted(v) for k, v in in_level.items()}, fps


def pair(v):
    """[(program, addr, size, fingerprint, counterpart or None, method, similarity)] for V."""
    exe, in_level, cat_fps = primary_functions()
    exe_fps, seen = {}, set()
    for _, _, name, b in exe:
        fp = ov.fingerprint(b)
        if fp in seen:
            exe_fps.pop(fp, None)
        else:
            exe_fps[fp] = name
        seen.add(fp)
    p_secs = versions.boot_sections(versions.PRIMARY)
    v_secs = versions.boot_sections(v)
    programs = [("boot", [(*v_secs[s], p_secs[s][0],
                           [f for f in exe if p_secs[s][0] <= f[0] < p_secs[s][0] + len(p_secs[s][1])])
                          for s in EXE_TEXT])]
    for lid in range(versions.LEVEL_COUNT):
        pbase, _ = versions.level_text(versions.PRIMARY, lid)
        vbase, vdata = versions.level_text(v, lid)
        programs.append((f"level_{lid:02d}", [(vbase, vdata, pbase, in_level[lid])]))
    rows = []
    for program, sections in programs:
        mine, theirs = [], []
        for base, data, pbase, funcs in sections:
            mine += ov.us_functions(data, base, pbase, funcs)
            theirs += funcs
        lookup = {**cat_fps, **exe_fps} if program == "boot" else {**exe_fps, **cat_fps}
        result = ov.pair_program(mine, theirs, lookup)
        by_name = {}
        for _, _, name, b in theirs:
            by_name.setdefault(name, ov.fingerprint(b))
        for i, (addr, size, b) in enumerate(mine):
            name, method, sim = result.get(i, (None, None, None))
            rows.append((program, addr, size, ov.fingerprint(b), name, method, sim))
        print(f"  {v.key} {program}: {len(mine)} functions, {len(result)} paired", flush=True)
    return rows


def primary_fingerprints():
    """{name: fingerprint} of the primary's functions (executable and catalogue)."""
    out = {}
    for name, _addr, _size, b in ov.exe_functions(everything=True):
        out[name] = ov.fingerprint(b)
    for line in ov.CATALOGUE.read_text().splitlines():
        if not line.startswith("#"):
            name, _kind, _size, fp, _rest = line.split("\t", 4)
            out.setdefault(name, fp)
    return out


def catalogue(v, rows):
    """The entries of functions.tsv (see the module docstring)."""
    pfp = primary_fingerprints()
    exe_rows = [r for r in rows if r[0] == "boot"]
    exe_by_fp = {}
    for r in exe_rows:
        exe_by_fp.setdefault(r[3], r)
    entries = {}          # key -> {"size", "fp", "places", "names": Counter, "methods", "sims"}
    order = []

    def add(key, r, place):
        e = entries.get(key)
        if e is None:
            e = entries[key] = {"size": r[2], "fp": r[3], "places": [], "names": collections.Counter(),
                                "best": {}, "kind": None}
            order.append(key)
        e["places"].append(place)
        if r[4]:
            e["names"][r[4]] += 1
            prev = e["best"].get(r[4])
            if prev is None or (r[6] or 0) > prev[1]:
                e["best"][r[4]] = (r[5], r[6] or 0.0)

    for r in exe_rows:
        add(("boot", r[1]), r, ("boot", r[1]))
        entries[("boot", r[1])]["kind"] = "exe"
    level_rows = sorted((r for r in rows if r[0] != "boot"), key=lambda r: (r[0], r[1]))
    for r in level_rows:
        lid = int(r[0][6:])
        if r[3] in exe_by_fp:
            add(("boot", exe_by_fp[r[3]][1]), r, (lid, r[1]))
        else:
            add(("fp", r[3]), r, (lid, r[1]))
    out = []
    for key in order:
        e = entries[key]
        if e["kind"] is None:
            e["kind"] = "level" if len({p[0] for p in e["places"]}) == 1 else "shared"
        counterpart = e["names"].most_common(1)[0][0] if e["names"] else None
        method, sim = e["best"].get(counterpart, (None, None))
        out.append({"key": key, "kind": e["kind"], "size": e["size"], "fp": e["fp"], "places": e["places"],
                    "counterpart": counterpart, "method": method, "sim": sim,
                    "same": counterpart is not None and pfp.get(counterpart) == e["fp"]})
    # One entry per counterpart name: the one with the same code first, else the most alike.
    claim = {}
    for e in out:
        c = e["counterpart"]
        if c is None:
            continue
        rank = (e["same"], e["sim"] or 0.0, -len(e["places"]))
        if c not in claim or rank > claim[c][0]:
            claim[c] = (rank, id(e))
    for e in out:
        c = e["counterpart"]
        own = own_name(v, e)
        e["name"] = c if c is not None and claim[c][1] == id(e) else own
        if e["name"] != c:
            e["counterpart_of_name"] = c
    return out


def primary_catalogue():
    """The primary's own functions in this table's form (its counterparts are itself), so the
    sync check (tools/version_sync.py) compares every version by the same columns."""
    out = []
    for name, addr, size, b in ov.exe_functions(everything=True):
        out.append({"name": name, "kind": "exe", "size": size, "fp": ov.fingerprint(b),
                    "places": [("boot", addr)], "counterpart": name, "method": "fingerprint",
                    "sim": 1.0, "same": True})
    seen = {e["name"]: e for e in out}
    for line in ov.CATALOGUE.read_text().splitlines():
        if line.startswith("#"):
            continue
        name, kind, size, fp, _n, places = line.split("\t")
        pl = [(int(x[:2]), int(x[3:], 16)) for x in places.split(",")]
        if name in seen:
            seen[name]["places"] += pl
            continue
        out.append({"name": name, "kind": kind, "size": int(size), "fp": fp, "places": pl,
                    "counterpart": name, "method": "fingerprint", "sim": 1.0, "same": True})
    return out


def own_name(v, e):
    prog, addr = e["places"][0]
    if prog == "boot":
        return f"func_{v.tag}_{addr:08X}"
    return f"func_{v.tag}_L{prog:02d}_{addr:08X}"


def fmt_place(p):
    return f"boot:{p[1]:08X}" if p[0] == "boot" else f"{p[0]:02d}:{p[1]:08X}"


def path(v):
    return v.config / "functions.tsv"


def write(v, entries):
    head = (f"# {v.label} ({v.id}): every distinct function and its {versions.PRIMARY.label} counterpart"
            " (tools/version_map.py; docs/VERSIONS.md).\n"
            "# name\tkind\tsize\tfingerprint\tcounterpart\tmethod\tsimilarity\tsame\tplaces\n")
    lines = []
    for e in entries:
        lines.append("\t".join([e["name"], e["kind"], str(e["size"]), e["fp"], e["counterpart"] or "-",
                                e["method"] or "-", "-" if e["sim"] is None else f"{e['sim']:.2f}",
                                "yes" if e["same"] and e["name"] == e["counterpart"] else "no",
                                ",".join(fmt_place(p) for p in sorted(e["places"], key=lambda p: (
                                    -1 if p[0] == "boot" else p[0], p[1])))]))
    v.config.mkdir(parents=True, exist_ok=True)
    path(v).write_text(head + "\n".join(lines) + "\n")


def read(v):
    """[{name, kind, size, fp, counterpart, method, sim, same, places: [(prog, addr)]}] of V's
    functions.tsv; prog is "boot" or a level number."""
    out = []
    for line in path(v).read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        n, k, s, fp, c, m, sim, same, places = line.split("\t")
        pl = []
        for p in places.split(","):
            a, b = p.split(":")
            pl.append(("boot" if a == "boot" else int(a), int(b, 16)))
        out.append({"name": n, "kind": k, "size": int(s), "fp": fp, "counterpart": None if c == "-" else c,
                    "method": None if m == "-" else m, "sim": None if sim == "-" else float(sim),
                    "same": same == "yes", "places": pl})
    return out


def summary(v, entries):
    t = collections.Counter()
    for e in entries:
        t[(e["kind"], "n")] += 1
        t[(e["kind"], "b")] += e["size"]
        t[(e["kind"], "same")] += e["same"] and e["name"] == e["counterpart"]
        t[(e["kind"], "other")] += e["counterpart"] is None or e["name"] != e["counterpart"]
    for k in ("exe", "shared", "level"):
        print(f"  {v.key} {k:<7} {t[(k, 'n')]:>6} functions {t[(k, 'b')]:>9} bytes; "
              f"{t[(k, 'same')]} the same code as the counterpart, {t[(k, 'other')]} without one")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", action="append")
    a = ap.parse_args()
    vs = [versions.get(k) for k in a.version] if a.version else list(versions.VERSIONS)
    for v in vs:
        entries = primary_catalogue() if v.primary else catalogue(v, pair(v))
        write(v, entries)
        summary(v, entries)
        print(f"wrote {path(v).relative_to(ROOT)}")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
Keep the versions in step (docs/VERSIONS.md, "Keeping the versions in step").

  python3 tools/version_sync.py check     # CI and gen_progress_report.py --check: tables only, no disc
  python3 tools/version_sync.py run       # before each push: rebuild, recheck and report every version
  python3 tools/version_sync.py carry     # copy a version's own C to versions with the same code, keep what matches
  python3 tools/version_sync.py ntsc      # the NTSC decomp (tools/ntsc_source.py): carry candidates, both ways

Every function has one C source for every version (src/), and a version has its own definition
(src/versions/<K>/) only where its bytes differ. `check` fails when:

  - a version's map (config/versions/<K>/map.tsv) or report is out of date with src/: a C function
    or a version-specific definition without its row, a row whose source is gone;
  - a version-specific definition is a copy: the shared C builds that version's bytes too (status
    `copy` from tools/version_check.py);
  - the same code is matched in one version and not in another: two versions' functions with one
    name and one identity (the fingerprint in config/versions/<K>/functions.tsv), finished in one,
    unfinished in the other, unless the other's code was checked and differs (status `differs` or
    `refs`, which compare every byte the C builds; the fingerprint masks some offsets);
  - the primary has C whose code a version shares (its row says `same`) and that version's check
    could not place it (`nomatch`): a translation table problem to fix, not a difference.

`run` is the command to run before each push, from a checkout with the dumps (tools/versions.py
dump) and the toolchain: the primary's build (bash tools/build_sn.sh, gen_progress_report.py) must
have run first; it then builds each version's own definitions, checks every C function against
every integrated version, carries version-specific C between versions, and rewrites the maps and
reports. Commit what it changes with the C.

The NTSC decomp, Lombyte's project, is the other decompilation of NTSC-U: it builds and counts
itself (tools/build_ntsc.sh) and is read, pinned to a commit, from a cache outside the repository
(tools/ntsc_source.py). Nothing here writes into its tree but its own report. `ntsc` lists, from
config/versions/us/ (our C checked against the NTSC-U bytes) and the NTSC decomp's own report, the
functions with the same code matched on one side and not on the other: config/versions/us/carry.tsv,
candidates for pull requests to the NTSC decomp, or for matching here.
"""
import argparse
import collections
import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import versions  # noqa: E402
import version_map  # noqa: E402

FINISHED = {"match", "asm", "joined"}


def read_map(v):
    out = {}
    for line in (v.config / "map.tsv").read_text().splitlines():
        if line and not line.startswith("#"):
            n, s, st, place, z, c = line.split("\t")
            out[n] = {"source": s, "status": st, "place": place}
    return out


def primary_finished():
    r = json.loads(versions.PRIMARY.report.read_text())
    return {f["name"] for u in r["units"] for f in u["functions"] if f["fuzzy_match_percent"] == 100.0}


def c_sources():
    """{name: source} of the shared C (src/, as the reports find it)."""
    import version_check as vc
    return vc.c_functions()


def check():
    problems = []
    shared = c_sources()
    fps = {v.key: {e["name"]: e["fp"] for e in version_map.read(v)} for v in versions.INTEGRATED}
    status = {versions.PRIMARY.key: {n: ("match" if n in primary_finished() else "none") for n in fps["pal"]}}
    import version_src
    for v in versions.INTEGRATED:
        if v.primary:
            continue
        m = read_map(v)
        status[v.key] = {n: r["status"] for n, r in m.items()}
        own = {n: str(p.relative_to(ROOT)) for n, p in version_src.overrides(v).items()}
        for n, src in shared.items():
            if n not in m:
                problems.append(f"{v.key}: {n} ({src}) has no row in config/versions/{v.key}/map.tsv")
            elif n not in own and m[n]["source"] != src:
                problems.append(f"{v.key}: {n}'s row says {m[n]['source']}, the C is in {src}")
        for n, src in own.items():
            if n not in m or m[n]["source"] != src:
                problems.append(f"{v.key}: {src} has no row of its own in config/versions/{v.key}/map.tsv")
        for n, r in m.items():
            if r["source"].startswith("src/") and r["source"] not in own.values() and n not in shared:
                problems.append(f"{v.key}: {n}'s row names {r['source']}, which no longer defines it")
            if r["status"] == "copy":
                problems.append(f"{v.key}: {r['source']} is a copy: the shared C matches {v.key} too; delete it")
            if r["status"] == "nomatch" and n in shared:
                problems.append(f"{v.key}: {n}: the shared C could not be placed here (nomatch): fix the table")
    # the same code, finished in one version and not in another
    keys = [v.key for v in versions.INTEGRATED]
    for n in sorted(set().union(*(fps[k] for k in keys))):
        groups = collections.defaultdict(list)
        for k in keys:
            if n in fps[k]:
                groups[fps[k][n]].append(k)
        for fp, ks in groups.items():
            done = [k for k in ks if status[k].get(n) in FINISHED]
            open_ = [k for k in ks if status[k].get(n, "none") not in FINISHED | {"differs", "refs", "absent"}
                     and k != "pal"]
            pal_open = "pal" in ks and "pal" not in done
            if done and open_:
                problems.append(f"{n}: the same code is finished in {', '.join(done)} and not in {', '.join(open_)}"
                                " (tools/version_sync.py carry)")
            if done and pal_open and any(k != "pal" for k in done):
                problems.append(f"{n}: matched in {', '.join(done)} with the primary's code, not in the primary")
    for p in problems:
        print("  " + p)
    import version_report
    rc = subprocess.run([sys.executable, str(ROOT / "tools/version_report.py"), "--check"]).returncode
    if problems or rc:
        sys.exit(f"*** {len(problems)} version sync problem(s){' and stale reports' if rc else ''}: "
                 "run python3 tools/version_sync.py run (docs/VERSIONS.md)")
    print("versions in step: " + ", ".join(f"{v.key}" for v in versions.INTEGRATED))


def sh(*cmd):
    print("+ " + " ".join(cmd), flush=True)
    r = subprocess.run(cmd, cwd=ROOT)
    if r.returncode:
        sys.exit(f"*** failed: {' '.join(cmd)}")


def carry(only=None):
    """Copy each version's own definitions to the versions with the same code that lack one,
    and keep a copy only where it builds that version's bytes."""
    import shutil
    import version_src
    kept = 0
    entries = {v.key: {e["name"]: e for e in version_map.read(v)} for v in versions.INTEGRATED if not v.primary}
    for src_v in versions.INTEGRATED:
        if src_v.primary:
            continue
        for n, path in version_src.overrides(src_v).items():
            for dst_v in versions.INTEGRATED:
                if dst_v.primary or dst_v is src_v or (only and dst_v.key not in only):
                    continue
                e, f = entries[dst_v.key].get(n), entries[src_v.key].get(n)
                if not e or not f or e["fp"] != f["fp"] or n in version_src.overrides(dst_v):
                    continue
                if read_map(dst_v).get(n, {}).get("status") in FINISHED:
                    continue
                verdict = version_src.try_one(dst_v, n, path)
                print(f"{dst_v.key} {n} from {src_v.key}: {verdict}")
                if verdict.startswith("EXACT"):
                    out = ROOT / "src/versions" / dst_v.key / path.relative_to(ROOT / "src/versions" / src_v.key)
                    out.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy(path, out)
                    kept += 1
    print(f"carried {kept} definitions")
    return kept


def run():
    py = sys.executable
    for v in versions.INTEGRATED:
        if not v.primary:
            sh(py, "tools/versions.py", "check", "--version", v.key)
    for v in versions.INTEGRATED:
        if not v.primary:
            sh(py, "tools/version_src.py", "build", "--version", v.key)
            sh(py, "tools/version_check.py", "--version", v.key)
    if carry():
        for v in versions.INTEGRATED:
            if not v.primary:
                sh(py, "tools/version_src.py", "build", "--version", v.key)
                sh(py, "tools/version_check.py", "--version", v.key)
    sh(py, "tools/version_report.py")
    check()


def ntsc():
    """config/versions/us/carry.tsv from our check against the NTSC-U bytes and the NTSC decomp's report."""
    import ntsc_source
    v = versions.get("us")
    sub = ntsc_source.path()
    if not (sub / "scripts/gen_progress_report.py").exists():
        sys.exit(f"{sub} is not a checkout of the NTSC decomp (tools/ntsc_source.py)")
    subprocess.run([sys.executable, "scripts/gen_progress_report.py"], cwd=sub, check=True,
                   stdout=subprocess.DEVNULL)
    rep = json.loads((sub / "build/progress/report.json").read_text())
    import re
    theirs = {}
    # its executable functions carry their file offset (the ELF's .text is at 0x1000, vram 0x100080),
    # its level functions their canonical place, the level in the name
    for u in rep["units"]:
        for f in u["functions"]:
            a = int((f.get("metadata") or {}).get("virtual_address", "0") or 0)
            m = re.match(r"FUN_L(\d\d)_", f["name"])
            key = (int(m.group(1)), a) if m else ("boot", a + 0xFF080)
            theirs.setdefault(key, []).append((f["name"], f.get("fuzzy_match_percent", 0.0) == 100.0))
    ours = read_map(v)
    pal_done = primary_finished()
    rows = []
    for e in version_map.read(v):
        place = e["places"][0]
        their = [t for p in e["places"] for t in theirs.get(p, [])]
        their_done = any(d for _, d in their)
        st = ours.get(e["name"], {}).get("status", "none")
        if st == "match" and their and not their_done:
            rows.append((e["name"], "to the NTSC decomp", their[0][0], version_map.fmt_place(place)))
        if their_done and e["same"] and e["name"] not in pal_done:
            rows.append((e["name"], "from the NTSC decomp", their[0][0], version_map.fmt_place(place)))
    head = ("# Functions with the same code in PAL v2.00 and NTSC-U, matched on one side only (tools/version_sync.py ntsc).\n"
            "# to the NTSC decomp: our C builds the NTSC-U bytes and the NTSC decomp has not matched it (a pull request\n"
            "# there); from the NTSC decomp: matched there, not here. Names, places and directions only.\n"
            "# name\tdirection\tNTSC decomp name\tNTSC-U place\n")
    (v.config / "carry.tsv").write_text(head + "".join("\t".join(r) + "\n" for r in rows))
    c = collections.Counter(r[1] for r in rows)
    print(f"ntsc: {c['to the NTSC decomp']} to carry to the NTSC decomp, {c['from the NTSC decomp']} from it;"
          f" wrote config/versions/us/carry.tsv")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("command", choices=("check", "run", "carry", "ntsc"))
    a = ap.parse_args()
    {"check": check, "run": run, "carry": carry, "ntsc": ntsc}[a.command]()


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
Plans, tracks and integrates waves of worker agents (docs/WORKER.md).

  python3 tools/wave.py plan NAME [--role match|compile] [--count N]
                         [--budget N] [--near | --fresh | --overlay] [func_X ...]
      Picks functions (the ones named, or from tools/triage.py), writes each
      one's CONTEXT.md and m2c sketch, starts a fresh BUDGET of try_func
      runs, and prints each worker's one-line prompt.
  python3 tools/wave.py plan NAME --queue [--min-size N] [--max-size N] ...
      A queue wave (docs/QUEUE.md): workers take several functions each.
  python3 tools/wave.py claim NAME ID [--count N]
      For a queue worker: claims the next N functions and prints each
      one's packet (dossier, assembly, matched C to start from).
  python3 tools/wave.py tokens NAME
      Tokens per worker and per match, from the sub-agent transcripts.
  python3 tools/wave.py status NAME
      One line per function: verdict, runs used, best candidate.
  python3 tools/wave.py integrate NAME
      Applies the EXACT results through tools/integrate.py (in Docker).
      Run the full build afterwards, as always.
  python3 tools/wave.py land NAME [--batch] [--reject func_X ...]
      (--batch, overlay waves: apply and re-check every EXACT, leave the
      report and the one commit to the lead; --reject skips matches the
      lead's review turned down.)
      For each EXACT result, one at a time: integrate it, run the full build,
      check it added one exact function and no size mismatch, regenerate the
      progress report and commit that function alone. A failure restores the
      source file and moves on. Needs src/ and progress/ clean.

  python3 tools/wave.py long NAME func_X ... [--arm opus] [--budget 30]
      Sets up long-function workers (docs/LONG_FUNCTIONS.md): each
      function's dossier, m2c sketch and PACKET.md, and a BUDGET in its
      build-sn/try/<func>/<arm>/ folder; prints one prompt per function.
      Refuses a function rank_candidates blocks.
  python3 tools/wave.py stage [--level NN] [--max F] [func_X ...]
      Shares near misses (docs/NONMATCHING.md): each overlay stub's closest
      attempt within --max (a fraction of the bytes; default 0.15) goes to
      nonmatching/<dir>/<func>.c through tools/nonmatching.py, unless the
      staged one is already as close. Packets start from it; landing the
      function removes it.
  python3 tools/wave.py salvage [--ports] [--level NN] [--reject func_X ...]
      Lands every overlay stub that already has an EXACT run logged in
      build-sn/try (a wave stopped before landing, a file that clashed
      then): batch-landed like `land --batch`. Candidates crediting Lombyte
      are left out unless --ports, which lands only those, so the two go in
      separate commits. --level NN keeps to func_LNN_ functions: another
      agent may own another level's work. Salvages of different levels
      (and --ports beside a plain one) run at once: each has its own
      manifests and lock owner, writes take the landing lock only while
      writing, and files that land one by one do so four at a time.

--near picks earlier attempts that came close (BYTES within 40, a size
within 8 bytes, or a near-miss in src/); --fresh, the default, picks
functions nobody has tried, smallest first. Waves are recorded in
build-sn/waves/NAME.json.

--overlay --near is a near wave (docs/QUEUE.md, "Near misses"): overlay
stubs whose best attempt came within --near-max of retail (a fraction of
the bytes; default 0.10) or within 8 bytes of its size, closest first.
Each one's best candidate is kept as build-sn/try/<func>/best.c, and
tools/near_diffs.py writes what still differs to BEST_DIFF.txt; both go in
the packet.

A run's chance of an EXACT falls with every run spent on the same function
(waves q6-q26: about 12% for runs 1-5, 8% for 6-10, 3-4% after), so the
default budget is 6: what's left of a function is a near wave's job.

Overlay functions (func_LNN_XXXXXXXX, docs/OVERLAYS.md) are a separate
pool: named explicitly (func_L00_... on the command line, same as any
other function) or picked with --overlay, which orders the catalogue's
shared-and-level functions the way docs/OVERLAYS.md's "Relatives" section
recommends -- shared code present in all 19 levels first, smaller first --
after screening asm/overlays/<name>.s through rank_candidates' blocked
patterns and skipping anything already matched. A wave is either all
overlay names or all executable ones, never mixed. `land` treats an
overlay wave differently (see land_overlay()): no full build (the
executable does not link src/overlays/), and progress/report.json does
not count overlay functions yet -- see the TODO where it's called.
"""
from __future__ import annotations
import argparse
import json
import re
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
# Inside OpenRAC, commit scopes name the game version first (OpenRAC's CONTRIBUTING.md).
SCOPE = "rac1/pal/" if (ROOT.parents[2] / "games/rac1/game.json").exists() else ""
sys.path.insert(0, str(ROOT / "tools"))

import claims as shared  # noqa: E402
import dossier  # noqa: E402
import integrate as checker  # noqa: E402
import rank_candidates  # noqa: E402
import triage  # noqa: E402
import srcnames

WAVES = ROOT / "build-sn/waves"
TRY = ROOT / "build-sn/try"
SECTION = {"match": "Matching", "compile": "First compile"}
CLOSE_BYTES, CLOSE_SIZE = 40, 8
FAR = 0.15          # a best attempt further off than this is a starting point, not a near miss
NEAR_MAX = 0.10     # --overlay --near: the furthest a best attempt may be, as a fraction of the bytes
CREDIT = "Adapted from Lombyte"
ALL_LEVELS = 19  # docs/OVERLAYS.md: how many levels there are in total

OVERLAY_NAME = re.compile(r"^func_L\d{2}_[0-9A-Fa-f]{8}$")
OVERLAY_STUB_LINE = re.compile(r"^\s*INCLUDE_ASM\([^)]*\bfunc_L\d{2}_[0-9A-Fa-f]{8}\)")
OVERLAY_DEF_LINE = re.compile(r"^(?!extern\b)[A-Za-z_].*?\bfunc_L\d{2}_[0-9A-Fa-f]{8}(?:_r)?\s*\(")
# The same trailing-comment convention exe stubs use for a known real name
# (tools/triage.py's NAME_COMMENT); overlay stubs don't have one yet, but a
# worker or a future generator may leave one the same way.
OVERLAY_NAME_COMMENT = re.compile(
    r"INCLUDE_ASM\([^)]*\b(func_L\d{2}_[0-9A-Fa-f]{8})\);[^\S\n]*/\*[^\S\n]*([A-Za-z_]\w*)[^\S\n]*(?:\(|\*/)")


def is_overlay_wave(names: list[str]) -> bool:
    """True if NAMES are all overlay functions, False if all executable;
    exits if it's a mix (docs/OVERLAYS.md functions and exe functions are
    matched, integrated and landed differently)."""
    overlay = [bool(OVERLAY_NAME.match(n)) for n in names]
    if any(overlay) and not all(overlay):
        sys.exit("a wave must be all overlay functions or all executable functions, not both")
    return bool(names) and overlay[0]


def closeness(row: dict) -> int | None:
    """How far an earlier attempt is from exact, in bytes; None if not close."""
    if row["status"] == "near-miss":
        return int(row["size"] * (100 - row["match_percent"]) / 100)
    verdict = row["last_attempt"]
    m = re.match(r"BYTES (\d+)/", verdict)
    if m and int(m.group(1)) <= CLOSE_BYTES:
        return int(m.group(1))
    m = re.search(r"SIZE \(?(?:ours )?(\d+)(?: / retail |/)(\d+)", verdict)
    if m and abs(int(m.group(1)) - int(m.group(2))) <= CLOSE_SIZE:
        return abs(int(m.group(1)) - int(m.group(2)))
    return None


def logged_runs(name: str) -> list[tuple[str, str]]:
    """(candidate file, verdict) of every run ever logged for NAME, oldest
    log first (runs.<stamp>.log, then runs.log), then each trial arm's
    (try_func --arm, docs/LONG_FUNCTIONS.md) with its folder in the path."""
    work = TRY / name
    logs = sorted(work.glob("runs.*.log")) + [work / "runs.log"] + sorted(work.glob("*/runs.log"))
    return [(str(log.parent.relative_to(work) / c), v.strip()) for log in logs if log.exists()
            for c, v in (l.split(None, 1) for l in log.read_text(errors="replace").splitlines() if " " in l)]


def miss(verdict: str) -> float | None:
    """How far a verdict is from EXACT: the fraction of bytes that differ,
    or 0.05 plus the size difference's share for a size within CLOSE_SIZE
    or 3% of the function; None if it is not close or not a comparison."""
    m = re.match(r"BYTES (\d+)/(\d+)", verdict)
    if m:
        return int(m.group(1)) / int(m.group(2))
    m = re.match(r"SIZE ours (\d+) / retail (\d+)", verdict)
    if m:
        off, size = abs(int(m.group(1)) - int(m.group(2))), int(m.group(2))
        if off <= max(CLOSE_SIZE, size * 0.03):
            return 0.05 + off / size        # one instruction too many in 2 KB beats half the bytes wrong
    return None


def overlay_stubs(findex) -> list[tuple[str, Path]]:
    return [(n, path) for path, fns in {e[0]: e[1] for e in findex.values()}.items() for n, is_c in fns if not is_c]


def logged_exact(name: str) -> str | None:
    """The candidate of NAME's latest logged EXACT run, if its file is still there."""
    hits = [c for c, v in logged_runs(name) if v.startswith("EXACT") and (TRY / name / c).exists()]
    hits = [c for c in hits if c.startswith("lead/")] or hits      # a lead's cleaned-up version wins
    return str((TRY / name / hits[-1]).relative_to(ROOT)) if hits else None


def choose_overlay_near(args) -> list[str]:
    """Overlay stubs whose best attempt came close (see miss()), closest
    first, skipping any that have an EXACT run (that's salvage's), that
    another agent holds, or that an earlier near wave queued."""
    findex = dossier.overlay_file_index()
    held = shared.claimed_by_others("")
    queued = set()
    for other in WAVES.glob("*.json"):
        try:
            record = json.loads(other.read_text())
        except ValueError:
            continue
        if isinstance(record, dict) and record.get("near"):
            queued.update(record.get("functions", []))
    rows = []
    for name, _path in overlay_stubs(findex):
        if name in held or name in queued or not (TRY / name).is_dir():
            continue
        runs = logged_runs(name)
        if any(v.startswith("EXACT") for _, v in runs):
            continue
        close = [(d, c) for c, v in runs if (d := miss(v)) is not None and (TRY / name / c).exists()]
        if not close:
            continue
        d, _ = min(close)
        if d <= args.near_max:
            rows.append((d, name))
    rows.sort()
    return [name for _, name in rows][:args.count]


def best_logged(name: str) -> tuple[float, str, str] | None:
    """(closeness, verdict, candidate path) of NAME's closest attempt: its
    run logs, or its staged near miss (nonmatching/) if that is as close."""
    import nonmatching
    close = [(d, -k, str(TRY / name / c), v) for k, (c, v) in enumerate(logged_runs(name))
             if (d := miss(v)) is not None and (TRY / name / c).exists()]
    best = min(close) if close else None            # of equally close attempts, the latest
    path = nonmatching.staged().get(name)
    if path is not None:
        v = nonmatching.verdict_of(path)
        d = miss(v)
        if d is not None and (best is None or d <= best[0]):
            return d, v, str(path)
    return (best[0], best[3], best[2]) if best else None


def keep_best(name: str) -> None:
    """Saves NAME's closest earlier candidate as best.c before a new round
    starts writing new candidates."""
    import nonmatching
    found = best_logged(name)
    if found:
        _, v, path = found
        (TRY / name / "best.c").write_text(nonmatching.body(Path(path).read_text(errors="replace")))
        (TRY / name / "BEST.md").write_text(f"{v}\n{Path(path).relative_to(ROOT)} when it was logged\n")


def choose(args) -> list[str]:
    if args.funcs:
        return args.funcs
    if getattr(args, "overlay", False) and getattr(args, "near", False):
        return choose_overlay_near(args)
    if getattr(args, "overlay", False):
        return choose_overlay(args)
    rows = [r for r in triage.triage() if r["route"] != "blocked"]
    if args.near:
        close = [(closeness(r), r["name"]) for r in rows]
        return [name for d, name in sorted(c for c in close if c[0] is not None)][:args.count]
    fresh = [r for r in rows if not r["last_attempt"] and "attempted before" not in r["reason"]]
    return [r["name"] for r in sorted(fresh, key=lambda r: r["size"])][:args.count]


def choose_overlay(args) -> list[str]:
    """The default overlay pool (docs/OVERLAYS.md, "Relatives"): shared
    functions present in every level first, smaller first within that,
    after screening each one's asm/overlays/<name>.s through
    rank_candidates' blocked-pattern triage and skipping anything already
    matched (kind "exe" is executable code under another name -- it has no
    src/overlays file of its own to land into) or without an asm file yet
    (docs/OVERLAYS.md's jump-table functions, or asm/overlays regenerating)."""
    catalogue = dossier.load_overlay_catalogue()
    findex = dossier.overlay_file_index()
    rows = []
    pieces = fragments()
    held = shared.claimed_by_others("")        # any agent's claim (tools/claims.py)
    queued = set()                              # an earlier wave's queue, even where no one claimed it yet
    for other in WAVES.glob("*.json"):
        try:
            queued.update(json.loads(other.read_text()).get("functions", []))
        except (ValueError, AttributeError):
            pass
    for name, (kind, size, levels_count, _places) in catalogue.items():
        if kind == "exe" or name in pieces or name in held or name in queued:
            continue
        entry = findex.get(name)
        if entry is None:
            continue
        _path, fns = entry
        is_c = next((c for n, c in fns if n == name), False)
        if is_c:
            continue
        asm = ROOT / "asm/overlays" / f"{name}.s"
        if not asm.exists():
            continue
        if any((TRY / name).glob("runs*.log")) or (TRY / name / "NOTES.md").exists():
            continue                                # an earlier wave tried it or stopped on it at once
        verdict, _cat, _detail = rank_candidates.classify(name, asm.read_text(errors="replace"), "text", size)
        if verdict == "blocked":
            continue
        if not args.min_size <= size <= args.max_size:
            continue
        rank = 0 if (kind == "shared" and levels_count == ALL_LEVELS) else 1
        rows.append((rank, size, name))
    rows.sort()
    if args.family:
        rows = family_order(rows, findex)
    return [name for _, _, name in rows][:args.count]


BRANCH_OUT = re.compile(r"\b(b[a-z0-9]*)\s+(?:[^,\n]+,\s*)*(func_(?:L\d\d_)?[0-9A-F]{8}|\.L[0-9A-F]{8})")


TEMP_REGS = {1, 2, 3, 12, 13, 14, 15, 24, 25}      # $at, $v0-$v1, $t4-$t7, $t8-$t9
STORE_OP = re.compile(r"^(sb|sh|sw|sd|sq|swl|swr|sdl|sdr|swc1|sdc1|sqc2)$")
READS_ALL_OP = re.compile(r"^(b[a-z]*|jr|j|mtc1|dmtc1|ctc1|mthi|mtlo|teq|tne|mult1?|multu1?|div1?|divu1?"
                          r"|madd|maddu|c\.[a-z.]+)$")
ASM_INS = re.compile(r"^\s*/\*[^*]*\*/\s+([a-z0-9.]+)\s*(.*)$")


def reads_unset_register(text: str) -> bool:
    """True when, in address order, the function reads a temporary register
    before anything in it writes one: the tail of a larger function, whose
    head set it. A call counts as writing $v0/$v1. Measured on the landed
    matches: none of them trips it."""
    written = set()
    for line in text.splitlines():
        m = ASM_INS.match(line)
        if not m or m.group(1) == ".word":
            continue
        op, args = m.group(1), m.group(2)
        if op in ("jal", "jalr"):
            written |= {2, 3}
            continue
        regs = [(int(r), i) for i, r in enumerate(re.findall(r"\$(\d+)\b", args))]
        if not regs:
            continue
        if STORE_OP.match(op) or READS_ALL_OP.match(op):
            reads, writes = [r for r, _ in regs], []
        else:
            reads, writes = [r for r, i in regs if i > 0], [regs[0][0]]
        if any(r in TEMP_REGS and r not in written for r in reads):
            return True
        written.update(writes)
    return False


def fragments() -> set[str]:
    """Catalogue entries that are pieces of a larger function, not
    functions: one that branches (not calls) outside itself. The entry such
    a branch lands in may still be a whole function (a shared return that
    is also called), so it stays in the pool. The splitter cuts at call targets
    and after returns, which also cuts functions with an early return or a
    shared tail. No C matches a piece alone."""
    out = set()
    for path in (ROOT / "asm/overlays").glob("func_L*.s"):
        text = path.read_text(errors="replace")
        labels = set(re.findall(r"^\s*(\.L[0-9A-F]{8}):", text, flags=re.M))
        for m in BRANCH_OUT.finditer(text):
            target = m.group(2)
            if target.startswith("func_") or target not in labels:
                out.add(path.stem)
        if reads_unset_register(text):
            out.add(path.stem)
    return out


def family_order(rows: list, findex: dict) -> list:
    """ROWS reordered for reuse (docs/OVERLAYS.md, "Relatives"): first the
    functions with a matched relative or variant parent (their packet
    carries that C: a port), most similar first; then one function, the
    smallest, of each family nobody has matched, largest family first, so
    each match opens the most ports."""
    def matched(name: str) -> bool:
        entry = findex.get(name)
        if entry and any(n == name and is_c for n, is_c in entry[1]):
            return True
        return logged(TRY / name, "")[0].startswith("EXACT")    # matched, not landed yet
    edges = []
    for path, a, b, sim in ((ROOT / "config/overlays/families.tsv", 0, 3, 6),
                            (ROOT / "config/overlays/variants.tsv", 0, 1, None)):
        for row in (l.split("\t") for l in path.read_text().splitlines() if not l.startswith("#")):
            edges.append((row[a], row[b], float(row[sim]) if sim else 1.0))
    group = {}
    def find(x):
        while group.setdefault(x, x) != x:
            group[x] = x = group[group[x]]
        return x
    best = {}
    for a, b, sim in edges:
        if sim >= 0.9:
            group[find(a)] = find(b)
        for x, y in ((a, b), (b, a)):
            if matched(y):
                best[x] = max(best.get(x, 0), sim)
    size_of = {name: size for _, size, name in rows}
    members = {}
    for name in size_of:
        members.setdefault(find(name), []).append(name)
    ports = sorted((r for r in rows if r[2] in best), key=lambda r: -best[r[2]])
    firsts = sorted((min(m, key=size_of.get) for m in members.values()
                     if len(m) > 1 and not any(n in best for n in m)),
                    key=lambda n: -sum(size_of[x] for x in members[find(n)]))
    picked = {r[2] for r in ports} | set(firsts)
    by_name = {r[2]: r for r in rows}
    return ports + [by_name[n] for n in firsts] + [r for r in rows if r[2] not in picked]


def plan(args, quiet: bool = False) -> None:
    names = choose(args)
    pieces = all_joined_pieces()
    dropped = [n for n in names if n in pieces]
    if dropped:         # not functions of their own: their owner's packet carries them
        print(f"left out, pieces of a joined function (config/overlays/joined.tsv): {' '.join(dropped)}")
        names = [n for n in names if n not in pieces]
    if not names:
        sys.exit("nothing to plan")
    overlay = is_overlay_wave(names)
    if overlay and args.role == "compile":
        sys.exit("no m2c sketch exists for overlay functions yet, so there is nothing for a "
                  "compile worker to start from (docs/OVERLAYS.md); use --role match")
    near = overlay and args.near
    held = [f"{n} ({shared.holder(n)})" for n in names if shared.holder(n)]
    if held:            # a claimed function never reaches this wave's workers: claim() skips it
        print("still claimed, release before launching (claims.py release OWNER func):\n  " + "\n  ".join(held))
    stamp = time.strftime("%Y%m%d-%H%M%S")
    for name in names:
        work = TRY / name
        work.mkdir(parents=True, exist_ok=True)
        if near:
            keep_best(name)
        if (work / "runs.log").exists():  # A fresh budget for this round.
            (work / "runs.log").rename(work / f"runs.{stamp}.log")
        (work / "BUDGET").write_text(f"{args.budget}\n")
    if overlay:
        dossier.overlay_write(names)
        if near:
            diffs = docker("python", "tools/near_diffs.py", *names)
            (WAVES / f"{args.name}.diffs.log").write_text(diffs.stdout + diffs.stderr)
    else:
        needs_sketch = [n for n in names if not (TRY / n / "m2c.c").exists()]
        broken = [n for n in names if n not in needs_sketch
                  and "{" not in (TRY / n / "m2c.c").read_text(errors="replace")]
        if args.role == "compile" and broken:
            sys.exit(f"no m2c sketch for {', '.join(broken)}: a compile worker needs one")
        if needs_sketch:
            dossier.sketches(needs_sketch)
        dossier.write(names)
    WAVES.mkdir(parents=True, exist_ok=True)
    record = {"name": args.name, "role": args.role, "budget": args.budget, "created": stamp,
              "started": time.time(), "functions": names, "pool": "overlay" if overlay else "exe",
              "queue": args.queue, "near": near}
    (WAVES / f"{args.name}.json").write_text(json.dumps(record, indent=2) + "\n")
    if quiet:
        return
    if args.queue:
        print(f"\nwave {args.name}: a queue of {len(names)} functions, budget {args.budget} runs each. "
              f"One prompt per worker (ID unique, N functions each, COUNT claimed at a time):\n\n"
              f"Read docs/QUEUE.md and follow it exactly. WAVE={args.name} ID=s01 N=6 COUNT=2")
        return
    print(f"\nwave {args.name}: {len(names)} {args.role} workers, budget {args.budget} runs each\n")
    for name in names:
        print(prompt(name, args.role, args.budget))


def prompt(name: str, role: str, budget: int) -> str:
    return (f"You are a {role} worker on {name} in the repository {ROOT} "
            f"(run every command from there). Follow docs/WORKER.md, the rules and the "
            f"\"{SECTION[role]}\" section; your context is build-sn/try/{name}/CONTEXT.md and "
            f"your budget is {budget} try_func runs. Work alone: never use the Agent, "
            f"WebSearch or WebFetch tools.")


def load(name: str) -> dict:
    path = WAVES / f"{name}.json"
    if not path.exists():
        sys.exit(f"no wave {name} in {WAVES.relative_to(ROOT)}")
    return json.loads(path.read_text())


def results(wave: dict) -> list[tuple[str, str, str, int]]:
    """Verdicts written during this wave; an older RESULT.md means pending."""
    rows = []
    for name in wave["functions"]:
        work = TRY / name
        path = work / "RESULT.md"
        fresh = path.exists() and path.stat().st_mtime >= wave.get("started", 0)
        result = path.read_text(errors="replace").strip().splitlines() if fresh else []
        runs = len((work / "runs.log").read_text().splitlines()) if (work / "runs.log").exists() else 0
        candidate = result[1].strip().strip("`").split()[0] if len(result) > 1 and result[1].strip() else ""
        if candidate and not (ROOT / candidate).exists() and (work / Path(candidate).name).exists():
            candidate = str((work / Path(candidate).name).relative_to(ROOT))  # A bare "p6.c".
        verdict = result[0].strip() if result else "(no result yet)"
        if not result and wave.get("queue"):
            verdict, candidate = logged(work, verdict, wave)
        rows.append((name, verdict, candidate, runs))
    return rows


def logged(work: Path, default: str, wave: dict | None = None) -> tuple[str, str]:
    """The best run in WORK/runs.log (a queue wave's record: try_func
    writes it, so nothing depends on what a worker reports): an EXACT, or
    else the closest BYTES, or else the last verdict."""
    # A later plan renames runs.log to runs.<its stamp>.log: this wave's
    # runs are in the first such file stamped after it, or in runs.log.
    later = sorted(p for p in work.glob("runs.*.log") if wave and p.name[5:-4] > wave.get("created", ""))
    log = later[0] if later else work / "runs.log"
    runs = [l.split(None, 1) for l in log.read_text().splitlines() if " " in l] if log.exists() else []
    if not runs:
        return default, ""
    def rank(i_run):
        i, (_, verdict) = i_run
        if verdict.startswith("EXACT"):
            return (0, -i)          # the latest EXACT: a lead's cleaned-up version wins
        m = re.match(r"BYTES (\d+)/", verdict)
        return (1, int(m.group(1))) if m else (2, 0)
    candidate, verdict = min(enumerate(runs), key=rank)[1]
    path = work / candidate
    return verdict.strip(), str(path.relative_to(ROOT)) if path.exists() else ""


def claims(wave: dict) -> Path:
    return WAVES / f"{wave['name']}.claims"


def claim(args) -> None:
    """Hands the next unclaimed functions of a queue wave to worker ID and
    prints each one's packet: its dossier, its assembly and matched C to
    start from. Creating the claim file is the lock."""
    wave = load(args.name)
    claims(wave).mkdir(parents=True, exist_ok=True)
    # A worker that still holds a function it has not worked (no run logged, no
    # NOTES.md written this wave) gets that packet again: workers re-ran claim to
    # see their packet and took functions they never worked.
    started = wave.get("started", 0)
    held = [p.name for p in sorted(claims(wave).iterdir())
            if p.read_text().strip() == args.id
            and not (TRY / p.name / "runs.log").exists()
            and not ((TRY / p.name / "NOTES.md").exists()
                     and (TRY / p.name / "NOTES.md").stat().st_mtime >= started)]
    if held:
        for name in held:
            print(packet(name, wave["budget"], wave.get("near", False)))
        return
    got = []
    for name in wave["functions"]:
        if len(got) == args.count:
            break
        if (claims(wave) / name).exists() or not shared.claim(f"{args.name}:{args.id}", name):
            continue                        # this wave's, or another agent's (tools/claims.py)
        (claims(wave) / name).write_text(args.id + "\n")
        got.append(name)
    if not got:
        print("QUEUE EMPTY")
        return
    for name in got:
        print(packet(name, wave["budget"], wave.get("near", False)))


def all_joined_pieces() -> set[str]:
    """Every catalogue entry that is a later piece of some joined function."""
    table = ROOT / "config/overlays/joined.tsv"
    if not table.exists():
        return set()
    return {p for line in table.read_text().splitlines() if line and not line.startswith("#")
            for p in line.split("\t", 1)[1].split()}


def joined_pieces(name: str) -> list[str]:
    """The catalogue entries after NAME that are pieces of the same function
    (config/overlays/joined.tsv; docs/OVERLAYS.md, "Joined functions")."""
    table = ROOT / "config/overlays/joined.tsv"
    if not table.exists():
        return []
    for line in table.read_text().splitlines():
        if line.startswith(name + "\t"):
            return line.split("\t", 1)[1].split()
    return []


def packet(name: str, budget: int, near: bool = False) -> str:
    work = TRY / name
    context = (work / "CONTEXT.md").read_text(errors="replace").splitlines() if (work / "CONTEXT.md").exists() else []
    keep = [l for l in context[1:] if not l.startswith(("  - `func_", "- No m2c", "- The executable does not",
                                                        "- Functions in this file", "- Retail assembly"))]
    asm_path = next((p for p in (ROOT / "asm/overlays" / f"{name}.s",
                                 *ROOT.glob(f"asm/nonmatchings/*/{name}.s")) if p.exists()), None)
    asm = [re.sub(r"^\s*/\*[^*]*\*/\s*", "    ", l) for l in asm_path.read_text().splitlines()
           if l.strip() and not l.startswith((".section", "/* Handwritten", "nonmatching"))] if asm_path else []
    for piece in joined_pieces(name):
        # A joined function: the catalogue cut it apart, the C is one function under the first name.
        path = ROOT / "asm/overlays" / f"{piece}.s"
        if path.exists():
            asm += [f"    # the same function goes on (catalogue entry {piece}: a branch to that name is a branch "
                    "to here, inside the function)"]
            asm += [re.sub(r"^\s*/\*[^*]*\*/\s*", "    ", l) for l in path.read_text().splitlines()
                    if l.strip() and not l.startswith((".section", "/* Handwritten", "nonmatching", ".align"))]
        else:
            asm += [f"    # the function ends with the 4-byte entry {piece}: the delay slot of the jump above, "
                    "or a `jr $31` and its nop"]
    used = [int(m.group(1)) for c in work.glob("p*.c") if (m := re.fullmatch(r"p(\d+)\.c", c.name))]
    first = max(used) + 1 if used else 0     # never overwrite an earlier round's candidates: the run logs name them
    out = [f"===== {name}: budget {budget} runs, work in build-sn/try/{name}/, "
           f"your first candidate is p{first}.c =====", *keep]
    if joined_pieces(name):
        out += ["- A joined function: the assembly below is several catalogue entries, written as ONE C function "
                f"named {name}. Earlier notes about its size were made against the first entry alone: ignore them."]
    out += ["", "## Assembly", *asm]
    best, diff = work / "best.c", work / "BEST_DIFF.txt"
    if near and best.exists():
        shown = diff.read_text(errors="replace").splitlines() if diff.exists() else ["(no diff written)"]
        far = (d := miss(shown[0])) is None or d > FAR
        how = ("An earlier attempt, still far off: start from it, keep what already matches (declarations, "
               "control flow, the blocks the diff below doesn't list) and rework the rest freely."
               if far else "A near miss (QUEUE.md, \"Near misses\"): start from this candidate and change as "
               "little as you can.")
        out += ["", f"## Best earlier attempt: {shown[0]} (build-sn/try/{name}/best.c)",
                f"{how} Your {budget} runs are a fresh budget: a \"budget spent\" in NOTES.md is the "
                f"earlier round's.", best.read_text(errors="replace"),
                "", "## What still differs (offset, ours, retail)", *(shown[1:] or ["- nothing listed"])]
    port = lombyte_port(name)
    if port:
        path, ntsc, text, caveat = port
        out += ["", f"## Lombyte's matched C for this function: {ntsc} in {path}",
                "Port it (QUEUE.md, \"Lombyte ports\"): it matched the US build, so start from it and keep its "
                "control flow; your candidate's comment must credit it." + caveat, text]
    for label, other in start_from(name):
        out += ["", f"## Matched C to start from: {label}", other]
    return "\n".join(out) + "\n"


def lombyte_port(name: str) -> tuple[str, str, str, str] | None:
    """(Lombyte file, its name, its C, a caveat when its function's size
    differs from ours) when Lombyte matched NAME's US counterpart
    (tools/lombyte.py; docs/SIBLING_DECOMPS.md)."""
    import lombyte
    if not lombyte.MAP.exists() or not lombyte.LOMBYTE.is_dir():
        return None
    pair = next((p for p in lombyte.load_map() if p["pal"] == name and p["ntsc_exact"]), None)
    found = pair and lombyte.definition(pair["ntsc"])
    if not found:
        return None
    path, text = found
    there = pair.get("ntsc_size", pair["size"])
    caveat = "" if there == pair["size"] else (
        f" Its function is {there} bytes and ours {pair['size']}: the PAL code differs, or the two projects "
        "cut the functions there differently (config/overlays/us_map.tsv), so adapt it, don't port it as is.")
    return path, pair["ntsc"], text, caveat


def start_from(name: str) -> list[tuple[str, str]]:
    """Matched C worth starting from: the function this one is a variant of
    or its variants, its nearest relative, then up to two short matched
    functions of its own file."""
    if not OVERLAY_NAME.match(name):
        return []
    findex = dossier.overlay_file_index()
    def c_of(other: str) -> str | None:
        entry = findex.get(other)
        if entry and any(n == other and is_c for n, is_c in entry[1]):
            return extract_definition(entry[0], other)
        if not OVERLAY_NAME.match(other):          # a relative in the executable
            start = re.compile(rf"^(?!extern\b)[A-Za-z_][^;]*\b{other}\s*\([^;]*$")
            for path in sorted((ROOT / "src/game").glob("*.c")):
                lines = path.read_text(errors="replace").splitlines()
                for i, line in enumerate(lines):
                    if other in line and start.match(line):
                        depth, seen = 0, False
                        for j in range(i, len(lines)):
                            depth += lines[j].count("{") - lines[j].count("}")
                            seen = seen or "{" in lines[j]
                            if seen and depth == 0:
                                return "\n".join(lines[i:j + 1]) + "\n"
        return None
    wanted = []
    for path, a, b, label in ((ROOT / "config/overlays/variants.tsv", 0, 1, "differs only in a number"),
                              (ROOT / "config/overlays/families.tsv", 0, 3, "its nearest relative")):
        for row in (l.split("\t") for l in path.read_text().splitlines() if path.exists() and not l.startswith("#")):
            if row[a] == name:
                wanted.append((row[b], label))
            elif row[b] == name:
                wanted.append((row[a], label))
    entry = findex.get(name)
    if entry:
        wanted += [(n, "same file") for n, is_c in entry[1] if is_c]
    out, seen = [], set()
    for other, label in wanted:
        body = None if other in seen else c_of(other)
        seen.add(other)
        if body and (label != "same file" or body.count("\n") <= 25):
            out.append((f"{other} ({label})", body))
        if len(out) == 3:
            break
    return out


def tokens(args) -> None:
    """Tokens each worker of a queue wave used, from Claude Code's sub-agent
    transcripts (a worker's prompt carries WAVE= and ID=), against what it
    matched according to runs.log."""
    wave = load(args.name)
    sizes = {n: s for n, (_k, s, *_r) in dossier.load_overlay_catalogue().items()} if wave.get("pool") == "overlay" else {}
    mine = {}
    for path in sorted(claims(wave).glob("func_*")) if claims(wave).is_dir() else []:
        verdict, _ = logged(TRY / path.name, "(no runs)", wave)
        mine.setdefault(path.read_text().strip(), []).append((path.name, verdict.startswith("EXACT")))
    keys = ("input_tokens", "cache_creation_input_tokens", "cache_read_input_tokens", "output_tokens")
    usage = {}
    for path in (Path.home() / ".claude/projects").glob("*/*/subagents/agent-*.jsonl"):
        ident, last, model = None, {}, "?"
        for line in path.open(errors="replace"):
            row = json.loads(line)
            message = row.get("message") or {}
            if ident is None and row.get("type") == "user":
                found = re.search(rf"WAVE={re.escape(wave['name'])} ID=(\w+)", json.dumps(message.get("content")))
                if not found:
                    break
                ident = found.group(1)
            if row.get("type") == "assistant" and message.get("usage"):
                last[message.get("id")] = message["usage"]    # streamed rows repeat a message
                model = message.get("model", model)
        if ident:
            total = usage.setdefault(ident, [model, {k: 0 for k in keys}])[1]
            for u in last.values():
                for k in keys:
                    total[k] += u.get(k) or 0
    print(f"{'worker':8} {'model':24} {'handled':>7} {'exact':>5} {'bytes':>6} {'input':>10} {'output':>8}")
    by_model = {}
    for ident in sorted(set(mine) | set(usage)):
        model, t = usage.get(ident, ("(no transcript)", {k: 0 for k in keys}))
        done = mine.get(ident, [])
        exact = [n for n, e in done if e]
        nbytes = sum(sizes.get(n, 0) for n in exact)
        read = t["input_tokens"] + t["cache_creation_input_tokens"] + t["cache_read_input_tokens"]
        print(f"{ident:8} {model:24} {len(done):7} {len(exact):5} {nbytes:6} {read:10} {t['output_tokens']:8}")
        m = by_model.setdefault(model, [0, 0, 0, 0, 0])
        for i, v in enumerate((len(done), len(exact), nbytes, read, t["output_tokens"])):
            m[i] += v
    for model, (done, exact, nbytes, read, out) in by_model.items():
        per = f"{read // exact:,} input tokens per match" if exact else "no match"
        print(f"{model}: {exact} of {done} exact, {nbytes} bytes, {per}, {out:,} output tokens")


def status(args) -> None:
    wave = load(args.name)
    rows = results(wave)
    print(f"wave {wave['name']}: {wave['role']}, budget {wave['budget']}, pool {wave.get('pool', 'exe')}")
    for name, verdict, candidate, runs in rows:
        print(f"  {name}  {verdict[:40]:40}  runs {runs:>2}/{wave['budget']}  {candidate}")
    exact = sum(v.startswith("EXACT") for _, v, _, _ in rows)
    pending = sum(v == "(no result yet)" for _, v, _, _ in rows)
    print(f"{exact} exact, {len(rows) - exact - pending} not exact, {pending} pending")


def integrate(args) -> None:
    wave = load(args.name)
    exact = [(n, c) for n, v, c, _ in results(wave) if v.startswith("EXACT") and c]
    if not exact:
        sys.exit("no EXACT results to integrate")
    manifest = WAVES / f"{args.name}.MANIFEST"
    manifest.write_text("".join(f"{n} {c}\n" for n, c in exact))
    subprocess.run(["bash", "tools/docker/run.sh", "python", "tools/integrate.py",
                    str(manifest.relative_to(ROOT)), "--apply"], cwd=ROOT, check=True)
    print("\nNext: bash tools/docker/run.sh bash tools/build_sn.sh, then the progress report and a commit.")


TRAILER = "Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>"


def docker(*command: str) -> subprocess.CompletedProcess:
    if sys.platform == "win32":
        # Windows runs the toolchain natively (README.md); no container.
        if command[0] == "python":
            command = (sys.executable, *command[1:])
        return subprocess.run(list(command), cwd=ROOT, capture_output=True, text=True)
    return subprocess.run(["bash", "tools/docker/run.sh", *command], cwd=ROOT,
                          capture_output=True, text=True)


def exact_in_report(exe_only: bool = False) -> set[str]:
    report = json.loads((ROOT / "progress/report.json").read_text())
    return {f["name"] for u in report["units"] for f in u.get("functions", [])
            if (f.get("fuzzy_match_percent") or 0) == 100
            and not (exe_only and "level_code" in (u.get("metadata") or {}).get("progress_categories", []))}


def land(args) -> None:
    wave = load(args.name)
    if is_overlay_wave(wave["functions"]):
        land_overlay(args, wave)
    else:
        land_exe(args, wave)


def land_exe(args, wave: dict) -> None:
    # Only src/ and the report are committed, so only they must be clean.
    dirty = subprocess.run(["git", "status", "--porcelain", "--untracked-files=no", "--", "src", "progress"],
                           cwd=ROOT, capture_output=True, text=True).stdout.strip()
    if dirty and not args.batch:        # --batch: the landing lock guards writes; the lead commits
        sys.exit("land needs src/ and progress/ clean:\n" + dirty)
    rows = {r["name"]: r for r in triage.triage()}
    landed, skipped = [], []
    before = len(exact_in_report(exe_only=True)) if args.batch else 0
    for name, verdict, candidate, _ in results(wave):
        if not verdict.startswith("EXACT") or not candidate:
            continue
        exact = exact_in_report()
        if name in exact:
            skipped.append((name, "already exact"))
            continue
        reason = checker.banned(str(ROOT / candidate))
        if reason:
            skipped.append((name, f"refused: {reason}"))
            continue
        source = ROOT / "src" / f"{rows[name]['unit']}.c"
        saved = source.read_text()
        manifest = WAVES / f"{args.name}-{name}.MANIFEST"
        manifest.write_text(f"{name} {candidate}\n")
        if args.batch:
            owner = f"land:{args.name}"
            if not shared.lock(owner):
                skipped.append((name, "landing lock busy"))
                continue
            try:
                saved = source.read_text()
                applied = docker("python", "tools/integrate.py", str(manifest.relative_to(ROOT)), "--apply",
                                 "--lock-owner", owner)
                if "1/1 exact" not in applied.stdout:
                    if source.read_text() != saved:
                        source.write_text(saved)
                    skipped.append((name, "not exact on re-check"))
                    continue
                if re.search(rf"INCLUDE_ASM\([^)]*\b{name}\)", source.read_text()):
                    skipped.append((name, "exact, but not applied (a definition under an alias?)"))
                    continue
            finally:
                shared.unlock(owner)
            landed.append(name)
            print(f"landed {name}", flush=True)
            continue
        applied = docker("python", "tools/integrate.py", str(manifest.relative_to(ROOT)), "--apply")
        if "1/1 exact" not in applied.stdout:
            source.write_text(saved)
            skipped.append((name, "not exact on re-check"))
            continue
        build = docker("bash", "tools/build_sn.sh")
        (WAVES / f"{args.name}-{name}.build.log").write_text(build.stdout + build.stderr)
        count = re.search(r"exact \(size AND bytes\):\s*(\d+)", build.stdout)
        mismatch = re.search(r"size mismatch:\s*(\d+)", build.stdout)
        if build.returncode or not count or int(count.group(1)) != len(exact) + 1 \
                or not mismatch or int(mismatch.group(1)):
            source.write_text(saved)
            skipped.append((name, "full build did not confirm it; source restored"))
            continue
        docker("python", "tools/gen_progress_report.py", "--no-build")
        row = rows[name]
        title = f"{row['symbol']} ({name})" if row["symbol"] else name
        message = (f"feat({SCOPE}{row['unit'].split('/')[0]}): {title} exact match\n\n"
                   f"Matched by a Sonnet worker in wave {args.name}; full build audited.\n\n{TRAILER}")
        subprocess.run(["git", "add", str(source.relative_to(ROOT)), "progress/report.json"], cwd=ROOT, check=True)
        subprocess.run(["git", "commit", "-q", "-m", message], cwd=ROOT, check=True)
        landed.append(name)
        print(f"landed {name}: {len(exact) + 1} exact", flush=True)
    for name, why in skipped:
        print(f"skipped {name}: {why}")
    if args.batch and landed:
        # One full build confirms the whole batch: every landed function
        # exact in the linked executable, nothing else moved.
        build = docker("bash", "tools/build_sn.sh")
        (WAVES / f"{args.name}.build.log").write_text(build.stdout + build.stderr)
        count = re.search(r"exact \(size AND bytes\):\s*(\d+)", build.stdout)
        mismatch = re.search(r"size mismatch:\s*(\d+)", build.stdout)
        ok = not build.returncode and count and int(count.group(1)) == before + len(landed) \
            and mismatch and not int(mismatch.group(1))
        print(f"full build: {count.group(1) if count else '?'} exact (expected {before + len(landed)}), "
              f"{mismatch.group(1) if mismatch else '?'} size mismatches: "
              + ("confirmed" if ok else f"NOT confirmed, see {args.name}.build.log"))
    print(f"{len(landed)} landed, {len(skipped)} skipped")


def overlay_known_name(saved_text: str, name: str) -> str | None:
    """A trailing name comment on NAME's stub line (`INCLUDE_ASM(...); /*
    Name */`), if one is there -- the same convention tools/triage.py reads
    for executable stubs; overlay stubs don't carry one yet as generated,
    but a worker or a future catalogue update may leave one the same way."""
    for fn, sym in OVERLAY_NAME_COMMENT.findall(saved_text):
        if fn == name:
            return sym
    return None


def overlay_source_of(name: str, findex) -> Path | None:
    entry = findex.get(name)
    return entry[0] if entry else None


def overlay_is_stub(source: Path, name: str) -> bool:
    for line in source.read_text(errors="replace").splitlines():
        if name in line and OVERLAY_STUB_LINE.match(line):
            return True
    return False


def extract_definition(source: Path, name: str) -> str:
    """NAME's C definition exactly as it stands in SOURCE right now, for
    re-checking the landed file itself rather than the pre-apply candidate
    (apply_candidate.py's own edits -- dropped externs, a moved comment --
    are text changes only, but this is the same belt-and-braces the
    executable land() gets from its full build, which overlay code has
    none of)."""
    lines = srcnames.read_c(source).splitlines()
    for i, line in enumerate(lines):
        if name in line and OVERLAY_DEF_LINE.match(line) and not line.rstrip().endswith(";"):
            depth, seen, j = 0, False, i
            for j in range(i, len(lines)):
                depth += lines[j].count("{") - lines[j].count("}")
                seen = seen or "{" in lines[j]
                if seen and depth == 0:
                    break
            return "\n".join(lines[i:j + 1]) + "\n"
    sys.exit(f"{name}: no C definition found in {source} right after landing it")


def land_overlay(args, wave: dict) -> None:
    """Lands overlay EXACTs (docs/OVERLAYS.md). Differs from land_exe():

    - No full build: the executable does not link src/overlays/, so
      tools/build_sn.sh cannot see these functions either way.
    - The re-check after applying is tools/try_func.py itself, run again
      against the function's own definition as it now sits in the landed
      file (overlay_check.check() already does the strict, unmasked
      relocation compare -- see docs/OVERLAYS.md's "Plan" step 2).
    - progress/report.json has no overlay units yet (another agent is
      adding them to tools/gen_progress_report.py); see the TODO below.
    """
    dirty = subprocess.run(["git", "status", "--porcelain", "--untracked-files=no", "--", "src", "progress"],
                           cwd=ROOT, capture_output=True, text=True).stdout.strip()
    if dirty and not args.batch:        # --batch: the landing lock guards writes; other agents may be mid-work
        sys.exit("land needs src/ and progress/ clean:\n" + dirty)
    findex = dossier.overlay_file_index()
    landed, skipped = [], []
    batch = []      # --batch: (name, candidate, source), landed together below
    for name, verdict, candidate, _ in results(wave):
        if not verdict.startswith("EXACT") or not candidate or name in args.reject:
            continue
        source = overlay_source_of(name, findex)
        if source is None:
            skipped.append((name, "not found under src/overlays/"))
            continue
        if not overlay_is_stub(source, name):
            skipped.append((name, "already landed (no longer a stub)"))
            continue
        reason = checker.banned(str(ROOT / candidate))
        if reason:
            skipped.append((name, f"refused: {reason}"))
            continue
        if args.batch:
            batch.append((name, candidate, source))
            continue
        owner = f"land:{args.name}"
        shared.lock(owner)                  # other agents write src/ too (tools/claims.py)
        try:
            outcome = land_one(args, name, candidate, source, owner)
        finally:
            shared.unlock(owner)
            time.sleep(3)       # let a waiting agent take the lock before the next function
        if outcome != "landed":
            skipped.append((name, outcome))
            continue
        landed.append(name)
        print(f"landed {name}", flush=True)
    if batch:
        done, failed = land_batch(args, batch)
        landed += done
        skipped += failed
    unstage(landed)
    freed = [n for n, v, _, _ in results(wave)
             if not v.startswith("EXACT") and (claims(wave) / n).exists()
             and shared.release(f"{args.name}:{(claims(wave) / n).read_text().strip()}", n)]
    for name, why in skipped:
        print(f"skipped {name}: {why}")
    print(f"{len(landed)} landed, {len(skipped)} skipped; {len(freed)} unmatched claims released")


def long_setup(args) -> None:
    """Long-function workers: see the module docstring."""
    catalogue = dossier.load_overlay_catalogue()
    for name in args.funcs:
        asm = ROOT / "asm/overlays" / f"{name}.s"
        if not OVERLAY_NAME.match(name) or not asm.exists():
            sys.exit(f"{name}: not a level function with assembly")
        verdict, why, _ = rank_candidates.classify(name, asm.read_text(errors="replace"), "text",
                                                   catalogue[name][1])
        if verdict == "blocked":
            sys.exit(f"{name}: blocked ({why}); pick another")
        if not shared.claim(f"{args.name}:{args.arm}", name):
            sys.exit(f"{name}: taken by {shared.holder(name)}")
    for name in args.funcs:
        (TRY / name).mkdir(parents=True, exist_ok=True)
    loop = "; ".join(f"python tools/m2c.py {n} > build-sn/try/{n}/m2c.c 2>&1" for n in args.funcs)
    docker("sh", "-c", loop)
    plan(argparse.Namespace(name=args.name, funcs=args.funcs, role="match", count=len(args.funcs),
                            budget=args.budget, near=False, fresh=False, overlay=True, queue=True,
                            family=False, min_size=0, max_size=10 ** 9, near_max=NEAR_MAX), quiet=True)
    for name in args.funcs:
        work = TRY / name
        (work / "PACKET.md").write_text(packet(name, args.budget))
        (work / args.arm).mkdir(exist_ok=True)
        (work / args.arm / "BUDGET").write_text(f"{args.budget}\n")
        print(f"Read docs/LONG_FUNCTIONS.md and follow it exactly. FUNC={name} ARM={args.arm}.")


def stage(args) -> None:
    """Shares near misses: see the module docstring."""
    import nonmatching
    findex = dossier.overlay_file_index()
    names = args.funcs or [n for n, _ in overlay_stubs(findex)
                           if args.level is None or n.startswith(f"func_L{args.level:02d}_")]
    jobs = []
    for name in names:
        if not (TRY / name).is_dir() and name not in nonmatching.staged():
            continue
        found = best_logged(name)
        if found is None or found[0] > args.max or "nonmatching/" in found[2]:
            continue                    # nothing close enough, or the staged one is already the closest
        jobs.append(f"{name}={Path(found[2]).relative_to(ROOT)}")
    if not jobs:
        sys.exit("nothing new to stage")
    out = docker("python", "tools/nonmatching.py", "stage", *jobs)
    print(out.stdout + out.stderr)


def unstage(names) -> None:
    """Landed functions' near misses are done: remove their staged files."""
    import nonmatching
    files = nonmatching.staged()
    gone = [files[n] for n in names if n in files]
    for path in gone:
        path.unlink()
    if gone:
        nonmatching.index()


def salvage(args) -> None:
    """Batch-lands every overlay stub with an EXACT run logged (see the
    module docstring); ports crediting Lombyte only with --ports."""
    findex = dossier.overlay_file_index()
    batch, skipped = [], []
    for name, source in overlay_stubs(findex):
        if args.level is not None and not name.startswith(f"func_L{args.level:02d}_"):
            continue
        candidate = logged_exact(name) if name not in args.reject else None
        if candidate is None:
            continue
        if (CREDIT in (ROOT / candidate).read_text(errors="replace")) != args.ports:
            continue
        reason = checker.banned(str(ROOT / candidate))
        if reason:
            skipped.append((name, f"refused: {reason}"))
            continue
        batch.append((name, candidate, source))
    if not batch:
        sys.exit("nothing to salvage")
    print(f"salvaging {len(batch)}: {' '.join(n for n, _, _ in batch)}", flush=True)
    # A name of its own per level, so salvages of several levels run at once
    # (their own manifests, logs and lock owner).
    level = f"-L{args.level:02d}" if args.level is not None else ""
    args.name, args.batch = ("salvage-ports" if args.ports else "salvage") + level, True
    landed, failed = land_batch(args, batch)
    unstage(landed)
    for name, why in skipped + failed:
        print(f"skipped {name}: {why}")
    print(f"{len(landed)} landed, {len(skipped) + len(failed)} skipped")


BUSY_HOURS = 12     # a claim younger than this, on a function still in assembly, is a worker at work
PARALLEL = 4        # files landed one by one at once (each build is a Docker run)


def busy(source: Path, own: str) -> str | None:
    """Why SOURCE must not be written now, or None: a function still
    INCLUDE_ASM in it is claimed by a running worker that isn't OWN's
    (an owner prefix such as "q7:"). try_func builds a scratch copy of the
    whole file, so a landing mid-edit breaks that worker's builds."""
    for line in source.read_text(errors="replace").splitlines():
        m = re.search(r"INCLUDE_ASM\([^)]*\b(func_L\d{2}_[0-9A-Fa-f]{8})\)", line)
        if not m:
            continue
        owner = shared.holder(m.group(1))
        if not owner or (own and owner.startswith(own)):
            continue
        claim_file = shared.path(m.group(1))
        if time.time() - claim_file.stat().st_mtime < BUSY_HOURS * 3600:
            return f"{source.name} is busy: {owner} holds {m.group(1)}"
    return None


def land_batch(args, batch: list) -> tuple[list[str], list[tuple[str, str]]]:
    """Lands BATCH (name, candidate, source) together: every candidate
    applied in one integrate run under the landing lock, then every touched
    file rebuilt and every C function in it checked strictly in one
    parallel tools/overlay_file_check.py run. A file with any problem is put
    back (if nobody wrote it meanwhile) and its candidates land one by one
    through land_one(), which also handles prototype clashes."""
    owner = f"land:{args.name}"
    held = {}
    for _, _, source in batch:
        if source not in held:
            held[source] = busy(source, f"{args.name}:")
    skipped_busy = [(n, held[s]) for n, _, s in batch if held[s]]
    batch = [b for b in batch if not held[b[2]]]
    if not batch:
        return [], skipped_busy
    files = sorted({source for _, _, source in batch})
    manifest = WAVES / f"{args.name}.batch.MANIFEST"
    manifest.write_text("".join(f"{name} {cand}\n" for name, cand, _ in batch))
    shared.lock(owner)
    try:
        saved = {f: f.read_text() for f in files}
        applied = docker("python", "tools/integrate.py", str(manifest.relative_to(ROOT)), "--apply", "--trust",
                         "--lock-owner", owner)
        (WAVES / f"{args.name}.batch.apply.log").write_text(applied.stdout + applied.stderr)
        written = {f: f.read_text() for f in files}
    finally:
        shared.unlock(owner)
    check = docker("python", "tools/overlay_file_check.py", *(str(f.relative_to(ROOT)) for f in files))
    (WAVES / f"{args.name}.batch.check.log").write_text(check.stdout + check.stderr)
    bad = {f for f in files if re.search(rf"(BUILD FAIL|NOT EXACT) {re.escape(str(f.relative_to(ROOT)))}:", check.stdout)}
    landed, skipped = [], []
    for name, cand, source in batch:
        if source in bad:
            continue
        if overlay_is_stub(source, name):
            skipped.append((name, "not applied (see the batch apply log)"))
        else:
            landed.append(name)
            print(f"landed {name}", flush=True)
    def one_by_one(f: Path) -> tuple[list[str], list[tuple[str, str]]]:
        # Each file on its own thread with its own lock owner: land_one
        # takes the lock only to write, so the files' builds and checks run
        # side by side, while one file's candidates still go in one at a time.
        mine = f"{owner}:{f.stem}"
        done, refused = [], []
        shared.lock(mine)
        try:
            if f.read_text() != written[f]:
                return [], [(n, f"{f.name} changed meanwhile; left as it is, check it")
                            for n, _, s in batch if s == f]
            f.write_text(saved[f])
        finally:
            shared.unlock(mine)
        for name, cand, source in batch:
            if source != f:
                continue
            outcome = land_one(args, name, cand, source, mine)
            if outcome == "landed":
                done.append(name)
                print(f"landed {name} (one by one)", flush=True)
            else:
                refused.append((name, outcome))
        return done, refused

    with ThreadPoolExecutor(max_workers=PARALLEL) as pool:
        for done, refused in pool.map(one_by_one, sorted(bad)):
            landed += done
            skipped += refused
    return landed, skipped + skipped_busy


def keep_own_types(text: str, names: set, suffix: str) -> str:
    """TEXT with each of NAMES renamed NAME_SUFFIX: a typedef or struct tag
    simply renamed, an extern also given __asm__("NAME") so it still means
    the same symbol (one that already has an alias keeps its own)."""
    for sym in sorted(names, key=len, reverse=True):
        new = f"{sym}_{suffix}"
        if re.search(rf"\b{new}\b", text):
            continue
        lines = []
        for line in text.splitlines(keepends=True):
            if line.lstrip().startswith("extern") and re.search(rf"\b{sym}\b", line) and "__asm__" not in line:
                line = re.sub(rf"\b{sym}\b", new, line, count=1)
                head, sep, tail = line.rpartition(";")
                attrs = re.search(r"\s(MACRO_ADDR|__attribute__\(\(.*\)\))\s*$", head)
                if attrs:
                    head = head[:attrs.start()] + f' __asm__("{sym}")' + head[attrs.start():]
                else:
                    head += f' __asm__("{sym}")'
                line = head + sep + tail
            else:
                line = re.sub(rf"\b{sym}\b", new, line)
            lines.append(line)
        text = "".join(lines)
    return text


def land_one(args, name: str, candidate: str, source: Path, owner: str) -> str:
    """Applies one overlay candidate and re-checks it in its file. Writes
    take the landing lock as OWNER (already held if the caller holds it),
    the builds and checks run without it. Returns "landed" or why not."""
    saved = source.read_text()
    known = overlay_known_name(saved, name)
    manifest = WAVES / f"{args.name}-{name}.MANIFEST"

    def apply(cand: str):
        manifest.write_text(f"{name} {cand}\n")
        return docker("python", "tools/integrate.py", str(manifest.relative_to(ROOT)), "--apply",
                      "--lock-owner", owner)

    def undo(why: str) -> str:
        # Put the file back only if what is there is still our own write:
        # an agent that edits without the lock must not lose its work.
        took = shared.lock(owner)
        try:
            if source.read_text() in (saved, written):
                source.write_text(saved)
                return why
        finally:
            if took:
                shared.unlock(owner)
        return why + f"; {source.name} changed meanwhile, left as it is: check it"

    original = candidate
    applied = apply(candidate)
    # A candidate written before its neighbours landed can redeclare a
    # function the file now defines or declares, with the type its author
    # guessed. Drop those externs (the file's declaration wins) and try
    # again; the strict check still decides.
    seen = set()
    for attempt in range(1, 4):
        log = TRY / name / "log.txt"
        clash = set(re.findall(r"(?:conflicting types for|redefinition of) `(\w+)'", log.read_text(errors="replace"))) \
            if "1/1 exact" not in applied.stdout and log.exists() else set()
        if not clash:
            break
        seen |= clash
        text = (ROOT / candidate).read_text()
        kept = [l for l in text.splitlines(keepends=True)
                if not (l.startswith("extern") and any(re.search(rf"\b{c}\b", l) for c in clash))]
        if len(kept) == len(text.splitlines()):
            break
        candidate = str((TRY / name / f"lead{attempt}.c").relative_to(ROOT))
        (ROOT / candidate).write_text("".join(kept))
        applied = apply(candidate)
    # The file's declaration can have other types than the ones the
    # candidate matched with. Then keep the candidate's own: rename its
    # typedefs and declare each clashing symbol under an alias naming that
    # one symbol (QUEUE.md, "Rules"), until nothing clashes.
    if "1/1 exact" not in applied.stdout and seen - {name}:
        text = (ROOT / original).read_text()
        for attempt in range(1, 5):
            text = keep_own_types(text, seen - {name}, name[-6:])
            candidate = str((TRY / name / f"alias{attempt}.c").relative_to(ROOT))
            (ROOT / candidate).write_text(text)
            applied = apply(candidate)
            log = TRY / name / "log.txt"
            clash = set(re.findall(r"(?:conflicting types for|redefinition of) `(\w+)'", log.read_text(errors="replace"))) \
                if "1/1 exact" not in applied.stdout and log.exists() else set()
            if not clash - seen - {name}:
                break
            seen |= clash
    written = source.read_text()
    if "1/1 exact" not in applied.stdout:
        return undo("not exact on re-check")
    recheck = WAVES / f"{args.name}-{name}.landed.c"
    try:
        recheck.write_text(extract_definition(source, name))
    except SystemExit:
        return undo("no C definition found in the file after applying (a definition split across lines?)")
    verify = docker("python", "tools/try_func.py", name, str(recheck.relative_to(ROOT)), "--no-budget")
    (WAVES / f"{args.name}-{name}.verify.log").write_text(verify.stdout + verify.stderr)
    if "EXACT" not in verify.stdout:
        return undo("not exact re-checked against the landed file")
    neighbours = docker("python", "tools/overlay_file_check.py", str(source.relative_to(ROOT)))
    (WAVES / f"{args.name}-{name}.file.log").write_text(neighbours.stdout + neighbours.stderr)
    if neighbours.returncode:
        return undo("the file's other C functions no longer all build EXACT with it")
    if args.batch:      # the lead regenerates the report and commits the batch
        return "landed"
    elf = ROOT / "build-sn/rac1.elf"
    if elf.exists():
        docker("python", "tools/gen_progress_report.py", "--no-build")
    title = f"{known} ({name})" if known else name
    message = (f"feat({SCOPE}overlays): {title} exact match\n\n"
               f"Matched by a Sonnet worker in wave {args.name}; checked strictly against "
               f"{name}'s real address in its level with tools/overlay_check.py "
               f"(docs/OVERLAYS.md).\n\n{TRAILER}")
    add = [str(source.relative_to(ROOT))] + (["progress/report.json"] if elf.exists() else [])
    subprocess.run(["git", "add", *add], cwd=ROOT, check=True)
    subprocess.run(["git", "commit", "-q", "-m", message], cwd=ROOT, check=True)
    return "landed"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    p = commands.add_parser("plan")
    p.add_argument("name")
    p.add_argument("funcs", nargs="*")
    p.add_argument("--role", choices=SECTION, default="match")
    p.add_argument("--count", type=int, default=10)
    p.add_argument("--budget", type=int, default=6)
    pick = p.add_mutually_exclusive_group()
    pick.add_argument("--near", action="store_true")
    pick.add_argument("--fresh", action="store_true")
    p.add_argument("--overlay", action="store_true")
    p.add_argument("--near-max", type=float, default=NEAR_MAX)
    p.add_argument("--queue", action="store_true")
    p.add_argument("--family", action="store_true")
    p.add_argument("--min-size", type=int, default=0)
    p.add_argument("--max-size", type=int, default=10 ** 9)
    c = commands.add_parser("claim")
    c.add_argument("name")
    c.add_argument("id")
    c.add_argument("--count", type=int, default=2)
    commands.add_parser("tokens").add_argument("name")
    commands.add_parser("status").add_argument("name")
    commands.add_parser("integrate").add_argument("name")
    l = commands.add_parser("land")
    l.add_argument("name")
    l.add_argument("--batch", action="store_true")
    l.add_argument("--reject", nargs="*", default=[])
    g = commands.add_parser("long")
    g.add_argument("name")
    g.add_argument("funcs", nargs="+")
    g.add_argument("--arm", default="opus")
    g.add_argument("--budget", type=int, default=30)
    st = commands.add_parser("stage")
    st.add_argument("funcs", nargs="*")
    st.add_argument("--level", type=int)
    st.add_argument("--max", type=float, default=FAR)
    s = commands.add_parser("salvage")
    s.add_argument("--ports", action="store_true")
    s.add_argument("--level", type=int)
    s.add_argument("--reject", nargs="*", default=[])
    args = parser.parse_args()
    {"plan": plan, "status": status, "integrate": integrate, "land": land,
     "claim": claim, "tokens": tokens, "salvage": salvage, "long": long_setup,
     "stage": stage}[args.command](args)


if __name__ == "__main__":
    main()

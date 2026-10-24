# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The command line:

  python3 tools/permuter list                               the profiles
  python3 tools/permuter fetch                              the patched decomp-permuter checkout
  python3 tools/permuter base PROFILE FUNC [--c DRAFT]      the base score, compiled and checked once
  python3 tools/permuter run PROFILE FUNC [--c DRAFT] [-j N] [--seconds S] [--out DIR] [--no-asm-passes]
  python3 tools/permuter score PROFILE FUNC OBJECT          aligned diffs of one object
  python3 tools/permuter queue ...                          the box runner (runner.py)

Results go to OUT (default build/permuter/<profile>/<func>/ in OpenRAC); see engine.py.
"""
from __future__ import annotations

import argparse
import multiprocessing as mp
import os
import sys
import tempfile
import time
from pathlib import Path

from openrac_common import paths

from . import engine, profile as profiles, score, upstream


def default_out(prof: profiles.Profile, func: str) -> Path:
    return paths.repo_root() / "build" / "permuter" / prof.name.replace("/", "-") / func


def _prepare(args) -> tuple[profiles.Profile, dict]:
    prof = profiles.load(args.profile)
    draft = Path(args.c).read_text(errors="replace") if args.c else None
    checkout = upstream.checkout()
    engine.run_start(prof)
    cfg = engine.prepare(prof, args.func, draft, checkout, asm_passes=not getattr(args, "no_asm_passes", False))
    return prof, cfg


def cmd_base(args) -> int:
    """Compile the base once and score it: what a run starts from, checked by the game's checker."""
    prof, cfg = _prepare(args)
    work = upstream.cache_dir() / "work"
    work.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="base-", dir=work) as tmp:
        cand = engine.Candidate(cfg, Path(tmp))
        obj = cand.compile(cfg["base"])
        if obj is None:
            print(f"{args.func}: the base does not compile\n{cand.last_error}")
            return 1
        n = score.diffs(score.from_object(Path(obj), args.func), cand.retail)
        exact = cand.exact()
        print(f"{args.func}: base score {n}; {prof.name}'s checker: {'EXACT' if exact else 'not exact'}")
        return 0 if (n == 0) == exact else 2


def cmd_run(args) -> int:
    prof, cfg = _prepare(args)
    out = Path(args.out) if args.out else default_out(prof, args.func)
    job = engine.Job(cfg, out, args.j, args.seconds).start()
    print(f"{args.func}: {prof.name}, flags {cfg['flags'] or '(the file’s own)'}; {job.jobs} workers; "
          f"asm passes {'on' if job.state['asm_passes'] else 'off'}; outputs in {out}", flush=True)
    seen = 0
    try:
        while job.thread.is_alive():
            time.sleep(1)
            s = job.snapshot(seen)
            for e in s["events"]:
                seen = e["seq"]
                if e["kind"] in ("found", "base", "fatal"):
                    extra = f" artefacts={e['artefacts']}" if e.get("artefacts") else ""
                    print(f"  [{e['t']:7.1f}s] {e['kind']} score={e.get('score')} pass={e.get('pass_')}{extra}"
                          + (f"\n{e['text']}" if e["kind"] == "fatal" else ""), flush=True)
            if not args.quiet:
                print(f"{s['status']:10s} {s['elapsed']:7.1f}s  iter {s['iterations']}  {s['rate']}/s  "
                      f"base {s['base_score']}  best {s['best_score']}", flush=True)
    except KeyboardInterrupt:
        job.stop()
        job.thread.join(15)
    s = job.snapshot()
    print(f"{args.func}: {s['status']}, base {s['base_score']}, best {s['best_score']}, "
          f"{s['iterations']} candidates in {s['elapsed']}s; {out / 'result.json'}")
    return 0 if s["status"] != "error" else 1


def cmd_score(args) -> int:
    prof = profiles.load(args.profile)
    name = profiles.retail_name(prof, args.func)
    retail = score.parse_asm(profiles.find_asm(prof, name).read_text(errors="replace"), name, prof.word_order)
    print(score.diffs(score.from_object(Path(args.object), args.func), retail))
    return 0


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(prog="tools/permuter", description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="command", required=True)
    sub.add_parser("list")
    sub.add_parser("fetch")
    for name in ("base", "run"):
        p = sub.add_parser(name)
        p.add_argument("profile")
        p.add_argument("func")
        p.add_argument("--c", help="a draft: the declarations it needs, then the function (default: the C "
                                   "the project has for it)")
        p.add_argument("--no-asm-passes", action="store_true", help="leave out the fence and barrier passes")
        if name == "run":
            p.add_argument("-j", type=int, default=max(1, (os.cpu_count() or 2) - 1))
            p.add_argument("--seconds", type=float, default=0, help="time limit (default none)")
            p.add_argument("--out")
            p.add_argument("--quiet", action="store_true", help="print events only, not a status line a second")
    p = sub.add_parser("score")
    p.add_argument("profile")
    p.add_argument("func")
    p.add_argument("object")
    p = sub.add_parser("queue", add_help=False)
    p.add_argument("rest", nargs=argparse.REMAINDER)
    args = ap.parse_args(argv)
    try:
        if args.command == "list":
            for name in profiles.names():
                prof = profiles.load(name)
                try:
                    where = prof.checkout
                except LookupError as e:
                    where = e
                print(f"{name:12s} {where}")
            return 0
        if args.command == "fetch":
            print(upstream.checkout())
            return 0
        if args.command == "queue":
            from . import runner
            return runner.main(args.rest)
        return {"base": cmd_base, "run": cmd_run, "score": cmd_score}[args.command](args)
    except (FileNotFoundError, KeyError, LookupError, RuntimeError, ValueError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    mp.freeze_support()
    sys.exit(main())

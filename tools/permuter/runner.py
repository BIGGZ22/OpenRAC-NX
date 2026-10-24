# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The queue runner for the build machines: one queue per machine, any game, a few runs at a time.

Generalised from the boxes' ~/Projects/RAC/permuter/run_queue.py (rac3 only, through
decomp-permuter's own command line): each item here is a `tools/permuter run` of any profile.

    python3 tools/permuter queue run [--dir D] [--slots N] [-j J] [--minutes M] [--max-load L]
    python3 tools/permuter queue add PROFILE FUNC [--c DRAFT] [--env NAME=VALUE ...] [--minutes M]
    python3 tools/permuter queue import-legacy OLD_PERMUTER_DIR --profile rac3/pal
    python3 tools/permuter queue status [--dir D]

The queue is a directory (--dir, default $OPENRAC_PERMUTER_QUEUE, else build/permuter/queue in
OpenRAC):

  queue.tsv   profile, func, draft (a path, or - for the project's own C), env (NAME=VALUE,...,
              or -), minutes (or - for --minutes). Taken in file order; # starts a comment.
  done.tsv    what ran: profile, func, base, best, verdict, seconds, result (its result.json).
              An item in done.tsv is not run again; delete its line to run it again.
  PAUSE       while it exists no item starts; running ones go on.
  STOP        ends the running items (they are not recorded, so they run again next time) and
              the runner.
  runs/       each item's outputs (engine.py): runs/<profile>/<func>/
  wins/       <profile>/<func>.c for each WIN: the function of a score 0 result the game's checker
              called EXACT and accept.py found no artefact in. It still needs a reader (README).

Verdicts: WIN; artefact (a 0 with artefacts, for a reader); better (best < base); none; error.

The CPU: every run is under `nice -n 10`; at most SLOTS runs of J workers each; no item starts
while the one-minute load average is at or above --max-load (default: the CPU count); a build
that needs the machine touches PAUSE (or STOP) and removes it after.
"""
from __future__ import annotations

import argparse
import json
import os
import signal
import subprocess
import sys
import time
from pathlib import Path

from openrac_common import paths, tables

COLUMNS = ("profile", "func", "draft", "env", "minutes")
DONE = ("profile", "func", "base", "best", "verdict", "seconds", "result")
ENTRY = Path(__file__).resolve().parent


def queue_dir(arg: str | None) -> Path:
    if arg:
        return Path(arg).expanduser()
    if os.environ.get("OPENRAC_PERMUTER_QUEUE"):
        return Path(os.environ["OPENRAC_PERMUTER_QUEUE"]).expanduser()
    return paths.repo_root() / "build" / "permuter" / "queue"


def items(d: Path) -> list[dict[str, str]]:
    return [row for row in tables.read_tsv(d / "queue.tsv", COLUMNS, strip=True) if row["profile"]]


def done_keys(d: Path) -> set[tuple[str, str]]:
    return {(r["profile"], r["func"]) for r in tables.read_tsv(d / "done.tsv", DONE, strip=True)}


def verdict(result: dict) -> str:
    if result.get("status") == "error" or result.get("base_score") is None:
        return "error"
    best = result.get("best") or {}
    if result.get("status") == "matched" and result.get("best_score") == 0:
        return "WIN" if not best.get("artefacts") else "artefact"
    if any(o["score"] == 0 for o in result.get("outputs", [])):
        return "artefact"
    return "better" if result["best_score"] < result["base_score"] else "none"


def record(d: Path, item: dict, result_path: Path, seconds: float) -> str:
    try:
        result = json.loads(result_path.read_text())
    except (OSError, ValueError):
        result = {"status": "error"}
    v = verdict(result)
    if v == "WIN":
        win = d / "wins" / item["profile"].replace("/", "-") / f"{item['func']}.c"
        win.parent.mkdir(parents=True, exist_ok=True)
        win.write_text(result["best"]["function"])
    row = [item["profile"], item["func"], str(result.get("base_score")), str(result.get("best_score")), v,
           str(int(seconds)), str(result_path)]
    new = not (d / "done.tsv").is_file()
    with open(d / "done.tsv", "a") as f:
        if new:
            f.write("\t".join(DONE) + "\n")
        f.write("\t".join(row) + "\n")
    return v


def start(d: Path, item: dict, jobs: int, minutes: float, log) -> tuple[subprocess.Popen, Path]:
    out = d / "runs" / item["profile"].replace("/", "-") / item["func"]
    minutes = float(item["minutes"]) if item.get("minutes") not in ("", "-", None) else minutes
    cmd = ["nice", "-n", "10", sys.executable, str(ENTRY), "run", item["profile"], item["func"],
           "-j", str(jobs), "--seconds", str(int(minutes * 60)), "--out", str(out), "--quiet"]
    if item.get("draft") not in ("", "-", None):
        cmd += ["--c", item["draft"]]
    env = dict(os.environ)
    if item.get("env") not in ("", "-", None):
        env.update(pair.split("=", 1) for pair in item["env"].split(","))
    out.mkdir(parents=True, exist_ok=True)
    pr = subprocess.Popen(cmd, env=env, stdout=open(out / "run.log", "w"), stderr=subprocess.STDOUT,
                          start_new_session=True)
    log(f"start {item['profile']} {item['func']} pid {pr.pid}")
    return pr, out / "result.json"


def run(args) -> int:
    d = queue_dir(args.dir)
    d.mkdir(parents=True, exist_ok=True)
    max_load = args.max_load or float(os.cpu_count() or 1)

    def log(text: str) -> None:
        print(time.strftime("%Y-%m-%d %H:%M:%S"), text, flush=True)

    running: dict[tuple[str, str], tuple[subprocess.Popen, Path, float, dict]] = {}
    log(f"queue {d}: {args.slots} slots x -j{args.j}, {args.minutes} min each, max load {max_load}")
    while True:
        if (d / "STOP").exists():
            for pr, _, _, _ in running.values():
                os.killpg(pr.pid, signal.SIGINT)
            for pr, _, _, _ in running.values():
                try:
                    pr.wait(30)
                except subprocess.TimeoutExpired:
                    os.killpg(pr.pid, signal.SIGKILL)
            log("STOP: running items ended, not recorded")
            return 0
        for key, (pr, result, began, item) in list(running.items()):
            if pr.poll() is not None:
                log(f"done {key[0]} {key[1]}: {record(d, item, result, time.time() - began)}")
                del running[key]
        finished = done_keys(d)
        waiting = [i for i in items(d) if (i["profile"], i["func"]) not in finished
                   and (i["profile"], i["func"]) not in running]
        if not waiting and not running:
            log("queue empty")
            return 0
        if (waiting and len(running) < args.slots and not (d / "PAUSE").exists()
                and os.getloadavg()[0] < max_load):
            item = waiting[0]
            pr, result = start(d, item, args.j, args.minutes, log)
            running[(item["profile"], item["func"])] = (pr, result, time.time(), item)
            time.sleep(args.stagger)
            continue
        time.sleep(5)


def add(args) -> int:
    d = queue_dir(args.dir)
    d.mkdir(parents=True, exist_ok=True)
    path = d / "queue.tsv"
    new = not path.is_file()
    draft = str(Path(args.c).resolve()) if args.c else "-"
    with open(path, "a") as f:
        if new:
            f.write("\t".join(COLUMNS) + "\n")
        f.write("\t".join([args.profile, args.func, draft, ",".join(args.env) or "-",
                           str(args.minutes) if args.minutes else "-"]) + "\n")
    return 0


def import_legacy(args) -> int:
    """The boxes' old queue (queue/queue.tsv: prog, func, diff, tier; drafts in queue/src/) as items
    of PROFILE with UYA_TARGET=prog, skipping what its done.tsv and skip.tsv list. The old queue
    is only read."""
    old = Path(args.old).expanduser() / "queue"
    skip = {(r[0], r[1]) for r in tables.read_tsv(old / "done.tsv") + tables.read_tsv(old / "skip.tsv") if len(r) > 1}
    d = queue_dir(args.dir)
    d.mkdir(parents=True, exist_ok=True)
    path = d / "queue.tsv"
    have = {(i["profile"], i["func"]) for i in items(d)}
    rows = []
    for r in tables.read_tsv(old / "queue.tsv"):
        if len(r) < 2 or (r[0], r[1]) in skip or (args.profile, r[1]) in have:
            continue
        draft = old / "src" / f"{r[0]}__{r[1]}.c"
        rows.append([args.profile, r[1], str(draft) if draft.is_file() else "-", f"UYA_TARGET={r[0]}", "-"])
    new = not path.is_file()
    with open(path, "a") as f:
        if new:
            f.write("\t".join(COLUMNS) + "\n")
        for row in rows:
            f.write("\t".join(row) + "\n")
    print(f"{len(rows)} items from {old} into {path}")
    return 0


def status(args) -> int:
    d = queue_dir(args.dir)
    finished = tables.read_tsv(d / "done.tsv", DONE, strip=True)
    keys = {(r["profile"], r["func"]) for r in finished}
    waiting = [i for i in items(d) if (i["profile"], i["func"]) not in keys]
    counts: dict[str, int] = {}
    for r in finished:
        counts[r["verdict"]] = counts.get(r["verdict"], 0) + 1
    state = "STOP" if (d / "STOP").exists() else "PAUSE" if (d / "PAUSE").exists() else "open"
    print(f"{d}: {state}; {len(waiting)} waiting, {len(finished)} done "
          + " ".join(f"{k}={v}" for k, v in sorted(counts.items())))
    return 0


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(prog="tools/permuter queue", description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="command", required=True)
    p = sub.add_parser("run")
    p.add_argument("--slots", type=int, default=max(1, (os.cpu_count() or 4) // 4))
    p.add_argument("-j", type=int, default=4)
    p.add_argument("--minutes", type=float, default=20)
    p.add_argument("--max-load", type=float, default=0)
    p.add_argument("--stagger", type=float, default=10, help="seconds between two starts")
    p = sub.add_parser("add")
    p.add_argument("profile")
    p.add_argument("func")
    p.add_argument("--c")
    p.add_argument("--env", action="append", default=[])
    p.add_argument("--minutes", type=float)
    p = sub.add_parser("import-legacy")
    p.add_argument("old")
    p.add_argument("--profile", default="rac3/pal")
    sub.add_parser("status")
    for p in sub.choices.values():
        p.add_argument("--dir")
    args = ap.parse_args(argv)
    return {"run": run, "add": add, "import-legacy": import_legacy, "status": status}[args.command](args)

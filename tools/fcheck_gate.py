#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
The gate for backfilled (functional) C: a function counts as done only when
fcheck says SAME on real calls of the retail game; one fcheck cannot reach is
"unchecked", never done (docs/workflow/BACKFILL.md).

  /opt/homebrew/bin/python3 tools/fcheck_gate.py run                   PAL: check every row of games/rac1/nonmatching/functional.tsv
  /opt/homebrew/bin/python3 tools/fcheck_gate.py run --only func_001F7E98 ...
  /opt/homebrew/bin/python3 tools/fcheck_gate.py report [--strict]     summary of the verdicts already in the tables
  /opt/homebrew/bin/python3 tools/fcheck_gate.py us                    US: record "unchecked (no US oracle)" in port/game/rac1-ntsc/hand/fcheck.tsv

`run` reads the backfill table and drives rac1-decomp's tools/fcheck.py (test-only: it runs the retail
code in the interpreter build of OpenRAC, openrac-fcheck, and compares what
our C leaves in memory and returns). For each row it copies the C into
rac1-decomp's build-sn/fcheck/gate/ (fcheck wants the file under its own
tree and the table's copy is the one that ships), captures call states for
the functions that have none yet in one headless run, then checks the rest in
parallel. Nothing in rac1-decomp is edited besides its git-ignored build-sn/.

Verdicts, written to OpenRAC-owned files keyed by function name (the backfill
tables themselves, synced from rac1-decomp or read by hostgen, are never
touched): port/game/rac1-pal/fcheck.tsv for functional.tsv and
port/game/rac1-ntsc/hand/fcheck.tsv for hand.tsv. Columns:

  status  same            fcheck compared real calls and everything matched
          differs         fcheck found a difference (the log says where)
          unchecked       fcheck could not compare (compile failure, every
                          state skipped, no US oracle); the note says why
          no-states       the capture run (title, menu, level 0) never calls it
          not-executable  not a func_XXXXXXXX function of the boot executable
                          (fcheck runs those only); level code is here
  date    the day it was run (YYYY-MM-DD)
  fcheck  rac1-decomp's commit that fcheck ran from (+dirty if tools/fcheck.py
          had local edits)
  csum    first 12 hex digits of the sha1 of the C file, so a later edit shows
          the verdict as stale (report lists those)
  note    free text: differing lines, the reason for unchecked

Environment: OPENRAC_ROOT is the OpenRAC checkout that has build/runtime/
openrac-fcheck and openrac-boot built (default ~/Projects/RAC/OpenRAC);
OPENRAC_DECOMP is rac1-decomp (default ~/Projects/RAC/rac1-decomp).
Headless only: fcheck opens no window.

  python3 -m unittest tools/test_fcheck_gate.py
"""
from __future__ import annotations

import argparse
import concurrent.futures
import datetime
import hashlib
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PAL_TABLE = ROOT / "games/rac1/nonmatching/functional.tsv"
US_TABLE = ROOT / "port/game/rac1-ntsc/hand/hand.tsv"
PAL_VERDICTS = ROOT / "port/game/rac1-pal/fcheck.tsv"
US_VERDICTS = ROOT / "port/game/rac1-ntsc/hand/fcheck.tsv"
PYTHON = "/opt/homebrew/bin/python3"
HEADER = "name\tstatus\tdate\tfcheck\tcsum\tnote"
HEAD_COMMENT = ("# fcheck verdicts for the rows of {table} (tools/fcheck_gate.py writes this; "
                "docs/workflow/BACKFILL.md, \"The gate\"). Only status same is done.")
STATUSES = ("same", "differs", "unchecked", "no-states", "not-executable")
EXECUTABLE = re.compile(r"func_[0-9A-F]{8}")
NO_US_ORACLE = "no US oracle (fcheck has no interpreter build for the NTSC-U disc)"


# --- The tables -------------------------------------------------------------------
#
# The backfill tables (functional.tsv, hand.tsv) are read, never written: the
# first is synced from rac1-decomp. The verdicts live in OpenRAC-owned files
# beside them, keyed by function name.

class Backfill:
    """A backfill table: name -> file (two columns, # comments)."""

    def __init__(self, path: Path):
        self.path = path
        self.files: dict[str, str] = {}
        for line in path.read_text().splitlines():
            if line.strip() and not line.startswith("#"):
                c = line.split("\t")
                if len(c) > 1:
                    self.files[c[0].strip()] = c[1].strip()

    def source(self, name: str) -> Path:
        """The C file of a row: relative to the table, or (PAL) to the decompilation."""
        f = self.files[name]
        here = self.path.parent / f
        return here if here.is_file() else self.path.parent.parent / f


class Row:
    """One verdict: name, status, date, fcheck commit, csum, note."""

    def __init__(self, name: str, status: str = "", date: str = "", fcheck: str = "",
                 csum: str = "", note: str = ""):
        self.name, self.status, self.date = name, status, date
        self.fcheck, self.csum, self.note = fcheck, csum, note

    @classmethod
    def parse(cls, line: str) -> "Row":
        c = line.rstrip("\n").split("\t")
        c += [""] * (6 - len(c))
        return cls(*[x.strip() for x in c[:6]])

    def format(self) -> str:
        return "\t".join([self.name, self.status, self.date, self.fcheck, self.csum, self.note.replace("\t", " ")]).rstrip("\t")


class Verdicts:
    """The verdict file for one backfill table, keyed by function name."""

    def __init__(self, path: Path, backfill: Backfill):
        self.path, self.backfill = path, backfill
        self.rows: dict[str, Row] = {}
        if path.is_file():
            for line in path.read_text().splitlines():
                if line.strip() and not line.startswith("#"):
                    r = Row.parse(line)
                    self.rows[r.name] = r

    def row(self, name: str) -> Row:
        return self.rows.setdefault(name, Row(name))

    def ordered(self) -> list[Row]:
        """One row per backfill row, in its order (a verdict without a row is dropped)."""
        return [self.rows.get(n) or Row(n) for n in self.backfill.files]

    def write(self) -> None:
        out = [HEAD_COMMENT.format(table=self.backfill.path.name), "# " + HEADER]
        out += [r.format() for r in self.ordered() if r.status]
        self.path.write_text("\n".join(out) + "\n")


def csum(path: Path) -> str:
    return hashlib.sha1(path.read_bytes()).hexdigest()[:12] if path.is_file() else ""


# --- Running fcheck ---------------------------------------------------------------

def decomp_root() -> Path:
    return Path(os.environ.get("OPENRAC_DECOMP", str(Path.home() / "Projects/RAC/rac1-decomp")))


def fcheck_commit(decomp: Path) -> str:
    def git(*a: str) -> str:
        return subprocess.run(["git", "-C", str(decomp), *a], capture_output=True, text=True).stdout.strip()
    rev = git("rev-parse", "--short=8", "HEAD")
    return rev + ("+dirty" if git("status", "--porcelain", "--", "tools/fcheck.py") else "")


def fcheck_env() -> dict:
    env = dict(os.environ)
    env.setdefault("OPENRAC_ROOT", str(Path.home() / "Projects/RAC/OpenRAC"))
    return env


def classify(rc: int, output: str) -> tuple[str, str]:
    """fcheck's exit code and text -> (status, note). 0 means SAME; 3 the
    function was never called in the capture run; 4 every state was skipped;
    2 the candidate did not build or place; anything else is a difference."""
    lines = [l for l in output.strip().splitlines() if l.strip()]
    if rc == 0:
        m = re.search(r"SAME on (\d+) real calls", output)
        return "same", f"{m.group(1)} calls" if m else ""
    if rc == 3:
        return "no-states", "never called in the capture run (title, menu, level 0)"
    if rc == 4:
        return "unchecked", "every captured state was skipped (nothing compared)"
    if rc == 2:
        return "unchecked", "did not build or place: " + (lines[-1] if lines else "?")[:160]
    diff = [l for l in lines if "SAME" not in l and not l.startswith("compared")]
    return "differs", "; ".join(diff[:2])[:200] or f"exit {rc}"


def run_one(decomp: Path, name: str, src: Path, log_dir: Path) -> tuple[str, str]:
    cand = decomp / "build-sn/fcheck/gate" / f"{name}.c"
    cand.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src, cand)
    r = subprocess.run([PYTHON, "tools/fcheck.py", name, str(cand.relative_to(decomp))],
                       cwd=decomp, capture_output=True, text=True, env=fcheck_env())
    text = r.stdout + r.stderr
    log_dir.mkdir(parents=True, exist_ok=True)
    (log_dir / f"{name}.log").write_text(text)
    return classify(r.returncode, text)


def capture_missing(decomp: Path, names: list[str]) -> None:
    """One headless capture run for every function with neither states nor a
    "never called" marker (fcheck's own `capture` form)."""
    states = decomp / "build-sn/fcheck/states"
    todo = [n for n in names if not list(states.glob(f"{n}.*.snap")) and not (states / f"{n}.none").exists()]
    if not todo:
        return
    lst = decomp / "build-sn/fcheck/gate_capture_list.txt"
    lst.write_text("\n".join(todo) + "\n")
    print(f"capturing call states for {len(todo)} functions (one headless run) ...", flush=True)
    subprocess.run([PYTHON, "tools/fcheck.py", "capture", str(lst.relative_to(decomp))],
                   cwd=decomp, capture_output=True, text=True, env=fcheck_env())


def summary(rows: list[Row]) -> str:
    counts = {s: 0 for s in STATUSES}
    counts[""] = 0
    for r in rows:
        counts[r.status if r.status in STATUSES else ""] += 1
    parts = [f"{counts[s]} {s}" for s in STATUSES] + ([f"{counts['']} no verdict"] if counts[""] else [])
    return f"{len(rows)} rows: " + ", ".join(parts)


def cmd_run(args: argparse.Namespace) -> int:
    decomp = decomp_root()
    back = Backfill(PAL_TABLE)
    table = Verdicts(PAL_VERDICTS, back)
    commit = fcheck_commit(decomp)
    today = datetime.date.today().isoformat()
    names = [n for n in back.files if not args.only or n in args.only]
    work: list[Row] = []
    for n in names:
        r = table.row(n)
        src = back.source(n)
        r.csum, r.date, r.fcheck = csum(src), today, commit
        if not EXECUTABLE.fullmatch(n):
            r.status, r.note = "not-executable", "level/overlay function; fcheck runs boot-executable functions only"
        elif not src.is_file():
            r.status, r.note = "unchecked", f"file {back.files[n]} is missing"
        else:
            work.append(r)
    capture_missing(decomp, [r.name for r in work])
    log_dir = decomp / "build-sn/fcheck/gate/logs"
    print(f"checking {len(work)} functions with {args.jobs} workers ...", flush=True)
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        futs = {pool.submit(run_one, decomp, r.name, back.source(r.name), log_dir): r for r in work}
        for i, f in enumerate(concurrent.futures.as_completed(futs), 1):
            r = futs[f]
            try:
                r.status, r.note = f.result()
            except Exception as e:  # a crash of the harness is not a verdict on the C
                r.status, r.note = "unchecked", f"gate error: {e}"[:160]
            print(f"[{i}/{len(work)}] {r.name}: {r.status} {r.note}", flush=True)
    if not args.dry_run:
        table.write()
    print(summary(table.ordered()))
    listing(table.ordered())
    return 0


def listing(rows: list[Row]) -> None:
    for status in ("differs", "unchecked", "no-states"):
        names = [r for r in rows if r.status == status]
        if names:
            print(f"\n{status} ({len(names)}):")
            for r in names:
                print(f"  {r.name}  {r.note}")


def cmd_us(args: argparse.Namespace) -> int:
    back = Backfill(US_TABLE)
    table = Verdicts(US_VERDICTS, back)
    today = datetime.date.today().isoformat()
    for n in back.files:
        r = table.row(n)
        r.status, r.date, r.fcheck, r.csum, r.note = "unchecked", today, "", csum(back.source(n)), NO_US_ORACLE
    if not args.dry_run:
        table.write()
    print(summary(table.ordered()))
    return 0


def cmd_report(args: argparse.Namespace) -> int:
    """Join each backfill table with its verdict file."""
    bad = 0
    for tab, ver in ((PAL_TABLE, PAL_VERDICTS), (US_TABLE, US_VERDICTS)):
        back = Backfill(tab)
        rows = Verdicts(ver, back).ordered()
        print(f"{tab.relative_to(ROOT)} + {ver.relative_to(ROOT)}: {summary(rows)}")
        stale = [r for r in rows if r.csum and r.csum != csum(back.source(r.name))]
        for r in stale:
            print(f"  stale (file changed since the verdict): {r.name}")
        for r in rows:
            if r.status == "differs":
                print(f"  differs: {r.name}  {r.note}")
        bad += sum(1 for r in rows if r.status in ("", "differs")) + len(stale)
    return 1 if args.strict and bad else 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("run", help="PAL: run fcheck on every row and write the verdicts")
    p.add_argument("--only", nargs="*", help="check these functions only")
    p.add_argument("-j", "--jobs", type=int, default=max(2, (os.cpu_count() or 4) // 2))
    p.add_argument("--dry-run", action="store_true", help="do not write the table")
    p.set_defaults(fn=cmd_run)
    p = sub.add_parser("us", help="US: record unchecked (no US oracle)")
    p.add_argument("--dry-run", action="store_true")
    p.set_defaults(fn=cmd_us)
    p = sub.add_parser("report", help="summarise the verdicts in both tables")
    p.add_argument("--strict", action="store_true", help="exit 1 while any row has no verdict, differs, or is stale")
    p.set_defaults(fn=cmd_report)
    args = ap.parse_args()
    return args.fn(args)


if __name__ == "__main__":
    sys.exit(main())

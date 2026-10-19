#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Gameplay regression runs of the native port (docs/port/REGRESSION.md).

    regress.py run --game rac1-pal|rac1-ntsc [--set wide] [--levels 0-18] [--only KEY,...] [--build DIR] [--jobs N]
    regress.py compare A B [--fields cam,fw] [--tol position=0.1]
    regress.py baseline save NAME [RUN ...] [--set wide]   (the latest run of each version by default)
    regress.py check NAME [RUN ...] [--set wide]           (the latest runs against the baseline)
    regress.py reach RUN [RUN ...] [--top N] [--json FILE]
    regress.py card --game rac1-pal|rac1-ntsc [--from DIR]  (the prepared memory card, a copy)
    regress.py sets                                          (the route sets and their routes)

A run is a folder (build/regress/runs/<game>[-<set>]-<time>/), latest:<game> (the last run of
that version's standard routes), latest:<game>@<set> (of a set) or baseline:NAME/<game>.
Everything is kept under build/, never committed.
"""

from __future__ import annotations

import argparse
import json
import shutil
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import card as card_mod  # noqa: E402
import compare as compare_mod  # noqa: E402
import reach as reach_mod  # noqa: E402
import routes as routes_mod  # noqa: E402
import runner  # noqa: E402


def baselines_dir() -> Path:
    return runner.REPO / "build" / "regress" / "baselines"


def parse_levels(text: str | None) -> list[int]:
    if not text:
        return list(routes_mod.LEVELS)
    out = []
    for part in text.split(","):
        if "-" in part:
            a, b = part.split("-", 1)
            out += range(int(a), int(b) + 1)
        elif part.strip():
            out.append(int(part))
    return out


def resolve(spec: str) -> Path:
    if spec.startswith("latest:"):
        p = runner.latest(spec.split(":", 1)[1])
        if p is None:
            raise SystemExit(f"{spec}: no run of that version yet")
        return p
    if spec.startswith("baseline:"):
        return baselines_dir() / spec.split(":", 1)[1]
    p = Path(spec)
    if not p.is_dir():
        raise SystemExit(f"{spec}: not a run folder")
    return p


def run_game(meta_path: Path) -> str | None:
    meta = meta_path / "run.json"
    return json.loads(meta.read_text()).get("game") if meta.exists() else None


def cmd_run(args) -> int:
    build = Path(args.build) if args.build else runner.REPO / "port" / "build" / "release"
    levels = parse_levels(args.levels) if args.levels or args.set in (None, "standard") else None
    only = [k.strip() for k in args.only.split(",")] if args.only else None
    started = time.monotonic()
    what = f"{len(levels)} levels" if levels is not None else "all routes"
    print(f"{args.game}: {args.set or 'standard'} set, {what}{' ' + args.only if only else ''} from {build}, "
          f"{args.jobs} at once")
    out = runner.run(args.game, levels, build, Path(args.out) if args.out else None, args.jobs,
                     Path(args.data) if args.data else None, Path(args.level_data) if args.level_data else None,
                     Path(args.routes) if args.routes else None, route_set=args.set, only=only)
    meta = json.loads((out / "run.json").read_text())
    counts = {}
    for r in meta["levels"].values():
        counts[r["status"]] = counts.get(r["status"], 0) + 1
    print(f"{out}: {', '.join(f'{n} {s}' for s, n in sorted(counts.items()))} in {time.monotonic() - started:.0f} s")
    return 0 if set(counts) <= {"ok"} else 1


def print_compare(a: Path, b: Path, fields, tol, levels, noise: dict | None = None, strict: bool = False) -> bool:
    """Prints the comparison; True when nothing diverges. A level in noise (a baseline whose own
    runs already part there) is shown but does not count."""
    results = compare_mod.compare_runs(a, b, fields, tol, levels, strict=strict)
    noisy = 0
    bad = []
    for r in results:
        line = r.line()
        n = (noise or {}).get(str(r.level))
        if r.state == "diverges" and n is not None:
            noisy += 1
            line += f"  [noise: the baseline's own runs part at c={n['c']} on {n['field']}]"
        elif r.state != "same":
            bad.append(r)
        print(line)
    div = sum(1 for r in bad if r.state == "diverges")
    same = len(results) - len(bad) - noisy
    print(f"{same} same, {div} diverge, {noisy} noisy, {len(bad) - div} not compared ({a.name} vs {b.name})")
    return not bad


def noise_of(reference: Path, twins: list[Path]) -> dict:
    """The levels where a baseline's own runs of one build part (the port is not fully
    deterministic: sound and streams follow the real clock): the first tick and field."""
    out = {}
    for twin in twins:
        for r in compare_mod.compare_runs(reference, twin):
            if r.state == "diverges":
                key = str(r.level)
                if key not in out or r.first.c < out[key]["c"]:
                    out[key] = {"c": r.first.c, "field": r.first.field, "twin": twin.name}
    return out


def parse_tol(items: list[str] | None) -> dict:
    out = {}
    for item in items or []:
        k, v = item.split("=", 1)
        if k not in compare_mod.DEFAULT_TOLERANCE:
            raise SystemExit(f"--tol {k}: one of {', '.join(compare_mod.DEFAULT_TOLERANCE)}")
        out[k] = float(v)
    return out


def cmd_compare(args) -> int:
    fields = args.fields.split(",") if args.fields else None
    ok = print_compare(resolve(args.a), resolve(args.b), fields, parse_tol(args.tol),
                       parse_levels(args.levels) if args.levels else None, strict=args.strict)
    return 0 if ok else 1


def latest_runs(route_set: str | None) -> list[Path]:
    return [p for p in (runner.latest(g, route_set) for g in routes_mod.GAMES) if p]


def cmd_baseline_save(args) -> int:
    runs = [resolve(r) for r in args.runs] if args.runs else latest_runs(args.set)
    if not runs:
        raise SystemExit("no runs to save: regress.py run first")
    dest = baselines_dir() / args.name
    by_game: dict[str, list[Path]] = {}
    for run in runs:
        by_game.setdefault(run_game(run) or run.name, []).append(run)
    for game, game_runs in by_game.items():
        # The first run of a version is the reference; more runs of it (the same build again) are
        # its twins, kept as <game>~2, ~3, ... and read for the noise check compares within.
        stale = [dest / game] + sorted(dest.glob(f"{game}~*"))
        for old in stale:
            if old.exists():
                shutil.rmtree(old)  # the baseline's own copies, made by this command
        targets = []
        for i, run in enumerate(game_runs):
            target = dest / (game if i == 0 else f"{game}~{i + 1}")
            shutil.copytree(run, target)
            targets.append(target)
            print(f"baseline {args.name}/{target.name} <- {run}")
        noise = noise_of(targets[0], targets[1:])
        (targets[0] / "noise.json").write_text(json.dumps(noise, indent=1) + "\n")
        if len(targets) > 1:
            print(f"  {len(noise)} levels part between its own runs: "
                  + (", ".join(f"{k} (c={v['c']} {v['field']})" for k, v in sorted(noise.items(), key=lambda kv: key_order(kv[0])))
                     or "none"))
    return 0


def cmd_check(args) -> int:
    base = baselines_dir() / args.name
    if not base.is_dir():
        raise SystemExit(f"no baseline {args.name} (regress.py baseline save {args.name})")
    runs = [resolve(r) for r in args.runs] if args.runs else latest_runs(args.set)
    fields = args.fields.split(",") if args.fields else None
    ok = True
    for run in runs:
        game = run_game(run)
        if game is None or not (base / game).is_dir():
            print(f"{run}: no baseline for {game}")
            continue
        print(f"{game}: {run.name} against baseline {args.name}")
        noise_file = base / game / "noise.json"
        noise = json.loads(noise_file.read_text()) if noise_file.exists() else {}
        ok &= check_status(base / game, run)
        ok &= print_compare(base / game, run, fields, parse_tol(args.tol), None, noise)
    return 0 if ok else 1


def cmd_card(args) -> int:
    out = card_mod.prepare(args.game, Path(args.source) if args.source else None)
    print(out)
    return 0


def cmd_sets(args) -> int:
    data = routes_mod.load(Path(args.routes)) if args.routes else routes_mod.load()
    for name in routes_mod.sets(data):
        rs = routes_mod.set_routes(name, routes_mod.GAMES[0], data)
        print(f"{name}: {len(rs)} routes")
        for r in rs:
            extra = ", ".join(x for x in (r.start if r.start != "direct" else "", "card" if r.card else "") if x)
            print(f"  {r.key:<18} level {r.level:2d}  {r.name}" + (f"  ({extra})" if extra else ""))
    return 0


def key_order(key: str) -> tuple:
    """Levels in number order, then a set's routes by name."""
    return (0, int(key), "") if key.isdigit() else (1, 0, key)


def check_status(base: Path, run: Path) -> bool:
    """Levels whose outcome changed (ok to crash, no-control to ok)."""
    a = json.loads((base / "run.json").read_text())["levels"]
    b = json.loads((run / "run.json").read_text())["levels"]
    same = True
    for level in sorted(set(a) & set(b), key=key_order):
        if a[level]["status"] != b[level]["status"]:
            same = False
            head = f"level {int(level):2d} {a[level].get('name', ''):<15}" if level.isdigit() else f"{level:<18}"
            print(f"{head} was {a[level]['status']}, now {b[level]['status']}")
    return same


def cmd_reach(args) -> int:
    files = []
    for r in args.runs:
        files += reach_mod.load_run(resolve(r))
    merged = reach_mod.merge(files)
    for game, rows in sorted(merged.items()):
        print(reach_mod.table(game, rows, args.top))
    if args.json:
        Path(args.json).write_text(json.dumps({g: [r.as_dict() for r in rows] for g, rows in merged.items()}, indent=1))
    return 0


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="command", required=True)

    r = sub.add_parser("run", help="play the routes, one hidden run per level")
    r.add_argument("--game", required=True, choices=routes_mod.GAMES)
    r.add_argument("--levels", help="which levels: 0-18 (default), 1,3,5-7")
    r.add_argument("--build", help="the port's build folder (default port/build/release)")
    r.add_argument("--jobs", type=int, default=6, help="runs at once (default 6)")
    r.add_argument("--out", help="the run's folder (default build/regress/runs/<game>-<time>)")
    r.add_argument("--data", help="the game's data folder (default: the checkout's)")
    r.add_argument("--level-data", help="the extracted levels (default: the checkout's)")
    r.add_argument("--routes", help="another routes file")
    r.add_argument("--set", help="a route set of routes.json (default: standard, the level routes)")
    r.add_argument("--only", help="a set's routes by name: 11-drown,05-long")
    r.set_defaults(fn=cmd_run)

    c = sub.add_parser("compare", help="two runs, aligned on control")
    c.add_argument("a")
    c.add_argument("b")
    c.add_argument("--fields", help=f"which: {','.join(compare_mod.FIELDS)} (default all)")
    c.add_argument("--tol", action="append", help="KIND=VALUE: position, direction, angle")
    c.add_argument("--levels")
    c.add_argument("--strict", action="store_true",
                   help="between PAL and US, also compare what routes.json lists as version differences")
    c.set_defaults(fn=cmd_compare)

    b = sub.add_parser("baseline", help="keep runs as a baseline, or check against one")
    bs = b.add_subparsers(dest="action", required=True)
    s = bs.add_parser("save")
    s.add_argument("name")
    s.add_argument("runs", nargs="*")
    s.add_argument("--set", help="the latest runs of this route set (default: standard)")
    s.set_defaults(fn=cmd_baseline_save)
    for parent in (bs, sub):
        k = parent.add_parser("check", help="the latest runs against a baseline")
        k.add_argument("name")
        k.add_argument("runs", nargs="*")
        k.add_argument("--fields")
        k.add_argument("--tol", action="append")
        k.add_argument("--set", help="the latest runs of this route set (default: standard)")
        k.set_defaults(fn=cmd_check)

    m = sub.add_parser("reach", help="what the runs reached without right C, ranked per version")
    m.add_argument("runs", nargs="+")
    m.add_argument("--top", type=int, default=25)
    m.add_argument("--json")
    m.set_defaults(fn=cmd_reach)

    d = sub.add_parser("card", help="the prepared memory card: a copy of a card, every item, five slots")
    d.add_argument("--game", required=True, choices=routes_mod.GAMES)
    d.add_argument("--from", dest="source", help="the card to copy (default: the player's, read only)")
    d.set_defaults(fn=cmd_card)

    t = sub.add_parser("sets", help="the route sets and their routes")
    t.add_argument("--routes", help="another routes file")
    t.set_defaults(fn=cmd_sets)

    args = p.parse_args(argv)
    return args.fn(args)


if __name__ == "__main__":
    sys.exit(main())

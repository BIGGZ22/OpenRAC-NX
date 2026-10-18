# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Plays the routes: one hidden port process per level, several at once.

Each level's run goes in RUN/LL/ (trace.jsonl, reach.json, run.log, the settings it started from)
and its outcome in RUN/run.json: ok (the route played to its end), no-control (the run ended
before the player had control), crash (the process stopped on a fault or an error code), hang (no
end before the timeout: the process is stopped) or exit (it ended on its own before the route's
end).
"""

from __future__ import annotations

import json
import os
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime
from pathlib import Path

import routes as routes_mod

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]

# Where each version's data and extracted levels are, relative to a checkout (the main checkout's,
# for a worktree, since game data is outside git).
DATA = {
    "rac1-pal": ("build/data/pal", "assets/port/rac1/pal"),
    "rac1-ntsc": ("build/ntsc-data", "build/ntsc-levels"),
}


def main_checkout() -> Path:
    """The checkout whose .git a worktree shares (the repository itself otherwise)."""
    try:
        common = subprocess.run(["git", "-C", str(REPO), "rev-parse", "--path-format=absolute", "--git-common-dir"],
                                capture_output=True, text=True, check=True).stdout.strip()
        return Path(common).parent
    except (OSError, subprocess.CalledProcessError):
        return REPO


def default_inputs(game: str) -> tuple[Path, Path]:
    data, levels = DATA[game]
    for root in (REPO, main_checkout()):
        if (root / data).is_dir() and (root / levels).is_dir():
            return root / data, root / levels
    return main_checkout() / data, main_checkout() / levels


def runs_dir() -> Path:
    return REPO / "build" / "regress" / "runs"


def read_trace(path: Path) -> list[dict]:
    """The ticks of a trace; a line cut short by a crash is left out."""
    out = []
    if not path.exists():
        return out
    with path.open() as f:
        for line in f:
            try:
                out.append(json.loads(line))
            except json.JSONDecodeError:
                break
    return out


def classify(rc: int | None, timed_out: bool, ticks: list[dict], length: int) -> str:
    last_c = next((t["c"] for t in reversed(ticks) if t.get("c") is not None), None)
    if timed_out:
        return "hang"
    if rc is not None and rc != 0:
        return "crash"
    if last_c is None:
        return "no-control"
    if last_c < length:
        return "exit"
    return "ok"


def run_level(game: str, route: routes_mod.Route, exe: Path, data: Path, levels: Path, out: Path,
              extra_env: dict | None = None) -> dict:
    out.mkdir(parents=True, exist_ok=True)
    for name in ("trace.jsonl", "reach.json"):
        (out / name).unlink(missing_ok=True)
    env = dict(os.environ)
    for k in list(env):
        if k.startswith("OPENRAC_"):
            del env[k]  # the caller's own switches would change the run
    env.update({"OPENRAC_HIDDEN": "1", "SDL_AUDIODRIVER": "dummy", "OPENRAC_UNCAPPED": "1", "OPENRAC_AUDIO_TICKED": "1",
                "OPENRAC_TRACE": str(out / "trace.jsonl"), "OPENRAC_REACH": str(out / "reach.json")})
    env.update(route.env())
    env.update(extra_env or {})
    cmd = [str(exe), "--data", str(data), "--levels", str(levels), "--window", "--no-card", "--skip-movies",
           "--settings", str(out / "settings.ini"), "--level", str(route.level), "--frames", str(route.max_frames)]
    (out / "settings.ini").unlink(missing_ok=True)  # the defaults, whatever the player chose
    started = time.monotonic()
    timed_out = False
    with (out / "run.log").open("w") as log:
        proc = subprocess.Popen(cmd, env=env, stdout=log, stderr=subprocess.STDOUT, cwd=out)
        try:
            rc = proc.wait(timeout=route.timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
            proc.kill()  # our own process
            rc = proc.wait()
    seconds = time.monotonic() - started
    ticks = read_trace(out / "trace.jsonl")
    status = classify(rc, timed_out, ticks, route.length)
    control = next((t["f"] - t["c"] for t in ticks if t.get("c") is not None), None)
    result = {"level": route.level, "name": route.name, "status": status, "rc": rc,
              "seconds": round(seconds, 1), "control_frame": control, "ticks": len(ticks),
              "last_frame": ticks[-1]["f"] if ticks else None,
              "last_mode": ticks[-1].get("m") if ticks else None,
              "last_level": ticks[-1].get("lv") if ticks else None}
    if status != "ok":
        result["notes"] = notes((out / "run.log").read_text(errors="replace").splitlines())
    return result


def notes(lines: list[str]) -> list[str]:
    """What a failed run's log says: the crash report's reason and the game functions on top of
    its backtrace, else its last errors, else its last lines."""
    reason = [l.split(":", 1)[1].strip() for l in lines if l.startswith("Reason:")]
    frames = []
    at = next((i for i, l in enumerate(lines) if l.startswith("Backtrace")), len(lines))
    for l in lines[at:]:
        for word in l.split():
            if word.startswith("game_") and word not in frames:
                frames.append(word)
    if reason:
        return reason[:1] + [f"in {' < '.join(frames[:4])}"] if frames else reason[:1]
    errors = [l for l in lines if "[error]" in l]
    return (errors or lines)[-3:]


def run(game: str, levels: list[int], build: Path, out: Path | None = None, jobs: int = 6,
        data: Path | None = None, level_dir: Path | None = None, routes_file: Path | None = None,
        progress=print) -> Path:
    exe = build / f"openrac-{game}"
    if not exe.exists():
        raise SystemExit(f"no {exe}: build the port first (cd port && cmake --build --preset release)")
    d, lv = default_inputs(game)
    data = data or d
    level_dir = level_dir or lv
    for p in (data, level_dir):
        if not p.is_dir():
            raise SystemExit(f"no {p}: pass --data and --levels")
    spec = routes_mod.load(routes_file) if routes_file else routes_mod.load()
    mark_latest = out is None
    if out is None:
        out = runs_dir() / f"{game}-{datetime.now().strftime('%Y%m%d-%H%M%S')}"
    out = out.resolve()  # each run's process works in its own folder
    out.mkdir(parents=True, exist_ok=True)
    started = time.monotonic()
    meta = {"game": game, "build": str(build), "exe": str(exe.resolve()), "data": str(data),
            "levels_dir": str(level_dir), "started": datetime.now().isoformat(timespec="seconds"),
            "commit": _commit(build), "levels": {}}

    def one(level: int) -> dict:
        r = routes_mod.route(level, game, spec)
        res = run_level(game, r, exe, data, level_dir, out / f"{level:02d}")
        progress(f"  {game} level {level:2d} {r.name:<15} {res['status']:<10} {res['seconds']:6.1f} s"
                 + (f"  control at {res['control_frame']}" if res["control_frame"] is not None else ""))
        return res

    with ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        for res in pool.map(one, levels):
            meta["levels"][str(res["level"])] = res
    meta["seconds"] = round(time.monotonic() - started, 1)
    (out / "run.json").write_text(json.dumps(meta, indent=1) + "\n")
    if mark_latest:
        # latest:<game> for compare, baseline save and check; a run with its own --out is kept
        # apart (another build's, for a comparison).
        (runs_dir() / f"latest-{game}").parent.mkdir(parents=True, exist_ok=True)
        (runs_dir() / f"latest-{game}").write_text(str(out) + "\n")
    return out


def _commit(build: Path) -> str | None:
    """The commit of the checkout the build belongs to."""
    root = build.resolve()
    while root != root.parent and not (root / ".git").exists():
        root = root.parent
    try:
        return subprocess.run(["git", "-C", str(root), "rev-parse", "--short=10", "HEAD"], capture_output=True,
                              text=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return None


def latest(game: str) -> Path | None:
    p = runs_dir() / f"latest-{game}"
    return Path(p.read_text().strip()) if p.exists() else None

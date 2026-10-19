# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Two sets of traces side by side: per level, the first tick from control on where they differ.

Ticks are paired by c, the frames since control, so two builds of one version and the two
versions (whose control comes at other window frames) are read at the same moments of a route.
Numbers compare within a tolerance per field; the hero's and the camera's angles wrap at a turn.
"""

from __future__ import annotations

import json
import math
from dataclasses import dataclass, field
from pathlib import Path

import routes as routes_mod
from runner import read_trace, run_routes, run_set

# Every field the trace has, in the order a divergence names them, and its tolerance's kind.
FIELDS = {
    "lv": "exact", "m": "exact", "cam": "position", "fw": "direction", "hp": "position", "hr": "angle",
    "hs": "exact", "mb": "exact", "dr": "exact",
}
DEFAULT_TOLERANCE = {"position": 0.05, "direction": 0.01, "angle": 0.01, "exact": 0}


@dataclass
class Divergence:
    c: int
    field: str
    a: object
    b: object
    others: list = field(default_factory=list)  # the other fields that differ at the same tick
    frames: tuple = (None, None)                # the window frames in each


@dataclass
class LevelResult:
    level: int | str           # the level (standard set) or the route's name (another set)
    name: str
    state: str                # "same", "diverges", "missing" (one side has no trace from control)
    compared: int = 0         # ticks compared
    first: Divergence | None = None
    note: str = ""

    def line(self) -> str:
        head = (f"level {self.level:2d} {self.name:<15}" if isinstance(self.level, int)
                else f"{self.level:<18} {self.name[:28]:<28}")
        if self.state == "missing":
            return f"{head} not compared: {self.note}"
        if self.state == "same":
            return f"{head} same through c={self.compared - 1}" + (f" ({self.note})" if self.note else "")
        d = self.first
        more = f" (+ {', '.join(d.others)})" if d.others else ""
        return (f"{head} diverges at c={d.c} (frames {d.frames[0]}/{d.frames[1]}): {d.field} "
                f"{_show(d.a)} vs {_show(d.b)}{more}" + (f"; {self.note}" if self.note else ""))


def _show(v) -> str:
    if isinstance(v, list):
        return "[" + ", ".join(_show(x) for x in v) + "]"
    if isinstance(v, float):
        return f"{v:.3f}"
    return "none" if v is None else str(v)


def _angle_gap(a: float, b: float) -> float:
    d = (a - b) % (2 * math.pi)
    return min(d, 2 * math.pi - d)


def differs(kind: str, a, b, tol: float) -> bool:
    if a is None or b is None:
        return (a is None) != (b is None)
    if isinstance(a, list) or isinstance(b, list):
        if not isinstance(a, list) or not isinstance(b, list) or len(a) != len(b):
            return True
        return any(differs(kind, x, y, tol) for x, y in zip(a, b))
    if kind == "exact":
        return a != b
    if kind == "angle":
        return _angle_gap(float(a), float(b)) > tol
    return abs(float(a) - float(b)) > tol


def by_control(ticks: list[dict]) -> dict[int, dict]:
    return {t["c"]: t for t in ticks if t.get("c") is not None}


def gap(kind: str, a, b) -> float:
    """How far apart two values are, in their field's terms (1 for exact fields that differ)."""
    if a is None or b is None or isinstance(a, list) != isinstance(b, list):
        return 0.0 if a == b else math.inf
    if isinstance(a, list):
        if len(a) != len(b):
            return math.inf
        return max((gap(kind, x, y) for x, y in zip(a, b)), default=0.0)
    if kind == "exact":
        return 0.0 if a == b else 1.0
    if kind == "angle":
        return _angle_gap(float(a), float(b))
    return abs(float(a) - float(b))


def ignored(differences: list[dict], c: int) -> set[str]:
    """The fields a level's version differences (routes.json "versions") leave out at tick c."""
    return {f for d in differences if c >= int(d.get("from", 0)) for f in d.get("fields", [])}


def version_differences(level: int | str, data: dict | None = None, route_set: str = "standard") -> list[dict]:
    """What routes.json says the two versions do differently on a level or a set's route
    ("versions": the fields not compared between PAL and US from a tick on, and why)."""
    data = data if data is not None else routes_mod.load()
    if route_set != "standard":
        return list(data.get("sets", {}).get(route_set, {}).get("routes", {}).get(str(level), {}).get("versions", []))
    return list(data.get("levels", {}).get(str(level), {}).get("versions", []))


def by_frame(ticks: list[dict]) -> dict[int, dict]:
    """A route without control (the title's), read by window frame, with "c" the frame."""
    return {t["f"]: dict(t, c=t["f"]) for t in ticks}


def compare_ticks(a: list[dict], b: list[dict], fields: list[str], tolerance: dict,
                  differences: list[dict] | None = None, frames: bool = False) -> tuple[int, Divergence | None, str]:
    """Ticks compared, the first divergence, and a note: the largest gap of each field that
    diverges over the whole route (a drift and a broken routine read differently), or where one
    side ends first. Fields a version difference leaves out (differences) are not compared from
    its tick on. frames: pair the ticks by window frame (routes without control)."""
    ca, cb = (by_frame(a), by_frame(b)) if frames else (by_control(a), by_control(b))
    if not ca or not cb:
        return 0, None, ""
    end = min(max(ca), max(cb))
    compared = 0
    first = None
    largest: dict[str, float] = {}
    for c in range(0, end + 1):
        ta, tb = ca.get(c), cb.get(c)
        if ta is None or tb is None:
            continue
        compared += 1
        skip = ignored(differences or [], c)
        bad = [f for f in fields if f not in skip and differs(FIELDS[f], ta.get(f), tb.get(f), tolerance[FIELDS[f]])]
        for f in bad:
            largest[f] = max(largest.get(f, 0.0), gap(FIELDS[f], ta.get(f), tb.get(f)))
        if bad and first is None:
            f = bad[0]
            first = Divergence(c, f, ta.get(f), tb.get(f), bad[1:], (ta.get("f"), tb.get("f")))
    notes = []
    if largest:
        order = list(FIELDS)
        notes.append("largest gap " + ", ".join(
            f"{f} {'differs' if FIELDS[f] == 'exact' else format(g, '.3f')}"
            for f, g in sorted(largest.items(), key=lambda kv: order.index(kv[0]))))
    if max(ca) != max(cb):
        notes.append(f"A ends at c={max(ca)}, B at c={max(cb)}")
    if differences:
        notes.append("version differences not compared: " + ", ".join(
            f"{'/'.join(d.get('fields', []))} from c={int(d.get('from', 0))}" for d in differences))
    return compared, first, "; ".join(notes)


def run_game(run: Path) -> str | None:
    """The game a run is of (run.json), or None."""
    meta = run / "run.json"
    return json.loads(meta.read_text()).get("game") if meta.exists() else None


def run_levels(run: Path) -> dict[int | str, tuple[str, Path]]:
    """A run's levels or routes (from run.json, else the folders) with their names and traces."""
    return {k: (name, folder / "trace.jsonl") for k, (name, _, folder) in run_routes(run).items()}


def _order(key) -> tuple:
    return (0, key, "") if isinstance(key, int) else (1, 0, str(key))


def compare_runs(a: Path, b: Path, fields: list[str] | None = None, tolerance: dict | None = None,
                 levels: list[int] | None = None, strict: bool = False,
                 routes: dict | None = None) -> list[LevelResult]:
    """Per level, the two runs side by side. Runs of two versions (run.json's game) leave out what
    routes.json says the versions do differently on that level, unless strict."""
    fields = fields or list(FIELDS)
    for f in fields:
        if f not in FIELDS:
            raise ValueError(f"unknown field {f!r} (known: {', '.join(FIELDS)})")
    tol = dict(DEFAULT_TOLERANCE)
    tol.update(tolerance or {})
    la, lb = run_levels(a), run_levels(b)
    ga, gb = run_game(a), run_game(b)
    versions = not strict and ga is not None and gb is not None and ga != gb
    data = (routes if routes is not None else routes_mod.load()) if versions else None
    route_set = run_set(a)
    out = []
    for level in sorted(set(la) | set(lb), key=_order):
        if levels is not None and level not in levels:
            continue
        name = (la.get(level) or lb.get(level))[0]
        if level not in la or level not in lb:
            out.append(LevelResult(level, name, "missing", note=f"only in {'A' if level in la else 'B'}"))
            continue
        ta, tb = read_trace(la[level][1]), read_trace(lb[level][1])
        if not any(t.get("c") is not None for t in ta + tb) and ta and tb and ga == gb:
            # A route without control (the title's), two runs of one version: by window frame.
            n, first, note = compare_ticks(ta, tb, fields, tol, None, frames=True)
            out.append(LevelResult(level, name, "diverges" if first else "same", n, first,
                                   "; ".join(x for x in ("by window frame", note) if x)))
            continue
        if not by_control(ta) or not by_control(tb):
            which = [s for s, t in (("A", ta), ("B", tb)) if not by_control(t)]
            out.append(LevelResult(level, name, "missing", note=f"no control in {' and '.join(which)}"))
            continue
        differences = version_differences(level, data, route_set) if versions else []
        n, first, note = compare_ticks(ta, tb, fields, tol, differences)
        out.append(LevelResult(level, name, "diverges" if first else "same", n, first, note))
    return out

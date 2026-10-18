# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The OPENRAC_REACH files of runs, merged: per version, what the routes reached that the port
has no right C for, ranked by how many levels reached it, then by its calls.

A row is a function and its kind: a stub (asm: still assembly in the decompilation; no-c: no C at
all; untranslated: C hostgen cannot write; missing: hand C's own report) or zero-args (a call it
makes with an argument hostgen filled with 0; the details name the callees).
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class Row:
    name: str
    kind: str
    levels: set = field(default_factory=set)
    calls: int = 0
    details: set = field(default_factory=set)

    def as_dict(self) -> dict:
        return {"name": self.name, "kind": self.kind, "levels": sorted(self.levels), "calls": self.calls,
                "details": sorted(self.details)}


def merge(files: list[tuple[str, int, dict]]) -> dict[str, list[Row]]:
    """files: (game, level, the reach file's JSON). Returns each game's rows, ranked."""
    rows: dict[str, dict[tuple[str, str], Row]] = {}
    for game, level, reach in files:
        game = reach.get("game") or game
        table = rows.setdefault(game, {})
        for s in reach.get("sites", []):
            if not s.get("calls"):
                continue
            key = (s["name"], s.get("kind", "?"))
            row = table.setdefault(key, Row(*key))
            row.levels.add(level)
            row.calls += int(s["calls"])
            if s.get("detail"):
                row.details.add(s["detail"])
    return {g: sorted(t.values(), key=lambda r: (-len(r.levels), -r.calls, r.name)) for g, t in rows.items()}


def load_run(run: Path) -> list[tuple[str, int, dict]]:
    meta = run / "run.json"
    game = json.loads(meta.read_text()).get("game", "?") if meta.exists() else "?"
    out = []
    for d in sorted(run.iterdir()):
        f = d / "reach.json"
        if d.is_dir() and d.name.isdigit() and f.exists():
            try:
                out.append((game, int(d.name), json.loads(f.read_text())))
            except json.JSONDecodeError:
                continue
    return out


def table(game: str, rows: list[Row], top: int) -> str:
    lines = [f"{game}: {len(rows)} functions reached without right C", "  levels  calls       kind          function"]
    for r in rows[:top]:
        extra = f"  -> {', '.join(sorted(r.details)[:2])}" if r.details else ""
        lines.append(f"  {len(r.levels):6d}  {r.calls:10d}  {r.kind:<12}  {r.name}{extra}")
    return "\n".join(lines)

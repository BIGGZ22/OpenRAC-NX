# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The routes (routes.json) and what the port is told to play them.

A route becomes the port's own scripting switches (docs/port/RAC1_PAL_TESTING.md): OPENRAC_PRESS
with its window-frame entries for the boot screens, its k entries (held while Start can skip what
plays, until control) and its c entries (frames from control), and OPENRAC_CONTROL_FRAMES, which
ends the run the route's length after control.
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROUTES = HERE / "routes.json"

# The pad's button bits, as OPENRAC_PRESS takes them (active while held).
BUTTONS = {
    "select": 0x1, "start": 0x8, "up": 0x10, "right": 0x20, "down": 0x40, "left": 0x80,
    "l2": 0x100, "r2": 0x200, "l1": 0x400, "r1": 0x800,
    "triangle": 0x1000, "circle": 0x2000, "cross": 0x4000, "square": 0x8000,
}

GAMES = ("rac1-pal", "rac1-ntsc")
LEVELS = range(19)


def mask(buttons: str | list | None) -> int:
    """'cross', 'r1+square' or ['r1', 'square'] as the pad's bits."""
    if not buttons:
        return 0
    names = buttons.split("+") if isinstance(buttons, str) else list(buttons)
    out = 0
    for n in names:
        n = n.strip().lower()
        if n not in BUTTONS:
            raise ValueError(f"unknown button {n!r} (known: {', '.join(BUTTONS)})")
        out |= BUTTONS[n]
    return out


@dataclass
class Route:
    level: int
    name: str
    game: str
    boot: list = field(default_factory=list)
    skip: dict | None = None
    steps: list = field(default_factory=list)
    length: int = 660
    max_frames: int = 9000
    timeout: float = 240.0

    def press(self) -> str:
        """The route as OPENRAC_PRESS."""
        out = []
        for b in self.boot:
            out.append(f"{int(b['at'])}:{mask(b.get('buttons')):x}:{int(b.get('hold', 5))}")
        if self.skip:
            out.append(f"k{int(self.skip['every'])}:{mask(self.skip.get('buttons', 'start')):x}"
                       f":{int(self.skip.get('hold', 5))}")
        for s in self.steps:
            lx, ly = s.get("left", (128, 128))
            rx, ry = s.get("right", (128, 128))
            out.append(f"c{int(s['at'])}:{mask(s.get('buttons')):x}:{int(s.get('hold', 5))}"
                       f":{int(lx)}:{int(ly)}:{int(rx)}:{int(ry)}")
        return ",".join(out)

    def env(self) -> dict[str, str]:
        return {"OPENRAC_DIRECT": "1", "OPENRAC_PRESS": self.press(),
                "OPENRAC_CONTROL_FRAMES": str(int(self.length))}


def load(path: Path = ROUTES) -> dict:
    return json.loads(Path(path).read_text())


def route(level: int, game: str, data: dict | None = None) -> Route:
    """Level's route for game: the defaults, the level's own keys, then its game's."""
    data = data if data is not None else load()
    spec = dict(data["defaults"])
    own = dict(data["levels"].get(str(level), {}))
    per_game = own.pop(game, {}) if isinstance(own.get(game), dict) else {}
    for other in GAMES:
        own.pop(other, None)
    spec.update(own)
    spec.update(per_game)
    sequence = spec.get("steps")
    if sequence is None:
        name = spec.get("sequence", "standard")
        if name not in data["sequences"]:
            raise ValueError(f"level {level}: no sequence {name!r}")
        sequence = data["sequences"][name]
    return Route(level=level, name=spec.get("name", f"level {level}"), game=game, boot=spec.get("boot", []),
                 skip=spec.get("skip"), steps=sequence, length=int(spec.get("length", 660)),
                 max_frames=int(spec.get("max_frames", 9000)), timeout=float(spec.get("timeout", 240)))

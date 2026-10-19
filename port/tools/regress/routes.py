# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The routes (routes.json) and what the port is told to play them.

A route becomes the port's own scripting switches (docs/port/RAC1_PAL_TESTING.md): OPENRAC_PRESS
with its window-frame entries for the boot screens, its k or s entries (held while Start can skip
what plays, until control: k counted on window frames, s from the frame what plays became
skippable) and its c entries (frames from control), OPENRAC_WRITE's c entries (a route's teleports,
items and other words of game memory, from control) and OPENRAC_CONTROL_FRAMES, which ends the run
the route's length after control.

The level routes (routes.json "levels", one a level) are the standard set; "sets" holds more,
each a table of named routes ("routes": {"11-drown": {"level": 11, ...}}) with defaults of its
own. A set's steps can do more than press: teleport the hero, give items, equip one, set his
health, write any word, and repeat a block for each entry of a list ("each").
"""

from __future__ import annotations

import copy
import json
import struct
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

# What a step can do besides pressing (a set's routes): each becomes words of game memory written
# from control, at the version's addresses (routes.json "addresses").
ACTIONS = ("teleport", "yaw", "give", "equip", "health", "write")


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


def presses(step: dict) -> bool:
    """Whether a step holds buttons or sticks (a step of a set's route can also only write)."""
    return "buttons" in step or "left" in step or "right" in step or not any(k in step for k in ACTIONS)


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
    key: str = ""                  # the route's folder in a run: LL for the standard set
    writes: list = field(default_factory=list)    # (frame from control, address, word, frames)
    start: str = "direct"          # direct: OPENRAC_DIRECT with --level N; title: from the boot screens
    card: str | None = None        # "prepared": a fresh copy of the prepared card (regress.py card)
    until: str = "control"         # control: the run ends length frames after it; frames: at max_frames
    versions: list = field(default_factory=list)  # what PAL and US do differently on it (compare)

    def __post_init__(self):
        if not self.key:
            self.key = f"{self.level:02d}"

    def press(self) -> str:
        """The route as OPENRAC_PRESS."""
        out = []
        for b in self.boot:
            out.append(f"{int(b['at'])}:{mask(b.get('buttons')):x}:{int(b.get('hold', 5))}")
        if self.skip:
            # k: window frames; s: from the frame the scene became skippable; S: s after control too.
            kind = ("S" if self.skip.get("after_control") else "s") if self.skip.get("from") == "scene" else "k"
            out.append(f"{kind}{int(self.skip['every'])}:{mask(self.skip.get('buttons', 'start')):x}"
                       f":{int(self.skip.get('hold', 5))}")
        for s in self.steps:
            if not presses(s):
                continue
            lx, ly = s.get("left", (128, 128))
            rx, ry = s.get("right", (128, 128))
            out.append(f"c{int(s['at'])}:{mask(s.get('buttons')):x}:{int(s.get('hold', 5))}"
                       f":{int(lx)}:{int(ly)}:{int(rx)}:{int(ry)}")
        return ",".join(out)

    def write(self) -> str:
        """The route's writes as OPENRAC_WRITE (c entries: frames from control)."""
        return ",".join(f"c{int(at)}:{address:x}:{value & 0xFFFFFFFF:x}:{int(frames)}"
                        for at, address, value, frames in self.writes)

    @property
    def until_frames(self) -> bool:
        """A route that never gets control (the title's menus) plays to max_frames and ends there."""
        return self.until == "frames"

    def env(self) -> dict[str, str]:
        env = {"OPENRAC_PRESS": self.press()}
        if self.start == "direct":
            env["OPENRAC_DIRECT"] = "1"
        if not self.until_frames:
            env["OPENRAC_CONTROL_FRAMES"] = str(int(self.length))
        if self.writes:
            env["OPENRAC_WRITE"] = self.write()
        return env


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
                 max_frames=int(spec.get("max_frames", 9000)), timeout=float(spec.get("timeout", 240)),
                 versions=list(spec.get("versions", [])))


# ---------------------------------------------------------------------------------------------
# Route sets.

def word(value) -> int:
    """A step's value as the 32-bit word written: a float as its bits, an int or "0x..." as is."""
    if isinstance(value, float):
        return struct.unpack("<I", struct.pack("<f", value))[0]
    if isinstance(value, str):
        if value.lower().startswith("0x"):
            return int(value, 16)
        return struct.unpack("<I", struct.pack("<f", float(value)))[0]
    return int(value) & 0xFFFFFFFF


def address(spec, addresses: dict) -> int:
    """'hero+0x80', 'owned' or '0x13F4D0', at a version's addresses."""
    if isinstance(spec, int):
        return spec
    base, _, off = str(spec).partition("+")
    base = base.strip()
    if base.lower().startswith("0x"):
        at = int(base, 16)
    elif base in addresses:
        at = int(str(addresses[base]), 16)
    else:
        raise ValueError(f"no address {base!r} for this version (routes.json addresses)")
    return at + (int(off, 0) if off else 0)


def expand(steps: list, lists: dict, points: dict, offset: int = 0, value=None) -> list[dict]:
    """Steps with each "each" block unrolled ({"at", "each": LIST, "every": N, "do": [...]}: the
    block's steps at their own frames once for every entry, "$" the entry) and named points
    resolved, in frame order."""
    out = []
    for s in steps:
        if "each" in s:
            entries = s["each"]
            if isinstance(entries, str):
                entries = lists[entries] if entries in lists else points.get(entries)
            if entries is None:
                raise ValueError(f"no list {s['each']!r}")
            for i, entry in enumerate(entries):
                out += expand(s["do"], lists, points, offset + int(s["at"]) + i * int(s["every"]), entry)
            continue
        t = copy.deepcopy(s)
        t["at"] = int(s["at"]) + offset
        for k, v in list(t.items()):
            if v == "$":
                t[k] = value
            if k == "teleport" and isinstance(t[k], str):
                if t[k] not in points:
                    raise ValueError(f"no point {t[k]!r}")
                t[k] = points[t[k]]
        out.append(t)
    return sorted(out, key=lambda t: t["at"])


def byte_words(base: int, ids: list[int], value: int = 1) -> list[tuple[int, int]]:
    """Bytes set to value at base + id, as the aligned words that hold them (a word's other bytes
    are written 0 unless listed too)."""
    words: dict[int, int] = {}
    for i in ids:
        a = base + int(i)
        words[a & ~3] = words.get(a & ~3, 0) | ((value & 0xFF) << (8 * (a & 3)))
    return sorted(words.items())


def writes_of(steps: list, game: str, data: dict) -> list[tuple]:
    """The words a set's steps write: (frame from control, address, word, frames)."""
    addresses = data.get("addresses", {}).get(game, {})
    items = data.get("items", {})
    out = []
    for s in steps:
        at = int(s["at"])
        if "teleport" in s:
            p = s["teleport"]
            hold = int(s.get("hold_place", 10))
            for i in range(3):
                out.append((at, address(f"hero+{0x80 + 4 * i}", addresses), word(float(p[i])), hold))
            if len(p) > 3:
                out.append((at, address("hero+0x98", addresses), word(float(p[3])), 50))
        if "yaw" in s:
            out.append((at, address("hero+0x98", addresses), word(float(s["yaw"])), 50))
        if "give" in s:
            ids = items.get(s["give"]) if isinstance(s["give"], str) else s["give"]
            if ids is None:
                raise ValueError(f"no item list {s['give']!r}")
            # The whole table is written (unlisted items 0), so the bytes beside a listed one are
            # what the list says rather than what a word's other bytes held.
            for name in ("owned", "acquired"):
                out += [(at, a, v, 1) for a, v in byte_words(address(name, addresses), ids)]
            ammo = items.get("ammo", {})
            for i in ids:
                if str(i) in ammo:
                    out.append((at, address(f"ammo+{4 * int(i)}", addresses), int(ammo[str(i)]), 1))
        if "equip" in s:
            out.append((at, address("hero+0x20B8", addresses), int(s["equip"]), 1))
        if "health" in s:
            out.append((at, address("hero+0x22A8", addresses), int(s["health"]), int(s.get("hold_health", 1))))
        if "write" in s:
            w = s["write"]
            out.append((at, address(w["address"], addresses), word(w["value"]), int(w.get("frames", 1))))
    return out


def sets(data: dict | None = None) -> list[str]:
    data = data if data is not None else load()
    return ["standard"] + sorted(data.get("sets", {}))


def set_routes(name: str | None, game: str, data: dict | None = None) -> list[Route]:
    """A set's routes for game: the standard defaults, the set's, each route's own keys, then its
    game's. "standard" (or None) is the level routes."""
    data = data if data is not None else load()
    if name in (None, "", "standard"):
        return [route(level, game, data) for level in LEVELS]
    if name not in data.get("sets", {}):
        raise ValueError(f"no route set {name!r} (known: {', '.join(sets(data))})")
    rs = data["sets"][name]
    sequences = dict(data["sequences"])
    sequences.update(rs.get("sequences", {}))
    lists = dict(data.get("lists", {}))
    lists.update(rs.get("lists", {}))
    out = []
    for key, own in rs["routes"].items():
        spec = dict(data["defaults"])
        spec.update(rs.get("defaults", {}))
        own = dict(own)
        per_game = own.pop(game, {}) if isinstance(own.get(game), dict) else {}
        for other in GAMES:
            own.pop(other, None)
        spec.update(own)
        spec.update(per_game)
        steps = spec.get("steps")
        if steps is None:
            seq = spec.get("sequence", "standard")
            if seq not in sequences:
                raise ValueError(f"{name}/{key}: no sequence {seq!r}")
            steps = sequences[seq]
        steps = expand(list(steps) + list(spec.get("then", [])), dict(lists, **spec.get("lists", {})),
                       dict(spec.get("points", {})))
        out.append(Route(level=int(spec.get("level", -1)), name=spec.get("name", key), game=game,
                         boot=spec.get("boot", []), skip=spec.get("skip"), steps=steps,
                         length=int(spec.get("length", 660)), max_frames=int(spec.get("max_frames", 9000)),
                         timeout=float(spec.get("timeout", 240)), key=key, writes=writes_of(steps, game, data),
                         start=spec.get("start", "direct"), card=spec.get("card"), until=spec.get("until", "control"),
                         versions=list(spec.get("versions", []))))
    return out

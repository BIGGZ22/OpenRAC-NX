#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Places along a level's path, read from the level's own data, for the routes' teleports.

    points.py LEVEL [--levels DIR] [--max N]

The extractor's placements.json (editor/port.py) lists every moby a level places. The hero's start
(class 0), the respawn points (805: the level's checkpoints), the gold bolts (1134), the infobots
(750), the teleporters (1135) and the vendors (11) mark the level's main path and its ends; the
path is those, the start first, then each time the nearest one not yet visited (at most --max);
a level with fewer than three of them uses its bolt crates and enemies too. A vendor's point is
where a player stands to talk to it: three units in front of it, facing it.
The wide route set (routes.json "sets") holds what this printed, as data; run it again after an
export changes the placements.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import runner

HERO, VENDOR, SPAWN, GOLD_BOLT, INFOBOT, TELEPORTER = 0, 11, 805, 1134, 750, 1135
PATH_CLASSES = (SPAWN, GOLD_BOLT, INFOBOT, TELEPORTER, VENDOR)
# A level with few of those (Veldin's first visit has none): its bolt crates (500) and enemies
# (Veldin's robots, 1440) mark where the player goes.
FALLBACK_CLASSES = (500, 1440)


def placements(level: int, root: Path | None = None) -> list[dict]:
    root = root or runner.default_inputs("rac1-pal")[1]
    return json.loads((root / f"level_{level:02d}" / "placements.json").read_text())["mobys"]


def position(m: dict) -> list[float]:
    return [float(x) for x in m["matrix"][12:15]]


def vendor_stand(m: dict, distance: float = 3.0) -> list[float]:
    """Where the hero talks to a vendor: in front of it, facing it (x, y, z, yaw)."""
    x, y, z = position(m)
    yaw = float(m.get("rotation", [0, 0, 0])[2])
    return [round(x + distance * math.cos(yaw), 2), round(y + distance * math.sin(yaw), 2), round(z + 0.3, 2),
            round(math.atan2(math.sin(yaw + math.pi), math.cos(yaw + math.pi)), 3)]


def level_points(level: int, root: Path | None = None, most: int = 10) -> dict:
    """The level's start, its vendors' points and its path (nearest first from the start)."""
    mobys = placements(level, root)
    start = next((position(m) for m in mobys if m["class"] == HERO), None)
    vendors = [vendor_stand(m) for m in mobys if m["class"] == VENDOR]
    places = []
    classes = PATH_CLASSES
    if sum(1 for m in mobys if m["class"] in PATH_CLASSES) < 3:
        classes = PATH_CLASSES + FALLBACK_CLASSES
    for m in mobys:
        if m["class"] in classes:
            p = vendor_stand(m)[:3] if m["class"] == VENDOR else position(m)
            p = [round(p[0], 2), round(p[1], 2), round(p[2] + 0.5, 2)]
            if all(math.dist(p, q) > 8.0 for q in places):  # one point for a cluster
                places.append(p)
    path = []
    here = start or (places[0] if places else [0.0, 0.0, 0.0])
    while places and len(path) < most:
        nxt = min(places, key=lambda q: math.dist(here, q))
        places.remove(nxt)
        path.append(nxt)
        here = nxt
    return {"start": [round(x, 2) for x in start] if start else None, "vendors": vendors, "path": path}


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("level", type=int)
    p.add_argument("--levels", help="the extracted levels (default: the checkout's PAL levels)")
    p.add_argument("--max", type=int, default=10, help="points on the path at most (default 10)")
    args = p.parse_args(argv)
    print(json.dumps(level_points(args.level, Path(args.levels) if args.levels else None, args.max)))
    return 0


if __name__ == "__main__":
    sys.exit(main())

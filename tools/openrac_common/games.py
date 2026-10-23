# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
The games' manifests: games/<game>/game.json, read one way.

  from openrac_common import games
  rac1 = games.load("rac1")                     # one manifest
  for game, name, version in games.versions():  # every version of every game, in game order
      print(game["id"], name, version["serial"], games.version_dir(game, name))

Written from tools/openrac.py:45-57 (vdir, games, versions), the most complete of the five
readers; the others are tools/sources.py:36 (manifests, version_dir), tools/xmap.py:58
(versions), port/tools/versionmap/versionmap.py:52 (version) and tools/bundle_standalone.py:243
(which copies every manifest). A manifest is returned as the parsed JSON, unchanged: its fields
are described in docs/LAYOUT.md and each game's README, and this module does not check them.
"""
import json
from pathlib import Path

from .paths import repo_root


def games_dir(root: Path | str | None = None) -> Path:
    """The games/ folder under ROOT, the top of OpenRAC (default: the checkout holding this package)."""
    return (Path(root) if root is not None else repo_root()) / "games"


def manifest_path(game: str, root: Path | str | None = None) -> Path:
    """Where GAME's manifest is: games/<game>/game.json under ROOT (default: this checkout)."""
    return games_dir(root) / game / "game.json"


def load(game: str, root: Path | str | None = None) -> dict:
    """GAME's manifest (games/<game>/game.json), parsed. Raises FileNotFoundError for a game
    OpenRAC does not have."""
    return json.loads(manifest_path(game, root).read_text(encoding="utf-8"))


def load_all(root: Path | str | None = None) -> list[dict]:
    """Every game's manifest, in game order (the directories' names sort as the release order)."""
    return [json.loads(p.read_text(encoding="utf-8")) for p in sorted(games_dir(root).glob("*/game.json"))]


def versions(root: Path | str | None = None) -> list[tuple[dict, str, dict]]:
    """(game, version name, version) for every version of every game, in game order and in each
    manifest's own order of versions."""
    return [(game, name, version) for game in load_all(root) for name, version in game["versions"].items()]


def version_dir(game: dict, name: str) -> str:
    """Version NAME's directory, relative to the top of OpenRAC: its "dir" in game.json (rac1's
    PAL tree is the merged games/rac1), else games/<game>/<version>."""
    return game["versions"][name].get("dir", f"games/{game['id']}/{name}")

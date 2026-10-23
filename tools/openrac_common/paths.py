# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Where the repository is: the one way a tool finds OpenRAC's top directory.

  from openrac_common import paths
  root = paths.repo_root()              # from this package's own place
  root = paths.repo_root(__file__)      # from the calling script's place

Today 23 scripts each write `Path(__file__).resolve().parents[N]` with their own N
(tools/openrac.py:34, port/tools/versionmap/versionmap.py:44, ...), which breaks when a
script moves. This walks up from a path instead and stops at the first directory that
holds `games/` and one of the two markers below.
"""
from pathlib import Path

# A checkout of OpenRAC has AGENTS.md at its top, beside games/.
CHECKOUT_MARKER = "AGENTS.md"

# The launcher's bundled copy of the tools (tools/bundle_standalone.py) has no AGENTS.md: its top
# holds games/*/game.json and a STANDALONE note, and the tools in it look for their root there.
STANDALONE_MARKER = "STANDALONE"


def is_root(directory: Path) -> bool:
    """Whether DIRECTORY is the top of a checkout of OpenRAC or of the launcher's bundled copy."""
    if not (directory / "games").is_dir():
        return False
    return (directory / CHECKOUT_MARKER).is_file() or (directory / STANDALONE_MARKER).is_file()


def repo_root(start: Path | str | None = None) -> Path:
    """OpenRAC's top directory: START (a file or directory; default this package) or the nearest
    directory above it that is_root accepts. Raises FileNotFoundError when there is none."""
    here = Path(start if start is not None else __file__).resolve()
    for directory in (here, *here.parents):
        if is_root(directory):
            return directory
    raise FileNotFoundError(
        f"{here} is not inside OpenRAC: no directory above it holds games/ and "
        f"{CHECKOUT_MARKER} or {STANDALONE_MARKER}")

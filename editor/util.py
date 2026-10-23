"""The small helpers several of the editor's modules share, one copy of each.

  import util
  util.sha256(data)                           # hex digest of bytes
  util.read_json(path, lambda p, exc: MyError(f"cannot read {p}: {exc}"))
  util.f32(x)                                 # a number rounded to the nearest 32-bit float
  util.cross(a, b)                            # the cross product of two 3-vectors

A helper belongs here when at least two of the editor's modules need it and it carries no format
knowledge of its own: a helper that raises FormatError lives in formats.py, beside the error.
Each was written two or more times over before; its docstring names the modules that had a copy.

The repository's shared helpers are tools/openrac_common, which has sha256 (digest) too. The
editor does not import it yet: the launcher carries the editor in its bundled copy, where the
import line openrac_common's README gives (finding a checkout by AGENTS.md) does not work. P2-10
moves these helpers onto it once that is settled.

Standard library only, no state: every function reads what it is given and returns.
"""

from collections.abc import Callable
import hashlib
import json
from pathlib import Path
import struct


def sha256(data: bytes) -> str:
    """The SHA-256 of DATA, as a hex string (assets.py and overrides.py had one each)."""
    return hashlib.sha256(data).hexdigest()


def read_json(path: Path | str, error: Callable[[Path | str, Exception], Exception]):
    """The JSON value in the file at PATH. When the file cannot be read or is not JSON, raises
    what ERROR makes of PATH and the cause, without the cause chained: each tool keeps its own
    error type and wording at its boundary (assets.py's AssetError, overrides.py's PackError)."""
    try:
        return json.loads(Path(path).read_text())
    except (OSError, json.JSONDecodeError) as exc:
        raise error(path, exc) from None


def f32(x: float) -> float:
    """X rounded to the nearest 32-bit float, as the game stores it (gamedata.py and
    moby_collision.py had one each; pack.py's f32 returns the packed bytes and writes -0 as 0,
    so it stays pack.py's own)."""
    return struct.unpack("<f", struct.pack("<f", x))[0]


def cross(a: tuple, b: tuple) -> tuple:
    """The cross product A x B of two 3-vectors (moby_collision_scene.py and sky.py had one each)."""
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


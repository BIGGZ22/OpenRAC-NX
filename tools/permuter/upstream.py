# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""decomp-permuter itself: a pinned checkout, fetched once into a cache and patched, never vendored.

The engine imports its randomizer and candidate logic (the checkout's `src` package). The cache is
$OPENRAC_PERMUTER_CACHE, else $XDG_CACHE_HOME/openrac, else ~/.cache/openrac; the checkout is
decomp-permuter-<commit> inside it, and a `.openrac-patched` file there records which patches were
applied, so a changed patch makes a fresh checkout. $DECOMP_PERMUTER names a checkout to use
instead, as it is (the caller says whether its passes are there).

    python3 tools/permuter fetch          # clone, check out the pin, apply patches/*.patch
"""
from __future__ import annotations

import hashlib
import os
import shutil
import sys
import types
from pathlib import Path

from openrac_common import proc

URL = "https://github.com/simonlindholm/decomp-permuter"
COMMIT = "8556c81d80d1c1af98a858c8f4dc951357f29139"
PATCHES = Path(__file__).resolve().parent / "patches"
ASM_PASSES = {"perm_reg_fence": 15, "perm_sched_barrier": 10}


def _patches() -> list[Path]:
    return sorted(p for p in PATCHES.glob("*.patch") if not p.name.startswith("."))


def cache_dir() -> Path:
    if os.environ.get("OPENRAC_PERMUTER_CACHE"):
        return Path(os.environ["OPENRAC_PERMUTER_CACHE"]).expanduser()
    base = os.environ.get("XDG_CACHE_HOME") or Path.home() / ".cache"
    return Path(base) / "openrac"


def patch_stamp() -> str:
    """One line per patch: its name and SHA-256, in order."""
    return "".join(f"{p.name} {hashlib.sha256(p.read_bytes()).hexdigest()}\n" for p in _patches())


def checkout() -> Path:
    """The patched checkout, fetched and patched on first use. Raises RuntimeError when git fails."""
    if os.environ.get("DECOMP_PERMUTER"):
        return Path(os.environ["DECOMP_PERMUTER"]).expanduser()
    dest = cache_dir() / f"decomp-permuter-{COMMIT[:7]}"
    stamp = dest / ".openrac-patched"
    if stamp.is_file() and stamp.read_text() == patch_stamp():
        return dest
    if dest.exists():
        shutil.rmtree(dest)
    dest.parent.mkdir(parents=True, exist_ok=True)
    tmp = dest.with_name(dest.name + ".tmp")
    if tmp.exists():
        shutil.rmtree(tmp)
    for args in (["clone", "-q", "--no-checkout", URL, str(tmp)], ["-C", str(tmp), "checkout", "-q", COMMIT]):
        if proc.run(["git", *args], check=False).returncode:
            raise RuntimeError(f"git {' '.join(args)} failed")
    for patch in _patches():
        if proc.run(["git", "-C", str(tmp), "apply", str(patch)], check=False).returncode:
            raise RuntimeError(f"{patch.name} does not apply to decomp-permuter {COMMIT[:7]}")
    (tmp / ".openrac-patched").write_text(patch_stamp())
    tmp.rename(dest)
    return dest


def _toml_shim() -> None:
    """decomp-permuter imports `toml`; stand in with the standard library's tomllib (3.11+)."""
    try:
        import toml  # noqa: F401
        return
    except ImportError:
        pass
    try:
        import tomllib
    except ImportError:
        return
    shim = types.ModuleType("toml")
    shim.load = lambda f: tomllib.loads(f.read() if hasattr(f, "read") else Path(f).read_text())
    shim.loads = tomllib.loads
    sys.modules["toml"] = shim


def load(path: Path) -> types.SimpleNamespace:
    """The checkout's modules: permuter, helpers, randomizer, and strip_other_fns."""
    _toml_shim()
    if str(path) not in sys.path:
        # last, so that its permuter.py never stands in for this package
        sys.path.append(str(path))
    import src.helpers as helpers
    import src.permuter as permuter
    import src.randomizer as randomizer
    import strip_other_fns
    return types.SimpleNamespace(permuter=permuter, helpers=helpers, randomizer=randomizer,
                                 strip_other_fns=strip_other_fns)


def has_asm_passes(modules: types.SimpleNamespace) -> bool:
    names = {p.__name__ for p in modules.randomizer.RANDOMIZATION_PASSES}
    return all(n in names for n in ASM_PASSES)

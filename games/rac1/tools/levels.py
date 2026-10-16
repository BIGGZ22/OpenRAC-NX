#!/usr/bin/env python3
"""
The 19 levels by index, with their planet names: one table for every tool
that names a level's directory or report category (docs/OVERLAYS.md,
"Levels").

The index is the game's own level number: the disc's level table order
(D_0013A548, docs/ASSETS.md), the order of the level overlays in
baserom/overlays/level_NN/, the NN in func_LNN_* / D_LNN_* symbols, and
the order of the save's unlocked-planet flags. Symbols and baserom paths
keep the bare number; only source directories and report labels carry the
planet.

  python3 tools/levels.py        # print the table
"""
from pathlib import Path

# (index, directory slug, display name)
LEVELS = [
    (0, "veldin1", "Veldin"),
    (1, "novalis", "Novalis"),
    (2, "aridia", "Aridia"),
    (3, "kerwan", "Kerwan"),
    (4, "eudora", "Eudora"),
    (5, "rilgar", "Rilgar"),
    (6, "blarg", "Blarg Station (Nebula G34)"),
    (7, "umbris", "Umbris"),
    (8, "batalia", "Batalia"),
    (9, "gaspar", "Gaspar"),
    (10, "orxon", "Orxon"),
    (11, "pokitaru", "Pokitaru"),
    (12, "hoven", "Hoven"),
    (13, "gemlik", "Gemlik Base"),
    (14, "oltanis", "Oltanis"),
    (15, "quartu", "Quartu"),
    (16, "kalebo3", "Kalebo III"),
    (17, "fleet", "Drek's Fleet"),
    (18, "veldin2", "Veldin (return)"),
]
NUM_LEVELS = len(LEVELS)
OVERLAYS_SRC = Path("src/overlays")


def dirname(level: int) -> str:
    """src/overlays/ directory name of LEVEL's own code, e.g. l03_kerwan."""
    return f"l{level:02d}_{LEVELS[level][1]}"


def title(level: int) -> str:
    """Report label, e.g. 'Level 03: Kerwan'."""
    return f"Level {level:02d}: {LEVELS[level][2]}"


def level_of_dir(name: str) -> int | None:
    """Inverse of dirname(): the level of a src/overlays/ directory name."""
    for i in range(NUM_LEVELS):
        if name == dirname(i):
            return i
    return None


if __name__ == "__main__":
    for i, slug, name in LEVELS:
        print(f"{i:2d}  {dirname(i):<14} {name}")

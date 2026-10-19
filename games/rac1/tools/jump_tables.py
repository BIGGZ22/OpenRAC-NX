#!/usr/bin/env python3
"""
Which of retail's jump tables the compiled game functions now provide.

Retail keeps the text objects' read-only data (strings and switch jump
tables) at the end of the data segment. A decompiled function with a
switch brings its own table, which has to land exactly where retail's
was. tools/fix_jump_tables.py puts each compiled table in a section of
its own, named after the retail table (`.rodata.jtbl_001E8C90`);
tools/build_sn_data.sh cuts those tables out of asm/data/data.data.s;
rac1.ld.sh links the pieces and the tables in address order. The tables
are 16-byte aligned and the pieces keep their own alignment, so each one
lands at its retail address and the gaps are zero-filled, as in retail.

  python tools/jump_tables.py labels   # the jtbl_ labels to cut out
  python tools/jump_tables.py ld       # the .data section's contents
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from libgcc_units import SEGMENT_SOURCES  # noqa: E402
import srcnames

FUNC_DEF = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b(func_[0-9A-Fa-f]{8})(?:_r)?\s*\(", re.M)
STUB = re.compile(r"INCLUDE_ASM\([^)]*\b(func_[0-9A-Fa-f]{8})\)")
JTBL = re.compile(r"%hi\((jtbl_(?:L\d\d_)?[0-9A-F]{8})\)")


def retail_tables(func: str) -> list[str]:
    """The jump tables retail's FUNC uses, in the order it first uses them."""
    for segment in ("text", "core_text"):
        p = Path(f"asm/nonmatchings/{segment}/{func}.s")
        if p.exists():
            return list(dict.fromkeys(JTBL.findall(p.read_text())))
    # level code (docs/OVERLAYS.md): a joined function's pieces may hold its tables
    found = []
    for part in [func] + joined_pieces(func):
        p = Path(f"asm/overlays/{part}.s")
        if p.exists():
            found += JTBL.findall(p.read_text())
    return list(dict.fromkeys(found))


def joined_pieces(func: str) -> list[str]:
    """The catalogue entries config/overlays/joined.tsv joins to FUNC."""
    p = Path("config/overlays/joined.tsv")
    if not p.exists():
        return []
    for line in p.read_text().splitlines():
        if line and not line.startswith("#"):
            owner, pieces = line.split("\t", 1)
            if owner == func:
                return pieces.split()
    return []


def provided() -> list[tuple[str, str, str]]:
    """(label, function, object) for every table a compiled function brings."""
    out = []
    for src in SEGMENT_SOURCES["text"]:
        text = srcnames.read_c(src, errors="replace")
        stubs = set(STUB.findall(text))
        obj = "build-sn/" + Path(src).with_suffix(".o").as_posix().removeprefix("src/")
        for func in dict.fromkeys(FUNC_DEF.findall(text)):
            if func not in stubs:
                out += [(label, func, obj) for label in retail_tables(func)]
    return sorted(out)


def main() -> None:
    tables = provided()
    if sys.argv[1:] == ["labels"]:
        print(" ".join(label for label, _, _ in tables))
    elif sys.argv[1:] == ["ld"]:
        # split_data_s.py numbers the pieces around the cuts from 1.
        for n, (label, _, obj) in enumerate(tables, 1):
            print(f"    build-sn/data_{n}.o(.data)")
            print(f"    {obj}(.rodata.{label})")
        print(f"    build-sn/data_{len(tables) + 1}.o(.data)")
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()

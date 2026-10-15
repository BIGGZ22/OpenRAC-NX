#!/usr/bin/env python3
"""
Give each jump table in compiled game code a section of its own, named
after the retail table it replaces, so rac1.ld.sh can place it at
retail's address (see tools/jump_tables.py).

GCC writes a switch's table into `.rdata` right after the indirect jump:

    .rdata
    .align  4
  $L12:
    .word   $L8
    ...
    .text

The k-th table in a function becomes `.rodata.<k-th jtbl_ label that
retail's copy of the function uses>`. A compiled table with no retail
counterpart cannot match, and is an error rather than an orphan section
the linker would put somewhere of its own choosing. So is any other
read-only data (a string literal): nothing places it at its retail
address yet, so game code declares its strings `extern`.

usage: python tools/fix_jump_tables.py IN.s OUT.s
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from jump_tables import retail_tables  # noqa: E402

ENT = re.compile(r"^\s*\.ent\s+(\S+)")
END = re.compile(r"^\s*\.end\s+(\S+)")
RDATA = re.compile(r"^\s*\.rdata\s*$")
TABLE_WORD = re.compile(r"^\s*\.(word|gpword)\s+\$L\d+\s*$")


def is_table(lines: list[str], i: int) -> bool:
    """`.rdata`, an optional `.align`, a label, then `.word $Ln` entries."""
    j = i + 1
    if j < len(lines) and lines[j].strip().startswith(".align"):
        j += 1
    return (j + 1 < len(lines) and re.match(r"^\$L\d+:\s*$", lines[j].strip()) is not None
            and TABLE_WORD.match(lines[j + 1]) is not None)


def main() -> None:
    src, dst = sys.argv[1:3]
    lines = Path(src).read_text().splitlines(keepends=True)
    func, used, moved = None, 0, 0
    for i, line in enumerate(lines):
        if (m := ENT.match(line)):
            func, used = m.group(1), 0
        elif END.match(line):
            func = None
        elif RDATA.match(line) and not (func and is_table(lines, i)):
            sys.exit(f"fix_jump_tables: error: {src}:{i + 1}: read-only data that is not a jump "
                     f"table (a string literal?) -- declare it extern instead")
        elif RDATA.match(line):
            labels = retail_tables(func)
            if used >= len(labels):
                sys.exit(f"fix_jump_tables: error: {func} has jump table #{used + 1}, retail's has "
                         f"{len(labels)} -- it cannot match")
            lines[i] = f'\t.section\t.rodata.{labels[used]},"a"\n'
            used += 1
            moved += 1
    Path(dst).write_text("".join(lines))
    print(f"fix_jump_tables: {moved} jump table(s) {src} -> {dst}")


if __name__ == "__main__":
    main()

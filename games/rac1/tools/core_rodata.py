#!/usr/bin/env python3
"""
The core_rdata pieces that compiled objects provide (config/core_rodata.txt).

  python tools/core_rodata.py labels   # labels to cut, for split_data_s.py
  python tools/core_rodata.py ld       # the .core_rdata section's contents

split_data_s.py starts a new piece at every cut label, so with labels
L1..Ln in address order the pieces are build-sn/core_rdata_1..n+1, and
piece k+1 follows label k. An object's .rodata goes after the piece that
ends at its first label; the pieces between its own labels are empty.
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def entries() -> list[tuple[str, list[str]]]:
    out = []
    for line in (ROOT / "config/core_rodata.txt").read_text().splitlines():
        line = line.split("#")[0].split()
        if line:
            out.append((line[0], line[1:]))
    return out


def main() -> None:
    rows = sorted(((int(lab.rsplit("_", 1)[1], 16), lab, obj if i == 0 else None)
                   for obj, labs in entries() for i, lab in enumerate(labs)))
    if sys.argv[1:] == ["labels"]:
        print(" ".join(lab for _, lab, _ in rows))
    elif sys.argv[1:] == ["ld"]:
        print("    build-sn/core_rdata_1.o(.rodata)")
        for k, (_, _, obj) in enumerate(rows, 2):
            if obj:
                print(f"    {obj}(.rodata)")
            print(f"    build-sn/core_rdata_{k}.o(.rodata)")
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()

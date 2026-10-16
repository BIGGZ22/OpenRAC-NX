#!/usr/bin/env python3
"""
The link's undefined symbols, read from the objects themselves.

  python tools/list_undefined.py [build-sn/rac1.ld]

Prints, sorted, every symbol that an object in the link script references
and that no object defines (globally) and the script does not assign.
tools/build_sn.sh equates these to their addresses (tools/gen_bss_equs.py).
Reading them from the objects, rather than from ld's "undefined reference"
errors, matters because this ld can crash on a long run of those errors,
sometimes before printing any.
"""
import re
import sys
from pathlib import Path

from elftools.elf.elffile import ELFFile


def main() -> None:
    script = Path(sys.argv[1] if len(sys.argv) > 1 else "build-sn/rac1.ld").read_text()
    objects = dict.fromkeys(re.findall(r"(build-sn/[\w./-]+\.o)", script))
    assigned = set(re.findall(r"^\s*([A-Za-z_][\w.$]*)\s*=[^=]", script, re.M))
    defined, wanted = set(assigned), set()
    for path in objects:
        with open(path, "rb") as f:
            symtab = ELFFile(f).get_section_by_name(".symtab")
            for sym in symtab.iter_symbols():
                if not sym.name:
                    continue
                if sym["st_shndx"] == "SHN_UNDEF":
                    wanted.add(sym.name)
                elif sym["st_info"]["bind"] in ("STB_GLOBAL", "STB_WEAK"):
                    defined.add(sym.name)
    for name in sorted(wanted - defined):
        print(name)


if __name__ == "__main__":
    main()

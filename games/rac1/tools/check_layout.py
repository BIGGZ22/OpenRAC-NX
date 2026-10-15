#!/usr/bin/env python3
"""
Verify that every function in the linked ELF sits at its retail address.

splat names each function func_<VRAM_HEX>, so the symbol name *is* the
expected address -- no baserom needed. Any symbol that has drifted means
bytes were lost or gained upstream of it, and the usual cause is
post-endlabel padding: a stub's .s file carries inter-function nops after
`endlabel`, and replacing that stub with C silently drops them. Every
function after the gap then has wrong `jal` targets while being
instruction-for-instruction identical to retail, which reads as a pile of
unrelated regressions.

This is the fast way to tell "I broke this function" from "I broke the
layout": run it after every conversion. It reports the FIRST drifting
symbol, which is the one immediately after the function that lost bytes.

Usage: python tools/check_layout.py [build-sn/rac1.elf]
"""
import re
import sys
from elftools.elf.elffile import ELFFile

NAME = re.compile(r"^func_([0-9A-F]{8})$")


def main() -> int:
    path = sys.argv[1] if len(sys.argv) > 1 else "build-sn/rac1.elf"
    with open(path, "rb") as f:
        elf = ELFFile(f)
        syms = elf.get_section_by_name(".symtab")
        if syms is None:
            print("no .symtab in", path)
            return 2
        drifted = []
        total = 0
        for sym in syms.iter_symbols():
            m = NAME.match(sym.name)
            if not m:
                continue
            total += 1
            want = int(m.group(1), 16)
            got = sym["st_value"]
            if got != want:
                drifted.append((want, got, sym.name))

    print("%d functions checked" % total)
    if not drifted:
        print("every function is at its retail address")
        return 0

    drifted.sort()
    print("%d at the WRONG address -- layout has drifted" % len(drifted))
    want, got, name = drifted[0]
    print("\nfirst drift: %s is at 0x%X, should be 0x%X (%+d bytes)"
          % (name, got, want, got - want))
    print("The function immediately BEFORE %s is the one that lost or\n"
          "gained bytes -- check its .s for nops after `endlabel` and\n"
          "emit them explicitly, the way func_001F6668's stub does." % name)
    return 1


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
The SN Systems ProDG GCC 2.95.3 EE assembler (the era-accurate compiler for
this game -- see docs/TOOLCHAIN.md) does not recognize symbolic GPR names
($ra, $sp, $t6, ...) at all ("illegal operands"), only numeric ($31, $29,
$14). It DOES recognize VU0 macro-mode COP2 vector instructions and their
symbolic float-register names ($vf0-$vf31, $vi0-$vi15, Q, ACC, I, R)
natively -- unlike modern binutils, no workaround needed for those.

This rewrites only the integer GPR names splat/spimdisasm emit into their
numeric equivalents, leaving everything else (mnemonics, VU float regs,
labels, comments) untouched. Verified byte-for-byte round-trip against a
real disassembled function from the retail binary before relying on this
for anything -- see docs/TOOLCHAIN.md.
"""
import re
import sys
from pathlib import Path

GPR = {
    "zero": 0, "at": 1, "v0": 2, "v1": 3,
    "a0": 4, "a1": 5, "a2": 6, "a3": 7,
    "t0": 8, "t1": 9, "t2": 10, "t3": 11, "t4": 12, "t5": 13, "t6": 14, "t7": 15,
    "s0": 16, "s1": 17, "s2": 18, "s3": 19, "s4": 20, "s5": 21, "s6": 22, "s7": 23,
    "t8": 24, "t9": 25, "k0": 26, "k1": 27,
    "gp": 28, "sp": 29, "fp": 30, "s8": 30, "ra": 31,
}

# $name, longest names first so e.g. $s8 doesn't get clipped by $s.
_NAMES = sorted(GPR, key=len, reverse=True)
REG_RE = re.compile(r"\$(" + "|".join(_NAMES) + r")\b")


def convert_text(text: str) -> str:
    return REG_RE.sub(lambda m: f"${GPR[m.group(1)]}", text)


def main() -> None:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "asm")
    changed = 0
    for path in sorted(root.rglob("*.s")):
        text = path.read_text()
        new = convert_text(text)
        if new != text:
            path.write_text(new)
            changed += 1
    print(f"Rewrote GPR names in {changed} file(s)")


if __name__ == "__main__":
    main()

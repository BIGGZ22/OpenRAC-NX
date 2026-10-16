#!/usr/bin/env python3
"""
Resolve orphan %hi relocations at assembly time.

  python tools/fix_orphan_hi.py IN.s OUT.s

Loop optimisation sometimes hoists a global's `lui $r,%hi(D_X)` out of a
loop and then never uses it with a %lo (func_001E9808 copies it to a
saved register that nothing reads). Retail's linker filled that `lui`
with the right high half. Ours doesn't: the assembler emits HI16, HI16,
LO16 for the symbol, and this ee-ld resolves the first, unpaired HI16 to
the low half of the address instead.

For every `%hi(SYM)` whose next reference to SYM in the file is another
`%hi(SYM)` or none at all (no `%lo(SYM)` in between), this writes the high
half as a constant. It only does that when SYM names its own address
(D_XXXXXXXX, func_XXXXXXXX, optionally +offset), so the value is known
here; other orphans are left alone.
"""
import re
import sys

REF = re.compile(r"%(hi|lo)\(((?:D|func)_([0-9A-Fa-f]{8}))((?:[+-](?:0x[0-9A-Fa-f]+|\d+))?)\)")


def main():
    src, dst = sys.argv[1], sys.argv[2]
    lines = open(src).read().split("\n")
    refs = []  # (line index, kind, symbol, match)
    for i, line in enumerate(lines):
        code = line.split("#", 1)[0]
        for m in REF.finditer(code):
            refs.append((i, m.group(1), m.group(2), m))
    fixes = {}
    for k, (i, kind, sym, m) in enumerate(refs):
        if kind != "hi":
            continue
        nxt = next((r for r in refs[k + 1:] if r[2] == sym), None)
        if nxt is None or nxt[1] == "hi":
            addr = int(m.group(3), 16) + (int(m.group(4), 0) if m.group(4) else 0)
            fixes.setdefault(i, []).append((m.group(0), hex(((addr + 0x8000) >> 16) & 0xFFFF)))
    for i, subs in fixes.items():
        for old, new in subs:
            lines[i] = lines[i].replace(old, new, 1)
    open(dst, "w").write("\n".join(lines))


if __name__ == "__main__":
    main()

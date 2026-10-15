#!/usr/bin/env python3
"""
Handles a `MACRO_ADDR` global access that the compiler put in a branch
delay slot: a load or store there is emitted $gp-relative, as retail's
toolchain did; anything else fails the build.

Why this exists
---------------
`MACRO_ADDR` (include/common.h) makes the compiler emit an unsplit
global access such as `sw $2,D_0015F020`. The assembler expands that to
two instructions (`lui $at` / `sw`), but the compiler counts it as one,
so it may place it in a delay slot. The compiler brackets delay slots
with `.set nomacro`, and our assembler only *warns* there ("macro used
after .set nomacro") and emits broken code: the second instruction falls
out of the slot.

Retail shows what its toolchain did instead. Of the 524 $gp-relative
accesses (in compiled, not hand-written, functions) to globals that
retail ALSO reaches through `lui` elsewhere, 505 sit in a branch delay
slot. The same variable is `lui`/`lw` in the body of a function and
`lw $x,off($gp)` in that function's delay slots (menu.cpp's
func_00209188 does both to D_0015EFB0 and D_0015EFB4). So a
one-instruction macro access in a slot came out $gp-relative. This
script does the same to our compiler's output, before it is assembled:

    lw  $2,D_0015EFB4        (between .set nomacro / .set macro)
 -> lw  $2,D_0015EFB4__gp

It only touches instructions that would otherwise be broken, so it
cannot change a function that assembles today. If the symbol is outside
the 64 KiB $gp window the link fails with a truncated GPREL16
relocation, which is loud. An `la`/`dla` in a slot is still rejected:
retail's form for that has not been established.

Symbols the assembler already treats as small (.extern size <= -G) are
one gp-relative instruction anyway and are left alone. A symbol declared
more than once with different sizes (a candidate's `extern short` alias
beside the file's own 4-byte declaration, say) counts as small only if
EVERY declaration is small: the assembler goes by the last `.extern` it
reads for the name (measured 2026-10-09: 4 then 2 assembles one
$gp-relative instruction, 2 then 4 a lui pair), which may be the 4-byte
one, so the access is taken as a two-instruction macro and rewritten.
Where the last one is the small one the rewrite gives the instruction
the assembler would have given anyway.

A scalar the C declares as the file's own small data (`SDATA(sym)` in
include/common.h: its assembler label is `sym__gp`) gets the same
treatment at every access, not only in delay slots: the label's .extern
is made 1 byte and the label is equated to the symbol, so the assembler
emits one $gp-relative instruction wherever the compiler wrote the
access. Retail reaches a level file's own tuning floats and ints that
way throughout, and they have to be declared with their real type: a
read through a cast of an `extern short` is not a scalar to the
compiler, which then orders the load behind every store through a
pointer (func_L17_002EEB08, func_L17_002EDE50).

Usage: python tools/check_macro_slots.py file.s   (rewrites in place)
"""
import re
import sys

# A memory operand that is a bare symbol (optionally +offset), not
# `off($reg)` or `%lo(sym)($reg)`. gcc spells FP loads and stores l.s/s.s
# (l.d/s.d) as often as lwc1/swc1.
MACRO = re.compile(
    r"^(\s*)(l[bhwd]u?|s[bhwd]|l[wd]c1|s[wd]c1|[ls]\.[sd]|la|dla)(\s+)(\$\w+)\s*,\s*"
    r"(?!%)(?![-+]?\d)([A-Za-z_.$][\w.$]*(?:\s*[-+]\s*\d+)?)\s*$"
)


G = 2  # the -G the build passes (Makefile.sn CFLAGS)


def main(path: str) -> int:
    bad = []
    fixed = 0
    aliased = set()
    nomacro = False
    with open(path) as f:
        lines = f.readlines()
    sizes = {}
    for line in lines:
        m = re.match(r"^\s*\.extern\s+([\w.$]+)\s*,\s*(\d+)", line)
        if m:
            sizes.setdefault(m.group(1), []).append(int(m.group(2)))
    small = {sym for sym, seen in sizes.items()
             if seen and all(0 < n <= G for n in seen)}
    # Labels the C itself declared as small data (SDATA in common.h).
    declared = {sym[:-len("__gp")] for sym in sizes if sym.endswith("__gp")}
    for n, line in enumerate(lines):
        m = re.match(r"^(\s*\.extern\s+)([\w.$]+__gp)\s*,\s*\d+", line)
        if m:
            lines[n] = "%s%s, 1\n" % (m.group(1), m.group(2))
    small |= {sym + "__gp" for sym in declared}
    for n, line in enumerate(lines):
        s = line.strip()
        if s.startswith(".set"):
            if s.split()[1:] == ["nomacro"]:
                nomacro = True
            elif s.split()[1:] == ["macro"]:
                nomacro = False
            continue
        m = MACRO.match(line.rstrip("\n"))
        if not (nomacro and m):
            continue
        ws, op, sp, reg, ref = m.groups()
        sym = re.split(r"[\s+-]", ref)[0]
        if sym in small:
            continue
        if op in ("la", "dla"):
            bad.append((n + 1, s))
            continue
        ref = ref.replace(" ", "")
        aliased.add(sym)
        lines[n] = "%s%s%s%s,%s\n" % (ws, op, sp, reg,
                                      sym + "__gp" + ref[len(sym):])
        fixed += 1
    for n, s in bad:
        print("%s:%d: address macro in a delay slot: %s" % (path, n, s))
    if bad:
        print("*** an `la` of a MACRO_ADDR symbol landed in a delay slot; "
              "retail's form for that is not established")
        return 1
    if fixed or declared:
        head = []
        for sym in sorted(aliased | declared):
            head.append("\t.extern %s__gp, 1\n" % sym)
            head.append("\t%s__gp = %s\n" % (sym, sym))
        # At the END of the file: directives placed before gcc's own
        # leading directives make the assembler report st_size 0 for
        # some functions later in the file (measured: func_00209040 and
        # func_00209698 in menu.o), which the sweep then reads as a size
        # mismatch. Appended, the sizes are right and the equates still
        # resolve.
        with open(path, "w") as f:
            f.writelines(lines + head)
        print("check_macro_slots: %d delay-slot access(es) made $gp-relative,"
              " %d declared small-data scalar(s) in %s"
              % (fixed, len(declared), path))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))

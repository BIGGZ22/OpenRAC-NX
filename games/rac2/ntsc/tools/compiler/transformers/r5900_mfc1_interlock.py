#!/usr/bin/env python3
"""Drop the coprocessor-load nop after mfc1 on the R5900 in gas/config/tc-mips.c.

The R5900 interlocks a move from a floating-point register: in the retail boot's .text, 88 mfc1
are directly followed by an instruction that reads their target, and the only mfc1 with a nop
before such a reader is in a hand-written VU0 routine (func_00283248). This assembler gave
every mfc1 the load delay of the MIPS I coprocessor move, so no function with that pair could
match from C. Only mfc1 is exempted: cfc2, the other move measured, has the nop in retail
(docs/COMPILER-NOTES.md, the mfc1 interlock)."""
import sys
from pathlib import Path

path = Path(sys.argv[1])
source = path.read_text(encoding="utf-8")
old = ("\t  && (((prev_pinfo & INSN_LOAD_COPROC_DELAY)\n"
       "               && ! cop_interlocks)\n")
new = ("\t  && (((prev_pinfo & INSN_LOAD_COPROC_DELAY)\n"
       "               && ! cop_interlocks\n"
       "               && ! (mips_cpu == 5900\n"
       "                     && strcmp (prev_insn.insn_mo->name, \"mfc1\") == 0))\n")
assert source.count(old) == 1, ("the coprocessor-load test in append_insn", source.count(old))
path.write_text(source.replace(old, new), encoding="utf-8", newline="")

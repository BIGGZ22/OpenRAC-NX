#!/usr/bin/env python3
"""The earlier transfer-hazard helper rac2_mtc1_nop_ok() in gas/config/tc-mips.c, which
restrict_mtc1_exemption.py then narrows (docs/COMPILER-NOTES.md, the tc-mips.c rows)."""
import sys
from pathlib import Path
tc_mips = Path(sys.argv[1])


def edit(path, old, new, count=1):
    text = path.read_text(encoding="utf-8")
    assert text.count(old) == count, (path, old[:60], text.count(old))
    path.write_text(text.replace(old, new), encoding="utf-8", newline="")


# U4: earlier transfer-hazard helper (no argument), at both FPR-write sites.
edit(tc_mips,
     "static int insn_uses_fpr_exact PARAMS ((struct mips_cl_insn *ip,\n"
     "\t\t\t\t\t      unsigned int reg));\n",
     "static int insn_uses_fpr_exact PARAMS ((struct mips_cl_insn *ip,\n"
     "\t\t\t\t\t      unsigned int reg));\n"
     "static int rac2_mtc1_nop_ok PARAMS ((void));\n")
for field in ("FT", "FS"):
    edit(tc_mips,
         "\t      if (mips_optimize == 0\n"
         "\t\t  || insn_uses_fpr_exact (ip,\n"
         f"\t\t\t\t\t   ((prev_insn.insn_opcode >> OP_SH_{field})\n"
         f"\t\t\t\t\t    & OP_MASK_{field})))\n"
         "\t\t++nops;\n",
         "\t      if ((mips_optimize == 0\n"
         "\t\t  || insn_uses_fpr_exact (ip,\n"
         f"\t\t\t\t\t   ((prev_insn.insn_opcode >> OP_SH_{field})\n"
         f"\t\t\t\t\t    & OP_MASK_{field})))\n"
         "\t\t\t\t\t   && rac2_mtc1_nop_ok ())\n"
         "\t\t++nops;\n")
edit(tc_mips,
     "      return 1;\n  return 0;\n}\n\nstatic void\nmacro_build (char *place,\n",
     "      return 1;\n  return 0;\n}\n\n"
     "/* RAC2 : earlier transfer-delay rule (reconstructed). */\n"
     "static int\n"
     "rac2_mtc1_nop_ok ()\n"
     "{\n"
     "  if (prev_insn.insn_mo == 0 || strcmp (prev_insn.insn_mo->name, \"mtc1\") != 0)\n"
     "    return 1;\n"
     "  if (prev_prev_insn.insn_mo == 0 || prev_prev_insn.insn_mo == &dummy_opcode)\n"
     "    return 0;\n"
     "  return 1;\n"
     "}\n\n"
     "static void\nmacro_build (char *place,\n")

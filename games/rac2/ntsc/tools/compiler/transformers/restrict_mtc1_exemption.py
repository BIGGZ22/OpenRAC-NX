"""Restrict the existing transfer-delay exemption to measured operand roles."""
from pathlib import Path
import sys

path = Path(sys.argv[1])
source = path.read_text(encoding="utf-8")
declaration = "static int rac2_mtc1_nop_ok PARAMS ((void));"
assert source.count(declaration) == 1, "Expected the earlier no-argument helper"
assert source.count("rac2_mtc1_nop_ok ())") == 2, "Expected both transfer-hazard sites"
source = source.replace(declaration, "static int rac2_mtc1_nop_ok PARAMS ((struct mips_cl_insn *ip));")
source = source.replace("rac2_mtc1_nop_ok ())", "rac2_mtc1_nop_ok (ip))")
start = source.index("\nstatic int\nrac2_mtc1_nop_ok ()")
comment = source.rfind("\n/*", 0, start)
assert comment >= 0 and source[comment:start].count("*/") == 1
end = source.index("\n}\n", start) + 3
replacement = '''
/* Private compatibility restricted to measured producer/consumer roles.
   Cleared history is not a function boundary. No game name/address policy. */
static int
rac2_mtc1_nop_ok (ip)
     struct mips_cl_insn *ip;
{
  unsigned int gpr, fpr;
  if (prev_insn.insn_mo == 0 || strcmp (prev_insn.insn_mo->name, "mtc1") != 0)
    return 1;
  if (prev_prev_insn.insn_mo != 0 && prev_prev_insn.insn_mo != &dummy_opcode)
    return 1;
  gpr = (prev_insn.insn_opcode >> OP_SH_RT) & OP_MASK_RT;
  fpr = (prev_insn.insn_opcode >> OP_SH_FS) & OP_MASK_FS;
  if (fpr != 0 || ip->insn_mo == 0)
    return 1;
  if (gpr == 4 && strcmp (ip->insn_mo->name, "cvt.s.w") == 0
      && ((ip->insn_opcode >> OP_SH_FS) & OP_MASK_FS) == 0
      && ((ip->insn_opcode >> OP_SH_FD) & OP_MASK_FD) == 0)
    return 0;
  if (gpr == 0 && strcmp (ip->insn_mo->name, "c.lt.s") == 0
      && ((ip->insn_opcode >> OP_SH_FS) & OP_MASK_FS) == 12
      && ((ip->insn_opcode >> OP_SH_FT) & OP_MASK_FT) == 0)
    return 0;
  return 1;
}
'''
path.write_text(source[:comment] + replacement + source[end:], encoding="utf-8", newline="")
print("Transfer-delay exemption restricted to the qualified operand patterns")

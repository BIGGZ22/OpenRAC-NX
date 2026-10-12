"""Emit the GPR save loop in ascending register order, as the retail does.

The released source walks `GP_REG_LAST` down to `GP_REG_FIRST`; the retail
saves `s0, s1, ..., ra` upward with the same layout and the same offsets, which
needs the multi-register displacement pre-computed and the per-register
decrement turned into an increment. The annotation text is part of the
qualified source identity: a differently worded comment produces different
bytes and therefore a different `mips.c` hash.
"""
from pathlib import Path
import sys

path = Path(sys.argv[1])
source = path.read_text(encoding="utf-8")
old = "      for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)\n\tif (BITSET_P (mask, regno - GP_REG_FIRST))\n\t  {\n"
assert source.count(old) == 1, "Expected the descending save loop"
new = """      /* RAC2 : emission en ordre CROISSANT (comme le retail) ; la boucle
\t d origine descend depuis $ra. Meme disposition, offsets identiques. */
      {
\tint n_rac2 = 0, rr_rac2;
\tfor (rr_rac2 = GP_REG_FIRST; rr_rac2 <= GP_REG_LAST; rr_rac2++)
\t  if (BITSET_P (mask, rr_rac2 - GP_REG_FIRST))
\t    n_rac2++;
\tif (n_rac2 > 1)
\t  gp_offset -= GET_MODE_SIZE (mips_reg_mode[0]) * (n_rac2 - 1);
      }
      for (regno = GP_REG_FIRST; regno <= GP_REG_LAST; regno++)
\tif (BITSET_P (mask, regno - GP_REG_FIRST))
\t  {
"""
source = source.replace(old, new)
old2 = "gp_offset -= GET_MODE_SIZE (mips_reg_mode[0]);"
assert source.count(old2) == 1, "Expected the per-register save decrement"
source = source.replace(old2, "gp_offset += GET_MODE_SIZE (mips_reg_mode[0]);")
path.write_text(source, encoding="utf-8", newline="")
print("GPR save loop emits in ascending order with unchanged offsets")

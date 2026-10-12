"""Emit the two save/restore blocks in the stock order: GP first, FP second.

The released 2.9 source emits the GPR block and the FPR block from one pass.
This transformer keeps both intact blocks, their offsets, their directions and
their shared save-area base, but drives them from a two-pass loop whose
selector puts the **GP block in pass 0 and the FP block in pass 1, for stores
and restores alike** -- the order the original SCE source emits.

The rule this replaces forced the FPR block first on saves and the GPR block
first on restores. That is not what the retail image does: of the frames that
save at least one GPR and at least one FPR, the first save is a GPR in 92.3 % of
`0_aranos_tutorial`'s 1 102 mixed frames, 90.6 % of the boot's 533 and 92.5 % of
all 31 278 across the 28 programs. The regression was measured on
3 October 2026: the `cc1` built at 00:13 that day emits GP first and the one
built at 06:23 does not, so the inversion entered with the save-block recipe
itself.

Do not restore a pre-patch `mips.c` to undo this: the qualified `mips.c` also
carries `fold_zero_ti_store.py` (`rac2_fold_zero_ti_store`, referenced from
`toplev.c`), and a build without it fails to link with
`undefined reference to rac2_fold_zero_ti_store`. The reversion belongs here,
inside the recipe that introduced the inversion.

`ascending_save_order.py` must still run first: this transformer rewrites the
region that one has already produced. The two block bodies are order
symmetric -- each initialises the shared base when it runs first and reuses an
already established base when it runs second -- so the selector alone decides
the emitted order.
"""
from pathlib import Path
import sys

MARKER = "RAC2 : GP block first in both directions"
p = Path(sys.argv[1])
s = p.read_text(encoding="utf-8")
a = s.index("  /* Save GP registers if needed.  */")
b = s.index("  /* Save floating point registers if needed.  */", a)
c = s.index("\n}\n\f", b)
gp, fp = s[a:b], s[b:c]
assert "for (regno = GP_REG_FIRST; regno <= GP_REG_LAST; regno++)" in gp
assert "regno -= fp_inc)" in fp
assert MARKER not in s
fp = fp.replace("already set up for gp registers above", "reuse an already established save-area base")
body = """  /* RAC2 : GP block first in both directions, as the stock source emits it.
     Keep both emission blocks and their offsets/directions intact.  The
     first GP pass needs the base initialized before either block runs.  */
  {
    int rac2_save_pass;
    base_reg_rtx = 0;
    base_offset = 0;
    for (rac2_save_pass = 0; rac2_save_pass < 2; rac2_save_pass++)
      {
        if (rac2_save_pass != 0)
          {
"""
body += "\n".join("          " + line if line else "" for line in fp.splitlines())
body += "\n          }\n        else\n          {\n"
body += "\n".join("          " + line if line else "" for line in gp.splitlines())
body += "\n          }\n      }\n  }\n"
p.write_text(s[:a] + body + s[c:], encoding="utf-8", newline="")
print("GP save block first; GPR/FPR restores in stock order")

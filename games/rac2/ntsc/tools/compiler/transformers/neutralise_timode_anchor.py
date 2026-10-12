"""Neutralise the r5900 save-mode anchor so GPR slots stay word sized.

The released source widens the callee-saved GPR slots to TImode for
`TARGET_MIPS5900`; the retail saves them in 8-byte `sd` slots. The anchor is
disabled in place rather than deleted, so the surrounding comment and the
assignment keep their positions. The annotation text is part of the qualified
source identity: a differently worded comment produces different bytes and
therefore a different `mips.c` hash, even though the code is unchanged.
"""
from pathlib import Path
import re
import sys

path = Path(sys.argv[1])
source = path.read_text(encoding="utf-8")
pattern = r"^  if \(TARGET_MIPS5900\)\n(?=    mips_reg_mode\[0\] = TImode;)"
assert len(re.findall(pattern, source, re.MULTILINE)) == 1, "Expected the r5900 save anchor"
source = re.sub(pattern, "  if (0 && TARGET_MIPS5900) /* RAC2 : ancre TImode neutralisee */\n",
                source, count=1, flags=re.MULTILINE)
path.write_text(source, encoding="utf-8", newline="")
print("TImode save anchor neutralised; GPR slots stay word sized")

"""Count a R5900 division guard's machine-description length in loop padding."""
from pathlib import Path
import sys

path=Path(sys.argv[1])
source=path.read_text(encoding="utf-8")
start=source.index("void\nmips_r5900_pad_loops (first)")
end=source.index("/* ASTRA P25:",start)
fragment=source[start:end]
old="\t  n++;"
new="\t  /* Use the MD word count for a division-guard expansion. */\n\t  n += pat == TRAP_IF ? get_attr_length (insn) : 1;"
assert fragment.count(old)==1, "Unexpected or already patched loop counter"
path.write_text(source[:start]+fragment.replace(old,new)+source[end:],encoding="utf-8",newline="")
print("Division guards use their MD instruction length in post-DBR padding")

"""Restore GNU-EE post-delay-slot short-loop padding after the 0054 patch."""
from pathlib import Path
import sys

path=Path(sys.argv[1])
source=path.read_text(encoding="utf-8")
anchor="extern void mips_r5900_pad_loops ();"
assert source.count(anchor)==1, "Unexpected compiler header"
assert "#define MACHINE_DEPENDENT_REORG_AFTER_DBR(X)" not in source, "Hook already present"
hook="#define MACHINE_DEPENDENT_REORG_AFTER_DBR(X) mips_r5900_pad_loops (X)\n"
path.write_text(source.replace(anchor,hook+anchor),encoding="utf-8",newline="")
print("Post-DBR loop-padding hook restored; other 0054 changes preserved")

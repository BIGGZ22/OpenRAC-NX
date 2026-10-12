"""Keep frame-order forcing opt-in, with an English source annotation."""
from pathlib import Path
import re
import sys

path = Path(sys.argv[1])
source = path.read_text(encoding="utf-8")
pattern = r'^char \*mips_astra_keep_frame_order = "1";[^\n]*$'
assert len(re.findall(pattern, source, re.MULTILINE)) == 1, "Expected the previously enabled default"
source = re.sub(pattern, "char *mips_astra_keep_frame_order = 0; /* RAC2: default OFF */",
                source, count=1, flags=re.MULTILINE)
path.write_text(source, encoding="utf-8", newline="")
print("Generic frame scheduling is the default; explicit opt-in remains available")

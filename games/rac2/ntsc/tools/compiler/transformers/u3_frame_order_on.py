#!/usr/bin/env python3
"""The earlier step that turned the frame-order option on by default, which
disable_frame_order_default.py later turns off again (docs/COMPILER-NOTES.md)."""
import sys
from pathlib import Path
p = Path(sys.argv[1])
t = p.read_text(encoding="utf-8")
old = "char *mips_astra_keep_frame_order;\n"
assert t.count(old) == 1
p.write_text(t.replace(old, 'char *mips_astra_keep_frame_order = "1"; /* RAC2 : defaut ON */\n'), encoding="utf-8", newline="")

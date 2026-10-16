#!/usr/bin/env python3
"""
Rewrite denormal `.float` data as `.word`, from the raw bytes spimdisasm
records beside each line.

spimdisasm prints every data word that looks like a float as `.float`
with a decimal value, and the build's assembler (GNU ee-as 2.9) does not
read denormals back exactly: `.float 1.401298464e-45` (the word
0x00000001) assembles to 0, and 0x200 comes out as 0x1FF. Before this
ran, 66 words of the data segments differed from retail for this reason
alone, and the function audit, which looks only at code, never showed
it. Only the denormals (exponent bits zero, mantissa not) are touched;
every other float round-trips.

Idempotent: a converted line is a `.word` and is not matched again.

usage: python tools/fix_denormal_floats.py asm
"""
import re
import sys
from pathlib import Path

# `/* ROM VRAM RAWBYTES */ .float VALUE`, the bytes little-endian.
LINE = re.compile(r"^(?P<head>\s*/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+(?P<raw>[0-9A-Fa-f]{8})\s*\*/\s*)"
                  r"\.float\s+(?P<value>\S+)\s*$")


def denormal(word: int) -> bool:
    return (word >> 23) & 0xFF == 0 and word & 0x7FFFFF != 0


def main() -> None:
    root = Path(sys.argv[1])
    total = files = 0
    for path in sorted(root.rglob("*.s")):
        lines = path.read_text().splitlines(keepends=True)
        changed = 0
        for i, line in enumerate(lines):
            m = LINE.match(line.rstrip("\n"))
            if not m:
                continue
            word = int.from_bytes(bytes.fromhex(m.group("raw")), "little")
            if denormal(word):
                lines[i] = f"{m.group('head')}.word 0x{word:08X} /* .float {m.group('value')} */\n"
                changed += 1
        if changed:
            path.write_text("".join(lines))
            total += changed
            files += 1
    print(f"fix_denormal_floats: {total} denormal float(s) in {files} file(s) -> .word")


if __name__ == "__main__":
    main()

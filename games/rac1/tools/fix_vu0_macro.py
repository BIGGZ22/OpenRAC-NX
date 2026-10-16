#!/usr/bin/env python3
"""
Modern binutils (2.45.1, as shipped by ps2dev's current toolchain build)
dropped assembler support for the R5900 VU0 "macro mode" COP2 vector
instructions (vaddq, vmulax, vdiv, etc) that Insomniac's original compiler
emitted freely. spimdisasm still decodes them fine and prints the correct
mnemonic + operands, so the *bytes* in asm/ are correct -- our assembler
just refuses to re-encode the mnemonic text back into those bytes.

This is a bounded, mechanical problem: every disassembled instruction line
already carries its raw 32-bit encoding in the leading comment
(`/* romOffset vram RAWBYTES */`, little-endian). For any line whose
mnemonic is one of the unsupported VU0 macro-mode ops, replace the
mnemonic+operand text with a `.word 0x<value>` using that same encoding
(spimdisasm already does exactly this itself for a few opcodes it can't
decode at all, e.g. vsqrt -- see any existing `.word ... # vsqrt` line).
This keeps the byte-exact instruction stream so the object still matches
the original binary; it just can't be edited as text until either
binutils regains support for these opcodes or someone hand-codes them
back to mnemonics.

Idempotent: skips lines already converted to `.word`.
"""
import re
import sys
from pathlib import Path

UNSUPPORTED = re.compile(
    r"\b("
    r"v(add|sub|mul|madd|msub)a[xyzw]?|"  # vadda/vsuba/vmula/vmadda/vmsuba + optional xyzw suffix
    r"vaddq|vsubq|vmulq|vmaddq|vsubaw|vmulaq|"
    r"vdiv|vsqrt|vrsqrt|vopmula|vopmsub|vwaitq|vmtir|vmfir|vilwr|viswr|"
    r"vrxor|vrget|vrnext|vcallms|vftoi|vitof|vclip|vmr32|vlqi|vsqi|vlqd|vsqd"
    r")"
    r"(\.[a-z]+)?\b"
)

LINE_RE = re.compile(
    r"^(?P<indent>\s*)/\*\s*(?P<rom>[0-9A-Fa-f]+)\s+(?P<vram>[0-9A-Fa-f]+)\s+"
    r"(?P<raw>[0-9A-Fa-f]{8})\s*\*/(?P<gap>\s*)(?P<mnem>\S.*)$"
)


def convert_line(line: str) -> str | None:
    if ".word" in line:
        return None
    m = LINE_RE.match(line.rstrip("\n"))
    if not m:
        return None
    if not UNSUPPORTED.search(m.group("mnem")):
        return None
    raw = m.group("raw")
    # raw is little-endian bytes as they appear in the file; the actual
    # instruction word is the byte-reversed form (matches spimdisasm's own
    # `.word 0x...` fallback lines elsewhere in the same files).
    word = "".join(reversed([raw[i : i + 2] for i in range(0, 8, 2)]))
    mnem_text = m.group("mnem").strip()
    return (
        f"{m.group('indent')}/* {m.group('rom')} {m.group('vram')} {raw} */"
        f"{m.group('gap')}.word      0x{word.upper()}"
        f"    # {mnem_text}\n"
    )


def main() -> None:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "asm")
    changed_files = 0
    changed_lines = 0
    for path in sorted(root.rglob("*.s")):
        text = path.read_text().splitlines(keepends=True)
        out = []
        file_changed = False
        for line in text:
            new = convert_line(line)
            if new is not None:
                out.append(new)
                file_changed = True
                changed_lines += 1
            else:
                out.append(line)
        if file_changed:
            path.write_text("".join(out))
            changed_files += 1
    print(f"Rewrote {changed_lines} instruction(s) across {changed_files} file(s)")


if __name__ == "__main__":
    main()

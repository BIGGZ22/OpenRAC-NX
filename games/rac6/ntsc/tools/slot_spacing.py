#!/usr/bin/env python3
"""Survey the spacing of the stack slots of the level modules' prologues.

The R5900 has 64-bit general purpose registers, and a compiler decides how
far apart it puts the slots of the frame it reserves.  The retail does it
two ways: most functions reserve their slots 8 bytes apart, and a family of
5 569 reserves them 16 bytes apart.  Both save with `sd`, which stores
eight bytes: it is the *spacing* of the slots that differs, and the
spacing is what decides the frame.

Reading the spacing takes care: the stores of the prologue count, the loads
of the epilogue do not, and the window stops at the first call and at the
second frame adjustment, so neither the epilogue nor the next function
leaks in.

The survey prints the counts per module; it never publishes the bytes it
reads.

Usage:
    python3 tools/slot_spacing.py --modules <private dir>
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys_path = str(ROOT / "tools")

import rabbitizer  # noqa: E402

from code_intervals import code_end_from_relocations  # noqa: E402

SP = 29
# The frame saves the callee-saved registers, the frame pointer, the
# return address, and the argument registers the function modifies:
# the retail stores the incoming arguments in the frame too.
SAVED = tuple(range(4, 28)) + (30, 31)
CALLS = ("jal", "jalr", "jalx")


def frame_stores(words: list[tuple[int, int, str]], index: int) -> list[int]:
    """The offsets of the stores of the frame of the prologue at `index`."""
    offsets: list[int] = []
    frames = 0
    for step in range(1, 16):
        if index + step >= len(words):
            break
        _site, word, name = words[index + step]
        if name in CALLS:
            break
        if name == "addiu":
            rs = (word >> 21) & 31
            rt = (word >> 16) & 31
            immediate = word & 0xFFFF
            if immediate & 0x8000:
                immediate -= 0x10000
            if rs == SP and rt == SP and immediate < 0:
                frames += 1
                if frames > 1:
                    break
                continue
        if name in ("sd", "sw"):
            rt = (word >> 16) & 31
            rs = (word >> 21) & 31
            if rs != SP or rt not in SAVED:
                continue
            offset = word & 0xFFFF
            if offset & 0x8000:
                offset -= 0x10000
            offsets.append(offset)
    return offsets


def slot_step(offsets: list[int]) -> int:
    """The smallest distance between two slots of the frame."""
    if len(offsets) < 2:
        return 0
    offsets = sorted(set(offsets))
    steps = [b - a for a, b in zip(offsets, offsets[1:]) if b - a > 0]
    return min(steps) if steps else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--modules", type=Path, required=True,
                        help="the private directory of the retail modules")
    args = parser.parse_args()

    narrow = wide = 0
    for path in sorted(args.modules.glob("*.REL")):
        data = path.read_bytes()
        end = code_end_from_relocations(data)
        words = []
        for site in range(0, end, 4):
            word = int.from_bytes(data[site:site + 4], "little")
            name = rabbitizer.Instruction(
                word, site, rabbitizer.InstrCategory.R5900).getOpcodeName()
            words.append((site, word, name))
        module_wide = 0
        for index, (site, word, name) in enumerate(words):
            if name != "addiu":
                continue
            rs = (word >> 21) & 31
            rt = (word >> 16) & 31
            immediate = word & 0xFFFF
            if immediate & 0x8000:
                immediate -= 0x10000
            if rs != SP or rt != SP or immediate >= 0:
                continue
            step = slot_step(frame_stores(words, index))
            if step == 0:
                continue
            if step >= 16:
                module_wide += 1
            else:
                narrow += 1
        wide += module_wide
        print(f"  {path.name:16s} {module_wide:5d} wide prologue(s)")
    print(f"the wide family: {wide} prologue(s) whose stack slots are 16 bytes")
    print(f"apart, and {narrow} whose slots are 8 bytes apart.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

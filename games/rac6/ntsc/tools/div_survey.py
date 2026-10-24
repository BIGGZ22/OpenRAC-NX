#!/usr/bin/env python3
"""Survey the divisions of the retail code, and the padding that surrounds them.

A wave lot measured a body that is one instruction short of the retail: the
retail puts a ``nop`` between the setup of the divisor and the ``div.s``.
Before that becomes a compiler route, it has to be *measured*: how many
divisions does the retail code hold, how many of them carry such a padding,
and is the padding always the same.

The survey prints counts only; it never publishes the bytes it reads.

Usage:
    python3 tools/div_survey.py --modules <private dir> [--json]
"""

from __future__ import annotations

import argparse
import json
import struct
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys_path = str(ROOT / "tools")

import rabbitizer  # noqa: E402

from code_intervals import code_end_from_relocations  # noqa: E402
from rel import HEADER, RELOCATION, iter_packed_relocations, unpack_relocation  # noqa: E402

CODE_START = 0x80
WORD = struct.Struct("<I")
NOP = 0x00000000
DIVISIONS = {"div.s", "div", "divu", "sqrt.s", "rcp.s", "sqr.s", "rsqrt.s"}


def is_branch(word: int) -> bool:
    """Tell whether a word is a branch or a jump (its delay slot follows)."""
    opcode = word >> 26
    if opcode in (0x02, 0x03):
        return True
    if opcode in (0x04, 0x05, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17):
        return True
    if opcode == 0x01:
        return (word >> 16) & 0x1F in (0, 1, 2, 3)
    if opcode in (0x10, 0x11):
        return (word >> 21) & 0x1F == 8
    return False


def survey(data: bytes, end: int) -> dict:
    """Count the divisions of one code interval and their padding.

    The survey also records *which* instruction the padding follows: the
    hazard source.  If the padding were a function of that instruction, the
    two distributions would not overlap.
    """
    total = 0
    padded = 0
    delay = 0
    in_delay = 0
    padding = Counter()
    delay_padding = Counter()
    kinds = Counter()
    alignment = Counter()
    hazard_padded: Counter[str] = Counter()
    hazard_bare: Counter[str] = Counter()
    for site in range(CODE_START, end, 4):
        word = WORD.unpack_from(data, site)[0]
        name = rabbitizer.Instruction(word, site,
                                      rabbitizer.InstrCategory.R5900).getOpcodeName()
        if name not in DIVISIONS:
            continue
        total += 1
        kinds[name] += 1
        alignment[site % 16] += 1
        # Walk back over the nops: the instruction the padding follows is
        # the hazard source.  A nop that fills the delay slot of a branch is
        # not a hazard pad, and the two are counted apart.
        back = site
        nops = 0
        while back - 4 >= CODE_START:
            if WORD.unpack_from(data, back - 4)[0] == NOP:
                nops += 1
                back -= 4
                continue
            break
        source_name = "none"
        source_word = 0
        if back - 4 >= CODE_START:
            source_word = WORD.unpack_from(data, back - 4)[0]
            source_name = rabbitizer.Instruction(
                source_word, back - 4, rabbitizer.InstrCategory.R5900).getOpcodeName()
        if nops and is_branch(source_word):
            delay += 1
            delay_padding[nops] += 1
        elif nops:
            padded += 1
            padding[nops] += 1
            (hazard_padded)[source_name] += 1
        else:
            hazard_bare[source_name] += 1
        if site >= CODE_START + 4 and is_branch(
                WORD.unpack_from(data, site - 4)[0]):
            in_delay += 1
    return {"divisions": total, "padded": padded, "delay_slot_nop": delay,
            "in_delay_slot": in_delay,
            "padding": dict(sorted(padding.items())),
            "delay_slot_padding": dict(sorted(delay_padding.items())),
            "kinds": dict(sorted(kinds.items())),
            "address_modulo_16": dict(sorted(alignment.items())),
            "eight_byte_aligned": alignment[0] + alignment[8],
            "hazard_source_of_a_padded_division": dict(hazard_padded.most_common(12)),
            "instruction_before_a_bare_division": dict(hazard_bare.most_common(12))}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--modules", type=Path, required=True)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    report = {"modules": {}, "totals": {"divisions": 0, "padded": 0,
                                        "delay_slot_nop": 0, "in_delay_slot": 0}}
    for path in sorted(args.modules.glob("*.REL")):
        data = path.read_bytes()
        end = code_end_from_relocations(data)
        result = survey(data, end)
        report["modules"][path.name] = result
        for key in report["totals"]:
            report["totals"][key] += result[key]
        print(f"  {path.name:16s} {result['divisions']:6d} division(s), "
              f"{result['padded']:6d} hazard pad, "
              f"{result['delay_slot_nop']:6d} delay-slot nop, "
              f"{result['in_delay_slot']:4d} in a delay slot")
    print(f"  {'TOTAL':16s} {report['totals']['divisions']:6d} division(s), "
          f"{report['totals']['padded']:6d} hazard pad, "
          f"{report['totals']['delay_slot_nop']:6d} delay-slot nop, "
          f"{report['totals']['in_delay_slot']:4d} in a delay slot")
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        first = next(iter(report["modules"].values()))
        print("  the hazard source of a padded division, and the instruction "
              "before a bare one:")
        for name in first["hazard_source_of_a_padded_division"]:
            padded = first["hazard_source_of_a_padded_division"][name]
            bare = first["instruction_before_a_bare_division"].get(name, 0)
            print(f"    {name:16s} padded {padded:6d}   bare {bare:6d}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Survey the stack-slot width of the boot's bodies.

The R5900 has 64-bit general purpose registers, and a compiler decides
whether a stack slot is 8 or 16 bytes wide.  The boot is not homogeneous: the
survey measures, for every bounded body of the boot, whether it saves its
registers with ``sd``/``ld`` (8-byte slots) or with ``sq``/``lq`` (16-byte
slots), and where each family lives.

The survey prints counts and address ranges; it never publishes the bytes it
reads.

Usage:
    python3 tools/slot_survey.py --boot <executable> --bounds <private dir>
"""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys_path = str(ROOT / "tools")

import rabbitizer  # noqa: E402

from elf import inspect_elf  # noqa: E402

QUAD = ("sq", "lq")
DOUBLE = ("sd", "ld")


def text_of(path: Path) -> tuple[int, bytes]:
    report = inspect_elf(path)
    section = next(item for item in report["sections"] if item["name"] == ".text")
    data = path.read_bytes()
    offset = int(section["file_offset"])
    return int(str(section["virtual_address"]), 16), \
        data[offset:offset + int(section["size"])]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--boot", type=Path, required=True)
    parser.add_argument("--bounds", type=Path, required=True,
                        help="directory holding boot.tsv")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    base, text = text_of(args.boot)
    path = args.bounds / "boot.tsv"
    lines = [line for line in path.read_text(encoding="utf-8").splitlines()
             if line and not line.startswith("#")]
    header = lines[0].split("\t")
    kinds: Counter[str] = Counter()
    addresses: dict[str, list[int]] = {}
    for line in lines[1:]:
        values = dict(zip(header, line.split("\t")))
        if values["status"] != "bounded_candidate":
            continue
        address = int(values["start"], 16)
        size = int(values["body_bytes"])
        quad = double = 0
        for step in range(0, size, 4):
            word = int.from_bytes(text[address - base + step:
                                       address - base + step + 4], "little")
            name = rabbitizer.Instruction(
                word, address + step, rabbitizer.InstrCategory.R5900).getOpcodeName()
            if name in QUAD:
                quad += 1
            elif name in DOUBLE:
                double += 1
        kind = ("quad" if quad and not double else
                "double" if double and not quad else
                "both" if quad and double else "none")
        kinds[kind] += 1
        addresses.setdefault(kind, []).append(address)

    report: dict[str, object] = {"counts": dict(sorted(kinds.items())), "runs": {}}
    for kind, rows in addresses.items():
        runs: list[list[int]] = []
        for address in sorted(rows):
            if runs and address - runs[-1][1] <= 0x100:
                runs[-1][1] = address
            else:
                runs.append([address, address])
        runs.sort(key=lambda item: -(item[1] - item[0]))
        report["runs"][kind] = [[f"0x{first:08X}", f"0x{last:08X}"]
                                for first, last in runs[:16]]

    if args.json:
        print(json.dumps(report, indent=2))
        return 0
    print("slot width of the bounded bodies:", dict(kinds.most_common()))
    for kind, rows in addresses.items():
        runs = report["runs"][kind]
        print(f"  {kind}: {len(rows)} bodies, {len(runs)} largest runs")
        for first, last in runs[:4]:
            print(f"      {first}..{last}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

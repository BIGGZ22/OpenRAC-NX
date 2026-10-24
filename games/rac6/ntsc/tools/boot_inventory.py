#!/usr/bin/env python3
"""Audit the function-entry candidates of the boot executable.

The boot of the disc is a linked MIPS ELF without a symbol table.  Its
candidate entries are therefore measured, not named: a direct call is
evidence for a start, and a real ``jr $ra`` (plus its delay slot) ends a
body before the next candidate.

The tool reads the structure of the executable, counts the candidates, and
bounds their bodies.  It does not split functions, does not apply
relocations and does not claim any byte match.

Usage:
    python3 tools/boot_inventory.py <executable> --out <private dir> [--write]
"""

from __future__ import annotations

import argparse
import bisect
import hashlib
import json
import struct
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SUMMARY = ROOT / "config/boot/summary.json"
WORD = struct.Struct("<I")
JR_RA = 0x03E00008


def entry_point(path: Path) -> int:
    """Return the entry point of an ELF."""
    from elf import inspect_elf
    return int(str(inspect_elf(path)["entry"]), 16)


def text_section(path: Path) -> tuple[int, bytes]:
    """Return the address and the bytes of the .text section of an ELF."""
    from elf import inspect_elf
    report = inspect_elf(path)
    section = next((item for item in report["sections"]
                    if item["name"] == ".text"), None)
    if section is None:
        raise ValueError("the executable has no .text section")
    data = Path(path).read_bytes()
    offset = int(section["file_offset"])
    size = int(section["size"])
    return int(str(section["virtual_address"]), 16), data[offset:offset + size]


def candidates(text: bytes, base: int) -> tuple[set[int], set[int], set[int]]:
    """Return (call targets, returns, branch targets) of a code section."""
    calls: set[int] = set()
    returns: set[int] = set()
    branches: set[int] = set()
    for index in range(len(text) // 4):
        address = base + 4 * index
        word = WORD.unpack_from(text, 4 * index)[0]
        opcode = word >> 26
        if opcode == 0x03:  # jal: the R5900 holds an absolute target
            target = (word & 0x03FFFFFF) << 2
            if base <= target < base + len(text):
                calls.add(target)
        elif opcode == 0x02:  # j
            target = (word & 0x03FFFFFF) << 2
            if base <= target < base + len(text):
                branches.add(target)
        elif word == JR_RA:
            returns.add(address)
    return calls, returns, branches


def rows(base: int, text: bytes, calls: set[int], returns: set[int],
         pointers: set[int] | None = None) -> list[dict]:
    """Bound the body of every candidate entry."""
    start = base
    end = base + len(text)
    pointers = pointers or set()
    starts = sorted({start, *calls, *pointers})
    ordered = sorted(returns)
    result: list[dict] = []
    for entry, limit in zip(starts, starts[1:] + [end]):
        index = bisect.bisect_right(ordered, limit - 8) - 1
        body_end = ordered[index] + 8 if index >= 0 and ordered[index] >= entry else None
        if body_end is None:
            status = "no_return"
        elif any(text[body_end - base:limit - base]):
            status = "nonzero_tail"
        else:
            status = "bounded_candidate"
        body = text[entry - base:body_end - base] if body_end else b""
        result.append({
            "start": entry, "limit": limit, "body_end": body_end,
            "evidence": "+".join(label for label, pool in
                                  (("call", calls), ("pointer", pointers))
                                  if entry in pool) or "interval_start",
            "status": status, "body_bytes": len(body),
            "raw_sha256": hashlib.sha256(body).hexdigest() if body else "",
        })
    return result


def pointers(sections: list[dict], data: bytes, base: int, text_end: int) -> set[int]:
    """Return the code addresses the data sections point to.

    A function that is only reached through a pointer is a candidate too:
    the pointer itself is the evidence.
    """
    found: set[int] = set()
    for section in sections:
        name = section["name"]
        if name in (".text", ".bss", ".sbss") or name.startswith("DVP"):
            continue
        offset = int(section["file_offset"])
        size = int(section["size"])
        if size <= 0 or offset + size > len(data):
            continue
        for at in range(offset, offset + size - 3, 4):
            value = int.from_bytes(data[at:at + 4], "little")
            if base <= value < text_end and value % 4 == 0:
                found.add(value)
    return found


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--out", type=Path, required=True,
                        help="private directory for the candidate rows")
    parser.add_argument("--write", action="store_true",
                        help="write the tracked summary")
    args = parser.parse_args()

    from elf import inspect_elf
    report = inspect_elf(args.executable)
    base, text = text_section(args.executable)
    calls, returns, _branches = candidates(text, base)
    # The entry point is a candidate of its own.
    entry = entry_point(args.executable)
    if base <= entry < base + len(text):
        calls.add(entry)
    # A function that is only reached through a pointer is a candidate too.
    data = args.executable.read_bytes()
    data_pointers = pointers(report["sections"], data, base, base + len(text))
    table = rows(base, text, calls | data_pointers, returns, data_pointers)
    bounded = [row for row in table if row["status"] == "bounded_candidate"]

    args.out.mkdir(parents=True, exist_ok=True)
    lines = ["# Candidate boundaries of the boot executable; no row is an "
             "accepted retail function size.",
             "start\tlimit\tbody_end\tevidence\tstatus\tbody_bytes\traw_sha256"]
    for row in table:
        lines.append("\t".join((
            f"0x{row['start']:08X}", f"0x{row['limit']:08X}",
            f"0x{row['body_end']:08X}" if row["body_end"] else "",
            row["evidence"], row["status"], str(row["body_bytes"]),
            row["raw_sha256"])))
    (args.out / "boot.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8",
                                       newline="\n")

    summary = {
        "status": "candidate_inventory_only",
        "executable": args.executable.name,
        "sha256": hashlib.sha256(args.executable.read_bytes()).hexdigest(),
        "text_address": f"0x{base:08X}",
        "text_bytes": len(text),
        "entry_intervals": len(table),
        "call_targets": len(calls),
        "data_pointers": len(data_pointers),
        "returns": len(returns),
        "boundary_status_counts": dict(sorted(Counter(
            row["status"] for row in table).items())),
        "bounded_candidate_body_bytes": sum(row["body_bytes"] for row in bounded),
        "named_symbols": 0,
        "name_note": ("The boot executable carries no symbol table: the "
                      "candidates are measured, not named."),
        "overall_percent": None,
    }
    content = json.dumps(summary, indent=2) + "\n"
    if args.write:
        SUMMARY.parent.mkdir(parents=True, exist_ok=True)
        SUMMARY.write_text(content, encoding="utf-8", newline="\n")
    elif not SUMMARY.is_file() or SUMMARY.read_text(encoding="utf-8") != content:
        raise ValueError("the boot inventory summary is missing or stale; "
                         "run with --write")
    print(f"{len(table)} candidate intervals, {len(bounded)} bounded, "
          f"{summary['bounded_candidate_body_bytes']} bounded body bytes; "
          f"no game-wide percentage")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

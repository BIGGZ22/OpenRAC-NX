#!/usr/bin/env python3
"""Audit the SNR2 function-entry candidates of the disc, and bound their bodies.

A **candidate** is a module offset that some evidence makes worth a look: a
direct call, a code pointer, an exported symbol, or the start of the code
interval.  A candidate is **bounded** when a real ``jr $ra`` (plus its delay
slot) ends its body before the next candidate starts, and nothing but zeros
follows that body up to the next candidate.

Nothing here proves that a candidate is a function, and nothing here claims a
byte match: the rows are a work queue, and the tracked summary never publishes
a percentage.

The per-module rows stay in the private workspace; only the summary is
tracked.

Adapted from ``OpenRAC/rac5-sm-decomp`` (GPL-3.0); see
``THIRD_PARTY_NOTICES.md``.

Usage:
    python3 tools/function_inventory.py <image> --out <private dir> [--write]
"""

from __future__ import annotations

import argparse
import bisect
import csv
import hashlib
import json
import struct
from collections import Counter, defaultdict
from pathlib import Path

from rel import HEADER, RELOCATION, SYMBOL, iter_packed_relocations, unpack_relocation

ROOT = Path(__file__).resolve().parent.parent
SUMMARY = ROOT / "config/levels/function_inventory_summary.json"
TARGET = ROOT / "config/target.json"
WORD = struct.Struct("<I")
CODE_START = 0x80
JR_RA = 0x03E00008

# Branch forms of the R5900 that can reach a local address.
#   0x04/0x05/0x06/0x07   beq / bne / blez / bgtz
#   0x14/0x15/0x16/0x17   beql / bnel / blezl / bgtzl
#   0x01                 REGIMM: bltz, bgez, bltzl, bgezl
#   0x10/0x11            COP0 / COP1 branch (register field 8)
# The 26-bit jumps (0x02 j, 0x03 jal) are handled apart: their target is an
# absolute address, not an offset.
OFFSET_BRANCH_OPCODES = {0x04, 0x05, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17}
COP_BRANCH_OPCODES = {0x10, 0x11}
COP_BRANCH_REGISTER = 8
REGIMM_BRANCH_REGS = {0, 1, 2, 3}


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def signed16(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def branch_target(word: int, site: int) -> int | None:
    """Return the branch target of one R5900 word, or None when it has none."""
    opcode = word >> 26
    if opcode in (0x02, 0x03):
        return (word & 0x03FFFFFF) << 2
    if opcode == 0x01:  # REGIMM: bltz, bgez and their likely variants
        if (word >> 16) & 0x1F not in REGIMM_BRANCH_REGS:
            return None
    elif opcode in OFFSET_BRANCH_OPCODES:
        pass
    elif opcode in COP_BRANCH_OPCODES:
        if (word >> 21) & 0x1F != COP_BRANCH_REGISTER:
            return None
    else:
        return None
    return site + 4 + (signed16(word) << 2)


def is_branch_word(word: int) -> bool:
    """Tell whether a word can be a branch, before decoding it."""
    opcode = word >> 26
    if opcode in (0x01, 0x02, 0x03) or opcode in OFFSET_BRANCH_OPCODES:
        return True
    if opcode in COP_BRANCH_OPCODES:
        return (word >> 21) & 0x1F == COP_BRANCH_REGISTER
    return False


def module_tables(data: bytes) -> tuple[dict[int, tuple], set[int], set[int]]:
    """Return (relocations by site, call targets, pointer targets)."""
    header = HEADER.unpack_from(data)
    names: list[str] = []
    addresses: list[int] = []
    for index in range(header[4]):
        name_at, address, _aux, _kind, _flags = SYMBOL.unpack_from(
            data, header[3] + index * SYMBOL.size)
        end = data.index(b"\0", name_at)
        names.append(data[name_at:end].decode("ascii", "strict"))
        addresses.append(address)

    indexed: dict[int, tuple] = {}
    calls: set[int] = set()
    pointers: set[int] = set()
    for index in range(header[2]):
        site, kind, symbol, addend = unpack_relocation(
            data, header[1] + index * RELOCATION.size)
        indexed[site] = ("named", kind, names[symbol], addend)
        if kind == 4 and addresses[symbol]:
            calls.add(addresses[symbol] + addend)
    for row in iter_packed_relocations(data, header[13], header[1]):
        indexed[row.site] = ("local", row.kind, "", 0)
        if row.kind == 2:
            word = WORD.unpack_from(data, row.site)[0]
            calls.add((word & 0x03FFFFFF) << 2)
        elif row.kind == 3:
            pointers.add(WORD.unpack_from(data, row.site)[0])
    return indexed, calls, pointers


def export_names(data: bytes) -> dict[int, str]:
    """Return the defined symbols of a module, by module-relative address."""
    header = HEADER.unpack_from(data)
    named: dict[int, str] = {}
    for index in range(header[4]):
        name_at, address, _aux, _kind, _flags = SYMBOL.unpack_from(
            data, header[3] + index * SYMBOL.size)
        if not address:
            continue
        end = data.index(b"\0", name_at)
        name = data[name_at:end].decode("ascii", "strict")
        named.setdefault(address, name)
    return named


def control_flow(data: bytes, code_end: int) -> tuple[set[int], set[int], list[tuple[int, int]]]:
    """Return (returns, branch edges) of the code interval."""
    returns: set[int] = set()
    edges: list[tuple[int, int]] = []
    for site in range(CODE_START, code_end, 4):
        word = WORD.unpack_from(data, site)[0]
        if word == JR_RA:
            returns.add(site)
        if is_branch_word(word):
            target = branch_target(word, site)
            if target is not None and CODE_START <= target < code_end:
                edges.append((site, target))
    return returns, edges


def masked_body(body: bytes, start: int, relocs: dict[int, tuple],
                sites: list[int]) -> tuple[str, int]:
    """Hash a body with its relocation fields removed; a lead, not an identity."""
    masked = bytearray(body)
    descriptors = []
    left = bisect.bisect_left(sites, start)
    right = bisect.bisect_left(sites, start + len(body))
    for site in sites[left:right]:
        source, kind, symbol, addend = relocs[site]
        offset = site - start
        word = WORD.unpack_from(masked, offset)[0]
        if (source == "local" and kind == 2) or (source == "named" and kind == 4):
            mask = 0xFC000000
        elif (source == "local" and kind == 3) or (source == "named" and kind == 2):
            mask = 0
        else:
            mask = 0xFFFF0000
        WORD.pack_into(masked, offset, word & mask)
        descriptors.append((offset, source, kind, symbol, addend))
    fingerprint = bytes(masked) + b"\0" + json.dumps(descriptors, separators=(",", ":")).encode()
    return digest(fingerprint), len(descriptors)


def module_rows(module: str, data: bytes, code_end: int) -> list[dict]:
    """Return the candidate intervals of one module."""
    relocs, calls, pointers = module_tables(data)
    exports = export_names(data)
    pointers = {target for target in pointers
                if CODE_START <= target < code_end and target % 4 == 0}
    returns, edges = control_flow(data, code_end)
    starts = sorted({CODE_START, *calls, *pointers, *exports})
    sites = sorted(relocs)
    ordered_returns = sorted(returns)
    rows: list[dict] = []
    for start, limit in zip(starts, starts[1:] + [code_end]):
        evidence = "+".join(label for label, pool in
                            (("call", calls), ("pointer", pointers), ("export", exports))
                            if start in pool) or "interval_start"
        index = bisect.bisect_right(ordered_returns, limit - 8) - 1
        body_end = ordered_returns[index] + 8 \
            if index >= 0 and ordered_returns[index] >= start else None
        if body_end is None:
            status = "no_return"
        elif any(data[body_end:limit]):
            status = "nonzero_tail"
        elif start not in calls and start not in exports:
            status = "pointer_only"
        else:
            status = "bounded_candidate"
        body = data[start:body_end] if body_end is not None else b""
        masked, reloc_count = masked_body(body, start, relocs, sites) if body else ("", 0)
        rows.append({
            "module": module, "start": start, "limit": limit, "body_end": body_end,
            "evidence": evidence, "status": status, "name": exports.get(start, ""),
            "body_bytes": len(body),
            "tail_bytes": limit - body_end
            if body_end is not None and not any(data[body_end:limit]) else 0,
            "raw_sha256": digest(body) if body else "",
            "masked_lead_sha256": masked, "body_relocations": reloc_count,
        })
    for source, target in edges:
        index = bisect.bisect_right(starts, target) - 1
        if index < 0:
            continue
        row = rows[index]
        if (row["status"] == "bounded_candidate" and row["start"] < target
                and row["body_end"] is not None and target < row["body_end"]
                and not row["start"] <= source < row["limit"]):
            row["status"] = "external_interior_target"
    return rows


def render_rows(rows: list[dict]) -> str:
    lines = ["# Candidate boundaries only; no row is an accepted retail function size.",
             "start\tlimit\tbody_end\tevidence\tstatus\tname\tbody_bytes\ttail_bytes\t"
             "raw_sha256\tmasked_lead_sha256\tbody_relocations"]
    for row in rows:
        lines.append("\t".join((
            f"0x{row['start']:08X}", f"0x{row['limit']:08X}",
            f"0x{row['body_end']:08X}" if row["body_end"] is not None else "",
            row["evidence"], row["status"], row["name"], str(row["body_bytes"]),
            str(row["tail_bytes"]), row["raw_sha256"], row["masked_lead_sha256"],
            str(row["body_relocations"]))))
    return "\n".join(lines) + "\n"


def aggregate(all_rows: list[dict], module_sizes: dict[str, int]) -> dict:
    statuses = Counter(row["status"] for row in all_rows)
    trusted = [row for row in all_rows if row["status"] == "bounded_candidate"]
    raw: dict[tuple[int, str], list[dict]] = defaultdict(list)
    masked: dict[tuple[int, str], list[dict]] = defaultdict(list)
    for row in trusted:
        raw[(row["body_bytes"], row["raw_sha256"])].append(row)
        masked[(row["body_bytes"], row["masked_lead_sha256"])].append(row)
    return {
        "status": "candidate_inventory_only",
        "module_count": len(module_sizes),
        "candidate_code_bytes_by_placement": sum(module_sizes.values()),
        "entry_intervals": len(all_rows),
        "boundary_status_counts": dict(sorted(statuses.items())),
        "bounded_candidate_body_bytes_by_placement": sum(row["body_bytes"] for row in trusted),
        "raw_exact_distinct_candidate_bytes": sum(size for size, _hash in raw),
        "raw_exact_multi_placement_groups": sum(len(group) > 1 for group in raw.values()),
        "relocation_masked_candidate_bytes": sum(size for size, _hash in masked),
        "masked_multi_placement_leads": sum(len(group) > 1 for group in masked.values()),
        "mask_warning": ("Masked relocation targets may refer to different code or "
                         "data; these are leads, not proved shared functions or a "
                         "progress denominator."),
        "overall_percent": None,
        "bytematch_ready": False,
        "modules": [
            {"module": module, "candidate_code_bytes": module_sizes[module],
             "candidate_intervals": sum(row["module"] == module for row in all_rows),
             "bounded_candidates": sum(row["module"] == module and
                                       row["status"] == "bounded_candidate"
                                       for row in all_rows)}
            for module in module_sizes
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("image", type=Path)
    parser.add_argument("--out", type=Path, required=True,
                        help="private directory for the per-module rows")
    parser.add_argument("--write", action="store_true",
                        help="write the tracked summary instead of checking it")
    args = parser.parse_args()

    from disc import IsoDisc
    from rel import module_paths

    all_rows: list[dict] = []
    module_sizes: dict[str, int] = {}
    args.out.mkdir(parents=True, exist_ok=True)
    with IsoDisc(args.image) as disc:
        for path in module_paths(disc):
            name = Path(path).name
            data = disc.file_bytes(path)
            code_end = code_end_of(data)
            rows = module_rows(name, data, code_end)
            (args.out / f"{name.replace('.REL', '')}.tsv").write_text(
                render_rows(rows), encoding="utf-8", newline="\n")
            all_rows.extend(rows)
            module_sizes[name] = code_end - CODE_START
            bounded = sum(row["status"] == "bounded_candidate" for row in rows)
            print(f"{name:16s} {len(rows):6d} intervals  {bounded:6d} bounded",
                  flush=True)

    summary = aggregate(all_rows, module_sizes)
    content = json.dumps(summary, indent=2) + "\n"
    if args.write:
        SUMMARY.write_text(content, encoding="utf-8", newline="\n")
    elif not SUMMARY.is_file() or SUMMARY.read_text(encoding="utf-8") != content:
        raise ValueError("function inventory summary is missing or stale; run with --write")
    print(f"{len(all_rows)} candidate entry intervals across {len(module_sizes)} modules; "
          f"{summary['bounded_candidate_body_bytes_by_placement']} bounded body bytes "
          "by placement; no game-wide percentage")
    return 0


def code_end_of(data: bytes) -> int:
    """Derive the end of the candidate code interval from the relocation tables."""
    from code_intervals import code_end_from_relocations
    return code_end_from_relocations(data)


if __name__ == "__main__":
    raise SystemExit(main())

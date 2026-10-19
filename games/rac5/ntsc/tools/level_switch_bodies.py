#!/usr/bin/env python3
"""Review contiguous switch bodies split by pointer-only static labels.

Retail bytes and Ghidra exports remain private. The committed summary records
only sizes, hashes and conditional code-shape evidence.
"""

from __future__ import annotations

import argparse
import bisect
import hashlib
import json
import struct
from collections import Counter, defaultdict
from pathlib import Path

from disc import IsoDisc, verify_disc
from level_function_inventory import control_flow, masked_body, relocations
from level_ghidra_crosscheck import rows

ROOT = Path(__file__).resolve().parent.parent
WORK = ROOT / "build/level-functions"
REVIEW = WORK / "switch_body_review.tsv"
OUTPUT = ROOT / "config/levels/switch_body_summary.json"
TARGET = ROOT / "config/target.json"
MODULES = ROOT / "config/levels/modules.tsv"
CODE = ROOT / "config/levels/code_intervals.tsv"
INVENTORY = ROOT / "config/levels/function_inventory_summary.json"
SPLIT = ROOT / "config/levels/split_dedupe_summary.json"
GHIDRA = ROOT / "config/levels/ghidra_crosscheck_summary.json"
FULL_FLOW = ROOT / "config/levels/full_module_flow_summary.json"
JR_RA = 0x03E00008


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def reviewed_switch(row: dict[str, str], full: dict, code: dict,
                    internal: list[dict[str, str]],
                    full_starts: list[int], code_starts: list[int],
                    split_parents: set[int], incoming: dict[int, list[int]],
                    incoming_targets: list[int], data: bytes,
                    code_end: int) -> tuple[int, int] | None:
    """Require a call entry, one contiguous full body and no external entry."""
    if row["status"] != "no_return" or row["evidence"] != "call":
        return None
    start = int(row["start"], 16)
    function = full.get(start)
    if function is None:
        return None
    left, end = int(function["min"], 16), int(function["max"], 16) + 1
    size = end - left
    if (left != start or end > code_end or size < 16
            or int(function["body_bytes"]) != size
            or function["ranges"] != f"0x{start:X}-0x{end-1:X}"
            or struct.unpack_from("<I", data, end - 8)[0] != JR_RA
            or not internal or any(item["evidence"] != "pointer" for item in internal)
            or any(int(item["start"], 16) in split_parents
                   for item in [row, *internal])):
        return None
    if (bisect.bisect_left(full_starts, end) -
            bisect.bisect_right(full_starts, start) or
            bisect.bisect_left(code_starts, end) -
            bisect.bisect_right(code_starts, start)):
        return None
    code_function = code.get(start)
    if code_function is not None and (
            int(code_function["min"], 16) != start or
            int(code_function["max"], 16) + 1 != end or
            int(code_function["body_bytes"]) > size):
        return None
    for target in incoming_targets[
            bisect.bisect_right(incoming_targets, start):
            bisect.bisect_left(incoming_targets, end)]:
        if any(source < start or source >= end for source in incoming[target]):
            return None
    return start, end


def main_write(image: Path, full_dir: Path) -> dict[str, object]:
    target = json.loads(TARGET.read_text(encoding="utf-8"))
    split = json.loads(SPLIT.read_text(encoding="utf-8"))
    if digest(WORK / "split_boundary_review.tsv") != split["split_review_sha256"]:
        raise ValueError("private split review differs from pinned summary")
    modules = {Path(row["path"]).name: row for row in rows(MODULES)}
    code = {row["module"]: int(row["end_exclusive"], 16) for row in rows(CODE)}
    code_hashes = {row["module"]: row["ghidra_export_sha256"]
                   for row in json.loads(GHIDRA.read_text(encoding="utf-8"))["modules"]}
    full_hashes = {row["module"]: row["full_export_sha256"]
                   for row in json.loads(FULL_FLOW.read_text(encoding="utf-8"))["modules"]}
    split_parents = defaultdict(set)
    for row in rows(WORK / "split_boundary_review.tsv"):
        split_parents[row["module"]].add(int(row["parent_start"], 16))
    if len(modules) != 21 or len(code) != 21 or set(modules) != set(code):
        raise ValueError("expected 21 pinned SNR2 modules")
    review = ["# Conditional contiguous switch-body leads only; no C match.",
              "module\tstart\tend\tbody_bytes\tinternal_entries\tbody_relocations\t"
              "raw_sha256\tmasked_sha256"]
    grouped = defaultdict(list)
    placement_count = placement_bytes = 0
    with IsoDisc(image) as disc:
        verify_disc(disc, target)
        for module in sorted(code):
            data = disc.file_bytes("LVL/" + module)
            if hashlib.sha256(data).hexdigest() != modules[module]["sha256"]:
                raise ValueError(f"{module} differs from pinned image")
            static = rows(WORK / f"{module}.tsv")
            starts = [int(row["start"], 16) for row in static]
            code_path = WORK / f"{module}.code.bin.ghidra.tsv"
            full_path = full_dir / f"{module}.ghidra.tsv"
            if (digest(code_path) != code_hashes[module] or
                    digest(full_path) != full_hashes[module]):
                raise ValueError(f"{module} Ghidra exports differ from pinned evidence")
            code_functions = {int(row["entry"], 16): row for row in rows(code_path)}
            full_functions = {int(row["entry"], 16): row for row in rows(full_path)}
            full_starts, code_starts = sorted(full_functions), sorted(code_functions)
            indexed, _pointers = relocations(data, code[module])
            reloc_sites = sorted(indexed)
            _calls, _returns, edges = control_flow(data, code[module])
            incoming = defaultdict(list)
            for source, destination in edges:
                incoming[destination].append(source)
            incoming_targets = sorted(incoming)
            for row in static:
                start = int(row["start"], 16)
                function = full_functions.get(start)
                if function is None:
                    continue
                end = int(function["max"], 16) + 1
                if end > code[module]:
                    continue
                internal = static[bisect.bisect_right(starts, start):
                                  bisect.bisect_left(starts, end)]
                bound = reviewed_switch(row, full_functions, code_functions,
                                        internal, full_starts, code_starts,
                                        split_parents[module], incoming,
                                        incoming_targets, data, code[module])
                if bound is None:
                    continue
                left, right = bound
                raw = data[left:right]
                raw_hash = hashlib.sha256(raw).hexdigest()
                masked_hash, fixup_count = masked_body(raw, left, indexed, reloc_sites)
                grouped[len(raw), masked_hash].append((module, left))
                placement_count += 1
                placement_bytes += len(raw)
                review.append(f"{module}\t0x{left:08X}\t0x{right:08X}\t"
                              f"{len(raw)}\t{len(internal)}\t{fixup_count}\t"
                              f"{raw_hash}\t{masked_hash}")
    REVIEW.write_text("\n".join(review) + "\n", encoding="utf-8", newline="\n")
    shared = [(size, members) for (size, _fingerprint), members in grouped.items()
              if len(members) >= 2 and
              len({module for module, _start in members}) == len(members)]
    summary = {
        "status": "contiguous_switch_body_leads_only",
        "module_count": len(code),
        "reviewed_placements": placement_count,
        "reviewed_placement_bytes": placement_bytes,
        "masked_groups": len(grouped),
        "shared_masked_groups": len(shared),
        "optimistic_shared_shape_savings": sum(size * (len(members) - 1)
                                               for size, members in shared),
        "review_sha256": digest(REVIEW),
        "target_sha256": digest(TARGET),
        "module_catalogue_sha256": digest(MODULES),
        "code_catalogue_sha256": digest(CODE),
        "inventory_sha256": digest(INVENTORY),
        "split_sha256": digest(SPLIT),
        "ghidra_sha256": digest(GHIDRA),
        "full_flow_sha256": digest(FULL_FLOW),
        "overall_percent": None,
        "bytematch_ready": False,
        "warning": ("Full-module Ghidra and static call/pointer evidence support "
                    "contiguous switch-body review leads. Masked shapes and "
                    "function ownership still need data and callgraph checks."),
    }
    OUTPUT.write_text(json.dumps(summary, indent=2) + "\n",
                      encoding="utf-8", newline="\n")
    return summary


def check_summary(summary: dict[str, object]) -> None:
    if (summary["status"] != "contiguous_switch_body_leads_only"
            or summary["module_count"] != 21
            or summary["reviewed_placements"] < summary["shared_masked_groups"]
            or summary["reviewed_placement_bytes"] < 0
            or summary["masked_groups"] < summary["shared_masked_groups"]
            or summary["optimistic_shared_shape_savings"] < 0
            or len(summary["review_sha256"]) != 64
            or summary["target_sha256"] != digest(TARGET)
            or summary["module_catalogue_sha256"] != digest(MODULES)
            or summary["code_catalogue_sha256"] != digest(CODE)
            or summary["inventory_sha256"] != digest(INVENTORY)
            or summary["split_sha256"] != digest(SPLIT)
            or summary["ghidra_sha256"] != digest(GHIDRA)
            or summary["full_flow_sha256"] != digest(FULL_FLOW)
            or summary["overall_percent"] is not None
            or summary["bytematch_ready"] is not False):
        raise ValueError("switch-body summary is stale or overclaims")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", type=Path, metavar="PINNED_ISO")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--full-ghidra-dir", type=Path,
                        default=WORK / "full-ghidra")
    args = parser.parse_args()
    summary = main_write(args.write, args.full_ghidra_dir) if args.write else json.loads(
        OUTPUT.read_text(encoding="utf-8"))
    check_summary(summary)
    print(f"reviewed switch bodies: {summary['reviewed_placements']} placements; "
          "no qualified unique-code denominator")


if __name__ == "__main__":
    main()

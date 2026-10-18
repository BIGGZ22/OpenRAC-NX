#!/usr/bin/env python3
"""Classify SNR2 candidate starts against private Ghidra body-address ranges.

This identifies likely internal blocks, noncontiguous bodies and unowned
spans. It does not turn Ghidra auto-analysis into an accepted function list.
"""

from __future__ import annotations

import argparse
import bisect
import hashlib
import json
import struct
from collections import Counter
from pathlib import Path

from level_ghidra_crosscheck import rows

ROOT = Path(__file__).resolve().parent.parent
WORK = ROOT / "build/level-functions"
RANGES = WORK / "ranges"
SUMMARY = ROOT / "config/levels/flow_range_summary.json"
INVENTORY = ROOT / "config/levels/function_inventory_summary.json"
CROSSCHECK = ROOT / "config/levels/ghidra_crosscheck_summary.json"
CODE_CATALOGUE = ROOT / "config/levels/code_intervals.tsv"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_ranges(functions: list[dict[str, str]]) -> tuple[list[tuple[int, int, int]], int]:
    intervals = []
    claimed = 0
    for row in functions:
        entry = int(row["entry"], 16)
        if not row.get("ranges"):
            raise ValueError("Ghidra export has no body-address ranges")
        size = 0
        for item in row["ranges"].split(";"):
            left, right = (int(value, 16) for value in item.split("-"))
            if left > right or left % 4 or (right + 1) % 4:
                raise ValueError("invalid R5900 Ghidra body range")
            size += right - left + 1
            intervals.append((left, right, entry))
        if size != int(row["body_bytes"]):
            raise ValueError("Ghidra body ranges disagree with reported size")
        claimed += size
    intervals.sort()
    for left, right in zip(intervals, intervals[1:]):
        if left[1] >= right[0]:
            raise ValueError("Ghidra function body ranges overlap")
    return intervals, claimed


def body_owner(address: int, intervals: list[tuple[int, int, int]],
               boundaries: list[int]) -> int | None:
    idx = bisect.bisect_right(boundaries, address) - 1
    return intervals[idx][2] if idx >= 0 and address <= intervals[idx][1] else None


def direct_noncall_target(word: int, site: int) -> int | None:
    """Decode R5900 direct branches and J; exclude link/call forms."""
    opcode = word >> 26
    if opcode == 2:  # J
        return ((site + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
    branch = opcode in (4, 5, 6, 7, 20, 21, 22, 23)
    if opcode == 1:  # REGIMM: BLTZ/BGEZ and likely variants, excluding *AL
        branch = ((word >> 16) & 31) in (0, 1, 2, 3)
    elif opcode in (16, 17, 18):  # COP0/1/2 branch-on-condition
        branch = ((word >> 21) & 31) == 8 and ((word >> 16) & 31) in (0, 1, 2, 3)
    if branch:
        immediate = word & 0xFFFF
        if immediate & 0x8000:
            immediate -= 0x10000
        return site + 4 + (immediate << 2)
    return None


def same_body_edges(code: bytes, intervals: list[tuple[int, int, int]],
                    boundaries: list[int]) -> set[int]:
    """Find targets of non-call direct control transfers within one Ghidra body."""
    supported = set()
    for offset in range(0, len(code), 4):
        site = offset + 0x80
        word = struct.unpack_from("<I", code, offset)[0]
        target = direct_noncall_target(word, site)
        if target is None:
            continue
        owner = body_owner(site, intervals, boundaries)
        if owner is not None and body_owner(target, intervals, boundaries) == owner:
            supported.add(target)
    return supported


def classify(static: list[dict[str, str]], functions: list[dict[str, str]],
             code: bytes | None = None) -> tuple[dict, str]:
    intervals, covered = parse_ranges(functions)
    boundaries = [row[0] for row in intervals]
    entries = {int(row["entry"], 16) for row in functions}
    supported = same_body_edges(code, intervals, boundaries) if code is not None else set()
    counts: Counter[str] = Counter()
    work = ["# Candidate placement status, never an accepted function claim.",
            "start\tstatic_status\tghidra_owner\tflow_disposition"]
    for row in static:
        start = int(row["start"], 16)
        owner = body_owner(start, intervals, boundaries)
        disposition = ("same_entry" if start in entries else
                       "inside_other_body" if owner is not None else "unowned")
        counts[f"{row['status']}:{disposition}"] += 1
        if disposition == "inside_other_body" and start in supported:
            counts["internal_block_with_same_body_direct_edge"] += 1
        if disposition != "same_entry" or row["status"] in ("no_return", "nonzero_tail"):
            work.append(f"0x{start:08X}\t{row['status']}\t"
                        f"{f'0x{owner:08X}' if owner is not None else ''}\t"
                        f"{disposition}{'+same_body_edge' if start in supported else ''}")
    return ({"ghidra_body_bytes_by_placement": covered,
            "ghidra_functions": len(functions),
            "candidate_entries": len(static),
            "disposition_counts": dict(sorted(
                (key, value) for key, value in counts.items() if ":" in key)),
            "internal_block_with_same_body_direct_edge":
                counts["internal_block_with_same_body_direct_edge"],
            "candidate_internal_block_leads": sum(
                value for key, value in counts.items() if key.endswith(":inside_other_body"))},
            "\n".join(work) + "\n")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()
    inventory = json.loads(INVENTORY.read_text(encoding="utf-8"))
    crosscheck = json.loads(CROSSCHECK.read_text(encoding="utf-8"))
    modules = sorted(row["module"] for row in inventory["modules"])
    if (len(modules) != 21 or crosscheck["module_count"] != 21
            or crosscheck["inventory_sha256"] != digest(INVENTORY)):
        raise ValueError("level boundary inputs are stale")
    if args.check:
        summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
        if (summary["status"] != "flow_range_candidates_only"
                or summary["inventory_sha256"] != digest(INVENTORY)
                or summary["crosscheck_sha256"] != digest(CROSSCHECK)
                or [row["module"] for row in summary["modules"]] != modules
                or summary["overall_percent"] is not None
                or summary["bytematch_ready"] is not False
                or summary["candidate_internal_block_leads"] != sum(
                    row["candidate_internal_block_leads"] for row in summary["modules"])
                or summary["internal_block_with_same_body_direct_edge"] != sum(
                    row["internal_block_with_same_body_direct_edge"]
                    for row in summary["modules"])
                or not 0 <= summary["internal_block_with_same_body_direct_edge"] <=
                summary["candidate_internal_block_leads"]):
            raise ValueError("flow-range snapshot is stale or overclaims")
        print(f"flow-range candidate metadata current: {len(modules)} modules")
        return
    results = []
    private_work = {}
    code_rows = {row["module"]: row for row in rows(CODE_CATALOGUE)}
    if set(code_rows) != set(modules):
        raise ValueError("level code catalogue differs from pinned modules")
    for module in modules:
        path = RANGES / f"{module}.code.bin.ghidra.tsv"
        code = (WORK / f"{module}.code.bin").read_bytes()
        if (len(code) != int(code_rows[module]["interval_bytes"])
                or hashlib.sha256(code).hexdigest() != code_rows[module]["sha256"]):
            raise ValueError(f"{module} private code slice differs from pinned interval")
        summary, work = classify(rows(WORK / f"{module}.tsv"), rows(path),
                                 code)
        original = next(row for row in crosscheck["modules"] if row["module"] == module)
        if summary["ghidra_functions"] != original["ghidra_auto_functions"]:
            raise ValueError(f"{module} Ghidra function count changed")
        summary.update({"module": module, "ranges_export_sha256": digest(path)})
        results.append(summary)
        private_work[module] = work
    report = {"status": "flow_range_candidates_only",
              "module_count": len(modules),
              "ghidra_body_bytes_by_placement": sum(
                  row["ghidra_body_bytes_by_placement"] for row in results),
              "candidate_internal_block_leads": sum(
                  row["candidate_internal_block_leads"] for row in results),
              "internal_block_with_same_body_direct_edge": sum(
                  row["internal_block_with_same_body_direct_edge"] for row in results),
              "inventory_sha256": digest(INVENTORY),
              "crosscheck_sha256": digest(CROSSCHECK),
              "overall_percent": None, "bytematch_ready": False,
              "warning": "Ghidra flow-body ownership is a review lead; omitted data and indirect control flow prevent an exhaustive function proof.",
              "modules": results}
    for module, content in private_work.items():
        (RANGES / f"{module}.flow_review.tsv").write_text(
            content, encoding="utf-8", newline="\n")
    SUMMARY.write_text(json.dumps(report, indent=2) + "\n",
                       encoding="utf-8", newline="\n")
    print(f"{report['candidate_internal_block_leads']} candidate entries fall "
          "inside another Ghidra body; no game-wide percentage")


if __name__ == "__main__":
    main()

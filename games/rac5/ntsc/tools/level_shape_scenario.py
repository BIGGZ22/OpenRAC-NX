#!/usr/bin/env python3
"""Measure an optimistic, relocation-masked level-code shape scenario.

This groups matching *candidate bytes*, not proven source functions. The private
write pass rehashes every selected interval against a pinned lawful disc.
"""

from __future__ import annotations

import argparse
import bisect
import hashlib
import json
from collections import Counter, defaultdict
from pathlib import Path

from disc import IsoDisc, verify_disc
from level_function_inventory import masked_body, relocations
from level_ghidra_crosscheck import rows
from level_switch_bodies import check_summary as check_switch_summary

ROOT = Path(__file__).resolve().parent.parent
WORK = ROOT / "build/level-functions"
OUTPUT = ROOT / "config/levels/shape_scenario_summary.json"
INVENTORY = ROOT / "config/levels/function_inventory_summary.json"
SPLIT = ROOT / "config/levels/split_dedupe_summary.json"
SWITCH = ROOT / "config/levels/switch_body_summary.json"
MODULES = ROOT / "config/levels/modules.tsv"
CODE = ROOT / "config/levels/code_intervals.tsv"
TARGET = ROOT / "config/target.json"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def shape_counts(units: list[tuple[str, int, str, str]]) -> dict[str, int]:
    """Count shapes once or at the maximum observed multiplicity per module."""
    raw: dict[tuple[int, str], Counter[str]] = defaultdict(Counter)
    masked: dict[tuple[int, str], Counter[str]] = defaultdict(Counter)
    for module, size, raw_hash, masked_hash in units:
        if size <= 0 or not raw_hash or not masked_hash:
            raise ValueError("shape unit is missing bytes or a fingerprint")
        raw[size, raw_hash][module] += 1
        masked[size, masked_hash][module] += 1
    return {
        "raw_shape_groups": len(raw),
        "masked_shape_groups": len(masked),
        "raw_max_per_module_bytes": sum(size * max(counts.values())
                                        for (size, _hash), counts in raw.items()),
        "masked_max_per_module_bytes": sum(size * max(counts.values())
                                           for (size, _hash), counts in masked.items()),
        "masked_one_per_shape_bytes": sum(size for size, _hash in masked),
        "masked_repeated_within_module_groups": sum(
            max(counts.values()) > 1 for counts in masked.values()),
    }


def measured_units(image: Path) -> tuple[list[tuple[str, int, str, str]], int, int]:
    inventory = json.loads(INVENTORY.read_text(encoding="utf-8"))
    split = json.loads(SPLIT.read_text(encoding="utf-8"))
    switch = json.loads(SWITCH.read_text(encoding="utf-8"))
    check_switch_summary(switch)
    target = json.loads(TARGET.read_text(encoding="utf-8"))
    modules = {Path(row["path"]).name: row for row in rows(MODULES)}
    code = {row["module"]: row for row in rows(CODE)}
    retained = set(code) - {module for group in
                            inventory["identical_full_interval_groups"]
                            for module in group[1:]}
    if (len(code) != 21 or set(modules) != set(code)
            or len(retained) != 20
            or split["retained_candidate_interval_bytes"] != sum(
                int(code[module]["interval_bytes"]) for module in retained)):
        raise ValueError("pinned module and retained-interval catalogues disagree")
    split_rows = rows(WORK / "split_boundary_review.tsv")
    if digest(WORK / "split_boundary_review.tsv") != split["split_review_sha256"]:
        raise ValueError("private split worklist differs from the pinned audit")
    selected: dict[str, dict[int, list[dict[str, str]]]] = defaultdict(
        lambda: defaultdict(list))
    for row in split_rows:
        if row["module"] not in code:
            raise ValueError("split worklist names an unknown module")
        selected[row["module"]][int(row["parent_start"], 16)].append(row)
    if (len({(module, parent) for module, parents in selected.items()
             for parent in parents}) != split["split_parent_intervals"]
            or len(split_rows) != split["split_segments"]):
        raise ValueError("private split worklist differs from public summary")
    switch_path = WORK / "switch_body_review.tsv"
    if digest(switch_path) != switch["review_sha256"]:
        raise ValueError("private switch worklist differs from public summary")
    switch_rows = rows(switch_path)
    switch_by_module: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in switch_rows:
        if row["module"] not in code:
            raise ValueError("switch worklist names an unknown module")
        switch_by_module[row["module"]].append(row)
    if len(switch_rows) != switch["reviewed_placements"]:
        raise ValueError("private switch worklist count changed")

    units: list[tuple[str, int, str, str]] = []
    represented = 0
    with IsoDisc(image) as disc:
        verify_disc(disc, target)
        for module in sorted(retained):
            data = disc.file_bytes("LVL/" + module)
            if hashlib.sha256(data).hexdigest() != modules[module]["sha256"]:
                raise ValueError(f"{module} differs from pinned module")
            start = int(code[module]["start"], 16)
            code_end = int(code[module]["end_exclusive"], 16)
            indexed, _pointers = relocations(data, code_end)
            sites = sorted(indexed)
            static = rows(WORK / f"{module}.tsv")
            by_start = {int(row["start"], 16): row for row in static}
            if len(by_start) != len(static):
                raise ValueError(f"{module} repeats a static candidate start")
            cursor = start
            switches = sorted(switch_by_module[module],
                              key=lambda item: int(item["start"], 16))
            switch_starts = [int(item["start"], 16) for item in switches]
            for prior, current in zip(switches, switches[1:]):
                if int(prior["end"], 16) > int(current["start"], 16):
                    raise ValueError(f"{module} switch bodies overlap")
            for row in static:
                left, limit = int(row["start"], 16), int(row["limit"], 16)
                size = int(row["body_bytes"])
                if left != cursor or not left < limit <= code_end or size > limit - left:
                    raise ValueError(f"{module} static candidate intervals do not partition code")
                cursor = limit
                position = bisect.bisect_right(switch_starts, left) - 1
                if (position >= 0 and
                        left < int(switches[position]["end"], 16)):
                    if left in selected[module]:
                        raise ValueError(f"{module} switch overlaps selected split parent")
                    continue
                if left in selected[module]:
                    if (row["status"] not in
                            ("bounded_candidate", "pointer_only", "nonzero_tail")
                            or not row["body_end"]):
                        raise ValueError(f"{module} split parent lacks a bounded body")
                    continue
                if size:
                    raw = data[left:left + size]
                    masked, _count = masked_body(raw, left, indexed, sites)
                    if (hashlib.sha256(raw).hexdigest() != row["raw_sha256"]
                            or masked != row["masked_lead_sha256"]):
                        raise ValueError(f"{module} static body differs from pinned image")
                    units.append((module, size, row["raw_sha256"], masked))
                    represented += size
            for row in switches:
                left, right = int(row["start"], 16), int(row["end"], 16)
                if left not in by_start or not start <= left < right <= code_end:
                    raise ValueError(f"{module} switch body escapes candidate code")
                raw = data[left:right]
                masked, _count = masked_body(raw, left, indexed, sites)
                if (hashlib.sha256(raw).hexdigest() != row["raw_sha256"]
                        or masked != row["masked_sha256"]
                        or right - left != int(row["body_bytes"])):
                    raise ValueError(f"{module} switch body differs from pinned image")
                units.append((module, len(raw), row["raw_sha256"], masked))
                represented += len(raw)
            if cursor != code_end:
                raise ValueError(f"{module} static candidate coverage ends early")
            for parent, spans in selected[module].items():
                original = by_start.get(parent)
                if original is None:
                    raise ValueError(f"{module} split parent is absent")
                body_end = int(original["body_end"], 16)
                cursor = parent
                for row in sorted(spans, key=lambda item: int(item["start"], 16)):
                    left, right = int(row["start"], 16), int(row["end"], 16)
                    size = int(row["span_bytes"])
                    if not cursor <= left < right <= body_end or right - left != size:
                        raise ValueError(f"{module} split spans overlap or escape parent")
                    cursor = right
                    raw = data[left:right]
                    masked, _count = masked_body(raw, left, indexed, sites)
                    if (hashlib.sha256(raw).hexdigest() != row["raw_sha256"]
                            or masked != row["masked_sha256"]):
                        raise ValueError(f"{module} split body differs from pinned image")
                    units.append((module, size, row["raw_sha256"], masked))
                    represented += size
    retained_bytes = split["retained_candidate_interval_bytes"]
    residual = retained_bytes - represented
    if residual < 0:
        raise ValueError("candidate bodies exceed retained code intervals")
    return units, retained_bytes, residual


def snapshot(units: list[tuple[str, int, str, str]], retained: int,
             residual: int) -> dict[str, object]:
    counts = shape_counts(units)
    if sum(size for _module, size, _raw, _masked in units) + residual != retained:
        raise ValueError("optimistic accounting does not conserve candidate bytes")
    return {
        "status": "optimistic_masked_shapes_only",
        "module_count": 20,
        "retained_candidate_interval_bytes": retained,
        "represented_body_bytes": retained - residual,
        "unrepresented_bytes_kept_unique": residual,
        "body_units": len(units),
        **counts,
        "raw_max_per_module_scenario_bytes":
            counts["raw_max_per_module_bytes"] + residual,
        "masked_max_per_module_scenario_bytes":
            counts["masked_max_per_module_bytes"] + residual,
        "masked_one_per_shape_scenario_bytes":
            counts["masked_one_per_shape_bytes"] + residual,
        "inventory_sha256": digest(INVENTORY),
        "split_sha256": digest(SPLIT),
        "switch_sha256": digest(SWITCH),
        "module_catalogue_sha256": digest(MODULES),
        "code_catalogue_sha256": digest(CODE),
        "overall_percent": None,
        "bytematch_ready": False,
        "warning": ("Relocation-masked equality preserves instruction and fixup "
                    "descriptors but does not prove shared data roles, function "
                    "boundaries or one C definition. These are optimistic "
                    "shape-only scenarios, not qualified unique-code sizes."),
    }


def check_summary(summary: dict[str, object]) -> None:
    split = json.loads(SPLIT.read_text(encoding="utf-8"))
    switch = json.loads(SWITCH.read_text(encoding="utf-8"))
    check_switch_summary(switch)
    if (summary["status"] != "optimistic_masked_shapes_only"
            or summary["module_count"] != 20
            or summary["retained_candidate_interval_bytes"] !=
            split["retained_candidate_interval_bytes"]
            or summary["represented_body_bytes"] +
            summary["unrepresented_bytes_kept_unique"] !=
            summary["retained_candidate_interval_bytes"]
            or summary["raw_max_per_module_scenario_bytes"] !=
            summary["raw_max_per_module_bytes"] +
            summary["unrepresented_bytes_kept_unique"]
            or summary["masked_max_per_module_scenario_bytes"] !=
            summary["masked_max_per_module_bytes"] +
            summary["unrepresented_bytes_kept_unique"]
            or summary["masked_one_per_shape_scenario_bytes"] !=
            summary["masked_one_per_shape_bytes"] +
            summary["unrepresented_bytes_kept_unique"]
            or not (0 < summary["masked_one_per_shape_scenario_bytes"] <=
                    summary["masked_max_per_module_scenario_bytes"] <=
                    summary["raw_max_per_module_scenario_bytes"] <=
                    summary["retained_candidate_interval_bytes"])
            or summary["inventory_sha256"] != digest(INVENTORY)
            or summary["split_sha256"] != digest(SPLIT)
            or summary["switch_sha256"] != digest(SWITCH)
            or summary["module_catalogue_sha256"] != digest(MODULES)
            or summary["code_catalogue_sha256"] != digest(CODE)
            or summary["overall_percent"] is not None
            or summary["bytematch_ready"] is not False):
        raise ValueError("optimistic shape scenario is stale or overclaims")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", type=Path, metavar="PINNED_ISO")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()
    if args.write:
        units, retained, residual = measured_units(args.write)
        summary = snapshot(units, retained, residual)
        OUTPUT.write_text(json.dumps(summary, indent=2) + "\n",
                          encoding="utf-8", newline="\n")
    else:
        summary = json.loads(OUTPUT.read_text(encoding="utf-8"))
    check_summary(summary)
    print("optimistic masked-code shape scenario: "
          f"{summary['masked_max_per_module_scenario_bytes']} candidate bytes; "
          "not a qualified unique-code denominator")


if __name__ == "__main__":
    main()

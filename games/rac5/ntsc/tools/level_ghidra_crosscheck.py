#!/usr/bin/env python3
"""Compare private Ghidra function bodies with the SNR2 static candidate audit.

Agreement is independent boundary evidence, not a compiler match or proof
that every remaining entry is a function. Only metadata leaves ignored build/.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
WORK = ROOT / "build/level-functions"
INVENTORY = ROOT / "config/levels/function_inventory_summary.json"
SUMMARY = ROOT / "config/levels/ghidra_crosscheck_summary.json"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def rows(path: Path) -> list[dict[str, str]]:
    with path.open(encoding="utf-8", newline="") as file:
        return list(csv.DictReader((line for line in file if not line.startswith("#")),
                                   delimiter="\t"))


def compare(module: str, static: list[dict[str, str]],
            analyzed: list[dict[str, str]]) -> tuple[dict, str, list[dict]]:
    candidates = {int(row["start"], 16): row for row in static}
    functions = {int(row["entry"], 16): row for row in analyzed}
    if len(candidates) != len(static) or len(functions) != len(analyzed):
        raise ValueError(f"duplicate candidate/Ghidra entry in {module}")
    corroborated = []
    mismatched = []
    no_return_shared = 0
    no_return_cross_limit_or_noncontiguous = 0
    for entry, row in functions.items():
        candidate = candidates.get(entry)
        if candidate is None:
            mismatched.append((entry, "ghidra_only", "", "", row["max"], row["body_bytes"]))
            continue
        if candidate["status"] == "no_return":
            no_return_shared += 1
            if (int(row["min"], 16) != entry
                    or int(row["body_bytes"]) != int(row["max"], 16) + 1 - entry
                    or int(row["max"], 16) + 1 > int(candidate["limit"], 16)):
                no_return_cross_limit_or_noncontiguous += 1
        body_end = int(candidate["body_end"], 16) if candidate["body_end"] else None
        ghidra_end = int(row["max"], 16) + 1
        ghidra_size = int(row["body_bytes"])
        if (candidate["status"] in ("bounded_candidate", "pointer_only",
                                    "nonzero_tail")
                and body_end == ghidra_end
                and int(row["min"], 16) == entry
                and ghidra_size == int(candidate["body_bytes"])):
            corroborated.append(candidate)
        else:
            mismatched.append((entry, "boundary_disagreement", candidate["status"],
                               candidate["body_end"], row["max"], row["body_bytes"]))
    for entry, candidate in candidates.items():
        if entry not in functions:
            mismatched.append((entry, "static_only", candidate["status"],
                               candidate["body_end"], "", ""))
    lines = ["# Metadata review worklist; Ghidra auto-analysis is not ground truth.",
             "entry\tdisposition\tstatic_status\tstatic_body_end\tghidra_max\tghidra_body_bytes"]
    for entry, disposition, status, body_end, ghidra_max, ghidra_size in sorted(mismatched):
        lines.append(f"0x{entry:08X}\t{disposition}\t{status}\t{body_end}\t"
                     f"{ghidra_max}\t{ghidra_size}")
    return {
        "module": module,
        "static_candidate_entries": len(candidates),
        "ghidra_auto_functions": len(functions),
        "shared_entries": len(candidates.keys() & functions.keys()),
        "corroborated_boundaries": len(corroborated),
        "corroborated_direct_call_boundaries": sum(
            "call" in row["evidence"] for row in corroborated),
        "corroborated_pointer_only_boundaries": sum(
            row["status"] == "pointer_only" for row in corroborated),
        "corroborated_nonzero_tail_boundaries": sum(
            row["status"] == "nonzero_tail" for row in corroborated),
        "corroborated_body_bytes": sum(int(row["body_bytes"]) for row in corroborated),
        "ghidra_only_entries": len(functions.keys() - candidates.keys()),
        "static_only_entries": len(candidates.keys() - functions.keys()),
        "disagreement_entries": sum(row[1] == "boundary_disagreement" for row in mismatched),
        "no_return_shared_entries": no_return_shared,
        "no_return_cross_limit_or_noncontiguous":
            no_return_cross_limit_or_noncontiguous,
    }, "\n".join(lines) + "\n", corroborated


def dedupe_leads(rows: list[dict]) -> dict[str, int]:
    raw: dict[tuple[int, str], list[dict]] = defaultdict(list)
    masked: dict[tuple[int, str], list[dict]] = defaultdict(list)
    for row in rows:
        size = int(row["body_bytes"])
        raw[(size, row["raw_sha256"])].append(row)
        masked[(size, row["masked_lead_sha256"])].append(row)
    return {
        "raw_exact_distinct_corrob_candidate_bytes": sum(size for size, _hash in raw),
        "masked_conservative_corrob_lead_bytes": sum(
            size if len({row["module"] for row in group}) == len(group)
            else size * len(group)
            for (size, _hash), group in masked.items()),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()
    inventory = json.loads(INVENTORY.read_text(encoding="utf-8"))
    modules = sorted(row["module"] for row in inventory["modules"])
    if len(modules) != 21 or inventory["status"] != "candidate_inventory_only":
        raise ValueError("static inventory is not the pinned 21-module audit")
    if args.check:
        summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
        if (summary["status"] != "independent_candidate_crosscheck"
                or summary["inventory_sha256"] != digest(INVENTORY)
                or [row["module"] for row in summary["modules"]] != modules
                or summary["overall_percent"] is not None
                or summary["bytematch_ready"] is not False
                or summary["identical_interval_analysis_agrees"] is not True
                or summary["corroborated_boundaries"] !=
                sum(row["corroborated_boundaries"] for row in summary["modules"])):
            raise ValueError("Ghidra cross-check snapshot is stale or overclaims")
        print(f"Ghidra candidate cross-check metadata current: {len(modules)} modules")
        return
    results = []
    worklists = {}
    agreed_rows = []
    ghidra_hashes = {}
    for module in modules:
        static_file = WORK / f"{module}.tsv"
        ghidra_file = WORK / f"{module}.code.bin.ghidra.tsv"
        result, worklist, agreed = compare(module, rows(static_file), rows(ghidra_file))
        result["ghidra_export_sha256"] = digest(ghidra_file)
        ghidra_hashes[module] = result["ghidra_export_sha256"]
        results.append(result)
        worklists[module] = worklist
        agreed_rows.extend({**row, "module": module} for row in agreed)
    for identical in inventory["identical_full_interval_groups"]:
        if len({ghidra_hashes[module] for module in identical}) != 1:
            raise ValueError("identical code intervals yielded different Ghidra boundaries")
    summary = {
        "status": "independent_candidate_crosscheck",
        "module_count": len(modules),
        "corroborated_boundaries": sum(row["corroborated_boundaries"] for row in results),
        "corroborated_body_bytes_by_placement":
            sum(row["corroborated_body_bytes"] for row in results),
        **dedupe_leads(agreed_rows),
        "identical_interval_analysis_agrees": True,
        "inventory_sha256": digest(INVENTORY),
        "overall_percent": None,
        "bytematch_ready": False,
        "warning": "Ghidra raw-code auto-analysis and static call/pointer discovery can share errors; disagreements and unreferenced code remain under review.",
        "modules": results,
    }
    for module, content in worklists.items():
        (WORK / f"{module}.crosscheck.tsv").write_text(
            content, encoding="utf-8", newline="\n")
    SUMMARY.write_text(json.dumps(summary, indent=2) + "\n",
                       encoding="utf-8", newline="\n")
    print(f"{summary['corroborated_boundaries']} candidate boundaries agree "
          f"across {len(modules)} modules; no game-wide percentage")


if __name__ == "__main__":
    main()

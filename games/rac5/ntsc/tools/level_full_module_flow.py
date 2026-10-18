#!/usr/bin/env python3
"""Review code-only flow disagreements with a private full-SNR2 Ghidra import.

The result is metadata-only candidate evidence. Ghidra's recovered switches and
function entries remain independent analysis leads, not accepted boundaries.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from collections import Counter, defaultdict
from pathlib import Path

from disc import IsoDisc, verify_disc
from level_flow_ranges import body_owner, digest, direct_noncall_target, parse_ranges
from level_ghidra_crosscheck import rows
from rel import HEADER, iter_packed_relocations

ROOT = Path(__file__).resolve().parent.parent
WORK = ROOT / "build/level-functions"
FLOW = ROOT / "config/levels/flow_range_summary.json"
MODULES = ROOT / "config/levels/modules.tsv"
CODE = ROOT / "config/levels/code_intervals.tsv"
TARGET = ROOT / "config/target.json"
SUMMARY = ROOT / "config/levels/full_module_flow_summary.json"


def candidate_signals(target: int, old_owner: int, full_entries: set[int],
                      ranges: list[tuple[int, int, int]], boundaries: list[int],
                      references: list[dict[str, str]],
                      packed_pointers: dict[int, int],
                      data: bytes) -> tuple[str, dict[str, bool], int | None]:
    owner = body_owner(target, ranges, boundaries)
    entry = target in full_entries
    pointer = computed_jump = direct_jump = external_call = False
    for ref in references:
        site = int(ref["from"], 16)
        if ref["type"] == "DATA" and packed_pointers.get(site) == target:
            pointer = True
        source_owner = body_owner(site, ranges, boundaries)
        if source_owner is None or site + 4 > len(data):
            continue
        word = struct.unpack_from("<I", data, site)[0]
        opcode = word >> 26
        function = word & 63 if opcode == 0 else None
        compatible_call = (function == 9 if ref["computed"] == "true" else
                           opcode == 3 or (opcode == 1 and ((word >> 16) & 31)
                                           in (16, 17, 18, 19)))
        compatible_jump = function == 8 and ((word >> 21) & 31) != 31
        if ref["call"] == "true" and source_owner != owner and compatible_call:
            external_call = True
        if ref["jump"] == "true" and ref["call"] != "true" and source_owner == owner:
            if ref["computed"] == "true" and compatible_jump:
                computed_jump = True
            elif (ref["computed"] != "true" and
                  direct_noncall_target(word, site) == target):
                direct_jump = True
    signals = {"full_entry": entry, "packed_pointer_reference": pointer,
               "same_owner_computed_jump": computed_jump,
               "same_owner_direct_jump": direct_jump,
               "external_call": external_call, "owner_changed": owner != old_owner,
               "linear_predecessor": linear_predecessor(target, owner, ranges,
                                                         boundaries, data)}
    if entry and external_call:
        category = "called_function_lead"
    elif entry:
        category = "uncorroborated_function_entry"
    elif owner == old_owner and pointer and computed_jump:
        category = "computed_dispatch_block_lead"
    elif owner == old_owner and direct_jump:
        category = "direct_flow_block_lead"
    elif owner != old_owner:
        category = "owner_disagreement"
    elif signals["linear_predecessor"]:
        category = "linear_fallthrough_block_lead"
    else:
        category = "unresolved"
    return category, signals, owner


def linear_predecessor(target: int, owner: int | None,
                       ranges: list[tuple[int, int, int]], boundaries: list[int],
                       data: bytes) -> bool:
    """Require a contiguous predecessor and no terminating transfer before it."""
    if (owner is None or target < 0x88 or target > len(data)
            or body_owner(target - 4, ranges, boundaries) != owner):
        return False
    previous = struct.unpack_from("<I", data, target - 4)[0]
    before_previous = struct.unpack_from("<I", data, target - 8)[0]
    for word in (previous, before_previous):
        if (word >> 26) == 0 and (word & 63) in (12, 13):  # syscall/break
            return False
    opcode = before_previous >> 26
    source = (before_previous >> 21) & 31
    dest = (before_previous >> 16) & 31
    if opcode == 2 or (opcode == 0 and (before_previous & 63) == 8):
        return False  # J or JR: target would follow their delay slot
    if (opcode in (4, 20) and source == dest) or (opcode == 6 and source == 0):
        return False  # unconditional BEQ/BEQL/BLEZ $zero
    if opcode == 1 and source == 0 and dest in (1, 3):
        return False  # unconditional BGEZ/BGEZL $zero
    return True


def refine_category(category: str, target: int, owner: int | None,
                    called_entries: set[int]) -> str:
    if category == "owner_disagreement" and owner in called_entries and target > owner:
        return "inside_new_called_body_lead"
    return category


def identical_pair_exports(modules: list[dict]) -> bool:
    by_name = {row["module"]: row for row in modules}
    return all(by_name["LEVEL_15.REL"][key] == by_name["LEVEL_21.REL"][key]
               for key in ("full_export_sha256", "reference_export_sha256"))


def classify_module(module: str, data: bytes, flow_rows: list[dict[str, str]],
                    functions: list[dict[str, str]],
                    reference_rows: list[dict[str, str]]) -> tuple[dict, str]:
    ranges, body_bytes = parse_ranges(functions)
    boundaries = [row[0] for row in ranges]
    entries = {int(row["entry"], 16) for row in functions}
    head = HEADER.unpack_from(data)
    pointers = {row.site: struct.unpack_from("<I", data, row.site)[0]
                for row in iter_packed_relocations(data, head[13], head[1])
                if row.kind == 3}
    candidates = {int(row["start"], 16):
                  (int(row["ghidra_owner"], 16),
                   row["flow_disposition"].endswith("+same_body_edge"))
                  for row in flow_rows
                  if row["flow_disposition"].startswith("inside_other_body")}
    refs: dict[int, list[dict[str, str]]] = defaultdict(list)
    for row in reference_rows:
        target = int(row["target"], 16)
        if target not in candidates:
            raise ValueError(f"{module} Ghidra reference targets an unlisted candidate")
        refs[target].append(row)
    counts: Counter[str] = Counter()
    previous_no_direct: Counter[str] = Counter()
    signal_counts: Counter[str] = Counter()
    work = ["# Full-module Ghidra signals remain review leads, not accepted functions.",
            "target\tcode_only_owner\tfull_module_owner\tcategory\tsignals"]
    classified = []
    for target, (old_owner, old_direct) in sorted(candidates.items()):
        category, signals, owner = candidate_signals(
            target, old_owner, entries, ranges, boundaries, refs[target], pointers,
            data)
        classified.append((target, old_owner, old_direct, owner, category, signals))
    called = {target for target, _old_owner, _old_direct, _owner, category, _signals in classified
              if category == "called_function_lead"}
    for target, old_owner, old_direct, owner, category, signals in classified:
        category = refine_category(category, target, owner, called)
        counts[category] += 1
        if not old_direct:
            previous_no_direct[category] += 1
        signal_counts.update(key for key, value in signals.items() if value)
        work.append(f"0x{target:08X}\t0x{old_owner:08X}\t"
                    f"{f'0x{owner:08X}' if owner is not None else ''}\t"
                    f"{category}\t{','.join(k for k, v in signals.items() if v)}")
    return ({"module": module, "candidates": len(candidates),
             "full_ghidra_functions": len(functions),
             "full_ghidra_body_bytes": body_bytes,
             "categories": dict(sorted(counts.items())),
             "previous_no_direct_categories": dict(sorted(previous_no_direct.items())),
             "signals": dict(sorted(signal_counts.items()))}, "\n".join(work) + "\n")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--extract", type=Path, metavar="PINNED_ISO",
                      help="write verified complete REL files under ignored build/")
    mode.add_argument("--write", type=Path, metavar="PINNED_ISO")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--ghidra-dir", type=Path,
                        help="private full-module Ghidra function and reference TSV directory")
    args = parser.parse_args()
    flow = json.loads(FLOW.read_text(encoding="utf-8"))
    modules = sorted(row["module"] for row in flow["modules"])
    if flow["status"] != "flow_range_candidates_only" or len(modules) != 21:
        raise ValueError("code-only flow review is missing")
    if args.check:
        summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
        if (summary["status"] != "full_module_flow_leads_only"
                or summary["flow_range_sha256"] != digest(FLOW)
                or summary["module_catalogue_sha256"] != digest(MODULES)
                or summary["code_catalogue_sha256"] != digest(CODE)
                or summary["module_count"] != len(modules)
                or [row["module"] for row in summary["modules"]] != modules
                or summary["candidate_entries"] != flow["candidate_internal_block_leads"]
                or summary["candidate_entries"] != sum(
                    row["candidates"] for row in summary["modules"])
                or summary["ignored_noncode_auto_functions"] != sum(
                    row["ignored_noncode_auto_functions"] for row in summary["modules"])
                or sum(summary["categories"].values()) != summary["candidate_entries"]
                or summary["previous_no_direct_entries"] !=
                flow["candidate_internal_block_leads"] -
                flow["internal_block_with_same_body_direct_edge"]
                or sum(summary["previous_no_direct_categories"].values()) !=
                summary["previous_no_direct_entries"]
                or any(sum(row["categories"].values()) != row["candidates"]
                       for row in summary["modules"])
                or Counter(summary["categories"]) != sum(
                    (Counter(row["categories"]) for row in summary["modules"]), Counter())
                or Counter(summary["signals"]) != sum(
                    (Counter(row["signals"]) for row in summary["modules"]), Counter())
                or Counter(summary["previous_no_direct_categories"]) != sum(
                    (Counter(row["previous_no_direct_categories"])
                     for row in summary["modules"]), Counter())
                or summary["identical_level15_21_exports"] !=
                identical_pair_exports(summary["modules"])
                or summary["identical_level15_21_exports"] is not True
                or summary["overall_percent"] is not None
                or summary["bytematch_ready"] is not False):
            raise ValueError("full-module flow summary is stale or overclaims")
        print(f"full-module flow candidate metadata current: {len(modules)} modules")
        return
    target = json.loads(TARGET.read_text(encoding="utf-8"))
    module_catalogue = {Path(row["path"]).name: row for row in rows(MODULES)}
    code_catalogue = {row["module"]: row for row in rows(CODE)}
    if set(module_catalogue) != set(modules) or set(code_catalogue) != set(modules):
        raise ValueError("module catalogue differs from flow review")
    if args.extract is not None:
        destination = WORK / "full"
        destination.mkdir(parents=True, exist_ok=True)
        with IsoDisc(args.extract) as disc:
            verify_disc(disc, target)
            for module in modules:
                data = disc.file_bytes("LVL/" + module)
                if hashlib.sha256(data).hexdigest() != module_catalogue[module]["sha256"]:
                    raise ValueError(f"{module} differs from pinned SNR2 module")
                (destination / module).write_bytes(data)
        print(f"{len(modules)} pinned full REL files extracted to ignored build/")
        return
    if args.ghidra_dir is None:
        parser.error("--ghidra-dir is required with --write")
    results = []
    worklists = {}
    with IsoDisc(args.write) as disc:
        verify_disc(disc, target)
        for module in modules:
            data = disc.file_bytes("LVL/" + module)
            if hashlib.sha256(data).hexdigest() != module_catalogue[module]["sha256"]:
                raise ValueError(f"{module} differs from pinned SNR2 module")
            functions_path = args.ghidra_dir / f"{module}.ghidra.tsv"
            refs_path = args.ghidra_dir / f"{module}.refs.tsv"
            exported = rows(functions_path)
            code_end = int(code_catalogue[module]["end_exclusive"], 16)
            module_end = len(data)
            # BinaryLoader seeds a synthetic entry at address zero even when
            # the SNR2 header block is non-executable.
            header = [row for row in exported if int(row["entry"], 16) < 0x80]
            if (len(header) != 1 or int(header[0]["entry"], 16) != 0
                    or int(header[0]["max"], 16) >= 0x80):
                raise ValueError(f"{module} has unexpected full-import header functions")
            # Some auto-analyzers invent tiny functions in non-executable data.
            # Keep them visible as rejected metadata, never as code owners.
            noncode = [row for row in exported if int(row["entry"], 16) >= code_end]
            if any(not (code_end <= int(row["min"], 16) <=
                        int(row["max"], 16) < module_end) for row in noncode):
                raise ValueError(f"{module} Ghidra data function crosses code boundary")
            functions = [row for row in exported
                         if 0x80 <= int(row["entry"], 16) < code_end]
            if any(not (0x80 <= int(row["entry"], 16) < code_end and
                        0x80 <= int(row["min"], 16) <= int(row["max"], 16) < code_end)
                   for row in functions):
                raise ValueError(f"{module} full Ghidra function escapes pinned code interval")
            result, work = classify_module(
                module, data, rows(WORK / "ranges" / f"{module}.flow_review.tsv"),
                functions, rows(refs_path))
            previous = next(row for row in flow["modules"] if row["module"] == module)
            if result["candidates"] != previous["candidate_internal_block_leads"]:
                raise ValueError(f"{module} candidate count changed")
            result["full_export_sha256"] = digest(functions_path)
            result["reference_export_sha256"] = digest(refs_path)
            result["ignored_noncode_auto_functions"] = len(header) + len(noncode)
            results.append(result)
            worklists[module] = work
    categories: Counter[str] = Counter()
    prior_categories: Counter[str] = Counter()
    signals: Counter[str] = Counter()
    for result in results:
        categories.update(result["categories"])
        prior_categories.update(result["previous_no_direct_categories"])
        signals.update(result["signals"])
    summary = {"status": "full_module_flow_leads_only",
               "module_count": len(modules),
               "candidate_entries": sum(row["candidates"] for row in results),
               "ignored_noncode_auto_functions": sum(
                   row["ignored_noncode_auto_functions"] for row in results),
               "categories": dict(sorted(categories.items())),
               "previous_no_direct_entries": sum(prior_categories.values()),
               "previous_no_direct_categories": dict(sorted(prior_categories.items())),
               "signals": dict(sorted(signals.items())),
               "identical_level15_21_exports": identical_pair_exports(results),
               "flow_range_sha256": digest(FLOW),
               "module_catalogue_sha256": digest(MODULES),
               "code_catalogue_sha256": digest(CODE),
               "overall_percent": None, "bytematch_ready": False,
               "warning": "Full-module Ghidra switch and call references are corroboration leads, not accepted retail function boundaries or a unique-code denominator.",
               "modules": results}
    review_dir = WORK / "full-flow-review"
    review_dir.mkdir(parents=True, exist_ok=True)
    for module, content in worklists.items():
        (review_dir / f"{module}.tsv").write_text(content, encoding="utf-8", newline="\n")
    SUMMARY.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8", newline="\n")
    print(f"full-module Ghidra signals reviewed: {summary['candidate_entries']} "
          "code-only inside-body candidates; no game-wide percentage")


if __name__ == "__main__":
    main()

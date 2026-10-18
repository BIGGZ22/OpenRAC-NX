#!/usr/bin/env python3
"""Find strongly corroborated relocation-translation templates across levels.

This is a structural shared-code audit, not source-level identity or a
ByteMatch denominator. It requires private Ghidra exports and the pinned ISO.
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

from disc import IsoDisc, verify_disc
from level_ghidra_crosscheck import rows
from rel import HEADER, packed_relocations_by_site

ROOT = Path(__file__).resolve().parent.parent
WORK = ROOT / "build/level-functions"
SUMMARY = ROOT / "config/levels/translation_families_summary.json"
INVENTORY = ROOT / "config/levels/function_inventory_summary.json"
GHIDRA_CROSSCHECK = ROOT / "config/levels/ghidra_crosscheck_summary.json"
CODE_CATALOGUE = ROOT / "config/levels/code_intervals.tsv"
MODULE_CATALOGUE = ROOT / "config/levels/modules.tsv"
TARGET = ROOT / "config/target.json"
WORD = struct.Struct("<I")
DATA_WINDOW = 64


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def uniform_translation(reference: dict[int, int], current: dict[int, int]) -> int | None:
    if reference.keys() != current.keys():
        return None
    if not reference:
        return 0  # No HI16 data target needs relocation; verify unpaired lows separately.
    shifts = {current[site] - value for site, value in reference.items()}
    return shifts.pop() if len(shifts) == 1 else None


def lows_follow_translation(reference: dict[int, int], current: dict[int, int],
                            shift: int) -> bool:
    return (reference.keys() == current.keys()
            and all(((current[site] - value) & 0xFFFF) == (shift & 0xFFFF)
                    for site, value in reference.items()))


def unique_data_window(data: bytes, code_end: int, payload_end: int,
                       target: int) -> bytes | None:
    """Return an exact local data signature only when it occurs once."""
    if not code_end <= target <= payload_end - DATA_WINDOW:
        return None
    signature = data[target:target + DATA_WINDOW]
    if (data.find(signature, code_end, payload_end) != target or
            data.find(signature, target + 1, payload_end) >= 0):
        return None
    return signature


def unique_target_shifts(reference: tuple[dict[int, int], dict[int, int]],
                         current: tuple[dict[int, int], dict[int, int]],
                         reference_data: bytes, current_data: bytes,
                         reference_code_end: int, current_code_end: int,
                         reference_payload_end: int, current_payload_end: int,
                         reference_cache: dict[int, bytes | None],
                         current_cache: dict[int, bytes | None]) -> tuple[int, ...] | None:
    """Require unique matching data bytes at every independently addressed target."""
    base_hi, base_low = reference
    hi, low = current
    if not base_hi or base_hi.keys() != hi.keys() or base_low.keys() != low.keys():
        return None
    shifts = set()
    for site, target in base_hi.items():
        other = hi[site]
        if target not in reference_cache:
            reference_cache[target] = unique_data_window(
                reference_data, reference_code_end, reference_payload_end, target)
        if other not in current_cache:
            current_cache[other] = unique_data_window(
                current_data, current_code_end, current_payload_end, other)
        signature = reference_cache[target]
        if signature is None or signature != current_cache[other]:
            return None
        shifts.add(other - target)
    if any(((low[site] - value) & 0xFFFF) not in
           {shift & 0xFFFF for shift in shifts} for site, value in base_low.items()):
        return None
    return tuple(sorted(shifts))


def local_references(data: bytes, start: int, end: int, code_end: int,
                     fixups: list, sites: list[int]) -> tuple[dict[int, int], dict[int, int]] | None:
    hi_targets = {}
    paired_low = set()
    low_values = {}
    for fixup in fixups[bisect.bisect_left(sites, start):bisect.bisect_left(sites, end)]:
        site = fixup.site - start
        if fixup.kind in (2, 3):
            return None  # local code jump or full pointer needs a separate graph audit
        if fixup.kind == 0:
            low_values[site] = WORD.unpack_from(data, fixup.site)[0] & 0xFFFF
            continue
        if fixup.kind != 1 or fixup.low_site is None or not start <= fixup.low_site < end:
            return None
        high = WORD.unpack_from(data, fixup.site)[0] & 0xFFFF
        low = WORD.unpack_from(data, fixup.low_site)[0] & 0xFFFF
        if low & 0x8000:
            low -= 0x10000
        target = (high << 16) + low
        if not code_end <= target < len(data):
            return None
        hi_targets[site] = target
        paired_low.add(fixup.low_site - start)
    return hi_targets, {site: value for site, value in low_values.items()
                        if site not in paired_low}


def ghidra_confirmed(row: dict[str, str], functions: dict[int, dict[str, str]]) -> bool:
    start = int(row["start"], 16)
    function = functions.get(start)
    return bool(function and row["body_end"]
                and int(function["min"], 16) == start
                and int(function["max"], 16) + 1 == int(row["body_end"], 16)
                and int(function["body_bytes"]) == int(row["body_bytes"]))


def local_code_references(data: bytes, start: int, end: int, fixups: list,
                          known_entries: dict[tuple[str, int], tuple],
                          module: str) -> dict[int, tuple[str, int | tuple]] | None:
    """Resolve each local J/JAL to an internal offset or corroborated callee.

    Unknown targets and 32-bit code pointers fail closed. The masked body hash
    already fixes the instruction opcode and relocation-site positions.
    """
    references = {}
    for fixup in fixups:
        if fixup.kind == 3:
            return None
        if fixup.kind != 2:
            continue
        word = WORD.unpack_from(data, fixup.site)[0]
        if word >> 26 not in (2, 3):
            return None
        target = (word & 0x03FFFFFF) << 2
        if start <= target < end:
            references[fixup.site - start] = ("internal", target - start)
        elif (module, target) in known_entries:
            references[fixup.site - start] = ("callee", known_entries[(module, target)])
        else:
            return None
    return references


def close_callgraph(pending: dict[tuple[int, str], tuple[dict, set]],
                    seeds: set[tuple[int, str]]) -> set[tuple[int, str]]:
    """Accept a family only when every external callee family is accepted."""
    accepted = set(seeds)
    while True:
        fresh = {key for key, (_group, targets) in pending.items()
                 if key not in accepted and targets <= accepted | {key}}
        if not fresh:
            return accepted - seeds
        accepted.update(fresh)


def combined_dedupe_savings(exact_bodies: dict[tuple[int, str], list[dict]],
                            accepted: list[dict], retained: set[str]) -> tuple[int, int]:
    """Union exact and structural bodies without counting overlapping links twice."""
    parent: dict[tuple[str, int], tuple[str, int]] = {}
    sizes: dict[tuple[str, int], int] = {}

    def add(node: tuple[str, int], size: int) -> None:
        if node in sizes and sizes[node] != size:
            raise ValueError("candidate body has conflicting sizes")
        parent.setdefault(node, node)
        sizes[node] = size

    def root(node: tuple[str, int]) -> tuple[str, int]:
        while parent[node] != node:
            parent[node] = parent[parent[node]]
            node = parent[node]
        return node

    def union(nodes: list[tuple[str, int]]) -> None:
        if not nodes:
            return
        for node in nodes[1:]:
            if sizes[node] != sizes[nodes[0]]:
                raise ValueError("candidate group mixes body sizes")
            parent[root(node)] = root(nodes[0])

    def savings() -> int:
        components: Counter[tuple[str, int]] = Counter(root(node) for node in parent)
        return sum(sizes[node] * (count - 1) for node, count in components.items())

    for (size, _fingerprint), group in exact_bodies.items():
        kept = [row for row in group if row["module"] in retained]
        if len({row["module"] for row in kept}) != len(kept):
            continue  # Repeated identical bodies in one module remain separate.
        nodes = [(row["module"], int(row["start"], 16)) for row in kept]
        for node in nodes:
            add(node, size)
        union(nodes)
    exact_savings = savings()
    for group in accepted:
        nodes = [(module, start) for module, start in group["placements"]
                 if module in retained]
        if len({module for module, _start in nodes}) != len(nodes):
            raise ValueError("structural family repeats a module")
        for node in nodes:
            add(node, group["body_bytes"])
        union(nodes)
    return exact_savings, savings()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", type=Path, metavar="PINNED_ISO")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()
    inventory = json.loads(INVENTORY.read_text(encoding="utf-8"))
    crosscheck = json.loads(GHIDRA_CROSSCHECK.read_text(encoding="utf-8"))
    if inventory["status"] != "candidate_inventory_only":
        raise ValueError("function inventory is not the pinned candidate audit")
    if (crosscheck["status"] != "independent_candidate_crosscheck"
            or crosscheck["module_count"] != 21
            or crosscheck["inventory_sha256"] != digest(INVENTORY)
            or crosscheck["identical_interval_analysis_agrees"] is not True):
        raise ValueError("Ghidra boundary cross-check is missing or stale")
    if args.check:
        summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
        if (summary["status"] != "structural_translation_leads"
                or summary["inventory_sha256"] != digest(INVENTORY)
                or summary["ghidra_crosscheck_sha256"] != digest(GHIDRA_CROSSCHECK)
                or summary["overall_percent"] is not None
                or summary["bytematch_ready"] is not False
                or summary["module_count"] != 21
                or not 0 <= summary["callgraph_resolved_groups"] <= summary["group_count"]
                or not 0 <= summary["callgraph_placement_body_bytes"] <=
                summary["placement_body_bytes"]
                or not 0 <= summary["groups_with_pointer_or_nonzero_tail"] <=
                summary["group_count"]
                or not 0 <= summary["placement_bytes_with_pointer_or_nonzero_tail"] <=
                summary["placement_body_bytes"]
                or not 0 <= summary["unique_data_window_groups"] <=
                summary["group_count"]
                or not 0 <= summary["unique_data_window_placement_bytes"] <=
                summary["placement_body_bytes"]
                or summary["template_lead_upper_bound_bytes"] !=
                inventory["exact_byte_dedupe_upper_bound"] -
                summary["extra_candidate_savings_beyond_exact"]):
            raise ValueError("translation-family snapshot is stale or overclaims")
        print(f"translation leads current: {summary['group_count']} groups")
        return
    target = json.loads(TARGET.read_text(encoding="utf-8"))
    interval_rows = {row["module"]: row for row in rows(CODE_CATALOGUE)}
    module_rows = {Path(row["path"]).name: row for row in rows(MODULE_CATALOGUE)}
    modules = sorted(interval_rows)
    if len(modules) != 21 or set(module_rows) != set(modules):
        raise ValueError("level catalogues differ from 21 pinned modules")
    bodies: dict[tuple[int, str], list[dict]] = defaultdict(list)
    exact_bodies: dict[tuple[int, str], list[dict]] = defaultdict(list)
    unconfirmed_bodies: dict[tuple[int, str], list[dict]] = defaultdict(list)
    ghidra = {}
    known_entries = {}
    for module in modules:
        file = WORK / f"{module}.code.bin.ghidra.tsv"
        ghidra[module] = {int(row["entry"], 16): row for row in rows(file)}
        for row in rows(WORK / f"{module}.tsv"):
            if row["status"] == "bounded_candidate":
                exact_bodies[(int(row["body_bytes"]), row["raw_sha256"])].append(
                    {**row, "module": module})
            confirmed = ghidra_confirmed(row, ghidra[module])
            if row["status"] in ("bounded_candidate", "pointer_only", "nonzero_tail"):
                key = (int(row["body_bytes"]), row["masked_lead_sha256"])
                if confirmed:
                    bodies[key].append({**row, "module": module})
                elif row["status"] == "bounded_candidate":
                    unconfirmed_bodies[key].append({**row, "module": module})
            if (row["status"] in ("bounded_candidate", "pointer_only", "nonzero_tail")
                    and confirmed):
                known_entries[(module, int(row["start"], 16))] = (
                    int(row["body_bytes"]), row["masked_lead_sha256"])
    accepted = []
    base_keys = set()
    pending = {}
    reasons = Counter()
    reasons["boundary_disagreement"] = sum(
        len(group) + len(bodies.get(key, [])) >= 2
        for key, group in unconfirmed_bodies.items())
    retained = set(modules) - {name for group in inventory["identical_full_interval_groups"]
                               for name in group[1:]}
    with IsoDisc(args.write) as disc:
        verify_disc(disc, target)
        data = {module: disc.file_bytes("LVL/" + module) for module in modules}
    for module in modules:
        if hashlib.sha256(data[module]).hexdigest() != module_rows[module]["sha256"]:
            raise ValueError(f"{module} differs from pinned SNR2 file")
    fixups = {}
    fixup_sites = {}
    payload_ends = {}
    signature_cache = {module: {} for module in modules}
    for module in modules:
        head = HEADER.unpack_from(data[module])
        payload_ends[module] = head[1]
        fixups[module] = packed_relocations_by_site(
            data[module], head[13], head[1])
        fixup_sites[module] = [row.site for row in fixups[module]]
    for (size, fingerprint), group in bodies.items():
        if len(group) < 2:
            continue
        names = [row["module"] for row in group]
        if len(set(names)) != len(names):
            reasons["repeated_module"] += 1
            continue
        if not all(ghidra_confirmed(row, ghidra[row["module"]]) for row in group):
            raise ValueError("uncorroborated body entered structural group")
        references = []
        call_references = []
        has_code_relocation = False
        for row in group:
            module = row["module"]
            start = int(row["start"], 16)
            code_end = int(interval_rows[module]["end_exclusive"], 16)
            local = fixups[module][bisect.bisect_left(fixup_sites[module], start):
                                   bisect.bisect_left(fixup_sites[module], start + size)]
            code = local_code_references(data[module], start, start + size,
                                         local, known_entries, module)
            if code is None:
                break
            has_code_relocation |= bool(code)
            data_fixups = [fixup for fixup in local if fixup.kind != 2]
            refs = local_references(data[module], start, start + size, code_end,
                                    data_fixups, [fixup.site for fixup in data_fixups])
            if refs is None:
                break
            references.append(refs)
            call_references.append(code)
        if len(references) != len(group):
            reasons["code_relocation_or_unpaired_hi"] += 1
            continue
        if any(code != call_references[0] for code in call_references):
            reasons["different_local_code_targets"] += 1
            continue
        ref_hi, ref_unpaired = references[0]
        translations = []
        for hi, unpaired in references:
            shift = uniform_translation(ref_hi, hi)
            if shift is None or not lows_follow_translation(ref_unpaired,
                                                              unpaired, shift):
                break
            translations.append(shift)
        if len(translations) != len(group):
            reference_module = group[0]["module"]
            translations = []
            for row, current in zip(group, references):
                module = row["module"]
                shifts = unique_target_shifts(
                    references[0], current, data[reference_module], data[module],
                    int(interval_rows[reference_module]["end_exclusive"], 16),
                    int(interval_rows[module]["end_exclusive"], 16),
                    payload_ends[reference_module], payload_ends[module],
                    signature_cache[reference_module], signature_cache[module])
                if shifts is None:
                    break
                translations.append(shifts)
            if len(translations) != len(group):
                reasons["nonuniform_data_translation"] += 1
                continue
            resolution = "unique_data_window"
        else:
            resolution = "callgraph" if has_code_relocation else "uniform_data"
        item = {"body_bytes": size, "fingerprint": fingerprint,
                         "placements": [(row["module"], int(row["start"], 16))
                                        for row in group],
                         "raw_variants": len({row["raw_sha256"] for row in group}),
                         "data_deltas": translations,
                         "uses_local_code_relocation": has_code_relocation,
                         "includes_pointer_or_nonzero_tail": any(
                             row["status"] != "bounded_candidate" for row in group),
                         "resolution": resolution}
        key = (size, fingerprint)
        if has_code_relocation:
            targets = {value[1] for value in call_references[0].values()
                       if value[0] == "callee"}
            pending[key] = (item, targets)
        else:
            accepted.append(item)
            base_keys.add(key)
    resolved_keys = close_callgraph(pending, base_keys)
    accepted.extend(pending[key][0] for key in sorted(resolved_keys))
    reasons["unresolved_callgraph_dependencies"] = len(pending) - len(resolved_keys)
    exact_savings, combined_savings = combined_dedupe_savings(
        exact_bodies, accepted, retained)
    retained_interval_bytes = sum(int(interval_rows[module]["interval_bytes"])
                                  for module in retained)
    if retained_interval_bytes - exact_savings != inventory["exact_byte_dedupe_upper_bound"]:
        raise ValueError("exact body deduplication differs from pinned inventory")
    extra_savings = combined_savings - exact_savings
    summary = {
        "status": "structural_translation_leads",
        "module_count": len(modules),
        "group_count": len(accepted),
        "placement_body_bytes": sum(group["body_bytes"] * len(group["placements"])
                                    for group in accepted),
        "template_body_bytes": sum(group["body_bytes"] for group in accepted),
        "callgraph_resolved_groups": len(resolved_keys),
        "callgraph_placement_body_bytes": sum(
            group["body_bytes"] * len(group["placements"]) for group in accepted
            if group["uses_local_code_relocation"]),
        "groups_with_pointer_or_nonzero_tail": sum(
            group["includes_pointer_or_nonzero_tail"] for group in accepted),
        "placement_bytes_with_pointer_or_nonzero_tail": sum(
            group["body_bytes"] * len(group["placements"]) for group in accepted
            if group["includes_pointer_or_nonzero_tail"]),
        "unique_data_window_groups": sum(
            group["resolution"] == "unique_data_window" for group in accepted),
        "unique_data_window_placement_bytes": sum(
            group["body_bytes"] * len(group["placements"]) for group in accepted
            if group["resolution"] == "unique_data_window"),
        "extra_candidate_savings_beyond_exact": extra_savings,
        "template_lead_upper_bound_bytes": retained_interval_bytes - combined_savings,
        "rejection_counts": dict(sorted(reasons.items())),
        "largest_groups": [
            {"body_bytes": group["body_bytes"],
             "placements": len(group["placements"]),
             "fingerprint": group["fingerprint"]}
            for group in sorted(accepted, key=lambda item: item["body_bytes"] *
                                (len(item["placements"]) - 1), reverse=True)[:20]],
        "inventory_sha256": digest(INVENTORY),
        "ghidra_crosscheck_sha256": digest(GHIDRA_CROSSCHECK),
        "overall_percent": None,
        "bytematch_ready": False,
        "warning": "The candidate-size scenario assumes each structural lead is one shared template. Unique 64-byte data windows corroborate target bytes but not source-level identity; complete function ownership is also unproven. This is not an unconditional upper bound or progress denominator.",
    }
    lines = ["# Structural translation leads only; no accepted C or denominator claims.",
             "body_bytes\tplacements\traw_variants\tresolution\tfingerprint\tmembers_and_data_deltas"]
    for group in sorted(accepted, key=lambda item: -item["body_bytes"] *
                        (len(item["placements"]) - 1)):
        members = ";".join(
            f"{module}:0x{start:X}:" +
            (",".join(f"{shift:+d}" for shift in delta) if isinstance(delta, tuple)
             else f"{delta:+d}")
            for (module, start), delta in zip(group["placements"],
                                              group["data_deltas"]))
        lines.append(f"{group['body_bytes']}\t{len(group['placements'])}\t"
                     f"{group['raw_variants']}\t{group['resolution']}\t"
                     f"{group['fingerprint']}\t{members}")
    (WORK / "translation_review_leads.tsv").write_text(
        "\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    SUMMARY.write_text(json.dumps(summary, indent=2) + "\n",
                       encoding="utf-8", newline="\n")
    print(f"{len(accepted)} structural leads, including {len(resolved_keys)} "
          "closed local-callgraph groups; "
          "no game-wide percentage")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Corroborate repeated level-code shapes with local context and data anchors.

Candidate equivalence remains conditional. The private write pass requires the
pinned ISO, Ghidra boundaries and previously reviewed relocation targets.
"""

from __future__ import annotations

import argparse
import bisect
import hashlib
import json
from collections import defaultdict
from pathlib import Path

from disc import IsoDisc, verify_disc
from level_function_inventory import masked_body, relocations
from level_ghidra_crosscheck import rows
from level_shape_scenario import measured_units
from level_switch_bodies import check_summary as check_switch_summary
from level_translation_families import (combined_dedupe_savings, ghidra_confirmed,
                                        local_code_references, local_references,
                                        unique_data_window)
from rel import HEADER, PackedRelocation, packed_relocations_by_site

ROOT = Path(__file__).resolve().parent.parent
WORK = ROOT / "build/level-functions"
OUTPUT = ROOT / "config/levels/context_translation_summary.json"
REVIEW = WORK / "context_translation_review.tsv"
INVENTORY = ROOT / "config/levels/function_inventory_summary.json"
SPLIT = ROOT / "config/levels/split_dedupe_summary.json"
SHAPE = ROOT / "config/levels/shape_scenario_summary.json"
SWITCH = ROOT / "config/levels/switch_body_summary.json"
TRANSLATION = ROOT / "config/levels/translation_families_summary.json"
GHIDRA = ROOT / "config/levels/ghidra_crosscheck_summary.json"
FULL_FLOW = ROOT / "config/levels/full_module_flow_summary.json"
MODULES = ROOT / "config/levels/modules.tsv"
CODE = ROOT / "config/levels/code_intervals.tsv"
TARGET = ROOT / "config/target.json"
REFERENCE = "LEVEL_01.REL"
REFERENCES = (REFERENCE, "LEVEL_16.REL", "LEVEL_08.REL")
MAX_ANCHOR_DISTANCE = 65536
MAX_ZERO_RUN = 4096


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def member_map(text: str, data_deltas: bool) -> dict[str, int]:
    result = {}
    for part in text.split(";"):
        fields = part.split(":")
        if len(fields) != (3 if data_deltas else 2):
            raise ValueError("invalid private family member")
        result[fields[0]] = int(fields[1], 16)
    return result


def context_key(static: list[dict[str, str]], index: int) -> str:
    neighborhood = [(static[j]["body_bytes"], static[j]["masked_lead_sha256"])
                    if 0 <= j < len(static) else None
                    for j in range(index - 1, index + 2)]
    return hashlib.sha256(json.dumps(neighborhood, separators=(",", ":"))
                          .encode("ascii")).hexdigest()


def short_zero_run(data: bytes, code_end: int, payload_end: int,
                   target: int) -> tuple[int, int] | None:
    """Return a bounded zero-run interval without assuming its flanks match."""
    if not code_end + 8 <= target < payload_end - 8 or data[target] != 0:
        return None
    left = target
    while left > code_end + 8 and target - left <= MAX_ZERO_RUN and data[left - 1] == 0:
        left -= 1
    right = target
    while right < payload_end - 8 and right - target <= MAX_ZERO_RUN and data[right] == 0:
        right += 1
    if not 64 <= right - left <= MAX_ZERO_RUN:
        return None
    return left, right


def unique_zero_run(data: bytes, code_end: int, payload_end: int,
                    target: int) -> tuple[int, bytes] | None:
    """Identify a short zero run by its exact unique nonzero flanks."""
    bounds = short_zero_run(data, code_end, payload_end, target)
    if bounds is None:
        return None
    left, right = bounds
    signature = data[left - 8:right + 8]
    if (data.find(signature, code_end, payload_end) != left - 8 or
            data.find(signature, left - 7, payload_end) >= 0):
        return None
    return target - left, signature


def code_pointer_fixups(data: bytes, local: list, start: int, end: int,
                        code_end: int) -> tuple[dict[int, int], set[int]]:
    """Find complete local HI/LO pairs aimed at code, including split entries."""
    targets = {}
    paired_sites = set()
    for item in local:
        if (item.kind != 1 or item.low_site is None or
                not start <= item.low_site < end):
            continue
        high = int.from_bytes(data[item.site:item.site + 4], "little") & 0xFFFF
        low = int.from_bytes(data[item.low_site:item.low_site + 4], "little") & 0xFFFF
        target = (high << 16) + ((low ^ 0x8000) - 0x8000)
        if 0x80 <= target < code_end and target % 4 == 0:
            targets[item.site] = target
            paired_sites.update((item.site, item.low_site))
    return targets, paired_sites


def semantic_hi_pairs(data: bytes, local: list, start: int, end: int,
                      code_end: int) -> dict[int, PackedRelocation]:
    """Resolve a cross-body loader HI/LO using a corroborated local delay-slot LO.

    The packed stream can select a later LO in another function. This narrow
    pattern proves the local ADDIU uses the same low immediate and register,
    so replacing the lookup site does not change the loader's target value.
    """
    low_sites = {item.site for item in local if item.kind == 0}
    resolved = {}
    for item in local:
        if (item.kind != 1 or item.low_site is None or
                start <= item.low_site < end or item.site + 12 > end or
                item.site + 8 not in low_sites):
            continue
        high, branch, low = (int.from_bytes(data[item.site + offset:
                                                item.site + offset + 4], "little")
                             for offset in (0, 4, 8))
        high_register = (high >> 16) & 31
        if (high >> 26 != 15 or (high >> 21) & 31 or not high_register or
                branch >> 26 != 4 or (branch >> 21) & 31 or
                (branch >> 16) & 31 or low >> 26 != 9 or
                ((low >> 21) & 31) != high_register or
                (low & 0xFFFF) !=
                (int.from_bytes(data[item.low_site:item.low_site + 4],
                                "little") & 0xFFFF)):
            continue
        signed_low = ((low & 0xFFFF) ^ 0x8000) - 0x8000
        target = ((high & 0xFFFF) << 16) + signed_low
        if code_end <= target < len(data):
            resolved[item.site] = PackedRelocation(1, item.site, item.site + 8)
    return resolved


def context_units(static: list[dict[str, str]], split_by_parent: dict[int, list],
                  data: bytes, switches: list[dict[str, str]] | None = None
                  ) -> list[dict[str, str]]:
    """Replace each reviewed parent with its verified spans and exact gaps."""
    result = []
    for row in static:
        parent = int(row["start"], 16)
        spans = split_by_parent.get(parent)
        if not spans:
            result.append({**row, "kind": "static"})
            continue
        cursor = parent
        limit = int(row["limit"], 16)
        for span in sorted(spans, key=lambda item: int(item["start"], 16)):
            left, right = int(span["start"], 16), int(span["end"], 16)
            if not cursor <= left < right <= limit:
                raise ValueError("split context span escapes or overlaps parent")
            if left > cursor:
                result.append({"start": f"0x{cursor:08X}",
                               "body_bytes": str(left - cursor),
                               "masked_lead_sha256": "gap:" + hashlib.sha256(
                                   data[cursor:left]).hexdigest(),
                               "kind": "gap"})
            result.append({"start": span["start"],
                           "body_bytes": span["span_bytes"],
                           "masked_lead_sha256": span["masked_sha256"],
                           "kind": "split"})
            cursor = right
        if cursor < limit:
            result.append({"start": f"0x{cursor:08X}",
                           "body_bytes": str(limit - cursor),
                           "masked_lead_sha256": "gap:" + hashlib.sha256(
                               data[cursor:limit]).hexdigest(),
                           "kind": "gap"})
    if not switches:
        return result
    ordered = sorted(switches, key=lambda row: int(row["start"], 16))
    starts = [int(row["start"], 16) for row in ordered]
    merged = []
    for row in result:
        start = int(row["start"], 16)
        index = bisect.bisect_right(starts, start) - 1
        if index < 0 or start >= int(ordered[index]["end"], 16):
            merged.append(row)
            continue
        switch = ordered[index]
        end = int(switch["end"], 16)
        if start == starts[index]:
            merged.append({"start": switch["start"],
                           "body_bytes": switch["body_bytes"],
                           "masked_lead_sha256": switch["masked_sha256"],
                           "kind": "switch"})
        if row["kind"] == "static" and int(row["limit"], 16) > end:
            limit = int(row["limit"], 16)
            merged.append({"start": f"0x{end:08X}",
                           "body_bytes": str(limit - end),
                           "masked_lead_sha256": "gap:" + hashlib.sha256(
                               data[end:limit]).hexdigest(),
                           "kind": "gap"})
    return merged


def candidate_boundary(row: dict[str, str], code_functions: dict,
                       full_functions: dict) -> str | None:
    """Accept a full-module-only function only with an independent call entry."""
    if ghidra_confirmed(row, code_functions):
        return "code"
    start = int(row["start"], 16)
    if (row["status"] in ("bounded_candidate", "nonzero_tail")
            and row["evidence"] == "call"
            and start not in code_functions
            and ghidra_confirmed(row, full_functions)):
        return "full_call"
    return None


def anchored_target(source_module: str, target_module: str,
                    source: int, target: int, anchors: dict,
                    reverse: dict, ordered: list[int],
                    source_data: bytes, target_data: bytes,
                    source_code_end: int, target_code_end: int,
                    source_payload_end: int, target_payload_end: int,
                    windows: dict) -> bool:
    """Require a unique direct, exact-data, or two-sided regional witness."""
    if not (source_code_end <= source <= source_payload_end - 64 and
            target_code_end <= target <= target_payload_end - 64):
        return False
    direct = anchors.get(source, {})
    if len(direct) == 1 and target in direct and reverse.get(target) == {source}:
        return True
    for module, address, data, code_end, payload_end in (
            (source_module, source, source_data, source_code_end, source_payload_end),
            (target_module, target, target_data, target_code_end, target_payload_end)):
        if (module, address) not in windows:
            windows[module, address] = unique_data_window(
                data, code_end, payload_end, address)
    signature = windows[source_module, source]
    if signature is not None and signature == windows[target_module, target]:
        return True
    # A field near an object's end may have different following bytes across
    # levels while its centered, unique local context remains byte-identical.
    if (source >= source_code_end + 32 and target >= target_code_end + 32):
        for module, address, data, code_end, payload_end in (
                (source_module, source, source_data, source_code_end,
                 source_payload_end),
                (target_module, target, target_data, target_code_end,
                 target_payload_end)):
            key = module, address, "centered"
            if key not in windows:
                windows[key] = unique_data_window(
                    data, code_end, payload_end, address - 32)
        centered_source = windows[source_module, source, "centered"]
        if (centered_source is not None and
                centered_source == windows[target_module, target, "centered"]):
            return True
    if source_data[source] == target_data[target] == 0:
        for module, address, data, code_end, payload_end in (
                (source_module, source, source_data, source_code_end,
                 source_payload_end),
                (target_module, target, target_data, target_code_end,
                 target_payload_end)):
            key = module, address, "zero_run"
            if key not in windows:
                windows[key] = unique_zero_run(data, code_end, payload_end,
                                               address)
        zero_run = windows[source_module, source, "zero_run"]
        if (zero_run is not None and
                zero_run == windows[target_module, target, "zero_run"]):
            return True
        for module, address, data, code_end, payload_end in (
                (source_module, source, source_data, source_code_end,
                 source_payload_end),
                (target_module, target, target_data, target_code_end,
                 target_payload_end)):
            key = module, address, "zero_bounds"
            if key not in windows:
                windows[key] = short_zero_run(data, code_end, payload_end,
                                              address)
        source_bounds = windows[source_module, source, "zero_bounds"]
        target_bounds = windows[target_module, target, "zero_bounds"]
        if (source_bounds is not None and target_bounds is not None and
                source_bounds[1] - source_bounds[0] ==
                target_bounds[1] - target_bounds[0] and
                source - source_bounds[0] == target - target_bounds[0]):
            shift = target - source
            nearby = ordered[bisect.bisect_left(ordered, source_bounds[0]):
                             bisect.bisect_left(ordered, source_bounds[1])]
            for anchor in nearby:
                if len(anchors[anchor]) != 1:
                    continue
                other, families = next(iter(anchors[anchor].items()))
                if (len(families) >= 2 and other - anchor == shift and
                        reverse.get(other) == {anchor} and
                        target_bounds[0] <= other < target_bounds[1]):
                    return True
    # Large zero-filled objects have no unique byte window. Two independent,
    # uniquely mapped references can still fix a nearby zero target's offset.
    if source_data[source] == target_data[target] == 0:
        shift = target - source
        nearby = ordered[bisect.bisect_left(ordered, source - 1024):
                         bisect.bisect_right(ordered, source + 1024)]
        witnesses = []
        for anchor in nearby:
            if anchor == source or len(anchors[anchor]) != 1:
                continue
            other, families = next(iter(anchors[anchor].items()))
            if (other - anchor != shift or reverse.get(other) != {anchor} or
                    any(source_data[min(anchor, source):max(anchor, source) + 1]) or
                    any(target_data[min(other, target):max(other, target) + 1])):
                continue
            witnesses.append((anchor, families))
        if (len({anchor for anchor, _families in witnesses}) >= 2 and
                len(set().union(*(families for _anchor, families in witnesses))) >= 2):
            return True
    index = bisect.bisect_left(ordered, source)
    if index == 0 or index == len(ordered):
        return False
    lower, upper = ordered[index - 1], ordered[index]
    if (source - lower > MAX_ANCHOR_DISTANCE or
            upper - source > MAX_ANCHOR_DISTANCE):
        return False
    lower_values, upper_values = anchors[lower], anchors[upper]
    if len(lower_values) != 1 or len(upper_values) != 1:
        return False
    lower_target = next(iter(lower_values))
    upper_target = next(iter(upper_values))
    shift = target - source
    return (lower_target - lower == upper_target - upper == shift
            and len(lower_values[lower_target] | upper_values[upper_target]) >= 2)


def close_context_dependencies(pending: dict[tuple[str, str], set]) -> set:
    """Close paired functions only after their paired callees have closed."""
    closed = set()
    while True:
        fresh = {key for key, needs in pending.items()
                 if key not in closed and needs <= closed | {key}}
        if not fresh:
            return closed
        closed.update(fresh)


def select_noncolliding_families(exact: dict, accepted: list[dict],
                                 candidates: dict, retained: set[str]
                                 ) -> tuple[dict, int, dict]:
    """Never merge two distinct placements from the same module transitively."""
    def evaluate(order: list) -> tuple[dict, int, int, dict]:
        parent = {}
        modules = {}
        sizes = {}

        def root(node):
            while parent[node] != node:
                parent[node] = parent[parent[node]]
                node = parent[node]
            return node

        def unite(placements: list[tuple[str, int]], size: int) -> bool:
            nodes = [node for node in placements if node[0] in retained]
            if len({module for module, _start in nodes}) != len(nodes):
                return False
            for node in nodes:
                if node not in parent:
                    parent[node] = node
                    modules[node] = {node[0]}
                    sizes[node] = size
                elif sizes[node] != size:
                    raise ValueError("candidate node has conflicting body sizes")
            roots = list(dict.fromkeys(root(node) for node in nodes))
            used = set()
            for item in roots:
                if used & modules[item]:
                    return False
                used |= modules[item]
            if roots:
                primary = roots[0]
                for item in roots[1:]:
                    parent[item] = primary
                    modules[primary] |= modules[item]
            return True

        for (size, _hash), group in exact.items():
            nodes = [(row["module"], int(row["start"], 16)) for row in group]
            if len({module for module, _start in nodes if module in retained}) != sum(
                    module in retained for module, _start in nodes):
                continue  # Mirrors the exact-group repeat guard.
            if not unite(nodes, size):
                raise ValueError("exact candidate groups collide by module")
        for family in accepted:
            if not unite(family["placements"], family["body_bytes"]):
                raise ValueError("previously accepted groups collide by module")

        selected = {}
        rejected = 0
        gain = 0
        for key in order:
            family = candidates[key]
            nodes = [node for node in family["placements"] if node[0] in retained]
            before = len({root(node) for node in nodes if node in parent}) + sum(
                node not in parent for node in nodes)
            if unite(family["placements"], family["body_bytes"]):
                selected[key] = family
                gain += family["body_bytes"] * max(0, before - 1)
            else:
                rejected += 1
        components = {node: root(node) for node in parent}
        return selected, rejected, gain, components

    keys = list(candidates)
    weighted = sorted(keys, key=lambda key: (
        -candidates[key]["body_bytes"] * (len(candidates[key]["placements"]) - 1),
        key))
    choices = (evaluate(weighted), evaluate(sorted(keys)))
    selected, rejected, _gain, components = max(
        choices, key=lambda result: result[2])
    return selected, rejected, components


def prune_uncovered_calls(families: dict, exact: dict, accepted: list[dict],
                          retained: set[str], external_calls
                          ) -> tuple[dict, int, int, int]:
    """Remove caller placements whose callee lost its final equivalence link."""
    current = {key: {**family, "placements": list(family["placements"])}
               for key, family in families.items()}
    removed_pairs = removed_families = 0
    for round_number in range(1, len(current) + 2):
        _selected, rejected, components = select_noncolliding_families(
            exact, accepted, current, retained)
        if rejected:
            raise ValueError("callee pruning received colliding context families")
        next_groups = {}
        changed = False
        for key, family in current.items():
            size = family["body_bytes"]
            reference, ref_start = family["placements"][0]
            reference_calls = external_calls(reference, ref_start, size)
            kept = [(reference, ref_start)]
            for module, start in family["placements"][1:]:
                calls = external_calls(module, start, size)
                if (calls.keys() == reference_calls.keys() and all(
                        components.get((reference, ref_target)) is not None and
                        components.get((reference, ref_target)) ==
                        components.get((module, calls[site]))
                        for site, ref_target in reference_calls.items())):
                    kept.append((module, start))
                else:
                    removed_pairs += 1
                    changed = True
            if len(kept) > 1:
                next_groups[key] = {**family, "placements": kept}
            else:
                removed_families += 1
        if not changed:
            return current, removed_pairs, removed_families, round_number
        current = next_groups
    raise ValueError("context callee pruning did not converge")


def main_write(image: Path, full_ghidra_dir: Path) -> dict[str, object]:
    inventory = json.loads(INVENTORY.read_text(encoding="utf-8"))
    split = json.loads(SPLIT.read_text(encoding="utf-8"))
    shape = json.loads(SHAPE.read_text(encoding="utf-8"))
    switch = json.loads(SWITCH.read_text(encoding="utf-8"))
    check_switch_summary(switch)
    translation = json.loads(TRANSLATION.read_text(encoding="utf-8"))
    ghidra_summary = json.loads(GHIDRA.read_text(encoding="utf-8"))
    full_summary = json.loads(FULL_FLOW.read_text(encoding="utf-8"))
    target = json.loads(TARGET.read_text(encoding="utf-8"))
    if (digest(WORK / "split_boundary_review.tsv") != split["split_review_sha256"]
            or digest(WORK / "split_translation_review_leads.tsv") !=
            split["translation_review_sha256"]
            or digest(WORK / "switch_body_review.tsv") != switch["review_sha256"]):
        raise ValueError("private split or switch worklists differ from pinned audit")
    _verified_units, retained_bytes, _residual = measured_units(image)
    if retained_bytes != shape["retained_candidate_interval_bytes"]:
        raise ValueError("optimistic shape input differs from retained intervals")
    retained = {row["module"] for row in inventory["modules"]} - {
        module for group in inventory["identical_full_interval_groups"]
        for module in group[1:]}
    compared = sorted(module for module in retained if module.startswith("LEVEL_"))
    if len(compared) != 19 or REFERENCE not in compared:
        raise ValueError("expected 19 retained numbered level modules")
    module_rows = {Path(row["path"]).name: row for row in rows(MODULES)}
    code_ends = {row["module"]: int(row["end_exclusive"], 16)
                 for row in rows(CODE)}
    ghidra_hashes = {row["module"]: row["ghidra_export_sha256"]
                     for row in ghidra_summary["modules"]}
    full_hashes = {row["module"]: row["full_export_sha256"]
                   for row in full_summary["modules"]}
    split_rows = rows(WORK / "split_boundary_review.tsv")
    parents = {(row["module"], int(row["parent_start"], 16))
               for row in split_rows}
    split_by_module = defaultdict(lambda: defaultdict(list))
    for row in split_rows:
        split_by_module[row["module"]][int(row["parent_start"], 16)].append(row)
    switch_by_module = defaultdict(list)
    switch_starts = set()
    for row in rows(WORK / "switch_body_review.tsv"):
        switch_by_module[row["module"]].append(row)
        switch_starts.add((row["module"], int(row["start"], 16)))
    data, fixups, sites, contexts, known = {}, {}, {}, {}, {}
    static_rows = {}
    full_only_placements = full_only_body_bytes = 0
    with IsoDisc(image) as disc:
        verify_disc(disc, target)
        for module in compared:
            data[module] = disc.file_bytes("LVL/" + module)
            if hashlib.sha256(data[module]).hexdigest() != module_rows[module]["sha256"]:
                raise ValueError(f"{module} differs from pinned module")
            head = HEADER.unpack_from(data[module])
            fixups[module] = packed_relocations_by_site(
                data[module], head[13], head[1])
            sites[module] = [item.site for item in fixups[module]]
            ghidra_path = WORK / f"{module}.code.bin.ghidra.tsv"
            if digest(ghidra_path) != ghidra_hashes[module]:
                raise ValueError(f"{module} independent Ghidra export changed")
            ghidra = {int(row["entry"], 16): row for row in rows(ghidra_path)}
            full_path = full_ghidra_dir / f"{module}.ghidra.tsv"
            if digest(full_path) != full_hashes[module]:
                raise ValueError(f"{module} full-module Ghidra export changed")
            full_functions = {int(row["entry"], 16): row
                              for row in rows(full_path)}
            static = rows(WORK / f"{module}.tsv")
            static_rows[module] = static
            contexts[module] = defaultdict(list)
            units = context_units(static, split_by_module[module], data[module],
                                  switch_by_module[module])
            for index, row in enumerate(units):
                if not row["masked_lead_sha256"]:
                    continue
                start = int(row["start"], 16)
                boundary = (row["kind"] if row["kind"] in ("split", "switch") else
                    candidate_boundary(row, ghidra, full_functions)
                    if row["kind"] == "static" and row["status"] in (
                        "bounded_candidate", "pointer_only", "nonzero_tail")
                    else None)
                confirmed = boundary is not None
                if confirmed:
                    if boundary == "full_call":
                        full_only_placements += 1
                        full_only_body_bytes += int(row["body_bytes"])
                    key = (int(row["body_bytes"]), row["masked_lead_sha256"])
                    known[module, start] = key + (
                        (row["kind"],) if row["kind"] in ("split", "switch") else ())
                    contexts[module][context_key(units, index)].append(row)

    previous = rows(WORK / "translation_review_leads.tsv")
    if len(previous) != translation["group_count"]:
        raise ValueError("private translation leads differ from public summary")
    accepted_nodes = set()
    accepted = []
    previous_members = []
    for row in previous:
        members = member_map(row["members_and_data_deltas"], True)
        accepted_nodes.update(members.items())
        accepted.append({"body_bytes": int(row["body_bytes"]),
                         "placements": list(members.items())})
        previous_members.append((row["fingerprint"], int(row["body_bytes"]),
                                 members))
    current = rows(WORK / "split_translation_review_leads.tsv")
    for index, row in enumerate(current):
        members = member_map(row["members"], False)
        accepted_nodes.update(members.items())
        accepted.append({"body_bytes": int(row["body_bytes"]),
                         "placements": list(members.items())})
        previous_members.append((f"split-reviewed:{index}",
                                 int(row["body_bytes"]), members))

    ref_cache = {}
    semantic_hi_sites = set()
    def references(module: str, start: int, size: int):
        key = module, start, size
        if key not in ref_cache:
            local = fixups[module][bisect.bisect_left(sites[module], start):
                                   bisect.bisect_left(sites[module], start + size)]
            code = local_code_references(data[module], start, start + size,
                                         local, known, module)
            pointers, paired_sites = code_pointer_fixups(
                data[module], local, start, start + size, code_ends[module])
            if any((module, target) not in known for target in pointers.values()):
                ref_cache[key] = code, None, None
                return ref_cache[key]
            semantic = semantic_hi_pairs(data[module], local, start, start + size,
                                         code_ends[module])
            semantic_hi_sites.update((module, site) for site in semantic)
            data_fixups = [semantic.get(item.site, item) for item in local
                           if item.kind != 2 and item.site not in paired_sites]
            refs = local_references(data[module], start, start + size,
                                    code_ends[module], data_fixups,
                                    [item.site for item in data_fixups])
            ref_cache[key] = code, refs, {site - start: target
                                           for site, target in pointers.items()}
        return ref_cache[key]

    def external_calls(module: str, start: int, size: int) -> dict[int, int]:
        local = fixups[module][bisect.bisect_left(sites[module], start):
                               bisect.bisect_left(sites[module], start + size)]
        targets = {}
        for item in local:
            if item.kind != 2:
                continue
            word = int.from_bytes(data[module][item.site:item.site + 4], "little")
            target = (word & 0x03FFFFFF) << 2
            if not start <= target < start + size:
                targets[item.site - start] = target
        pointers, _paired_sites = code_pointer_fixups(
            data[module], local, start, start + size, code_ends[module])
        for site, target in pointers.items():
            if not start <= target < start + size:
                targets[site - start] = target
        return targets

    context_ids = {(module, int(member[0]["start"], 16)): context
                   for module in compared
                   for context, member in contexts[module].items()
                   if len(member) == 1}
    candidate_pairs = {}
    windows = {}
    for reference in REFERENCES:
        for module in compared:
            if module == reference:
                continue
            anchors = defaultdict(lambda: defaultdict(set))
            for fingerprint, size, members in previous_members:
                if reference not in members or module not in members:
                    continue
                source = references(reference, members[reference], size)[1]
                dest = references(module, members[module], size)[1]
                if source is None or dest is None or source[0].keys() != dest[0].keys():
                    raise ValueError("accepted data-anchor references changed")
                for site, address in source[0].items():
                    anchors[address][dest[0][site]].add(fingerprint)
            ordered = sorted(anchors)
            reverse = defaultdict(set)
            for address, targets in anchors.items():
                for other in targets:
                    reverse[other].add(address)
            for context, originals in contexts[reference].items():
                others = contexts[module].get(context, [])
                if len(originals) != 1 or len(others) != 1:
                    continue
                original, other = originals[0], others[0]
                left_start, right_start = (int(original["start"], 16),
                                           int(other["start"], 16))
                size = int(original["body_bytes"])
                if ((reference, left_start) in accepted_nodes and
                        (module, right_start) in accepted_nodes):
                    continue
                code_a, refs_a, pointers_a = references(reference, left_start, size)
                code_b, refs_b, pointers_b = references(module, right_start, size)
                if (code_a is None or code_a != code_b or refs_a is None or
                        refs_b is None or pointers_a is None or pointers_b is None or
                        pointers_a.keys() != pointers_b.keys()):
                    continue
                hi_a, low_a = refs_a
                hi_b, low_b = refs_b
                if hi_a.keys() != hi_b.keys() or low_a.keys() != low_b.keys():
                    continue
                source_end = HEADER.unpack_from(data[reference])[1]
                dest_end = HEADER.unpack_from(data[module])[1]
                if not all(anchored_target(
                        reference, module, address, hi_b[site],
                        anchors, reverse, ordered,
                        data[reference], data[module], code_ends[reference],
                        code_ends[module], source_end, dest_end, windows)
                        for site, address in hi_a.items()):
                    continue
                shifts = {hi_b[site] - address for site, address in hi_a.items()}
                if any(((low_b[site] - value) & 0xFFFF) not in
                       {shift & 0xFFFF for shift in shifts}
                       for site, value in low_a.items()):
                    continue
                candidate_pairs[reference, context, module] = (
                    size, original["masked_lead_sha256"], left_start, right_start)

    pending = {}
    invalid_pairs = 0
    for (reference, context, module), (size, _masked, left_start, right_start) in candidate_pairs.items():
        source_calls = external_calls(reference, left_start, size)
        target_calls = external_calls(module, right_start, size)
        if source_calls.keys() != target_calls.keys():
            invalid_pairs += 1
            continue
        dependencies = set()
        for site, source_target in source_calls.items():
            target = target_calls[site]
            if ((reference, source_target) in accepted_nodes and
                    (module, target) in accepted_nodes):
                continue
            source_context = context_ids.get((reference, source_target))
            target_context = context_ids.get((module, target))
            if (source_context is None or source_context != target_context or
                    (reference, source_context, module) not in candidate_pairs):
                invalid_pairs += 1
                break
            dependencies.add((reference, source_context, module))
        else:
            pending[reference, context, module] = dependencies
    # A pair cannot depend on a candidate rejected by the call-target check.
    closed = close_context_dependencies(pending)
    family_groups = {}
    for reference, context, module in sorted(closed):
        size, masked, left_start, right_start = candidate_pairs[reference, context, module]
        key = reference, context, size, masked, left_start
        family = family_groups.setdefault(key, {"body_bytes": size,
            "placements": [(reference, left_start)]})
        family["placements"].append((module, right_start))

    exact = defaultdict(list)
    for module in retained:
        static = static_rows[module] if module in static_rows else rows(
            WORK / f"{module}.tsv")
        for row in static:
            start = int(row["start"], 16)
            if row["status"] == "bounded_candidate" and (module, start) not in parents:
                exact[int(row["body_bytes"]), row["raw_sha256"]].append(
                    {"module": module, "start": row["start"]})
    for row in rows(WORK / "split_boundary_review.tsv"):
        exact[int(row["span_bytes"]), row["raw_sha256"]].append(
            {"module": row["module"], "start": row["start"]})
    old_exact, old_combined = combined_dedupe_savings(exact, accepted, retained)
    if (old_exact != split["split_aware_exact_candidate_savings"] or
            old_combined != split["combined_candidate_savings"]):
        raise ValueError("context audit does not reproduce split accounting")
    raw_context_families = len(family_groups)
    family_groups, collision_rejections, _components = select_noncolliding_families(
        exact, accepted, family_groups, retained)
    family_groups, callee_removed_pairs, callee_removed_families, prune_rounds = (
        prune_uncovered_calls(family_groups, exact, accepted, retained,
                              external_calls))
    _new_exact, new_combined = combined_dedupe_savings(
        exact, accepted + list(family_groups.values()), retained)
    review = ["# Conditional context/data structural leads only; no C match.",
              "reference\tcontext_sha256\tbody_bytes\tplacements\tmembers"]
    for (reference, context, size, _masked, _start), family in sorted(family_groups.items()):
        review.append(f"{reference}\t{context}\t{size}\t{len(family['placements'])}\t" +
                      ";".join(f"{name}:0x{start:X}"
                               for name, start in family["placements"]))
    REVIEW.write_text("\n".join(review) + "\n", encoding="utf-8", newline="\n")
    summary = {
        "status": "context_translation_leads_only",
        "reference_modules": list(REFERENCES),
        "compared_modules": len(compared),
        "context_pair_candidates": len(candidate_pairs),
        "rejected_call_target_pairs": invalid_pairs,
        "closed_context_pairs": len(closed),
        "dependency_closed_pairs": sum(bool(pending[key]) for key in closed),
        "pre_union_context_families": raw_context_families,
        "collision_rejected_families": collision_rejections,
        "callee_removed_pairs": callee_removed_pairs,
        "callee_removed_families": callee_removed_families,
        "callee_prune_rounds": prune_rounds,
        "context_families": len(family_groups),
        "switch_context_families": sum(
            any((module, start) in switch_starts
                for module, start in family["placements"])
            for family in family_groups.values()),
        "full_module_call_placements": full_only_placements,
        "full_module_call_body_bytes": full_only_body_bytes,
        "semantic_hi_sites": len(semantic_hi_sites),
        "placement_body_bytes": sum(item["body_bytes"] * len(item["placements"])
                                    for item in family_groups.values()),
        "previous_combined_candidate_savings": old_combined,
        "combined_candidate_savings": new_combined,
        "additional_candidate_savings": new_combined - old_combined,
        "conditional_candidate_bytes": retained_bytes - new_combined,
        "review_sha256": digest(REVIEW),
        "inventory_sha256": digest(INVENTORY),
        "split_sha256": digest(SPLIT),
        "shape_sha256": digest(SHAPE),
        "switch_sha256": digest(SWITCH),
        "translation_sha256": digest(TRANSLATION),
        "ghidra_sha256": digest(GHIDRA),
        "full_flow_sha256": digest(FULL_FLOW),
        "module_catalogue_sha256": digest(MODULES),
        "code_catalogue_sha256": digest(CODE),
        "overall_percent": None,
        "bytematch_ready": False,
        "warning": ("Unique neighboring code shapes and independently corroborated "
                    "data anchors support conditional pairing only. Callees must "
                    "already be accepted or close in the same placement-paired "
                    "callgraph. Neither source equivalence nor a game-wide "
                    "unique-code denominator is qualified."),
    }
    OUTPUT.write_text(json.dumps(summary, indent=2) + "\n",
                      encoding="utf-8", newline="\n")
    return summary


def check_summary(summary: dict[str, object]) -> None:
    split = json.loads(SPLIT.read_text(encoding="utf-8"))
    switch = json.loads(SWITCH.read_text(encoding="utf-8"))
    check_switch_summary(switch)
    if (summary["status"] != "context_translation_leads_only"
            or summary["reference_modules"] != list(REFERENCES)
            or summary["compared_modules"] != 19
            or not 0 <= summary["closed_context_pairs"] <=
            summary["context_pair_candidates"]
            or not 0 <= summary["dependency_closed_pairs"] <=
            summary["closed_context_pairs"]
            or not 0 <= summary["rejected_call_target_pairs"] <=
            summary["context_pair_candidates"]
            or summary["pre_union_context_families"] !=
            summary["context_families"] + summary["collision_rejected_families"] +
            summary["callee_removed_families"]
            or summary["callee_removed_pairs"] < summary["callee_removed_families"]
            or summary["callee_prune_rounds"] < 1
            or summary["context_families"] < 0
            or not 0 <= summary["switch_context_families"] <=
            summary["context_families"]
            or summary["full_module_call_placements"] < 0
            or summary["full_module_call_body_bytes"] < 0
            or not 0 <= summary["semantic_hi_sites"] <= 1000
            or summary["placement_body_bytes"] < 0
            or summary["previous_combined_candidate_savings"] !=
            split["combined_candidate_savings"]
            or summary["additional_candidate_savings"] !=
            summary["combined_candidate_savings"] -
            summary["previous_combined_candidate_savings"]
            or summary["conditional_candidate_bytes"] !=
            split["retained_candidate_interval_bytes"] -
            summary["combined_candidate_savings"]
            or not 0 <= summary["conditional_candidate_bytes"] <=
            split["conditional_candidate_bytes"]
            or len(summary["review_sha256"]) != 64
            or summary["inventory_sha256"] != digest(INVENTORY)
            or summary["split_sha256"] != digest(SPLIT)
            or summary["shape_sha256"] != digest(SHAPE)
            or summary["switch_sha256"] != digest(SWITCH)
            or summary["translation_sha256"] != digest(TRANSLATION)
            or summary["ghidra_sha256"] != digest(GHIDRA)
            or summary["full_flow_sha256"] != digest(FULL_FLOW)
            or summary["module_catalogue_sha256"] != digest(MODULES)
            or summary["code_catalogue_sha256"] != digest(CODE)
            or summary["overall_percent"] is not None
            or summary["bytematch_ready"] is not False):
        raise ValueError("context translation snapshot is stale or overclaims")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", type=Path, metavar="PINNED_ISO")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--full-ghidra-dir", type=Path,
                        default=WORK / "full-ghidra",
                        help="private pinned full-module Ghidra export directory")
    args = parser.parse_args()
    summary = main_write(args.write, args.full_ghidra_dir) if args.write else json.loads(
        OUTPUT.read_text(encoding="utf-8"))
    check_summary(summary)
    print(f"context/data structural scenario: {summary['conditional_candidate_bytes']} "
          "candidate bytes; no qualified unique-code denominator")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Audit site-paired, relocation-normalized level-code roles.

This is an aggressive structural deduplication scenario. A callee can occupy
the same call role in two overlays while having different code of its own.
Such a caller lead is neither a shared-source proof nor a C match. The pinned
image and private Ghidra exports are needed only for --write; public output is
metadata and hashes, never retail bytes.
"""

from __future__ import annotations

import argparse
import bisect
import hashlib
import json
from collections import Counter, defaultdict
from pathlib import Path

from disc import IsoDisc, verify_disc
from level_context_translation import (anchored_target, candidate_boundary,
                                       code_pointer_fixups, context_units,
                                       member_map, select_noncolliding_families,
                                       semantic_hi_pairs)
from level_function_inventory import masked_body, relocations
from level_ghidra_crosscheck import rows
from level_translation_families import (combined_dedupe_savings,
                                        local_references)
from rel import HEADER, packed_relocations_by_site

ROOT = Path(__file__).resolve().parent.parent
WORK = ROOT / "build/level-functions"
REVIEW = WORK / "role_graph_review.tsv"
OUTPUT = ROOT / "config/levels/role_graph_summary.json"
TARGET = ROOT / "config/target.json"
MODULES = ROOT / "config/levels/modules.tsv"
CODE = ROOT / "config/levels/code_intervals.tsv"
INVENTORY = ROOT / "config/levels/function_inventory_summary.json"
TRANSLATION = ROOT / "config/levels/translation_families_summary.json"
SPLIT = ROOT / "config/levels/split_dedupe_summary.json"
SWITCH = ROOT / "config/levels/switch_body_summary.json"
CONTEXT = ROOT / "config/levels/context_translation_summary.json"
GHIDRA = ROOT / "config/levels/ghidra_crosscheck_summary.json"
FULL_FLOW = ROOT / "config/levels/full_module_flow_summary.json"
REFERENCES = ("LEVEL_01.REL", "LEVEL_02.REL", "LEVEL_03.REL", "LEVEL_05.REL",
              "LEVEL_07.REL", "LEVEL_08.REL", "LEVEL_09.REL", "LEVEL_10.REL",
              "LEVEL_17.REL", "LEVEL_18.REL", "LEVEL_20.REL", "LEVEL_22.REL",
              "LEVEL_24.REL")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def data_anchor_members() -> list[tuple[str, int, dict[str, int]]]:
    """Use previously accepted families only; new role pairs cannot self-anchor."""
    result = []
    for path, field, deltas in (
            (WORK / "translation_review_leads.tsv", "members_and_data_deltas", True),
            (WORK / "split_translation_review_leads.tsv", "members", False),
            (WORK / "context_translation_review.tsv", "members", False)):
        for row in rows(path):
            family_id = row.get("fingerprint", row.get("members", ""))
            result.append((family_id, int(row["body_bytes"]),
                           member_map(row[field], deltas)))
    return result


def exact_and_accepted(retained: set[str]) -> tuple[dict, list[dict], list[dict]]:
    exact = defaultdict(list)
    split_rows = rows(WORK / "split_boundary_review.tsv")
    parents = {(row["module"], int(row["parent_start"], 16)) for row in split_rows}
    for module in retained:
        for row in rows(WORK / f"{module}.tsv"):
            start = int(row["start"], 16)
            if row["status"] == "bounded_candidate" and (module, start) not in parents:
                exact[int(row["body_bytes"]), row["raw_sha256"]].append(
                    {**row, "module": module})
    for row in split_rows:
        if row["module"] in retained:
            exact[int(row["span_bytes"]), row["raw_sha256"]].append(row)
    accepted = []
    for path, field, deltas in (
            (WORK / "translation_review_leads.tsv", "members_and_data_deltas", True),
            (WORK / "split_translation_review_leads.tsv", "members", False)):
        for row in rows(path):
            accepted.append({"body_bytes": int(row["body_bytes"]),
                             "placements": list(member_map(row[field], deltas).items())})
    context = [{"body_bytes": int(row["body_bytes"]),
                "placements": list(member_map(row["members"], False).items())}
               for row in rows(WORK / "context_translation_review.tsv")]
    return exact, accepted, context


def reviewed_catalogue(data: dict[str, bytes], code_ends: dict[str, int],
                       full_dir: Path, ghidra_hashes: dict[str, str],
                       full_hashes: dict[str, str]) -> tuple[dict, dict, dict, dict]:
    split_by_module = defaultdict(lambda: defaultdict(list))
    for row in rows(WORK / "split_boundary_review.tsv"):
        split_by_module[row["module"]][int(row["parent_start"], 16)].append(row)
    switch_by_module = defaultdict(list)
    for row in rows(WORK / "switch_body_review.tsv"):
        switch_by_module[row["module"]].append(row)
    known, static, code_ghidra, full_ghidra = {}, {}, {}, {}
    for module in sorted(data):
        code_path = WORK / f"{module}.code.bin.ghidra.tsv"
        full_path = full_dir / f"{module}.ghidra.tsv"
        if digest(code_path) != ghidra_hashes[module] or digest(full_path) != full_hashes[module]:
            raise ValueError(f"{module} independent Ghidra evidence differs")
        code_ghidra[module] = {int(row["entry"], 16): row for row in rows(code_path)}
        full_ghidra[module] = {int(row["entry"], 16): row for row in rows(full_path)}
        static[module] = rows(WORK / f"{module}.tsv")
        units = context_units(static[module], split_by_module[module], data[module],
                              switch_by_module[module])
        indexed, _pointers = relocations(data[module], code_ends[module])
        indexed_sites = sorted(indexed)
        for row in units:
            fingerprint = row["masked_lead_sha256"]
            if not fingerprint:
                continue
            kind, start = row["kind"], int(row["start"], 16)
            boundary = (kind if kind in ("split", "switch") else
                        candidate_boundary(row, code_ghidra[module],
                                           full_ghidra[module])
                        if kind == "static" and row.get("status") in
                        ("bounded_candidate", "pointer_only", "nonzero_tail") else None)
            if boundary is None:
                continue
            size = int(row["body_bytes"])
            actual, _count = masked_body(data[module][start:start + size], start,
                                         indexed, indexed_sites)
            if actual != fingerprint:
                raise ValueError(f"{module} reviewed body at 0x{start:X} differs")
            known[module, start] = size, fingerprint
    return known, static, code_ghidra, full_ghidra


def relative_ranges(row: dict[str, str], start: int) -> tuple[tuple[int, int], ...]:
    return tuple((int(left, 16) - start, int(right, 16) - start)
                 for part in row["ranges"].split(";")
                 for left, right in [part.split("-")])


def corroborated_scatter(ref: str, mod: str, a: int, b: int,
                         static_by_start: dict, code_ghidra: dict,
                         full_ghidra: dict) -> bool:
    left, right = full_ghidra[ref].get(a), full_ghidra[mod].get(b)
    code_left, code_right = code_ghidra[ref].get(a), code_ghidra[mod].get(b)
    static_left, static_right = static_by_start[ref].get(a), static_by_start[mod].get(b)
    if any(item is None for item in (left, right, code_left, code_right,
                                      static_left, static_right)):
        return False
    return (all(left.get(key) == code_left.get(key) and
                right.get(key) == code_right.get(key)
                for key in ("min", "max", "body_bytes"))
            and left["body_bytes"] == right["body_bytes"]
            and "ranges" in left and "ranges" in right
            and "call" in static_left["evidence"] and "call" in static_right["evidence"]
            and relative_ranges(left, a) == relative_ranges(right, b))


class RoleGraphAudit:
    def __init__(self, data: dict[str, bytes], code_ends: dict[str, int],
                 known: dict, static: dict, code_ghidra: dict,
                 full_ghidra: dict) -> None:
        self.data, self.code_ends, self.known = data, code_ends, known
        self.static, self.code_ghidra, self.full_ghidra = (
            static, code_ghidra, full_ghidra)
        self.static_starts = {module: [int(row["start"], 16) for row in members]
                              for module, members in static.items()}
        self.static_by_start = {module: {int(row["start"], 16): row
                                         for row in members}
                                for module, members in static.items()}
        self.head = {module: HEADER.unpack_from(payload)
                     for module, payload in data.items()}
        self.fixups = {module: packed_relocations_by_site(
            payload, self.head[module][13], self.head[module][1])
                       for module, payload in data.items()}
        self.sites = {module: [item.site for item in members]
                      for module, members in self.fixups.items()}
        self.member_cache = {}
        self.anchor_ref_cache = {}
        self.anchor_cache = {}

    def local_fixups(self, module: str, start: int, size: int) -> list:
        positions = self.sites[module]
        return self.fixups[module][bisect.bisect_left(positions, start):
                                   bisect.bisect_left(positions, start + size)]

    def anchor_refs(self, module: str, start: int, size: int):
        key = module, start, size
        if key not in self.anchor_ref_cache:
            local = [item for item in self.local_fixups(module, start, size)
                     if item.kind != 2]
            self.anchor_ref_cache[key] = local_references(
                self.data[module], start, start + size, self.code_ends[module],
                local, [item.site for item in local])
        return self.anchor_ref_cache[key]

    def anchors(self, ref: str, mod: str, accepted_members: list):
        key = ref, mod
        if key not in self.anchor_cache:
            anchors = defaultdict(lambda: defaultdict(set))
            for family_id, size, members in accepted_members:
                if ref not in members or mod not in members:
                    continue
                source = self.anchor_refs(ref, members[ref], size)
                target = self.anchor_refs(mod, members[mod], size)
                if (source is None or target is None or
                        source[0].keys() != target[0].keys()):
                    continue
                for site, address in source[0].items():
                    anchors[address][target[0][site]].add(family_id)
            reverse = defaultdict(set)
            for address, targets in anchors.items():
                for target in targets:
                    reverse[target].add(address)
            self.anchor_cache[key] = anchors, reverse, sorted(anchors), {}
        return self.anchor_cache[key]

    def member(self, module: str, start: int, size: int):
        key = module, start, size
        if key not in self.member_cache:
            local = self.local_fixups(module, start, size)
            pointers, paired = code_pointer_fixups(
                self.data[module], local, start, start + size,
                self.code_ends[module])
            semantic = semantic_hi_pairs(self.data[module], local, start,
                                         start + size, self.code_ends[module])
            data_fixups = [semantic.get(item.site, item) for item in local
                           if item.kind != 2 and item.site not in paired]
            data_refs = local_references(
                self.data[module], start, start + size, self.code_ends[module],
                data_fixups, [item.site for item in data_fixups])
            outgoing, internal = {}, {}
            valid_code = True
            for item in local:
                if item.kind != 2:
                    continue
                word = int.from_bytes(self.data[module][item.site:item.site + 4],
                                      "little")
                if word >> 26 not in (2, 3):
                    valid_code = False
                    break
                target = (word & 0x03FFFFFF) << 2
                if start <= target < start + size:
                    internal[item.site - start] = target - start
                else:
                    outgoing[item.site - start] = target
            if valid_code:
                outgoing.update({site - start: target for site, target in pointers.items()
                                 if not start <= target < start + size})
                self.member_cache[key] = (data_refs, outgoing, internal,
                                          {site - start for site in pointers})
            else:
                self.member_cache[key] = None
        return self.member_cache[key]

    def owner(self, module: str, target: int):
        starts = self.static_starts[module]
        index = bisect.bisect_right(starts, target) - 1
        if index < 0:
            return None
        row = self.static[module][index]
        if target >= int(row["limit"], 16):
            return None
        return (int(row["body_bytes"]), row["masked_lead_sha256"],
                target - starts[index])

    def pair(self, ref: str, mod: str, a: int, b: int,
             accepted_members: list):
        shape = self.known.get((ref, a))
        if shape is None or shape != self.known.get((mod, b)):
            return None
        size = shape[0]
        left, right = self.member(ref, a, size), self.member(mod, b, size)
        if left is None or right is None:
            return None
        refs_a, calls_a, internal_a, pointers_a = left
        refs_b, calls_b, internal_b, pointers_b = right
        if (refs_a is None or refs_b is None or
                refs_a[0].keys() != refs_b[0].keys() or
                refs_a[1].keys() != refs_b[1].keys() or
                calls_a.keys() != calls_b.keys() or internal_a != internal_b or
                pointers_a != pointers_b):
            return None
        anchors, reverse, ordered, windows = self.anchors(
            ref, mod, accepted_members)
        if not all(anchored_target(
                ref, mod, address, refs_b[0][site], anchors, reverse, ordered,
                self.data[ref], self.data[mod], self.code_ends[ref],
                self.code_ends[mod], self.head[ref][1], self.head[mod][1],
                windows) for site, address in refs_a[0].items()):
            return None
        shifts = {refs_b[0][site] - address for site, address in refs_a[0].items()}
        if any(((refs_b[1][site] - value) & 0xFFFF) not in
               {shift & 0xFFFF for shift in shifts}
               for site, value in refs_a[1].items()):
            return None
        dependencies, roles = [], []
        for site, source in calls_a.items():
            target = calls_b[site]
            first, second = self.known.get((ref, source)), self.known.get((mod, target))
            if (site in pointers_a and first is None and second is None and
                    self.owner(ref, source) is not None and
                    self.owner(ref, source) == self.owner(mod, target)):
                continue  # matched internal code label, not a callee family
            if first is not None and first == second:
                dependencies.append((source, target))
            elif first is not None and second is not None:
                roles.append((site, source, target, "variant"))
            elif corroborated_scatter(ref, mod, source, target,
                                      self.static_by_start, self.code_ghidra,
                                      self.full_ghidra):
                roles.append((site, source, target, "scatter"))
            else:
                dependencies.append((source, target))
        return dependencies, roles


def paired_graph(audit: RoleGraphAudit, ref: str, mod: str,
                 accepted_members: list) -> tuple[set[tuple[int, int]], dict,
                                                  dict, int, int]:
    shapes = {module: defaultdict(list) for module in (ref, mod)}
    for (module, address), shape in audit.known.items():
        if module in shapes:
            shapes[module][shape].append(address)
    seeds = sorted((members[0], shapes[mod][shape][0])
                   for shape, members in shapes[ref].items()
                   if len(members) == 1 and len(shapes[mod].get(shape, [])) == 1)
    pending = list(seeds)
    vertices = {}
    while pending:
        pair = pending.pop()
        if pair in vertices:
            continue
        result = audit.pair(ref, mod, *pair, accepted_members)
        vertices[pair] = result
        if result is not None:
            dependencies, _roles = result
            pending.extend(child for child in dependencies if child not in vertices)
    alive = {pair for pair, result in vertices.items() if result is not None}
    while True:
        fresh = {pair for pair in alive
                 if all(child in alive for child in vertices[pair][0])}
        if fresh == alive:
            break
        alive = fresh
    dependencies = {pair: vertices[pair][0] for pair in alive}
    roles = {pair: vertices[pair][1] for pair in alive}
    return alive, dependencies, roles, len(seeds), len(vertices)


def prune_role_graph(exact: dict, baseline: list[dict], candidates: dict,
                     dependencies: dict, role_edges: dict, retained: set[str]
                     ) -> tuple[dict, dict[str, int]]:
    current, collisions, _components = select_noncolliding_families(
        exact, baseline, candidates, retained)
    metrics = {"collision_rejected_pairs": collisions,
               "role_removed_pairs": 0,
               "dependency_removed_pairs": 0,
               "prune_rounds": 0}
    for round_number in range(1, len(current) + 2):
        _empty, issues, components = select_noncolliding_families(
            exact, baseline + list(current.values()), {}, retained)
        if issues:
            raise ValueError("role worklist collides after global union")

        def identity(node):
            return components.get(node, node)

        outgoing = defaultdict(lambda: defaultdict(set))
        incoming = defaultdict(lambda: defaultdict(set))
        for key in current:
            ref, mod, _a, _b = key
            for _site, source, target, _kind in role_edges.get(key, []):
                left, right = identity((ref, source)), identity((mod, target))
                outgoing[left, mod][right].add(key)
                incoming[right, ref][left].add(key)

        ambiguous = set()
        for mapping in (outgoing, incoming):
            for targets in mapping.values():
                if len(targets) <= 1:
                    continue
                ordered = sorted(targets.items(), key=lambda item: (
                    -sum(current[key]["body_bytes"] for key in item[1]),
                    -len(item[1]), str(item[0])))
                for _alternative, members in ordered[1:]:
                    ambiguous.update(members)
        unsupported = set()
        for key in current:
            for ref, mod, source, target in dependencies[key]:
                left, right = (components.get((ref, source)),
                               components.get((mod, target)))
                if left is None or left != right:
                    unsupported.add(key)
                    break
        removed = ambiguous | unsupported
        metrics["prune_rounds"] = round_number
        metrics["role_removed_pairs"] += len(ambiguous)
        metrics["dependency_removed_pairs"] += len(unsupported)
        if not removed:
            metrics["role_maps"] = len(outgoing)
            metrics["variant_role_edges"] = sum(
                kind == "variant" for key in current
                for _site, _a, _b, kind in role_edges.get(key, []))
            metrics["scatter_role_edges"] = sum(
                kind == "scatter" for key in current
                for _site, _a, _b, kind in role_edges.get(key, []))
            return current, metrics
        current = {key: family for key, family in current.items()
                   if key not in removed}
    raise ValueError("role dependency pruning did not converge")


def main_write(image: Path, full_dir: Path) -> dict[str, object]:
    target = json.loads(TARGET.read_text(encoding="utf-8"))
    inventory = json.loads(INVENTORY.read_text(encoding="utf-8"))
    context = json.loads(CONTEXT.read_text(encoding="utf-8"))
    retained = {row["module"] for row in inventory["modules"]} - {
        module for group in inventory["identical_full_interval_groups"]
        for module in group[1:]}
    module_rows = {Path(row["path"]).name: row for row in rows(MODULES)}
    code_ends = {row["module"]: int(row["end_exclusive"], 16) for row in rows(CODE)}
    ghidra_hashes = {row["module"]: row["ghidra_export_sha256"]
                     for row in json.loads(GHIDRA.read_text(encoding="utf-8"))["modules"]}
    full_hashes = {row["module"]: row["full_export_sha256"]
                   for row in json.loads(FULL_FLOW.read_text(encoding="utf-8"))["modules"]}
    if (len(module_rows) != 21 or len(retained) != 20 or
            not set(REFERENCES) <= retained or set(module_rows) != set(code_ends)):
        raise ValueError("pinned module or retained set differs")
    for path, expected in (
            (WORK / "split_boundary_review.tsv",
             json.loads(SPLIT.read_text(encoding="utf-8"))["split_review_sha256"]),
            (WORK / "switch_body_review.tsv",
             json.loads(SWITCH.read_text(encoding="utf-8"))["review_sha256"]),
            (WORK / "context_translation_review.tsv", context["review_sha256"])):
        if digest(path) != expected:
            raise ValueError(f"private review changed: {path.name}")
    data = {}
    with IsoDisc(image) as disc:
        verify_disc(disc, target)
        for module in sorted(retained):
            payload = disc.file_bytes("LVL/" + module)
            if hashlib.sha256(payload).hexdigest() != module_rows[module]["sha256"]:
                raise ValueError(f"{module} differs from pinned disc")
            data[module] = payload
    known, static, code_ghidra, full_ghidra = reviewed_catalogue(
        data, code_ends, full_dir, ghidra_hashes, full_hashes)
    audit = RoleGraphAudit(data, code_ends, known, static, code_ghidra, full_ghidra)
    exact, accepted, context_families = exact_and_accepted(retained)
    baseline = accepted + context_families
    accepted_members = data_anchor_members()
    if len(context_families) != context["context_families"]:
        raise ValueError("private context worklist differs")
    _old_exact, old_combined = combined_dedupe_savings(exact, baseline, retained)
    if old_combined != context["combined_candidate_savings"]:
        raise ValueError("strict context accounting changed")
    _selected, rejected, baseline_components = select_noncolliding_families(
        exact, accepted, {i: row for i, row in enumerate(context_families)},
        retained)
    if rejected:
        raise ValueError("strict context worklist has a collision")

    candidates, dependencies, role_edges = {}, {}, {}
    seed_pairs = reviewed_pairs = locally_closed_pairs = 0
    for ref in REFERENCES:
        before_ref = len(candidates)
        for mod in sorted(retained - {ref}):
            alive, deps, roles, seeds, reviewed = paired_graph(
                audit, ref, mod, accepted_members)
            seed_pairs += seeds
            reviewed_pairs += reviewed
            locally_closed_pairs += len(alive)
            for a, b in alive:
                left, right = (ref, a), (mod, b)
                if (baseline_components.get(left) is not None and
                        baseline_components.get(left) == baseline_components.get(right)):
                    continue
                key = ref, mod, a, b
                candidates[key] = {"body_bytes": known[left][0],
                                   "placements": [left, right]}
                dependencies[key] = [(ref, mod, x, y) for x, y in deps[a, b]]
                role_edges[key] = roles[a, b]
        print(f"{ref}: {len(candidates) - before_ref} new role candidates",
              flush=True)

    closed, metrics = prune_role_graph(
        exact, baseline, candidates, dependencies, role_edges, retained)
    _new_exact, combined = combined_dedupe_savings(
        exact, baseline + list(closed.values()), retained)
    review = ["# Conditional site-paired role leads only; no C match.",
              "reference\tmodule\tstart\tother_start\tbody_bytes\tmasked_sha256\t"
              "dependencies\tvariant_roles\tscatter_roles"]
    for ref, mod, a, b in sorted(closed):
        key = ref, mod, a, b
        kinds = Counter(kind for _site, _x, _y, kind in role_edges[key])
        review.append(f"{ref}\t{mod}\t0x{a:08X}\t0x{b:08X}\t"
                      f"{known[ref, a][0]}\t{known[ref, a][1]}\t"
                      f"{len(dependencies[key])}\t{kinds['variant']}\t"
                      f"{kinds['scatter']}")
    REVIEW.write_text("\n".join(review) + "\n", encoding="utf-8", newline="\n")
    summary = {
        "status": "role_mapped_structural_leads_only",
        "reference_modules": list(REFERENCES),
        "retained_modules": len(retained),
        "seed_pairs": seed_pairs,
        "reviewed_pairs": reviewed_pairs,
        "locally_closed_pairs": locally_closed_pairs,
        "new_candidate_pairs": len(candidates),
        "global_role_pairs": len(closed),
        **metrics,
        "previous_combined_candidate_savings": old_combined,
        "combined_candidate_savings": combined,
        "additional_candidate_savings": combined - old_combined,
        "conditional_candidate_bytes":
            json.loads(SPLIT.read_text(encoding="utf-8"))["retained_candidate_interval_bytes"]
            - combined,
        "review_sha256": digest(REVIEW),
        "target_sha256": digest(TARGET),
        "module_catalogue_sha256": digest(MODULES),
        "code_catalogue_sha256": digest(CODE),
        "inventory_sha256": digest(INVENTORY),
        "translation_sha256": digest(TRANSLATION),
        "split_sha256": digest(SPLIT),
        "switch_sha256": digest(SWITCH),
        "context_sha256": digest(CONTEXT),
        "ghidra_sha256": digest(GHIDRA),
        "full_flow_sha256": digest(FULL_FLOW),
        "overall_percent": None,
        "bytematch_ready": False,
        "warning": ("Role-mapped relocation-normalized code shapes are conditional "
                    "structural leads. Variant callees need not share source. "
                    "Neither a C match nor a qualified unique-code denominator "
                    "or game-wide percentage is established."),
    }
    OUTPUT.write_text(json.dumps(summary, indent=2) + "\n",
                      encoding="utf-8", newline="\n")
    return summary


def check_summary(summary: dict[str, object]) -> None:
    context = json.loads(CONTEXT.read_text(encoding="utf-8"))
    split = json.loads(SPLIT.read_text(encoding="utf-8"))
    if (summary["status"] != "role_mapped_structural_leads_only"
            or summary["reference_modules"] != list(REFERENCES)
            or summary["retained_modules"] != 20
            or not 0 <= summary["global_role_pairs"] <=
            summary["new_candidate_pairs"] <= summary["locally_closed_pairs"] <=
            summary["reviewed_pairs"]
            or not 0 <= summary["seed_pairs"] <= summary["reviewed_pairs"]
            or summary["collision_rejected_pairs"] < 0
            or summary["role_removed_pairs"] < 0
            or summary["dependency_removed_pairs"] < 0
            or summary["prune_rounds"] < 1
            or summary["role_maps"] < 0
            or summary["variant_role_edges"] < 0
            or summary["scatter_role_edges"] < 0
            or summary["previous_combined_candidate_savings"] !=
            context["combined_candidate_savings"]
            or summary["additional_candidate_savings"] !=
            summary["combined_candidate_savings"] -
            summary["previous_combined_candidate_savings"]
            or summary["additional_candidate_savings"] < 0
            or summary["conditional_candidate_bytes"] !=
            split["retained_candidate_interval_bytes"] -
            summary["combined_candidate_savings"]
            or not 0 < summary["conditional_candidate_bytes"] <=
            context["conditional_candidate_bytes"]
            or len(summary["review_sha256"]) != 64
            or summary["target_sha256"] != digest(TARGET)
            or summary["module_catalogue_sha256"] != digest(MODULES)
            or summary["code_catalogue_sha256"] != digest(CODE)
            or summary["inventory_sha256"] != digest(INVENTORY)
            or summary["translation_sha256"] != digest(TRANSLATION)
            or summary["split_sha256"] != digest(SPLIT)
            or summary["switch_sha256"] != digest(SWITCH)
            or summary["context_sha256"] != digest(CONTEXT)
            or summary["ghidra_sha256"] != digest(GHIDRA)
            or summary["full_flow_sha256"] != digest(FULL_FLOW)
            or summary["overall_percent"] is not None
            or summary["bytematch_ready"] is not False):
        raise ValueError("role graph summary is stale or overclaims")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", type=Path, metavar="PINNED_ISO")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--full-ghidra-dir", type=Path,
                        default=WORK / "full-ghidra")
    args = parser.parse_args()
    summary = (main_write(args.write, args.full_ghidra_dir) if args.write else
               json.loads(OUTPUT.read_text(encoding="utf-8")))
    check_summary(summary)
    print("role-mapped structural scenario: "
          f"{summary['conditional_candidate_bytes']} candidate bytes; "
          "no qualified unique-code denominator")


if __name__ == "__main__":
    main()

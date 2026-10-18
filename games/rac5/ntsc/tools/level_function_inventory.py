#!/usr/bin/env python3
"""Audit SNR2 function-entry candidates and relocation-aware duplicate leads.

Direct calls, exported entries and relocated code pointers are evidence for
possible starts, not a proof that every interval is a function. Detailed
metadata-only rows stay under ignored build/; the tracked summary never
claims a game-wide progress denominator.
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
from rel import HEADER, RELOCATION, SYMBOL, iter_packed_relocations, unpack_relocation

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "build/level-functions"
SUMMARY = ROOT / "config/levels/function_inventory_summary.json"
CODE_CATALOGUE = ROOT / "config/levels/code_intervals.tsv"
MODULE_CATALOGUE = ROOT / "config/levels/modules.tsv"
EXPORT_CATALOGUE = ROOT / "config/levels/exports.tsv"
TARGET = ROOT / "config/target.json"
WORD = struct.Struct("<I")
CODE_START = 0x80
JR_RA = 0x03E00008


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def records(path: Path) -> list[dict[str, str]]:
    with path.open(encoding="utf-8", newline="") as file:
        return list(csv.DictReader((line for line in file if not line.startswith("#")),
                                   delimiter="\t"))


def relocations(data: bytes, code_end: int) -> tuple[dict[int, tuple], set[int]]:
    head = HEADER.unpack_from(data)
    packed = list(iter_packed_relocations(data, head[13], head[1]))
    indexed = {row.site: ("local", row.kind, "", 0) for row in packed}
    pointers = {WORD.unpack_from(data, row.site)[0] for row in packed if row.kind == 3}
    pointers = {target for target in pointers if CODE_START <= target < code_end
                and target % 4 == 0}
    symbols = []
    for index in range(head[4]):
        name_at = SYMBOL.unpack_from(data, head[3] + index * SYMBOL.size)[0]
        end = data.index(b"\0", name_at)
        symbols.append(data[name_at:end].decode("ascii", "strict"))
    for index in range(head[2]):
        site, kind, symbol, addend = unpack_relocation(
            data, head[1] + index * RELOCATION.size)
        if site in indexed:
            raise ValueError("local and named relocations overlap")
        indexed[site] = ("named", kind, symbols[symbol], addend)
    return indexed, pointers


def masked_body(body: bytes, start: int, relocs: dict[int, tuple],
                sites: list[int]) -> tuple[str, int]:
    """Hash code with relocation fields removed; this is a *lead*, not identity."""
    masked = bytearray(body)
    descriptors = []
    left = bisect.bisect_left(sites, start)
    right = bisect.bisect_left(sites, start + len(body))
    for site in sites[left:right]:
        source, kind, symbol, addend = relocs[site]
        offset = site - start
        word = WORD.unpack_from(masked, offset)[0]
        if (source == "local" and kind == 2) or (source == "named" and kind == 4):
            mask = 0xFC000000  # J/JAL target field
        elif (source == "local" and kind == 3) or (source == "named" and kind == 2):
            mask = 0  # full 32-bit pointer
        else:
            mask = 0xFFFF0000  # HI16/LO16 immediate
        WORD.pack_into(masked, offset, word & mask)
        descriptors.append((offset, source, kind, symbol, addend))
    fingerprint = bytes(masked) + b"\0" + json.dumps(descriptors, separators=(",", ":")).encode()
    return digest(fingerprint), len(descriptors)


def control_flow(data: bytes, code_end: int) -> tuple[set[int], set[int], list[tuple[int, int]]]:
    import rabbitizer
    if rabbitizer.__version__ != "1.16.2":
        raise ValueError("function inventory requires rabbitizer 1.16.2")
    calls: set[int] = set()
    returns: set[int] = set()
    edges: list[tuple[int, int]] = []
    for site in range(CODE_START, code_end, 4):
        word = WORD.unpack_from(data, site)[0]
        if word == JR_RA:
            returns.add(site)
        insn = rabbitizer.Instruction(word, site, rabbitizer.InstrCategory.R5900)
        if not insn.isValid():
            raise ValueError(f"invalid R5900 candidate word at 0x{site:X}")
        if insn.isBranch():
            target = insn.getBranchVramGeneric()
            if CODE_START <= target < code_end:
                edges.append((site, target))
        if insn.isJumpWithAddress():
            target = insn.getInstrIndexAsVram()
            if CODE_START <= target < code_end:
                edges.append((site, target))
                if insn.isFunctionCall():
                    calls.add(target)
    return calls, returns, edges


def module_rows(module: str, data: bytes, code_end: int, exports: set[int]) -> list[dict]:
    relocs, pointers = relocations(data, code_end)
    calls, returns, edges = control_flow(data, code_end)
    starts = sorted({CODE_START, *calls, *pointers, *exports})
    sites = sorted(relocs)
    ordered_returns = sorted(returns)
    rows = []
    for start, limit in zip(starts, starts[1:] + [code_end]):
        evidence = "+".join(label for label, pool in
                            (("call", calls), ("pointer", pointers), ("export", exports))
                            if start in pool) or "interval_start"
        idx = bisect.bisect_right(ordered_returns, limit - 8) - 1
        body_end = ordered_returns[idx] + 8 if idx >= 0 and ordered_returns[idx] >= start else None
        if body_end is None:
            status = "no_return"
        elif any(data[body_end:limit]):
            status = "nonzero_tail"
        elif start not in calls and start not in exports:
            status = "pointer_only"
        else:
            status = "bounded_candidate"
        body = data[start:body_end] if body_end is not None else b""
        normalized, reloc_count = masked_body(body, start, relocs, sites) if body else ("", 0)
        rows.append({"module": module, "start": start, "limit": limit,
                     "body_end": body_end, "evidence": evidence, "status": status,
                     "body_bytes": len(body), "zero_tail_bytes": limit - body_end
                     if body_end is not None and not any(data[body_end:limit]) else 0,
                     "raw_sha256": digest(body) if body else "",
                     "masked_lead_sha256": normalized,
                     "body_relocations": reloc_count})
    # A direct edge into the interior from another candidate interval may
    # mean this is an internal basic block rather than a distinct function.
    for source, target in edges:
        idx = bisect.bisect_right(starts, target) - 1
        if idx < 0:
            continue
        row = rows[idx]
        if (row["status"] == "bounded_candidate" and row["start"] < target
                and row["body_end"] is not None and target < row["body_end"]
                and not row["start"] <= source < row["limit"]):
            row["status"] = "external_interior_target"
    return rows


def render_rows(rows: list[dict]) -> str:
    lines = ["# Candidate boundaries only; no row is an accepted retail function size.",
             "start\tlimit\tbody_end\tevidence\tstatus\tbody_bytes\tzero_tail_bytes\t"
             "raw_sha256\tmasked_lead_sha256\tbody_relocations"]
    for row in rows:
        lines.append("\t".join((f"0x{row['start']:08X}", f"0x{row['limit']:08X}",
                                f"0x{row['body_end']:08X}" if row["body_end"] is not None else "",
                                row["evidence"], row["status"], str(row["body_bytes"]),
                                str(row["zero_tail_bytes"]), row["raw_sha256"],
                                row["masked_lead_sha256"], str(row["body_relocations"]))))
    return "\n".join(lines) + "\n"


def aggregate(all_rows: list[dict], module_sizes: dict[str, int]) -> dict:
    statuses = Counter(row["status"] for row in all_rows)
    trusted = [row for row in all_rows if row["status"] == "bounded_candidate"]
    raw: dict[tuple[int, str], list[dict]] = defaultdict(list)
    masked: dict[tuple[int, str], list[dict]] = defaultdict(list)
    for row in trusted:
        raw[(row["body_bytes"], row["raw_sha256"])].append(row)
        masked[(row["body_bytes"], row["masked_lead_sha256"])].append(row)
    exact_distinct = sum(size for size, _hash in raw)
    masked_lead = sum(size for size, _hash in masked)
    # Groups with two occurrences in one module may be different functions
    # with identical shapes. Do not collapse those in the conservative lead.
    masked_conservative = sum(
        size if len({row["module"] for row in group}) == len(group)
        else size * len(group)
        for (size, _hash), group in masked.items())
    return {
        "status": "candidate_inventory_only",
        "module_count": len(module_sizes),
        "candidate_code_bytes_by_placement": sum(module_sizes.values()),
        "entry_intervals": len(all_rows),
        "boundary_status_counts": dict(sorted(statuses.items())),
        "bounded_candidate_body_bytes_by_placement": sum(row["body_bytes"] for row in trusted),
        "raw_exact_distinct_candidate_bytes": exact_distinct,
        "raw_exact_multi_placement_groups": sum(len(group) > 1 for group in raw.values()),
        "relocation_masked_candidate_bytes": masked_lead,
        "relocation_masked_conservative_lead_bytes": masked_conservative,
        "masked_multi_placement_leads": sum(len(group) > 1 for group in masked.values()),
        "masked_leads_with_repeated_module": sum(
            len({row["module"] for row in group}) != len(group)
            for group in masked.values()),
        "mask_warning": "Masked relocation targets may refer to different code or data; these are leads, not proved shared functions or a progress denominator.",
        "overall_percent": None,
        "bytematch_ready": False,
        "modules": [
            {"module": module, "candidate_code_bytes": module_sizes[module],
             "candidate_intervals": sum(row["module"] == module for row in all_rows),
             "bounded_candidates": sum(row["module"] == module and
                                       row["status"] == "bounded_candidate" for row in all_rows)}
            for module in module_sizes
        ],
        "catalogue_sha256": {path.name: digest(path.read_bytes()) for path in
                             (TARGET, CODE_CATALOGUE, MODULE_CATALOGUE, EXPORT_CATALOGUE)},
    }


def render_leads(rows: list[dict], kind: str) -> str:
    field = "raw_sha256" if kind == "exact" else "masked_lead_sha256"
    groups: dict[tuple[int, str], list[dict]] = defaultdict(list)
    for row in rows:
        if row["status"] == "bounded_candidate":
            groups[(row["body_bytes"], row[field])].append(row)
    lines = ["# Identical code" if kind == "exact" else
             "# Relocation-masked leads only; callee/data semantics are unreviewed.",
             "body_bytes\tplacements\tmodules\trepeated_module\tsha256\tmembers"]
    ranked = sorted(((size, key, members) for (size, key), members in groups.items()
                     if len(members) > 1),
                    key=lambda item: (-(item[0] * (len(item[2]) - 1)), item[1]))
    for size, key, members in ranked:
        distinct = {row["module"] for row in members}
        locations = ";".join(f"{row['module']}:0x{row['start']:X}" for row in members)
        lines.append(f"{size}\t{len(members)}\t{len(distinct)}\t"
                     f"{int(len(distinct) != len(members))}\t{key}\t{locations}")
    return "\n".join(lines) + "\n"


def exact_dedupe_bound(rows: list[dict], module_sizes: dict[str, int],
                       code_hashes: dict[str, str]) -> tuple[int, list[list[str]]]:
    """Deduplicate only identical whole intervals or non-overlapping raw bodies.

    Residual/uncertain bytes stay counted. This is an upper bound on distinct
    bytes in the candidate intervals, not a qualified game-code denominator.
    """
    groups: dict[tuple[int, str], list[str]] = defaultdict(list)
    for module in sorted(module_sizes):
        groups[(module_sizes[module], code_hashes[module])].append(module)
    kept = {names[0] for names in groups.values()}
    identical_intervals = [names for names in groups.values() if len(names) > 1]
    bodies: dict[tuple[int, str], list[dict]] = defaultdict(list)
    for row in rows:
        if row["module"] in kept and row["status"] == "bounded_candidate":
            bodies[(row["body_bytes"], row["raw_sha256"])].append(row)
    proved_savings = sum(
        size * (len(group) - 1)
        for (size, _hash), group in bodies.items()
        if len({row["module"] for row in group}) == len(group))
    return sum(module_sizes[module] for module in kept) - proved_savings, identical_intervals


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", type=Path, metavar="PINNED_ISO")
    mode.add_argument("--check", action="store_true")
    args = parser.parse_args()
    if args.check:
        summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
        for path in (TARGET, CODE_CATALOGUE, MODULE_CATALOGUE, EXPORT_CATALOGUE):
            if summary["catalogue_sha256"].get(path.name) != digest(path.read_bytes()):
                raise ValueError(f"function inventory is stale: {path.name}")
        known = {row["module"]: int(row["interval_bytes"])
                 for row in records(CODE_CATALOGUE)}
        hashes = {row["module"]: row["sha256"] for row in records(CODE_CATALOGUE)}
        full_groups: dict[tuple[int, str], list[str]] = defaultdict(list)
        for module in sorted(known):
            full_groups[(known[module], hashes[module])].append(module)
        modules = summary["modules"]
        statuses = summary["boundary_status_counts"]
        if (summary["status"] != "candidate_inventory_only"
                or summary["overall_percent"] is not None
                or summary["bytematch_ready"] is not False
                or summary["module_count"] != len(known) or len(known) != 21
                or {row["module"]: row["candidate_code_bytes"] for row in modules} != known
                or sum(row["candidate_intervals"] for row in modules) != summary["entry_intervals"]
                or sum(row["bounded_candidates"] for row in modules) != statuses["bounded_candidate"]
                or sum(statuses.values()) != summary["entry_intervals"]
                or summary["bounded_candidate_body_bytes_by_placement"] >
                summary["candidate_code_bytes_by_placement"]
                or summary["identical_full_interval_groups"] !=
                [names for names in full_groups.values() if len(names) > 1]
                or not 0 < summary["exact_byte_dedupe_upper_bound"] <=
                summary["candidate_code_bytes_by_placement"]):
            raise ValueError("candidate inventory cannot publish a percentage")
        print(f"candidate inventory metadata current: {summary['module_count']} modules")
        return
    target = json.loads(TARGET.read_text(encoding="utf-8"))
    intervals = {row["module"]: row for row in records(CODE_CATALOGUE)}
    modules = {Path(row["path"]).name: row for row in records(MODULE_CATALOGUE)}
    exports: dict[str, set[int]] = defaultdict(set)
    for row in records(EXPORT_CATALOGUE):
        module = Path(row["module"]).name
        offset = int(row["offset"], 16)
        if row["symbol"] == "lib_main__FiPPc":
            exports[module].add(offset)
    if len(intervals) != 21 or set(intervals) != set(modules):
        raise ValueError("pinned level catalogues do not contain 21 placements")
    all_rows = []
    module_sizes = {}
    outputs = {}
    private_code = {}
    with IsoDisc(args.write) as disc:
        verify_disc(disc, target)
        for module in sorted(modules):
            data = disc.file_bytes("LVL/" + module)
            code_end = int(intervals[module]["end_exclusive"], 16)
            if (digest(data) != modules[module]["sha256"]
                    or digest(data[CODE_START:code_end]) != intervals[module]["sha256"]):
                raise ValueError(f"{module} differs from pinned SNR2 code")
            rows = module_rows(module, data, code_end, exports[module])
            outputs[module] = render_rows(rows)
            private_code[module] = data[CODE_START:code_end]
            all_rows.extend(rows)
            module_sizes[module] = code_end - CODE_START
    summary = aggregate(all_rows, module_sizes)
    bound, identical_intervals = exact_dedupe_bound(
        all_rows, module_sizes,
        {module: intervals[module]["sha256"] for module in module_sizes})
    summary["exact_byte_dedupe_upper_bound"] = bound
    summary["identical_full_interval_groups"] = identical_intervals
    OUTPUT.mkdir(parents=True, exist_ok=True)
    for module, content in outputs.items():
        (OUTPUT / f"{module}.tsv").write_text(content, encoding="utf-8", newline="\n")
        (OUTPUT / f"{module}.code.bin").write_bytes(private_code[module])
    (OUTPUT / "exact_duplicate_groups.tsv").write_text(
        render_leads(all_rows, "exact"), encoding="utf-8", newline="\n")
    (OUTPUT / "masked_review_leads.tsv").write_text(
        render_leads(all_rows, "masked"), encoding="utf-8", newline="\n")
    SUMMARY.write_text(json.dumps(summary, indent=2) + "\n",
                       encoding="utf-8", newline="\n")
    print(f"{len(all_rows)} candidate entry intervals across {len(module_sizes)} modules; "
          f"{summary['bounded_candidate_body_bytes_by_placement']} bounded body "
          "bytes by placement; no game-wide percentage")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Triage boot C witnesses without treating a successful link as acceptance.

The public catalogue can identify mechanically simple candidates. A private
complete-boot gate receipt, bound to the exact current inputs, is required to
call their compiled bytes verified. Semantic and boundary review still decide
whether a candidate may be promoted from nonmatching/ to src/.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
from collections import Counter
from pathlib import Path

from boot_c_campaign import (ACCEPTED_SOURCES, REPLACEMENTS, ROOT, source_path,
                             validate_replacements)
from c_source_policy import require_c_source

CATALOGUE = ROOT / "config/boot/witness_boundaries.tsv"
PROFILE = ROOT / "config/boot/compiler_trial.json"
CFLAGS = ROOT / "config/cflags.txt"
GATE = ROOT / "build/boot-trial-gate/report.json"
OUTPUT = ROOT / "build/boot-promotion-audit/report.json"


def _sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def input_fingerprint() -> str:
    """Bind a gate receipt to source bytes, owners, profile, per-source flags and catalogue."""
    inputs = {
        "profile_sha256": _sha(PROFILE.read_bytes()),
        "cflags_sha256": _sha(CFLAGS.read_bytes()),
        "catalogue_sha256": _sha(CATALOGUE.read_bytes()),
        "owners": [
            {"name": item.name, "start": item.start, "end": item.end,
             "source_pin": item.source_sha256, "object_pin": item.object_sha256,
             "source_sha256": _sha(source_path(item.name).read_bytes()),
             "source_location": source_path(item.name).relative_to(ROOT).as_posix()}
            for item in REPLACEMENTS
        ],
    }
    return _sha(json.dumps(inputs, sort_keys=True, separators=(",", ":")).encode())


def catalogue_rows() -> list[dict]:
    with CATALOGUE.open(encoding="utf-8", newline="") as file:
        raw = csv.DictReader((line for line in file if not line.startswith("#")),
                             delimiter="\t")
        rows = []
        for item in raw:
            row = {"symbol": item["symbol"],
                   "start": int(item["start"], 16),
                   "body_end": int(item["body_end"], 16),
                   "next_label": int(item["next_label"], 16),
                   "body_bytes": int(item["body_bytes"]),
                   "zero_padding_bytes": int(item["zero_padding_bytes"]),
                   "exit": item["exit"]}
            if (not row["symbol"] or row["body_bytes"] < 8
                    or row["body_bytes"] % 4
                    or row["start"] + row["body_bytes"] != row["body_end"]
                    or row["body_end"] + row["zero_padding_bytes"] != row["next_label"]
                    or row["zero_padding_bytes"] < 0):
                raise ValueError("invalid generated boot body row")
            rows.append(row)
    if (len({row["symbol"] for row in rows}) != len(rows)
            or len({row["start"] for row in rows}) != len(rows)):
        raise ValueError("duplicate generated boot body")
    return rows


def verified_linker_alignment(row: dict, owner_end: int) -> bool:
    """Permit only the zero word inserted by the campaign's ALIGN(8).

    The generated catalogue separately verifies retail zero bytes and rejects
    direct control-flow targets in this gap. This predicate never counts the
    gap as compiled C and does not replace semantic boundary review.
    """
    padding = row["zero_padding_bytes"]
    return (padding in (0, 4)
            and row["body_end"] + padding == owner_end
            and row["next_label"] == owner_end
            and padding == (-row["body_end"]) % 8)


def gate_state(gate: dict | None, fingerprint: str,
               candidate_count: int, body_count: int,
               accepted: list[dict]) -> str:
    if gate is None:
        return "missing"
    if (gate.get("status") != "passed_boot_only"
            or gate.get("raw_load_and_header_exact") is not True
            or gate.get("whole_elf_exact") is not True):
        return "failed"
    if (gate.get("promotion_input_sha256") != fingerprint
            or gate.get("candidate_objects") != candidate_count
            or gate.get("candidate_functions") != body_count
            or gate.get("accepted_functions") !=
            [row["symbol"] for row in accepted]
            or gate.get("counted_matches") != len(accepted)
            or gate.get("counted_bytes") !=
            sum(row["body_bytes"] for row in accepted)):
        return "stale"
    return "current_exact"


def audit(gate: dict | None = None) -> dict:
    validate_replacements()
    rows = catalogue_rows()
    fingerprint = input_fingerprint()
    accepted = [row for row in rows for owner in REPLACEMENTS
                if owner.name in ACCEPTED_SOURCES
                and owner.start <= row["start"] < owner.end]
    state = gate_state(gate, fingerprint, len(REPLACEMENTS), len(rows), accepted)
    results = []
    used: set[str] = set()
    for owner in REPLACEMENTS:
        span = sorted((row for row in rows
                       if owner.start <= row["start"] < owner.end),
                      key=lambda row: row["start"])
        used.update(row["symbol"] for row in span)
        reasons = []
        source = source_path(owner.name)
        raw = source.read_text(encoding="utf-8").replace("\r\n", "\n")
        try:
            require_c_source(raw, owner.name)
        except ValueError:
            reasons.append("source_policy_failed")
        if _sha(raw.encode()) != owner.source_sha256:
            reasons.append("source_pin_mismatch")
        if not span:
            reasons.append("no_catalogued_body")
        if len(span) > 1:
            reasons.append("multiple_bodies_in_one_owner")
        if span and span[0]["start"] != owner.start:
            reasons.append("body_or_padding_outside_owner")
        for index, row in enumerate(span):
            expected_next = (span[index + 1]["start"]
                             if index + 1 < len(span) else owner.end)
            if (row["body_end"] > owner.end
                    or row["next_label"] != expected_next):
                reasons.append("body_or_padding_outside_owner")
            if not verified_linker_alignment(row, owner.end):
                reasons.append("padding_needs_boundary_review")
            if row["exit"] != "jr_ra":
                reasons.append("exit_needs_flow_review")
        if state != "current_exact":
            reasons.append(f"complete_gate_{state}")
        if any(reason in reasons for reason in
               ("source_policy_failed", "source_pin_mismatch",
                "no_catalogued_body", "body_or_padding_outside_owner")):
            status = "blocked"
        elif owner.name in ACCEPTED_SOURCES:
            status = "accepted" if state == "current_exact" else "accepted_needs_gate"
        elif state == "current_exact" and not reasons:
            status = "ready_for_semantic_review"
        else:
            status = "needs_review_or_gate"
        results.append({"owner": owner.name,
                        "source": source.relative_to(ROOT).as_posix(),
                        "start": f"0x{owner.start:08X}",
                        "end": f"0x{owner.end:08X}",
                        "body_bytes": sum(row["body_bytes"] for row in span),
                        "symbols": [row["symbol"] for row in span],
                        "status": status, "reasons": sorted(set(reasons))})
    if len(used) != len(rows):
        raise ValueError("catalogued boot body has no C owner")
    counts = Counter(row["status"] for row in results)
    return {"format": "rac5-boot-promotion-audit-v1",
            "gate": state, "input_sha256": fingerprint,
            "candidate_owners": len(REPLACEMENTS), "catalogued_bodies": len(rows),
            "status_counts": dict(sorted(counts.items())),
            "candidates": results,
            "note": "ready_for_semantic_review is not acceptance; verify retail bounds, behavior, symbol and type evidence, then move source to src/ and rerun the complete gate"}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gate", type=Path, default=GATE,
                        help="private complete-boot gate receipt")
    parser.add_argument("--write", action="store_true",
                        help="write metadata-only report under ignored build/")
    parser.add_argument("--check", action="store_true",
                        help="fail on broken public source or ownership evidence")
    parser.add_argument("--require-gate", action="store_true",
                        help="require a current exact private boot gate receipt")
    parser.add_argument("--candidate", help="show one campaign source owner")
    args = parser.parse_args()
    gate = json.loads(args.gate.read_text(encoding="utf-8")) if args.gate.is_file() else None
    report = audit(gate)
    if args.require_gate and report["gate"] != "current_exact":
        raise SystemExit(f"complete boot gate is {report['gate']}; rerun it first")
    if args.check and any(set(item["reasons"]) & {
            "source_policy_failed", "source_pin_mismatch",
            "body_or_padding_outside_owner"} for item in report["candidates"]):
        raise SystemExit("boot promotion public evidence is inconsistent")
    if args.write:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if args.candidate:
        rows = [row for row in report["candidates"]
                if row["owner"] == args.candidate]
        if len(rows) != 1:
            raise SystemExit(f"unknown or ambiguous campaign owner: {args.candidate}")
        print(json.dumps({"gate": report["gate"], **rows[0]}, indent=2))
    else:
        print(f"boot promotion audit: {report['gate']}; "
              f"{report['catalogued_bodies']} bodies / {report['candidate_owners']} owners; "
              f"{report['status_counts']}")


if __name__ == "__main__":
    main()

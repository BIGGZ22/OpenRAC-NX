"""A linked C body is a review candidate, never automatic matching credit."""

from __future__ import annotations

import sys
import unittest
from dataclasses import replace
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import boot_promotion_audit as promotion  # noqa: E402


class BootPromotionAuditTests(unittest.TestCase):
    def test_only_verified_single_word_linker_gap_is_eligible(self) -> None:
        row = {"body_end": 0x100C, "next_label": 0x1010,
               "zero_padding_bytes": 4}
        self.assertTrue(promotion.verified_linker_alignment(row, 0x1010))
        self.assertFalse(promotion.verified_linker_alignment(
            {**row, "zero_padding_bytes": 8}, 0x1014))
        self.assertFalse(promotion.verified_linker_alignment(
            {**row, "next_label": 0x1014}, 0x1010))

    def synthetic_current_gate(self) -> dict:
        rows = promotion.catalogue_rows()
        accepted = [row for row in rows for owner in promotion.REPLACEMENTS
                    if owner.name in promotion.ACCEPTED_SOURCES
                    and owner.start <= row["start"] < owner.end]
        return {"status": "passed_boot_only",
                "raw_load_and_header_exact": True,
                "whole_elf_exact": True,
                "promotion_input_sha256": promotion.input_fingerprint(),
                "candidate_objects": len(promotion.REPLACEMENTS),
                "candidate_functions": len(rows),
                "accepted_functions": [row["symbol"] for row in accepted],
                "counted_matches": len(accepted),
                "counted_bytes": sum(row["body_bytes"] for row in accepted)}

    def test_without_private_gate_no_new_body_is_promoted(self) -> None:
        report = promotion.audit()
        self.assertEqual(report["gate"], "missing")
        self.assertEqual(report["catalogued_bodies"], len(promotion.catalogue_rows()))
        self.assertFalse(any(row["status"] == "ready_for_semantic_review"
                             for row in report["candidates"]))
        accepted = [row for row in report["candidates"]
                    if row["owner"] == "memcard_busy"]
        self.assertEqual(accepted[0]["status"], "accepted_needs_gate")

    def test_receipt_must_match_current_sources_and_catalogue(self) -> None:
        gate = self.synthetic_current_gate()
        self.assertEqual(promotion.audit(gate)["gate"], "current_exact")
        gate["promotion_input_sha256"] = "0" * 64
        report = promotion.audit(gate)
        self.assertEqual(report["gate"], "stale")
        self.assertFalse(any(row["status"] == "ready_for_semantic_review"
                             for row in report["candidates"]))

    def test_compiled_c_with_wrong_source_pin_is_blocked(self) -> None:
        changed = (replace(promotion.REPLACEMENTS[0], source_sha256="0" * 64),
                   *promotion.REPLACEMENTS[1:])
        with patch.object(promotion, "REPLACEMENTS", changed):
            report = promotion.audit(self.synthetic_current_gate())
        self.assertEqual(report["candidates"][0]["status"], "blocked")
        self.assertIn("source_pin_mismatch", report["candidates"][0]["reasons"])

    def test_catalogue_cannot_leave_assembly_prefix_in_owner(self) -> None:
        rows = promotion.catalogue_rows()
        changed = [dict(row) for row in rows]
        changed[0]["start"] += 4
        changed[0]["body_bytes"] -= 4
        with patch.object(promotion, "catalogue_rows", return_value=changed):
            report = promotion.audit(self.synthetic_current_gate())
        self.assertEqual(report["candidates"][0]["status"], "blocked")
        self.assertIn("body_or_padding_outside_owner",
                      report["candidates"][0]["reasons"])


if __name__ == "__main__":
    unittest.main()

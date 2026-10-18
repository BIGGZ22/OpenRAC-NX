"""Relocation masking gives review leads, never accepted function matches."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_function_inventory import (aggregate, exact_dedupe_bound,  # noqa: E402
                                      masked_body, render_leads)


class LevelFunctionInventoryTests(unittest.TestCase):
    def test_mask_respects_relocation_kind_and_named_symbol(self) -> None:
        first = bytes.fromhex("3412023c0800e003")
        second = bytes.fromhex("7856023c0800e003")
        local = {0x100: ("local", 1, "", 0)}
        self.assertEqual(masked_body(first, 0x100, local, [0x100])[0],
                         masked_body(second, 0x100, local, [0x100])[0])
        named_a = {0x100: ("named", 5, "some_import", 0)}
        named_b = {0x100: ("named", 5, "other_import", 0)}
        self.assertNotEqual(masked_body(first, 0x100, named_a, [0x100])[0],
                            masked_body(first, 0x100, named_b, [0x100])[0])

    def test_same_module_repeats_are_not_collapsed_in_conservative_lead(self) -> None:
        rows = [{"module": module, "status": "bounded_candidate",
                 "body_bytes": 8, "raw_sha256": "a", "masked_lead_sha256": "b",
                 "start": start}
                for module, start in (("LEVEL_01.REL", 0x100),
                                      ("LEVEL_01.REL", 0x200),
                                      ("LEVEL_02.REL", 0x100))]
        report = aggregate(rows, {"LEVEL_01.REL": 100, "LEVEL_02.REL": 100})
        self.assertEqual(report["relocation_masked_candidate_bytes"], 8)
        self.assertEqual(report["relocation_masked_conservative_lead_bytes"], 24)
        self.assertIsNone(report["overall_percent"])
        self.assertFalse(report["bytematch_ready"])
        self.assertIn("\t1\t", render_leads(rows, "masked"))

    def test_exact_bound_keeps_ambiguous_bytes_and_drops_full_duplicate(self) -> None:
        rows = [{"module": module, "status": "bounded_candidate",
                 "body_bytes": 8, "raw_sha256": "same"}
                for module in ("A", "B", "C")]
        bound, duplicates = exact_dedupe_bound(
            rows, {"A": 100, "B": 100, "C": 100},
            {"A": "whole-a", "B": "whole-a", "C": "whole-c"})
        self.assertEqual(duplicates, [["A", "B"]])
        self.assertEqual(bound, 192)  # 200 kept bytes, one exact 8-byte body reuse


if __name__ == "__main__":
    unittest.main()

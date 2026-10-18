"""Independent boundary agreement must require a contiguous matching body."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_ghidra_crosscheck import compare, dedupe_leads  # noqa: E402


class LevelGhidraCrosscheckTests(unittest.TestCase):
    def test_agrees_only_with_matching_contiguous_extent(self) -> None:
        static = [{"start": "0x00000100", "body_end": "0x00000108",
                   "body_bytes": "8", "status": "bounded_candidate", "evidence": "call"}]
        ghidra = [{"entry": "0x100", "min": "0x100", "max": "0x107",
                   "body_bytes": "8"}]
        result, _, agreed = compare("LEVEL_01.REL", static, ghidra)
        self.assertEqual(result["corroborated_boundaries"], 1)
        self.assertEqual(len(agreed), 1)
        ghidra[0]["body_bytes"] = "4"
        result, worklist, agreed = compare("LEVEL_01.REL", static, ghidra)
        self.assertEqual(result["corroborated_boundaries"], 0)
        self.assertEqual(agreed, [])
        self.assertIn("boundary_disagreement", worklist)
        static[0]["status"] = "nonzero_tail"
        ghidra[0]["body_bytes"] = "8"
        result, _, _ = compare("LEVEL_01.REL", static, ghidra)
        self.assertEqual(result["corroborated_nonzero_tail_boundaries"], 1)
        static[0]["status"] = "no_return"
        static[0]["limit"] = "0x104"
        result, _, _ = compare("LEVEL_01.REL", static, ghidra)
        self.assertEqual(result["no_return_shared_entries"], 1)
        self.assertEqual(result["no_return_cross_limit_or_noncontiguous"], 1)

    def test_masked_lead_does_not_collapse_same_module_repeats(self) -> None:
        rows = [{"module": name, "body_bytes": "8", "raw_sha256": raw,
                 "masked_lead_sha256": "same"}
                for name, raw in (("LEVEL_01.REL", "a"), ("LEVEL_01.REL", "b"),
                                  ("LEVEL_02.REL", "c"))]
        counts = dedupe_leads(rows)
        self.assertEqual(counts["raw_exact_distinct_corrob_candidate_bytes"], 24)
        self.assertEqual(counts["masked_conservative_corrob_lead_bytes"], 24)


if __name__ == "__main__":
    unittest.main()

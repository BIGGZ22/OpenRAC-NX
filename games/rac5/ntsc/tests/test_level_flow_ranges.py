"""A pointer entry inside a discontiguous body remains a review lead."""

from __future__ import annotations

import sys
import struct
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_flow_ranges import classify, direct_noncall_target, parse_ranges  # noqa: E402


class LevelFlowRangesTests(unittest.TestCase):
    def test_distinguishes_internal_pointer_and_unowned_entry(self) -> None:
        functions = [{"entry": "0x100", "body_bytes": "16",
                      "ranges": "0x100-0x107;0x120-0x127"}]
        candidates = [{"start": "0x100", "status": "bounded_candidate"},
                      {"start": "0x120", "status": "pointer_only"},
                      {"start": "0x130", "status": "no_return"}]
        result, worklist = classify(candidates, functions)
        self.assertEqual(result["ghidra_body_bytes_by_placement"], 16)
        self.assertEqual(result["candidate_internal_block_leads"], 1)
        self.assertEqual(result["disposition_counts"]["no_return:unowned"], 1)
        self.assertIn("0x00000120\tpointer_only\t0x00000100", worklist)

    def test_rejects_overlapping_body_ranges(self) -> None:
        functions = [{"entry": "0x100", "body_bytes": "8",
                      "ranges": "0x100-0x107"},
                     {"entry": "0x104", "body_bytes": "8",
                      "ranges": "0x104-0x10B"}]
        with self.assertRaisesRegex(ValueError, "overlap"):
            parse_ranges(functions)

    def test_same_body_branch_corrobates_internal_block(self) -> None:
        functions = [{"entry": "0x100", "body_bytes": "8",
                      "ranges": "0x100-0x103;0x120-0x123"}]
        candidates = [{"start": "0x120", "status": "pointer_only"}]
        code = bytearray(0xA4)  # code file starts at module offset 0x80
        struct.pack_into("<I", code, 0x80, 0x10000007)  # beq at 0x100 -> 0x120
        result, worklist = classify(candidates, functions, bytes(code))
        self.assertEqual(result["internal_block_with_same_body_direct_edge"], 1)
        self.assertIn("inside_other_body+same_body_edge", worklist)

    def test_direct_noncall_decoder_excludes_calls(self) -> None:
        self.assertEqual(direct_noncall_target(0x10000007, 0x100), 0x120)
        self.assertEqual(direct_noncall_target(0x08000048, 0x100), 0x120)
        self.assertIsNone(direct_noncall_target(0x0C000048, 0x100))
        self.assertIsNone(direct_noncall_target(0x04110007, 0x100))  # BGEZAL


if __name__ == "__main__":
    unittest.main()

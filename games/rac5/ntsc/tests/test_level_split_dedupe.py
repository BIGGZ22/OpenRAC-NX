"""Selected sub-bodies require independent agreement and leave gaps untouched."""

from __future__ import annotations

import struct
import sys
import unittest
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_split_dedupe import (best_partial_family, close_callgraph_by_placement,
                                graph_target_shifts, split_parent)  # noqa: E402


class LevelSplitDedupeTests(unittest.TestCase):
    def test_partial_family_closes_callers_only_at_covered_placements(self) -> None:
        members = [{"module": name, "start": "0x100"} for name in ("A", "B", "C")]

        def resolve(group):
            if any(row["module"] == "C" for row in group):
                return None
            return ({"placements": [(row["module"], 0x100) for row in group]}, set())

        self.assertEqual(best_partial_family(members, resolve)[0]["placements"],
                         [("A", 0x100), ("B", 0x100)])
        callee = (796, "shape")
        pending = {
            (9196, "caller_ab"): ({"placements": [("A", 0x200), ("B", 0x200)]},
                                   {callee}),
            (9196, "caller_ac"): ({"placements": [("A", 0x300), ("C", 0x300)]},
                                   {callee}),
        }
        self.assertEqual(close_callgraph_by_placement(
            pending, {callee: {"A", "B"}}), {(9196, "caller_ab")})

    def test_zero_data_needs_two_unique_incoming_anchors_and_same_layout(self) -> None:
        base, other = bytearray(512), bytearray(512)
        for data, target in ((base, 100), (other, 220)):
            struct.pack_into("<I", data, 4, 512)  # SNR2 payload end.
            data[target:target + 64] = bytes(range(64))
        blobs = {"base": bytes(base), "other": bytes(other)}
        references = ({4: 100, 8: 320, 12: 312}, {})
        current = ({4: 220, 8: 360, 12: 352}, {})
        edges = {("caller_a", 4), ("caller_b", 8)}
        incoming = {"base": {320: edges}, "other": {360: edges}}
        targets = {"base": {edge: {320} for edge in edges},
                   "other": {edge: {360} for edge in edges}}

        def check(value: tuple[dict[int, int], dict[int, int]],
                  graph: dict) -> bool:
            return graph_target_shifts(references, value, "base", "other",
                                       blobs, {"base": 64, "other": 64},
                                       graph, targets, defaultdict(dict))

        self.assertTrue(check(current, incoming))
        self.assertTrue(graph_target_shifts(
            ({4: 100, 12: 312}, {}), ({4: 220, 12: 352}, {}),
            "base", "other", blobs, {"base": 64, "other": 64},
            incoming, targets, defaultdict(dict)))
        self.assertFalse(check(({4: 220, 8: 360, 12: 356}, {}), incoming))
        self.assertFalse(check(current, {"base": {320: {("caller_a", 4)}},
                                         "other": {360: {("caller_a", 4)}}}))
        ambiguous = {"base": targets["base"],
                     "other": {edge: {360, 400} for edge in edges}}
        self.assertFalse(graph_target_shifts(
            references, current, "base", "other", blobs,
            {"base": 64, "other": 64}, incoming, ambiguous,
            defaultdict(dict)))

    def test_selects_only_valid_returning_sub_bodies(self) -> None:
        data = bytearray(0x200)
        struct.pack_into("<I", data, 0x90, 0x03E00008)
        struct.pack_into("<I", data, 0xAC, 0x03E00008)
        row = {"status": "bounded_candidate", "start": "0x80",
               "body_end": "0xB4", "body_bytes": "52"}
        ranges = {
            0x80: {"min": "0x80", "max": "0x97", "body_bytes": "20",
                   "ranges": "0x80-0x87;0x8C-0x97"},
            0x9C: {"min": "0x9C", "max": "0xB3", "body_bytes": "24",
                   "ranges": "0x9C-0xB3"},
        }
        self.assertEqual(split_parent(row, ranges, ranges, ranges,
                                      [0x80, 0x9C], bytes(data), []),
                         [(0x80, 0x98, 20), (0x9C, 0xB4, 24)])
        self.assertEqual(split_parent({**row, "status": "pointer_only"},
                                      ranges, ranges, ranges, [0x80, 0x9C],
                                      bytes(data), []),
                         [(0x80, 0x98, 20), (0x9C, 0xB4, 24)])
        self.assertEqual(split_parent({**row, "status": "nonzero_tail"},
                                      ranges, ranges, ranges, [0x80, 0x9C],
                                      bytes(data), []),
                         [(0x80, 0x98, 20), (0x9C, 0xB4, 24)])
        data[0x88] = 1
        self.assertEqual(split_parent(row, ranges, ranges, ranges,
                                      [0x80, 0x9C], bytes(data), []),
                         [(0x9C, 0xB4, 24)])
        data[0x88] = 0
        self.assertEqual(split_parent(row, ranges, ranges, ranges,
                                      [0x80, 0x9C], bytes(data), [0x98]),
                         [(0x80, 0x98, 20), (0x9C, 0xB4, 24)])
        divergent = {**ranges, 0x9C: {**ranges[0x9C], "body_bytes": "20"}}
        self.assertEqual(split_parent(row, ranges, ranges, divergent,
                                      [0x80, 0x9C], bytes(data), []),
                         [(0x80, 0x98, 20)])
        struct.pack_into("<I", data, 0x90, 0)
        self.assertEqual(split_parent(row, ranges, ranges, ranges,
                                      [0x80, 0x9C], bytes(data), []),
                         [(0x9C, 0xB4, 24)])
        struct.pack_into("<I", data, 0xAC, 0)
        self.assertIsNone(split_parent(row, ranges, ranges, ranges,
                                       [0x80, 0x9C], bytes(data), []))


if __name__ == "__main__":
    unittest.main()

"""Regression coverage for catalogue regeneration of four joins of executable code."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import overlay_boundary_fixes as fixes


class MatchedBoundaryTests(unittest.TestCase):
    def setUp(self):
        self.specs = fixes.matched_merges()
        self.correct = {row[0]: row for row in fixes.load(fixes.ov.CATALOGUE)}

    def split_again(self):
        rows = copy.deepcopy(self.correct)
        for spec in self.specs:
            head = rows.pop(spec["exe"])
            offset = spec["prefix_size"]
            rows[spec["tail"]] = (spec["tail"], "shared", spec["size"] - offset,
                                  spec["tail_fingerprint"], [(lv, a + offset) for lv, a in head[4]])
            prefix = rows[spec["prefix"]]
            rows[spec["prefix"]] = (*prefix[:4], sorted(prefix[4] + head[4]))
        return rows

    def test_rebuilds_only_the_four_proven_ranges(self):
        self.assertEqual({s["head"]: s["exe"] for s in self.specs}, {
            "func_L00_0024BBB0": "func_002095E8", "func_L00_0024BF98": "func_002099D0",
            "func_L00_0024BFE0": "func_00209A18", "func_L00_0025D3C0": "func_00215078"})
        rows = self.split_again()
        fixes.restore_matched_merges(rows)
        self.assertEqual(rows, self.correct)

    def test_already_correct_is_idempotent(self):
        rows = copy.deepcopy(self.correct)
        fixes.restore_matched_merges(rows)
        self.assertEqual(rows, self.correct)

    def test_refuses_a_missing_absorbed_prefix(self):
        rows = self.split_again()
        spec = self.specs[0]
        prefix = rows[spec["prefix"]]
        target = (spec["places"][0][0], int(spec["places"][0][1], 16))
        rows[spec["prefix"]] = (*prefix[:4], [p for p in prefix[4] if p != target])
        with self.assertRaisesRegex(ValueError, "missing audited prefix"):
            fixes.restore_matched_merges(rows)

    def test_refuses_an_unverified_tail_size(self):
        rows = self.split_again()
        tail = self.specs[0]["tail"]
        row = rows[tail]
        rows[tail] = (row[0], row[1], row[2] + 4, row[3], row[4])
        with self.assertRaisesRegex(ValueError, "split tail differs"):
            fixes.restore_matched_merges(rows)


if __name__ == "__main__":
    unittest.main()

"""Optimistic grouping is deterministic and remains explicitly unqualified."""

from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_shape_scenario import OUTPUT, check_summary, shape_counts  # noqa: E402


class LevelShapeScenarioTests(unittest.TestCase):
    def test_repeated_shapes_in_one_module_keep_distinct_placements(self) -> None:
        units = [("A", 8, "raw1", "shape_x"),
                 ("A", 8, "raw2", "shape_x"),
                 ("B", 8, "raw3", "shape_x"),
                 ("B", 12, "raw4", "shape_y")]
        self.assertEqual(shape_counts(units), {
            "raw_shape_groups": 4,
            "masked_shape_groups": 2,
            "raw_max_per_module_bytes": 36,
            "masked_max_per_module_bytes": 28,
            "masked_one_per_shape_bytes": 20,
            "masked_repeated_within_module_groups": 1,
        })

    def test_committed_scenario_is_scoped_and_pinned(self) -> None:
        summary = json.loads(OUTPUT.read_text(encoding="utf-8"))
        check_summary(summary)
        self.assertIsNone(summary["overall_percent"])
        self.assertFalse(summary["bytematch_ready"])
        self.assertLess(summary["masked_max_per_module_scenario_bytes"],
                        summary["retained_candidate_interval_bytes"])
        altered = {**summary, "masked_max_per_module_scenario_bytes": 0}
        with self.assertRaisesRegex(ValueError, "stale or overclaims"):
            check_summary(altered)


if __name__ == "__main__":
    unittest.main()

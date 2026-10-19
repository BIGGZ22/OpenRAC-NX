"""Role-paired structural leads require closed dependencies and unique roles."""

from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_role_graph import (OUTPUT, check_summary, paired_graph,
                              prune_role_graph)  # noqa: E402


class RoleGraphTests(unittest.TestCase):
    def test_greatest_fixed_point_drops_missing_child_but_keeps_cycle(self) -> None:
        class Audit:
            known = {("A", 0): (8, "root"), ("B", 0): (8, "root"),
                     ("A", 8): (8, "child"), ("B", 8): (8, "child")}

            def pair(self, _ref, _mod, a, _b, _anchors):
                return ([(8, 8)], []) if a == 0 else None

        alive, _deps, _roles, seeds, _reviewed = paired_graph(
            Audit(), "A", "B", [])
        self.assertEqual(seeds, 2)
        self.assertEqual(alive, set())

        class CyclicAudit(Audit):
            def pair(self, _ref, _mod, a, _b, _anchors):
                return ([(8, 8)] if a == 0 else [(0, 0)], [])

        alive, _deps, _roles, _seeds, _reviewed = paired_graph(
            CyclicAudit(), "A", "B", [])
        self.assertEqual(alive, {(0, 0), (8, 8)})

    def test_role_conflict_and_lost_callee_prune_callers(self) -> None:
        first = ("A", "B", 0, 0)
        preferred = ("A", "B", 8, 8)
        caller = ("A", "B", 24, 24)
        candidates = {
            first: {"body_bytes": 8, "placements": [("A", 0), ("B", 0)]},
            preferred: {"body_bytes": 16, "placements": [("A", 8), ("B", 8)]},
            caller: {"body_bytes": 32, "placements": [("A", 24), ("B", 24)]},
        }
        dependencies = {first: [], preferred: [], caller: [("A", "B", 0, 0)]}
        roles = {first: [(4, 100, 200, "variant")],
                 preferred: [(4, 100, 204, "variant")], caller: []}
        selected, metrics = prune_role_graph(
            {}, [], candidates, dependencies, roles, {"A", "B"})
        self.assertEqual(set(selected), {preferred})
        self.assertEqual(metrics["role_removed_pairs"], 1)
        self.assertGreaterEqual(metrics["dependency_removed_pairs"], 1)

    def test_committed_role_report_is_explicitly_unqualified(self) -> None:
        summary = json.loads(OUTPUT.read_text(encoding="utf-8"))
        check_summary(summary)
        self.assertIsNone(summary["overall_percent"])
        self.assertFalse(summary["bytematch_ready"])


if __name__ == "__main__":
    unittest.main()

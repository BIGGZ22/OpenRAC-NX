"""A partial progress snapshot must never become a guessed game-wide score."""

from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import boot_c_campaign  # noqa: E402
from boot_trial_gate import accepted_body_bytes  # noqa: E402
from progress_status import OUTPUT, snapshot  # noqa: E402


class ProgressStatusTests(unittest.TestCase):
    def test_committed_snapshot_is_reproducible_and_scoped(self) -> None:
        report = json.loads(OUTPUT.read_text(encoding="utf-8"))
        self.assertEqual(snapshot(report["evidence"]), report)
        self.assertIsNone(report["overall_percent"])
        self.assertFalse(report["bytematch_objdiff_ready"])
        self.assertEqual(report["boot"]["accepted_c_bytes"],
                         accepted_body_bytes())
        self.assertEqual(report["boot"]["accepted_functions"][0], "MEMCARD_Idle__Fv")
        self.assertEqual(len(report["boot"]["accepted_functions"]),
                         len(boot_c_campaign.ACCEPTED_SOURCES))
        self.assertEqual(report["levels"]["accepted_c_bytes_per_unique_source"], 80)
        self.assertEqual(report["levels"]["accepted_c_bytes_by_placement"], 1600)
        self.assertEqual(report["levels"]["accepted_frontend_c_bytes"], 476)
        self.assertEqual(report["levels"]["accepted_frontend_functions"],
                         ["func_00002F80"])
        self.assertEqual(report["levels"]["accepted_shared_functions"],
                         ["func_00000100", "func_shared_zero_buffer"])

    def test_rejects_unproven_match_and_relocated_image(self) -> None:
        evidence = json.loads(OUTPUT.read_text(encoding="utf-8"))["evidence"]
        changed = copy.deepcopy(evidence)
        changed["boot"]["counted_bytes"] = 32
        with self.assertRaisesRegex(ValueError, "qualified scope"):
            snapshot(changed)
        changed = copy.deepcopy(evidence)
        changed["levels"][0]["relocated_sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "pinned placement"):
            snapshot(changed)
        changed = copy.deepcopy(evidence)
        changed["level_c"]["matched_placement_bytes"] = 2084
        with self.assertRaisesRegex(ValueError, "level C evidence"):
            snapshot(changed)
        changed = copy.deepcopy(evidence)
        changed["level_c"]["frontend_accepted"]["body_bytes"] = 472
        with self.assertRaisesRegex(ValueError, "frontend C evidence|level C evidence"):
            snapshot(changed)


if __name__ == "__main__":
    unittest.main()

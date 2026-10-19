"""The public dashboard must count only accepted C against a labelled estimate."""

from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import boot_trial_gate  # noqa: E402
from bytematch_estimate import report  # noqa: E402


class ByteMatchEstimateTests(unittest.TestCase):
    def setUp(self) -> None:
        self.status = json.loads((ROOT / "progress/status.json").read_text())
        self.shape = json.loads((ROOT / "config/levels/shape_scenario_summary.json").read_text())

    def test_report_counts_verified_c_once_and_labels_estimate(self) -> None:
        result = report(self.status, self.shape)
        self.assertEqual(result["version"], 2)
        accepted = boot_trial_gate.accepted_body_bytes()
        self.assertEqual(result["measures"]["matched_code"], str(accepted + 556))
        self.assertEqual(result["measures"]["total_code"], "6418204")
        self.assertEqual(result["measures"]["total_units"], 3)
        self.assertEqual([u["measures"]["matched_code"] for u in result["units"]],
                         [str(accepted), "476", "80"])
        self.assertIn("optimistic", result["units"][2]["name"])
        self.assertFalse(self.status["bytematch_objdiff_ready"])

    def test_stale_or_promoted_scope_is_rejected(self) -> None:
        changed = copy.deepcopy(self.shape)
        changed["masked_max_per_module_scenario_bytes"] += 4
        with self.assertRaisesRegex(ValueError, "disagree"):
            report(self.status, changed)
        changed_status = copy.deepcopy(self.status)
        changed_status["bytematch_objdiff_ready"] = True
        with self.assertRaisesRegex(ValueError, "provisional"):
            report(changed_status, self.shape)


if __name__ == "__main__":
    unittest.main()

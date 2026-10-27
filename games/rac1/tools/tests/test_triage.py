from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import triage


class AssemblyRoutingTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        for replacement in (
            patch.object(triage, "ROOT", self.root),
            patch.object(triage, "sdk_symbols", return_value={}),
            patch.object(triage, "reference_index", return_value={}),
            patch.object(triage, "last_attempt", return_value=""),
            patch.object(triage.rank_candidates, "already_attempted", return_value=set()),
        ):
            replacement.start()
            self.addCleanup(replacement.stop)

    def classify(self, name, unit, directory=None, verdict=("candidate", "candidate", "")):
        if directory is not None:
            assembly = self.root / directory / (name + ".s")
            assembly.parent.mkdir(parents=True, exist_ok=True)
            assembly.write_text("test assembly")
        row = {"name": name, "unit": unit, "size": 24, "status": "stub", "match_percent": 0}
        with patch.object(triage, "report_functions", return_value=[row]), \
                patch.object(triage.rank_candidates, "classify", return_value=verdict) as classifier:
            result = triage.triage()[0]
        return result, classifier

    def test_shared_overlay_uses_overlay_assembly(self):
        result, classifier = self.classify("func_L00_00248EF8", "Shared level code", "asm/overlays")
        self.assertEqual(result["route"], "opus")
        classifier.assert_called_once_with("func_L00_00248EF8", "test assembly", "text", 24)

    def test_level_overlay_preserves_a_real_blocker(self):
        result, classifier = self.classify("func_L10_002C0378", "Level 10: Orxon", "asm/overlays",
                                           ("blocked", "alternate entry point", "alabel"))
        self.assertEqual(result["route"], "blocked")
        self.assertEqual(result["reason"], "alternate entry point")
        classifier.assert_called_once_with("func_L10_002C0378", "test assembly", "text", 24)

    def test_missing_overlay_does_not_use_an_executable_stub(self):
        result, classifier = self.classify("func_L00_00248EF8", "Shared level code", "asm/nonmatchings/core_text")
        self.assertEqual(result["route"], "blocked")
        self.assertIn("no asm", result["reason"])
        classifier.assert_not_called()

    def test_game_assembly_keeps_its_segment(self):
        result, classifier = self.classify("func_00213C78", "game/mobyutil", "asm/nonmatchings/text")
        self.assertEqual(result["route"], "opus")
        classifier.assert_called_once_with("func_00213C78", "test assembly", "text", 24)

    def test_core_assembly_keeps_its_segment(self):
        result, classifier = self.classify("func_0012C608", "core/0012AC80", "asm/nonmatchings/core_text")
        self.assertEqual(result["route"], "opus")
        classifier.assert_called_once_with("func_0012C608", "test assembly", "core_text", 24)


if __name__ == "__main__":
    unittest.main()

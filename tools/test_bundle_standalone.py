"""The copy of OpenRAC an installed launcher carries: its actions and its files."""
import json
import tempfile
import unittest
from pathlib import Path

import bundle_standalone as bundle


class PlayerActions(unittest.TestCase):
    def setUp(self):
        self.actions = bundle.player_actions()
        self.pal = {a["id"]: a for a in self.actions["versions"]["rac1/pal"]}

    def test_only_what_a_player_does(self):
        self.assertEqual(self.actions["repository"], [])
        for key, actions in self.actions["versions"].items():
            self.assertLessEqual({a["id"] for a in actions}, set(bundle.KEEP), key)
        self.assertEqual(set(self.pal), {"port-levels", "play"})

    def test_levels_go_to_the_data_folder(self):
        levels = self.pal["port-levels"]
        self.assertEqual(levels["args"][-2:], ["{data}/port", "--anywhere"])
        self.assertTrue(levels["artifact"].startswith("{data}/port/"))

    def test_play_reads_them_there(self):
        play = self.pal["play"]
        self.assertNotIn("--levels", play["args"])      # the game's own default is <data>/port
        self.assertEqual([need["path"] for need in play["needs"]], [self.pal["port-levels"]["artifact"]])
        self.assertNotIn("python", play.get("requires", []))

    def test_nothing_writes_in_the_copy(self):
        for action in self.pal.values():
            for arg in action.get("args", []):
                self.assertFalse(arg.startswith(("assets/", "build/", "{root}/assets", "{root}/build")), arg)


class Bundle(unittest.TestCase):
    def test_a_copy_without_game_or_python(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "openrac"
            import sys
            argv, sys.argv = sys.argv, ["bundle_standalone.py", str(out)]
            try:
                self.assertEqual(bundle.main(), 0)
            finally:
                sys.argv = argv
            for rel in ("STANDALONE", "tools/openrac.py", "tools/extractor.py", "editor/extract.py",
                        "editor/moby_classes.tsv", "games/rac1/game.json", "progress/summary.json",
                        "launcher/actions.json"):
                self.assertTrue((out / rel).is_file(), rel)
            self.assertFalse(list(out.rglob("test_*.py")))
            json.loads((out / "launcher/actions.json").read_text())


if __name__ == "__main__":
    unittest.main()

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
        self.ntsc = {a["id"]: a for a in self.actions["versions"]["rac1/ntsc"]}

    def test_only_what_a_player_does(self):
        self.assertEqual(self.actions["repository"], [])
        for key, actions in self.actions["versions"].items():
            self.assertLessEqual({a["id"] for a in actions}, set(bundle.KEEP), key)
        self.assertEqual(set(self.pal), {"port-levels", "port-levels-check", "play"})
        self.assertEqual(set(self.ntsc), {"port-levels", "port-levels-check", "play"})

    def test_levels_go_to_the_data_folder(self):
        levels = self.pal["port-levels"]
        self.assertEqual(levels["args"][-3:], ["{data}/port", "--anywhere", "--replace"])
        self.assertTrue(levels["artifact"].startswith("{data}/port/"))

    def test_play_reads_them_there(self):
        play = self.pal["play"]
        self.assertNotIn("--levels", play["args"])      # the game's own default is <data>/port
        # Play waits for the check's stamp, and the check for the export.
        check = self.pal["port-levels-check"]
        self.assertEqual(check["makes"], "{data}/port/verified-3.json")
        self.assertNotIn("--levels", check["args"])
        self.assertEqual([need["path"] for need in check["needs"]], [self.pal["port-levels"]["artifact"]])
        self.assertEqual([need["path"] for need in play["needs"]], [check["makes"]])
        self.assertNotIn("python", play.get("requires", []))

    def test_ntsc_levels_have_a_folder_of_their_own(self):
        # <data> is the game's, shared by PAL and NTSC-U: the US levels must not land on PAL's.
        levels, check = self.ntsc["port-levels"], self.ntsc["port-levels-check"]
        self.assertEqual(levels["args"][-3:], ["{data}/port-ntsc", "--anywhere", "--replace"])
        self.assertEqual(levels["artifact"], "{data}/port-ntsc/level_99/manifest.json")
        # The program's own default is <data>/port, so the check names the folder.
        at = check["args"].index("--levels")
        self.assertEqual(check["args"][at + 1], "{data}/port-ntsc")
        self.assertEqual(check["makes"], "{data}/port-ntsc/verified-3.json")
        self.assertEqual([need["path"] for need in check["needs"]], [levels["artifact"]])
        self.assertTrue(check["artifact"].endswith("openrac-rac1-ntsc.exe"))

    def test_folders_and_arguments(self):
        self.assertEqual(bundle.levels_folder("rac1/pal"), "{data}/port")
        self.assertEqual(bundle.levels_folder("rac1/ntsc"), "{data}/port-ntsc")
        self.assertEqual(bundle.levels_folder("rac9/ntsc-j"), "{data}/port-ntsc-j")
        args = ["--data", "{data}", "--levels", "{root}/assets/port/x", "--window"]
        self.assertEqual(bundle.point_at(args, "{data}/port"), ["--data", "{data}", "--window"])
        self.assertEqual(bundle.point_at(args, "{data}/port-x"), ["--data", "{data}", "--levels", "{data}/port-x", "--window"])
        self.assertEqual(bundle.point_at(["--data", "{data}"], "{data}/port-x"), ["--data", "{data}"])

    def test_every_action_starts_in_a_folder_the_copy_has(self):
        # 0.4.0 started the game in games/rac1, which only a checkout has: Play failed with
        # "the directory name is invalid".
        self.assertEqual(self.pal["play"]["cwd"], "{data}")
        self.assertIn(self.pal["port-levels"]["cwd"], (".", "{root}"))

    def test_nothing_writes_in_the_copy(self):
        for action in [*self.pal.values(), *self.ntsc.values()]:
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

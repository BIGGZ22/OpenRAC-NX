"""The copy of OpenRAC an installed launcher carries: its actions and its files."""
import json
import shutil
import subprocess
import sys
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
        # Playing, and modding: every player gets the Mods page, not only a checkout's.
        self.assertEqual(set(self.pal), set(bundle.KEEP))
        self.assertEqual(set(self.ntsc), set(bundle.KEEP))

    def test_the_later_games_levels_are_read_for_packs_in_the_data_folder(self):
        # Going Commando, Up Your Arsenal and Deadlocked: early access, so no play and no packing
        # back; their levels are read into the data folder for texture and model packs.
        for key, first in (("rac2/ntsc", "00"), ("rac3/ntsc", "01"), ("rac4/ntsc", "00")):
            with self.subTest(key):
                actions = {a["id"]: a for a in self.actions["versions"][key]}
                self.assertNotIn("editor-pack", actions)
                levels = actions["port-levels"]
                self.assertEqual(levels["args"][-3:], ["{data}/port/ntsc", "--anywhere", "--replace"])
                self.assertEqual(levels["artifact"], f"{{data}}/port/ntsc/level_{first}/manifest.json")
                self.assertEqual(actions["editor-extract"]["args"][-3:], ["{data}/levels/ntsc", "--split", "--anywhere"])
                self.assertEqual([n["path"] for n in actions["assets-export"]["needs"]], [levels["artifact"]])

    def test_mods_work_in_the_data_folder(self):
        extract, pack = self.pal["editor-extract"], self.pal["editor-pack"]
        self.assertEqual(extract["args"][-3:], ["{data}/levels/pal", "--split", "--anywhere"])
        self.assertEqual(pack["args"][-3:], ["{data}/levels/pal", "{data}/packed/pal", "--anywhere"])
        self.assertEqual(pack["artifact"], "{data}/levels/pal/level_00/project.godot")
        self.assertEqual(self.pal["editor-open"]["artifact"], "{data}/levels/pal/level_{level}/project.godot")
        self.assertIn("{data}/levels/pal/level_{level}", self.pal["editor-open"]["args"])
        stack = self.pal["overrides-stack"]["args"]
        self.assertEqual(stack[stack.index("--packed") + 1], "{data}/packed/pal")
        self.assertEqual(stack[stack.index("--levels") + 1], "{data}/port/pal")
        exported = self.pal["assets-export"]
        self.assertEqual(exported["args"][2], "{data}/port/pal")
        self.assertEqual([need["path"] for need in exported["needs"]], ["{data}/port/pal/level_99/manifest.json"])
        edited = self.pal["play-edited"]
        self.assertEqual(edited["args"][edited["args"].index("--levels") + 1], "{data}/port/pal")
        self.assertEqual([need["path"] for need in edited["needs"]], [self.pal["port-levels-check"]["makes"]])
        self.assertEqual(edited["cwd"], "{data}")
        # NTSC-U's editor folders are its own.
        self.assertEqual(self.ntsc["editor-extract"]["args"][-3], "{data}/levels/ntsc")

    def test_no_mod_action_names_the_checkout(self):
        for action in [*self.pal.values(), *self.ntsc.values()]:
            text = json.dumps({k: action.get(k) for k in ("args", "artifact", "needs", "makes")})
            self.assertNotIn("assets/", text.replace("{data}", ""), action["id"])

    def test_levels_go_to_the_data_folder(self):
        levels = self.pal["port-levels"]
        self.assertEqual(levels["args"][-3:], ["{data}/port/pal", "--anywhere", "--replace"])
        self.assertEqual(levels["artifact"], "{data}/port/pal/level_99/manifest.json")

    def test_play_reads_them_there(self):
        play = self.pal["play"]
        self.assertEqual(play["args"][play["args"].index("--levels") + 1], "{data}/port/pal")
        # Play waits for the check's stamp, and the check for the export.
        check = self.pal["port-levels-check"]
        self.assertEqual(check["makes"], "{data}/port/pal/verified-3.json")
        self.assertEqual(check["args"][check["args"].index("--levels") + 1], "{data}/port/pal")
        self.assertEqual([need["path"] for need in check["needs"]], [self.pal["port-levels"]["artifact"]])
        self.assertEqual([need["path"] for need in play["needs"]], [check["makes"]])
        self.assertNotIn("python", play.get("requires", []))

    def test_the_probe_bake_reads_and_writes_the_data_folder(self):
        for version, actions in (("pal", self.pal), ("ntsc", self.ntsc)):
            bake = actions["probe-bake"]
            args = bake["args"]
            self.assertEqual(args[args.index("--levels-dir") + 1], f"{{data}}/port/{version}")
            self.assertEqual(args[args.index("--output-dir") + 1], f"{{data}}/probes/{version}")
            self.assertIn("--update", args)
            self.assertEqual([need["path"] for need in bake["needs"]], [actions["port-levels-check"]["makes"]])
            self.assertTrue(bake["artifact"].endswith("openrac-probe-bake.exe"))

    def test_ntsc_levels_have_a_folder_of_their_own(self):
        # <data> is the game's, shared by PAL and NTSC-U: each version's levels are <data>/port/<version>.
        levels, check = self.ntsc["port-levels"], self.ntsc["port-levels-check"]
        self.assertEqual(levels["args"][-3:], ["{data}/port/ntsc", "--anywhere", "--replace"])
        self.assertEqual(levels["artifact"], "{data}/port/ntsc/level_99/manifest.json")
        at = check["args"].index("--levels")
        self.assertEqual(check["args"][at + 1], "{data}/port/ntsc")
        self.assertEqual(check["makes"], "{data}/port/ntsc/verified-3.json")
        self.assertEqual([need["path"] for need in check["needs"]], [levels["artifact"]])
        self.assertTrue(check["artifact"].endswith("openrac-rac1-ntsc.exe"))

    def test_folders_and_arguments(self):
        self.assertEqual(bundle.levels_folder("rac1/pal"), "{data}/port/pal")
        self.assertEqual(bundle.levels_folder("rac1/ntsc"), "{data}/port/ntsc")
        self.assertEqual(bundle.levels_folder("rac9/ntsc-j"), "{data}/port/ntsc-j")
        args = ["--data", "{data}", "--levels", "{root}/assets/port/x", "--window"]
        self.assertEqual(bundle.point_at(args, "{data}/port/x"), ["--data", "{data}", "--levels", "{data}/port/x", "--window"])
        self.assertEqual(bundle.point_at(["--data", "{data}"], "{data}/port/x"), ["--data", "{data}"])

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
                        "launcher/actions.json", "port/enhanced/tools/probes/bake_all.py",
                        "port/enhanced/tools/probes/moby_seed_whitelist.json"):
                self.assertTrue((out / rel).is_file(), rel)
            self.assertFalse(list(out.rglob("test_*.py")))
            json.loads((out / "launcher/actions.json").read_text())
            # The copy's tools run from it: their helpers are carried, and find the copy's root.
            self.assertTrue((out / "tools/openrac_common/__init__.py").is_file())
            for tool in ("openrac.py", "extractor.py"):
                done = subprocess.run([sys.executable, str(out / "tools" / tool), "--help"], cwd=tmp,
                                      capture_output=True, text=True)
                self.assertNotIn("Traceback", done.stderr, tool)
            root = subprocess.run([sys.executable, "-c", "from openrac_common import paths; print(paths.repo_root())"],
                                  cwd=out / "tools", capture_output=True, text=True, check=True).stdout.strip()
            self.assertEqual(Path(root), out.resolve())

    def test_games_as_built(self):
        # On macOS a game is its bundle, carried whole; the build folder's link to it is not.
        with tempfile.TemporaryDirectory() as tmp:
            build, dest = Path(tmp) / "build", Path(tmp) / "dest"
            app = "apps/openrac-rac1-pal/Ratchet & Clank.app"
            (build / app / "Contents/MacOS").mkdir(parents=True)
            (build / app / "Contents/MacOS/Ratchet & Clank").write_bytes(b"pal")
            (build / app / "Contents/Info.plist").write_text("<plist/>")
            (build / "openrac-rac1-pal").symlink_to(f"{app}/Contents/MacOS/Ratchet & Clank")
            # Elsewhere a game is one file (and a .pdb is left behind).
            (build / "openrac-rac1-ntsc.exe").write_bytes(b"ntsc")
            (build / "openrac-rac1-ntsc.pdb").write_bytes(b"")
            dest.mkdir()
            self.assertEqual(bundle.copy_games(build, dest), ["apps/openrac-rac1-pal", "openrac-rac1-ntsc.exe"])
            self.assertEqual((dest / app / "Contents/MacOS/Ratchet & Clank").read_bytes(), b"pal")
            self.assertTrue((dest / app / "Contents/Info.plist").is_file())
            self.assertEqual(sorted(p.name for p in dest.iterdir()), ["apps", "openrac-rac1-ntsc.exe"])

    @unittest.skipUnless(sys.platform.startswith("linux") and shutil.which("cc")
                         and (shutil.which("llvm-strip") or shutil.which("strip")), "needs a Linux compiler and strip")
    def test_linux_programs_go_without_their_debug_info(self):
        with tempfile.TemporaryDirectory() as tmp:
            source, program = Path(tmp) / "main.c", Path(tmp) / "openrac-rac1-pal"
            source.write_text("int main(void) { return 0; }\n")
            subprocess.run(["cc", "-g", "-o", str(program), str(source)], check=True)
            self.assertIn(b".debug_info", program.read_bytes())
            self.assertTrue(bundle.strip_debug(program))
            self.assertNotIn(b".debug_info", program.read_bytes())
            self.assertIn(b"main", program.read_bytes())  # the symbols stay
            subprocess.run([str(program)], check=True)
            # Not an ELF program (a Windows .exe): left as it is.
            exe = Path(tmp) / "openrac-rac1-pal.exe"
            exe.write_bytes(b"MZ")
            self.assertFalse(bundle.strip_debug(exe))
            self.assertEqual(exe.read_bytes(), b"MZ")

    def test_the_android_copy(self):
        # The games and the Python program are the APK's libraries: the copy carries a PYTHONHOME.
        import sys
        with tempfile.TemporaryDirectory() as tmp:
            home, out = Path(tmp) / "python", Path(tmp) / "openrac"
            (home / "lib/python3.13/lib-dynload").mkdir(parents=True)
            (home / "lib/python3.13/os.py").write_text("")
            (home / "lib/python3.13/test").mkdir()
            argv, sys.argv = sys.argv, ["bundle_standalone.py", str(out), "--android", "--python", str(home)]
            try:
                self.assertEqual(bundle.main(), 0)
            finally:
                sys.argv = argv
            self.assertTrue((out / "python/lib/python3.13/os.py").is_file())
            self.assertFalse((out / "python/lib/python3.13/test").exists())
            self.assertFalse((out / "port").exists())
            self.assertIn("Android", (out / "STANDALONE").read_text())


if __name__ == "__main__":
    unittest.main()

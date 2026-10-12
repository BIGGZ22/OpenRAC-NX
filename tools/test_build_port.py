"""tools/build_port.py's plans, without building anything:
python3 -m unittest discover -s tools"""

import contextlib
import io
import tempfile
import unittest
from pathlib import Path

import build_port

CACHE = """# This is the CMakeCache file.
//Path to a program.
CMAKE_C_COMPILER:STRING=C:/LLVM/bin/clang.exe
CMAKE_CXX_COMPILER:STRING=C:/LLVM/bin/clang++.exe
CMAKE_MAKE_PROGRAM:FILEPATH=C:/Ninja/ninja.exe
OPENRAC_RAC1_PAL_SOURCE:PATH=C:/work/rac1-decomp
WITH_EQUALS:STRING=a=b
"""
BUILD = ["cmake", "--build", "--preset", "release", "--target", "openrac-rac1-pal", "openrac-extractor"]


class Reading(unittest.TestCase):
    def test_cmake_cache_entries(self):
        cache = build_port.parse_cache(CACHE)
        self.assertEqual(cache["CMAKE_C_COMPILER"], "C:/LLVM/bin/clang.exe")
        self.assertEqual(cache["OPENRAC_RAC1_PAL_SOURCE"], "C:/work/rac1-decomp")
        self.assertEqual(cache["WITH_EQUALS"], "a=b")
        self.assertEqual(len(cache), 5)

    def test_what_cmd_set_prints(self):
        env = build_port.parse_environment("Path=C:\\a;C:\\b\r\nINCLUDE=C:\\inc\r\n=C:=C:\\\r\n")
        self.assertEqual(env, {"PATH": "C:\\a;C:\\b", "INCLUDE": "C:\\inc"})

    def test_names_follow_the_games(self):
        self.assertEqual(build_port.source_variable("rac1-pal"), "OPENRAC_RAC1_PAL_SOURCE")
        self.assertEqual(build_port.targets(["rac1-pal"]), ["openrac-rac1-pal", "openrac-extractor"])
        self.assertIn("rac1-pal", build_port.games())
        self.assertEqual(build_port.default_source("rac1-pal"), build_port.ROOT / "games" / "rac1" / "pal")

    def test_clang_is_looked_for_in_order(self):
        env = {"OPENRAC_CLANG": "X:/mine/clang.exe", "PROGRAMFILES": "X:/PF", "USERPROFILE": "X:/me", "PATH": ""}
        found = build_port.clang_candidates(env, Path("X:/VS"))
        self.assertEqual(found[:4], [
            Path("X:/mine/clang.exe"),
            Path("X:/PF/LLVM/bin/clang.exe"),
            Path("X:/me/scoop/apps/llvm/current/bin/clang.exe"),
            Path("X:/VS/VC/Tools/Llvm/x64/bin/clang.exe"),
        ])


class Commands(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.build = Path(self.tmp.name) / "release"
        self.build.mkdir()

    def tearDown(self):
        self.tmp.cleanup()

    def plan(self, cache, source=None, compilers=None, configured=True):
        if configured:
            (self.build / "build.ninja").write_text("")
        return build_port.commands("cmake", "release", self.build, ["rac1-pal"], source, cache, compilers)

    def test_a_new_folder_is_configured_from_the_preset_with_the_compilers_found(self):
        steps = self.plan({}, compilers=("C:/LLVM/clang.exe", "C:/LLVM/clang++.exe", "C:/Ninja/ninja.exe"),
                          configured=False)
        self.assertEqual(steps, [
            ["cmake", "--preset", "release", "-DCMAKE_C_COMPILER=C:/LLVM/clang.exe",
             "-DCMAKE_CXX_COMPILER=C:/LLVM/clang++.exe", "-DCMAKE_MAKE_PROGRAM=C:/Ninja/ninja.exe"],
            BUILD,
        ])

    def test_a_new_folder_takes_the_presets_compilers_when_none_are_found(self):
        self.assertEqual(self.plan({}, configured=False), [["cmake", "--preset", "release"], BUILD])

    def test_a_configured_folder_is_only_built(self):
        self.assertEqual(self.plan(build_port.parse_cache(CACHE)), [BUILD])

    def test_the_same_source_again_is_only_built(self):
        cache = build_port.parse_cache(CACHE)
        self.assertEqual(self.plan(cache, source=Path(cache["OPENRAC_RAC1_PAL_SOURCE"])), [BUILD])

    def test_another_source_is_configured_without_the_preset(self):
        steps = self.plan(build_port.parse_cache(CACHE), source=Path("/elsewhere/rac1"))
        self.assertEqual(steps[0][:2], ["cmake", "-S"])
        self.assertNotIn("--preset", steps[0])
        self.assertEqual(steps[0][-1], "-DOPENRAC_RAC1_PAL_SOURCE=" + Path("/elsewhere/rac1").as_posix())
        self.assertEqual(steps[1], BUILD)

    def test_a_new_folder_runs_hostgen_under_this_python(self):
        steps = build_port.commands("cmake", "release", self.build, ["rac1-pal"], None, {}, None, "/usr/bin/python3")
        self.assertEqual(steps[0], ["cmake", "--preset", "release", "-DPython3_EXECUTABLE=/usr/bin/python3"])

    def test_more_targets_are_built_with_the_game(self):
        (self.build / "build.ninja").write_text("")
        steps = build_port.commands("cmake", "release", self.build, ["rac1-pal"], None,
                                    build_port.parse_cache(CACHE), more=["runtime_test"])
        self.assertEqual(steps, [BUILD + ["runtime_test"]])

    def test_an_unfinished_folder_keeps_its_own_compilers(self):
        steps = self.plan(build_port.parse_cache(CACHE), compilers=("other", "other++", "ninja2"), configured=False)
        self.assertEqual(steps[0][:3], ["cmake", "--preset", "release"])
        self.assertIn("-DCMAKE_C_COMPILER=C:/LLVM/bin/clang.exe", steps[0])
        self.assertNotIn("-DCMAKE_C_COMPILER=other", steps[0])


class CommandLine(unittest.TestCase):
    def refused(self, argv):
        with self.assertRaises(SystemExit), contextlib.redirect_stderr(io.StringIO()):
            build_port.main(argv)

    def test_an_unknown_game_is_refused(self):
        self.refused(["rac9-pal"])

    def test_a_source_for_two_games_is_refused(self):
        self.refused(["rac1-pal", "rac1-ntsc", "--source", "."])


if __name__ == "__main__":
    unittest.main()

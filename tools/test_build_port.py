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
        self.assertEqual(build_port.default_source("rac1-pal"), build_port.ROOT / "games" / "rac1")

    def test_clang_is_looked_for_in_order(self):
        env = {"OPENRAC_CLANG": "X:/mine/clang.exe", "PROGRAMFILES": "X:/PF", "USERPROFILE": "X:/me", "PATH": ""}
        found = build_port.clang_candidates(env, Path("X:/VS"))
        self.assertEqual(found[:4], [
            Path("X:/mine/clang.exe"),
            Path("X:/PF/LLVM/bin/clang.exe"),
            Path("X:/me/scoop/apps/llvm/current/bin/clang.exe"),
            Path("X:/VS/VC/Tools/Llvm/x64/bin/clang.exe"),
        ])


class Clean(unittest.TestCase):
    def test_a_read_only_file_is_deleted_too(self):
        # git's objects are read-only, and Windows will not delete those: --clean stopped at a
        # dependency CMake had cloned into the build folder.
        with tempfile.TemporaryDirectory() as tmp:
            build = Path(tmp) / "release"
            pack = build / "_deps" / "cgltf-src" / ".git" / "objects" / "pack"
            pack.mkdir(parents=True)
            index = pack / "pack-0.idx"
            index.write_bytes(b"x")
            index.chmod(0o444)
            build_port.remove_tree(build)
            self.assertFalse(build.exists())


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

    def test_a_disc_is_kept_for_the_games_translation(self):
        disc = Path("/discs/rac1.iso")
        new = build_port.commands("cmake", "release", self.build, ["rac1-pal"], None, {}, disc=disc)
        self.assertEqual(new[0], ["cmake", "--preset", "release", "-DOPENRAC_RAC1_PAL_RETAIL_IMAGE=" + disc.as_posix()])
        (self.build / "build.ninja").write_text("")
        cache = build_port.parse_cache(CACHE)
        steps = build_port.commands("cmake", "release", self.build, ["rac1-pal"], None, cache, disc=disc)
        self.assertEqual(steps[0], ["cmake", "-S", str(build_port.PORT), "-B", str(self.build),
                                    "-DOPENRAC_RAC1_PAL_RETAIL_IMAGE=" + disc.as_posix()])
        cache["OPENRAC_RAC1_PAL_RETAIL_IMAGE"] = disc.as_posix()
        self.assertEqual(build_port.commands("cmake", "release", self.build, ["rac1-pal"], None, cache, disc=disc),
                         [BUILD])

    def test_an_unfinished_folder_keeps_its_own_compilers(self):
        steps = self.plan(build_port.parse_cache(CACHE), compilers=("other", "other++", "ninja2"), configured=False)
        self.assertEqual(steps[0][:3], ["cmake", "--preset", "release"])
        self.assertIn("-DCMAKE_C_COMPILER=C:/LLVM/bin/clang.exe", steps[0])
        self.assertNotIn("-DCMAKE_C_COMPILER=other", steps[0])

    def test_a_clean_build_keeps_the_folders_choices(self):
        cache = build_port.parse_cache(CACHE)
        cache["OPENRAC_RAC1_PAL_RETAIL_IMAGE"] = "/discs/rac1.iso"
        kept = build_port.kept_choices(cache)
        self.assertEqual(kept, {"OPENRAC_RAC1_PAL_SOURCE": "C:/work/rac1-decomp",
                                "OPENRAC_RAC1_PAL_RETAIL_IMAGE": "/discs/rac1.iso"})
        steps = build_port.commands("cmake", "release", self.build, ["rac1-pal"], None, {}, kept=kept)
        self.assertEqual(steps[0], ["cmake", "--preset", "release", "-DOPENRAC_RAC1_PAL_SOURCE=C:/work/rac1-decomp",
                                    "-DOPENRAC_RAC1_PAL_RETAIL_IMAGE=/discs/rac1.iso"])
        # A --source given with --clean wins over the kept one.
        steps = build_port.commands("cmake", "release", self.build, ["rac1-pal"], Path("/new/rac1"), {}, kept=kept)
        self.assertIn("-DOPENRAC_RAC1_PAL_SOURCE=" + Path("/new/rac1").as_posix(), steps[0])
        self.assertNotIn("-DOPENRAC_RAC1_PAL_SOURCE=C:/work/rac1-decomp", steps[0])


    def test_the_probe_baker_is_kept_and_configured_with_its_embree(self):
        cache = build_port.parse_cache(CACHE)
        cache["OPENRAC_PROBE_BAKER"] = "ON"
        self.assertEqual(build_port.kept_choices(cache)["OPENRAC_PROBE_BAKER"], "ON")
        defines = {"OPENRAC_PROBE_BAKER": "ON", "embree_DIR": "/b/embree/install/lib/cmake/embree-4.4.1"}
        steps = build_port.commands("cmake", "release", self.build, ["rac1-pal"], None, {}, defines=defines,
                                    more=[build_port.BAKER])
        self.assertIn("-DOPENRAC_PROBE_BAKER=ON", steps[0])
        self.assertIn("-Dembree_DIR=/b/embree/install/lib/cmake/embree-4.4.1", steps[0])
        self.assertEqual(steps[-1][-1], "openrac-probe-bake")

    def test_embree_is_pinned_and_built_static_without_tbb(self):
        self.assertEqual(len(build_port.EMBREE_SHA256), 64)
        self.assertIn(build_port.EMBREE_VERSION, build_port.EMBREE_URL)
        self.assertEqual(build_port.EMBREE_OPTIONS["EMBREE_STATIC_LIB"], "ON")
        self.assertEqual(build_port.EMBREE_OPTIONS["EMBREE_TASKING_SYSTEM"], "INTERNAL")


class Stamp(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.build = Path(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def read(self):
        import json
        return json.loads((self.build / "openrac-build.json").read_text(encoding="utf-8"))

    def test_a_build_records_its_version_commit_and_games(self):
        build_port.write_stamp(self.build, ["rac1-pal"])
        stamp = self.read()
        self.assertEqual(stamp["version"], build_port.checkout_version())
        self.assertEqual(stamp["commit"], build_port.checkout_commit())
        self.assertEqual(stamp["games"], ["rac1-pal"])
        # Another game built from the same checkout joins it.
        build_port.write_stamp(self.build, ["rac1-ntsc"])
        self.assertEqual(self.read()["games"], ["rac1-ntsc", "rac1-pal"])

    def test_a_build_from_another_checkout_starts_the_list_again(self):
        (self.build / "openrac-build.json").write_text('{"version": "0.0.1", "commit": null, "games": ["rac1-pal"]}')
        build_port.write_stamp(self.build, ["rac1-ntsc"])
        self.assertEqual(self.read()["games"], ["rac1-ntsc"])


class CommandLine(unittest.TestCase):
    def refused(self, argv):
        with self.assertRaises(SystemExit), contextlib.redirect_stderr(io.StringIO()):
            build_port.main(argv)

    def test_an_unknown_game_is_refused(self):
        self.refused(["rac9-pal"])

    def test_a_source_for_two_games_is_refused(self):
        self.refused(["rac1-pal", "rac1-ntsc", "--source", "."])

    def test_a_disc_for_two_games_or_none_is_refused(self):
        self.refused(["rac1-pal", "rac1-ntsc", "--disc", __file__])
        self.refused(["rac1-pal", "--disc", "/no/such/disc.iso"])


if __name__ == "__main__":
    unittest.main()

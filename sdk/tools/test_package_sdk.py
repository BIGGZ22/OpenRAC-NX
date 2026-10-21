# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests of package_sdk.py: python3 -m unittest discover -s sdk/tools"""
import hashlib
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import package_sdk  # noqa: E402


class PackageSdk(unittest.TestCase):
    def test_version_is_the_interfaces(self):
        v = package_sdk.version()
        self.assertRegex(v, r"^\d+\.\d+\.\d+$")

    def test_archive_is_whole_and_the_same_bytes_each_time(self):
        with tempfile.TemporaryDirectory() as a, tempfile.TemporaryDirectory() as b:
            one = package_sdk.write(Path(a))
            two = package_sdk.write(Path(b))
            self.assertEqual(one.read_bytes(), two.read_bytes())
            top = f"openrac-mod-sdk-{package_sdk.version()}/"
            names = zipfile.ZipFile(one).namelist()
            for need in ["include/openrac/openrac_mod.h", "include/openrac/games/rac1.h",
                         "cmake/OpenRACModConfig.cmake", "cmake/OpenRACModConfigVersion.cmake",
                         "tools/package_mod.py", "template/openrac-mod.json", "README.md",
                         "docs/MODS.md", "LICENSE", "VERSION", "template/.github/workflows/build.yml",
                         "template/tools/fetch_sdk.py", "template/sdk.lock", "template/.gitignore", "template/.gitattributes",
                         "template/LICENSE", "catalogue/rac1/behaviour.json", "include/openrac/mod.hpp",
                         "include/openrac/games/rac1.hpp", "template/src/mod.cpp", "tests/mod_hpp_test.cpp",
                         "tests/CMakeLists.txt",
                         "template-script/openrac-mod.json", "template-script/main.luau",
                         "template-script/sdk.lock", "template-script/tools/fetch_sdk.py",
                         "template-script/.github/workflows/build.yml", "template-script/.gitattributes"]:
                self.assertIn(top + need, names)
            self.assertFalse([n for n in names if "__pycache__" in n or "/build/" in n])
            sha = (Path(a) / (one.name + ".sha256")).read_text().split()[0]
            self.assertEqual(sha, hashlib.sha256(one.read_bytes()).hexdigest())


    def test_the_templates_share_their_files(self):
        # One workflow and one fetch_sdk.py for every mod repository (MODS.md, sections 10 and 11).
        sdk = package_sdk.SDK
        for name in (".github/workflows/build.yml", "tools/fetch_sdk.py", ".gitattributes"):
            self.assertEqual((sdk / "template" / name).read_bytes(), (sdk / "template-script" / name).read_bytes(),
                             name)

    def test_types_go_in_when_made(self):
        names = set(package_sdk.files())
        for f in sorted((package_sdk.SDK / "types").glob("*.d.luau")):
            self.assertIn(f"types/{f.name}", names)

if __name__ == "__main__":
    unittest.main()

"""Exact game/version/level selection and independent profile copies, without disc data."""

from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest
from unittest import mock

from enhanced import profiles


class ProfileTests(unittest.TestCase):
    def test_only_the_exact_game_version_and_level_match(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "profiles/rac1-ntsc/level_01/probe-bake.json"
            source.parent.mkdir(parents=True)
            source.write_bytes(b'{"version":1,"sky_intensity":0.5}\n')
            for game, version, number in (("rac1", "ntsc", 1), ("rac1", "pal", 1),
                                          ("rac2", "ntsc", 1), ("rac1", "ntsc", 2)):
                with self.subTest(game=game, version=version, level=number):
                    directory = root / f"{game}-{version}-{number}"
                    directory.mkdir()
                    with mock.patch.object(profiles, "PROFILE_ROOT", root / "profiles"):
                        profiles.write_bake_profile(SimpleNamespace(game=game, version=version, id=number), directory)
                    copied = directory / "probe-bake.json"
                    self.assertEqual(copied.exists(), (game, version, number) == ("rac1", "ntsc", 1))
                    if copied.exists():
                        self.assertEqual(copied.read_bytes(), source.read_bytes())
                        copied.write_bytes(b'{"version":1}\n')
                        self.assertIn(b'"sky_intensity":0.5', source.read_bytes())

    def test_copy_errors_abort_export_instead_of_using_different_settings(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "profiles/rac1-ntsc/level_01/probe-bake.json"
            source.parent.mkdir(parents=True)
            source.write_text('{"version":1}')
            with mock.patch.object(profiles, "PROFILE_ROOT", root / "profiles"), \
                    mock.patch.object(profiles.shutil, "copyfile", side_effect=OSError("cannot copy profile")):
                with self.assertRaisesRegex(OSError, "cannot copy profile"):
                    profiles.write_bake_profile(SimpleNamespace(game="rac1", version="ntsc", id=1), root)


if __name__ == "__main__":
    unittest.main()

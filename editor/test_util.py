"""The editor's shared helpers (util.py), the boundaries that keep each tool's own errors, and
the disc identities read from games/*/game.json."""

import hashlib
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))

import assets  # noqa: E402
import disc  # noqa: E402
import games  # noqa: E402
import overrides  # noqa: E402
import pack  # noqa: E402
import util  # noqa: E402
import wads  # noqa: E402


class Sha256Tests(unittest.TestCase):
    def test_hex_digest_of_bytes(self):
        self.assertEqual(util.sha256(b""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")
        self.assertEqual(util.sha256(b"openrac"), hashlib.sha256(b"openrac").hexdigest())

    def test_the_tools_use_this_one(self):
        self.assertIs(assets.sha256, util.sha256)
        self.assertIs(overrides.sha256, util.sha256)

    def test_a_files_digest_is_the_digest_of_its_bytes(self):
        # assets.py and overrides.py hash a file through openrac_common's digest.sha256 (in blocks),
        # where they hashed its bytes read whole: the same hex string, including past one block.
        self.assertIs(assets.digest, overrides.digest)
        with tempfile.TemporaryDirectory() as folder:
            for data in (b"", b"openrac", bytes(range(256)) * 4097):
                path = Path(folder) / "file.bin"
                path.write_bytes(data)
                self.assertEqual(assets.digest.sha256(path), util.sha256(data))


class ReadJsonTests(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.dir = Path(self.folder.name)

    def tearDown(self):
        self.folder.cleanup()

    def write(self, name: str, text: str) -> Path:
        path = self.dir / name
        path.write_text(text)
        return path

    def test_reads_any_json_value(self):
        self.assertEqual(util.read_json(self.write("a.json", '{"a": [1, 2]}'), None), {"a": [1, 2]})
        self.assertEqual(util.read_json(str(self.write("b.json", "[3]")), None), [3])

    def test_the_callers_error_for_a_missing_or_broken_file(self):
        def error(path, exc):
            return KeyError(path, type(exc).__name__)

        broken, missing = self.write("broken.json", "{"), self.dir / "missing.json"
        for path, cause in ((broken, "JSONDecodeError"), (missing, "FileNotFoundError"), (str(missing), "FileNotFoundError")):
            with self.subTest(path=path), self.assertRaises(KeyError) as caught:
                util.read_json(path, error)
            self.assertEqual(caught.exception.args, (path, cause))     # PATH as given
            self.assertIsNone(caught.exception.__cause__)
            self.assertTrue(caught.exception.__suppress_context__)

    def test_assets_keeps_its_asset_error(self):
        with self.assertRaises(assets.AssetError) as caught:
            assets.read_json(self.dir / "missing.json")
        self.assertTrue(str(caught.exception).startswith(f"cannot read {self.dir / 'missing.json'}: "))
        self.assertEqual(assets.read_json(self.write("list.json", "[1]")), [1])

    def test_overrides_keeps_its_pack_error_and_wants_an_object(self):
        broken = self.write("broken.json", "{")
        with self.assertRaises(overrides.PackError) as caught:
            overrides.read_json(broken)
        self.assertTrue(str(caught.exception).startswith(f"{broken} cannot be read ("))
        self.assertEqual(overrides.read_json(self.write("list.json", "[1]")), {})
        self.assertEqual(overrides.read_json(str(self.write("object.json", '{"v": 1}'))), {"v": 1})


class F32Tests(unittest.TestCase):
    def test_rounds_to_the_nearest_32_bit_float(self):
        self.assertEqual(util.f32(0.1), struct.unpack("<f", struct.pack("<f", 0.1))[0])
        self.assertNotEqual(util.f32(0.1), 0.1)
        self.assertEqual(util.f32(1.5), 1.5)
        self.assertEqual(util.f32(2 ** 24 + 1), 2 ** 24)

    def test_keeps_minus_zero_unlike_packs(self):
        self.assertEqual(struct.pack("<f", util.f32(-0.0)), struct.pack("<f", -0.0))
        self.assertEqual(pack.f32(-0.0), struct.pack("<f", 0.0))


class CrossTests(unittest.TestCase):
    def test_right_handed(self):
        self.assertEqual(util.cross((1, 0, 0), (0, 1, 0)), (0, 0, 1))
        self.assertEqual(util.cross((0, 1, 0), (1, 0, 0)), (0, 0, -1))
        self.assertEqual(util.cross((1.5, 2.0, -3.0), (4.0, -0.5, 2.0)), (2.5, -15.0, -8.75))


# What disc.py, wads.py and games.py wrote out before they read games/*/game.json: (region, label,
# executable, size, SHA-1) by (game, version), and the versions of each game the editor reads.
OLD_IDENTITIES = {
    ("rac1", "pal"): ("PAL", "PAL v2.00", "SCES_509.16", 1388100, "79956931bd62fafd8d20fa2eae796dbaf2e15e83"),
    ("rac1", "ntsc"): ("NTSC-U", "NTSC-U v1.00", "SCUS_971.99", 1383028, "72fd1de379dafd8f243dceeb3be50980f3fd3cd5"),
    ("rac2", "ntsc"): ("NTSC-U", "NTSC-U v1.01", "SCUS_972.68", 2618684, "bacda860cbd6723142e86db56f197e47e728970f"),
    ("rac3", "ntsc"): ("NTSC-U", "NTSC-U v1.00", "SCUS_973.53", 771008, "601012bb03ef1213ad1253baebae8a071b8a7df7"),
    ("rac4", "ntsc"): ("NTSC-U", "NTSC-U v1.00", "SCUS_974.65", 1692216, "aa91b1c3b9b1a244320c47580b77342ef9856e95"),
}
OLD_VERSIONS = {"rac1": ("pal", "ntsc"), "rac2": ("ntsc",), "rac3": ("ntsc",), "rac4": ("ntsc",)}
FIELDS = ("region", "label", "executable", "executable_size", "executable_sha1")


class DiscIdentityTests(unittest.TestCase):
    def test_game_json_gives_the_old_constants(self):
        for (game, version), old in OLD_IDENTITIES.items():
            with self.subTest(game=game, version=version):
                self.assertEqual(util.disc_identity(game, version), dict(zip(FIELDS, old)))

    def test_the_versions_carry_them(self):
        readers = {("rac1", v.key): v for v in disc.VERSIONS} | {(v.game, v.key): v for v in wads.VERSIONS}
        self.assertEqual(sorted(readers), sorted(OLD_IDENTITIES))
        for key, version in readers.items():
            with self.subTest(version=key):
                self.assertEqual(tuple(getattr(version, f) for f in FIELDS), OLD_IDENTITIES[key])
        self.assertEqual((disc.PAL.key, disc.PAL.gameplay, disc.PAL.normal_table), ("pal", "gameplay_pal", 0x165600))
        self.assertEqual((disc.NTSC.key, disc.NTSC.gameplay, disc.NTSC.normal_table), ("ntsc", "gameplay_ntsc", 0x165500))
        self.assertEqual([v.level_header for v in wads.VERSIONS], [0x60, 0x60, 0xc68])

    def test_games_versions_are_the_old_ones(self):
        self.assertEqual(games.VERSIONS, OLD_VERSIONS)
        self.assertEqual(list(games.VERSIONS["rac1"]), ["pal", "ntsc"])

    def test_unknown_games_and_versions(self):
        with self.assertRaises(FileNotFoundError):
            util.disc_identity("rac9", "pal")
        with self.assertRaises(KeyError):
            util.disc_identity("rac2", "pal")


if __name__ == "__main__":
    unittest.main()

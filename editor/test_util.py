"""The editor's shared helpers (util.py), and the boundaries that keep each tool's own errors."""

import hashlib
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))

import assets  # noqa: E402
import overrides  # noqa: E402
import pack  # noqa: E402
import util  # noqa: E402


class Sha256Tests(unittest.TestCase):
    def test_hex_digest_of_bytes(self):
        self.assertEqual(util.sha256(b""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")
        self.assertEqual(util.sha256(b"openrac"), hashlib.sha256(b"openrac").hexdigest())

    def test_the_tools_use_this_one(self):
        self.assertIs(assets.sha256, util.sha256)
        self.assertIs(overrides.sha256, util.sha256)


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


if __name__ == "__main__":
    unittest.main()

"""tools/extractor.py on synthetic disc images only, never a real disc:
python3 -m unittest discover -s tools"""

import hashlib
import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import extractor
from test_openrac import image

BOOT = b"\x7fELF" + bytes(3000)
CNF = b"BOOT2 = cdrom0:\\SCES_999.99;1\r\nVER = 1.00\r\n"


def fake_builds(boot_sha1: str | None, boot_sha256: str | None = None):
    """A game.json that knows one build, SCES_999.99 of game "rac9", by its boot's SHA-1 (or SHA-256 alone)."""
    game = {"id": "rac9", "title": "Nine"}
    boot = {"sha1": boot_sha1} if boot_sha1 else {"sha256": boot_sha256}
    version = {"serial": "SCES_999.99", "region": "PAL", "boot": boot, "disc": {}}
    return lambda wanted=None: {"SCES_999.99": (game, "pal", version)} if wanted in (None, "rac9") else {}


class ExtractTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self.tmp.name)
        self.iso = self.dir / "my disc.iso"
        self.iso.write_bytes(image({"SYSTEM.CNF": CNF, "SCES_999.99": BOOT, "IOPRP.IMG": b"iop"}))
        self.proj = self.dir / "proj"

    def tearDown(self):
        self.tmp.cleanup()

    def run_extract(self, sha1=None):
        sha1 = sha1 or hashlib.sha1(BOOT).hexdigest()
        with mock.patch.object(extractor, "builds", fake_builds(sha1)):
            return extractor.extract(self.iso, "rac9", self.proj, minimum=0)

    def test_extracts_and_validates_a_known_build(self):
        info = self.run_extract()
        out = self.proj / "iso_data" / "rac9"
        self.assertEqual((out / "SCES_999.99").read_bytes(), BOOT)
        self.assertEqual((out / "SYSTEM.CNF").read_bytes(), CNF)
        self.assertEqual((out / "disc.iso").read_bytes(), self.iso.read_bytes())
        written = json.loads((out / "buildinfo.json").read_text())
        self.assertEqual(written, [info])
        self.assertEqual(info["serial"], "SCES_999.99")
        self.assertEqual(info["version"], "rac9/pal")
        self.assertEqual(info["files"], 3)
        self.assertFalse((self.proj / "iso_data" / "_temp").exists())

    def test_a_second_extraction_replaces_the_first(self):
        self.run_extract()
        (self.proj / "iso_data" / "rac9" / "stale").write_text("x")
        self.run_extract()
        self.assertFalse((self.proj / "iso_data" / "rac9" / "stale").exists())

    def test_refuses_an_unknown_revision_and_keeps_nothing(self):
        with self.assertRaises(extractor.ExtractError) as caught:
            self.run_extract(sha1="0" * 40)
        self.assertEqual(caught.exception.code, 4002)
        self.assertFalse((self.proj / "iso_data" / "rac9").exists())
        self.assertFalse((self.proj / "iso_data" / "_temp").exists())

    def test_a_build_pinned_by_sha256_alone(self):
        good = hashlib.sha256(BOOT).hexdigest()
        with mock.patch.object(extractor, "builds", fake_builds(None, good)):
            self.assertEqual(extractor.extract(self.iso, "rac9", self.proj, minimum=0)["serial"], "SCES_999.99")
        with mock.patch.object(extractor, "builds", fake_builds(None, "0" * 64)):
            with self.assertRaises(extractor.ExtractError) as caught:
                extractor.extract(self.iso, "rac9", self.proj, minimum=0)
        self.assertEqual(caught.exception.code, 4002)

    def test_refuses_another_game(self):
        with mock.patch.object(extractor, "builds", fake_builds(hashlib.sha1(BOOT).hexdigest())):
            with self.assertRaises(extractor.ExtractError) as caught:
                extractor.extract(self.iso, "rac8", self.proj, minimum=0)
        self.assertEqual(caught.exception.code, 4001)

    def test_refuses_what_is_not_a_disc_image(self):
        cases = [(self.dir / "notes.txt", b"hello", 4040), (self.dir / "blank.iso", bytes(40 * 2048), 4020)]
        for path, data, code in cases:
            path.write_bytes(data)
            with self.assertRaises(extractor.ExtractError) as caught:
                extractor.extract(path, "rac9", self.proj, minimum=0)
            self.assertEqual(caught.exception.code, code, path.name)
        with self.assertRaises(extractor.ExtractError) as caught:
            extractor.extract(self.iso, "rac9", self.proj)
        self.assertEqual(caught.exception.code, 4041)

    def test_no_boot_executable(self):
        self.iso.write_bytes(image({"README.TXT": b"hello"}))
        with self.assertRaises(extractor.ExtractError) as caught:
            self.run_extract()
        self.assertEqual(caught.exception.code, 4000)

    def test_the_later_steps_do_not_exist_yet(self):
        code = extractor.main([str(self.iso), "--game", "rac1", "--decompile", "--proj-path", str(self.proj)])
        self.assertEqual(code, 1)

    def test_contents_hash_ignores_order(self):
        a = extractor.contents_hash({"A": "1", "B": "2"})
        self.assertEqual(a, extractor.contents_hash({"B": "2", "A": "1"}))
        self.assertNotEqual(a, extractor.contents_hash({"A": "1", "B": "3"}))

    def test_every_real_build_is_known(self):
        serials = extractor.builds()
        for serial in ("SCES_509.16", "SCUS_971.99", "SCUS_972.68", "SCUS_973.53", "SCUS_974.65"):
            self.assertIn(serial, serials)
            self.assertEqual(len(serials[serial][2]["boot"]["sha1"]), 40)
        # Size Matters' project pins its boot by SHA-256 only.
        self.assertEqual(len(serials["SCUS_976.15"][2]["boot"]["sha256"]), 64)


if __name__ == "__main__":
    unittest.main()

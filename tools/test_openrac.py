"""tools/openrac.py on synthetic data only, never a disc:
python3 -m unittest discover -s tools"""

import json
import struct
import tempfile
import unittest
from pathlib import Path

import openrac

SECTOR = openrac.SECTOR


def record(name: bytes, lba: int, size: int, directory: bool = False) -> bytes:
    """One ISO 9660 directory record."""
    # length, extended attributes, extent and size (little-endian halves; the
    # big-endian halves stay zero), 7 date bytes, flags, unit size, gap, volume.
    body = struct.pack("<BBIIII7sBBBHHB", 0, 0, lba, 0, size, 0, bytes(7),
                       2 if directory else 0, 0, 0, 1, 1, len(name)) + name
    body = bytes([33 + len(name) + (len(name) + 1) % 2]) + body[1:]
    return body + b"\0" * (body[0] - len(body))


def image(files: dict[str, bytes]) -> bytes:
    """A minimal ISO 9660 image: the volume descriptor, a root directory, FILES after it."""
    root_lba, first = 18, 19
    entries, blobs, lba = [record(b"\0", root_lba, SECTOR, True), record(b"\1", root_lba, SECTOR, True)], [], first
    for name, data in files.items():
        entries.append(record(name.encode() + b";1", lba, len(data)))
        blobs.append((lba, data))
        lba += (len(data) + SECTOR - 1) // SECTOR
    out = bytearray(lba * SECTOR)
    pvd = bytearray(SECTOR)
    pvd[0:6] = b"\1CD001"
    pvd[156:156 + 34] = record(b"\0", root_lba, SECTOR, True)
    out[16 * SECTOR:17 * SECTOR] = pvd
    directory = b"".join(entries)
    out[root_lba * SECTOR:root_lba * SECTOR + len(directory)] = directory
    for at, data in blobs:
        out[at * SECTOR:at * SECTOR + len(data)] = data
    return bytes(out)


class DiscTests(unittest.TestCase):
    def test_finds_the_boot_executable(self):
        boot = b"\x7fELF" + bytes(3000)
        with tempfile.TemporaryDirectory() as tmp:
            iso = Path(tmp) / "any name.iso"
            iso.write_bytes(image({"SYSTEM.CNF": b"BOOT2 = cdrom0:\\SCES_509.16;1\r\nVER = 2.00\r\n",
                                   "SCES_509.16": boot}))
            self.assertEqual(openrac.boot_serial(iso), "SCES_509.16")
            self.assertEqual(openrac.read_boot(iso, "SCES_509.16"), boot)

    def test_not_a_ps2_disc(self):
        with tempfile.TemporaryDirectory() as tmp:
            iso = Path(tmp) / "blank.iso"
            iso.write_bytes(bytes(20 * SECTOR))
            self.assertIsNone(openrac.boot_serial(iso))
            iso.write_bytes(image({"README.TXT": b"hello"}))
            self.assertIsNone(openrac.boot_serial(iso))

    def test_checksums(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "f"
            path.write_bytes(b"abc")
            sums = openrac.checksums(path, full=True)
        self.assertEqual(sums["sha1"], "a9993e364706816aba3e25717850c26c9cd0d89d")
        self.assertEqual(sums["crc32"], "352441c2")
        self.assertEqual(sums["size"], 3)


class ManifestTests(unittest.TestCase):
    def test_every_manifest_is_complete(self):
        serials = set()
        for game, name, version in openrac.versions():
            for key in ("region", "serial", "disc", "boot", "target"):
                self.assertIn(key, version, f"{game['id']}/{name}")
            # Size and SHA-256 pin a build; the other checksums follow once someone with the disc
            # records them (rac5's project pins its boot by SHA-256 alone).
            for key in ("size", "sha256"):
                self.assertIn(key, version["disc"], f"{game['id']}/{name}")
                self.assertIn(key, version["boot"], f"{game['id']}/{name}")
            self.assertIn("sha1", version["disc"], f"{game['id']}/{name}")
            if game["id"] != "rac5":
                for key in ("crc32", "md5", "sha1"):
                    self.assertIn(key, version["disc"], f"{game['id']}/{name}")
                    self.assertIn(key, version["boot"], f"{game['id']}/{name}")
            self.assertNotIn(version["serial"], serials)
            serials.add(version["serial"])
            if version.get("source"):
                self.assertTrue((openrac.ROOT / openrac.vdir(game, name)).is_dir(), f"{game['id']}/{name}")
                self.assertRegex(version["source"]["commit"], r"^[0-9a-f]{40}$")


class ProgressTests(unittest.TestCase):
    def test_objdiff_report(self):
        with tempfile.TemporaryDirectory(dir=openrac.ROOT / "tools") as tmp:
            path = Path(tmp) / "report.json"
            path.write_text(json.dumps({"measures": {"matched_code": "250", "total_code": "1000",
                                                     "matched_functions": 3, "total_functions": 10,
                                                     "fuzzy_match_percent": 30.5}}))
            m = openrac.measure({"progress": {"report": str(path.relative_to(openrac.ROOT)), "format": "objdiff"}})
        self.assertEqual((m["matched_code"], m["total_code"], m["percent"], m["fuzzy_percent"]), (250, 1000, 25.0, 30.5))

    def test_missing_report_and_no_progress(self):
        self.assertEqual(openrac.measure({"progress": {"report": "nowhere.json", "format": "objdiff"}}),
                         {"missing": "nowhere.json"})
        self.assertIsNone(openrac.measure({}))

    def test_progress_table_uses_bytematch_badges(self):
        game = {"id": "rac1", "title": "Ratchet & Clank",
                "versions": {"pal": {"dir": "games/rac1"}}}
        version = {"serial": "SCES_509.16", "bytematch": "OpenRAC/rac1-decomp", "source": {}}
        measured = {"percent": 25.0, "matched_code": 250, "total_code": 1000,
                    "matched_functions": 3, "total_functions": 10,
                    "fuzzy_percent": 30.5, "date": "2026-10-22"}
        table = openrac.render([(game, "pal", version, measured)], dates=False, badges=True)
        self.assertIn("[![Code](https://bytematch.dev/OpenRAC/rac1-decomp.svg?"
                      "mode=shield&label=Code&measure=matched_code_percent&version=SCES_509.16)]"
                      "(https://bytematch.dev/OpenRAC/rac1-decomp/SCES_509.16)", table)
        self.assertIn("label=Functions&measure=matched_functions", table)
        self.assertIn("label=Fuzzy&measure=fuzzy_match_percent", table)

    def test_progress_table_falls_back_without_bytematch(self):
        game = {"id": "rac5", "title": "Ratchet & Clank: Size Matters", "versions": {"ntsc": {}}}
        version = {"serial": "SCUS_976.15"}
        measured = {"percent": 25.0, "matched_code": 250, "total_code": 1000,
                    "matched_functions": 3, "total_functions": 10,
                    "fuzzy_percent": 30.5, "date": "2026-10-22"}
        table = openrac.render([(game, "ntsc", version, measured)], dates=False, badges=True)
        self.assertIn("25.00% (250 / 1,000 bytes)", table)
        self.assertNotIn("bytematch.dev", table)

    def test_progress_table_badges_code_without_a_report(self):
        game = {"id": "rac5", "title": "Ratchet & Clank: Size Matters", "versions": {"ntsc": {}}}
        version = {"serial": "SCUS_976.15", "bytematch": "OpenRAC/rac5-sm-decomp"}
        table = openrac.render([(game, "ntsc", version, None)], dates=False, badges=True)
        self.assertIn("label=Code&measure=matched_code_percent&version=SCUS_976.15", table)
        self.assertIn("| – | – |", table)


if __name__ == "__main__":
    unittest.main()

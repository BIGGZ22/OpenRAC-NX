"""Synthetic ISO9660 tests; no game bytes."""

from __future__ import annotations

import hashlib
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from disc import IsoDisc, SECTOR_SIZE, verify_disc  # noqa: E402


def record(name: bytes, extent: int, size: int, *, directory: bool = False) -> bytes:
    length = 33 + len(name) + (len(name) % 2 == 0)
    data = bytearray(length)
    data[0] = length
    struct.pack_into("<I", data, 2, extent)
    struct.pack_into(">I", data, 6, extent)
    struct.pack_into("<I", data, 10, size)
    struct.pack_into(">I", data, 14, size)
    data[25] = 2 if directory else 0
    data[28] = data[31] = 1
    data[32] = len(name)
    data[33:33 + len(name)] = name
    return bytes(data)


class IsoTests(unittest.TestCase):
    def make_image(self, folder: Path, *, truncate: bool = False) -> tuple[Path, dict[str, object]]:
        system = b"BOOT2 = cdrom0:\\SCUS_976.15;1\nVER = 1.00\nVMODE = NTSC\n"
        boot = b"synthetic boot bytes"
        data = bytearray(24 * SECTOR_SIZE)
        pvd = 16 * SECTOR_SIZE
        data[pvd:pvd + 7] = b"\x01CD001\x01"
        data[pvd + 40:pvd + 72] = b"TEST_DISC".ljust(32, b" ")
        struct.pack_into("<I", data, pvd + 80, 24)
        data[pvd + 156:pvd + 190] = record(b"\x00", 20, SECTOR_SIZE, directory=True)
        directory = (record(b"\x00", 20, SECTOR_SIZE, directory=True)
                     + record(b"\x01", 20, SECTOR_SIZE, directory=True)
                     + record(b"SYSTEM.CNF;1", 21, len(system))
                     + record(b"SCUS_976.15;1", 22, len(boot)))
        data[20 * SECTOR_SIZE:20 * SECTOR_SIZE + len(directory)] = directory
        data[21 * SECTOR_SIZE:21 * SECTOR_SIZE + len(system)] = system
        data[22 * SECTOR_SIZE:22 * SECTOR_SIZE + len(boot)] = boot
        image = folder / "test.iso"
        image.write_bytes(data[:-SECTOR_SIZE] if truncate else data)
        on_disk = image.read_bytes()
        target: dict[str, object] = {
            "boot_file": "SCUS_976.15", "system_cnf_version": "1.00",
            "iso_size": len(data), "iso_sha1": hashlib.sha1(data).hexdigest(),
            "iso_sha256": hashlib.sha256(data).hexdigest(),
            "boot_size": len(boot), "boot_sha256": hashlib.sha256(boot).hexdigest(),
        }
        self.assertEqual(len(on_disk), len(data) - (SECTOR_SIZE if truncate else 0))
        return image, target

    def test_complete_disc_and_hash_gate(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            image, target = self.make_image(Path(tmp))
            with IsoDisc(image) as disc:
                info = disc.inspect()
                self.assertEqual((info["boot_file"], info["version"], info["file_count"]),
                                 ("SCUS_976.15", "1.00", 2))
                self.assertEqual(info["missing_sectors"], 0)
                self.assertTrue(info["all_recorded_extents_present"])
                verify_disc(disc, target)
                self.assertEqual(disc.file_bytes("SCUS_976.15"), b"synthetic boot bytes")

    def test_short_image_is_detected_even_with_all_files_present(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            image, target = self.make_image(Path(tmp), truncate=True)
            with IsoDisc(image) as disc:
                info = disc.inspect()
                self.assertEqual(info["missing_sectors"], 1)
                self.assertTrue(info["all_recorded_extents_present"])
                with self.assertRaisesRegex(ValueError, "length"):
                    verify_disc(disc, target)

    def test_invalid_descriptor_and_path(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            image, _target = self.make_image(Path(tmp))
            with IsoDisc(image) as disc:
                with self.assertRaises(ValueError):
                    disc.find("../SCUS_976.15")
                with self.assertRaises(ValueError):
                    disc.read(disc.size - 1, 2)
            with image.open("r+b") as output:
                output.seek(16 * SECTOR_SIZE)
                output.write(b"bad")
            with self.assertRaisesRegex(ValueError, "primary volume"):
                IsoDisc(image)

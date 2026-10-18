"""Synthetic ELF checks for local compiler witnesses; no game data."""

from __future__ import annotations

import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from compare_boot_slice import compare, text_slice  # noqa: E402


def make_elf(path: Path, address: int, code: bytes) -> None:
    image = bytearray(256)
    image[:16] = b"\x7fELF\x01\x01\x01" + bytes(9)
    struct.pack_into("<HHIIIIIHHHHHH", image, 16,
                     2, 8, 1, address, 52, 96, 0x20924001,
                     52, 32, 1, 40, 3, 2)
    struct.pack_into("<IIIIIIII", image, 52,
                     1, 80, address, address, len(code), len(code), 5, 4)
    image[80:80 + len(code)] = code
    names = b"\0.text\0.shstrtab\0"
    struct.pack_into("<IIIIIIIIII", image, 136,
                     1, 1, 6, address, 80, len(code), 0, 0, 4, 0)
    struct.pack_into("<IIIIIIIIII", image, 176,
                     7, 3, 0, 0, 216, len(names), 0, 0, 1, 0)
    image[216:216 + len(names)] = names
    path.write_bytes(image)


class CompareBootSliceTests(unittest.TestCase):
    def test_equal_and_different_slices(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            boot, candidate = root / "boot", root / "candidate"
            make_elf(boot, 0x100000, b"\x01\x02\x03\x04\x05\x06\x07\x08")
            make_elf(candidate, 0x100000, b"\x01\x02\x03\x04\x05\x06\x07\x08")
            self.assertEqual(compare(boot, candidate, 0x100004, 4)[:2], (True, 4))
            make_elf(candidate, 0x100000, b"\x01\x02\x03\x04\x05\x00\x07\x08")
            self.assertEqual(compare(boot, candidate, 0x100004, 4)[:2], (False, 3))

    def test_rejects_wrong_placement_and_pin(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            boot, candidate = root / "boot", root / "candidate"
            make_elf(boot, 0x100000, b"\x00\x00\x00\x00")
            make_elf(candidate, 0x200000, b"\x00\x00\x00\x00")
            with self.assertRaisesRegex(ValueError, "outside executable"):
                compare(boot, candidate, 0x100000, 4)
            with self.assertRaisesRegex(ValueError, "pinned USA"):
                compare(boot, candidate, 0x100000, 4, "0" * 64)
            with self.assertRaisesRegex(ValueError, "size must be positive"):
                text_slice(boot, 0x100000, 0)


if __name__ == "__main__":
    unittest.main()

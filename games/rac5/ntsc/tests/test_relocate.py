"""Synthetic relocation tests; no retail bytes or memory dumps."""

from __future__ import annotations

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from rel import HEADER, MAGIC, RELOCATION, SYMBOL  # noqa: E402
from relocate import ROOT, output_path, patched_word, relocate_module  # noqa: E402


def sample() -> bytes:
    data = bytearray(0x200)
    HEADER.pack_into(data, 0, MAGIC, 0x1C0, 1, 0x1CC, 3, 0x180, 0, 0,
                     0x1F0, 0, 0x80, len(data), 0, 0x1F0, 0x1F0)
    data[0x17F] = 0
    data[0x180:0x189] = b"external\0"
    data[0x190:0x196] = b"entry\0"
    SYMBOL.pack_into(data, 0x1CC, 0x17F, 0, 0, 0, 0)
    SYMBOL.pack_into(data, 0x1D8, 0x180, 0, 0, 1, 0)
    SYMBOL.pack_into(data, 0x1E4, 0x190, 0x100, 0, 2, 0)
    struct.pack_into("<IIIII", data, 0x100,
                     0x3C020000, 0x24421234, 0x0C000000, 0x00000120, 0x0C000000)
    RELOCATION.pack_into(data, 0x1C0, 0x110, (1 << 8) | 4, 0x54)
    data[0x1F0:0x1FD] = b"\x01" + struct.pack("<I", 0x100) + b"\x04\x06\x07" + b"\0" * 5
    return bytes(data)


class RelocateTests(unittest.TestCase):
    def test_both_relocation_tables_and_symbol_metadata(self) -> None:
        image, missing = relocate_module("LVL/TEST.REL", sample(), 0x8000,
                                         {"external": 0x20000})
        self.assertEqual(missing, ())
        self.assertEqual(struct.unpack_from("<IIIII", image, 0x100),
                         (0x3C020001, 0x24429234, 0x0C002000, 0x8120, 0x0C008015))
        self.assertEqual(struct.unpack_from("<I", image, 0x1CC + 4)[0], 0x8000)
        self.assertEqual(struct.unpack_from("<I", image, 0x1D8 + 4)[0], 0x20000)
        self.assertEqual(struct.unpack_from("<I", image, 0x1E4 + 4)[0], 0x8100)
        self.assertEqual(image[0x1E4 + 11], 1)
        self.assertEqual(struct.unpack_from("<I", image, 0x1C0)[0], 0x8110)
        self.assertEqual(struct.unpack_from("<I", image, 0x34)[0], 0x81F0)

    def test_missing_import_and_alignment(self) -> None:
        image, missing = relocate_module("LVL/TEST.REL", sample(), 0x8000, {})
        self.assertEqual(missing, ("external",))
        self.assertEqual(struct.unpack_from("<I", image, 0x1D8 + 4)[0], 0)
        with self.assertRaisesRegex(ValueError, "alignment"):
            relocate_module("LVL/TEST.REL", sample(), 0x8004, {})

    def test_mips_relocation_word_forms(self) -> None:
        self.assertEqual(patched_word(0x3C020000, 5, 0x18000), 0x3C020002)
        self.assertEqual(patched_word(0x24420000, 6, 0x12345678), 0x24425678)
        self.assertEqual(patched_word(0x00000000, 2, 0x12345678), 0x12345678)
        with self.assertRaisesRegex(ValueError, "unsupported"):
            patched_word(0, 99, 0)

    def test_game_bytes_must_stay_under_ignored_build(self) -> None:
        destination = output_path("TEST.REL", 0x8000, None)
        self.assertTrue(destination.is_relative_to((ROOT / "build").resolve()))
        with self.assertRaisesRegex(ValueError, "ignored build"):
            output_path("TEST.REL", 0x8000, ROOT / "README.md")


if __name__ == "__main__":
    unittest.main()

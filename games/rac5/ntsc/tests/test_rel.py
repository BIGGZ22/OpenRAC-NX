"""Synthetic SNR2 metadata tests; no game bytes."""

from __future__ import annotations

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from rel import (HEADER, MAGIC, RELOCATION, SYMBOL, iter_packed_relocations,
                 packed_relocations_by_site, packed_relocation_types,
                 unpack_relocation,
                 parse_module, render_exports, render_modules)  # noqa: E402


def sample() -> bytearray:
    data = bytearray(0xE0)
    HEADER.pack_into(data, 0, MAGIC, 0xB0, 1, 0xBC, 2, 0xAC, 0, 0,
                     0xD4, 0, 0x80, len(data), 0, 0xD4, 0xD4)
    data[0x80:0x80 + len(b"external\0")] = b"external\0"
    data[0x90:0x90 + len(b"lib_main__FiPPc\0")] = b"lib_main__FiPPc\0"
    RELOCATION.pack_into(data, 0xB0, 0xA4, 4, 0)
    SYMBOL.pack_into(data, 0xBC, 0x80, 0, 0, 1, 0)
    SYMBOL.pack_into(data, 0xC8, 0x90, 0xA0, 0, 2, 0)
    return data


class RelTests(unittest.TestCase):
    def test_parse_and_render_metadata(self) -> None:
        module = parse_module("LVL/TEST.REL", bytes(sample()))
        self.assertEqual(module.relocation_count, 1)
        self.assertEqual(module.symbol_count, 2)
        self.assertEqual(module.imports, ("external",))
        self.assertEqual(module.import_references, (("external", 1),))
        self.assertEqual(module.relocation_types, ((4, 1),))
        self.assertEqual(module.packed_relocation_types, ())
        self.assertEqual([(x.name, x.address) for x in module.exports],
                         [("lib_main__FiPPc", 0xA0)])
        self.assertIn("LVL/TEST.REL\t224\t", render_modules([module]))
        self.assertIn("LVL/TEST.REL\tlib_main__FiPPc\t0x000000A0", render_exports([module]))

    def test_rejects_bad_magic_and_size(self) -> None:
        blob = sample()
        blob[0] = 0
        with self.assertRaisesRegex(ValueError, "magic"):
            parse_module("test", bytes(blob))
        with self.assertRaisesRegex(ValueError, "file size"):
            parse_module("test", bytes(sample()[:-1]))

    def test_rejects_bad_relocation_index_and_symbol_name(self) -> None:
        blob = sample()
        blob[0xB0 + 5] = 2
        with self.assertRaisesRegex(ValueError, "relocation"):
            parse_module("test", bytes(blob))
        blob = sample()
        struct.pack_into("<I", blob, 0xC8, 0xD8)
        with self.assertRaisesRegex(ValueError, "symbol"):
            parse_module("test", bytes(blob))

    def test_named_relocation_addend_is_last_word(self) -> None:
        blob = sample()
        RELOCATION.pack_into(blob, 0xB0, 0xA4, 4, 0x12345)
        self.assertEqual(blob[0xB6:0xB8], b"\0\0")
        self.assertEqual(blob[0xB8:0xBC], struct.pack("<i", 0x12345))
        self.assertEqual(unpack_relocation(blob, 0xB0)[3], 0x12345)
        self.assertEqual(parse_module("test", bytes(blob)).relocation_count, 1)

    def test_named_relocation_symbol_index_uses_24_bits(self) -> None:
        blob = sample()
        RELOCATION.pack_into(blob, 0xB0, 0xA4, (257 << 8) | 4, 0)
        self.assertEqual(unpack_relocation(blob, 0xB0), (0xA4, 4, 257, 0))
        with self.assertRaisesRegex(ValueError, "relocation"):
            parse_module("test", bytes(blob))

    def test_packed_local_fixups_and_hi16_pair(self) -> None:
        data = bytearray(0x200)
        # Absolute HI16 at 0x100, then relative LO16, 26-bit and 32-bit sites.
        stream = b"\x01" + struct.pack("<I", 0x100) + b"\x04\x06\x07" + b"\0" * 5
        data[0x180:0x180 + len(stream)] = stream
        self.assertEqual(packed_relocation_types(bytes(data), 0x180, 0x180),
                         ((0, 1), (1, 1), (2, 1), (3, 1)))

    def test_interval_index_sorts_backward_stream_without_changing_hi_pair(self) -> None:
        data = bytearray(0x200)
        stream = (b"\x01" + struct.pack("<I", 0x100) +
                  b"\x00" + struct.pack("<I", 0x120) +
                  b"\x02" + struct.pack("<I", 0x108) +
                  b"\x00" + struct.pack("<I", 0))
        data[0x180:0x180 + len(stream)] = stream
        original = list(iter_packed_relocations(data, 0x180, 0x180))
        indexed = packed_relocations_by_site(data, 0x180, 0x180)
        self.assertEqual([item.site for item in original], [0x100, 0x120, 0x108])
        self.assertEqual([item.site for item in indexed], [0x100, 0x108, 0x120])
        self.assertEqual(indexed[0].low_site, 0x120)

    def test_packed_fixups_reject_bad_sites_and_unpaired_hi16(self) -> None:
        data = bytearray(0x190)
        data[0x180:0x189] = b"\x01" + struct.pack("<I", 0x100) + b"\0" * 4
        with self.assertRaisesRegex(ValueError, "no following LO16"):
            packed_relocation_types(bytes(data), 0x180, 0x180)
        data[0x180:0x185] = b"\x03" + struct.pack("<I", 0x102)
        with self.assertRaisesRegex(ValueError, "invalid packed relocation site"):
            packed_relocation_types(bytes(data), 0x180, 0x180)

    def test_packed_fixups_require_terminator_and_zero_padding(self) -> None:
        data = bytearray(0x186)
        data[0x180:0x185] = b"\x03" + struct.pack("<I", 0x100)
        data[0x185] = 4
        with self.assertRaisesRegex(ValueError, "unterminated"):
            packed_relocation_types(bytes(data), 0x180, 0x180)
        data = bytearray(0x18B)
        data[0x180:0x185] = b"\x03" + struct.pack("<I", 0x100)
        data[0x18A] = 1
        with self.assertRaisesRegex(ValueError, "nonzero data"):
            packed_relocation_types(bytes(data), 0x180, 0x180)
        data[0x180:0x185] = b"\x03" + struct.pack("<I", 0x180)
        with self.assertRaisesRegex(ValueError, "invalid packed relocation site"):
            packed_relocation_types(bytes(data), 0x180, 0x180)

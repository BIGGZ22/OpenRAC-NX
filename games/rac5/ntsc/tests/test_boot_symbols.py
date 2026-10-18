"""Synthetic linked-address SNR2 tests; no retail symbols or bytes."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from boot_symbols import (BootSymbol, parse_sndata, render_code_intervals,
                          render_symbols)  # noqa: E402
from rel import HEADER, MAGIC, RELOCATION, SYMBOL  # noqa: E402


class BootSymbolTests(unittest.TestCase):
    def make_table(self) -> bytearray:
        base = 0x1000
        data = bytearray(0xC0)
        HEADER.pack_into(data, 0, MAGIC, base + 0x80, 1, base + 0x8C, 2,
                         0, 0, 0, base + 0xA4, 0, 0, 0xA4, 0, 0, 0)
        data[0x40:0x44] = b"foo\0"
        data[0x48:0x4C] = b"bar\0"
        RELOCATION.pack_into(data, 0x80, 0x2000, 4, 0)
        SYMBOL.pack_into(data, 0x8C, base + 0x40, 0x2004, 0, 2, 0)
        SYMBOL.pack_into(data, 0x98, base + 0x48, 0, 0, 1, 0)
        return data

    def test_linked_addresses_and_unresolved_symbol(self) -> None:
        sections: list[dict[str, object]] = [
            {"name": ".text", "virtual_address": "0x00002000", "size": 0x100, "flags": 6}
        ]
        symbols = parse_sndata(bytes(self.make_table()), 0x1000, sections)
        self.assertEqual([(s.name, s.section) for s in symbols],
                         [("foo", ".text"), ("bar", "unresolved")])
        self.assertIn("foo\t0x00002004\t2\t.text", render_symbols(symbols))

    def test_rejects_out_of_range_symbol_table(self) -> None:
        data = self.make_table()
        data[12:16] = (0x2000).to_bytes(4, "little")
        with self.assertRaisesRegex(ValueError, "table bounds"):
            parse_sndata(bytes(data), 0x1000, [])

    def test_code_label_bounds_keep_aliases_and_exclude_data_markers(self) -> None:
        report = {"sections": [{"name": ".text", "virtual_address": "0x00002000",
                                "size": 0x30}]}
        symbols = [BootSymbol("first", 0x2004, 2, ".text"),
                   BootSymbol("alias", 0x2004, 3, ".text"),
                   BootSymbol("second", 0x2010, 2, ".text"),
                   BootSymbol("not_code", 0x2000, 4, ".text")]
        rendered = render_code_intervals(report, symbols)
        self.assertIn("0x00002004\t0x00002010\t12\t2,3\talias | first\tnext_label", rendered)
        self.assertIn("0x00002010\t0x00002030\t32\t2\tsecond\ttext_end", rendered)
        self.assertNotIn("not_code", rendered)

"""Synthetic VU overlay metadata tests; no retail overlay bytes."""

from __future__ import annotations

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from boot_dvp import parse_records, rebuild_strings  # noqa: E402


class BootDvpTests(unittest.TestCase):
    def test_duplicate_section_name_with_distinct_string_offsets(self) -> None:
        strings = b"\0overlay\0\0overlay\0"
        table = struct.pack("<III", 1, 0x2000, 0) + struct.pack("<III", 10, 0x2040, 0x800)
        rows = parse_records(table, strings, {"overlay": 32})
        self.assertEqual([(row.name_offset, row.lma, row.vma, row.size) for row in rows],
                         [(1, 0x2000, 0, 32), (10, 0x2040, 0x800, 32)])
        self.assertEqual(rebuild_strings(rows, len(strings)), strings)

    def test_rejects_unknown_section_and_partial_record(self) -> None:
        with self.assertRaisesRegex(ValueError, "partial record"):
            parse_records(bytes(11), b"\0x\0", {"x": 4})
        with self.assertRaisesRegex(ValueError, "no section"):
            parse_records(struct.pack("<III", 1, 0x2000, 0), b"\0x\0", {})


if __name__ == "__main__":
    unittest.main()

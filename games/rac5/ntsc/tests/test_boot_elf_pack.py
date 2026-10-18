"""Verify the public ELF layout can regenerate metadata without game bytes."""

from __future__ import annotations

import copy
import hashlib
import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from boot_elf_pack import CATALOGUE, dvp_rows, pack_file, string_table  # noqa: E402


class BootElfPackTests(unittest.TestCase):
    def test_recreates_public_container_around_synthetic_load(self) -> None:
        layout = json.loads(CATALOGUE.read_text(encoding="utf-8"))
        program = layout["program_headers"][0]
        image = pack_file(layout, dvp_rows(), bytes(program[4]))
        self.assertEqual(len(image), layout["file_size"])
        self.assertEqual(image[:16], bytes.fromhex(layout["elf_ident_hex"]))
        self.assertEqual(image[program[1]:program[1] + program[4]], bytes(program[4]))
        self.assertNotEqual(hashlib.sha256(image).hexdigest(),
                            layout["target_sha256"])

    def test_rejects_stale_name_offset_and_load_size(self) -> None:
        layout = json.loads(CATALOGUE.read_text(encoding="utf-8"))
        load = bytes(layout["program_headers"][0][4])
        with self.assertRaisesRegex(ValueError, "load size"):
            pack_file(layout, dvp_rows(), load[:-1])
        changed = copy.deepcopy(layout)
        changed["section_headers"][1]["words"][0] += 1
        with self.assertRaisesRegex(ValueError, "section header"):
            pack_file(changed, dvp_rows(), load)

    def test_section_name_tokens_are_rebuilt(self) -> None:
        layout = json.loads(CATALOGUE.read_text(encoding="utf-8"))
        self.assertEqual(len(string_table(layout["section_name_tokens"])),
                         layout["section_headers"][-1]["words"][5])


if __name__ == "__main__":
    unittest.main()

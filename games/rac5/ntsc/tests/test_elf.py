"""Synthetic MIPS ELF tests; no retail code."""

from __future__ import annotations

import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from elf import inspect_elf, read_global_functions  # noqa: E402


class ElfTests(unittest.TestCase):
    def make_elf(self, path: Path) -> None:
        data = bytearray(256)
        data[:16] = b"\x7fELF\x01\x01\x01" + bytes(9)
        struct.pack_into("<HHIIIIIHHHHHH", data, 16,
                         2, 8, 1, 0x00100000, 52, 96, 0x20924001,
                         52, 32, 1, 40, 3, 2)
        struct.pack_into("<IIIIIIII", data, 52,
                         1, 80, 0x00100000, 0x00100000, 4, 8, 7, 4)
        data[80:84] = bytes(4)
        names = b"\0.text\0.shstrtab\0"
        struct.pack_into("<IIIIIIIIII", data, 96 + 40,
                         1, 1, 6, 0x00100000, 80, 4, 0, 0, 4, 0)
        struct.pack_into("<IIIIIIIIII", data, 96 + 80,
                         7, 3, 0, 0, 216, len(names), 0, 0, 1, 0)
        data[216:216 + len(names)] = names
        path.write_bytes(data)

    def test_mips_elf_metadata(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "synthetic.elf"
            self.make_elf(path)
            result = inspect_elf(path)
            self.assertEqual(result["entry"], "0x00100000")
            self.assertEqual(result["sections"][1]["name"], ".text")
            self.assertEqual(result["programs"][0]["memory_size"], 8)

    def test_rejects_invalid_program_extent(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "synthetic.elf"
            self.make_elf(path)
            data = bytearray(path.read_bytes())
            struct.pack_into("<I", data, 52 + 16, len(data))
            path.write_bytes(data)
            with self.assertRaisesRegex(ValueError, "program header"):
                inspect_elf(path)

    def test_reads_linked_function_body_extent(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "synthetic.elf"
            self.make_elf(path)
            data = bytearray(path.read_bytes())
            data.extend(bytes(256))
            struct.pack_into("<H", data, 16 + 32, 5)  # section count
            names = b"\0.text\0.shstrtab\0.strtab\0.symtab\0"
            strings = b"\0test_func\0"
            data[320:320 + len(names)] = names
            data[360:360 + len(strings)] = strings
            struct.pack_into("<IIIIIIIIII", data, 96 + 80,
                             7, 3, 0, 0, 320, len(names), 0, 0, 1, 0)
            struct.pack_into("<IIIIIIIIII", data, 96 + 120,
                             17, 3, 0, 0, 360, len(strings), 0, 0, 1, 0)
            struct.pack_into("<IIIIIIIIII", data, 96 + 160,
                             25, 2, 0, 0, 384, 32, 3, 0, 4, 16)
            struct.pack_into("<IIIBBH", data, 400,
                             1, 0x00100000, 4, 0x12, 0, 1)
            path.write_bytes(data)
            self.assertEqual(read_global_functions(path),
                             {"test_func": (0x00100000, 4)})

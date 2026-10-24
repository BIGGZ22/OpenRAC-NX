#!/usr/bin/env python3
"""Unit tests of the repository tools, on synthetic data only.

No game byte is read here: the tests build the small structures they need and
check the behaviour the tools promise.  Run them with:

    python3 -m unittest discover -s tests -v
"""

from __future__ import annotations

import struct
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

import disc  # noqa: E402
import function_inventory as inventory  # noqa: E402
import lot_table  # noqa: E402
import mangle  # noqa: E402
from c_source_policy import policy_errors  # noqa: E402


class DiscTest(unittest.TestCase):
    def test_hash_image(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.bin"
            path.write_bytes(b"rac-sac" * 1000)
            sha1, sha256 = disc.hash_image(path)
            self.assertEqual(len(sha1), 40)
            self.assertEqual(len(sha256), 64)
            self.assertNotEqual(sha1, sha256)

    def test_entry_parsing(self) -> None:
        # A directory record: length 34, extent 7, size 2048, no flags.
        record = bytearray(34)
        record[0] = 34
        struct.pack_into("<I", record, 2, 7)
        struct.pack_into("<I", record, 10, 2048)
        record[25] = 0
        record[32] = 1
        record[33] = ord("A")
        entry = disc.IsoDisc._entry(bytes(record), 0, "")
        self.assertEqual(entry.name, "A")
        self.assertEqual(entry.extent, 7)
        self.assertEqual(entry.size, 2048)
        self.assertFalse(entry.directory)
        self.assertEqual(entry.end, 7 * 2048 + 2048)

    def test_rejects_a_bad_record(self) -> None:
        with self.assertRaises(ValueError):
            disc.IsoDisc._entry(bytes(40), 0, "")


class MangleTest(unittest.TestCase):
    def test_free_function(self) -> None:
        parts = mangle.demangle("GLOBAL_GetFlagBit__FUiUc")
        self.assertEqual(parts["kind"], "free function")
        # The encoding carries the parameter list, not the return type.
        self.assertEqual([str(a) for a in parts["arguments"]],
                         ["unsigned int", "unsigned char"])

    def test_void_function(self) -> None:
        self.assertEqual(mangle.prototype("FADEOVERLAY_IsAtFinalUpAlpha__Fv"),
                         "? FADEOVERLAY_IsAtFinalUpAlpha(void)")

    def test_member_function(self) -> None:
        parts = mangle.demangle("LevelDataCB__11ApeLevelDefP12TiXmlElement")
        self.assertEqual(parts["kind"], "member function")
        self.assertEqual(parts["class"], "ApeLevelDef")
        self.assertEqual([str(a) for a in parts["arguments"]], ["TiXmlElement *"])

    def test_constructor_and_destructor(self) -> None:
        self.assertEqual(mangle.demangle("__11ApeLevelDef")["kind"], "constructor")
        self.assertEqual(mangle.demangle("_$_11ApeLevelDef")["kind"], "destructor")

    def test_unknown_code_is_reported(self) -> None:
        parts = mangle.demangle("weird__Fzz")
        self.assertEqual(str(parts["returns"]), "?")


class BranchTest(unittest.TestCase):
    def test_jump(self) -> None:
        word = 0x0C000000 | (0x1000 >> 2)
        self.assertEqual(inventory.branch_target(word, 0x80), 0x1000)

    def test_conditional_branch(self) -> None:
        word = 0x10800002 | 0xFFFF  # beq $zero, $zero, -1
        self.assertEqual(inventory.branch_target(word, 0x80), 0x80 + 4 - 4)

    def test_cop1_branch(self) -> None:
        word = 0x45000008  # bc1f with the register field 8
        self.assertTrue(inventory.is_branch_word(word))
        self.assertEqual(inventory.branch_target(word, 0x80), 0x80 + 4 + 32)

    def test_vector_opcode_is_not_a_branch(self) -> None:
        self.assertFalse(inventory.is_branch_word(0x4BE52108))


class PolicyTest(unittest.TestCase):
    """The evidence of the project is the instruction the compiler produced."""

    def test_plain_c_is_accepted(self) -> None:
        source = "int f(int a) { return a + 1; }\n"
        self.assertEqual(policy_errors(source), [])

    def test_inline_assembly_is_refused(self) -> None:
        source = "int f(void) { asm(\"nop\"); return 0; }\n"
        self.assertTrue(policy_errors(source))

    def test_global_assembly_is_refused(self) -> None:
        source = "asm(\".word 0x00000000\");\nint f(void) { return 0; }\n"
        self.assertTrue(policy_errors(source))

    def test_register_variable_is_refused(self) -> None:
        source = "int f(void) { register int a = 0; return a; }\n"
        self.assertTrue(policy_errors(source))

    def test_assembler_directive_is_refused(self) -> None:
        source = "int f(void) { .word 0x00000000\n return 0; }\n"
        self.assertTrue(policy_errors(source))

    def test_assembler_name_label_is_allowed(self) -> None:
        source = 'extern int f(void) __asm__("f_00001000");\nint f(void) { return 0; }\n'
        self.assertEqual(policy_errors(source), [])

    def test_comment_does_not_hide_a_verdict(self) -> None:
        source = "/* asm(\"nop\"); */\nint f(void) { return 0; }\n"
        self.assertEqual(policy_errors(source), [])


class LotTableTest(unittest.TestCase):
    def test_init_and_check(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "L01.tsv"
            lot_table.main.__wrapped__ if False else None
            path.write_text("\t".join(lot_table.HEADER) + "\n", encoding="utf-8")
            rows = lot_table.read_table(path)
            self.assertEqual(rows, [])
            path.write_text(
                "\t".join(lot_table.HEADER) + "\n"
                "FE.REL\t0x001000\t64\tfunction_inventory:bounded_candidate\t"
                "call+export\tName\t" + "a" * 64 + "\t31\t31\n",
                encoding="utf-8")
            rows = lot_table.read_table(path)
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0]["module"], "FE.REL")

    def test_sha_must_be_hex(self) -> None:
        self.assertIsNone(lot_table.SHA.fullmatch("z" * 64))
        self.assertIsNotNone(lot_table.SHA.fullmatch("a" * 64))


if __name__ == "__main__":
    unittest.main()

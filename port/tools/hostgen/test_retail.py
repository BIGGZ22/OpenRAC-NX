# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for retail.py, the retail program's instructions read from its bytes, and for what asm.py
reads besides glabel functions (a function spimdisasm took for data, an entry with no label).

    python3 -m unittest port/tools/hostgen/test_retail.py

The bytes are the tests' own (tests/mips.py assembles them); the ones that translate and run a
program need Clang, as test_hostgen.py's do.
"""
from __future__ import annotations

import shutil
import struct
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE / "tests"))

import asm  # noqa: E402
import mips  # noqa: E402
import retail  # noqa: E402

JUMP_TABLE = """
        sltiu $2, $4, 3
        beq $2, $0, other
        sll $4, $4, 2
        lui $2, %hi(jtbl_00300000)
        addiu $2, $2, %lo(jtbl_00300000)
        addu $4, $4, $2
        lw $4, 0($4)
        jr $4
        nop
    case0:
        jr $31
        addiu $2, $0, 10
    case1:
        jr $31
        lw $2, -0x10($28)
    case2:
        lui $3, %hi(D_00300010)
        lw $2, %lo(D_00300010)($3)
        jr $31
        addiu $2, $2, 1
    other:
        jr $31
        addiu $2, $0, -1
"""
SYMBOLS = {"jtbl_00300000": 0x00300000, "D_00300010": 0x00300010}


def words_of(text: str) -> list[int]:
    return [int.from_bytes(bytes.fromhex(m.split()[2]), "little")
            for m in (l.split("/*")[1].split("*/")[0] for l in text.splitlines() if "/*" in l and "*/" in l)
            if len(m.split()) == 3]


def elf(segments: list[tuple[int, bytes]]) -> bytes:
    """A minimal EE executable: the ELF header and one loadable program header a segment."""
    n = len(segments)
    head = 52 + 32 * n
    out = bytearray(struct.pack("<4s5B7x2H5I6H", b"\x7fELF", 1, 1, 1, 0, 0, 2, 8, 1, segments[0][0], 52, 0, 0,
                                52, 32, n, 40, 0, 0))
    offset = head
    for va, data in segments:
        out += struct.pack("<8I", 1, offset, va, va, len(data), len(data), 7, 16)
        offset += len(data)
    for _, data in segments:
        out += data
    return bytes(out)


def program(words: dict[int, list[int]]) -> retail.Retail:
    r = retail.Retail(None, {}, 0x00200000)
    r.exe = [retail.Part("text", base, struct.pack(f"<{len(w)}I", *w), True) for base, w in words.items()]
    return r


class Pairs(unittest.TestCase):
    def code(self, r: retail.Retail, starts: dict[int, str]) -> retail.RetailCode:
        return retail.RetailCode(r, {-1: starts}, r.low)

    def test_relocations_and_jump_table_as_the_assembly_says(self):
        text = mips.assemble("func_00100000", 0x00100000, JUMP_TABLE, SYMBOLS)
        labels = mips.label_addresses(0x00100000, JUMP_TABLE)
        words = words_of(text)
        table = [labels["case0"], labels["case1"], labels["case2"]]
        r = program({0x00100000: words, 0x00300000: table + [0, 41]})
        code = self.code(r, {0x00100000: "func_00100000"})
        lines = {a: code.get((-1, a))[0] for a in range(0x00100000, 0x00100000 + 4 * len(words), 4)}
        from_text = {l.addr: l.reloc for l in asm.parse_file(self._file(text))[0].lines if l.reloc}
        # Each lui the assembly writes as %hi(symbol) has its pair here: the table, then a global.
        lui = {a: l.reloc for a, l in lines.items() if l.reloc}
        self.assertEqual(sorted(lui), sorted(a for a, rel in from_text.items() if rel[0] == "hi"))
        self.assertEqual([rel[1] for _, rel in sorted(lui.items())], ["jtbl_00300000", "D_00300010"])
        block = code.get((-1, 0x00100000))[1]
        self.assertEqual(list(block.tables.values()), [table])

    def test_a_lui_in_a_delay_slot_pairs_at_the_branch_target(self):
        source = """
            bne $4, $0, there
            lui $3, %hi(D_00300040)
            jr $31
            daddu $2, $0, $0
        there:
            jr $31
            lw $2, %lo(D_00300040)($3)
        """
        words = words_of(mips.assemble("func_00100000", 0x00100000, source, {"D_00300040": 0x00300040}))
        code = self.code(program({0x00100000: words}), {0x00100000: "func_00100000"})
        self.assertEqual(code.get((-1, 0x00100004))[0].reloc, ("hi", "D_00300040", 0))

    def test_a_function_address_names_the_function(self):
        source = """
            lui $4, %hi(func_00100100)
            jr $31
            addiu $4, $4, %lo(func_00100100)
        """
        words = words_of(mips.assemble("func_00100000", 0x00100000, source, {"func_00100100": 0x00100100}))
        r = program({0x00100000: words, 0x00100100: [0x03E00008, 0]})
        code = self.code(r, {0x00100000: "func_00100000", 0x00100100: "func_00100100"})
        self.assertEqual(code.get((-1, 0x00100000))[0].reloc, ("hi", "func_00100100", 0))
        # Calls and the code addresses handed on, for entries with no label of their own.
        r.low = 0x00100000
        self.assertIn((-1, 0x00100100), r.called())

    def test_elf_sections_or_segments(self):
        image = elf([(0x00100000, struct.pack("<2I", 0x03E00008, 0))])
        (part,) = retail.elf_parts(image)
        self.assertEqual((part.base, part.code, part.word(0x00100000)), (0x00100000, True, 0x03E00008))

    def _file(self, text: str) -> Path:
        d = Path(tempfile.mkdtemp())
        self.addCleanup(shutil.rmtree, d)
        p = d / "func_00100000.s"
        p.write_text(text)
        return p


class AssemblyFiles(unittest.TestCase):
    def test_a_function_written_as_data_is_code(self):
        text = "\n".join([
            ".align 3", "nonmatching func_00100000", "", "dlabel func_00100000",
            "    /* 0 00100000 0800E003 */ .word 0x03E00008",
            "    /* 4 00100004 00000000 */ .word 0x00000000", "", "nonmatching D_00100008", "",
            "dlabel D_00100008", "    /* 8 00100008 FF00FF00 */ .word 0x00FF00FF", ""])
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / "func_00100000.s"
            p.write_text(text)
            (block,) = asm.parse_file(p)
        self.assertEqual(block.name, "func_00100000")
        self.assertEqual([l.word for l in block.lines], [0x03E00008, 0])

    def test_an_entry_with_no_label(self):
        text = mips.assemble("func_00100000", 0x00100000, "addiu $2, $0, 1\njr $31\nnop\n")
        with tempfile.TemporaryDirectory() as tmp:
            (Path(tmp) / "func_00100000.s").write_text(text)
            index = asm.read_index([Path(tmp)], lambda name: -1)
        # Inside the function: only for the decompilation's own stub, or where code calls it.
        self.assertIsNone(index.find(["func_00100004"], [(-1, 0x00100004)]))
        self.assertEqual(index.find(["func_00100004"], [(-1, 0x00100004)], own=True), (-1, 0x00100004))
        index.called.add((-1, 0x00100004))
        self.assertEqual(index.find(["func_00100004"], [(-1, 0x00100004)]), (-1, 0x00100004))


try:
    import test_hostgen  # noqa: E402
    HAVE_CLANG = test_hostgen.have_compiler()
except Exception:   # noqa: BLE001
    HAVE_CLANG = False


@unittest.skipUnless(HAVE_CLANG, "needs Clang on a POSIX host")
class FromRetail(unittest.TestCase):
    """The same program translated from the retail bytes gives the same C and the same results as
    from its assembly files."""

    def run_both(self, c: str, start: int, source: str, data: dict[int, list[int]]) -> tuple[list[str], list[str]]:
        t = test_hostgen.FromAssembly("run_asm")
        text = mips.assemble("func_00100000", start, source, SYMBOLS,
                             {"jtbl_00300000": ["case0", "case1", "case2"]})
        from_files = t.run_program({"a.c": c}, config=test_hostgen.ASM_CONFIG,
                                   extra_files={"game/asm/func_00100000.s": text})
        files_c = t.last_generated
        with tempfile.TemporaryDirectory() as tmp:
            exe = Path(tmp) / "game.elf"
            exe.write_bytes(elf([(start, struct.pack(f"<{len(words_of(text))}I", *words_of(text)))]
                                + [(a, struct.pack(f"<{len(w)}I", *w)) for a, w in data.items()]))
            from_bytes = t.run_program({"a.c": c}, config=test_hostgen.ASM_CONFIG,
                                       hostgen_args=["--retail-exe", str(exe)])
            self.assertEqual(t.last_report["functions_from_assembly"], 1)
        bytes_c = t.last_generated
        strip = lambda s: "\n".join(l for l in s.splitlines() if "translated from its assembly" not in l)
        self.assertEqual(strip(files_c), strip(bytes_c))
        return from_files, from_bytes

    def test_jump_table_and_globals(self):
        labels = mips.label_addresses(0x00100000, JUMP_TABLE)
        c = f"""
            extern unsigned int D_00300000[8];
            extern int D_0017FFF0;
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(int);
            int test_main(void) {{
                D_00300000[0] = 0x{labels['case0']:08X}u;
                D_00300000[1] = 0x{labels['case1']:08X}u;
                D_00300000[2] = 0x{labels['case2']:08X}u;
                D_00300000[4] = 41;
                D_0017FFF0 = 20;
                test_print(func_00100000(0)); test_print(func_00100000(1));
                test_print(func_00100000(2)); test_print(func_00100000(9));
                return 0;
            }}
        """
        table = [labels["case0"], labels["case1"], labels["case2"]]
        a, b = self.run_both(c, 0x00100000, JUMP_TABLE, {0x00300000: table})
        self.assertEqual(a, ["10", "20", "42", "-1"])
        self.assertEqual(b, a)


if __name__ == "__main__":
    unittest.main()

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""tools/permuter on synthetic code and files only, never a game: python3 -m unittest discover -s tools"""

import json
import struct
import tempfile
import unittest
from pathlib import Path

from permuter import accept, elf, profile, runner as queue, score, upstream

ADDIU_SP = 0x27BDFFF0
JR_RA = 0x03E00008


def lui(rt, imm):
    return 0x3C000000 | rt << 16 | imm


def lw(rt, base, imm):
    return 0x8C000000 | base << 21 | rt << 16 | imm & 0xFFFF


def jal(target):
    return 0x0C000000 | target >> 2 & 0x3FFFFFF


def beq(rs, rt, off):
    return 0x10000000 | rs << 21 | rt << 16 | off & 0xFFFF


def asm(*lines):
    """splat lines of (word, text), the word written as stored (le-bytes)."""
    out = ["nonmatching func_00100000, 0x20", "glabel func_00100000"]
    for i, (w, text) in enumerate(lines):
        stored = struct.pack("<I", w).hex().upper()
        out.append(f"    /* {i * 4:X} {0x100000 + i * 4:08X} {stored} */  {text}")
    out += ["endlabel func_00100000", "glabel func_00100020", "    /* 20 00100020 0800E003 */ jr $ra"]
    return "\n".join(out)


def obj(words, relocs):
    """A relocatable ELF32 holding func_00100000 = WORDS in .text, RELOCS {index: (type, symbol)}."""
    text = b"".join(struct.pack("<I", w) for w in words)
    names = sorted({s for _, s in relocs.values()})
    strtab = b"\0func_00100000\0" + b"".join(n.encode() + b"\0" for n in names)
    offsets = {n: strtab.index(b"\0" + n.encode() + b"\0") + 1 for n in names}
    syms = struct.pack("<IIIBBH", 0, 0, 0, 0, 0, 0) + struct.pack("<IIIBBH", 1, 0, len(text), 0x12, 0, 1)
    index = {n: 2 + i for i, n in enumerate(names)}
    syms += b"".join(struct.pack("<IIIBBH", offsets[n], 0, 0, 0x10, 0, 0) for n in names)
    rel = b"".join(struct.pack("<II", i * 4, index[s] << 8 | t) for i, (t, s) in sorted(relocs.items()))
    shstr = b"\0.text\0.symtab\0.strtab\0.rel.text\0.shstrtab\0"
    bodies = [text, syms, strtab, rel, shstr]
    at, offs = 52, []
    for b in bodies:
        offs.append(at)
        at += len(b)
    shoff = at
    sh = [struct.pack("<10I", *[0] * 10),
          struct.pack("<10I", 1, 1, 6, 0, offs[0], len(text), 0, 0, 4, 0),
          struct.pack("<10I", 7, 2, 0, 0, offs[1], len(syms), 3, 2, 4, 16),
          struct.pack("<10I", 15, 3, 0, 0, offs[2], len(strtab), 0, 0, 1, 0),
          struct.pack("<10I", 23, 9, 0, 0, offs[3], len(rel), 2, 1, 4, 8),
          struct.pack("<10I", 33, 3, 0, 0, offs[4], len(shstr), 0, 0, 1, 0)]
    header = b"\x7fELF\x01\x01\x01" + b"\0" * 9 + struct.pack("<HHIIIIIHHHHHH", 1, 8, 1, 0, 0, shoff, 0, 52,
                                                             0, 0, 40, len(sh), 5)
    return header + b"".join(bodies) + b"".join(sh)


class ScoreTest(unittest.TestCase):
    RETAIL = asm((lui(2, 0x16), "lui $v0, %hi(D_00163E74)"), (lw(4, 2, 0x3EF4), "lw $a0, %lo(D_00163E74)($v0)"),
                 (jal(0x11A1B0), "jal func_0011A1B0"), (0, "nop"), (JR_RA, "jr $ra"), (0, "nop"), (0, "nop"))

    def retail(self):
        return score.parse_asm(self.RETAIL, "func_00100000")

    def candidate(self, words, relocs):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "c.o"
            path.write_bytes(obj(words, relocs))
            return score.from_object(path, "func_00100000")

    def test_parse_reads_words_masks_and_stops_at_the_next_function(self):
        f = self.retail()
        self.assertEqual(f.words[:3], [lui(2, 0x16), lw(4, 2, 0x3EF4), jal(0x11A1B0)])
        self.assertEqual(len(f.words), 6, "the padding nop after the delay slot is trimmed")
        self.assertEqual(f.masks[:3], [0xFFFF, 0xFFFF, 0x3FFFFFF])
        self.assertEqual(f.call(2), "func_0011A1B0")

    def test_word_order_word(self):
        text = "glabel func_00100000\n/* 11A1C8 3C020016 */ lui $v0, 0x16\n"
        self.assertEqual(score.parse_asm(text, "func_00100000", "word").words, [lui(2, 0x16)])

    def test_relocated_fields_match_and_a_wrong_callee_does_not(self):
        words = [lui(2, 0), lw(4, 2, 0), jal(0), 0, JR_RA, 0]
        relocs = {0: (5, "D_00163E74"), 1: (6, "D_00163E74"), 2: (4, "func_0011A1B0")}
        self.assertEqual(score.diffs(self.candidate(words, relocs), self.retail()), 0)
        relocs[2] = (4, "func_0011A1B8")
        self.assertEqual(score.diffs(self.candidate(words, relocs), self.retail()), 1)

    def test_an_unrelocated_offset_counts_and_a_missing_instruction_counts(self):
        retail = score.Function([ADDIU_SP, lw(4, 2, 0x3EF4), JR_RA, 0], [0] * 4)
        self.assertEqual(score.diffs(score.Function([ADDIU_SP, lw(4, 2, 0x10), JR_RA, 0], [0] * 4), retail), 1)
        self.assertEqual(score.diffs(score.Function([ADDIU_SP, JR_RA, 0], [0] * 3), retail), 1)

    def test_branches_compare_through_the_alignment(self):
        retail = score.Function([beq(2, 0, 1), 0, ADDIU_SP, JR_RA, 0], [0] * 5)
        same = score.Function([beq(2, 0, 1), 0, ADDIU_SP, JR_RA, 0], [0] * 5)
        other = score.Function([beq(2, 0, 2), 0, ADDIU_SP, JR_RA, 0], [0] * 5)
        self.assertEqual(score.diffs(same, retail), 0)
        self.assertEqual(score.diffs(other, retail), 1)


class ElfTest(unittest.TestCase):
    def test_not_elf(self):
        with tempfile.TemporaryDirectory() as d:
            (Path(d) / "x").write_bytes(b"nope")
            with self.assertRaises(ValueError):
                elf.read(Path(d) / "x")

    def test_function_and_relocations(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "c.o"
            path.write_bytes(obj([ADDIU_SP, jal(0), 0], {1: (4, "func_00200000")}))
            code, relocs = elf.read(path).function("func_00100000")
        self.assertEqual(len(code), 12)
        self.assertEqual(relocs, {4: (4, "func_00200000")})


class ProfileTest(unittest.TestCase):
    SOURCE = ('#include "common.h"\n\nINCLUDE_ASM("asm/game", func_00100000);\n\n'
              'static int helper(int x) { return x; }\n\nint func_00100008(int a) {\n    if (a) { return helper(a); }\n'
              '    return 0;\n}\n')

    def test_definition_and_stub(self):
        start, end = profile.definition(self.SOURCE, "func_00100008")
        self.assertTrue(self.SOURCE[start:end].startswith("int func_00100008(int a) {"))
        self.assertTrue(self.SOURCE[start:end].endswith("}"))
        self.assertIsNone(profile.definition("extern int func_00100008(int);\n", "func_00100008"))
        start, end = profile.stub(self.SOURCE, "func_00100000")
        self.assertEqual(self.SOURCE[start:end], 'INCLUDE_ASM("asm/game", func_00100000);')

    def test_split_draft(self):
        decls, fn = profile.split_draft("extern int g;\nvoid func_00100000(void) { g = 1; }\n", "func_00100000")
        self.assertEqual(decls, "extern int g;\n")
        self.assertEqual(fn, "void func_00100000(void) { g = 1; }")
        with self.assertRaises(ValueError):
            profile.split_draft("int x;\n", "func_00100000")

    def test_strip_top_level_asm_keeps_asm_inside_functions(self):
        text = '__asm__(".extern D_1, 4");\nvoid f(void) { __asm__("" : "+r"(x)); }\n'
        self.assertEqual(profile.strip_top_level_asm(text).lstrip(), 'void f(void) { __asm__("" : "+r"(x)); }\n')
        self.assertEqual(profile.strip_top_level_asm('extern int g(int) __asm__("h");\n'), "extern int g(int) ;\n")

    def test_every_profile_loads_and_names_its_commands(self):
        for name in profile.names():
            p = profile.load(name)
            self.assertTrue(p.compile and p.exact and p.sources and p.asm, name)
            self.assertIn(p.word_order, ("le-bytes", "word"), name)
            self.assertIn("{func}", p.exact, name)

    def test_expand_and_rename(self):
        p = profile.load("rac2/ntsc")
        p.root = Path("/x")
        self.assertEqual(profile.retail_name(p, "func_L00_002AE070"), "f_002AE070")
        self.assertEqual(p.expand(["cc", "{flags}", "{checkout}/{func}"], {"flags": "-O2 -G8", "func": "f"}),
                         ["cc", "-O2", "-G8", "/x/f"])


class QueueTest(unittest.TestCase):
    def test_verdicts(self):
        self.assertEqual(queue.verdict({"status": "matched", "base_score": 3, "best_score": 0, "best": {"artefacts": []}}), "WIN")
        self.assertEqual(queue.verdict({"status": "matched", "base_score": 3, "best_score": 0,
                                        "best": {"artefacts": ["dead store: x"]}}), "artefact")
        self.assertEqual(queue.verdict({"status": "time limit", "base_score": 3, "best_score": 2, "outputs": []}), "better")
        self.assertEqual(queue.verdict({"status": "error", "base_score": None}), "error")

    def test_add_and_record(self):
        with tempfile.TemporaryDirectory() as d:
            queue.main(["add", "rac4/ntsc", "func_00100000", "--dir", d, "--env", "A=1"])
            self.assertEqual(queue.items(Path(d))[0]["env"], "A=1")
            result = Path(d) / "result.json"
            result.write_text(json.dumps({"status": "matched", "base_score": 2, "best_score": 0,
                                          "best": {"artefacts": [], "function": "void f(void) {}\n"}}))
            item = queue.items(Path(d))[0]
            self.assertEqual(queue.record(Path(d), item, result, 12.0), "WIN")
            self.assertEqual(queue.done_keys(Path(d)), {("rac4/ntsc", "func_00100000")})
            self.assertTrue((Path(d) / "wins" / "rac4-ntsc" / "func_00100000.c").is_file())


class UpstreamTest(unittest.TestCase):
    def test_patches_are_stamped(self):
        stamp = upstream.patch_stamp()
        self.assertIn("asm-passes.patch", stamp)
        self.assertIn("aligned-scorer.patch", stamp)


@unittest.skipUnless((upstream.cache_dir() / f"decomp-permuter-{upstream.COMMIT[:7]}").is_dir(),
                     "no decomp-permuter checkout (python3 tools/permuter fetch)")
class AcceptTest(unittest.TestCase):
    def setUp(self):
        upstream.load(upstream.cache_dir() / f"decomp-permuter-{upstream.COMMIT[:7]}")

    def test_artefacts_new_in_the_candidate(self):
        base = "int g(int);\nint f(int a) {\n    int x;\n    x = g(a);\n    return x;\n}\n"
        dup = "int g(int);\nint f(int a) {\n    int x;\n    if (a) { x = g(a); } else { x = g(a); }\n    return x;\n}\n"
        dead = "int g(int);\nint f(int a) {\n    int x;\n    x = 1;\n    x = g(a);\n    return x;\n}\n"
        uninit = "int g(int);\nint f(int a) {\n    int x;\n    int y;\n    x = g(y);\n    return x;\n}\n"
        self.assertEqual(accept.artefacts(base, base, "f"), [])
        self.assertTrue(accept.artefacts(base, dup, "f")[0].startswith("duplicated call: g(a)"))
        self.assertTrue(accept.artefacts(base, dead, "f")[0].startswith("dead store: `x = 1`"))
        self.assertEqual(accept.artefacts(base, uninit, "f"), ["uninitialised read: y is read before it is set"])


if __name__ == "__main__":
    unittest.main()

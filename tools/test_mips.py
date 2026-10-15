"""tools/mips.py and tools/xmap.py on synthetic code only, never a disc:
python3 -m unittest discover -s tools"""

import struct
import tempfile
import unittest
from pathlib import Path

import mips
import xmap

NOP, JR_RA = 0x00000000, 0x03E00008


def code(*instructions: int) -> bytes:
    return struct.pack(f"<{len(instructions)}I", *instructions)


def jal(target: int) -> int:
    return 0x0C000000 | (target >> 2) & 0x3FFFFFF


def lui(rt: int, imm: int) -> int:
    return 0x3C000000 | rt << 16 | imm


def addiu(rt: int, rs: int, imm: int) -> int:
    return 0x24000000 | rs << 21 | rt << 16 | imm & 0xFFFF


def lw(rt: int, rs: int, imm: int) -> int:
    return 0x8C000000 | rs << 21 | rt << 16 | imm & 0xFFFF


def function(callee: int, table: int, constant: int, offset: int = 8) -> bytes:
    """addiu sp; jal callee; lui/addiu a table address; li a0, constant; lw from a struct; jr ra."""
    return code(addiu(29, 29, -16), jal(callee), lui(2, table >> 16), addiu(2, 2, table & 0xFFFF),
                addiu(4, 0, constant), lw(5, 4, offset), JR_RA, addiu(29, 29, 16))


class FingerprintTests(unittest.TestCase):
    def test_the_same_function_at_another_address(self):
        a = function(0x00201000, 0x00150010, 7)
        b = function(0x00345678, 0x0016F420, 7)
        self.assertEqual(mips.fingerprint(a), mips.fingerprint(b))
        self.assertEqual(mips.shape(a), mips.shape(b))

    def test_another_constant_or_offset_is_a_relative_not_a_copy(self):
        a = function(0x00201000, 0x00150010, 7)
        for other in (function(0x00201000, 0x00150010, 8), function(0x00201000, 0x00150010, 7, offset=12)):
            self.assertNotEqual(mips.fingerprint(a), mips.fingerprint(other))
            self.assertEqual(mips.shape(a), mips.shape(other))

    def test_another_instruction_is_neither(self):
        a = function(0x00201000, 0x00150010, 7)
        b = a[:16] + code(0x00852021) + a[20:]         # addu a0, a0, a1 in place of the li
        self.assertNotEqual(mips.shape(a), mips.shape(b))

    def test_padding_does_not_count(self):
        a = function(0x00201000, 0x00150010, 7)
        self.assertEqual(mips.fingerprint(a), mips.fingerprint(a + bytes(8)))


class SplitTests(unittest.TestCase):
    def test_cuts_after_returns_and_at_call_targets(self):
        base = 0x00200000
        first = code(addiu(29, 29, -16), jal(base + 0x20), NOP, JR_RA, addiu(29, 29, 16)) + bytes(12)
        second = code(JR_RA, NOP)
        third = code(addiu(29, 29, -32), JR_RA, addiu(29, 29, 32))
        text = first + second + third
        cuts = mips.split(text, base)
        self.assertEqual([off for off, _ in cuts], [0, 0x20, 0x28])
        # The first function's size leaves out its padding; a nop in a delay slot stays.
        self.assertEqual(mips.code_size(text[0:0x20]), 20)
        self.assertEqual(mips.code_size(code(JR_RA, NOP) + bytes(8)), 8)


class ElfTests(unittest.TestCase):
    def test_reads_stored_sections(self):
        text, names = code(JR_RA, NOP), b"\0.text\0.shstrtab\0"
        header_size, section_size = 0x34, 0x28
        text_at, names_at = header_size, header_size + len(text)
        table_at = names_at + len(names)
        sections = [struct.pack("<10I", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
                    struct.pack("<10I", 1, 1, 6, 0x00200000, text_at, len(text), 0, 0, 4, 0),
                    struct.pack("<10I", 7, 3, 0, 0, names_at, len(names), 0, 0, 1, 0)]
        header = (b"\x7fELF" + bytes(12) + struct.pack("<HHIIIIIHHHHHH", 2, 8, 1, 0x00200000, 0, table_at, 0,
                                                        header_size, 0, 0, section_size, 3, 2))
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "program.elf"
            path.write_bytes(header + text + names + b"".join(sections))
            found = mips.elf_sections(path)
            self.assertEqual(found[".text"][:2], (0x00200000, text))
            path.write_bytes(b"not an elf")
            with self.assertRaises(ValueError):
                mips.elf_sections(path)


class SameRulesAsRac1PalTests(unittest.TestCase):
    """tools/mips.py was lifted from rac1/pal's overlays.py, which keeps its own copy
    (shared/files.json, "related"). They must keep giving the same answers."""

    def test_fingerprints_and_cuts_agree(self):
        import importlib.util
        import sys
        tools = mips.Path(__file__).resolve().parent.parent / "games/rac1/tools"
        spec = importlib.util.spec_from_file_location("rac1_pal_overlays", tools / "overlays.py")
        sys.path.insert(0, str(tools))
        try:
            overlays = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(overlays)
        finally:
            sys.path.remove(str(tools))
        samples = [function(0x00201000, 0x00150010, 7), function(0x00345678, 0x0016F420, 9, offset=0x1234),
                   code(addiu(29, 29, -32), lui(3, 0x42BE), lw(2, 28, -0x7FF0), JR_RA, addiu(29, 29, 32))]
        for sample in samples:
            self.assertEqual(mips.fingerprint(sample), overlays.fingerprint(sample))
            self.assertEqual(mips.shape(sample), overlays.coarse_fingerprint(sample))
        text = b"".join(s + bytes(8) for s in samples)
        self.assertEqual(mips.split(text, 0x00200000), overlays.split(text, 0x00200000))


class SharedFilesTests(unittest.TestCase):
    def test_reports_missing_and_differing_copies(self):
        import json
        import shared
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for name, text in (("a/x.c", "one"), ("b/x.c", "one"), ("a/y.c", "one"), ("b/y.c", "two")):
                (root / name).parent.mkdir(exist_ok=True)
                (root / name).write_text(text)
            (root / "files.json").write_text(json.dumps({
                "groups": [{"what": "same", "copies": ["a/x.c", "b/x.c"]}, {"what": "drifted", "copies": ["a/y.c", "b/y.c"]},
                           {"what": "gone", "copies": ["a/x.c", "b/z.c"]}],
                "related": [{"files": ["a/x.c", "b/none.c"], "why": "kept apart"}]}))
            old = shared.ROOT, shared.MANIFEST
            shared.ROOT, shared.MANIFEST = root, root / "files.json"
            try:
                found = shared.problems()
                shared.sync("a/y.c")
                self.assertEqual((root / "b/y.c").read_text(), "one")
            finally:
                shared.ROOT, shared.MANIFEST = old
        self.assertEqual(len(found), 3)
        self.assertTrue(any("copies differ" in f and "drifted" in f for f in found))
        self.assertTrue(any("b/z.c" in f for f in found) and any("b/none.c" in f for f in found))

    def test_the_manifest_holds(self):
        import shared
        self.assertEqual(shared.problems(), [])


class XmapTests(unittest.TestCase):
    def test_level_numbers_and_classes(self):
        self.assertEqual(xmap.level_id(Path("baserom/overlays/level_05"), "rac1-level"), 5)
        self.assertEqual(xmap.level_id(Path("levels/singleplayer/16_kerwan/overlay.elf"), "elf"), 16)
        self.assertEqual(xmap.klass("boot", "core.text"), "core")
        self.assertEqual(xmap.klass("boot", ".text"), "game")
        self.assertEqual(xmap.klass("level:03", ".text"), "level")
        self.assertEqual(xmap.klass("level:03", "net.text"), "net")

    def test_ports_are_matched_there_present_here_and_open(self):
        same, relative, done = "aa", "bb", "cc"
        entry = lambda fp, shape, size: {"name": fp, "shape": shape, "program": "boot", "address": 0x100, "size": size}
        src = {"matched": {same: entry(same, "s1", 64), relative: entry(relative, "s2", 64),
                           done: entry(done, "s3", 64), "tiny": entry("tiny", "s4", 8)}}
        there = lambda shape: {"size": 64, "shape": shape, "class": "core", "count": 1, "places": [["boot", 0x200]]}
        dst = {"functions": {same: there("s1"), "other": there("s2"), done: there("s3"), "tiny": there("s4")},
               "listed": {}, "matched": {done: entry(done, "s3", 64)}}
        kinds = {p["from"]["name"]: p["kind"] for p in xmap.port_list(src, dst)}
        self.assertEqual(kinds, {same: "same", relative: "shape"})


if __name__ == "__main__":
    unittest.main()

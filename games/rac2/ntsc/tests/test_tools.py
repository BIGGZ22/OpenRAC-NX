"""Tests for tools/levels.py and tools/elf.py: no game data, no toolchains."""
import struct
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import elf  # noqa: E402
import levels  # noqa: E402


def j(target):
    return (2 << 26) | (target >> 2)


def lui(rt, imm):
    return (0x0F << 26) | (rt << 16) | imm


def addiu(rt, rs, imm):
    return (0x09 << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def lw(rt, rs, imm):
    return (0x23 << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


JR_RA, NOP = 0x03E00008, 0


def code(*w):
    return struct.pack(f"<{len(w)}I", *w)


class Identity(unittest.TestCase):
    def test_words_and_trim(self):
        self.assertEqual(levels.words(code(1, 2, 3)), (1, 2, 3))
        self.assertEqual(levels.words(b"\1\0\0\0\2"), (1,))        # partial word dropped
        self.assertEqual(levels.trim(code(5, 0, 0)), code(5))
        self.assertEqual(levels.trim(code(0, 0)), b"")
        self.assertEqual(levels.trim(b"\0\0\0\0\0\0"), b"\0\0")    # only whole words

    def test_trailing_zero_words_ignored(self):
        body = code(addiu(4, 0, 5), JR_RA, NOP)
        padded = body + code(0, 0, 0)
        self.assertEqual(levels.identity(padded), levels.identity(body))
        self.assertEqual(levels.fingerprint(padded), levels.fingerprint(body))

    def test_fingerprint_shape(self):
        fp = levels.fingerprint(code(JR_RA, NOP))
        self.assertEqual(len(fp), 16)
        int(fp, 16)

    def test_jump_target_masked_without_address(self):
        a = code(j(0x100000), NOP)
        b = code(j(0x200000), NOP)
        self.assertEqual(levels.identity(a), levels.identity(b))

    def test_jump_target_below_level_base_kept_with_address(self):
        a = code(j(0x100000), NOP)
        b = code(j(0x200000), NOP)
        self.assertNotEqual(levels.identity(a, 0x2A0000), levels.identity(b, 0x2A0000))
        self.assertNotEqual(levels.fingerprint(a, 0x2A0000), levels.fingerprint(b, 0x2A0000))

    def test_jump_target_in_levels_masked_with_address(self):
        a = code(j(0x2B0000), NOP)
        b = code(j(0x2C0000), NOP)
        self.assertEqual(levels.identity(a, 0x2A0000), levels.identity(b, 0x2A0000))
        self.assertEqual(levels.identity(a, 0x2A0000), code(2 << 26))

    def test_lui_of_non_resident_address_masked(self):
        a = code(lui(4, 0x2A), lw(5, 4, 0x1234), JR_RA)
        b = code(lui(4, 0x2B), lw(5, 4, 0x5678), JR_RA)
        self.assertEqual(levels.identity(a), levels.identity(b))

    def test_lui_of_resident_address_kept(self):
        # 0x15 is below LEVEL_BASE >> 16: the same in every program, so it is compared,
        # and so is the offset used on it.
        a = code(lui(4, 0x15), lw(5, 4, 0x1234), JR_RA)
        b = code(lui(4, 0x16), lw(5, 4, 0x1234), JR_RA)
        c = code(lui(4, 0x15), lw(5, 4, 0x1238), JR_RA)
        self.assertNotEqual(levels.identity(a), levels.identity(b))
        self.assertNotEqual(levels.identity(a), levels.identity(c))
        self.assertEqual(levels.identity(a), a)

    def test_constants_compared(self):
        a = code(addiu(4, 0, 5), JR_RA, NOP)
        b = code(addiu(4, 0, 6), JR_RA, NOP)
        self.assertNotEqual(levels.identity(a), levels.identity(b))

    def test_stack_offsets_compared(self):
        a = code(lw(4, 29, 0x10), JR_RA)
        b = code(lw(4, 29, 0x14), JR_RA)
        self.assertNotEqual(levels.identity(a), levels.identity(b))

    def test_gp_relative_masked(self):
        a = code(lw(4, 28, 0x1234), JR_RA)
        b = code(lw(4, 28, 0x4321), JR_RA)
        self.assertEqual(levels.identity(a), levels.identity(b))
        self.assertEqual(levels.identity(a), code(lw(4, 28, 0), JR_RA))

    def test_small_field_offsets_compared(self):
        a = code(lw(4, 5, 0x10), JR_RA)
        b = code(lw(4, 5, 0x20), JR_RA)
        self.assertNotEqual(levels.identity(a), levels.identity(b))


class Catalogue(unittest.TestCase):
    def test_read_catalogue_and_boot(self):
        with tempfile.TemporaryDirectory() as d:
            cat = Path(d) / "functions.tsv"
            cat.write_text(
                "# name\tkind\tsize\tfingerprint\tlevels\tplaces\n"
                "func_L00_002A3B80\tshared\t8\t17b4ae028fe3a32c\t2\t00:002A3B80,01:002A5F80\n"
                "func_L01_002A6000\tlevel\t16\t0123456789abcdef\t1\t01:002A6000\n")
            boot = Path(d) / "boot.tsv"
            boot.write_text("# name\taddress\tsize\nfunc_00115200\t00115200\t12\nfunc_00115210\t00115210\t4\n")
            old = levels.CATALOGUE, levels.BOOT_FUNCTIONS
            levels.CATALOGUE, levels.BOOT_FUNCTIONS = cat, boot
            try:
                rows = levels.read_catalogue()
                self.assertEqual(rows, [
                    ("func_L00_002A3B80", "shared", 8, "17b4ae028fe3a32c", [(0, 0x2A3B80), (1, 0x2A5F80)]),
                    ("func_L01_002A6000", "level", 16, "0123456789abcdef", [(1, 0x2A6000)]),
                ])
                self.assertEqual(levels.read_boot_functions(),
                                 [("func_00115200", 0x115200, 12), ("func_00115210", 0x115210, 4)])
            finally:
                levels.CATALOGUE, levels.BOOT_FUNCTIONS = old

    def test_level_names(self):
        self.assertEqual(len(levels.NAMES), 27)
        self.assertEqual(levels.level_dir(1), "01_oozla")


class Elf(unittest.TestCase):
    SECTIONS = [
        {"name": ".text", "address": 0x100000, "type": elf.SHT_PROGBITS, "data": code(addiu(4, 0, 1), JR_RA, NOP, NOP)},
        {"name": ".data", "address": 0x200000, "type": elf.SHT_PROGBITS, "data": bytes(range(40))},
        {"name": ".bss", "address": 0x300000, "type": elf.SHT_NOBITS, "data": b"\0" * 64},
    ]

    def write(self, d, name, sections=None, entry=0x100000):
        p = Path(d) / name
        elf.write_exec(p, entry, sections or self.SECTIONS)
        return p

    def test_round_trip(self):
        with tempfile.TemporaryDirectory() as d:
            e = elf.read(self.write(d, "a.elf"))
            self.assertEqual(e["entry"], 0x100000)
            self.assertEqual(e["type"], 2)
            loads = [s for s in e["segments"] if s["type"] == elf.PT_LOAD]
            self.assertEqual([(s["address"], s["filesz"]) for s in loads],
                             [(0x100000, 16), (0x200000, 40)])
            by_name = {s["name"]: s for s in e["sections"]}
            for want in self.SECTIONS:
                got = by_name[want["name"]]
                self.assertEqual((got["address"], got["type"], got["size"]),
                                 (want["address"], want["type"], len(want["data"])))
            text = by_name[".text"]
            self.assertEqual(e["data"][text["offset"]:text["offset"] + text["size"]], self.SECTIONS[0]["data"])

    def test_compare_loads(self):
        with tempfile.TemporaryDirectory() as d:
            a = self.write(d, "a.elf")
            self.assertEqual(elf.compare_loads(a, self.write(d, "b.elf"))[0], True)
            self.assertIn("loaded bytes equal", elf.compare_loads(a, a)[1])
            changed = [dict(s) for s in self.SECTIONS]
            changed[1]["data"] = bytes([0xFF]) + changed[1]["data"][1:]
            ok, why = elf.compare_loads(a, self.write(d, "c.elf", changed))
            self.assertFalse(ok)
            self.assertIn("0x00200000", why)
            ok, why = elf.compare_loads(a, self.write(d, "d.elf", entry=0x100004))
            self.assertFalse(ok)
            self.assertIn("entry", why)
            fewer = self.SECTIONS[:1]
            ok, why = elf.compare_loads(a, self.write(d, "e.elf", fewer))
            self.assertFalse(ok)
            self.assertIn("loadable segments", why)

    def test_not_elf(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "x"
            p.write_bytes(b"nope")
            with self.assertRaises(ValueError):
                elf.read(p)


def dir_record(extent, size, name):
    n = name.encode()
    length = 33 + len(n)
    length += length & 1
    rec = bytearray(length)
    rec[0] = length
    struct.pack_into("<I", rec, 2, extent)
    struct.pack_into(">I", rec, 6, extent)
    struct.pack_into("<I", rec, 10, size)
    struct.pack_into(">I", rec, 14, size)
    rec[32] = len(n)
    rec[33:33 + len(n)] = n
    return bytes(rec)


class IsoTest(unittest.TestCase):
    def test_find(self):
        img = bytearray(2048 * 24)
        root, gdir, content = 20, 21, 22
        pvd = bytearray(2048)
        pvd[156:156 + 34] = dir_record(root, 2048, "\0")[:34].ljust(34, b"\0")
        struct.pack_into("<I", pvd, 158, root)
        struct.pack_into("<I", pvd, 166, 2048)
        img[16 * 2048:17 * 2048] = pvd
        img[root * 2048:root * 2048 + 40] = dir_record(gdir, 2048, "G")
        img[gdir * 2048:gdir * 2048 + 60] = dir_record(content, 4096, "LEVEL0.WAD;1")
        img[content * 2048:content * 2048 + 4] = b"WAD!"
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "t.iso"
            p.write_bytes(bytes(img))
            iso = levels.Iso(p)
            self.assertEqual(iso.find("G/LEVEL0.WAD"), (content, 4096))
            self.assertEqual(iso.find("G"), (gdir, 2048))
            self.assertEqual(iso.read(content, 4), b"WAD!")
            with self.assertRaises(KeyError):
                iso.find("G/MISSING")
            iso.f.close()


class Todo(unittest.TestCase):
    def test_every_marker_is_listed_once_with_its_size(self):
        import progress
        import todo
        rows = todo.todo()
        names = [r[0] for r in rows]
        self.assertEqual(len(names), len(set(names)))
        markers = {n for fns in progress.source_functions().values() for n, has_c in fns if not has_c}
        self.assertEqual(set(names), markers)
        sizes = {n: s for n, _, s in levels.read_boot_functions()}
        for name, size, places, kind, path in rows:
            self.assertGreater(size, 0)
            self.assertGreaterEqual(places, 1)
            if kind == "boot":
                self.assertEqual(size, sizes[name])

    def test_tiers_cover_every_size(self):
        import todo
        self.assertEqual(todo.TIERS[0][1], 0)
        for (_, _, hi), (_, lo, _) in zip(todo.TIERS, todo.TIERS[1:]):
            self.assertEqual(lo, hi + 1)


if __name__ == "__main__":
    unittest.main()

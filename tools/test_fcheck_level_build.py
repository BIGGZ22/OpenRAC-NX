# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for tools/fcheck_level_build.py's pure parts: how a symbol resolves in a level, the boot
executable's bytes by address, and the scratch source of a function no file has. rac1-decomp's
overlay_check is a fake here, and the executable a synthetic ELF: no decompilation, no container."""
import struct
import sys
import tempfile
import types
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import fcheck_level_build as flb  # noqa: E402

RESIDENT_MAX = 0x15F000


class Unresolved(Exception):
    pass


def fake_overlay_check(table: dict[tuple[str, int], int]):
    """overlay_check with resolve_symbol answering from TABLE ((symbol, level) -> address) and an
    empty catalogue, so no executable function has several places."""
    def resolve_symbol(sym, level, near):
        if (sym, level) in table:
            return table[(sym, level)]
        raise Unresolved(sym)
    return types.SimpleNamespace(RESIDENT_MAX=RESIDENT_MAX, Unresolved=Unresolved, resolve_symbol=resolve_symbol,
                                 load_catalogue=lambda: {})


class ResolverTests(unittest.TestCase):
    def resolve(self, name: str, level: int, sym: str, table=None, named=None):
        oc = fake_overlay_check(table or {})
        with mock.patch.dict(sys.modules, {"overlay_check": oc}):
            return flb.resolver(name, level, lambda s: (named or {}).get(s))(sym)

    def test_the_games_libc_is_resident(self):
        self.assertEqual(self.resolve("func_00100000", 5, "memcpy"), 0x00115248)
        self.assertEqual(self.resolve("func_00100000", 5, "memset"), 0x001153FC)

    def test_overlay_check_resolves_first(self):
        self.assertEqual(self.resolve("func_00100000", 5, "func_00200000", {("func_00200000", 5): 0x300000}),
                         0x300000)

    def test_a_suffixed_name_resolves_as_the_bare_name(self):
        self.assertEqual(self.resolve("func_00100000", 5, "D_0013e650_DD68", {("D_0013E650", 5): 0x13E650}),
                         0x13E650)
        self.assertEqual(self.resolve("func_00100000", 5, "D_0013e650_DD68", {("D_0013e650", 5): 0x13E651}),
                         0x13E651)

    def test_level_data_is_where_its_name_says_in_the_levels_own_function(self):
        self.assertEqual(self.resolve("func_L05_00250000", 5, "D_00260000"), 0x260000)
        # In an executable function, or one of another level, it stays unresolved.
        self.assertIsNone(self.resolve("func_00180000", 5, "D_00260000"))
        self.assertIsNone(self.resolve("func_L04_00250000", 5, "D_00260000"))

    def test_named_symbols(self):
        named = {"resident": 0x120000, "moved": 0x170000, "away": 0x180000}
        table = {("func_00170000", 5): 0x270000}
        self.assertEqual(self.resolve("func_00100000", 5, "resident", table, named), 0x120000)
        self.assertEqual(self.resolve("func_00100000", 5, "moved", table, named), 0x270000)
        self.assertIsNone(self.resolve("func_00100000", 5, "unknown", table, named))
        # Not in the level: kept in the level's own function only.
        self.assertIsNone(self.resolve("func_00100000", 5, "away", table, named))
        self.assertEqual(self.resolve("func_L05_00250000", 5, "away", table, named), 0x180000)

    def test_addressed(self):
        self.assertEqual(flb.ADDRESSED.match("func_L05_0025AbCd").groups(), ("func", "05", "0025AbCd"))
        self.assertEqual(flb.ADDRESSED.match("D_0013E650_DD68").groups(), ("D", None, "0013E650"))
        self.assertIsNone(flb.ADDRESSED.match("FUN_00100000"))


class FilesTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name)
        for name, value in (("ROOT", self.root), ("_exe", None)):
            patcher = mock.patch.object(flb, name, value)
            patcher.start()
            self.addCleanup(patcher.stop)

    def test_exe_bytes_reads_by_address_through_the_program_header(self):
        # An ELF header whose program headers start at 0x34, the first loading offset 0x100 at 0x100000.
        raw = bytearray(0x200)
        struct.pack_into("<I", raw, 0x1C, 0x34)
        struct.pack_into("<III", raw, 0x34, 1, 0x100, 0x100000)
        raw[0x110:0x114] = b"\x01\x02\x03\x04"
        (self.root / "baserom").mkdir()
        (self.root / "baserom/SCES_509.16").write_bytes(bytes(raw))
        self.assertEqual(flb.exe_bytes(0x100010, 4), b"\x01\x02\x03\x04")

    def test_scratch_source(self):
        seg, src, first, last = flb.scratch_source("__muldi3")
        self.assertEqual((seg, src, first, last), ("core", Path("build-sn/fcheck/gate/scratch/__muldi3.c"), 1, 1))
        self.assertEqual((self.root / src).read_text(), '#include "common.h"\n/* the candidate */\n')


if __name__ == "__main__":
    unittest.main()

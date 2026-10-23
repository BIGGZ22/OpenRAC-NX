# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""hostgen's and no_c's readers of the port's tables, on synthetic tables only:

    python3 -m unittest discover -s port/tools/hostgen

The readers go through tools/openrac_common/tables.py (P2-10). The expected values here are the
ones the readers gave before that move, so a quirk of the tables (a comment only in the first
column, cells stripped, short rows, a missing table) keeps reading the same.
"""
import tempfile
import unittest
from pathlib import Path

import hostgen
import no_c

# The game's library table (port/game/<id>/libraries.tsv), with every quirk a reader meets.
LIBRARIES = (
    "# The library table.\n"
    "#\n"
    "# symbol\tlibrary\tname\tport\tnotes\tsignature (optional)\n"
    "func_00100000\tlibc\tstrlen\thost\t\tsize_t strlen(const char *s)\n"
    "\n"
    "func_00100010\tlibc\t memcpy \thost\tthe shared one\n"
    "   \n"
    "func_00100020\tgame\tTween\thook\n"
    "# a comment between rows\n"
    "func_00100030\tgame\tWrapped\twrap\tnote\n"
    "  # indented\tis\tdata\thost\n"
    "func_00100040\tgame\tHooked\t hook \n"
    "short\trow\n"
    "func_00100050\tgame\tLong\tskip\tn\tsig\textra\n"
)
# The shared table (port/game/common/libraries.tsv): name, library, signature.
LIBRARY_API = (
    "# name\tlibrary\tsignature\n"
    "strlen\tlibc\tsize_t strlen(const char *)\n"
    "memcpy\tlibc\tvoid *memcpy(void *, const void *, size_t)\n"
    "lonely\n"
)
# no_c.tsv: its column names as its first uncommented line, rows kept by hand.
NO_C = (
    "# a comment\n"
    "name\tclass\tkind\tc\treachable\tevidence\n"
    "func_a\ta\tpadding\tnone\tnothing\tby hand: kept\n"
    "func_b\tc\tcode\tnone\tcalled\tthe assembly\n"
    "func_c\tb\thost\tnone\tnothing\tby hand: also kept\textra\n"
    "func_d\ta\tpadding\tnone\tnothing\tby hand: short\n"
)
MEMCPY = "void *memcpy(void *, const void *, size_t)"


class TablesTest(unittest.TestCase):
    """A fresh folder holding the tables above, as a game's port folder holds them."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self.tmp.name)
        for name, text in (("libraries.tsv", LIBRARIES), ("api.tsv", LIBRARY_API), ("no_c.tsv", NO_C)):
            (self.dir / name).write_text(text, encoding="utf-8")
        self.cfg = {"libraries": "libraries.tsv", "library_api": "api.tsv"}

    def tearDown(self):
        self.tmp.cleanup()


class HostgenLibraries(TablesTest):
    def test_library_api_names_each_signature(self):
        self.assertEqual(hostgen.library_api(self.dir, self.cfg),
                         {"strlen": "size_t strlen(const char *)", "memcpy": MEMCPY})

    def test_a_missing_library_api_is_empty(self):
        self.assertEqual(hostgen.library_api(self.dir, {"library_api": "none.tsv"}), {})

    def test_host_table_takes_the_shared_signature_when_the_row_has_none(self):
        # An indented "#" is data, not a comment: only the first column starts one.
        self.assertEqual(hostgen.host_table(self.dir, self.cfg), {
            "func_00100000": ("strlen", "size_t strlen(const char *s)"),
            "func_00100010": ("memcpy", MEMCPY),
            "# indented": ("data", ""),
        })

    def test_wrapped_hooked_and_wrapped_by_host(self):
        self.assertEqual(hostgen.wrapped(self.dir, self.cfg), {"func_00100020", "func_00100030", "func_00100040"})
        self.assertEqual(hostgen.wrapped_by_host(self.dir, self.cfg), {"func_00100030"})
        self.assertEqual(hostgen.hooked(self.dir, self.cfg), {"func_00100020": "Tween", "func_00100040": "Hooked"})

    def test_no_table_configured_is_nothing(self):
        for reader in (hostgen.host_table, hostgen.wrapped, hostgen.wrapped_by_host, hostgen.hooked):
            with self.subTest(reader=reader.__name__):
                self.assertFalse(reader(self.dir, {}))

    def test_a_configured_table_that_is_missing_raises(self):
        cfg = {"libraries": "none.tsv"}
        for reader in (hostgen.host_table, hostgen.wrapped, hostgen.wrapped_by_host, hostgen.hooked):
            with self.subTest(reader=reader.__name__), self.assertRaises(FileNotFoundError):
                reader(self.dir, cfg)


class NoCTables(TablesTest):
    def test_keep_rows_are_the_full_rows_written_by_hand(self):
        row = dict(zip(no_c.COLUMNS, ["func_a", "a", "padding", "none", "nothing", "by hand: kept"]))
        short = dict(zip(no_c.COLUMNS, ["func_d", "a", "padding", "none", "nothing", "by hand: short"]))
        self.assertEqual(no_c.keep_rows(self.dir / "no_c.tsv"), {"func_a": row, "func_d": short})
        self.assertEqual(no_c.keep_rows(self.dir / "none.tsv"), {})

    def test_host_names_are_the_host_rows_and_host_functions(self):
        cfg = {"libraries": "libraries.tsv", "_dir": self.dir, "host_functions": ["func_x"]}
        self.assertEqual(no_c.host_names(cfg), {
            "func_x": "hostgen.json host_functions",
            "func_00100000": "libraries.tsv host (libc strlen)",
            "func_00100010": "libraries.tsv host (libc memcpy)",
            "# indented": "libraries.tsv host (is data)",
        })
        self.assertEqual(no_c.host_names({"libraries": "none.tsv", "_dir": self.dir}), {})

    def test_decomp_lists_read_the_decompilations_own_tables(self):
        config = self.dir / "config"
        (config / "overlays").mkdir(parents=True)
        (config / "linker_remnants.txt").write_text("# remnants\nsrc/a/func_r1\n  \nfunc_r2\tnote\n")
        (config / "overlays/linker_remnants.txt").write_text("func_r3\n")
        (config / "overlays/joined.tsv").write_text("# joined\nkeep\t func_j1 \tmore\nlone\n")
        self.assertEqual(no_c.decomp_lists(self.dir),
                         {"remnant": {"func_r1", "func_r2", "func_r3"}, "handwritten": set(), "joined": {"func_j1"}})
        self.assertEqual(no_c.decomp_lists(self.dir / "none"), {"remnant": set(), "handwritten": set(), "joined": set()})


if __name__ == "__main__":
    unittest.main()

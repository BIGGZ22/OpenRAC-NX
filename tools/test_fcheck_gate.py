# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for tools/fcheck_gate.py: the verdict files joined to the backfill tables, and fcheck's exit codes."""
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import fcheck_gate as g  # noqa: E402


class GateTests(unittest.TestCase):
    def test_classify(self):
        self.assertEqual(g.classify(0, "x\nfunc_1: SAME on 3 real calls\n"), ("same", "3 calls"))
        self.assertEqual(g.classify(3, "")[0], "no-states")
        self.assertEqual(g.classify(4, "")[0], "unchecked")
        s, note = g.classify(2, "a\nCOMPILE failed")
        self.assertEqual(s, "unchecked")
        self.assertIn("COMPILE failed", note)
        s, note = g.classify(1, "f.0.snap: DIFFERENT memory at 00123456\ncompared 3 of 3 states\n")
        self.assertEqual(s, "differs")
        self.assertIn("DIFFERENT", note)

    def test_row_roundtrip(self):
        r = g.Row.parse("func_00100000\tsame\t2026-10-18\tabc12345\t0123456789ab\t3 calls")
        self.assertEqual(g.Row.parse(r.format()).note, "3 calls")
        self.assertEqual(g.Row("func_1").format(), "func_1")

    def test_verdicts_join_backfill_and_leave_it_alone(self):
        with tempfile.TemporaryDirectory() as d:
            d = Path(d)
            (d / "a.c").write_text("int f(void){return 1;}\n")
            t = d / "t.tsv"
            text = "# a comment\nfunc_00100000\ta.c\nfunc_L01_00100000\ta.c\n"
            t.write_text(text)
            back = g.Backfill(t)
            v = g.Verdicts(d / "fcheck.tsv", back)
            r = v.row("func_00100000")
            r.status, r.date, r.fcheck, r.csum, r.note = "same", "2026-10-18", "abc", g.csum(d / "a.c"), "3 calls"
            v.row("gone").status = "same"   # no backfill row: dropped
            v.write()
            self.assertEqual(t.read_text(), text)
            again = g.Verdicts(d / "fcheck.tsv", back).ordered()
            self.assertEqual([x.name for x in again], ["func_00100000", "func_L01_00100000"])
            self.assertEqual(again[0].status, "same")
            self.assertEqual(again[1].status, "")
            self.assertEqual(back.source("func_00100000"), d / "a.c")

    def test_summary(self):
        rows = [g.Row("a", "same"), g.Row("b", "differs"), g.Row("c")]
        self.assertEqual(g.summary(rows), "3 rows: 1 same, 1 differs, 0 unchecked, 0 no-states, 0 not-executable, 1 no verdict")


if __name__ == "__main__":
    unittest.main()

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

    def test_waive(self):
        known = {"func_1": "retail's own assembly differs the same way"}
        self.assertEqual(g.waive("func_1", "differs", "w", known)[0], "unchecked")
        self.assertIn("retail's own assembly", g.waive("func_1", "differs", "w", known)[1])
        self.assertEqual(g.waive("func_1", "same", "3 calls", known), ("same", "3 calls"))
        self.assertEqual(g.waive("func_2", "differs", "w", known), ("differs", "w"))
        with tempfile.TemporaryDirectory() as d:
            t = Path(d) / "a.tsv"
            t.write_text("# c\nfunc_1\twhy\n")
            self.assertEqual(g.artefacts(t), {"func_1": "why"})
            self.assertEqual(g.artefacts(Path(d) / "none.tsv"), {})

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

    def test_prepare(self):
        src = "extern float sqrtf(float);\nstatic int helper(int x) {\n    return x;\n}\nint func_00100000(int a) {\n    return helper(a) + (int)sqrtf(a);\n}\n"
        t = g.prepare(src, "func_00100000")
        self.assertIn("static __inline__ int helper(int x) {", t)
        self.assertIn("#define sqrtf __gate_sqrtf", t)
        self.assertTrue(t.rstrip().endswith("#undef sqrtf"))
        t = g.prepare(src, "func_00100000", {"func_00100000", "D_00123456_X"})
        self.assertIn("#define D_00123456_X D_00123456_X__gate", t)
        self.assertIn("#define func_00100000 func_00100000__gate", t)
        # The exported label sits right before the renamed definition, the size after it.
        self.assertLess(t.index("func_00100000:"), t.index("int func_00100000(int a) {"))
        self.assertIn(".size func_00100000, . - func_00100000", t)
        t = g.prepare("extern void *memcpy(void *, const void *, unsigned long);\n"
                      "unsigned long long func_00100000(char *d) {\n    memcpy(d, d + 4, 4);\n    return sqrtf(1.0f);\n}\n",
                      "func_00100000")
        self.assertNotIn("unsigned long);", t)
        self.assertIn("    memcpy(d, d + 4, 4);", t)
        self.assertIn("    return sqrtf(1.0f);", t)
        self.assertIn("unsigned long func_00100000", t)
        self.assertNotIn("__gate", g.prepare("int func_00100000(void) { return 0; }\n", "func_00100000"))

    def test_clashes(self):
        log = ("src.c:12: conflicting types for `func_001F4868'\nsrc.c:3: previous declaration of `func_001F4868'\n"
               "src.c:20: redefinition of `struct Foo'\n")
        self.assertEqual(g.clashes(log), {"func_001F4868", "Foo"})

    def test_combine(self):
        self.assertEqual(g.combine("func_00200000", None, [("level 01", "same", "3 calls"), ("level 02", "same", "1 calls")]),
                         ("same", "3 calls (level 01), 1 calls (level 02)"))
        self.assertEqual(g.combine("func_00200000", None, [("level 01", "same", "3 calls"), ("level 02", "differs", "x")])[0],
                         "differs")
        self.assertEqual(g.combine("func_L03_00200000", None, []),
                         ("no-states", "never called in level 03's capture run"))
        self.assertEqual(g.combine("func_00200000", None, [])[0], "no-states")

    def test_summary(self):
        rows = [g.Row("a", "same"), g.Row("b", "differs"), g.Row("c")]
        self.assertEqual(g.summary(rows), "3 rows: 1 same, 1 differs, 0 unchecked, 0 no-states, 0 not-executable, 1 no verdict")


if __name__ == "__main__":
    unittest.main()

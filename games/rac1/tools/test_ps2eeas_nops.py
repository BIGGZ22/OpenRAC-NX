"""Regression checks for shared FP labels and short-loop hazard padding."""
import unittest

from ps2eeas_nops import fp_label_nop, implicit_loop_compare_nop, mfc1_branches, mfc1_sites


class SharedFpLabelTests(unittest.TestCase):
    # c.lt.s $f0,$f20; nop; bc1tl ...
    text = b"".join(w.to_bytes(4, "little") for w in (0x4614003C, 0, 0x45030002))

    def check(self, source):
        return fp_label_nop(source.splitlines(True), 0,
                            len(source.splitlines()) - 1, self.text, 8)

    def test_implicit_nop_after_shared_label(self):
        self.assertTrue(self.check("c.lt.s $f0,$f20\n$L174:\n.set noreorder\n"
                                   ".set nomacro\nbc1tl $L152\n"))

    def test_explicit_nop_is_already_correct(self):
        self.assertFalse(self.check("c.lt.s $f0,$f20\n$L174:\nnop\nbc1tl $L152\n"))

    def test_comparison_without_shared_label_is_unchanged(self):
        self.assertFalse(self.check("$L174:\nc.lt.s $f0,$f20\nbc1tl $L152\n"))

    def test_label_after_another_instruction_is_unchanged(self):
        self.assertFalse(self.check("c.lt.s $f0,$f20\nmov.s $f1,$f0\n"
                                    "$L174:\nbc1tl $L152\n"))

    def test_missing_object_nop_is_not_replaced(self):
        lines = ["c.lt.s $f0,$f20\n", "$L174:\n", "bc1tl $L152\n"]
        self.assertFalse(fp_label_nop(lines, 0, 2, self.text, 4))


class LoopCompareNopTests(unittest.TestCase):
    text = b"".join(w.to_bytes(4, "little") for w in (0x4614003C, 0, 0x4503FFFD))

    def check(self, source):
        lines = source.splitlines(True)
        return implicit_loop_compare_nop(self.text, 0, 8, lines, len(lines) - 1)

    def test_implicit_loop_hazard_must_be_preserved(self):
        self.assertTrue(self.check("$L1:\nc.lt.s $f0,$f20\n.set noreorder\n"
                                   ".set nomacro\nbc1tl $L1\n"))

    def test_written_nop_is_not_counted_twice(self):
        self.assertFalse(self.check("$L1:\nc.lt.s $f0,$f20\nnop\nbc1tl $L1\n"))

    def test_other_source_instruction_does_not_lose_hazard(self):
        self.assertFalse(self.check("$L1:\nc.lt.s $f0,$f20\nmov.s $f1,$f0\n"
                                    "bc1tl $L1\n"))


class Mfc1BranchTests(unittest.TestCase):
    """Rule 4: GNU as pads an mfc1 before a branch that reads its register; ps2eeas did not."""
    # mfc1 $v0,$f1; nop; bnel $s2,$v0; addu (its delay slot); mfc1 $v0,$f1; beq $s3,$v0; addu;
    # mfc1 $v0,$f1; nop; beq $s3,$a0 (another register); nop
    words = (0x44020800, 0, 0x56420004, 0x00641821, 0x44020800, 0x12620002, 0x00641821,
             0x44020800, 0, 0x12640001, 0)
    text = b"".join(w.to_bytes(4, "little") for w in words)

    def test_object_pairs(self):
        self.assertEqual(mfc1_branches(0, len(self.text), self.text), [(0, True), (16, False)])

    def sites(self, source):
        lines = source.splitlines(True)
        return mfc1_sites(lines, 0, len(lines))

    def test_branch_reading_the_register(self):
        self.assertEqual(self.sites("\tmfc1\t$2,$f1\n\t#nop\n\t.set\tnoreorder\n\t.set\tnomacro\n"
                                    "\tbnel\t$18,$2,$L17\n"), [0])

    def test_branch_reading_another_register(self):
        self.assertEqual(self.sites("\tmfc1\t$2,$f1\n\t.set\tnoreorder\n\tbeq\t$19,$20,$L16\n"), [])
        self.assertEqual(self.sites("\tmfc1\t$2,$f1\n\t.set\tnoreorder\n\tbeq\t$19,$21,$L2\n"), [])

    def test_call_or_plain_instruction_after(self):
        self.assertEqual(self.sites("\tmfc1\t$19,$f0\n\t.set\tnoreorder\n\tjal\tfunc_001F9BD8\n"), [])
        self.assertEqual(self.sites("\tmfc1\t$2,$f1\n\taddu\t$3,$2,$4\n"), [])

    def test_mfc1_in_a_delay_slot(self):
        self.assertEqual(self.sites("\t.set\tnoreorder\n\tbeq\t$4,$0,$L3\n\tmfc1\t$2,$f1\n"
                                    "\t.set\treorder\n\t.set\tnoreorder\n\tbne\t$2,$0,$L4\n"), [])
        text = b"".join(w.to_bytes(4, "little") for w in (0x10800003, 0x44020800, 0x14400001, 0))
        self.assertEqual(mfc1_branches(0, len(text), text), [])


if __name__ == "__main__":
    unittest.main()

"""Checks for the pass that makes delay-slot and declared small-data accesses $gp-relative."""
import tempfile
import unittest
from pathlib import Path

import check_macro_slots


def run(source: str) -> str:
    with tempfile.TemporaryDirectory() as d:
        path = Path(d) / "t.s"
        path.write_text(source)
        assert check_macro_slots.main(str(path)) == 0
        return path.read_text()


class DeclaredSmallData(unittest.TestCase):
    def test_declared_label_becomes_one_byte_and_is_equated(self):
        out = run("\t.extern\tD_X__gp, 4\n\tl.s\t$f0,D_X__gp\n")
        self.assertIn(".extern\tD_X__gp, 1\n", out)
        self.assertIn("\tD_X__gp = D_X\n", out)
        self.assertNotIn("D_X__gp, 4", out)

    def test_declared_label_in_a_delay_slot_is_not_suffixed_twice(self):
        out = run("\t.extern\tD_X__gp, 4\n\t.set\tnomacro\n\tlw\t$2,D_X__gp\n\t.set\tmacro\n")
        self.assertNotIn("__gp__gp", out)

    def test_plain_macro_access_in_a_delay_slot_still_gets_its_label(self):
        out = run("\t.extern\tD_Y, 4\n\t.set\tnomacro\n\tlw\t$2,D_Y\n\t.set\tmacro\n")
        self.assertIn("lw\t$2,D_Y__gp", out)
        self.assertIn("\tD_Y__gp = D_Y\n", out)

    def test_a_file_with_neither_is_left_alone(self):
        source = "\t.extern\tD_Z, 4\n\tlw\t$2,D_Z\n"
        self.assertEqual(run(source), source)


if __name__ == "__main__":
    unittest.main()

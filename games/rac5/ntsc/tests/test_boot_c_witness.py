"""Keep the public C source tied to the measured whole-boot witness."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from boot_c_witness import check_source, split_aligned_prefix  # noqa: E402


class BootCWitnessTests(unittest.TestCase):
    def test_candidate_source_has_measured_identity(self) -> None:
        check_source()

    def test_aligned_prefix_preserves_instruction_order(self) -> None:
        assembly = "\n".join([
            ".set noat", ".set noreorder", ".section .text.before",
            ".globl boot_text_start", "boot_text_start:",
            *["    nop"] * 32, "L_after_prefix:", "    jr $ra",
            ".section .text.after", "    nop", "",
        ])
        prefix, remainder = split_aligned_prefix(assembly)
        self.assertIn(".section .text.align\n.align 7", prefix)
        self.assertEqual(prefix.count("    nop"), 32)
        self.assertTrue(remainder.startswith(".set noat\n.set noreorder\n"
                                             ".section .text.before\nL_after_prefix:"))
        self.assertIn(".section .text.after\n    nop", remainder)


if __name__ == "__main__":
    unittest.main()

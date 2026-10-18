"""Synthetic whole-module replacement boundaries."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_program_gate import CODE_START, replace_code, replace_range  # noqa: E402


class LevelProgramGateTests(unittest.TestCase):
    def test_replaces_only_candidate_code_interval(self) -> None:
        source = b"H" * CODE_START + b"A" * 16 + b"D" * 12
        self.assertEqual(replace_code(source, b"B" * 16, CODE_START + 16),
                         b"H" * CODE_START + b"B" * 16 + b"D" * 12)
        self.assertEqual(replace_range(source, b"B" * 4, CODE_START + 4,
                                       CODE_START + 8),
                         b"H" * CODE_START + b"A" * 4 + b"B" * 4
                         + b"A" * 8 + b"D" * 12)

    def test_rejects_truncated_or_oversized_replacement(self) -> None:
        source = b"H" * CODE_START + b"A" * 16
        for replacement in (b"B" * 15, b"B" * 17):
            with self.assertRaisesRegex(ValueError, "extent"):
                replace_code(source, replacement, CODE_START + 16)


if __name__ == "__main__":
    unittest.main()

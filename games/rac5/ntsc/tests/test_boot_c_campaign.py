"""Synthetic checks for the provisional multi-object boot split."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import boot_c_campaign  # noqa: E402


class BootCCampaignTests(unittest.TestCase):
    def test_interval_ownership_rejects_duplicates_overlap_and_unreviewed_ranges(self) -> None:
        start = 0x1000
        def owner(name, lo, hi):
            return boot_c_campaign.Replacement(name, start + lo, start + hi, "", "")
        valid = (owner("first", 128, 136), owner("second", 136, 152))
        with patch.object(boot_c_campaign, "TEXT_START", start), patch.object(boot_c_campaign, "TEXT_SIZE", 160):
            with patch.object(boot_c_campaign, "REPLACEMENTS", valid):
                boot_c_campaign.validate_replacements()
            cases = (
                (valid[0], owner("first", 136, 152)),
                (owner("first", 128, 144), owner("second", 136, 152)),
                tuple(reversed(valid)),
                (owner("first", 120, 136),),
                (owner("first", 128, 168),),
                (owner("first", 132, 136),),
                (owner("../outside", 128, 136),),
            )
            for rows in cases:
                with self.subTest(rows=rows), patch.object(boot_c_campaign, "REPLACEMENTS", rows):
                    with self.assertRaises(ValueError):
                        boot_c_campaign.validate_replacements()

    def test_split_excludes_object_bytes_and_keeps_external_label(self) -> None:
        start = 0x1000
        trial = boot_c_campaign.Replacement("trial", start + 128, start + 136,
                                            "", "")
        source = [".set noat", ".set noreorder", ".section .text",
                  ".globl boot_text_start", "boot_text_start:"]
        for index in range(40):
            if index in (32, 34):
                source.append(f"L_{start + 4 * index:08X}:")
            source.append("    nop")
        with (patch.object(boot_c_campaign, "TEXT_START", start),
              patch.object(boot_c_campaign, "TEXT_SIZE", 160),
              patch.object(boot_c_campaign, "REPLACEMENTS", (trial,))):
            prefix, remainder, labels = boot_c_campaign.split_assembly(
                "\n".join(source) + "\n")
        self.assertEqual(prefix.count("    nop"), 32)
        self.assertEqual(remainder.count("    nop"), 6)
        self.assertEqual(labels, ["L_00001080"])
        self.assertIn(".section .text.seg1\nL_00001088:", remainder)


if __name__ == "__main__":
    unittest.main()

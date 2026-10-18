"""Synthetic exit metadata must preserve the observed transfer, not invent a return."""
import sys
from pathlib import Path
import unittest
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from boot_witness_boundaries import render, terminal_kind, catalogue_totals


class WitnessBoundariesTests(unittest.TestCase):
    def test_multiple_bodies_do_not_hide_an_uncatalogued_object(self):
        spans = [SimpleNamespace(start=0x1000, end=0x1020),
                 SimpleNamespace(start=0x2000, end=0x2010)]
        rows = [("first", 0x1000, 0x1008, 0x1010, "jr_ra"),
                ("second", 0x1010, 0x1020, 0x1020, "call")]
        totals = catalogue_totals(rows, spans)
        self.assertEqual(totals, {"objects": 2, "bodies": 2, "uncatalogued_object_spans": 1,
                                  "body_bytes": 24, "zero_padding_bytes": 8})

    def test_exit_classification_and_catalogue_serialization(self):
        cases = [(0x03E00008, "jr_ra"), (0x08000400, "jump"),
                 (0x0C000400, "call"), (0x03200008, "jr_reg"),
                 (0x0320F809, "call"), (0x03200009, "jr_reg"),
                 (0x0000000D, "break"), (0x0000000C, "syscall")]
        for word, kind in cases:
            with self.subTest(kind=kind, word=word):
                self.assertEqual(terminal_kind(word), kind)
                row = render([("synthetic", 0x1000, 0x1008, 0x1010, kind)]).splitlines()[-1]
                self.assertEqual(row.split('\t')[-1], kind)

    def test_non_transfer_cannot_claim_a_body_ending(self):
        with self.assertRaisesRegex(ValueError, "control transfer"):
            terminal_kind(0)

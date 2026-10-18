"""Context pairs require independent and unambiguous data correspondence."""

from __future__ import annotations

import json
import struct
import sys
import unittest
from pathlib import Path
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_context_translation import (OUTPUT, anchored_target, check_summary,
                                       candidate_boundary,
                                       close_context_dependencies,
                                       code_pointer_fixups,
                                       context_key, context_units,
                                       prune_uncovered_calls,
                                       select_noncolliding_families,
                                       semantic_hi_pairs,
                                       short_zero_run,
                                       unique_zero_run)  # noqa: E402
from rel import PackedRelocation  # noqa: E402


class LevelContextTranslationTests(unittest.TestCase):
    def test_cross_body_hi_uses_identical_local_delay_slot_low(self) -> None:
        data = bytearray(512)
        struct.pack_into("<I", data, 0x90, 0x3C020000)
        struct.pack_into("<I", data, 0x94, 0x10000074)
        struct.pack_into("<I", data, 0x98, 0x24440180)
        struct.pack_into("<I", data, 0xC8, 0x24440180)
        fixups = [PackedRelocation(1, 0x90, 0xC8),
                  PackedRelocation(0, 0x98)]
        self.assertEqual(semantic_hi_pairs(data, fixups, 0x80, 0xC0, 0x100),
                         {0x90: PackedRelocation(1, 0x90, 0x98)})
        struct.pack_into("<I", data, 0xC8, 0x24440184)
        self.assertEqual(semantic_hi_pairs(data, fixups, 0x80, 0xC0, 0x100), {})
        struct.pack_into("<I", data, 0xC8, 0x24440180)
        struct.pack_into("<I", data, 0x94, 0x24420100)
        self.assertEqual(semantic_hi_pairs(data, fixups, 0x80, 0xC0, 0x100), {})

    def test_code_pointer_pair_requires_complete_local_pair(self) -> None:
        data = bytearray(0x200)
        struct.pack_into("<I", data, 0x90, 0x3C010000)
        struct.pack_into("<I", data, 0x98, 0x342100C0)
        pair = [SimpleNamespace(site=0x90, kind=1, low_site=0x98),
                SimpleNamespace(site=0x98, kind=0, low_site=None)]
        self.assertEqual(code_pointer_fixups(data, pair, 0x80, 0xA0, 0x100),
                         ({0x90: 0xC0}, {0x90, 0x98}))
        self.assertEqual(code_pointer_fixups(data, pair, 0x80, 0x94, 0x100),
                         ({}, set()))
        self.assertEqual(code_pointer_fixups(data, pair, 0x80, 0xA0, 0xC0),
                         ({}, set()))

    def test_full_only_boundary_needs_direct_call_and_no_code_conflict(self) -> None:
        row = {"start": "0x80", "body_end": "0x90", "body_bytes": "16",
               "status": "bounded_candidate", "evidence": "call"}
        function = {"entry": "0x80", "min": "0x80", "max": "0x8F",
                    "body_bytes": "16"}
        self.assertEqual(candidate_boundary(row, {}, {0x80: function}), "full_call")
        self.assertEqual(candidate_boundary(
            {**row, "status": "nonzero_tail"}, {}, {0x80: function}),
            "full_call")
        self.assertEqual(candidate_boundary(row, {0x80: function},
                                            {0x80: function}), "code")
        self.assertIsNone(candidate_boundary(
            row, {0x80: {**function, "body_bytes": "12"}}, {0x80: function}))
        self.assertIsNone(candidate_boundary(
            {**row, "status": "pointer_only"}, {}, {0x80: function}))

    def test_split_context_preserves_exact_residual_gaps(self) -> None:
        static = [{"start": "0x80", "limit": "0xB0",
                   "body_bytes": "48", "masked_lead_sha256": "parent"}]
        split = {0x80: [
            {"start": "0x80", "end": "0x90", "span_bytes": "16",
             "masked_sha256": "first"},
            {"start": "0x98", "end": "0xA0", "span_bytes": "8",
             "masked_sha256": "second"},
        ]}
        units = context_units(static, split, bytes(0x200))
        self.assertEqual([row["kind"] for row in units],
                         ["split", "gap", "split", "gap"])
        self.assertEqual([int(row["body_bytes"]) for row in units],
                         [16, 8, 8, 16])
        self.assertNotIn("parent", [row["masked_lead_sha256"] for row in units])

    def test_switch_context_replaces_case_labels_and_keeps_tail(self) -> None:
        static = [
            {"start": "0x80", "limit": "0x90", "body_bytes": "0",
             "masked_lead_sha256": "", "status": "no_return"},
            {"start": "0x90", "limit": "0xB0", "body_bytes": "32",
             "masked_lead_sha256": "case", "status": "pointer_only"},
        ]
        switch = [{"start": "0x80", "end": "0xA0", "body_bytes": "32",
                   "masked_sha256": "whole"}]
        units = context_units(static, {}, bytes(0x200), switch)
        self.assertEqual([row["kind"] for row in units], ["switch", "gap"])
        self.assertEqual([row["masked_lead_sha256"] for row in units][0], "whole")
        self.assertEqual([int(row["body_bytes"]) for row in units], [32, 16])

    def test_removed_callee_cascades_to_its_caller(self) -> None:
        families = {
            "caller": {"body_bytes": 8, "placements": [("A", 0), ("B", 0)]},
            "callee": {"body_bytes": 8, "placements": [("A", 100), ("B", 100)]},
        }

        def calls(_module, start, _size):
            return {4: 100} if start == 0 else {4: 200}

        selected, removed, groups, rounds = prune_uncovered_calls(
            families, {}, [], {"A", "B"}, calls)
        self.assertEqual(selected, {})
        self.assertEqual((removed, groups, rounds), (2, 2, 3))

    def test_global_union_rejects_same_module_collision(self) -> None:
        large = {"body_bytes": 8, "placements": [("A", 0), ("B", 0), ("C", 0)]}
        conflicting = {"body_bytes": 8, "placements": [("A", 8), ("B", 0)]}
        selected, rejected, components = select_noncolliding_families(
            {}, [], {"large": large, "conflicting": conflicting}, {"A", "B", "C"})
        self.assertEqual(set(selected), {"large"})
        self.assertEqual(rejected, 1)
        self.assertEqual(components["A", 0], components["B", 0])
        selected, rejected, _components = select_noncolliding_families(
            {}, [large], {"conflicting": conflicting}, {"A", "B", "C"})
        self.assertEqual(selected, {})
        self.assertEqual(rejected, 1)

    def test_context_callgraph_needs_closed_callee_pair(self) -> None:
        callee = ("callee", "LEVEL_02.REL")
        caller = ("caller", "LEVEL_02.REL")
        missing = ("missing", "LEVEL_02.REL")
        self.assertEqual(close_context_dependencies({
            callee: set(), caller: {callee}}), {callee, caller})
        self.assertEqual(close_context_dependencies({
            callee: set(), caller: {missing}}), {callee})
        self.assertEqual(close_context_dependencies({caller: {caller}}), {caller})

    def test_unique_code_context_changes_with_neighbors(self) -> None:
        static = [{"body_bytes": "8", "masked_lead_sha256": str(index)}
                  for index in range(3)]
        original = context_key(static, 1)
        self.assertEqual(original, context_key(static, 1))
        static[0]["masked_lead_sha256"] = "changed"
        self.assertNotEqual(original, context_key(static, 1))

    def test_target_needs_unique_window_or_unambiguous_anchors(self) -> None:
        source, dest = bytearray(512), bytearray(512)
        source[100:164] = bytes(range(64))
        dest[200:264] = bytes(range(64))
        args = (bytes(source), bytes(dest), 64, 64, 512, 512, {})
        self.assertTrue(anchored_target("A", "B", 100, 200, {}, {}, [], *args))
        self.assertTrue(anchored_target(
            "A", "B", 300, 400, {300: {400: {"one"}}}, {400: {300}}, [300], *args))
        self.assertFalse(anchored_target(
            "A", "B", 500, 400, {500: {400: {"one"}}}, {400: {500}}, [500], *args))
        self.assertFalse(anchored_target(
            "A", "B", 300, 400, {300: {400: {"one"}, 404: {"two"}}},
            {400: {300}}, [300], *args))
        anchors = {280: {380: {"one"}}, 320: {420: {"two"}}}
        reverse = {380: {280}, 420: {320}}
        self.assertTrue(anchored_target(
            "A", "B", 300, 400, anchors, reverse, [280, 320], *args))
        self.assertFalse(anchored_target(
            "A", "B", 300, 404, anchors, reverse, [280, 320], *args))
        anchors[320][420] = {"one"}
        self.assertFalse(anchored_target(
            "A", "B", 300, 400, anchors, reverse, [280, 320], *args))

    def test_centered_unique_data_window_corroborates_object_end(self) -> None:
        source, dest = bytearray(512), bytearray(512)
        source[96:160] = bytes(range(64))
        dest[208:272] = bytes(range(64))
        source[160:192] = b"A" * 32
        dest[272:304] = b"B" * 32
        self.assertTrue(anchored_target(
            "A", "B", 128, 240, {}, {}, [], bytes(source), bytes(dest),
            64, 64, 512, 512, {}))

    def test_two_independent_zero_neighbors_fix_offset(self) -> None:
        data = bytes(512)
        anchors = {280: {380: {"first"}}, 284: {384: {"second"}}}
        reverse = {380: {280}, 384: {284}}
        args = (data, data, 64, 64, 512, 512, {})
        self.assertTrue(anchored_target(
            "A", "B", 300, 400, anchors, reverse, [280, 284], *args))
        anchors[284][384] = {"first"}
        self.assertFalse(anchored_target(
            "A", "B", 300, 400, anchors, reverse, [280, 284], *args))
        anchors[284][384] = {"second"}
        changed = bytearray(data)
        changed[290] = 1
        self.assertFalse(anchored_target(
            "A", "B", 300, 400, anchors, reverse, [280, 284],
            bytes(changed), data, 64, 64, 512, 512, {}))

    def test_unique_zero_run_flanks_fix_interior_offset(self) -> None:
        source, dest = bytearray(b"Q" * 512), bytearray(b"R" * 512)
        for data, start in ((source, 100), (dest, 200)):
            data[start - 8:start] = b"ABCDEFGH"
            data[start:start + 80] = bytes(80)
            data[start + 80:start + 88] = b"IJKLMNOP"
        self.assertEqual(unique_zero_run(source, 64, 512, 140),
                         unique_zero_run(dest, 64, 512, 240))
        self.assertTrue(anchored_target(
            "A", "B", 140, 240, {}, {}, [], bytes(source), bytes(dest),
            64, 64, 512, 512, {}))
        dest[270] = 1
        self.assertFalse(anchored_target(
            "A", "B", 140, 240, {}, {}, [], bytes(source), bytes(dest),
            64, 64, 512, 512, {}))

    def test_two_independent_callers_identify_short_zero_run(self) -> None:
        source, dest = bytearray(b"Q" * 512), bytearray(b"R" * 512)
        source[100:180] = bytes(80)
        dest[200:280] = bytes(80)
        anchors = {103: {203: {"first", "second"}}}
        reverse = {203: {103}}
        self.assertEqual(short_zero_run(source, 64, 512, 170), (100, 180))
        self.assertTrue(anchored_target(
            "A", "B", 170, 270, anchors, reverse, [103],
            bytes(source), bytes(dest), 64, 64, 512, 512, {}))
        anchors[103][203] = {"first"}
        self.assertFalse(anchored_target(
            "A", "B", 170, 270, anchors, reverse, [103],
            bytes(source), bytes(dest), 64, 64, 512, 512, {}))
        anchors[103][203] = {"first", "second"}
        dest[280] = 0
        self.assertFalse(anchored_target(
            "A", "B", 170, 270, anchors, reverse, [103],
            bytes(source), bytes(dest), 64, 64, 512, 512, {}))

    def test_window_cache_is_scoped_to_each_module(self) -> None:
        first, second, target = bytearray(512), bytearray(512), bytearray(512)
        first[100:164] = bytes(range(64))
        second[100:164] = bytes(reversed(range(64)))
        target[200:264] = bytes(range(64))
        windows = {}
        tail = ({}, {}, [], 64, 64, 512, 512, windows)
        self.assertTrue(anchored_target(
            "A", "B", 100, 200, tail[0], tail[1], tail[2],
            bytes(first), bytes(target), *tail[3:]))
        self.assertFalse(anchored_target(
            "C", "B", 100, 200, tail[0], tail[1], tail[2],
            bytes(second), bytes(target), *tail[3:]))

    def test_committed_context_report_remains_unqualified(self) -> None:
        report = json.loads(OUTPUT.read_text(encoding="utf-8"))
        check_summary(report)
        self.assertIsNone(report["overall_percent"])
        self.assertFalse(report["bytematch_ready"])
        changed = {**report, "conditional_candidate_bytes": 0}
        with self.assertRaisesRegex(ValueError, "stale or overclaims"):
            check_summary(changed)


if __name__ == "__main__":
    unittest.main()

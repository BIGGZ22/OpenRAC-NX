"""Full-module signals remain conservative review categories."""

from __future__ import annotations

import sys
import struct
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_full_module_flow import candidate_signals, linear_predecessor, refine_category  # noqa: E402


class FullModuleFlowTests(unittest.TestCase):
    def test_verified_table_pointer_and_same_body_computed_jump(self) -> None:
        ranges = [(0x100, 0x10F, 0x100)]
        refs = [{"from": "0x200", "type": "DATA", "computed": "false",
                 "jump": "false", "call": "false"},
                {"from": "0x104", "type": "COMPUTED_JUMP", "computed": "true",
                 "jump": "true", "call": "false"}]
        data = bytearray(0x204)
        struct.pack_into("<I", data, 0x104, 0x00200008)  # jr $at
        category, signals, owner = candidate_signals(
            0x108, 0x100, {0x100}, ranges, [0x100], refs, {0x200: 0x108},
            bytes(data))
        self.assertEqual(category, "computed_dispatch_block_lead")
        self.assertEqual(owner, 0x100)
        self.assertTrue(signals["packed_pointer_reference"])
        self.assertTrue(signals["same_owner_computed_jump"])
        category, _, _ = candidate_signals(
            0x108, 0x100, {0x100}, ranges, [0x100], refs, {}, bytes(data))
        self.assertEqual(category, "linear_fallthrough_block_lead")
        struct.pack_into("<I", data, 0x104, 0x03E00008)  # jr $ra is a return
        category, _, _ = candidate_signals(
            0x108, 0x100, {0x100}, ranges, [0x100], refs, {0x200: 0x108},
            bytes(data))
        self.assertEqual(category, "linear_fallthrough_block_lead")
        struct.pack_into("<I", data, 0x100, 0x03E00008)  # return before delay slot
        self.assertFalse(linear_predecessor(0x108, 0x100, ranges, [0x100], bytes(data)))
        category, _, _ = candidate_signals(
            0x108, 0x100, {0x100}, ranges, [0x100], refs, {0x200: 0x108},
            bytes(data))
        self.assertEqual(category, "unresolved")

    def test_cross_body_call_and_new_entry_is_function_lead(self) -> None:
        ranges = [(0x100, 0x10F, 0x100), (0x120, 0x12F, 0x120)]
        refs = [{"from": "0x104", "type": "COMPUTED_CALL", "computed": "true",
                 "jump": "false", "call": "true"}]
        data = bytearray(0x130)
        struct.pack_into("<I", data, 0x104, 0x0020F809)  # jalr $at
        category, signals, owner = candidate_signals(
            0x120, 0x100, {0x100, 0x120}, ranges, [0x100, 0x120], refs, {},
            bytes(data))
        self.assertEqual(category, "called_function_lead")
        self.assertEqual(owner, 0x120)
        self.assertTrue(signals["owner_changed"])
        category, _, _ = candidate_signals(
            0x120, 0x100, {0x100}, ranges, [0x100, 0x120], [], {}, bytes(data))
        self.assertEqual(category, "owner_disagreement")
        self.assertEqual(refine_category(category, 0x128, 0x120, {0x120}),
                         "inside_new_called_body_lead")
        self.assertEqual(refine_category(category, 0x128, 0x120, set()),
                         "owner_disagreement")


if __name__ == "__main__":
    unittest.main()

"""Switch-body leads require one owner and no external interior entry."""

from __future__ import annotations

import json
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from level_switch_bodies import OUTPUT, check_summary, reviewed_switch  # noqa: E402


class LevelSwitchBodiesTests(unittest.TestCase):
    def test_rejects_external_edges_and_competing_owners(self) -> None:
        data = bytearray(0x200)
        struct.pack_into("<I", data, 0xB8, 0x03E00008)
        row = {"start": "0x80", "status": "no_return", "evidence": "call"}
        body = {"min": "0x80", "max": "0xBF", "body_bytes": "64",
                "ranges": "0x80-0xBF"}
        code = {0x80: {**body, "body_bytes": "32"}}
        internal = [{"start": "0x90", "evidence": "pointer"}]

        def review(full=None, code_map=None, split=None, incoming=None):
            full = full if full is not None else {0x80: body}
            code_map = code_map if code_map is not None else code
            incoming = incoming if incoming is not None else {}
            return reviewed_switch(row, full, code_map, internal,
                                   sorted(full), sorted(code_map),
                                   split or set(), incoming, sorted(incoming),
                                   bytes(data), 0x200)

        self.assertEqual(review(), (0x80, 0xC0))
        self.assertIsNone(review(incoming={0x90: [0x40]}))
        self.assertIsNone(review(full={0x80: body, 0x90: body}))
        self.assertIsNone(review(split={0x90}))
        self.assertIsNone(review(code_map={0x80: {**body, "max": "0xAF"}}))

    def test_committed_snapshot_is_unqualified(self) -> None:
        report = json.loads(OUTPUT.read_text(encoding="utf-8"))
        check_summary(report)
        self.assertIsNone(report["overall_percent"])
        self.assertFalse(report["bytematch_ready"])


if __name__ == "__main__":
    unittest.main()

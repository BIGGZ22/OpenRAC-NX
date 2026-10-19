# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for no_c.py: which functions with no C need C, from a small made-up program.

    python3 -m unittest port/tools/hostgen/test_no_c.py      (no Clang needed)
"""
from __future__ import annotations

import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import no_c  # noqa: E402
import retail  # noqa: E402

JR_RA, NOP, FILL = 0x03E00008, 0, 0xCDCDCDCD


def jal(target: int) -> int:
    return 3 << 26 | (target >> 2) & 0x3FFFFFF


def beq_to(pc: int, target: int) -> int:
    return 4 << 26 | ((target - pc - 4) >> 2) & 0xFFFF


class Classify(unittest.TestCase):
    def test_classes(self):
        base = 0x00100000
        code = {
            0x00100000: [jal(0x00100020), NOP, beq_to(0x00100008, 0x00100030), NOP, JR_RA, NOP, 0, 0],  # main, C
            0x00100020: [JR_RA, NOP, 0, 0],             # called by main: needs C
            0x00100030: [JR_RA, NOP, 0, 0],             # main's branch lands here: a piece of main
            0x00100040: [FILL, FILL, 0, 0],             # padding
            0x00100050: [JR_RA, NOP, 0, 0],             # nothing refers to it
            0x00100060: [JR_RA, NOP, 0, 0],             # its address is in the data
            0x00100070: [jal(0x00100080), NOP, JR_RA, NOP],  # the host's own
            0x00100080: [JR_RA, NOP, 0, 0],             # only the host's function calls it
        }
        words = []
        for a in sorted(code):
            words += code[a]
        r = retail.Retail(None, {}, 0x00200000)
        r.exe = [retail.Part(".text", base, struct.pack(f"<{len(words)}I", *words), True),
                 retail.Part(".data", 0x00180000, struct.pack("<2I", 0x00100060, 7), False)]
        names = {a: f"func_{a:08X}" for a in code}
        stubs = [n for a, n in names.items() if a not in (0x00100000, 0x00100070)]
        report = {
            "without_c_places": {n: {"places": [[-1, n[5:]]], "kind": "asm"} for n in stubs},
            "from_assembly": {}, "assembly_unsupported": {}, "without_c_or_assembly": [],
            "call_graph": {"func_00100000": ["func_00100020"]},
        }
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            gen = tmp / "gen"
            gen.mkdir()
            (gen / "report.json").write_text(json.dumps(report))
            (gen / "functions.c").write_text("static const openrac_fn_entry k_exe[] = {\n" + "".join(
                f'    {{0x{a:08X}u, (openrac_host_fn){n}, "{n}"}},\n' for a, n in sorted(names.items())) + "};\n")
            cfg = {"host_functions": ["func_00100070"], "asm": {"dirs": []}, "_dir": tmp}
            rows = {row["name"]: row for row in no_c.classify(cfg, gen, tmp, tmp, r, {})}
        got = {n: (row["cls"], row["kind"]) for n, row in rows.items()}
        self.assertEqual(got, {
            "func_00100020": ("c", "needs C"),
            "func_00100030": ("a", "piece"),
            "func_00100040": ("a", "padding"),
            "func_00100050": ("a", "unreachable"),
            "func_00100060": ("c", "needs C"),
            "func_00100080": ("b", "behind host"),
        })
        self.assertIn("called by func_00100000", rows["func_00100020"]["reachable"])
        self.assertIn("data at .data 00180000", rows["func_00100060"]["reachable"])


if __name__ == "__main__":
    unittest.main()

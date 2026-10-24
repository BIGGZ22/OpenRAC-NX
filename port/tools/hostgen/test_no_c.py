# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for no_c.py: which functions with no C need C, from a small made-up program.

    python3 -m unittest port/tools/hostgen/test_no_c.py      (no Clang needed)
"""
from __future__ import annotations

import json
import struct
import tempfile
import unittest
from pathlib import Path

import no_c
import retail

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


class ClassifyEveryKind(unittest.TestCase):
    """Every class and kind classify gives, from one made-up program: the whole rows are pinned
    (written before classify was cut into steps, so the cut is held to the same table)."""

    def rows(self) -> dict[str, dict]:
        r = retail.Retail(None, {}, 0x00200000)
        code = {
            0x00100000: [jal(0x00100030), jal(0x001000B0), JR_RA, NOP],  # main (C)
            0x00100010: [beq_to(0x00100010, 0x00100020), NOP, JR_RA, NOP],              # the host's entry
            0x00100020: [JR_RA, NOP, 0, 0],             # only the host entry's branch reaches it
            0x00100030: [JR_RA, NOP, 0, 0],             # called by main
            0x00100040: [FILL, FILL, 0, 0],             # padding, no callers
            0x00100050: [JR_RA, NOP, 0, 0],             # a linker remnant
            0x00100060: [JR_RA, NOP, 0, 0],             # joined to the one before
            0x00100070: [jal(0x00100080), NOP, JR_RA, NOP],  # a function only the host runs
            0x00100080: [JR_RA, NOP, 0, 0],             # called by that one only
            0x00100090: [JR_RA, NOP, 0, 0],             # another spelling of main
            0x001000A0: [JR_RA, NOP, 0, 0],             # judged by hand
            0x001000B0: [JR_RA, NOP, 0, 0],             # hand-written, called by main
            0x001000C0: [JR_RA, NOP, 0, 0],             # nothing refers to it
        }
        words = []
        for a in sorted(code):
            words += code[a]
        r.exe = [retail.Part(".text", 0x00100000, struct.pack(f"<{len(words)}I", *words), True)]
        r.entry = 0x00100010
        places = {
            "func_00100020": {"places": [[-1, "00100020"]], "kind": "asm"},
            "func_00100030": {"places": [[-1, "00100030"]], "kind": "asm"},
            "func_00100040": {"places": [[-1, "00100040"]], "kind": "asm"},
            "func_00100050": {"places": [[-1, "00100050"]], "kind": "asm"},
            "func_00100060": {"places": [[-1, "00100060"]], "kind": "asm"},
            "func_00100070": {"places": [[-1, "00100070"]], "kind": "asm"},
            "func_00100080": {"places": [[-1, "00100080"]], "kind": "asm"},
            "func_001000C0": {"places": [[-1, "001000C0"]], "kind": "asm"},
            "func_00100090": {"places": [[-1, "00100090"]], "kind": "asm", "other_spelling": "main"},
            "func_001000A0": {"places": [[-1, "001000A0"]], "kind": "asm"},
            "func_001000B0": {"places": [[-1, "001000B0"]], "kind": "asm"},
            "sqrtf": {"places": [], "kind": "decl"},
            "only_declared": {"places": [], "kind": "decl"},
            "named_only": {"places": [], "kind": "decl"},
            "host_named": {"places": [], "kind": "decl"},
        }
        report = {
            "without_c_places": places,
            "from_assembly": {"func_00100030": {}},
            "assembly_unsupported": {"func_00100020": "an unsupported opcode"},
            "without_c_or_assembly": ["func_00100040"],
            "call_graph": {"main": ["named_only"]},
        }
        names = {0x00100000: "main", 0x00100010: "crt0", 0x00100020: "func_00100020",
                 0x00100030: "func_00100030", 0x00100040: "func_00100040", 0x00100050: "func_00100050",
                 0x00100060: "func_00100060", 0x00100070: "func_00100070"}
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            gen = tmp / "gen"
            gen.mkdir()
            (gen / "report.json").write_text(json.dumps(report))
            (gen / "functions.c").write_text("static const openrac_fn_entry k_exe[] = {\n" + "".join(
                f'    {{0x{a:08X}u, (openrac_host_fn){n}, "{n}"}},\n' for a, n in sorted(names.items())) + "};\n")
            (tmp / "config/overlays").mkdir(parents=True)
            (tmp / "config/linker_remnants.txt").write_text("src/x/func_00100050\n")
            (tmp / "config/overlays/joined.tsv").write_text("4\tfunc_00100060\n")
            (tmp / "config/handwritten_asm.txt").write_text("func_001000B0\n")
            (tmp / "libs.tsv").write_text("host_named\tx\ty\thost\n")
            cfg = {"host_functions": ["func_00100070"], "libraries": "libs.tsv", "asm": {"dirs": []},
                   "_dir": tmp, "entry": "main"}
            kept = {"func_001000A0": {"class": "a", "kind": "by hand", "evidence": "by hand: not an address"}}
            return {row["name"]: row for row in no_c.classify(cfg, gen, tmp, tmp, r, kept)}

    def test_every_row(self):
        self.assertEqual(self.rows(), GOLDEN_ROWS)


GOLDEN_ROWS = {'func_00100020': {'c': 'none',
                   'cls': 'b',
                   'evidence': 'only the branches and jumps of crt0 [host] reach it: part of that '
                               "function's code; an unsupported opcode",
                   'kind': 'piece',
                   'name': 'func_00100020',
                   'reachable': 'branched to by crt0 [host] (the executable)'},
 'func_00100030': {'c': 'translated',
                   'cls': 'c',
                   'evidence': 'translated from its assembly',
                   'kind': 'needs C',
                   'name': 'func_00100030',
                   'reachable': 'called by main [C] (the executable)'},
 'func_00100040': {'c': 'none',
                   'cls': 'a',
                   'evidence': "its first word is the linker's 0xCDCDCDCD fill (the executable)",
                   'kind': 'padding',
                   'name': 'func_00100040',
                   'reachable': 'nothing: no call, jump, branch or address of it'},
 'func_00100050': {'c': 'none',
                   'cls': 'a',
                   'evidence': "the decompilation's linker remnants list (what the linker left of "
                               'a stripped function)',
                   'kind': 'linker remnant',
                   'name': 'func_00100050',
                   'reachable': 'nothing: no call, jump, branch or address of it'},
 'func_00100060': {'c': 'none',
                   'cls': 'a',
                   'evidence': 'config/overlays/joined.tsv: a piece of the function before it',
                   'kind': 'piece',
                   'name': 'func_00100060',
                   'reachable': 'nothing: no call, jump, branch or address of it'},
 'func_00100070': {'c': 'none',
                   'cls': 'b',
                   'evidence': 'hostgen.json host_functions',
                   'kind': 'host',
                   'name': 'func_00100070',
                   'reachable': 'nothing: no call, jump, branch or address of it'},
 'func_00100080': {'c': 'none',
                   'cls': 'b',
                   'evidence': 'only functions the port replaces reach it: func_00100070',
                   'kind': 'behind host',
                   'name': 'func_00100080',
                   'reachable': 'called by func_00100070 [host] (the executable)'},
 'func_00100090': {'c': 'none',
                   'cls': 'a',
                   'evidence': 'the same function as main, which has C: calls to this name go '
                               'there',
                   'kind': 'other spelling',
                   'name': 'func_00100090',
                   'reachable': 'nothing: no call, jump, branch or address of it'},
 'func_001000A0': {'c': 'none',
                   'cls': 'a',
                   'evidence': 'by hand: not an address',
                   'kind': 'by hand',
                   'name': 'func_001000A0',
                   'reachable': 'nothing: no call, jump, branch or address of it'},
 'func_001000B0': {'c': 'none',
                   'cls': 'c',
                   'evidence': 'hand-written; no assembly',
                   'kind': 'needs C',
                   'name': 'func_001000B0',
                   'reachable': 'called by main [C] (the executable)'},
 'func_001000C0': {'c': 'none',
                   'cls': 'a',
                   'evidence': 'no call, jump, branch, address or data word refers to it in any '
                               'program (the executable)',
                   'kind': 'unreachable',
                   'name': 'func_001000C0',
                   'reachable': 'nothing: no call, jump, branch or address of it'},
 'host_named': {'c': 'none',
                'cls': 'b',
                'evidence': 'libraries.tsv host (x y)',
                'kind': 'host',
                'name': 'host_named',
                'reachable': 'nothing: no C calls it, and it has no address'},
 'named_only': {'c': 'none',
                'cls': 'c',
                'evidence': 'C calls it by a name with no address',
                'kind': 'no address',
                'name': 'named_only',
                'reachable': 'C calls it: main'},
 'only_declared': {'c': 'none',
                   'cls': 'a',
                   'evidence': 'declared by the decompilation, defined nowhere, and no C calls it '
                               'or takes its address',
                   'kind': 'declared only',
                   'name': 'only_declared',
                   'reachable': 'nothing: no C calls it, and it has no address'},
 'sqrtf': {'c': 'none',
           'cls': 'b',
           'evidence': "the C library's own name: hostgen writes calls to it as the host "
                       "compiler's (lower.py)",
           'kind': 'host',
           'name': 'sqrtf',
           'reachable': 'nothing: no C calls it, and it has no address'}}


if __name__ == "__main__":
    unittest.main()

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""xcheck.py on a made-up game: a tiny executable, its functions.c, hand C that agrees with it and
hand C that does not. Needs Clang (as hostgen does); skipped without it."""
import io
import json
import shutil
import struct
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

import xcheck

BASE = 0x00100000
NOP, JR_RA = 0x00000000, 0x03E00008


def lui(rt, imm):
    return 0x3C000000 | rt << 16 | imm


def lw(rt, off, rs):
    return 0x8C000000 | rs << 21 | rt << 16 | (off & 0xFFFF)


def jal(target):
    return 0x0C000000 | (target >> 2) & 0x3FFFFFF


def elf(words: dict[int, int], size: int = 0x400) -> bytes:
    """An ELF32 executable with one loaded segment from BASE holding WORDS (address -> word)."""
    body = bytearray(size)
    for a, w in words.items():
        struct.pack_into("<I", body, a - BASE, w)
    head = bytearray(0x54)
    head[:4] = b"\x7fELF"
    head[4:7] = bytes([1, 1, 1])
    struct.pack_into("<HHIIIIIHHHHHH", head, 16, 2, 8, 1, BASE, 0x34, 0, 0, 0x34, 0x20, 1, 0, 0, 0)
    struct.pack_into("<8I", head, 0x34, 1, 0x54, BASE, BASE, size, size, 5, 0x10)
    return bytes(head) + bytes(body)


# The retail program: FUN_00100000 calls func_00100100 and returns the word at 0x00131234;
# FUN_00100040 calls func_00100200; func_00100100 and func_00100200 just return.
RETAIL = {
    0x00100000: jal(0x00100100), 0x00100004: NOP, 0x00100008: lui(2, 0x0013),
    0x0010000C: lw(2, 0x1234, 2), 0x00100010: JR_RA, 0x00100014: NOP,
    0x00100040: jal(0x00100200), 0x00100044: NOP, 0x00100048: JR_RA, 0x0010004C: NOP,
    0x00100100: JR_RA, 0x00100104: NOP,
    0x00100200: JR_RA, 0x00100204: NOP,
}

FUNCTIONS_C = """static const openrac_fn_entry k_exe[] = {
    {0x00100000u, (openrac_host_fn)game_FUN_00100000, "FUN_00100000"},
    {0x00100040u, (openrac_host_fn)game_FUN_00100040, "FUN_00100040"},
    {0x00100100u, (openrac_host_fn)game_func_00100100, "func_00100100"},
    {0x00100200u, (openrac_host_fn)game_func_00100200, "func_00100200"},
    {0x00100300u, (openrac_host_fn)game_func_00100300, "func_00100300"},
};
"""

AGREES = """extern void func_00100100(void);
extern int D_00131234;
int FUN_00100000(void) {
    func_00100100();
    return D_00131234;
}
"""

# A wrapper of the port's own at an address no retail function starts at, called for func_00100200.
WRAPPED = """extern void target_00100200(void) __asm__("func_00100200");
void func_00100300(void) { target_00100200(); }
void FUN_00100040(void) { func_00100300(); }
"""


class XcheckTest(unittest.TestCase):
    def setUp(self):
        if shutil.which("clang") is None:
            self.skipTest("needs Clang")
        self.tmp = Path(tempfile.mkdtemp(prefix="xcheck_test"))
        game = self.tmp / "port" / "game" / "test"
        (game / "hand").mkdir(parents=True)
        decomp = self.tmp / "decomp"
        decomp.mkdir()
        (decomp / "game.elf").write_bytes(elf(RETAIL))
        (game / "hostgen.json").write_text(json.dumps({
            "source": "decomp", "relocate": [0x00150000, 0x00200000], "hand": "hand/hand.tsv",
            "includes": [], "defines": [],
            "names": {"code": [r"^func_(?:L(?P<overlay>\d\d)_)?(?P<addr>[0-9A-Fa-f]{8})$",
                               r"^FUN_(?:L(?P<overlay>\d\d)_)?(?P<addr>[0-9A-Fa-f]{8})$"],
                      "data": [r"^D_(?:L(?P<overlay>\d\d)_)?(?P<addr>[0-9A-Fa-f]{6,8})$"]},
            "asm": {"gp": "0x00160000", "retail": {"exe": "game.elf"}}}))
        gen = self.tmp / "gen"
        gen.mkdir()
        (gen / "functions.c").write_text(FUNCTIONS_C)
        self.game, self.gen = game, gen
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", AGREES, WRAPPED)

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def hand(self, table, a, b, allow=None):
        (self.game / "hand" / "hand.tsv").write_text("# name\tfile\n" + table)
        (self.game / "hand" / "a.c").write_text(a)
        (self.game / "hand" / "b.c").write_text(b)
        path = self.game / "hand" / "allow.tsv"
        if allow is None:
            path.unlink(missing_ok=True)
        else:
            path.write_text("# row\tkind\titem\treason\n" + allow)

    def run_mode(self, mode):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            code = xcheck.main([mode, str(self.game), "--gen", str(self.gen)])
        return code, out.getvalue()

    def test_agreeing_c_passes(self):
        code, out = self.run_mode("check")
        self.assertEqual(code, 0, out)
        self.assertEqual(out, "")

    def test_wrong_callee(self):
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", AGREES.replace("00100100", "00100200"), WRAPPED)
        code, out = self.run_mode("check")
        self.assertEqual(code, 1)
        self.assertIn("c-call func_00100200", out)
        self.assertIn("code-call func_00100100", out)

    def test_wrong_global(self):
        # A global carried 0x100 off, as a PAL address left untranslated is.
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", AGREES.replace("D_00131234", "D_00131334"), WRAPPED)
        code, out = self.run_mode("check")
        self.assertEqual(code, 1)
        self.assertIn("c-data 0x00131334", out)
        self.assertIn("code-data 0x00131234", out)

    def test_field_offset_through_a_struct(self):
        # The field of a struct laid over a global: Clang folds it into one address, as retail does.
        c = ("typedef struct { int pad[0x48D]; int field; } S;\nextern S D_00130000;\nextern void func_00100100(void);\n"
             "int FUN_00100000(void) { func_00100100(); return D_00130000.field; }\n")
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", c, WRAPPED)
        self.assertEqual(self.run_mode("check")[0], 0)
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", c.replace("0x48D", "0x48C"), WRAPPED)
        code, out = self.run_mode("check")
        self.assertEqual(code, 1)
        self.assertIn("c-data 0x00131230", out)

    def test_allow_list_explains_and_goes_stale(self):
        wrong = AGREES.replace("00100100", "00100200")
        allow = ("FUN_00100000\tc-call\tfunc_00100200\ton purpose\n"
                 "FUN_00100000\tcode-call\tfunc_00100100\ton purpose\n")
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", wrong, WRAPPED, allow)
        self.assertEqual(self.run_mode("check")[0], 0)
        # Fixed C with the reasons left behind: the stale rows fail the check.
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", AGREES, WRAPPED, allow)
        code, out = self.run_mode("check")
        self.assertEqual(code, 1)
        self.assertIn("nothing differs any more", out)

    def test_allow_list_needs_a_reason(self):
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", AGREES, WRAPPED, "FUN_00100000\tc-call\t*\t\n")
        with self.assertRaises(SystemExit):
            self.run_mode("check")

    def test_rows_hostgen_does_not_read_are_left_out(self):
        (self.gen / "report.json").write_text(json.dumps({"candidates_used": ["FUN_00100040"]}))
        self.hand("FUN_00100000\ta.c\nFUN_00100040\tb.c\n", AGREES.replace("00100100", "00100200"), WRAPPED)
        self.assertEqual(self.run_mode("check")[0], 0)

    def test_no_retail_program_skips(self):
        (self.tmp / "decomp" / "game.elf").unlink()
        self.assertEqual(self.run_mode("check")[0], 77)


if __name__ == "__main__":
    unittest.main()

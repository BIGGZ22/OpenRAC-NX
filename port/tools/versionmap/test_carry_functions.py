# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""carry_functions.py on a small made-up game: two versions, a map each way, a generated folder each.
The carry runs the source through Clang's preprocessor, so these tests need Clang."""
import io
import json
import shutil
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path

import carry_functions

HEADER = """typedef int s32;
typedef struct Thing { s32 a; s32 b; } Thing;
typedef struct Unused { s32 z; } Unused;
#define Readable func_00100200
"""

SOURCE = """#include "common.h"
extern int D_00150000;
extern int Readable(void);
static int helper(Thing *t) { return t->a + D_00150000; }
int counter = 3;
int func_00100100(Thing *t) {
    counter++;
    return helper(t) + Readable() + func_00100400(t);
}
void func_00100300(Unused *u) { u->z = 0; }
int func_00100400(Thing *t) { return t->b; }
"""

TARGET = """INCLUDE_ASM("asm/nonmatchings", FUN_00100080);
INCLUDE_ASM("asm/nonmatchings", FUN_00100280);
INCLUDE_ASM("asm/nonmatchings", FUN_00100480);
"""

FUNCTIONS = """static const openrac_fn_entry k_exe[] = {
%s};
"""


def entry(address: int, name: str) -> str:
    return f'    {{0x{address:08X}u, (openrac_host_fn){name}, "{name}"}},\n'


class Fixture:
    def __init__(self, root: Path):
        self.root = root
        pal, us = root / "pal", root / "us"
        (pal / "include").mkdir(parents=True)
        (pal / "src").mkdir()
        (pal / "include/common.h").write_text(HEADER)
        (pal / "src/a.c").write_text(SOURCE)
        (us / "src").mkdir(parents=True)
        (us / "src/b.c").write_text(TARGET)
        (us / "config").mkdir()
        (us / "config/functions.tsv").write_text(
            "# name\tkind\tsize\tfingerprint\tlevels\tplaces\nFUN_00100480\texe\t24\t0\t0\t\n")
        port_pal, port_us = root / "port/pal", root / "port/us"
        port_pal.mkdir(parents=True)
        (port_us / "hand/functional").mkdir(parents=True)
        (port_pal / "hostgen.json").write_text(json.dumps({
            "id": "x-pal", "title": "X (PAL)", "source": str(pal), "sources": ["src"], "includes": ["include", "src"]}))
        (port_us / "hostgen.json").write_text(json.dumps({
            "id": "x-us", "title": "X (US)", "source": str(us), "sources": ["src"], "includes": ["src"],
            "places": "config/functions.tsv", "hand": "hand/hand.tsv"}))
        (port_us / "hand/hand.tsv").write_text("# name\tfile\nFUN_00100999\tfunctional/FUN_00100999.c\n")
        (port_us / "hand/functional/FUN_00100999.c").write_text("int FUN_00100999(void) { return 0; }\n")
        for gen, rows in ((root / "gen/pal", [(0x100100, "func_00100100"), (0x100200, "func_00100200"),
                                              (0x100300, "func_00100300"), (0x100400, "func_00100400")]),
                          (root / "gen/us", [])):
            gen.mkdir(parents=True)
            (gen / "functions.c").write_text(FUNCTIONS % "".join(entry(a, n) for a, n in rows))
            (gen / "stubs.c").write_text("")
            (gen / "report.json").write_text("{}")
        boot_to = {"functions": {"00100100": ["00100080", "same", 8, 8], "00100200": ["00100180", "same", 8, 8],
                                 "00100300": ["00100280", "near", 8, 8], "00100400": ["00100380", "same", 8, 8]},
                   "data": {"00150000": "00140000"}}
        boot_back = {"functions": {"00100080": ["00100100", "same", 8, 8], "00100280": ["00100300", "near", 8, 8]},
                     "data": {}}
        (root / "to.json").write_text(json.dumps({"from": "x/pal", "to": "x/us", "programs": {"boot": boot_to}}))
        (root / "back.json").write_text(json.dumps({"from": "x/us", "to": "x/pal", "programs": {"boot": boot_back}}))

    def args(self, *more: str) -> list[str]:
        r = self.root
        return ["--target", str(r / "port/us/hostgen.json"), "--source", str(r / "port/pal/hostgen.json"),
                "--to-map", str(r / "to.json"), "--back-map", str(r / "back.json"), "--gen", str(r / "gen/us"),
                "--source-gen", str(r / "gen/pal"), "--out", str(r / "out"), "--no-check", "--jobs", "1", *more]


class ExtractTest(unittest.TestCase):
    def test_only_what_the_function_needs(self):
        text = HEADER.replace("#define Readable func_00100200\n", "") + SOURCE.replace("#include \"common.h\"\n", "") \
            .replace("Readable", "func_00100200")
        body, own, aliases = carry_functions.extract(text, "func_00100100")
        self.assertIn("static int helper(Thing *t)", body)
        self.assertIn("int func_00100400(Thing *t);", body)      # defined after it: its declaration, ahead of it
        self.assertLess(body.index("int func_00100400(Thing *t);"), body.index("int func_00100100("))
        self.assertIn("extern int counter;", body)               # data defined here becomes a declaration
        self.assertNotIn("Unused", body)
        self.assertNotIn("func_00100300", body)
        self.assertEqual(own, ["Thing", "helper", "s32"])
        self.assertEqual(aliases, ["func_00100100"])

    def test_candidate_declarations_win(self):
        text = ("extern char D_00150000[];\nint openrac_mark;\nextern int D_00150000;\n"
                "int f(void) { return D_00150000; }\n")
        body, _own, _aliases = carry_functions.extract(text, "f", "openrac_mark")
        self.assertIn("extern int D_00150000;", body)
        self.assertNotIn("char D_00150000", body)

    def test_a_definition_under_an_assembler_name(self):
        text = 'int f_impl(int a) __asm__("func_00100100");\nint f_impl(int a) { return a; }\n'
        body, _own, aliases = carry_functions.extract(text, "func_00100100")
        self.assertEqual(aliases, ["f_impl", "func_00100100"])
        self.assertIn("int f_impl(int a) { return a; }", body)

    def test_records_by_value(self):
        body = ("typedef struct { int a; } Goal;\ntypedef struct P { int b; } *PP;\n"
                "float g(Goal goal);\nint h(PP p, Goal *q);\nint f(void) { Goal x; return g(x) + h(0, &x); }\n")
        self.assertEqual(carry_functions.records_by_value(body, ["Goal", "P", "PP"], ["f"]), ["g (Goal)"])

    def test_rename_leaves_strings(self):
        self.assertEqual(carry_functions.rename('Thing t; char *s = "Thing";', ["Thing"], "_c1"),
                         'Thing_c1 t; char *s = "Thing";')


@unittest.skipUnless(shutil.which("clang"), "needs Clang")
class CarryTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.fx = Fixture(Path(self.tmp.name))

    def tearDown(self):
        self.tmp.cleanup()

    def run_carry(self, *more):
        with redirect_stdout(io.StringIO()) as out:
            self.assertEqual(carry_functions.main(self.fx.args(*more)), 0)
        return json.loads(out.getvalue().strip().splitlines()[-1])

    def read(self, name):
        return json.loads((self.fx.root / "out" / name).read_text())

    def test_lists(self):
        summary = self.run_carry()
        self.assertEqual(summary, {"carried": 1, "near": 1, "write": 1, "skipped": 0, "covered": 0})
        self.assertEqual(self.read("near.json")[0]["us_name"], "FUN_00100280")
        self.assertEqual(self.read("near.json")[0]["pal_name"], "func_00100300")
        self.assertEqual(self.read("write.json")[0], {"us_name": "FUN_00100480", "us_asm": None, "bytes": 24,
                                                      "kind": "exe", "pal_name": None, "pair": None})
        self.assertFalse((self.fx.root / "port/us/hand/functional/FUN_00100080.c").exists())   # not without --write

    def test_write(self):
        summary = self.run_carry("--write")
        self.assertEqual(summary["stale_removed"], 1)
        hand = self.fx.root / "port/us/hand"
        self.assertFalse((hand / "functional/FUN_00100999.c").exists())
        rows = [line for line in (hand / "hand.tsv").read_text().splitlines() if not line.startswith("#")]
        self.assertEqual(rows, ["FUN_00100080\tfunctional/FUN_00100080.c"])
        text = (hand / "functional/FUN_00100080.c").read_text()
        self.assertIn("func_00100100, matched C there", text)
        self.assertIn("int FUN_00100080(Thing_c100080 *t)", text)
        self.assertIn("static int helper_c100080(Thing_c100080 *t)", text)
        self.assertIn("D_00140000", text)                         # the data, at this version's address
        self.assertIn("func_00100180()", text)                    # the readable name, carried by address
        self.assertIn("func_00100380(t)", text)
        self.assertNotIn("func_00100100", text.split("*/", 1)[1])

    def test_write_again(self):
        self.run_carry("--write")
        hand = self.fx.root / "port/us/hand"
        first = (hand / "functional/FUN_00100080.c").read_text()
        # The generated folder now says the function has C: the tool still carries its own again.
        gen = self.fx.root / "gen/us/functions.c"
        gen.write_text(FUNCTIONS % entry(0x100080, "FUN_00100080"))
        summary = self.run_carry("--write")
        self.assertEqual((summary["carried"], summary["stale_removed"], summary["withdrawn"]), (1, 0, 0))
        self.assertEqual((hand / "functional/FUN_00100080.c").read_text(), first)
        # And what the source no longer has C for goes, with its row.
        (self.fx.root / "pal/src/a.c").write_text(SOURCE.replace("int func_00100100(Thing *t) {", "int other(Thing *t) {"))
        summary = self.run_carry("--write")
        self.assertEqual(summary["withdrawn"], 1)
        self.assertFalse((hand / "functional/FUN_00100080.c").exists())
        self.assertNotIn("FUN_00100080", (hand / "hand.tsv").read_text())


if __name__ == "__main__":
    unittest.main()

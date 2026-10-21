"""The behaviour catalogue on synthetic inputs: small names.tsv, functions.tsv, C and headers; no disc data."""

import json
import unittest

import catalogue
from catalogue import Inputs, build, definitions, field_comments, pvar_type, render_json, render_md, render_tsv, signature

HEADER = """
typedef unsigned char u8; typedef unsigned short u16; typedef int s32; typedef unsigned int u32;
typedef float f32; typedef short s16;
struct Moby {
    u8 pad0[0x74];
    void (*update)(struct Moby *moby);
    u8 *pvars;
    u8 pad7C[0x2A];
    s16 oclass;
    u8 padA8[0x58];
};
typedef struct Moby Moby;
"""

PVARS = """
struct update20 { // 0x8
\t/* 0x00 */ int timer;
\t/* 0x04 */ float speed; // how fast it goes
};
typedef struct update20 PVar_Moby20_Thing; /* A thing (class 20) */
"""

SHARED = """#include "moby.h"
extern void func_00100200(Moby *m, int n);
extern void helper_r(Moby *m) __asm__("func_L00_00100300");
typedef struct {
    s32 timer;     /* frames left */
    f32 speed;
} TestVars;

/* Updates classes 10 and 11 (a comment with (parentheses) in it). */
void TestUpdate(Moby *m) {
    TestVars *v = (TestVars *)m->pvars;
    if (v->timer) {
        v->timer--;
    }
    helper_r(m);
    FastVecAdd(0, 0, 0);
    func_00100200(m, 1);
}

void func_L00_00100400_r(Moby *m) __asm__("func_L00_00100400");
void func_L00_00100400_r(Moby *m) {
}

void Quiet(void) {
}

INCLUDE_ASM("asm/x", func_L00_00100500);
INCLUDE_ASM("asm/x", func_L00_00100600);
"""

CANDIDATE = """void func_L00_00100500(Moby *m) {
    ((struct update20 *)m->pvars)->timer = 0;
}
"""

NAMES_TSV = """# name\troles (role:levels)
func_L00_00100000\tUpdateMoby_10:00/01,UpdateMoby_11:01
func_L00_00100400\tUpdateMoby_12:00
func_00100700\tUpdateMoby_13:00/01,InitCamera_0:00
func_L00_00100500\tUpdateMoby_20:01
func_L00_00100600\tUpdateMoby_21:00
func_L00_00100800\tUpdateMoby_22:00
"""

FUNCTIONS_TSV = """# name\tkind\tsize\tfingerprint\tlevels\tplaces (level:address)
func_L00_00100000\tshared\t64\t0\t3\t00:00100000,01:00200000,02:00300000
func_L00_00100400\tlevel\t8\t0\t1\t00:00100400
"""

LIBRARIES_PAL = """# symbol\tlibrary\tname\tport\tnotes\tsignature
func_L00_00100400\tmoby\tQuietUpdate\thook\ta hook point\tvoid (unsigned int)
func_L00_00100800\tdraw\tSomeWrap\twrap\tthe port draws it
"""

LIBRARIES_NTSC = """FUN_L00_00100abc\tmoby\tQuietUpdate\thook\ta hook point\tvoid (unsigned int)
"""

SPLIT = """# function\tvariant\tplaces
func_L00_00100000\tfunc_L00_00100010\t02:00300000
"""

SDK = '#define OPENRAC_RAC1_SIG_QuietUpdate "void(u32)"\n'


def inputs(**changes) -> Inputs:
    values = dict(
        names_tsv=NAMES_TSV, functions_tsv=FUNCTIONS_TSV,
        names_h={"TestUpdate": "func_L00_00100000", "Quiet": "func_00100700", "FastVecAdd": "func_001F9BD8",
                 "SpinHelper": "func_L00_00100300"},
        sources=[("games/rac1/src/a.c", SHARED)],
        candidates={"func_L00_00100500": "games/rac1/nonmatching/functional/b.c"},
        extra_sources=[("games/rac1/nonmatching/functional/b.c", CANDIDATE)],
        libraries_pal=LIBRARIES_PAL, libraries_ntsc=LIBRARIES_NTSC, split_places=SPLIT,
        headers=[("games/rac1/include/moby.h", HEADER)], pvars_header=("games/rac1/include/moby_pvars.h", PVARS),
        class_names={10: "bolt", 11: "big_bolt", 20: "thing"}, api={"FastVecAdd": "out = a + b"}, sdk_header=SDK)
    values.update(changes)
    return Inputs(**values)


class ReadingTests(unittest.TestCase):
    def test_signatures(self):
        self.assertEqual(signature("void", "Moby *m", {}), "void(u32)")
        self.assertEqual(signature("void", "void", {}), "void()")
        self.assertEqual(signature("int", "int mode, float a, unsigned short b, struct Moby *m", {}), "i32(i32,f32,u16,u32)")
        self.assertIsNone(signature("void", "Vec4f v", {}))          # a record by value
        self.assertIsNone(signature("void", "int n, ...", {}))       # varargs
        self.assertIsNone(signature("double", "void", {}))
        self.assertEqual(catalogue.c_signature("void(u32,f32)"), "void (unsigned int, float)")
        self.assertEqual(catalogue.c_signature("void()"), "void (void)")

    def test_definitions_one_by_one(self):
        defs = definitions(SHARED)
        self.assertEqual(set(defs), {"TestUpdate", "func_L00_00100400_r", "Quiet"})
        self.assertEqual(defs["TestUpdate"][:2], ("void", "Moby *m"))
        self.assertIn("helper_r(m);", defs["TestUpdate"][2])
        self.assertNotIn("if", defs)

    def test_pvar_type_and_comments(self):
        self.assertEqual(pvar_type(definitions(SHARED)["TestUpdate"][2]), ["TestVars"])
        self.assertEqual(pvar_type("struct Foo *f; f = (struct Foo *)m->pvars;"), ["Foo"])
        self.assertEqual(pvar_type("((Bar *)m->pvars)->x = 1;"), ["Bar"])
        notes = field_comments("\t/* 0x04 */ float speed; // how fast\n\ts32 timer;  /* left */\n")
        self.assertEqual(notes["speed"], ("how fast", 4))
        self.assertEqual(notes["timer"], ("left", None))

    def test_ranges(self):
        self.assertEqual(catalogue._ranges([0, 1, 2, 5, 7, 8]), "0-2, 5, 7-8")
        self.assertEqual(catalogue._ranges([]), "none")


class BuildTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cat = build(inputs())

    def test_moby_fields_from_the_header(self):
        moby = self.cat["moby"]
        self.assertEqual(moby["class_field"], {"name": "oclass", "offset": 0xA6, "type": "s16"})
        self.assertEqual(moby["update_field"]["offset"], 0x74)
        self.assertEqual(moby["pvars_field"]["offset"], 0x78)

    def test_shared_update_function(self):
        f = self.cat["functions"]["func_L00_00100000"]
        self.assertEqual(f["name"], "TestUpdate")
        self.assertEqual(f["classes"], [10, 11])
        self.assertEqual(f["levels"], [0, 1, 2])  # where its code is, from functions.tsv
        self.assertEqual(f["split"], {"func_L00_00100010": [2]})
        self.assertEqual(f["source"], {"kind": "decomp", "file": "games/rac1/src/a.c", "defined_as": "TestUpdate"})
        self.assertEqual(f["signature"], "void(u32)")
        self.assertEqual(f["hook"]["status"], "needs_hook_row")
        self.assertEqual(f["hook"]["proposed"], "TestUpdate")
        self.assertEqual(f["hook"]["row"].split("\t")[:4], ["func_L00_00100000", "moby", "TestUpdate", "hook"])
        self.assertTrue(f["hook"]["row"].endswith("\tvoid (unsigned int)"))
        self.assertIn("not the places split_places.tsv gives to func_L00_00100010 (levels 2)", f["hook"]["covers"])
        c10 = next(c for c in self.cat["classes"] if c["id"] == 10)
        self.assertEqual(c10["shared_with"], [11])
        self.assertEqual(c10["levels"], [0, 1])
        self.assertEqual(c10["pvars"], ["TestVars"])

    def test_calls_through_aliases_and_names(self):
        f = self.cat["functions"]["func_L00_00100000"]
        self.assertEqual(f["calls"], ["func_L00_00100300", "func_001F9BD8", "func_00100200"])
        helpers = self.cat["helpers"]
        self.assertEqual(helpers["func_L00_00100300"]["name"], "SpinHelper")
        self.assertTrue(helpers["func_001F9BD8"]["engine_helper"])
        self.assertFalse(helpers["func_00100200"]["engine_helper"])
        self.assertFalse(helpers["func_00100200"]["in_port"])

    def test_hook_statuses(self):
        fs = self.cat["functions"]
        quiet = fs["func_L00_00100400"]
        self.assertEqual(quiet["hook"]["status"], "hook_point")
        self.assertEqual(quiet["hook"]["point"], "QuietUpdate")
        self.assertIsNone(quiet["hook"]["missing"])  # the NTSC table has it too
        self.assertEqual(self.cat["hook_points"]["QuietUpdate"],
                         {"symbol": "func_L00_00100400", "ntsc_symbol": "FUN_L00_00100abc", "signature": "void(u32)",
                          "in_sdk_header": True})
        bare = fs["func_00100700"]
        self.assertEqual((bare["signature"], bare["hook"]["status"]), ("void()", "needs_hook_row"))
        self.assertIn("does not get the moby", bare["hook"]["missing"])
        self.assertEqual(fs["func_L00_00100500"]["source"]["kind"], "candidate")
        self.assertEqual(fs["func_L00_00100500"]["hook"]["proposed"], "MobyUpdate_20")
        self.assertEqual(fs["func_L00_00100600"]["hook"]["status"], "needs_c")
        self.assertEqual(fs["func_L00_00100800"]["hook"]["status"], "wrapped")
        self.assertEqual(fs["func_L00_00100800"]["port_row"], "wrap")

    def test_pvar_structs(self):
        structs = self.cat["pvar_structs"]
        header = structs["update20"]
        self.assertEqual((header["size"], header["declared_size"], header["classes"]), (8, 8, [20]))
        self.assertEqual(header["fields"][1], {"offset": 4, "declared_offset": None, "type": "f32", "name": "speed",
                                               "comment": "how fast it goes"})
        ours = structs["TestVars"]
        self.assertEqual((ours["size"], ours["file"], ours["classes"]), (8, "games/rac1/src/a.c", [10, 11]))
        self.assertEqual(ours["fields"][0]["comment"], "frames left")
        thing = next(c for c in self.cat["classes"] if c["id"] == 20)
        self.assertEqual(thing["pvars"], ["update20"])  # the header's, and the candidate C reads the same

    def test_counts(self):
        n = self.cat["counts"]
        self.assertEqual((n["classes"], n["update_functions"], n["shared_update_functions"]), (7, 6, 1))
        self.assertEqual(n["update_functions_by_hook_status"],
                         {"hook_point": 1, "needs_c": 1, "needs_hook_row": 3, "wrapped": 1})

    def test_proposed_names_do_not_clash(self):
        cat = build(inputs(names_h={"QuietUpdate": "func_00100700", "TestUpdate": "func_L00_00100000"}))
        self.assertEqual(cat["functions"]["func_00100700"]["hook"]["proposed"], "QuietUpdate_2")


class RenderTests(unittest.TestCase):
    def test_outputs(self):
        cat = build(inputs())
        self.assertEqual(json.loads(render_json(cat)), cat)
        tsv = [line.split("\t") for line in render_tsv(cat).splitlines() if not line.startswith("#")]
        self.assertEqual(tsv[0], list(catalogue.TSV_COLUMNS))
        self.assertEqual(len(tsv) - 1, 7)
        self.assertTrue(all(len(row) == len(tsv[0]) for row in tsv))
        md = render_md(cat)
        self.assertIn("| `func_L00_00100400` |", md)
        self.assertIn("**`QuietUpdate`**", md)
        self.assertIn("`s16 oclass` at +0xa6", md)
        self.assertIn("### TestVars", md)
        self.assertEqual(render_md(build(inputs())), md)  # the same bytes each time


if __name__ == "__main__":
    unittest.main()

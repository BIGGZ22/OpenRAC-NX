import re
import tempfile
import unittest
from pathlib import Path

import behaviour
from behaviour import Translator, class_script, load_api, load_macros, moby_data_script
from cparse import Parser, Types, function_source, lex

HAVE_C = (behaviour.DECOMPILATION / "config" / "overlays" / "names.tsv").is_file()

HEADER = """
typedef unsigned char u8; typedef unsigned short u16; typedef int s32; typedef float f32; typedef short s16;
typedef struct { f32 x; f32 y; f32 z; f32 w; } Vec4f;
struct Moby {
    Vec4f bsphere; Vec4f pos; u8 state; u8 group; u8 unk22; u8 unk23;
    void *pclass; void *next; f32 scale; u8 unk30; u8 unk31; s16 unk32; u16 flags; u16 unk36;
    unsigned long spawn_frame; Vec4f rot; u8 pad50[0x24]; void (*update)(struct Moby *m); void *pvars;
};
typedef struct Moby Moby;
typedef struct { u8 pad0[4]; f32 speed; s16 timer; u8 count; u8 pad; f32 dir[3]; } Vars;
extern f32 D_0015EE6C;
extern void func_001F9BD8(void *, void *, void *);
extern int func_001F9908(void *);
"""


def translate(body: str, params: str = "Moby *moby") -> behaviour.Result:
    source = Path(tempfile.mkdtemp()) / "t.c"
    source.write_text(HEADER + f"void func_L00_00000001({params}) {{\n{body}\n}}\n")
    tr = Translator(behaviour.load_types(source), load_api(), load_macros())
    return tr.function("func_L00_00000001", source)


class StructTests(unittest.TestCase):
    def test_moby_layout_follows_the_header(self):
        types = Types()
        types.read(HEADER)
        moby = types.structs["Moby"]
        self.assertEqual(moby.at(0x10).name, "pos")
        self.assertEqual(moby.at(0x38).name, "spawn_frame")  # a u64 aligns to 8
        self.assertEqual(moby.at(0x40).name, "rot")
        self.assertEqual(moby.named("pvars").offset, 0x78)  # the function pointer before it takes 4 bytes

    @unittest.skipUnless(HAVE_C, "decompilation not in this checkout")
    def test_real_moby_header(self):
        moby = behaviour.load_types(None).structs["Moby"]
        self.assertEqual((moby.size, moby.named("state").offset, moby.named("pvars").offset), (0x100, 0x20, 0x78))
        self.assertEqual(moby.named("oclass").offset, 0xA6)

    def test_function_found_by_name(self):
        text = "extern void f(int);\nstatic int g(int a) { return a; }\nvoid f(int x) { if (x) { g(x); } }\n"
        self.assertEqual(function_source(text, "f")[2].strip(), "if (x) { g(x); }")
        self.assertIsNone(function_source(text, "missing"))

    def test_parser_keeps_going_past_a_bad_statement(self):
        types = Types()
        stmts = Parser(lex("a = 1; do { a++; } while (a < 3); b = 2;"), types).parse_block_body()
        self.assertEqual([s[0] for s in stmts], ["expr", "raw", "expr"])


class TranslateTests(unittest.TestCase):
    def test_switch_becomes_match(self):
        r = translate("switch (moby->state) { case 0: moby->state = 1; break; case 1: case 2: moby->unk31 = 0; break; "
                      "default: break; }")
        self.assertIn("match moby.state:\n\t\t0:\n\t\t\tmoby.state = 1\n\t\t1, 2:\n\t\t\tmoby.unk31 = 0\n\t\t_:\n\t\t\tpass", r.text)
        self.assertEqual(r.untranslated, [])

    def test_break_inside_a_case_uses_the_one_pass_loop(self):
        r = translate("switch (moby->state) { case 0: if (moby->unk31) break; moby->state = 2; break; case 1: break; }")
        self.assertIn("for _sw in 1:", r.text)
        self.assertIn("\t\t\t\tbreak", r.text)
        self.assertNotIn("match", r.text)

    def test_fallthrough_is_not_translated(self):
        r = translate("switch (moby->state) { case 0: moby->unk31 = 1; case 1: moby->unk31 = 2; break; }")
        self.assertEqual(len(r.untranslated), 1)
        self.assertIn("falls through", r.untranslated[0][1])
        self.assertIn("pass  # not translated", r.text)

    def test_unsigned_bytes_wrap_and_pointers_compare_to_null(self):
        r = translate("Moby *o; o = moby->next; if (o == 0) { moby->unk23 -= 1; moby->unk23++; }")
        self.assertIn("if o == null:", r.text)
        self.assertIn("moby.unk23 = (moby.unk23 - 1) & 0xff", r.text)
        self.assertIn("moby.unk23 = (moby.unk23 + 1) & 0xff", r.text)

    def test_vector_members_index_and_whole_vectors_copy(self):
        r = translate("moby->pos.z = moby->pos.z + 1.0f; moby->rot = moby->pos;")
        self.assertIn("moby.pos[2] = moby.pos[2] + 1.0", r.text)
        self.assertIn("RcEngine.qcopy(moby.rot, moby.pos)", r.text)

    def test_counting_loop_and_globals(self):
        r = translate("int i; float t = 0.0f; for (i = 0; i < 4; i++) { t += D_0015EE6C; }")
        self.assertIn("for i_2 in range(0, 4):", r.text)
        self.assertIn("t += RcEngine.dt", r.text)

    def test_pvars_struct_and_field_references(self):
        r = translate("Vars *v; v = (Vars *)moby->pvars; if (FastDecTimer(&v->timer)) { v->speed = v->dir[1]; }")
        self.assertEqual(r.pvar_struct, "Vars")
        self.assertIn('RcEngine.dec_timer(RcRef.of(v, "timer"))', r.text)
        self.assertIn("v.speed = v.dir[1]", r.text)

    def test_scalar_local_whose_address_is_taken_is_boxed(self):
        r = translate("float f = 1.0f; func_001F9908(&f); f = f + 1.0f;")
        self.assertIn("var f := RcRef.new()", r.text)
        self.assertIn("f.v = f.v + 1.0", r.text)

    def test_unknown_calls_are_counted_not_hidden(self):
        r = translate("func_L00_0099999(moby, 3);")
        self.assertIn('RcEngine.unmapped(&"func_L00_0099999", [moby, 3])', r.text)
        self.assertEqual(r.unmapped_statements, 1)
        self.assertEqual(r.unmapped_calls["func_L00_0099999"], 1)

    def test_raw_memory_access_is_reported(self):
        r = translate("*(float *)((char *)moby + 0x10) = 1.0f; moby->state = 3;")
        self.assertEqual((r.statements, len(r.untranslated)), (2, 1))
        self.assertIn("dereference of a raw pointer", r.untranslated[0][1])
        self.assertIn("moby.state = 3", r.text)

    def test_pre_decrement_in_a_condition_is_done_first(self):
        r = translate("if (--moby->unk23 == 0xFF) { moby->state = 1; }")
        self.assertIn("moby.unk23 = (moby.unk23 - 1) & 0xff\n\tif moby.unk23 == 0xFF:", r.text)

    def test_view_struct_members_map_to_moby_fields_by_offset(self):
        source = Path(tempfile.mkdtemp()) / "t.c"
        source.write_text(HEADER + "typedef struct { u8 pad0[0x10]; f32 p[4]; u8 st; } View;\n"
                          "void func_L00_00000002(View *m) { m->st = 3; m->p[2] = 1.0f; }\n")
        r = Translator(behaviour.load_types(source), load_api(), load_macros()).function("func_L00_00000002", source)
        self.assertIn("moby.state = 3", r.text)  # the update function's parameter is always named moby
        self.assertIn("moby.pos[2] = 1.0", r.text)


class ApiTests(unittest.TestCase):
    def test_every_api_name_exists_in_the_engine_stub(self):
        engine = (behaviour.SCRIPTS / "rc_engine.gd").read_text()
        for entry in load_api().values():
            gd = "moby_at" if entry.gd == "moby_table" else entry.gd  # translated as a call, not a read
            self.assertTrue(re.search(rf"static (?:func|var) {gd}\b", engine), f"{entry.symbol} -> {gd} is not in rc_engine.gd")

    def test_moby_data_script_is_up_to_date(self):
        self.assertEqual((behaviour.SCRIPTS / "rc_moby_data.gd").read_text(), moby_data_script())


@unittest.skipUnless(HAVE_C, "decompilation not in this checkout")
class ClassScriptTests(unittest.TestCase):
    def test_crate_fragment_script(self):
        text, result = class_script(357, 0)
        self.assertTrue(text.startswith("extends RcMoby\n## Moby class 357: crate_fragment."))
        self.assertIn("@export var velocity: PackedFloat32Array", text)
        self.assertIn("## +0x10 f32[4]", text)
        self.assertIn("func rc_update(moby: RcMobyData) -> void:", text)
        self.assertEqual(result.pvar_struct, "DebrisData2B0D30")
        self.assertGreaterEqual(result.translated / result.statements, 0.9)
        self.assertNotIn("\n    ", text)  # tabs only

    def test_the_three_prototype_classes_translate(self):
        for class_id in (357, 749, 500):
            _, result = class_script(class_id, 0)
            self.assertGreater(result.statements, 20)
            self.assertLessEqual(len(result.untranslated), 3, class_id)

    def test_class_without_c_is_skipped(self):
        self.assertIsNone(class_script(1, 0))

    def test_attach_is_idempotent(self):
        with tempfile.TemporaryDirectory() as d:
            level = Path(d)
            (level / "mobys").mkdir()
            scene = level / "mobys" / "moby_9.tscn"
            scene.write_text('[gd_scene format=3]\n\n[ext_resource type="PackedScene" path="res://m.glb" id="model"]\n\n'
                             '[node name="Moby_9" type="Node3D"]\nmetadata/rc1_class = 9\n')
            for _ in range(2):
                behaviour.attach(level, 9, "res://b.gd")
            text = scene.read_text()
            self.assertEqual(text.count('id="behaviour"'), 1)
            self.assertEqual(text.count("script = ExtResource"), 1)
            self.assertLess(text.index("[ext_resource type=\"Script\""), text.index("[node name="))


if __name__ == "__main__":
    unittest.main()

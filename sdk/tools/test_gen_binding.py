# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests of gen_binding.py: python3 -m unittest discover -s sdk/tools -p test_gen_binding.py"""
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_binding  # noqa: E402

ROOT = gen_binding.ROOT

CATALOGUE = {
    "format": 1,
    "game": "rac1",
    "moby": {"class": "0xAA", "class_type": "i16", "pvars": "0x78"},
    "classes": [
        {"name": "Crate", "class": 500, "update": {"pal": "func_L05_00200000", "ntsc": "FUN_L05_00200000"},
         "pvars": [{"name": "timer", "type": "f32", "offset": "0x10"},
                   {"name": "kind", "type": "u8", "offset": "0x4", "readonly": True}]},
        {"name": "Sign", "class": 12, "pvars": []},
    ],
}


class Binding(unittest.TestCase):
    def test_committed_outputs_are_the_tables(self):
        self.assertEqual(gen_binding.main(["rac1", "--check"]), 0)
        self.assertEqual(gen_binding.main(["--table", str(ROOT / "port/tests/script_binding_test.json"),
                                           "--c", str(ROOT / "port/tests/script_binding_test.c"), "--check"]), 0)

    def test_hooks_are_the_libraries_hook_rows(self):
        b = gen_binding.load("rac1")
        self.assertEqual(gen_binding.check_rows(b), [])
        names = [h["name"] for h in b["hooks"]]
        self.assertEqual(names, ["Pad_ProcessReport", "Hero_UpdateMotionInput", "Hero_TurnToHeading",
                                 "Hero_SetPlanarVelocity", "FollowCamera_ApplyOverrides", "Hero_AdvanceAnimation"])
        # A symbol that is not the row's is found.
        b["hooks"][0] = dict(b["hooks"][0], symbols={"pal": "func_00000000", "ntsc": "FUN_00217328"})
        problems = gen_binding.check_rows(b)
        self.assertTrue(any("func_00000000" in p for p in problems), problems)

    def test_signatures(self):
        self.assertEqual(gen_binding.parse_signature("void()"), ("void", []))
        self.assertEqual(gen_binding.parse_signature("f32(i32,u32,f32)"), ("f32", ["i32", "u32", "f32"]))
        for bad in ["void", "int(i32)", "void(i33)", "void(i32, f32)"]:
            with self.assertRaises(gen_binding.BindingError):
                gen_binding.parse_signature(bad)

    def test_outputs(self):
        b = gen_binding.load("rac1")
        c = gen_binding.c_binding(b)
        self.assertIn("static void Hero_TurnToHeading_7(int32_t mode, float accel, float damping, float limit) {", c)
        self.assertIn('{"hero.state", OPENRAC_SV_I32, 1, k_field_5}', c)
        # hero.state: the hero's base on each disc (0x13F450, 0x13F350) and its offset, 0x2084.
        self.assertTrue("static const uint32_t k_field_5[] = {0x001414D4u, 0x001413D4u};" in c)
        hpp = gen_binding.cpp_header(b)
        self.assertIn("struct Hero_TurnToHeading {", hpp)
        self.assertIn('static constexpr const char* signature = "void(i32,f32,f32,f32)";', hpp)
        self.assertIn("using fn = void (*)(int32_t, float, float, float);", hpp)
        self.assertIn("using fn = void (*)();", hpp)
        types = gen_binding.luau_types(b)
        self.assertIn('type HookPoint = "Pad_ProcessReport" |', types)
        self.assertIn("declare openrac: {", types)
        self.assertIn("state: number, -- read-only;", types)
        self.assertNotIn("pvars", types)

    def test_catalogue(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "catalogue.json"
            path.write_text(json.dumps(CATALOGUE))
            b = gen_binding.load("rac1", path)
            names = [h["name"] for h in b["hooks"]]
            self.assertIn("Moby_Crate_Update", names)
            self.assertNotIn("Moby_Sign_Update", names)  # no update function: no hook point
            crate = next(h for h in b["hooks"] if h["name"] == "Moby_Crate_Update")
            self.assertEqual((crate["signature"], crate["params"]), ("void(u32)", ["moby"]))
            c = gen_binding.c_binding(b)
            self.assertIn('{"timer", OPENRAC_SV_F32, 0, 0x10u}', c)
            self.assertIn('{"kind", OPENRAC_SV_U8, 1, 0x4u}', c)
            self.assertIn('{"Crate", 500, 2, k_class_0_pvars}', c)
            self.assertIn("static const openrac_script_moby k_moby = {0xAAu, OPENRAC_SV_I16, 0x78u, 2, k_classes};", c)
            self.assertIn("pvars: (moby: number)", gen_binding.luau_types(b))
            self.assertIn("struct Moby_Crate_Update {", gen_binding.cpp_header(b))
            # Its update function is to be a hook row on each disc too.
            problems = gen_binding.check_rows(b)
            self.assertTrue(any("func_L05_00200000 Moby_Crate_Update" in p for p in problems), problems)
            # A catalogue of another game, a pvar of no known type: refused.
            path.write_text(json.dumps(dict(CATALOGUE, game="rac2")))
            with self.assertRaises(gen_binding.BindingError):
                gen_binding.load("rac1", path)
            bad = json.loads(json.dumps(CATALOGUE))
            bad["classes"][0]["pvars"][0]["type"] = "vec4"
            path.write_text(json.dumps(bad))
            with self.assertRaises(gen_binding.BindingError):
                gen_binding.load("rac1", path)

    def test_table_rules(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "t.json"
            base = {"format": 1, "game": "t", "discs": ["one"], "hooks": [], "fields": []}
            for broken in [
                {"hooks": [{"name": "A", "signature": "void()", "symbols": {}},
                           {"name": "A", "signature": "void()", "symbols": {}}]},
                {"hooks": [{"name": "A", "signature": "void(i32)", "params": ["x", "y"], "symbols": {}}]},
                {"fields": [{"name": "a", "type": "f64", "address": {"one": "0x10"}}]},
                {"fields": [{"name": "a", "type": "i32", "address": {"one": "0x10"}},
                            {"name": "a.b", "type": "i32", "address": {"one": "0x14"}}]},
            ]:
                path.write_text(json.dumps(dict(base, **broken)))
                with self.assertRaises(gen_binding.BindingError, msg=broken):
                    gen_binding.load("", None, path)


if __name__ == "__main__":
    unittest.main()

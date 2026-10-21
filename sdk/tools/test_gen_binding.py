# SPDX-License-Identifier: MIT
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

SIX = ["Pad_ProcessReport", "Hero_UpdateMotionInput", "Hero_TurnToHeading", "Hero_SetPlanarVelocity",
       "FollowCamera_ApplyOverrides", "Hero_AdvanceAnimation"]


def catalogue(classes):
    """A classes.json (editor/catalogue.py's format 1) of these classes."""
    return {"format": 1, "game": "t", "disc": "one",
            "moby": {"struct": "Moby", "size": 256, "class_field": {"name": "oclass", "offset": 166, "type": "s16"},
                     "update_field": {"name": "update", "offset": 116, "type": "void*"},
                     "pvars_field": {"name": "pvars", "offset": 120, "type": "u8*"}},
            "classes": classes}


def update(fn, name, hook, sig="void(u32)"):
    return {"function": fn, "name": name, "levels": [5], "hook": hook, "hook_status": "needs_hook_row",
            "signature": sig, "gets_moby": sig == "void(u32)", "shared_with": []}


CLASSES = [
    {"id": 500, "name": "crate", "levels": [5], "updates": [update("func_L05_00200000", "CrateUpdate", "Moby_Crate_Update")],
     "pvars": [{"struct": "Data", "size": 64, "checked": True, "fields": [
         {"name": "timer", "type": "f32", "offset": 16, "meaning": None},
         {"name": "kind", "type": "u8", "offset": 4, "meaning": None},
         {"name": "target", "type": "Moby*", "offset": 20, "meaning": None},
         {"name": "spot", "type": "Vec4", "offset": 32, "meaning": None},
         {"name": "big", "type": "s8[64]", "offset": 48, "meaning": None}]}]},
    # Two classes on one update: one hook point.
    {"id": 13, "name": "bolt", "levels": [5], "updates": [update("func_L05_00200100", "BoltUpdate", "Moby_Bolt_Update")], "pvars": []},
    {"id": 14, "name": "bolt_hexnut", "levels": [5], "updates": [update("func_L05_00200100", "BoltUpdate", "Moby_Bolt_Update")], "pvars": []},
    # No name for the function, nor for the class.
    {"id": 77, "name": None, "levels": [5], "updates": [update("func_L05_00200200", None, "Moby_Class77_Update")], "pvars": []},
    # A function named by another project for another use: the class's name is taken.
    {"id": 80, "name": "lamp", "levels": [5], "updates": [update("func_L05_00200300", "register_stream", "Moby_Lamp_Update", "void()")], "pvars": []},
    # Two functions whose names give one id: each gets its lowest class number.
    {"id": 90, "name": "door", "levels": [5], "updates": [update("func_L05_00200400", "DoorUpdate", "Moby_Door_Update")], "pvars": []},
    {"id": 91, "name": "door2", "levels": [5], "updates": [update("func_L05_00200500", "Door_Update", "Moby_Door91_Update")], "pvars": []},
    # Left out by the table.
    {"id": 99, "name": "skip", "levels": [5], "updates": [update("func_L05_00200600", "SkipUpdate", "Moby_Skip_Update")], "pvars": []},
]


def table(hooks=None, **more):
    t = {"format": 1, "game": "t", "discs": ["one", "two"],
         "hooks": hooks if hooks is not None else
         [{"id": "t.player.add", "name": "Test_Add", "signature": "i32(i32)", "params": ["x"],
           "symbols": {"one": "func_00100000", "two": "FUN_00100000"}}],
         "fields": []}
    t.update(more)
    return t


class Fixture:
    """A table, its catalogue, its other disc's symbols and its lock in a temporary folder."""

    def __init__(self, tmp: str, t: dict, classes=None):
        self.dir = Path(tmp)
        self.path = self.dir / "t.json"
        if classes is not None:
            (self.dir / "classes.json").write_text(json.dumps(catalogue(classes)))
            (self.dir / "two.tsv").write_text("# pairs\n" + "".join(
                f"{u['function']}\tFUN_{u['function'][5:]}\tsame\n" for c in classes for u in c["updates"]))
            t["catalogue"] = str(self.dir / "classes.json")
            t["catalogue_symbols"] = {"two": str(self.dir / "two.tsv")}
            t.setdefault("catalogue_exclude", {"func_L05_00200600": "a test's"})
        self.path.write_text(json.dumps(t))
        self.lock = self.dir / "t.ids.lock"

    def load(self):
        return gen_binding.load("", None, self.path, self.lock)

    def write_lock(self, b):
        self.lock.write_text(gen_binding.lock_text("t", b["hooks"]))


class Binding(unittest.TestCase):
    def test_committed_outputs_are_the_tables(self):
        self.assertEqual(gen_binding.main(["rac1", "--check"]), 0)
        self.assertEqual(gen_binding.main(["--table", str(ROOT / "port/tests/script_binding_test.json"),
                                           "--c", str(ROOT / "port/tests/script_binding_test.c"), "--check"]), 0)

    def test_hooks_are_the_libraries_hook_rows(self):
        b = gen_binding.load("rac1")
        self.assertEqual(gen_binding.check_rows(b), [])
        self.assertEqual([h["name"] for h in b["hooks"][:6]], SIX)
        self.assertEqual(b["hooks"][2]["id"], "rac1.player.turn_to_heading")
        # A symbol that is not the row's is found.
        b["hooks"][0] = dict(b["hooks"][0], symbols={"pal": "func_00000000", "ntsc": "FUN_00217328"})
        problems = gen_binding.check_rows(b)
        self.assertTrue(any("func_00000000" in p for p in problems), problems)

    def test_the_catalogues_update_functions_are_hook_points(self):
        b = gen_binding.load("rac1")
        cat = [h for h in b["hooks"] if h["origin"] == "catalogue"]
        self.assertGreater(len(cat), 600)
        bolt = next(h for h in cat if h["name"] == "Moby_Bolt_Update")
        self.assertEqual(bolt["id"], "rac1.moby.bolt.update")
        self.assertEqual(set(bolt["symbols"]), {"pal", "ntsc"})
        self.assertIn("func_L00_002A7078", bolt["aliases"])
        # Profile_Mark and the functions defined under another name are left out, with a reason.
        left = dict(b["left"])
        self.assertIn("func_001E9768", left)
        self.assertNotIn("Moby_Butterfly_Update", {h["name"] for h in b["hooks"]})
        # The one the port wraps too is a hook point (under the wrap: hostgen's __hooked).
        self.assertEqual(b["aliases"]["Moby_NanotechBall_Update"], "rac1.moby.nanotech_ball.update")
        # Every id of the committed lock is still given.
        locked = gen_binding.read_lock(ROOT / "sdk/bindings/rac1.ids.lock")
        self.assertTrue(set(locked) <= {h["id"] for h in b["hooks"]})

    def test_signatures(self):
        self.assertEqual(gen_binding.parse_signature("void()"), ("void", []))
        self.assertEqual(gen_binding.parse_signature("f32(i32,u32,f32)"), ("f32", ["i32", "u32", "f32"]))
        for bad in ["void", "int(i32)", "void(i33)", "void(i32, f32)"]:
            with self.assertRaises(gen_binding.BindingError):
                gen_binding.parse_signature(bad)

    def test_outputs(self):
        b = gen_binding.load("rac1")
        c = gen_binding.c_binding(b)
        self.assertIn("S2_POINT(Hero_TurnToHeading, 2) /* rac1.player.turn_to_heading */", c)
        self.assertIn('{"rac1.player.turn_to_heading", "void(i32,f32,f32,f32)", 4, k_S2_kinds, k_S2_names,', c)
        self.assertIn('{"Hero_TurnToHeading", "rac1.player.turn_to_heading"},', c)
        self.assertIn('{"hero.state", OPENRAC_SV_I32, 1, k_field_5}', c)
        # hero.state: the hero's base on each disc (0x13F450, 0x13F350) and its offset, 0x2084.
        self.assertIn("static const uint32_t k_field_5[] = {0x001414D4u, 0x001413D4u};", c)
        # The class of a moby at +0xA6 (s16), its pvars at +0x78.
        self.assertIn("static const openrac_script_moby k_moby = {0xA6u, OPENRAC_SV_I16, 0x78u,", c)
        hpp = gen_binding.cpp_header(b)
        self.assertIn("struct Hero_TurnToHeading {", hpp)
        self.assertIn('static constexpr const char* id = "rac1.player.turn_to_heading";', hpp)
        self.assertIn('static constexpr const char* signature = "void(i32,f32,f32,f32)";', hpp)
        self.assertIn("using fn = void (*)(int32_t, float, float, float);", hpp)
        self.assertIn("using turn_to_heading = rac1::Hero_TurnToHeading;", hpp)
        types = gen_binding.luau_types(b)
        self.assertIn('"rac1.player.turn_to_heading"', types)
        self.assertIn('| "Hero_TurnToHeading"', types)
        self.assertIn("declare openrac: {", types)
        self.assertIn("state: number, -- read-only;", types)
        self.assertIn("pvars: (moby: number)", types)
        self.assertIn("EXPERIMENTAL", types)
        ids = json.loads(gen_binding.launcher_ids([b]))
        self.assertEqual(ids["games"]["rac1"]["Hero_TurnToHeading"], "rac1.player.turn_to_heading")

    def test_catalogue_ids_and_pvars(self):
        with tempfile.TemporaryDirectory() as tmp:
            f = Fixture(tmp, table(), CLASSES)
            b = f.load()
            ids = {h["name"]: h["id"] for h in b["hooks"]}
            self.assertEqual(ids["Test_Add"], "t.player.add")
            self.assertEqual(ids["Moby_Crate_Update"], "t.moby.crate.update")
            self.assertEqual(ids["Moby_Bolt_Update"], "t.moby.bolt.update")  # one for both classes
            self.assertEqual(ids["Moby_Class77_Update"], "t.moby.class_77.update")
            self.assertEqual(ids["Moby_Lamp_Update"], "t.moby.lamp.update")
            self.assertEqual(ids["Moby_Door_Update"], "t.moby.door_90.update")
            self.assertEqual(ids["Moby_Door91_Update"], "t.moby.door_91.update")
            self.assertNotIn("Moby_Skip_Update", ids)
            self.assertEqual(b["left"], [("func_L05_00200600", "a test's")])
            bolt = next(h for h in b["hooks"] if h["name"] == "Moby_Bolt_Update")
            self.assertEqual(bolt["classes"], [13, 14])
            self.assertEqual(bolt["symbols"], {"one": "func_L05_00200100", "two": "FUN_L05_00200100"})
            lamp = next(h for h in b["hooks"] if h["name"] == "Moby_Lamp_Update")
            self.assertEqual((lamp["signature"], lamp["params"]), ("void()", []))
            crate = next(c for c in b["classes"] if c["class"] == 500)
            pv = {p["name"]: (p["type"], p["offset"], p["readonly"]) for p in crate["pvars"]}
            self.assertEqual(pv["timer"], ("f32", 16, False))
            self.assertEqual(pv["target"], ("u32", 20, True))  # a pointer: read-only
            self.assertEqual(pv["spot_y"], ("f32", 36, False))  # a vector's lanes
            self.assertNotIn("big", pv)  # a large array is left out
            c = gen_binding.c_binding(b)
            self.assertIn('{"timer", OPENRAC_SV_F32, 0, 0x10u}', c)
            self.assertIn('{"crate", 500, ', c)
            # Each disc's libraries.tsv gets the catalogue's rows in a block of their own.
            block = gen_binding.libraries_block(b, "two")
            self.assertIn("FUN_L05_00200100\tmoby\tMoby_Bolt_Update\thook\t", block)
            self.assertNotIn("Test_Add", block)
            text = gen_binding.with_block("a\trow\n", block)
            self.assertEqual(gen_binding.with_block(text, block), text)  # in place, once

    def test_ids_never_change_and_old_names_still_bind(self):
        with tempfile.TemporaryDirectory() as tmp:
            f = Fixture(tmp, table(), CLASSES)
            f.write_lock(f.load())
            # The decompilation renames the crate's update and the catalogue its hook name; a class
            # comes before it with the same name. The id stays the crate function's (its anchor),
            # and the old names stay aliases.
            renamed = json.loads(json.dumps(CLASSES))
            renamed[0]["updates"][0].update(name="CrateBoxUpdate", hook="Moby_CrateBox_Update")
            renamed.insert(0, {"id": 1, "name": "crate", "levels": [5],
                               "updates": [update("func_L05_00100000", "CrateUpdate", "Moby_NewCrate_Update")], "pvars": []})
            g = Fixture(tmp, table(), renamed)
            b = g.load()
            crate = next(h for h in b["hooks"] if h["name"] == "Moby_CrateBox_Update")
            self.assertEqual(crate["id"], "t.moby.crate.update")
            self.assertEqual(b["aliases"]["Moby_Crate_Update"], "t.moby.crate.update")
            self.assertEqual(b["aliases"]["Moby_CrateBox_Update"], "t.moby.crate.update")
            newer = next(h for h in b["hooks"] if h["name"] == "Moby_NewCrate_Update")
            self.assertEqual(newer["id"], "t.moby.crate_1.update")  # the taken id is never reused
            # The C binding resolves the old name to the id.
            self.assertIn('{"Moby_Crate_Update", "t.moby.crate.update"},', gen_binding.c_binding(b))
            # A hook point of the lock that is gone: refused (ids are never removed).
            gone = [c for c in renamed if c["id"] != 500]
            h = Fixture(tmp, table(), gone)
            with self.assertRaises(gen_binding.BindingError):
                h.load()
            # A table that gives a locked hook point another id: refused.
            moved = table([{"id": "t.player.plus", "name": "Test_Add", "signature": "i32(i32)", "params": ["x"],
                            "symbols": {"one": "func_00100000"}}])
            m = Fixture(tmp, moved, CLASSES)
            with self.assertRaises(gen_binding.BindingError):
                m.load()

    def test_table_rules(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "t.json"
            base = {"format": 1, "game": "t", "discs": ["one"], "hooks": [], "fields": []}
            hook = {"id": "t.a", "name": "A", "signature": "void()", "symbols": {"one": "func_00100000"}}
            for broken in [
                {"hooks": [hook, dict(hook, id="t.b")]},                       # one name twice
                {"hooks": [hook, dict(hook, name="B", symbols={"one": "func_00100100"})]},  # one id twice
                {"hooks": [dict(hook, id=None)]},                               # no id
                {"hooks": [dict(hook, id="T.Upper")]},                          # not an id
                {"hooks": [dict(hook, signature="void(i32)", params=["x", "y"])]},
                {"hooks": [hook, dict(hook, id="t.b", name="B", symbols={"one": "func_00100100"}, aliases=["A"])]},
                {"fields": [{"name": "a", "type": "f64", "address": {"one": "0x10"}}]},
                {"fields": [{"name": "a", "type": "i32", "address": {"one": "0x10"}},
                            {"name": "a.b", "type": "i32", "address": {"one": "0x14"}}]},
            ]:
                t = dict(base, **broken)
                t["hooks"] = [{k: v for k, v in h.items() if v is not None} for h in t["hooks"]]
                path.write_text(json.dumps(t))
                with self.assertRaises(gen_binding.BindingError, msg=broken):
                    gen_binding.load("", None, path)


if __name__ == "__main__":
    unittest.main()

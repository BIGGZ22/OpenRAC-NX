# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for port/tools/regress: comparing traces, merging reach files, routes as presses.

    python3 -m unittest discover -s port/tools/regress
"""

from __future__ import annotations

import json
import math
import struct
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

import compare
import reach
import routes
import runner


def tick(f, c, **kw):
    t = {"f": f, "c": c, "lv": 1, "m": 0, "cam": [1.0, 2.0, 3.0], "fw": [1.0, 0.0, 0.0],
         "hp": [10.0, 20.0, 30.0], "hr": [0.0, 0.0, 1.0], "hs": 0, "mb": 100, "dr": 31}
    t.update(kw)
    return t


def trace(control_at: int, count: int, before: int = 3, change=None) -> list[dict]:
    """before ticks without control, then count ticks from control; change(c, tick) edits one."""
    out = [tick(control_at - before + i, None, m=2) for i in range(before)]
    for c in range(count):
        t = tick(control_at + c, c)
        if change:
            change(c, t)
        out.append(t)
    return out


def write_run(root: Path, game: str, levels: dict[int, list[dict]]) -> Path:
    root.mkdir(parents=True)
    meta = {"game": game, "levels": {}}
    for level, ticks in levels.items():
        d = root / f"{level:02d}"
        d.mkdir()
        (d / "trace.jsonl").write_text("".join(json.dumps(t) + "\n" for t in ticks))
        meta["levels"][str(level)] = {"name": f"L{level}", "status": "ok"}
    (root / "run.json").write_text(json.dumps(meta))
    return root


class Compare(unittest.TestCase):
    tol = compare.DEFAULT_TOLERANCE

    def test_aligned_on_control_not_on_window_frames(self):
        # The same route reached control at other window frames: no divergence.
        n, first, note = compare.compare_ticks(trace(500, 50), trace(560, 50, before=10), list(compare.FIELDS), self.tol)
        self.assertEqual(n, 50)
        self.assertIsNone(first)
        self.assertEqual(note, "")

    def test_first_diverging_tick_and_field(self):
        def bend(c, t):
            if c >= 20:
                t["fw"] = [0.0, 0.0, -1.0]
                t["hp"] = [10.0, 20.0, 31.0]
        n, first, note = compare.compare_ticks(trace(500, 40), trace(500, 40, change=bend), list(compare.FIELDS), self.tol)
        self.assertEqual(first.c, 20)
        self.assertEqual(first.field, "fw")
        self.assertEqual(first.others, ["hp"])
        self.assertEqual(first.frames, (520, 520))
        self.assertIn("fw 1.000", note)
        self.assertIn("hp 1.000", note)

    def test_tolerance(self):
        def nudge(c, t):
            t["cam"] = [1.0 + 0.04, 2.0, 3.0]
        _, first, _ = compare.compare_ticks(trace(0, 10), trace(0, 10, change=nudge), ["cam"], self.tol)
        self.assertIsNone(first)
        _, first, _ = compare.compare_ticks(trace(0, 10), trace(0, 10, change=nudge), ["cam"], dict(self.tol, position=0.01))
        self.assertEqual(first.c, 0)

    def test_angles_wrap(self):
        def wrap(c, t):
            t["hr"] = [0.0, 0.0, 1.0 - 2 * math.pi + 0.001]
        _, first, _ = compare.compare_ticks(trace(0, 5), trace(0, 5, change=wrap), ["hr"], self.tol)
        self.assertIsNone(first)

    def test_exact_fields_and_fields_chosen(self):
        def mode(c, t):
            if c == 3:
                t["m"] = 3
        _, first, _ = compare.compare_ticks(trace(0, 5), trace(0, 5, change=mode), list(compare.FIELDS), self.tol)
        self.assertEqual((first.c, first.field), (3, "m"))
        _, first, _ = compare.compare_ticks(trace(0, 5), trace(0, 5, change=mode), ["cam", "fw"], self.tol)
        self.assertIsNone(first)

    def test_a_missing_value_differs(self):
        def lose(c, t):
            if c == 2:
                t["hp"] = None
        _, first, _ = compare.compare_ticks(trace(0, 5), trace(0, 5, change=lose), ["hp"], self.tol)
        self.assertEqual(first.c, 2)

    def test_version_differences_are_left_out_between_versions_only(self):
        def more(c, t):
            t["mb"] = 101
            if c >= 4:
                t["hr"] = [0.5, 0.0, 1.0]
        data = {"levels": {"3": {"name": "L3", "versions": [
            {"fields": ["mb"], "from": 0, "why": "spawns"}, {"fields": ["hr"], "from": 4, "why": "tumble"}]}}}
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            pal = write_run(tmp / "pal", "rac1-pal", {3: trace(0, 8)})
            us = write_run(tmp / "us", "rac1-ntsc", {3: trace(0, 8, change=more)})
            us2 = write_run(tmp / "us2", "rac1-ntsc", {3: trace(0, 8)})
            [r] = compare.compare_runs(pal, us, routes=data)
            self.assertEqual(r.state, "same")
            self.assertIn("version differences not compared: mb from c=0, hr from c=4", r.note)
            [r] = compare.compare_runs(pal, us, routes=data, strict=True)
            self.assertEqual((r.state, r.first.c, r.first.field), ("diverges", 0, "mb"))
            [r] = compare.compare_runs(us2, us, routes=data)  # two runs of one version: everything
            self.assertEqual((r.first.c, r.first.field), (0, "mb"))

    def test_the_routes_version_differences_name_known_fields(self):
        for level, spec in routes.load()["levels"].items():
            for d in spec.get("versions", []):
                with self.subTest(level=level):
                    self.assertTrue(set(d["fields"]) <= set(compare.FIELDS))
                    self.assertTrue(d.get("why"))

    def test_runs_per_level(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)

            def drift(c, t):
                if c >= 7:
                    t["cam"] = [5.0, 2.0, 3.0]
            a = write_run(tmp / "a", "rac1-pal", {0: trace(100, 20), 1: trace(100, 20), 2: trace(100, 20)})
            b = write_run(tmp / "b", "rac1-ntsc", {0: trace(130, 20), 1: trace(130, 20, change=drift),
                                                   2: [tick(1, None, m=6)], 3: trace(1, 5)})
            res = {r.level: r for r in compare.compare_runs(a, b)}
            self.assertEqual(res[0].state, "same")
            self.assertEqual(res[1].state, "diverges")
            self.assertEqual(res[1].first.c, 7)
            self.assertIn("diverges at c=7 (frames 107/137): cam", res[1].line())
            self.assertEqual(res[2].state, "missing")
            self.assertIn("no control in B", res[2].line())
            self.assertEqual(res[3].state, "missing")
            self.assertEqual([r.level for r in compare.compare_runs(a, b, levels=[1])], [1])

    def test_a_shorter_side_is_noted(self):
        n, first, note = compare.compare_ticks(trace(0, 30), trace(0, 12), list(compare.FIELDS), self.tol)
        self.assertEqual(n, 12)
        self.assertIsNone(first)
        self.assertIn("A ends at c=29, B at c=11", note)

    def test_a_trace_cut_by_a_crash_reads_to_the_last_whole_line(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / "trace.jsonl"
            p.write_text(json.dumps(tick(1, 0)) + "\n" + json.dumps(tick(2, 1)) + "\n" + '{"f": 3, "c"')
            self.assertEqual([t["f"] for t in runner.read_trace(p)], [1, 2])


class Reach(unittest.TestCase):
    def test_merge_ranks_by_levels_then_calls(self):
        files = [
            ("rac1-pal", 0, {"game": "rac1-pal", "sites": [
                {"name": "func_A", "kind": "asm", "calls": 5},
                {"name": "func_B", "kind": "zero-args", "detail": "func_X argument 2", "calls": 1000},
                {"name": "func_C", "kind": "no-c", "calls": 0}]}),
            ("rac1-pal", 1, {"game": "rac1-pal", "sites": [
                {"name": "func_A", "kind": "asm", "calls": 7},
                {"name": "func_B", "kind": "zero-args", "detail": "func_Y argument 1", "calls": 1},
                {"name": "func_D", "kind": "untranslated", "calls": 9000}]}),
            ("rac1-ntsc", 0, {"game": "rac1-ntsc", "sites": [{"name": "FUN_a", "kind": "asm", "calls": 1}]}),
        ]
        merged = reach.merge(files)
        self.assertEqual(sorted(merged), ["rac1-ntsc", "rac1-pal"])
        pal = merged["rac1-pal"]
        self.assertEqual([r.name for r in pal], ["func_B", "func_A", "func_D"])  # C was never called
        self.assertEqual(pal[0].levels, {0, 1})
        self.assertEqual(pal[0].calls, 1001)
        self.assertEqual(pal[0].details, {"func_X argument 2", "func_Y argument 1"})
        self.assertEqual(pal[1].calls, 12)
        self.assertEqual(merged["rac1-ntsc"][0].as_dict()["levels"], [0])

    def test_one_function_of_two_kinds_is_two_rows(self):
        files = [("g", 0, {"sites": [{"name": "f", "kind": "untranslated", "calls": 1},
                                     {"name": "f", "kind": "zero-args", "detail": "h argument 1", "calls": 2}]})]
        rows = reach.merge(files)["g"]
        self.assertEqual(sorted(r.kind for r in rows), ["untranslated", "zero-args"])

    def test_load_run_reads_each_level(self):
        with tempfile.TemporaryDirectory() as tmp:
            run = write_run(Path(tmp) / "r", "rac1-ntsc", {3: [], 5: []})
            (run / "03" / "reach.json").write_text(json.dumps({"game": "rac1-ntsc", "sites": [
                {"name": "f", "kind": "asm", "calls": 2}]}))
            (run / "05" / "reach.json").write_text('{"game": "rac1-ntsc", "sit')  # cut short: left out
            self.assertEqual([(g, lv) for g, lv, _ in reach.load_run(run)], [("rac1-ntsc", 3)])


class Routes(unittest.TestCase):
    def test_every_level_of_both_versions_has_a_route(self):
        for game in routes.GAMES:
            for level in routes.LEVELS:
                r = routes.route(level, game)
                self.assertTrue(r.steps, (game, level))
                self.assertGreater(r.length, max(s["at"] + s.get("hold", 5) for s in r.steps) - 1)

    def test_press_string(self):
        data = {"defaults": {"boot": [{"at": 100, "buttons": "cross", "hold": 5}],
                             "skip": {"every": 30, "buttons": "start", "hold": 5}, "sequence": "s", "length": 50},
                "sequences": {"s": [{"at": 10, "hold": 20, "left": [128, 0]},
                                    {"at": 40, "buttons": "r1+square", "hold": 3}]},
                "levels": {"2": {"name": "Two", "rac1-ntsc": {"length": 70}}}}
        r = routes.route(2, "rac1-pal", data)
        self.assertEqual(r.press(), "100:4000:5,k30:8:5,c10:0:20:128:0:128:128,c40:8800:3:128:128:128:128")
        self.assertEqual(r.env()["OPENRAC_CONTROL_FRAMES"], "50")
        self.assertEqual(routes.route(2, "rac1-ntsc", data).length, 70)
        with self.assertRaises(ValueError):
            routes.mask("cros")


class RouteSets(unittest.TestCase):
    data = {
        "defaults": {"boot": [{"at": 100, "buttons": "cross", "hold": 5}],
                     "skip": {"every": 30, "buttons": "start", "hold": 5}, "sequence": "s", "length": 50},
        "sequences": {"s": [{"at": 10, "hold": 20, "left": [128, 0]}]},
        "levels": {},
        "addresses": {"rac1-pal": {"hero": "0x13F450", "owned": "0x13D5C8", "acquired": "0x13D5F0", "ammo": "0x13D530"},
                      "rac1-ntsc": {"hero": "0x13F350", "owned": "0x13D4C0", "acquired": "0x13D4E8", "ammo": "0x13D428"}},
        "items": {"few": [2, 3, 10], "ammo": {"10": 40}},
        "sets": {"t": {
            "defaults": {"skip": {"every": 30, "buttons": "start", "hold": 5, "from": "scene", "after_control": True},
                         "length": 900},
            "lists": {"held": [15, 10]},
            "sequences": {"tour": [
                {"at": 5, "give": "few"},
                {"at": 20, "each": "path", "every": 100, "do": [{"at": 0, "teleport": "$"},
                                                                {"at": 30, "buttons": "cross", "hold": 5}]},
                {"at": 400, "teleport": "vendor"},
                {"at": 500, "each": "held", "every": 50, "do": [{"at": 0, "equip": "$"}, {"at": 20, "buttons": "circle"}]}]},
            "routes": {
                "04-tour": {"level": 4, "name": "tour", "sequence": "tour", "points": {"vendor": [1.0, 2.0, 3.0, 0.5]},
                            "lists": {"path": [[10.0, 20.0, 30.0], [11.0, 21.0, 31.0]]}},
                "title-load": {"start": "title", "card": "prepared", "skip": None, "level": 5, "steps": []},
                "title-menu": {"start": "title", "until": "frames", "max_frames": 2000, "skip": None, "steps": []},
                "02-old": {"level": 2, "skip": {"every": 30, "buttons": "start", "hold": 5, "from": "scene"},
                           "rac1-ntsc": {"length": 70}}}}}}

    def routes(self, game="rac1-pal"):
        return {r.key: r for r in routes.set_routes("t", game, self.data)}

    def test_each_block_unrolls_with_the_entry_and_named_points(self):
        r = self.routes()["04-tour"]
        teleports = [(s["at"], s["teleport"]) for s in r.steps if "teleport" in s]
        self.assertEqual(teleports, [(20, [10.0, 20.0, 30.0]), (120, [11.0, 21.0, 31.0]), (400, [1.0, 2.0, 3.0, 0.5])])
        self.assertEqual([(s["at"], s["equip"]) for s in r.steps if "equip" in s], [(500, 15), (550, 10)])
        self.assertEqual(r.press(), "100:4000:5,S30:8:5,c50:4000:5:128:128:128:128,c150:4000:5:128:128:128:128,"
                                    "c520:2000:5:128:128:128:128,c570:2000:5:128:128:128:128")
        self.assertEqual(r.length, 900)

    def test_writes_are_each_versions_words(self):
        pal, us = self.routes()["04-tour"], self.routes("rac1-ntsc")["04-tour"]
        w = {(at, a): (v, n) for at, a, v, n in pal.writes}
        self.assertEqual(w[(20, 0x13F4D0)], (0x41200000, 10))      # x = 10.0, held 10 frames
        self.assertEqual(w[(400, 0x13F4E8)], (0x3F000000, 50))     # the yaw, held 50
        self.assertEqual(w[(500, 0x13F450 + 0x20B8)], (15, 1))     # the pending item
        # Items 2 and 3 share the word at 0x13D5C8, 10 the one at 0x13D5D0; ammo by item.
        self.assertEqual(w[(5, 0x13D5C8)], (0x01010000, 1))
        self.assertEqual(w[(5, 0x13D5D0)], (0x00010000, 1))
        self.assertEqual(w[(5, 0x13D530 + 40)], (40, 1))
        self.assertIn((20, 0x13F3D0, 0x41200000, 10), us.writes)
        self.assertTrue(pal.env()["OPENRAC_WRITE"].startswith("c5:"))
        self.assertIn("c20:13f4d0:41200000:10", pal.env()["OPENRAC_WRITE"])

    def test_title_routes_and_skips(self):
        rs = self.routes()
        load, menu, old = rs["title-load"], rs["title-menu"], rs["02-old"]
        self.assertNotIn("OPENRAC_DIRECT", load.env())
        self.assertEqual(load.env()["OPENRAC_CONTROL_FRAMES"], "900")
        self.assertEqual((load.card, load.level), ("prepared", 5))
        self.assertTrue(menu.until_frames)
        self.assertNotIn("OPENRAC_CONTROL_FRAMES", menu.env())
        self.assertIn("s30:8:5", old.press())                      # counted from the scene, before control only
        self.assertEqual(self.routes("rac1-ntsc")["02-old"].length, 70)

    def test_the_standard_set_is_the_level_routes(self):
        self.assertEqual([r.key for r in routes.set_routes("standard", "rac1-pal")], [f"{i:02d}" for i in range(19)])
        with self.assertRaises(ValueError):
            routes.set_routes("nope", "rac1-pal", self.data)

    def test_the_wide_set_resolves_for_both_versions(self):
        for game in routes.GAMES:
            rs = routes.set_routes("wide", game)
            self.assertGreater(len(rs), 40)
            for r in rs:
                with self.subTest(game=game, route=r.key):
                    r.env()
                    if not r.until_frames:
                        self.assertGreaterEqual(r.length, max((s["at"] + s.get("hold", 5) for s in r.steps), default=0))

    def test_a_set_run_is_compared_and_merged_by_route(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            runs = []
            for game, shift in (("rac1-pal", 0), ("rac1-ntsc", 40)):
                root = tmp / game
                root.mkdir()
                meta = {"game": game, "set": "t", "levels": {}}
                for key, level in (("04-tour", 4), ("title-menu", -1)):
                    (root / key).mkdir()
                    ticks = trace(100 + shift, 10) if key == "04-tour" else [tick(f, None, lv=-1) for f in range(5)]
                    (root / key / "trace.jsonl").write_text("".join(json.dumps(t) + "\n" for t in ticks))
                    (root / key / "reach.json").write_text(json.dumps({"game": game, "sites": [
                        {"name": "f", "kind": "asm", "calls": 1}]}))
                    meta["levels"][key] = {"name": key, "level": level, "key": key, "dir": key, "status": "ok"}
                (root / "run.json").write_text(json.dumps(meta))
                runs.append(root)
            res = {r.level: r for r in compare.compare_runs(*runs, routes=self.data)}
            self.assertEqual(res["04-tour"].state, "same")
            self.assertEqual(res["title-menu"].state, "missing")  # no control: PAL and US are not paired
            [r] = [r for r in compare.compare_runs(runs[0], runs[0], routes=self.data) if r.level == "title-menu"]
            self.assertEqual(r.state, "same")                      # one version: by window frame
            self.assertIn("04-tour", res["04-tour"].line())
            rows = reach.merge(reach.load_run(runs[0]))["rac1-pal"]
            self.assertEqual(rows[0].levels, {"04-tour", "title-menu"})


class Card(unittest.TestCase):
    def test_sections_round_trip_and_the_prepared_save(self):
        import card
        chunks = [[0, bytearray(struct.pack("<i", 1)), b""], [1, bytearray(4), b""], [5, bytearray(128), b""],
                  [9, bytearray(148), b""], [10, bytearray(37), b"\0\0\0"], [11, bytearray(37), b"\0\0\0"],
                  [12, bytearray(b"\xff" * 12), b""], [13, bytearray(32), b""], [14, bytearray(20), b""],
                  [19, bytearray(4), b""], [20, bytearray(80), b""]]
        level = [[3003, bytearray(4), b""]]
        save = card.to_bytes(chunks, [level] * 20, len(card.encode_section(level)))
        glob, levels, ls = card.parse(save)
        self.assertEqual(card.to_bytes(glob, levels, ls), save)
        out, _, _ = card.parse(card.prepared(save, 13))
        self.assertEqual(struct.unpack("<i", card.chunk(out, card.PLANET))[0], 13)
        self.assertEqual(card.chunk(out, card.OWNED)[15], 1)
        self.assertEqual(struct.unpack_from("<i", card.chunk(out, card.AMMO), 4 * 15)[0], 200)
        self.assertEqual(sum(card.chunk(out, card.PLANETS)[1:17]), 16)
        self.assertEqual(card.chunk(out, card.FLAGS)[card.SAVE_NOTICE_SEEN], 1, "the save notice seen")
        bad = bytearray(save)
        bad[20] ^= 1
        with self.assertRaises(ValueError):
            card.parse(bytes(bad))


class Points(unittest.TestCase):
    def test_path_runs_nearest_first_from_the_start(self):
        import points

        def moby(cls, x, y, z, yaw=0.0):
            return {"class": cls, "matrix": [0] * 12 + [x, y, z, 1], "rotation": [0, 0, yaw]}
        mobys = [moby(0, 0, 0, 0), moby(805, 50, 0, 0), moby(805, 10, 0, 0), moby(1134, 30, 0, 0),
                 moby(1134, 31, 0, 0), moby(11, 0, 20, 0, math.pi / 2)]
        orig = points.placements
        points.placements = lambda level, root=None: mobys
        try:
            p = points.level_points(1)
        finally:
            points.placements = orig
        self.assertEqual(p["start"], [0, 0, 0])
        self.assertEqual([q[0] for q in p["path"]], [10, 30, 50, 0])  # 31 merged into 30; the vendor last
        self.assertAlmostEqual(p["vendors"][0][1], 23.0)                    # three units in front of it
        self.assertAlmostEqual(p["vendors"][0][3], -math.pi / 2, places=3)  # facing it


class Outcome(unittest.TestCase):
    def test_run_selects_the_platform_executable(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for platform, suffix in (("nt", ".exe"), ("posix", "")):
                with self.subTest(platform=platform):
                    exe = root / f"openrac-rac1-ntsc{suffix}"
                    exe.touch()
                    result = {"status": "ok", "seconds": 0, "control_frame": 1}
                    with patch.object(runner, "os", SimpleNamespace(name=platform)), \
                         patch.object(runner, "run_level", return_value=result) as play:
                        runner.run("rac1-ntsc", [0], root, out=root / platform,
                                   data=root, level_dir=root, jobs=1, progress=lambda _: None)
                    self.assertEqual(play.call_count, 1)
                    self.assertEqual(play.call_args.args[2], exe)
                    exe.unlink()

    def test_classify(self):
        ticks = trace(10, 21)
        self.assertEqual(runner.classify(0, False, ticks, 20), "ok")
        self.assertEqual(runner.classify(0, False, ticks, 40), "exit")
        self.assertEqual(runner.classify(0, False, [tick(1, None)], 20), "no-control")
        self.assertEqual(runner.classify(3, False, ticks, 20), "crash")
        self.assertEqual(runner.classify(-9, True, ticks, 20), "hang")

    def test_notes_name_the_crash(self):
        lines = ["[info] x", "[verbose]   from game_FUN_0012d9d8", "Reason:   access (SIGBUS) at game address 0x43348F7B, which is not mapped",
                 "Backtrace (the newest call first):", "4   prog  0x1 game_FUN_00210850 + 80", "5   prog  0x2 game_FUN_L00_0024f7c8 + 116"]
        self.assertEqual(runner.notes(lines), ["access (SIGBUS) at game address 0x43348F7B, which is not mapped",
                                               "in game_FUN_00210850 < game_FUN_L00_0024f7c8"])
        self.assertEqual(runner.notes(["a", "[error] b", "c"]), ["[error] b"])


if __name__ == "__main__":
    unittest.main()

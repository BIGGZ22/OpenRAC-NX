# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for port/tools/regress: comparing traces, merging reach files, routes as presses.

    python3 -m unittest discover -s port/tools/regress
"""

from __future__ import annotations

import json
import math
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import compare  # noqa: E402
import reach  # noqa: E402
import routes  # noqa: E402
import runner  # noqa: E402


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


class Outcome(unittest.TestCase):
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

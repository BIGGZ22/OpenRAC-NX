# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Tests for render_bench.py: reading the benchmark's output, combining runs, the tables and the
comparison, on made-up output (no GPU, no benchmark run).

  python3 -m unittest discover -s port/tools/perf
"""
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import render_bench as rb  # noqa: E402


def counters(**values):
    """A summary's counter object: every counter the tables read at 0, `values` over them."""
    names = rb.FRAME_COUNTERS + ["vk_pipelines_created", "vk_pipelines_created_after_warmup",
                                 "vk_shader_compiles", "shader_compile_s"]
    out = {name: 0 for name in names}
    out.update(values)
    return out


def summary(frames=10, picture=5.0, heap=100, **measured):
    """A made-up summary line's object, as render_bench.cpp prints it."""
    return {
        "frames": frames,
        "picture_ms": {"median": picture, "p95": picture * 2, "max": picture * 3},
        "first_picture_ms": 50.0,
        "load_ms": 20.0,
        "bench_heap_allocs": heap,
        "bench_heap_allocs_per_frame": heap / frames,
        "load": counters(vk_pipelines_created=3, vk_shader_compiles=12, shader_compile_s=0.5),
        "warmup": counters(vk_pipelines_created=4, vk_texture_creations=600),
        "counters": counters(**measured),
    }


def output(frames, summary_object):
    """The benchmark's stdout for `frames` (each a frame object) and a summary."""
    lines = ["OpenGL Vulkan 1.3 on a test device"]
    lines += [json.dumps(frame) for frame in frames]
    lines.append(json.dumps({"summary": summary_object}))
    return "\n".join(lines) + "\n"


class ParseTest(unittest.TestCase):
    def test_frames_and_the_summary_are_read_and_other_lines_left_out(self):
        frame = {"frame": 0, "sections": [], "gauges": {}}
        frames, read = rb.parse_output(output([frame, frame], summary()))
        self.assertEqual(len(frames), 2)
        self.assertEqual(read["frames"], 10)

    def test_output_without_a_summary_has_none(self):
        frames, read = rb.parse_output('{"frame": 0}\n')
        self.assertEqual(len(frames), 1)
        self.assertIsNone(read)


class SectionsTest(unittest.TestCase):
    def test_sections_take_their_median_by_path_and_gpu_only_where_measured(self):
        frames = [
            {"sections": [{"name": "terrain", "depth": 0, "cpu_ms": t, "gpu_ms": -1},
                          {"name": "inner", "depth": 1, "cpu_ms": 9.0, "gpu_ms": 9.0},
                          {"name": "mobys", "depth": 0, "cpu_ms": 1.0, "gpu_ms": g}]}
            for t, g in ((1.0, 0.5), (3.0, 0.7), (2.0, -1))
        ]
        medians = rb.section_medians(frames)
        self.assertEqual(medians["terrain"], {"cpu_ms": 2.0, "gpu_ms": None})
        self.assertAlmostEqual(medians["mobys"]["gpu_ms"], 0.6)
        self.assertEqual(medians["terrain / inner"], {"cpu_ms": 9.0, "gpu_ms": 9.0})


class CombineTest(unittest.TestCase):
    def test_runs_with_the_same_counters_are_deterministic_and_times_take_the_median(self):
        runs = [([], summary(picture=p, vk_cmds_recorded=500)) for p in (4.0, 6.0, 5.0)]
        result = rb.combine("base", runs)
        self.assertTrue(result.deterministic)
        self.assertEqual(result.timings["picture_ms"], {"median": 5.0, "low": 4.0, "high": 6.0})

    def test_a_counter_that_differs_between_runs_is_not_deterministic(self):
        runs = [([], summary(vk_cmds_recorded=500)), ([], summary(vk_cmds_recorded=501))]
        self.assertFalse(rb.combine("base", runs).deterministic)

    def test_the_compile_seconds_may_differ(self):
        first = summary()
        second = summary()
        second["load"]["shader_compile_s"] = 0.9
        self.assertTrue(rb.combine("base", [([], first), ([], second)]).deterministic)

    def test_no_summary_is_a_skip(self):
        self.assertTrue(rb.combine("base", [([], None)]).skipped)


class TablesTest(unittest.TestCase):
    def test_counters_are_per_frame_and_the_load_and_warmup_are_totals(self):
        result = rb.combine("base", [([], summary(frames=10, vk_cmds_recorded=500))])
        row = rb.counters_row(result)
        self.assertEqual(row["vk_cmds_recorded"], 50.0)
        self.assertEqual(row["pipelines (load+warmup)"], 3 + 4)
        self.assertEqual(row["textures created (warmup)"], 600)
        self.assertEqual(row["bench_heap_allocs/frame"], 10.0)

    def test_the_counters_table_has_a_column_per_setting_and_says_whether_runs_agreed(self):
        a = rb.combine("a", [([], summary())])
        b = rb.combine("b", [([], summary())])
        table = rb.counters_table([a, b])
        self.assertIn("| Counter | a | b |", table)
        self.assertIn("| the same in every run | yes | yes |", table)

    def test_numbers_have_thousands_separators(self):
        self.assertEqual(rb.format_number(1234567), "1,234,567")
        self.assertEqual(rb.format_number(2.25), "2.2")
        self.assertEqual(rb.format_number(None), "n/a")


class CompareTest(unittest.TestCase):
    def test_percent_handles_equal_zero_and_new_counters(self):
        self.assertEqual(rb.percent(0, 0), 0.0)
        self.assertIsNone(rb.percent(0, 5))
        self.assertEqual(rb.percent(200, 150), -25.0)

    def test_a_comparison_row_gives_the_change(self):
        before = rb.combine("base", [([], summary(frames=10, vk_cmds_recorded=1000))])
        after = rb.combine("base", [([], summary(frames=10, vk_cmds_recorded=400))])
        table = rb.comparison_table(before, after)
        self.assertIn("| vk_cmds_recorded | 100 | 40 | -60.0% |", table)

    def test_saved_files_compare_by_setting_name(self):
        before = rb.combine("base", [([], summary(vk_cmds_recorded=100))])
        after = rb.combine("base", [([], summary(vk_cmds_recorded=50))])
        with tempfile.TemporaryDirectory() as folder:
            rb.save([before], Path(folder) / "before.json")
            rb.save([after], Path(folder) / "after.json")
            text = rb.compare_files(Path(folder) / "before.json", Path(folder) / "after.json")
        self.assertIn("### base", text)
        self.assertIn("-50.0%", text)


class MatrixTest(unittest.TestCase):
    def test_the_full_matrix_is_every_combination_with_unique_names(self):
        settings = rb.full_matrix()
        self.assertEqual(len(settings), 2 * 2 * 2 * 2 * 3)
        self.assertEqual(len({s.name for s in settings}), len(settings))

    def test_the_reduced_matrix_changes_one_thing_at_a_time_and_halves_the_size(self):
        settings = rb.reduced_matrix("1920x1080")
        self.assertEqual(settings[0].options, ())
        self.assertTrue(all(len(s.options) == 2 for s in settings[1:]))
        self.assertIn(("--size", "960x540"), [s.options for s in settings])

    def test_a_settings_arguments_put_its_options_after_the_runs_own(self):
        arguments = rb.Setting("x", ("--size", "960x540")).arguments(10, 2, "1920x1080")
        self.assertEqual(arguments[:6], ["--frames", "10", "--warmup", "2", "--size", "1920x1080"])
        self.assertEqual(arguments[-2:], ["--size", "960x540"])


if __name__ == "__main__":
    unittest.main()

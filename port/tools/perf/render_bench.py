#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Runs openrac_render_bench (port/tests/render_bench.cpp) over a matrix of settings and prints the
render counters and the timings as tables (docs/port/RENDER_PERF.md).

  render_bench.py --synthetic [--reduced | --full]   the synthetic scene, the reduced matrix by default
  render_bench.py --level DIR                         an extracted level (the maintainers' runs)
  render_bench.py --synthetic --compare-opt NAME      the matrix with registry entry NAME on and off
  render_bench.py --compare BEFORE.json AFTER.json    two saved results, with a percentage per counter

Each setting runs --runs times (5 by default). The counters (counts and bytes) must come out the
same in every run, and the table says so; the milliseconds are the median of the runs with their
lowest and highest, and only inform: on a shared machine they are noise beside the counters.
Results are saved under port/build/render-bench/ (which git ignores), named by --label.

The benchmark draws offscreen: no window is shown and no picture is written.
"""
import argparse
import itertools
import json
import os
import statistics
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

PORT = Path(__file__).resolve().parents[2]
DEFAULT_BENCH = PORT / "build" / "release" / "openrac_render_bench"
DEFAULT_OUT = PORT / "build" / "render-bench"

# The exit status of a benchmark that found no Vulkan (ctest's skip).
SKIPPED = 77

# The counters a table shows, each per measured frame unless it is a run's total (the pipelines,
# the compiles, the textures of the load and warm-up), in the order of RENDER_PERF.md's glossary.
FRAME_COUNTERS = [
    "vk_cmds_recorded",
    "vk_cmds_draw",
    "vk_cmds_dynamic_state",
    "vk_cmds_bind_descriptors",
    "vk_render_passes",
    "vk_attachment_store_bytes",
    "vk_barriers",
    "vk_barriers_all_commands",
    "vk_submits",
    "vk_fullscreen_copies",
    "vk_ring_bytes",
    "vk_descriptor_sets_allocated",
    "vk_descriptor_writes",
    "vk_texture_creations",
    "vk_texture_upload_bytes",
    "vk_gpu_payload_bytes",
]


@dataclass(frozen=True)
class Setting:
    """One cell of the matrix: a name and the benchmark's options for it."""

    name: str
    options: tuple = ()

    def arguments(self, frames, warmup, size):
        """The benchmark's command-line arguments for this setting."""
        base = ["--frames", str(frames), "--warmup", str(warmup), "--size", size]
        return base + list(self.options)


def full_matrix():
    """Every combination of Enhanced, GPU-driven, materials, cache and camera."""
    settings = []
    for enhanced, gpu, materials, cache, camera in itertools.product(
        ("off", "on"), ("on", "off"), ("on", "off"), ("cold", "warm"), ("walk", "static", "turn")
    ):
        name = f"enh-{enhanced} gpu-{gpu} mat-{materials} {cache} {camera}"
        options = (
            "--enhanced", enhanced, "--gpu-driven", gpu, "--materials", materials,
            "--cache", cache, "--camera", camera,
        )
        settings.append(Setting(name, options))
    return settings


def reduced_matrix(size):
    """The default: the base setting and one change of it at a time, and the base at half size."""
    width, height = (int(n) for n in size.split("x"))
    half = f"{max(width // 2, 16)}x{max(height // 2, 16)}"
    return [
        Setting("base (walk, cold)"),
        Setting("enhanced", ("--enhanced", "on")),
        Setting("warm cache", ("--cache", "warm")),
        Setting("static camera", ("--camera", "static")),
        Setting("between x3", ("--between", "3")),
        Setting("gpu-driven off", ("--gpu-driven", "off")),
        Setting("materials off", ("--materials", "off")),
        Setting(f"half size {half}", ("--size", half)),
    ]


def parse_output(text):
    """The frames and the summary of one benchmark run's output; lines that are not JSON (the
    context's own line) are left out. Returns (frames, summary); summary is None when missing."""
    frames = []
    summary = None
    for line in text.splitlines():
        line = line.strip()
        if not line.startswith("{"):
            continue
        record = json.loads(line)
        if "summary" in record:
            summary = record["summary"]
        else:
            frames.append(record)
    return frames, summary


def section_paths(sections):
    """Each section of a frame with its path: its own name after its parents', joined by " / "."""
    path = []
    for section in sections:
        depth = section.get("depth", 0)
        path = path[:depth] + [section["name"]]
        yield " / ".join(path), section


def section_medians(frames):
    """Each section's median CPU and GPU milliseconds over `frames`, by its path (GPU: None when
    no frame had a GPU time for it)."""
    cpu = {}
    gpu = {}
    for frame in frames:
        for name, section in section_paths(frame.get("sections", [])):
            cpu.setdefault(name, []).append(section["cpu_ms"])
            if section.get("gpu_ms") is not None and section["gpu_ms"] >= 0:
                gpu.setdefault(name, []).append(section["gpu_ms"])
    return {
        name: {"cpu_ms": statistics.median(values),
               "gpu_ms": statistics.median(gpu[name]) if name in gpu else None}
        for name, values in cpu.items()
    }


def spread(values):
    """The median of `values` with their lowest and highest."""
    return {"median": statistics.median(values), "low": min(values), "high": max(values)}


@dataclass
class Result:
    """What the runs of one setting measured."""

    name: str
    summary: dict = None
    deterministic: bool = True
    timings: dict = field(default_factory=dict)
    sections: dict = field(default_factory=dict)
    skipped: bool = False


def deterministic_part(summary):
    """The counts and bytes of a summary, which must be the same in every run: everything of its
    load, warm-up and measured counters but the compile seconds."""
    part = {}
    for phase in ("load", "warmup", "counters"):
        part[phase] = {k: v for k, v in summary[phase].items() if k != "shader_compile_s"}
    part["bench_heap_allocs"] = summary["bench_heap_allocs"]
    part["draw_calls"] = summary.get("draw_calls")
    part["triangles"] = summary.get("triangles")
    part["frames"] = summary["frames"]
    return part


def combine(name, runs):
    """One setting's Result from its runs, each a (frames, summary) pair."""
    summaries = [summary for _, summary in runs if summary is not None]
    if not summaries:
        return Result(name, skipped=True)
    first = summaries[0]
    deterministic = all(deterministic_part(s) == deterministic_part(first) for s in summaries)
    timings = {
        "picture_ms": spread([s["picture_ms"]["median"] for s in summaries]),
        "picture_p95_ms": spread([s["picture_ms"]["p95"] for s in summaries]),
        "first_picture_ms": spread([s["first_picture_ms"] for s in summaries]),
        "load_ms": spread([s["load_ms"] for s in summaries]),
        "shader_compile_s": spread([s["load"]["shader_compile_s"] for s in summaries]),
    }
    return Result(name, first, deterministic, timings, section_medians(runs[0][0]))


def per_frame(summary, counter):
    """`counter` of the measured frames, per frame."""
    frames = max(summary["frames"], 1)
    return summary["counters"].get(counter, 0) / frames


def counters_row(result):
    """The deterministic numbers of one result, as the table and the comparison read them."""
    s = result.summary
    frames = max(s["frames"], 1)
    row = {"draw calls": s.get("draw_calls", 0) / frames, "triangles": s.get("triangles", 0) / frames}
    row.update({counter: per_frame(s, counter) for counter in FRAME_COUNTERS})
    row["pipelines (load+warmup)"] = (
        s["load"]["vk_pipelines_created"] + s["warmup"]["vk_pipelines_created"]
    )
    row["pipelines (measured)"] = s["counters"]["vk_pipelines_created"]
    row["pipelines after warmup"] = s["counters"]["vk_pipelines_created_after_warmup"]
    row["textures created (warmup)"] = s["warmup"]["vk_texture_creations"]
    row["texture upload bytes (warmup)"] = s["warmup"]["vk_texture_upload_bytes"]
    row["shader compiles (load)"] = s["load"]["vk_shader_compiles"]
    row["bench_heap_allocs/frame"] = s["bench_heap_allocs_per_frame"]
    return row


def format_number(value):
    """A counter's value for a table: whole numbers with thousands separators, others to 1 place."""
    if value is None:
        return "n/a"
    if float(value).is_integer():
        return f"{int(value):,}"
    return f"{value:,.1f}"


def counters_table(results):
    """The deterministic counters of `results`, one column per setting, in Markdown."""
    shown = [r for r in results if not r.skipped]
    if not shown:
        return "no setting ran (no Vulkan?)"
    rows = list(counters_row(shown[0]).keys())
    lines = ["| Counter | " + " | ".join(r.name for r in shown) + " |",
             "|---|" + "---|" * len(shown)]
    for key in rows:
        cells = [format_number(counters_row(r)[key]) for r in shown]
        lines.append(f"| {key} | " + " | ".join(cells) + " |")
    lines.append("| the same in every run | " + " | ".join(
        "yes" if r.deterministic else "NO" for r in shown) + " |")
    return "\n".join(lines)


def timings_table(results):
    """The milliseconds of `results` (median of the runs, lowest to highest), in Markdown."""
    shown = [r for r in results if not r.skipped]
    keys = ["picture_ms", "picture_p95_ms", "first_picture_ms", "load_ms", "shader_compile_s"]
    lines = ["| Timing (information) | " + " | ".join(r.name for r in shown) + " |",
             "|---|" + "---|" * len(shown)]
    for key in keys:
        cells = []
        for r in shown:
            t = r.timings[key]
            cells.append(f"{t['median']:.2f} ({t['low']:.2f} to {t['high']:.2f})")
        lines.append(f"| {key} | " + " | ".join(cells) + " |")
    return "\n".join(lines)


def sections_table(result):
    """One result's CPU and GPU milliseconds per top-level section (the first run's medians)."""
    lines = ["| Section | CPU ms | GPU ms |", "|---|---|---|"]
    for name, times in result.sections.items():
        gpu = "n/a" if times["gpu_ms"] is None else f"{times['gpu_ms']:.3f}"
        lines.append(f"| {name} | {times['cpu_ms']:.3f} | {gpu} |")
    return "\n".join(lines)


def percent(before, after):
    """How far `after` is from `before`, in percent; None when before is 0 and after is not."""
    if before == after:
        return 0.0
    if before == 0:
        return None
    return (after - before) / before * 100.0


def comparison_table(before, after, label_before="before", label_after="after"):
    """Two results' counters side by side with a percentage column, in Markdown."""
    rows_before = counters_row(before)
    rows_after = counters_row(after)
    lines = [f"| Counter | {label_before} | {label_after} | change |", "|---|---|---|---|"]
    for key, value in rows_before.items():
        change = percent(value, rows_after[key])
        shown = "new" if change is None else f"{change:+.1f}%"
        lines.append(f"| {key} | {format_number(value)} | {format_number(rows_after[key])} | {shown} |")
    t_before = before.timings["picture_ms"]
    t_after = after.timings["picture_ms"]
    lines.append(
        f"| picture_ms (median, information) | {t_before['median']:.2f} | {t_after['median']:.2f} | "
        f"{percent(t_before['median'], t_after['median']) or 0.0:+.1f}% |"
    )
    return "\n".join(lines)


def run_once(bench, arguments):
    """One run of the benchmark: (frames, summary), or None when it skipped; raises when it
    failed."""
    environment = dict(os.environ)
    environment.setdefault("OPENRAC_GL_API", "vulkan")
    process = subprocess.run(
        [str(bench)] + arguments, capture_output=True, text=True, env=environment, check=False
    )
    if process.returncode == SKIPPED:
        return None
    if process.returncode != 0:
        raise RuntimeError(
            f"{bench} {' '.join(arguments)} exited {process.returncode}:\n{process.stderr[-2000:]}"
        )
    return parse_output(process.stdout)


def run_setting(bench, setting, runs, frames, warmup, size, extra=()):
    """`runs` runs of `setting`, combined into a Result."""
    outputs = []
    for _ in range(runs):
        output = run_once(bench, setting.arguments(frames, warmup, size) + list(extra))
        if output is None:
            return Result(setting.name, skipped=True)
        outputs.append(output)
    return combine(setting.name, outputs)


def result_to_json(result):
    """A Result as a JSON-ready dictionary."""
    return {"name": result.name, "summary": result.summary, "deterministic": result.deterministic,
            "timings": result.timings, "sections": result.sections, "skipped": result.skipped}


def result_from_json(data):
    """A Result from result_to_json's dictionary."""
    return Result(data["name"], data["summary"], data["deterministic"], data["timings"],
                  data["sections"], data["skipped"])


def save(results, path):
    """Writes `results` to `path` as JSON."""
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps([result_to_json(r) for r in results], indent=1) + "\n")


def load(path):
    """The Results saved at `path`."""
    return [result_from_json(item) for item in json.loads(Path(path).read_text())]


def compare_files(before_path, after_path):
    """The comparison tables of two saved result files, setting by setting (by name)."""
    before = {r.name: r for r in load(before_path) if not r.skipped}
    after = {r.name: r for r in load(after_path) if not r.skipped}
    parts = []
    for name in before:
        if name in after:
            parts.append(f"### {name}\n\n" + comparison_table(before[name], after[name]))
    return "\n\n".join(parts) if parts else "no setting in both files"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--bench", type=Path, default=DEFAULT_BENCH, help="openrac_render_bench")
    parser.add_argument("--synthetic", action="store_true", help="the synthetic scene (default)")
    parser.add_argument("--level", type=Path, help="an extracted level folder to draw")
    parser.add_argument("--reduced", action="store_true", help="the reduced matrix (default)")
    parser.add_argument("--full", action="store_true", help="every combination")
    parser.add_argument("--runs", type=int, default=5, help="runs of each setting (default 5)")
    parser.add_argument("--frames", type=int, default=60, help="measured frames (default 60)")
    parser.add_argument("--warmup", type=int, default=10, help="warm-up frames (default 10)")
    parser.add_argument("--size", default="1920x1080", help="the picture (default 1920x1080)")
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT, help="where results are saved")
    parser.add_argument("--label", default="latest", help="the saved file's name (default latest)")
    parser.add_argument("--compare", nargs=2, metavar=("BEFORE", "AFTER"), help="two saved files")
    parser.add_argument("--compare-opt", metavar="NAME", help="registry entry NAME on and off")
    parser.add_argument("--sections", action="store_true", help="also each setting's passes")
    args = parser.parse_args(argv)

    if args.compare:
        print(compare_files(*args.compare))
        return 0

    settings = full_matrix() if args.full else reduced_matrix(args.size)
    extra = ["--level", str(args.level)] if args.level else []
    if args.compare_opt:
        parts = []
        for setting in settings:
            on = run_setting(args.bench, setting, args.runs, args.frames, args.warmup, args.size, extra)
            off = run_setting(args.bench, setting, args.runs, args.frames, args.warmup, args.size,
                              extra + ["--opt-off", args.compare_opt])
            if on.skipped or off.skipped:
                parts.append(f"### {setting.name}\n\nskipped (no Vulkan)")
                continue
            parts.append(f"### {setting.name}\n\n"
                         + comparison_table(off, on, f"{args.compare_opt} off", f"{args.compare_opt} on"))
        print("\n\n".join(parts))
        return 0

    results = [run_setting(args.bench, s, args.runs, args.frames, args.warmup, args.size, extra)
               for s in settings]
    save(results, args.out / f"{args.label}.json")
    print(counters_table(results))
    print()
    print(timings_table(results))
    if args.sections:
        for result in results:
            if not result.skipped:
                print(f"\n### {result.name}\n\n" + sections_table(result))
    print(f"\nsaved: {args.out / (args.label + '.json')}")
    return 0 if all(r.deterministic for r in results) else 1


if __name__ == "__main__":
    sys.exit(main())

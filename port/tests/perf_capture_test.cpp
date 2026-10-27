// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The performance capture's summary (common/perf_capture.h): frames of its own, made here, summed
// into the report, with the verdict for a frame held up by the game's code, by the GPU, and one
// that keeps its target; and the capture through stats' frame reader, ended by its length.

#include <chrono>
#include <string>
#include <string_view>
#include <thread>

#include "check.h"
#include "common/perf_capture.h"
#include "common/stats.h"

namespace {

namespace capture = openrac::perf_capture;
namespace stats = openrac::stats;

bool has(const std::string& text, std::string_view part) {
    return text.find(part) != std::string::npos;
}

// A tick's frame of `frame_ms`, the game's code taking `game_ms`, the renderer `render_ms` of it,
// `gpu_ms` on the GPU (-1: not timed) in two passes.
stats::Frame frame(
    std::uint64_t serial, double frame_ms, double game_ms, double render_ms, double gpu_ms
) {
    stats::Frame f;
    f.serial = serial;
    f.tick = serial;
    f.kind = stats::FrameKind::Tick;
    f.start_s = static_cast<double>(serial) * frame_ms / 1000.0;
    f.interval_ms = frame_ms;
    f.game_ms = game_ms;
    f.closed = true;
    f.counters[static_cast<std::size_t>(stats::Counter::DrawCalls)] = 100;
    f.sections.push_back({"scene", 0, 0.5, -1.0, 0, 0});
    f.sections.push_back({"render", 0, render_ms, -1.0, 100, 5000});
    f.sections.push_back(
        {"terrain", 1, render_ms * 0.75, gpu_ms < 0.0 ? -1.0 : gpu_ms * 0.25, 60, 4000}
    );
    f.sections.push_back(
        {"mobys", 1, render_ms * 0.25, gpu_ms < 0.0 ? -1.0 : gpu_ms * 0.75, 40, 1000}
    );
    f.sections.push_back({"swap", 0, 1.0, -1.0, 0, 0});
    f.gauges.emplace_back("mobys", 120.0);
    return f;
}

void percentiles() {
    CHECK(capture::percentile({}, 0.5) == 0.0);
    CHECK(capture::percentile({3.0f, 1.0f, 2.0f}, 0.5) == 2.0);
    CHECK(capture::percentile({3.0f, 1.0f, 2.0f}, 1.0) == 3.0);
    CHECK(capture::percentile({3.0f, 1.0f, 2.0f}, 0.0) == 1.0);
    CHECK(capture::percentile({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 0.95) == 10.0);
}

void cpu_bound() {
    capture::Recorder r(20.0);
    for (std::uint64_t i = 0; i < 100; ++i) {
        r.add(frame(i, 50.0, 30.0, 12.0, 6.0));
    }
    CHECK(r.frames() == 100);
    CHECK(r.seconds() > 4.9 && r.seconds() < 5.1);
    const std::string v = r.verdict();
    CHECK(has(v, "CPU-bound"));
    CHECK(has(v, "game tick 30.00 ms"));
    CHECK(has(v, "in render: terrain 9.00 ms"));
    CHECK(has(v, "Costliest pass on the GPU: mobys 4.50 ms"));

    // The whole frame timed on the GPU, its passes' times near 0 (the Vulkan renderer's): no pass
    // is named.
    capture::Recorder whole(20.0);
    for (std::uint64_t i = 0; i < 100; ++i) {
        stats::Frame f = frame(i, 50.0, 30.0, 12.0, 0.004);
        f.gpu_frame_ms = 8.0;
        whole.add(f);
    }
    const std::string w = whole.verdict();
    CHECK(!has(w, "Costliest pass on the GPU"));
    CHECK(has(w, "passes' GPU times are not measured"));

    capture::Context context;
    context.title = "rac1-pal, Kerwan";
    context.lines.emplace_back("gpu", "a test");
    const std::string report = r.report(context);
    CHECK(has(report, "# OpenRAC diagnostics recording"));
    CHECK(has(report, "rac1-pal, Kerwan"));
    CHECK(has(report, "- gpu: a test"));
    CHECK(has(report, "100 frames (100 tick, 0 between ticks)"));
    CHECK(has(report, "20.0 frames/s"));
    // The tree: the passes indented under render.
    CHECK(has(report, "\nrender "));
    CHECK(has(report, "\n  terrain "));
    CHECK(has(report, "draw_calls 100 / 100"));
    CHECK(has(report, "mobys 120 / 120"));
    CHECK(has(report, "## Worst frames"));
}

void gpu_bound() {
    capture::Recorder r(20.0);
    for (std::uint64_t i = 0; i < 50; ++i) {
        r.add(frame(i, 40.0, 3.0, 4.0, 36.0));
    }
    CHECK(has(r.verdict(), "GPU-bound"));
}

void keeps_up() {
    capture::Recorder r(20.0);
    for (std::uint64_t i = 0; i < 50; ++i) {
        r.add(frame(i, 20.0, 3.0, 4.0, 5.0));
    }
    CHECK(has(r.verdict(), "Keeps its target"));
}

// Each side's work without its waits: the window thread's frame less the waits for the tick, the
// render thread and the present; the render thread's `render`; and what that leaves for 60 fps.
void work_without_waits() {
    capture::Recorder r(8.0);
    for (std::uint64_t i = 0; i < 20; ++i) {
        stats::Frame f = frame(i, 6.0, 1.0, 2.5, -1.0);
        f.gpu_frame_ms = 3.0;
        f.sections.push_back({"vulkan present", 1, 1.2, -1.0, 0, 0});
        f.sections.push_back({"vulkan queue present", 2, 1.0, -1.0, 0, 0});
        f.sections.push_back({"wait", 0, 1.0, -1.0, 0, 0});
        f.gauges.emplace_back("render thread wait ms", 2.0);
        r.add(f);
    }
    const std::string report = r.report({});
    CHECK(has(report, "window work       2.00"));
    CHECK(has(report, "render work       2.50"));
    const std::string v = r.verdict();
    CHECK(has(v, "processor may be 6.7x slower"));
    CHECK(has(v, "GPU 5.6x slower"));

    // Recorded on the window thread (no render thread to wait for): no render work of its own.
    capture::Recorder direct(8.0);
    direct.add(frame(0, 6.0, 1.0, 2.5, -1.0));
    CHECK(has(direct.report({}), "window work       6.00"));
    CHECK(!has(direct.report({}), "render work"));
}

void skips_paused_frames() {
    capture::Recorder r(20.0);
    stats::Frame held = frame(0, 20.0, 0.0, 1.0, -1.0);
    held.kind = stats::FrameKind::Held;
    r.add(held);
    r.add(frame(1, 20.0, 3.0, 4.0, -1.0));
    CHECK(r.frames() == 1);
    const std::string report = r.report({});
    CHECK(has(report, "1 paused or movie frames left out"));
    CHECK(has(report, "gpu") && has(report, "not measured"));
}

void empty_report() {
    capture::Recorder r(20.0);
    CHECK(has(r.report({}), "No frame was measured."));
}

// The capture as the window runs it: started, fed by stats' frames, finished by its length.
void through_stats() {
    CHECK(!stats::enabled());
    CHECK(capture::start(0.0, 20.0));
    CHECK(!capture::start(5.0, 20.0));  // one at a time
    CHECK(capture::active());
    CHECK(stats::enabled());  // the capture keeps the measurements on
    for (int i = 0; i < 5; ++i) {
        stats::begin_frame(stats::FrameKind::Tick, static_cast<std::uint64_t>(i));
        {
            const stats::Scope s("render");
        }
    }
    CHECK(capture::take_finished() == nullptr);  // runs until stopped
    capture::stop();
    // Frames are taken once a later one closes them; the first begun past the end finishes it.
    std::unique_ptr<capture::Recorder> done;
    for (int i = 5; i < 400 && !done; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));  // past the shortest length
        stats::begin_frame(stats::FrameKind::Tick, static_cast<std::uint64_t>(i));
        done = capture::take_finished();
    }
    CHECK(done != nullptr);
    CHECK(done != nullptr && done->frames() >= 4);
    CHECK(!capture::active());
    CHECK(!stats::enabled());
}

}  // namespace

int main() {
    percentiles();
    cpu_bound();
    gpu_bound();
    keeps_up();
    work_without_waits();
    skips_paused_frames();
    empty_report();
    through_stats();
    return openrac::test::result();
}

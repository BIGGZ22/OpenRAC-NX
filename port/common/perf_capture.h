// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A performance capture: the frames of a stretch of play, measured as the overlay measures them
// (common/stats.h), summed up into one short text report that a person can paste into an issue or
// a chat and that says where the frames' time went. docs/port/PERFORMANCE_CAPTURE.md describes it.
//
// The Recorder takes closed frames and keeps a row per frame and a series per timed section; its
// report holds the device and settings it is given, the frame times' percentiles, a verdict
// (CPU- or GPU-bound, and the part that costs most), every section's CPU and GPU time, the
// counters and gauges, the time line second by second and the worst frames. It knows no window
// and no GPU: the tests feed it frames of their own.
//
// The capture itself (start, stop, take_finished) is one Recorder installed as stats' frame reader
// for a set time. The window starts it (the drawer, the Developer menu, OPENRAC_PERF_CAPTURE),
// shows that it runs, and writes and copies the report once it is finished (debug/capture.cpp).

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/stats.h"

namespace openrac::perf_capture {

// What the report says about the run beside its numbers: "name: value" lines, in order (the build,
// the device, the GPU and its driver, the picture's settings, the level).
struct Context {
    std::string title;  // the report's first line after its heading: game, level, what was done
    std::vector<std::pair<std::string, std::string>> lines;
};

class Recorder {
public:
    // `target_ms`: the frame time the run should keep, in milliseconds (the game's tick, 20 for
    // PAL, or the display's interval when frames are drawn between ticks).
    explicit Recorder(double target_ms);

    // A closed frame, in order. Held frames (the game paused) and the game's own full-screen
    // pictures are counted but not measured: they say nothing of the game's cost.
    void add(const stats::Frame& frame);

    std::size_t frames() const { return m_frames.size(); }

    // From the first frame's start to the last one's end.
    double seconds() const;

    double target_ms() const { return m_target_ms; }

    // The report, as Markdown with fixed-width tables: a few kilobytes for a minute of play.
    std::string report(const Context& context) const;

    // One line: is the frame within its target, and if not, what holds it up (part of report()).
    std::string verdict() const;

private:
    struct FrameRow {
        std::uint64_t serial = 0;
        std::uint64_t tick = 0;
        stats::FrameKind kind = stats::FrameKind::Tick;
        double start_s = 0.0;
        double frame_ms = 0.0;
        double game_ms = -1.0;  // the game's code on a tick's frame; -1 on the others
        double gpu_ms = -1.0;   // -1: not measured
        // The window thread's work: the frame without its waits (for the next tick, the render
        // thread, the swapchain and the display). The render thread's (a threaded renderer's
        // `render`), -1 when the world is recorded on the window thread, inside the first.
        double main_work_ms = 0.0;
        double render_work_ms = -1.0;
        std::array<std::uint64_t, stats::kCounterCount> counters{};
        // The three costliest top-level sections, by CPU time.
        std::array<std::pair<int, double>, 3> top{};  // section series and its time; -1: none
    };

    struct Series {
        std::string name;
        int depth = 0;
        int parent = -1;         // the series of the section it is inside, -1 at the top
        std::vector<float> cpu;  // its CPU time in each frame it was in
        std::vector<float> gpu;  // its GPU time in each frame it was timed on the GPU
        double cpu_sum = 0.0, gpu_sum = 0.0;
        std::uint64_t draw_calls = 0, triangles = 0;
    };

    struct Values {
        double sum = 0.0, max = 0.0;
        std::size_t n = 0;
    };

    // The series of the section `path` names (its name and its parents' names), made when new.
    int series(const std::string& path, const std::string& name, int depth, int parent);
    // The mean of a top-level section's CPU time over every measured frame.
    double mean_cpu(const char* name) const;
    double mean_game() const;  // the game's time per frame
    double mean_gpu() const;   // over the frames timed on the GPU; -1 when none was

    // Each side's work in every frame it was measured (FrameRow): the window thread, the render
    // thread and the GPU. What a slower device has to do in each frame.
    struct Work {
        std::vector<float> main, render, gpu;
    };

    Work work() const;
    // Their p90s; -1 for the render thread or the GPU when none was measured.
    std::array<double, 3> work_p90() const;

    double m_target_ms;
    std::vector<FrameRow> m_frames;
    std::size_t m_skipped = 0;  // held and picture frames
    std::vector<Series> m_series;
    std::unordered_map<std::string, int> m_series_of;
    std::vector<std::pair<std::string, Values>> m_gauges;
};

// Returns the value at fraction `p` (0..1) of `values` by nearest rank; 0 for none. Sorts a copy.
double percentile(std::vector<float> values, double p);

// ---- The capture ----

// Starts measuring: from the next frame begun, for `seconds` (0: until stop(), at most ten
// minutes). False when a capture runs already. The frames are measured whether the overlay is
// shown or not.
bool start(double seconds, double target_ms);

// Ends the capture now; the frames begun until now are still taken as their GPU times come back.
void stop();

// A capture was started and has not been taken yet.
bool active();
// Seconds since it started, and its length (0: until stopped).
double elapsed_s();
double length_s();

// The finished capture, once every frame up to its end was taken; null until then. Taking it ends
// the frame reader: the next capture can start.
std::unique_ptr<Recorder> take_finished();

}  // namespace openrac::perf_capture

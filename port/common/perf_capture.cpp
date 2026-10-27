// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "common/perf_capture.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <iterator>
#include <mutex>

namespace openrac::perf_capture {

namespace {

using Clock = std::chrono::steady_clock;

// Sections deeper than this are kept with their parent at this depth.
constexpr int kMaxDepth = 16;
// How many of the worst frames the report lists.
constexpr std::size_t kWorstFrames = 8;
// The time line's rows at most: a long capture puts more than a second in a row.
constexpr double kTimeLineRows = 60.0;
// The frame time a weak device should keep: 60 frames a second, the game's own rate.
constexpr double kLowEndBudgetMs = 1000.0 / 60.0;
// A capture run until stopped ends by itself after this long, so its rows stay a few megabytes.
constexpr double kLongestCapture = 600.0;

template <typename... Args>
void put(std::string& out, std::format_string<Args...> fmt, Args&&... args) {
    std::format_to(std::back_inserter(out), fmt, std::forward<Args>(args)...);
}

// A count as people read it: 812, 12.4k, 1.12M.
std::string amount(double v) {
    if (v >= 1e6) {
        return std::format("{:.2f}M", v / 1e6);
    }
    if (v >= 1e4) {
        return std::format("{:.1f}k", v / 1e3);
    }
    if (v == std::floor(v)) {
        return std::format("{:.0f}", v);
    }
    return std::format("{:.1f}", v);
}

std::string ms(double v) {
    return v < 0.0 ? std::string("-") : std::format("{:.2f}", v);
}

double mean(const std::vector<float>& values) {
    if (values.empty()) {
        return -1.0;
    }
    double sum = 0.0;
    for (const float v : values) {
        sum += v;
    }
    return sum / static_cast<double>(values.size());
}

}  // namespace

double percentile(std::vector<float> values, double p) {
    if (values.empty()) {
        return 0.0;
    }
    std::sort(values.begin(), values.end());
    const double rank = std::ceil(std::clamp(p, 0.0, 1.0) * static_cast<double>(values.size()));
    const std::size_t at = rank < 1.0 ? 0 : static_cast<std::size_t>(rank) - 1;
    return values[std::min(at, values.size() - 1)];
}

Recorder::Recorder(double target_ms) : m_target_ms(target_ms > 0.0 ? target_ms : 20.0) {}

int Recorder::series(const std::string& path, const std::string& name, int depth, int parent) {
    if (const auto found = m_series_of.find(path); found != m_series_of.end()) {
        return found->second;
    }
    const int index = static_cast<int>(m_series.size());
    Series& s = m_series.emplace_back();
    s.name = name;
    s.depth = depth;
    s.parent = parent;
    m_series_of.emplace(path, index);
    return index;
}

void Recorder::add(const stats::Frame& f) {
    if (f.kind == stats::FrameKind::Held || f.kind == stats::FrameKind::Picture) {
        ++m_skipped;
        return;
    }
    FrameRow row;
    row.serial = f.serial;
    row.tick = f.tick;
    row.kind = f.kind;
    row.start_s = f.start_s;
    row.frame_ms = f.interval_ms;
    row.game_ms = f.kind == stats::FrameKind::Tick ? f.game_ms : -1.0;
    // A frame still waiting for some of its GPU times would understate the GPU's.
    row.gpu_ms = f.pending_gpu == 0 ? f.gpu_ms() : -1.0;
    row.counters = f.counters;
    row.top.fill({-1, 0.0});

    // What the window thread waited for, not worked at: the next tick, the render thread, a free
    // swapchain image and the queue's present (a driver holds it while the display or the GPU has
    // no room for another picture), the frame slot's fence.
    double waited = 0.0;
    for (const stats::Section& section : f.sections) {
        if ((section.depth == 0 && section.name == "wait") || section.name == "vulkan acquire"
            || section.name == "vulkan queue present" || section.name == "vulkan frame wait") {
            waited += section.cpu_ms;
        }
        if (section.depth == 0 && section.name == "render") {
            row.render_work_ms = std::max(row.render_work_ms, 0.0) + section.cpu_ms;
        }
    }
    const auto worker = std::find_if(f.gauges.begin(), f.gauges.end(), [](const auto& g) {
        return g.first == "render thread wait ms";
    });
    if (worker != f.gauges.end()) {
        waited += worker->second;
    } else {
        row.render_work_ms = -1.0;  // recorded on the window thread: part of its work
    }
    row.main_work_ms = std::max(0.0, f.interval_ms - waited);

    // A section's series is found by its path, its parents' names and its own: the same name may
    // be timed inside two passes. A section timed twice in a frame adds up.
    std::vector<std::string> path;
    std::vector<int> parents;
    std::vector<std::pair<double, double>> in_frame(m_series.size(), {-1.0, -1.0});
    for (const stats::Section& section : f.sections) {
        const int depth = std::clamp(section.depth, 0, kMaxDepth);
        path.resize(std::min<std::size_t>(path.size(), static_cast<std::size_t>(depth)));
        parents.resize(path.size());
        std::string key;
        for (const std::string& name : path) {
            key += name;
            key += '/';
        }
        key += section.name;
        const int index = series(
            key, section.name, static_cast<int>(path.size()), parents.empty() ? -1 : parents.back()
        );
        path.push_back(section.name);
        parents.push_back(index);
        if (in_frame.size() < m_series.size()) {
            in_frame.resize(m_series.size(), {-1.0, -1.0});
        }
        auto& [cpu, gpu] = in_frame[static_cast<std::size_t>(index)];
        cpu = std::max(cpu, 0.0) + section.cpu_ms;
        if (section.gpu_ms >= 0.0) {
            gpu = std::max(gpu, 0.0) + section.gpu_ms;
        }
        Series& s = m_series[static_cast<std::size_t>(index)];
        s.draw_calls += section.draw_calls;
        s.triangles += section.triangles;
    }
    for (std::size_t i = 0; i < in_frame.size(); ++i) {
        const auto [cpu, gpu] = in_frame[i];
        Series& s = m_series[i];
        if (cpu >= 0.0) {
            s.cpu.push_back(static_cast<float>(cpu));
            s.cpu_sum += cpu;
            // The waits for the next tick are idle time, not a cost.
            if (s.depth == 0 && s.name != "wait") {
                const auto at = std::find_if(row.top.begin(), row.top.end(), [cpu](const auto& t) {
                    return t.first < 0 || cpu > t.second;
                });
                if (at != row.top.end()) {
                    std::move_backward(at, row.top.end() - 1, row.top.end());
                    *at = {static_cast<int>(i), cpu};
                }
            }
        }
        if (gpu >= 0.0) {
            s.gpu.push_back(static_cast<float>(gpu));
            s.gpu_sum += gpu;
        }
    }

    for (const auto& [name, value] : f.gauges) {
        auto found = std::find_if(m_gauges.begin(), m_gauges.end(), [&](const auto& g) {
            return g.first == name;
        });
        if (found == m_gauges.end()) {
            m_gauges.emplace_back(name, Values{});
            found = std::prev(m_gauges.end());
        }
        Values& v = found->second;
        v.max = v.n == 0 ? value : std::max(v.max, value);
        v.sum += value;
        v.n++;
    }
    m_frames.push_back(row);
}

double Recorder::seconds() const {
    if (m_frames.empty()) {
        return 0.0;
    }
    return m_frames.back().start_s + m_frames.back().frame_ms / 1000.0 - m_frames.front().start_s;
}

double Recorder::mean_cpu(const char* name) const {
    if (m_frames.empty()) {
        return 0.0;
    }
    for (const Series& s : m_series) {
        if (s.depth == 0 && s.name == name) {
            return s.cpu_sum / static_cast<double>(m_frames.size());
        }
    }
    return 0.0;
}

double Recorder::mean_game() const {
    double sum = 0.0;
    for (const FrameRow& f : m_frames) {
        sum += std::max(f.game_ms, 0.0);
    }
    return m_frames.empty() ? 0.0 : sum / static_cast<double>(m_frames.size());
}

double Recorder::mean_gpu() const {
    double sum = 0.0;
    std::size_t n = 0;
    for (const FrameRow& f : m_frames) {
        if (f.gpu_ms >= 0.0) {
            sum += f.gpu_ms;
            n++;
        }
    }
    return n == 0 ? -1.0 : sum / static_cast<double>(n);
}

Recorder::Work Recorder::work() const {
    Work w;
    for (const FrameRow& f : m_frames) {
        w.main.push_back(static_cast<float>(f.main_work_ms));
        if (f.render_work_ms >= 0.0) {
            w.render.push_back(static_cast<float>(f.render_work_ms));
        }
        if (f.gpu_ms >= 0.0) {
            w.gpu.push_back(static_cast<float>(f.gpu_ms));
        }
    }
    return w;
}

std::array<double, 3> Recorder::work_p90() const {
    const Work w = work();
    return {
        percentile(w.main, 0.9),
        w.render.empty() ? -1.0 : percentile(w.render, 0.9),
        w.gpu.empty() ? -1.0 : percentile(w.gpu, 0.9)
    };
}

std::string Recorder::verdict() const {
    if (m_frames.empty()) {
        return "No frame was measured.";
    }
    double frame = 0.0;
    for (const FrameRow& f : m_frames) {
        frame += f.frame_ms;
    }
    frame /= static_cast<double>(m_frames.size());
    const double game = mean_game();
    const double gpu = mean_gpu();
    const double swap = mean_cpu("swap");
    // The window's thread works through these; the wait for the next tick and the swap are not
    // work. (With the Vulkan renderer the world is recorded on a thread of its own, inside
    // `render` here.)
    const double cpu = game + mean_cpu("scene") + mean_cpu("prepare") + mean_cpu("render")
                       + mean_cpu("present") + mean_cpu("overlay");

    // The costliest top-level section and, inside `render`, the costliest pass on each side.
    std::string costliest = "game tick";
    double costliest_ms = game;
    for (const Series& s : m_series) {
        const double v = s.cpu_sum / static_cast<double>(m_frames.size());
        if (s.depth == 0 && s.name != "wait" && s.name != "swap" && v > costliest_ms) {
            costliest = s.name;
            costliest_ms = v;
        }
    }
    const auto pass = [&](bool on_gpu) {
        std::string name;
        double best = 0.0;
        for (const Series& s : m_series) {
            if (s.depth != 1 || s.parent < 0
                || m_series[static_cast<std::size_t>(s.parent)].name != "render") {
                continue;
            }
            const double v =
                (on_gpu ? s.gpu_sum : s.cpu_sum) / static_cast<double>(m_frames.size());
            if (v > best) {
                best = v;
                name = std::format("{} {:.2f} ms", s.name, v);
            }
        }
        return name;
    };
    const std::string cpu_pass = pass(false);
    // The passes' own GPU times, when they hold the frame's: the Vulkan renderer times only the
    // whole frame on the GPU (its passes' times are near 0), so no pass is named there.
    double passes_gpu = 0.0;
    for (const Series& s : m_series) {
        if (s.depth == 1 && s.parent >= 0
            && m_series[static_cast<std::size_t>(s.parent)].name == "render") {
            passes_gpu += s.gpu_sum / static_cast<double>(m_frames.size());
        }
    }
    const bool passes_timed = gpu < 0.0 || passes_gpu >= 0.25 * gpu;
    const std::string gpu_pass = passes_timed ? pass(true) : std::string();

    std::string out;
    if (frame <= m_target_ms * 1.05) {
        put(out, "Keeps its target: {:.2f} ms a frame against {:.2f} ms. ", frame, m_target_ms);
    } else if (gpu >= 0.0 && gpu >= 0.8 * frame) {
        put(out,
            "GPU-bound: the GPU takes {:.2f} ms of a {:.2f} ms frame (target {:.2f}). ",
            gpu,
            frame,
            m_target_ms);
    } else if (cpu >= 0.8 * frame || (gpu >= 0.0 && cpu > gpu)) {
        put(out,
            "CPU-bound: the window's thread works {:.2f} ms of a {:.2f} ms frame (target {:.2f}), "
            "the GPU {}. ",
            cpu,
            frame,
            m_target_ms,
            gpu >= 0.0 ? std::format("{:.2f} ms", gpu) : std::string("not measured"));
    } else if (gpu < 0.0 && swap >= 0.3 * frame) {
        put(out,
            "Probably GPU-bound: no GPU timers, and the swap waits {:.2f} ms of a {:.2f} ms "
            "frame. ",
            swap,
            frame);
    } else {
        put(out,
            "Mixed: CPU work {:.2f} ms, GPU {}, swap {:.2f} ms of a {:.2f} ms frame (target "
            "{:.2f}). ",
            cpu,
            gpu >= 0.0 ? std::format("{:.2f} ms", gpu) : std::string("not measured"),
            swap,
            frame,
            m_target_ms);
    }
    put(out, "Costliest on the CPU: {} {:.2f} ms", costliest, costliest_ms);
    if (!cpu_pass.empty()) {
        put(out, " (in render: {})", cpu_pass);
    }
    put(out, ".");
    if (!gpu_pass.empty()) {
        put(out, " Costliest pass on the GPU: {}.", gpu_pass);
    } else if (!passes_timed) {
        put(out, " The passes' GPU times are not measured, only the whole frame's.");
    }
    // How much slower a device's processor and GPU can be and still keep 60 frames a second, from
    // each side's work without its waits (p90): the measure that carries over to a weaker device.
    const auto [main_work, render_work, gpu_work] = work_p90();
    const double cpu_work = std::max(main_work, render_work);
    if (cpu_work > 0.0) {
        put(out,
            " For 60 fps ({:.1f} ms) a device's processor may be {:.1f}x slower than this one's",
            kLowEndBudgetMs,
            kLowEndBudgetMs / cpu_work);
        if (gpu_work > 0.0) {
            put(out, ", its GPU {:.1f}x slower at this resolution", kLowEndBudgetMs / gpu_work);
        }
        put(out, ".");
    }
    return out;
}

std::string Recorder::report(const Context& context) const {
    std::string out;
    put(out, "# OpenRAC diagnostics recording\n\n");
    if (!context.title.empty()) {
        put(out, "{}\n\n", context.title);
    }
    for (const auto& [name, value] : context.lines) {
        put(out, "- {}: {}\n", name, value);
    }
    std::size_t ticks = 0, between = 0;
    std::vector<float> frame_ms, game_ms;
    for (const FrameRow& f : m_frames) {
        frame_ms.push_back(static_cast<float>(f.frame_ms));
        if (f.kind == stats::FrameKind::Tick) {
            ticks++;
            game_ms.push_back(static_cast<float>(std::max(f.game_ms, 0.0)));
        } else {
            between++;
        }
    }
    const double length = seconds();
    put(out,
        "- capture: {:.1f} s, {} frames ({} tick, {} between ticks{}), target {:.2f} ms ({:.0f} "
        "fps)\n",
        length,
        m_frames.size(),
        ticks,
        between,
        m_skipped > 0 ? std::format(", {} paused or movie frames left out", m_skipped)
                      : std::string(),
        m_target_ms,
        1000.0 / m_target_ms);
    if (m_frames.empty()) {
        put(out, "\nNo frame was measured.\n");
        return out;
    }

    put(out, "\n## Verdict\n\n{}\n", verdict());

    // ---- Frame times ----
    const double median = percentile(frame_ms, 0.5);
    std::size_t hitches = 0;
    for (const float v : frame_ms) {
        hitches += v > 2.0 * median ? 1 : 0;
    }
    put(out, "\n## Frame times (ms)\n\n```\n");
    put(out, "{:<14}{:>8}{:>8}{:>8}{:>8}{:>8}{:>8}\n", "", "avg", "p50", "p90", "p99", "max", "n");
    const auto line = [&](const char* name, const std::vector<float>& v) {
        if (v.empty()) {
            put(out, "{:<14}{:>8}\n", name, "not measured");
            return;
        }
        put(out,
            "{:<14}{:>8.2f}{:>8.2f}{:>8.2f}{:>8.2f}{:>8.2f}{:>8}\n",
            name,
            mean(v),
            percentile(v, 0.5),
            percentile(v, 0.9),
            percentile(v, 0.99),
            percentile(v, 1.0),
            v.size());
    };
    const Work w = work();
    line("frame", frame_ms);
    line("game tick", game_ms);
    line("gpu", w.gpu);
    // Each side's work without its waits: on a slower device the frame is at least the largest.
    line("window work", w.main);
    if (!w.render.empty()) {
        line("render work", w.render);
    }
    put(out,
        "```\n\n{:.1f} frames/s, {:.1f} ticks/s; {} frames over twice the median ({:.1f} ms).\n",
        length > 0.0 ? static_cast<double>(m_frames.size()) / length : 0.0,
        length > 0.0 ? static_cast<double>(ticks) / length : 0.0,
        hitches,
        2.0 * median);

    // ---- Sections ----
    // In their tree: each after its parent, in the order they were first timed.
    put(out,
        "\n## Sections (ms a frame over every frame; p95 and max over the frames they ran "
        "in)\n\n```\n");
    put(out,
        "{:<34}{:>7}{:>7}{:>7}{:>8}{:>7}{:>8}{:>8}\n",
        "section",
        "cpu",
        "p95",
        "max",
        "gpu",
        "p95",
        "draws",
        "tris");
    const double n = static_cast<double>(m_frames.size());
    std::vector<int> order;
    const auto visit = [&](auto&& self, int parent) -> void {
        for (std::size_t i = 0; i < m_series.size(); ++i) {
            if (m_series[i].parent == parent) {
                order.push_back(static_cast<int>(i));
                self(self, static_cast<int>(i));
            }
        }
    };
    visit(visit, -1);
    for (const int i : order) {
        const Series& s = m_series[static_cast<std::size_t>(i)];
        std::string name(static_cast<std::size_t>(s.depth) * 2, ' ');
        name += s.name;
        if (name.size() > 33) {
            name.resize(33);
        }
        put(out,
            "{:<34}{:>7.2f}{:>7.2f}{:>7.2f}{:>8}{:>7}{:>8}{:>8}\n",
            name,
            s.cpu_sum / n,
            percentile(s.cpu, 0.95),
            percentile(s.cpu, 1.0),
            s.gpu.empty() ? "-" : ms(s.gpu_sum / n),
            s.gpu.empty() ? "-" : ms(percentile(s.gpu, 0.95)),
            amount(static_cast<double>(s.draw_calls) / n),
            amount(static_cast<double>(s.triangles) / n));
    }
    put(out, "```\n");

    // ---- Counters and gauges ----
    put(out, "\n## Counters (a frame: avg / max)\n\n");
    for (std::size_t c = 0; c < stats::kCounterCount; ++c) {
        double sum = 0.0, max = 0.0;
        for (const FrameRow& f : m_frames) {
            const double v = static_cast<double>(f.counters[c]);
            sum += v;
            max = std::max(max, v);
        }
        put(out,
            "{}{} {} / {}",
            c == 0 ? "" : ", ",
            stats::counter_name(static_cast<stats::Counter>(c)),
            amount(sum / n),
            amount(max));
    }
    put(out, "\n");
    if (!m_gauges.empty()) {
        std::vector<std::pair<std::string, Values>> gauges = m_gauges;
        std::sort(gauges.begin(), gauges.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
        put(out, "\n## Gauges (avg / max)\n\n");
        bool first = true;
        for (const auto& [name, v] : gauges) {
            if (v.max == 0.0 && v.sum == 0.0) {
                continue;  // never set to anything: says nothing here
            }
            put(out,
                "{}{} {} / {}",
                first ? "" : ", ",
                name,
                amount(v.sum / static_cast<double>(v.n)),
                amount(v.max));
            first = false;
        }
        put(out, "\n");
    }

    // ---- The time line ----
    const double bucket = std::max(1.0, std::ceil(length / kTimeLineRows));
    put(out, "\n## Time line ({:.0f} s a row)\n\n```\n", bucket);
    put(out, "{:>6}{:>7}{:>9}{:>9}{:>9}{:>9}\n", "s", "fps", "frame", "max", "game", "gpu");
    const double origin = m_frames.front().start_s;
    for (std::size_t at = 0; at < m_frames.size();) {
        const double row_start =
            origin + bucket * std::floor((m_frames[at].start_s - origin) / bucket);
        std::size_t count = 0, row_ticks = 0, row_gpu = 0;
        double sum = 0.0, max = 0.0, game = 0.0, gpu = 0.0;
        for (; at < m_frames.size() && m_frames[at].start_s < row_start + bucket; ++at) {
            const FrameRow& f = m_frames[at];
            count++;
            sum += f.frame_ms;
            max = std::max(max, f.frame_ms);
            if (f.game_ms >= 0.0) {
                game += f.game_ms;
                row_ticks++;
            }
            if (f.gpu_ms >= 0.0) {
                gpu += f.gpu_ms;
                row_gpu++;
            }
        }
        put(out,
            "{:>6.0f}{:>7.1f}{:>9.2f}{:>9.2f}{:>9}{:>9}\n",
            row_start - origin,
            sum > 0.0 ? 1000.0 * static_cast<double>(count) / sum : 0.0,
            sum / static_cast<double>(count),
            max,
            row_ticks > 0 ? ms(game / static_cast<double>(row_ticks)) : std::string("-"),
            row_gpu > 0 ? ms(gpu / static_cast<double>(row_gpu)) : std::string("-"));
    }
    put(out, "```\n");

    // ---- The worst frames ----
    std::vector<std::size_t> worst(m_frames.size());
    for (std::size_t i = 0; i < worst.size(); ++i) {
        worst[i] = i;
    }
    const std::size_t shown = std::min(kWorstFrames, worst.size());
    std::partial_sort(
        worst.begin(),
        worst.begin() + static_cast<std::ptrdiff_t>(shown),
        worst.end(),
        [&](std::size_t a, std::size_t b) { return m_frames[a].frame_ms > m_frames[b].frame_ms; }
    );
    put(out, "\n## Worst frames\n\n");
    for (std::size_t k = 0; k < shown; ++k) {
        const FrameRow& f = m_frames[worst[k]];
        put(out,
            "- {:.2f} ms at {:.1f} s (tick {}, {}): game {}, gpu {}",
            f.frame_ms,
            f.start_s - origin,
            f.tick,
            stats::frame_kind_name(f.kind),
            ms(f.game_ms),
            ms(f.gpu_ms));
        for (const auto& [index, time] : f.top) {
            if (index >= 0) {
                put(out, ", {} {:.2f}", m_series[static_cast<std::size_t>(index)].name, time);
            }
        }
        const auto counter = [&](stats::Counter c) {
            return f.counters[static_cast<std::size_t>(c)];
        };
        put(out,
            "; draws {}, texture uploads {} ({} KB), buffer uploads {} KB\n",
            amount(static_cast<double>(counter(stats::Counter::DrawCalls))),
            counter(stats::Counter::TextureUploads),
            counter(stats::Counter::TextureUploadBytes) / 1024,
            counter(stats::Counter::BufferUploadBytes) / 1024);
    }
    return out;
}

// ---- The capture ----

namespace {

struct Session {
    std::mutex mutex;
    std::unique_ptr<Recorder> running;
    std::unique_ptr<Recorder> finished;
    bool active = false;
    Clock::time_point started{};
    double length = 0.0;        // seconds; 0 until stop() when started without one
    double first_start = -1.0;  // the first frame's start, on stats' clock
};

Session& session() {
    static Session s;
    return s;
}

double since(Clock::time_point t) {
    return std::chrono::duration<double>(Clock::now() - t).count();
}

// stats' frame reader: each closed frame, in order, until the first one begun after the end.
void read(const stats::Frame& frame, void*) {
    Session& s = session();
    const std::lock_guard lock(s.mutex);
    if (!s.running) {
        return;
    }
    if (s.first_start < 0.0) {
        s.first_start = frame.start_s;
    }
    const double length = s.length > 0.0 ? s.length : kLongestCapture;
    if (frame.start_s - s.first_start >= length) {
        s.finished = std::move(s.running);
        return;
    }
    s.running->add(frame);
}

}  // namespace

bool start(double seconds, double target_ms) {
    Session& s = session();
    {
        const std::lock_guard lock(s.mutex);
        if (s.active) {
            return false;
        }
        s.running = std::make_unique<Recorder>(target_ms);
        s.finished.reset();
        s.active = true;
        s.started = Clock::now();
        s.length = std::max(seconds, 0.0);
        s.first_start = -1.0;
    }
    stats::set_reader(read, nullptr);
    return true;
}

void stop() {
    Session& s = session();
    const std::lock_guard lock(s.mutex);
    if (s.running && (s.length == 0.0 || since(s.started) < s.length)) {
        s.length = std::max(since(s.started), 1e-3);
    }
}

bool active() {
    Session& s = session();
    const std::lock_guard lock(s.mutex);
    return s.active;
}

double elapsed_s() {
    Session& s = session();
    const std::lock_guard lock(s.mutex);
    return s.active ? since(s.started) : 0.0;
}

double length_s() {
    Session& s = session();
    const std::lock_guard lock(s.mutex);
    return s.length;
}

std::unique_ptr<Recorder> take_finished() {
    Session& s = session();
    std::unique_ptr<Recorder> done;
    {
        const std::lock_guard lock(s.mutex);
        // Frames come back a few late; one that never comes (the window stopped drawing) ends the
        // capture two seconds after its time with what it has.
        if (!s.finished && s.running && s.length > 0.0 && since(s.started) > s.length + 2.0) {
            s.finished = std::move(s.running);
        }
        if (!s.finished) {
            return nullptr;
        }
        done = std::move(s.finished);
        s.active = false;
    }
    stats::set_reader(nullptr, nullptr);
    return done;
}

}  // namespace openrac::perf_capture

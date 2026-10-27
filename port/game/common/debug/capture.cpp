// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's performance capture (debug/overlay.h, common/perf_capture.h): started from
// the drawer, the Developer menu, Ctrl+F3 or OPENRAC_PERF_CAPTURE, shown in the status badge while
// it runs, and once it is finished zipped with the frames it measured and copied to the
// clipboard, ready to paste. docs/port/PERFORMANCE_CAPTURE.md describes it.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <imgui.h>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "common/build_info.h"
#include "common/env.h"
#include "common/log.h"
#include "common/perf_capture.h"
#include "common/sampler.h"
#include "common/slow_cpu.h"
#include "common/stats.h"
#include "common/system_info.h"
#include "common/zip_writer.h"
#include "debug/overlay.h"
#include "options_menu.h"
#include "renderer/gl.h"
#include "renderer/gl_stats.h"
#include "scene/stb_extra.h"
#include "settings.h"

namespace openrac::frontend::debug_overlay {

namespace {

using Clock = std::chrono::steady_clock;

// The capture lengths the drawer and the menu offer, in seconds; 0 runs until stopped.
constexpr double kLengths[] = {15.0, 30.0, 60.0, 120.0, 0.0};
// The simulated slow processor's factors they offer (common/slow_cpu.h); 1 is off.
constexpr int kSlowFactors[] = {1, 2, 4, 6};

// What the capture keeps between frames.
struct Capture {
    bool configured = false;
    std::optional<Clock::time_point> start_at;  // a start asked for, after a countdown
    bool wait_for_drawer = false;               // and once the drawer is out of the picture
    std::string path;                           // the report's file; empty: one of its own
    bool hid_panel = false;                     // the panel was hidden for the capture
    bool opened_dump = false;                   // the frame dump beside it is the capture's own
    // OPENRAC_PERF_CAPTURE=FRAME:SECONDS[:FILE]: a capture by script.
    std::uint64_t script_frame = 0;
    double script_seconds = 0.0;
    std::string script_path;
    bool scripted = false;
    // What the device said when the capture started, said again at its end.
    std::string clocks_at_start, thermal_at_start;
    int level_at_start = -1;
    std::uint64_t tick_at_start = 0;
};

Capture c;

void configure_capture() {
    if (c.configured) {
        return;
    }
    c.configured = true;
    if (const std::optional<std::string_view> v = env::value(env::Var::PerfCapture)) {
        const std::string text(*v);
        char* end = nullptr;
        c.script_frame = std::strtoull(text.c_str(), &end, 10);
        if (end != nullptr && *end == ':') {
            c.script_seconds = std::strtod(end + 1, &end);
            if (end != nullptr && *end == ':') {
                c.script_path = end + 1;
            }
        }
        c.scripted = c.script_seconds > 0.0;
        if (!c.scripted) {
            log::warn("OPENRAC_PERF_CAPTURE: expected FRAME:SECONDS[:FILE], got {}", text);
        }
    }
}

// The folder the captures go to: the working folder's perf/; on Android, whose apps run in "/",
// the app's own files (HOME), which the Files app shows as "Diagnostics recordings".
std::filesystem::path capture_dir() {
    std::filesystem::path dir = "perf";
#if defined(__ANDROID__)
    if (const std::optional<std::string> home = env::system_variable("HOME");
        home && !home->empty()) {
        dir = std::filesystem::path(*home) / "perf";
    }
#endif
    std::error_code ignored;
    std::filesystem::create_directories(dir, ignored);
    return dir;
}

std::string level_name(int level) {
    return level >= 0 && level < static_cast<int>(kLevelNames.size())
               ? std::format("{} (level {})", kLevelNames[static_cast<std::size_t>(level)], level)
               : std::string("the boot program");
}

// The frame time the run should keep: the game's tick, or the display's interval when frames are
// drawn between ticks.
double target_ms(const Shown& shown) {
    if (shown.interpolating && shown.refresh_interval > 0.0) {
        return shown.refresh_interval * 1000.0;
    }
    return shown.tick_period > 0.0 ? shown.tick_period * 1000.0 : 20.0;
}

void begin_capture(const Shown& shown, double seconds, std::string path) {
    if (!perf_capture::start(seconds, target_ms(shown))) {
        return;
    }
    c.path = std::move(path);
    sampler::start();  // where the window and render threads' time goes (common/sampler.h)
    c.clocks_at_start = system_info::cpu_clocks();
    c.thermal_at_start = system_info::thermal();
    c.level_at_start = s.level;
    c.tick_at_start = s.tick;
    // The panel costs a frame time of its own: hidden while the capture runs (the badge stays).
    c.hid_panel = s.visible;
    if (s.visible) {
        set_visible(false);
    }
    // The frames themselves as well, beside the report, unless a dump is open already.
    c.opened_dump = false;
    if (!stats::dumping()) {
        std::string error;
        std::filesystem::path frames = c.path;
        frames.replace_extension(".jsonl");
        if (c.path.empty()) {
            frames = capture_dir() / "capture_frames.jsonl";  // zipped with the report
        }
        c.opened_dump = stats::open_dump(frames.string(), error);
        if (!c.opened_dump) {
            log::warn("performance capture: {}", error);
        }
    }
    log::info(
        "performance capture: started, {}",
        seconds > 0.0 ? std::format("{:.0f} s", seconds) : "until stopped"
    );
}

// The report's lines about the build, the device, the GPU and the picture.
perf_capture::Context context(const Shown& shown) {
    perf_capture::Context ctx;
    const int level = s.level;
    ctx.title = std::format(
        "{}, {}{}, ticks {} to {}",
        s.game.empty() ? "?" : s.game,
        level_name(level),
        level != c.level_at_start ? std::format(" (started in {})", level_name(c.level_at_start))
                                  : std::string(),
        c.tick_at_start,
        s.tick
    );
    const auto add = [&ctx](const char* name, std::string value) {
        if (!value.empty()) {
            ctx.lines.emplace_back(name, std::move(value));
        }
    };
    add("build", std::format("OpenRAC {} ({})", build_info::version(), build_info::commit()));
    add("system", system_info::os());
    add("processor", std::format("{}, {} memory", system_info::cpu(), system_info::memory()));
    const std::string clocks = system_info::cpu_clocks();
    if (!clocks.empty()) {
        add("cpu clocks", std::format("start {}; end {}", c.clocks_at_start, clocks));
    }
    const std::string thermal = system_info::thermal();
    if (!thermal.empty()) {
        add("thermal", std::format("start {}; end {}", c.thermal_at_start, thermal));
    }
    const renderer::gl_stats::Info& gpu = renderer::gl_stats::info();
    const char* api = s.gl_subset ? "Vulkan" : gl::is_es() ? "OpenGL ES" : "OpenGL";
    add("graphics",
        std::format(
            "{} on {} ({}), {}{}",
            api,
            gpu.renderer,
            gpu.vendor,
            gpu.version,
            gpu.software ? ", SOFTWARE RASTERISER" : ""
        ));
    add("gpu timers", gpu.timer_queries || stats::gpu_timers() ? "yes" : "no (GPU columns empty)");
    add("window",
        std::format(
            "{}x{}, the picture {}x{} in it, frame buffer {}x{}",
            shown.window_width,
            shown.window_height,
            shown.box_width,
            shown.box_height,
            shown.frame_width,
            shown.frame_height
        ));
    add("pacing",
        std::format(
            "tick {:.2f} ms, vsync {}, frames between ticks {}, refresh {}",
            shown.tick_period * 1000.0,
            shown.vsync ? "on" : "off",
            shown.interpolating ? "on" : "off",
            shown.refresh_interval > 0.0 ? std::format("{:.2f} ms", shown.refresh_interval * 1000.0)
                                         : std::string("unknown")
        ));
    std::string settings_line;
    using namespace options_menu;
    for (const int row :
         {kAspect,
          kRenderScale,
          kMsaa,
          kAnisotropy,
          kSceneryDetail,
          kFrameRate,
          kFrameCap,
          kVSync}) {
        settings_line +=
            std::format("{}{} {}", settings_line.empty() ? "" : ", ", label(row), value(row));
    }
    add("settings", settings_line);
    if (slow_cpu::factor() > 1) {
        add("simulated slow cpu",
            std::format(
                "{}x: the window and render threads' processor times here are {} times this "
                "device's (the GPU's are not)",
                slow_cpu::factor(),
                slow_cpu::factor()
            ));
    }
    const auto& graphics = settings::get();
    if (gl::enhanced_rendering_available()) {
        add("enhanced",
            std::format(
                "{}{}{}{}",
                graphics.enhanced_graphics ? "on" : "off",
                graphics.enhanced_shadows ? ", modern shadows" : "",
                graphics.probe_lighting ? ", probe lighting" : "",
                graphics.gtao ? ", GTAO" : ""
            ));
    }
    const stats::ProcessMemory memory = stats::process_memory();
    add("process",
        std::format(
            "{} resident{}",
            megabytes(memory.resident),
            memory.peak > 0 ? std::format(", peak {}", megabytes(memory.peak)) : std::string()
        ));
    return ctx;
}

// Raw deflate by stb_image_write's compressor (scene/stb_extra.h): a zlib stream, whose two-byte
// header and four-byte checksum zip does not keep.
std::vector<std::uint8_t> deflate(std::span<const std::uint8_t> data) {
    int length = 0;
    const std::unique_ptr<unsigned char, decltype(&std::free)> packed(
        stbi_zlib_compress(
            const_cast<unsigned char*>(data.data()), static_cast<int>(data.size()), &length, 8
        ),
        &std::free
    );
    std::vector<std::uint8_t> raw;
    if (packed != nullptr && length > 6) {
        raw.assign(packed.get() + 2, packed.get() + length - 4);
    }
    return raw;
}

std::vector<std::uint8_t> file_bytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void finish_capture(const Shown& shown, const perf_capture::Recorder& recorder) {
    // The hot functions of the threads sampled while it ran, at the report's end.
    const std::string report = recorder.report(context(shown)) + sampler::stop();
    if (c.opened_dump) {
        stats::close_dump();
    }
    // UTC, which every system's library formats the same way.
    const std::string stamp = std::format(
        "{:%Y%m%d-%H%M%S}",
        std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())
    );
    const std::filesystem::path dir = capture_dir();
    const std::string base = std::format("perf_{}_{}", s.game.empty() ? "game" : s.game, stamp);
    std::filesystem::path path =
        c.path.empty() ? dir / (base + ".md") : std::filesystem::path(c.path);
    std::error_code ignored;
    bool written = false;
    if (c.path.empty()) {
        // One zip of the report and its frames, a tenth of their size (a minute of frames is tens
        // of megabytes as text), to be sent on whole.
        std::vector<zip_writer::Entry> entries;
        entries.push_back({base + ".md", std::vector<std::uint8_t>(report.begin(), report.end())});
        const std::filesystem::path frames = dir / "capture_frames.jsonl";
        if (c.opened_dump) {
            entries.push_back({base + ".jsonl", file_bytes(frames)});
        }
        std::string error;
        const std::filesystem::path zip = dir / (base + ".zip");
        written = zip_writer::write(zip, entries, deflate, error);
        if (written) {
            path = zip;
            if (c.opened_dump) {
                std::filesystem::remove(frames, ignored);
            }
        } else {
            log::warn("performance capture: {}", error);
            // The loose files, then.
            if (c.opened_dump) {
                std::filesystem::rename(frames, dir / (base + ".jsonl"), ignored);
            }
        }
    }
    c.opened_dump = false;
    if (!written) {
        if (std::ofstream out(path, std::ios::binary); out) {
            out << report;
            written = static_cast<bool>(out);
        }
    }
    const bool copied = SDL_SetClipboardText(report.c_str());
    log::info(
        "performance capture: {} frames, report {}{}",
        recorder.frames(),
        written ? path.string() : "not written",
        copied ? ", copied to the clipboard" : ""
    );
    s.toast = std::format(
        "Diagnostics report {}{}",
        copied ? "copied to the clipboard" : "ready",
        written ? " · saved to " + path.string() : std::string(" (could not be saved)")
    );
    s.toast_until = ImGui::GetTime() + 8.0;
    s.message = s.toast;
    if (c.hid_panel) {
        set_visible(true);
        c.hid_panel = false;
    }
}

}  // namespace

const char* capture_length_name() {
    const double seconds = s.capture_seconds;
    static std::string name;
    name = seconds > 0.0 ? std::format("{:.0f} s", seconds) : std::string("Until stopped");
    return name.c_str();
}

std::string slow_cpu_name() {
    return slow_cpu::factor() > 1 ? std::format("{}x", slow_cpu::factor()) : std::string("Off");
}

void step_slow_cpu(int by) {
    constexpr int n = static_cast<int>(std::size(kSlowFactors));
    int at = 0;
    for (int i = 0; i < n; ++i) {
        if (kSlowFactors[i] == slow_cpu::factor()) {
            at = i;
        }
    }
    slow_cpu::set_factor(kSlowFactors[(at + by + n) % n]);
}

void step_capture_length(int by) {
    constexpr int n = static_cast<int>(std::size(kLengths));
    int at = 0;
    for (int i = 0; i < n; ++i) {
        if (kLengths[i] == s.capture_seconds) {
            at = i;
        }
    }
    s.capture_seconds = kLengths[(at + by + n) % n];
}

void toggle_capture(bool after_drawer) {
    if (perf_capture::active()) {
        perf_capture::stop();
        return;
    }
    if (c.start_at) {
        c.start_at.reset();  // asked again during the countdown: not after all
        return;
    }
    // From the drawer: a moment to put the thumbs back on the controls once it has gone.
    c.wait_for_drawer = after_drawer;
    c.start_at = Clock::now() + std::chrono::seconds(after_drawer ? 3 : 0);
    c.path.clear();
}

bool capture_busy() {
    // Asked before every picture's overlay, so the script's capture is known from the first one.
    configure_capture();
    return c.start_at.has_value() || perf_capture::active() || c.scripted;
}

bool capture_pending() {
    return c.start_at.has_value();
}

std::string capture_badge() {
    if (c.start_at) {
        const double left = std::chrono::duration<double>(*c.start_at - Clock::now()).count();
        return c.wait_for_drawer
                   ? std::string("Recording starts when the menu closes")
                   : std::format("Recording starts in {:.0f}", std::max(left, 0.0) + 0.5);
    }
    if (!perf_capture::active()) {
        return {};
    }
    const double length = perf_capture::length_s();
    const double elapsed = perf_capture::elapsed_s();
    if (length > 0.0 && elapsed >= length) {
        return "Recording finished, writing the report";
    }
    return length > 0.0 ? std::format("REC {:.0f} / {:.0f} s", elapsed, length)
                        : std::format("REC {:.0f} s", elapsed);
}

void capture_frame(const Shown& shown) {
    configure_capture();
    if (c.scripted && s.tick >= c.script_frame && !perf_capture::active()) {
        c.scripted = false;
        begin_capture(shown, c.script_seconds, c.script_path);
    }
    if (c.start_at) {
        if (c.wait_for_drawer && (s.drawer_open || s.drawer_at > 0.0f)) {
            c.start_at = Clock::now() + std::chrono::seconds(3);  // counted from when it is gone
        } else {
            c.wait_for_drawer = false;
            if (Clock::now() >= *c.start_at) {
                c.start_at.reset();
                begin_capture(shown, s.capture_seconds, {});
            }
        }
    }
    if (perf_capture::active()) {
        // Opening the drawer pauses the game: the capture ends with what it has.
        if (s.drawer_open) {
            perf_capture::stop();
        }
        if (const std::unique_ptr<perf_capture::Recorder> done = perf_capture::take_finished()) {
            finish_capture(shown, *done);
        }
    }
}

}  // namespace openrac::frontend::debug_overlay

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac_render_bench: the render benchmark. It draws the synthetic scene of render_fixture.h (or,
// for the maintainers, an extracted level) on the Vulkan renderer, in a hidden window that shows
// nothing and writes no picture, and prints one JSON line per frame (the frame's sections with
// their CPU and GPU milliseconds, every vk_* gauge, the heap allocations made inside the frame's
// render call) and one summary line (the frame times' median, 95th percentile and maximum, and the
// render counters over the load, the warm-up and the measured frames). docs/port/RENDER_PERF.md
// says what each number is; port/tools/perf/render_bench.py runs it over a matrix of settings.
//
//   openrac_render_bench [--size WxH] [--frames N] [--warmup M] [--enhanced on|off]
//                        [--gpu-driven on|off] [--materials on|off] [--cache cold|warm]
//                        [--opt-off LIST] [--scenario NAME] [--camera static|walk|turn]
//                        [--between K] [--msaa N] [--tris N] [--seed N] [--level DIR]
//
// It exits 77 (skipped) where Vulkan cannot open, 1 when a check fails (a validation error, a
// scene that cannot be made), 2 for options it cannot read.
//
// The program replaces the global operator new and delete to count what is allocated on its own
// thread while a frame's render call runs (bench_heap_allocs, bench_heap_bytes): the deterministic
// measure of the draw path's allocations.

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <new>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/render_opt.h"
#include "renderer/vulkan/vkgl.h"
#include "scene/level.h"
#include "tests/gpu_draw_reference.h"
#include "tests/render_fixture.h"
#include "tests/renderer_gl_context.h"

namespace {

namespace fs = std::filesystem;
namespace render = openrac::test::render;
namespace vkgl = openrac::vkgl;
using openrac::stats::FrameKind;

// The exit status for options the program cannot read.
constexpr int kBadOptions = 2;

// Pictures drawn after the last measured one, so that its GPU times are back when it is printed:
// the backend reads a frame's timers when its slot comes round again, two frames on.
constexpr std::uint64_t kLag = 3;

// ---- Counting the heap ----

// The allocations this thread makes while counting, and their bytes. Thread-local, so the driver's
// own threads (MoltenVK's, NVIDIA's) are not counted: the measure is the draw path's alone.
struct HeapCount {
    bool counting = false;     // True inside a frame's render call.
    std::uint64_t allocs = 0;  // operator new calls since counting began.
    std::uint64_t bytes = 0;   // The bytes they asked for.
};

// The counts of the thread that draws; operator new below, which has no other way to reach them,
// adds to them (section 11.4's exception for a callback without a user pointer).
thread_local HeapCount tls_heap;

}  // namespace

// The global allocation functions, replaced in this program only, counting while tls_heap says so.
// The memory comes from the C library: the one place a test may allocate raw memory, since
// counting the heap cannot use the heap it counts (check_conventions.py's ALLOCATION_EXCEPTIONS).
void* operator new(std::size_t size) {
    // Counted only inside a frame's render call on this thread.
    if (tls_heap.counting) {
        ++tls_heap.allocs;
        tls_heap.bytes += size;
    }

    void* memory = std::malloc(size == 0 ? 1 : size);

    // The C library has no memory left: what operator new says then.
    if (memory == nullptr) {
        throw std::bad_alloc();
    }

    return memory;
}

// The array form, counted as the single one.
void* operator new[](std::size_t size) {
    return ::operator new(size);
}

// Gives back what operator new took.
void operator delete(void* memory) noexcept {
    std::free(memory);
}

// The sized form, which C++14 calls where it knows the size.
void operator delete(void* memory, std::size_t /*size*/) noexcept {
    std::free(memory);
}

// The array form.
void operator delete[](void* memory) noexcept {
    std::free(memory);
}

// The sized array form.
void operator delete[](void* memory, std::size_t /*size*/) noexcept {
    std::free(memory);
}

namespace {

// ---- Options ----

// What the command line asks for (the file comment lists the options).
struct Options {
    int width = 1920;  // The picture, in pixels.
    int height = 1080;
    std::uint64_t frames = 60;  // Pictures measured.
    std::uint64_t warmup = 10;  // Pictures drawn first and not measured.
    bool enhanced = false;      // Sun shadows and GTAO.
    bool gpu_driven = true;  // The GPU-driven instances (OPENRAC_GPU_DRIVEN_OFF's reference off).
    bool materials = true;   // The native material path (OPENRAC_GPU_MATERIALS_OFF's off).
    bool warm = false;       // The run drawn twice in one process; the second one printed.
    std::string opt_off;     // Registry entries switched off, as OPENRAC_RENDER_OPT_OFF says.
    std::string scenario = "walk";
    std::optional<render::CameraPath> camera;  // Over the scenario's, when given.
    std::optional<int> between;                // Over the scenario's, when given.
    int samples = 1;                           // MSAA.
    std::optional<std::uint64_t> triangles;    // Over the scene's default.
    std::uint64_t seed = 1;
    fs::path level;  // An extracted level to draw in place of the synthetic one.
};

// Returns whether `text` is on or off into `out`; false for anything else.
bool read_switch(std::string_view text, bool& out) {
    // The two words a switch takes.
    if (text == "on" || text == "off") {
        out = text == "on";
        return true;
    }

    return false;
}

// Returns `text` read as a whole number of at least `least`, or nothing.
std::optional<std::uint64_t> read_count(std::string_view text, std::uint64_t least) {
    std::uint64_t value = 0;

    // Digits only: no sign, no space, no suffix.
    for (const char c : text) {
        // A character that is not a digit ends the reading as a failure.
        if (c < '0' || c > '9') {
            return std::nullopt;
        }

        value = value * 10 + static_cast<std::uint64_t>(c - '0');
    }

    return !text.empty() && value >= least ? std::optional<std::uint64_t>(value) : std::nullopt;
}

// Returns the value of option `name` at `argv[n + 1]`, moving `n` past it; nothing at the end.
std::optional<std::string_view> value_of(int argc, char** argv, int& n) {
    // An option at the end of the line has no value.
    if (n + 1 >= argc) {
        return std::nullopt;
    }

    ++n;
    return std::string_view(argv[n]);
}

// Reads --size WxH into `options`. False, with `error`, unless both sides are at least 16 pixels.
bool read_size(std::string_view value, Options& options, std::string& error) {
    constexpr std::uint64_t kLeast = 16;
    const std::size_t x = value.find('x');
    const auto width = read_count(value.substr(0, x), kLeast);
    const auto height =
        x == std::string_view::npos ? std::nullopt : read_count(value.substr(x + 1), kLeast);

    // Both sides, each a number of at least kLeast.
    if (!width || !height) {
        error = "--size takes WxH, each side at least 16";
        return false;
    }

    options.width = static_cast<int>(*width);
    options.height = static_cast<int>(*height);
    return true;
}

// Reads one of the options that take a whole number into `options`. False, with `error`, when
// `value` is not one (or is 0 for --frames and --msaa, which need at least one).
bool read_number(
    std::string_view name, std::string_view value, Options& options, std::string& error
) {
    const bool positive = name == "--frames" || name == "--msaa";
    const auto count = read_count(value, positive ? 1 : 0);

    // Digits, and at least 1 where 0 means nothing to draw.
    if (!count) {
        error = std::string(name) + " takes a whole number";
        return false;
    }

    // Each number into its place.
    if (name == "--frames") {
        options.frames = *count;
    } else if (name == "--warmup") {
        options.warmup = *count;
    } else if (name == "--between") {
        options.between = static_cast<int>(*count);
    } else if (name == "--msaa") {
        options.samples = static_cast<int>(*count);
    } else if (name == "--tris") {
        options.triangles = *count;
    } else {
        options.seed = *count;
    }

    return true;
}

// Reads one of the on-or-off options into `options`. False, with `error`, for another word.
bool read_on_off(
    std::string_view name, std::string_view value, Options& options, std::string& error
) {
    bool& target = name == "--enhanced"     ? options.enhanced
                   : name == "--gpu-driven" ? options.gpu_driven
                                            : options.materials;
    error = std::string(name) + " takes on or off";
    return read_switch(value, target);
}

// Reads one of the options that take a word or a path into `options`. False, with `error`, for
// a word the option does not take.
bool read_word(
    std::string_view name, std::string_view value, Options& options, std::string& error
) {
    // --cache cold or warm.
    if (name == "--cache") {
        options.warm = value == "warm";
        error = "--cache takes cold or warm";
        return value == "warm" || value == "cold";
    }

    // --camera static, walk or turn.
    if (name == "--camera") {
        options.camera = render::camera_path(value);
        error = "--camera takes static, walk or turn";
        return value == "static" || value == "walk" || value == "turn";
    }

    // --scenario, one of render_fixture.h's.
    if (name == "--scenario") {
        options.scenario = std::string(value);
        error = "no scenario " + std::string(value);
        return render::find_scenario(value) != nullptr;
    }

    // --opt-off, a list checked when the registry is asked (switch_off).
    if (name == "--opt-off") {
        options.opt_off = std::string(value);
        return true;
    }

    options.level = fs::path(value);
    return true;
}

// Reads one option `name` with its `value` into `options`. False, with `error`, when it is not one.
bool read_option(
    std::string_view name, std::string_view value, Options& options, std::string& error
) {
    constexpr std::string_view kNumbers[] =
        {"--frames", "--warmup", "--between", "--msaa", "--tris", "--seed"};
    constexpr std::string_view kSwitches[] = {"--enhanced", "--gpu-driven", "--materials"};
    constexpr std::string_view kWords[] =
        {"--cache", "--camera", "--scenario", "--opt-off", "--level"};

    // The picture's size.
    if (name == "--size") {
        return read_size(value, options, error);
    }

    // A whole number.
    if (std::ranges::find(kNumbers, name) != std::end(kNumbers)) {
        return read_number(name, value, options, error);
    }

    // On or off.
    if (std::ranges::find(kSwitches, name) != std::end(kSwitches)) {
        return read_on_off(name, value, options, error);
    }

    // A word or a path.
    if (std::ranges::find(kWords, name) != std::end(kWords)) {
        return read_word(name, value, options, error);
    }

    error = "unknown option " + std::string(name);
    return false;
}

// Returns the options of the command line, or nothing with `error`.
std::optional<Options> read_options(int argc, char** argv, std::string& error) {
    Options options;

    // Each option and its value.
    for (int n = 1; n < argc; ++n) {
        const std::string_view name = argv[n];
        const auto value = value_of(argc, argv, n);

        // Every option takes a value.
        if (!value) {
            error = std::string(name) + " needs a value";
            return std::nullopt;
        }

        // A name or a value it cannot read.
        if (!read_option(name, *value, options, error)) {
            return std::nullopt;
        }
    }

    return options;
}

// ---- The environment it runs in ----

// Sets the environment variable `name` to `value` for this process. Called before anything reads
// the environment: common/env.h reads it once, at the first question.
void set_variable(const char* name, const std::string& value) {
#if defined(_WIN32)
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}

// A private folder for the pipeline cache, removed when the run ends, so that a cold run starts
// with no cache and no run leaves one behind.
class CacheFolder {
public:
    // Makes the folder under the system's temporary folder, named after this process.
    CacheFolder() {
        std::error_code error;
        m_path = fs::temp_directory_path(error)
                 / ("openrac-render-bench-"
                    + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(m_path, error);
    }

    CacheFolder(const CacheFolder&) = delete;
    CacheFolder& operator=(const CacheFolder&) = delete;

    // Removes the folder and the cache file in it.
    ~CacheFolder() {
        std::error_code error;
        fs::remove_all(m_path, error);
    }

    // The cache file the backend reads and writes (OPENRAC_VULKAN_PIPELINE_CACHE).
    [[nodiscard]] fs::path file() const { return m_path / "pipelines.bin"; }

private:
    fs::path m_path;  // The folder.
};

// ---- Printing ----

// Returns `text` as a JSON string, quoted, with its quotes, backslashes and control characters
// escaped.
std::string json_string(std::string_view text) {
    std::string out = "\"";

    // Each character, escaped where JSON needs it.
    for (const char c : text) {
        // The two that end or escape a string, and the control characters.
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (static_cast<unsigned char>(c) < 0x20) {
            out += ' ';
        } else {
            out += c;
        }
    }

    return out + "\"";
}

// Returns `value` as a JSON number (a finite one; NaN and infinities as null).
std::string json_number(double value) {
    // JSON has no NaN or infinity.
    if (!std::isfinite(value)) {
        return "null";
    }

    char text[32];
    std::snprintf(text, sizeof text, "%.6g", value);
    return text;
}

// Prints the closed frame `frame` (picture `index` of the measured ones) as one JSON line.
void print_frame(const openrac::stats::Frame& frame, std::uint64_t index, double picture_ms) {
    std::string line = "{\"frame\":" + std::to_string(index) + ",\"kind\":"
                       + json_string(openrac::stats::frame_kind_name(frame.kind)) + ",\"tick\":"
                       + std::to_string(frame.tick) + ",\"alpha\":" + json_number(frame.alpha)
                       + ",\"picture_ms\":" + json_number(picture_ms) + ",\"sections\":[";

    // Each timed section, in the order it was opened.
    for (std::size_t n = 0; n < frame.sections.size(); ++n) {
        const auto& section = frame.sections[n];
        line += (n == 0 ? "" : ",") + std::string("{\"name\":") + json_string(section.name)
                + ",\"depth\":" + std::to_string(section.depth) + ",\"cpu_ms\":"
                + json_number(section.cpu_ms) + ",\"gpu_ms\":" + json_number(section.gpu_ms) + "}";
    }

    line += "],\"counters\":{";

    // The frame's counters of common/stats.h (draw calls, triangles, uploads), by name.
    for (std::size_t n = 0; n < openrac::stats::kCounterCount; ++n) {
        const auto counter = static_cast<openrac::stats::Counter>(n);
        line += (n == 0 ? "" : ",") + json_string(openrac::stats::counter_name(counter)) + ":"
                + std::to_string(frame.counters[n]);
    }

    line += "},\"gauges\":{";

    // Each gauge, by name.
    for (std::size_t n = 0; n < frame.gauges.size(); ++n) {
        line += (n == 0 ? "" : ",") + json_string(frame.gauges[n].first) + ":"
                + json_number(frame.gauges[n].second);
    }

    std::printf("%s}}\n", line.c_str());
}

// Returns the counters of `after` less those of `before`, as a JSON object: each counter's part.
std::string counters_json(
    const vkgl::SubmissionCounters& before, const vkgl::SubmissionCounters& after
) {
    // The counters in the order of SubmissionCounters (docs/port/RENDER_PERF.md, "The counters").
    // clang-format off: one counter per row.
    const std::pair<const char*, std::uint64_t vkgl::SubmissionCounters::*> kCounters[] = {
        {"vk_pipelines_created", &vkgl::SubmissionCounters::pipelines_created},
        {"vk_pipelines_created_after_warmup", &vkgl::SubmissionCounters::pipelines_created_after_warmup},
        {"vk_descriptor_sets_allocated", &vkgl::SubmissionCounters::descriptor_sets_allocated},
        {"vk_descriptor_writes", &vkgl::SubmissionCounters::descriptor_writes},
        {"vk_render_passes", &vkgl::SubmissionCounters::render_passes},
        {"vk_attachment_load_bytes", &vkgl::SubmissionCounters::attachment_load_bytes},
        {"vk_attachment_store_bytes", &vkgl::SubmissionCounters::attachment_store_bytes},
        {"vk_barriers", &vkgl::SubmissionCounters::barriers},
        {"vk_barriers_all_commands", &vkgl::SubmissionCounters::barriers_all_commands},
        {"vk_submits", &vkgl::SubmissionCounters::submits},
        {"vk_frames_presented", &vkgl::SubmissionCounters::frames_presented},
        {"vk_fullscreen_copies", &vkgl::SubmissionCounters::fullscreen_copies},
        {"vk_ring_bytes", &vkgl::SubmissionCounters::ring_bytes},
        {"vk_texture_creations", &vkgl::SubmissionCounters::texture_creations},
        {"vk_texture_upload_bytes", &vkgl::SubmissionCounters::texture_upload_bytes},
        {"vk_buffer_static_bytes_host_visible", &vkgl::SubmissionCounters::buffer_static_bytes_host_visible},
        {"vk_gpu_payload_bytes", &vkgl::SubmissionCounters::gpu_payload_bytes},
        {"vk_shader_compiles", &vkgl::SubmissionCounters::shader_compiles},
        {"vk_uniform_uploads", &vkgl::SubmissionCounters::uniform_uploads},
        {"vk_descriptor_sets", &vkgl::SubmissionCounters::descriptor_sets},
    };
    // clang-format on
    // The kinds of command, in the order of vkgl::Command.
    const char* const kKinds[] = {
        "vk_cmds_bind_pipeline",
        "vk_cmds_bind_descriptors",
        "vk_cmds_bind_buffers",
        "vk_cmds_dynamic_state",
        "vk_cmds_draw",
        "vk_cmds_barrier",
        "vk_cmds_copy",
        "vk_cmds_dispatch",
        "vk_cmds_render_pass",
        "vk_cmds_query",
        "vk_cmds_other",
    };
    std::string out =
        "{\"vk_cmds_recorded\":"
        + std::to_string(vkgl::commands_recorded(after) - vkgl::commands_recorded(before));

    // Each kind of command.
    for (std::size_t kind = 0; kind < vkgl::kCommandKinds; ++kind) {
        out += ",\"" + std::string(kKinds[kind])
               + "\":" + std::to_string(after.commands[kind] - before.commands[kind]);
    }

    // Each of the other counters.
    for (const auto& [name, member] : kCounters) {
        out += ",\"" + std::string(name) + "\":" + std::to_string(after.*member - before.*member);
    }

    out += ",\"vk_cpu_mirror_bytes\":" + std::to_string(after.cpu_mirror_bytes);
    out += ",\"shader_compile_s\":" + json_number(after.shader_compile_s - before.shader_compile_s);
    return out + "}";
}

// Returns the `share` quantile (0 to 1) of `values`, the nearest rank; 0 for none.
double quantile(std::vector<double> values, double share) {
    // Nothing measured.
    if (values.empty()) {
        return 0.0;
    }

    std::sort(values.begin(), values.end());
    const auto rank =
        static_cast<std::size_t>(share * static_cast<double>(values.size() - 1) + 0.5);
    return values[std::min(rank, values.size() - 1)];
}

// ---- The run ----

// What one run measured, for the summary line.
struct Measured {
    vkgl::SubmissionCounters at_open;  // When the context had opened.
    vkgl::SubmissionCounters at_load;  // When the scene had been uploaded.
    vkgl::SubmissionCounters at_warm;  // After the warm-up's pictures.
    vkgl::SubmissionCounters at_end;   // After the measured pictures.
    double load_ms = 0.0;              // The scene's upload.
    double first_ms = 0.0;             // The first picture after the upload.
    std::vector<double> picture_ms;    // Each measured picture.
    std::uint64_t heap_allocs = 0;     // Inside the measured pictures' render calls.
    std::uint64_t heap_bytes = 0;
    std::uint64_t draw_calls = 0;  // The renderer's draw calls in the measured pictures.
    std::uint64_t triangles = 0;   // And their triangles (Renderer::last_stats).
};

// Returns the milliseconds since `start`.
double ms_since(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
        .count();
}

// Draws one run of `options` on the scene of `level`: the upload, the warm-up and the measured
// pictures, printing each measured one when its GPU times are back if `print`. False with `error`
// when the scene cannot be made.
bool run(
    openrac::test::GlContext& context,
    const openrac::viewer::LevelData& level,
    const render::SceneSize& size,
    const Options& options,
    bool print,
    Measured& out,
    std::string& error
) {
    const render::Scenario& scenario = *render::find_scenario(options.scenario);
    const render::CameraPath path = options.camera.value_or(scenario.camera);
    const int between = options.between.value_or(scenario.between);
    const render::Walk walk = render::walk_over(level);
    const float aspect = static_cast<float>(options.width) / static_cast<float>(options.height);
    const std::uint64_t pictures = options.warmup + options.frames;
    const double ticks = static_cast<double>(pictures) / static_cast<double>(between + 1);

    out.at_open = vkgl::submission_counters();
    const auto load_start = std::chrono::steady_clock::now();
    render::SceneRenderer scene;
    render::DrawOptions draw{options.width, options.height, options.samples, options.enhanced};

    // The scene on the GPU: meshes, textures, shaders, the renderers and the picture.
    if (!scene.open(level, draw, error)) {
        return false;
    }

    out.load_ms = ms_since(load_start);
    out.at_load = vkgl::submission_counters();
    vkgl::begin_pipeline_warmup(static_cast<std::uint32_t>(options.warmup));
    std::vector<double> times(pictures + kLag, 0.0);

    // Every picture, then kLag more so that the last measured one's GPU times are back.
    for (std::uint64_t index = 0; index < pictures + kLag; ++index) {
        const render::Picture shown = render::picture(index, between);
        openrac::stats::begin_frame(
            shown.between ? FrameKind::Between : FrameKind::Tick, shown.tick, shown.alpha
        );

        // The picture kLag back is closed now, with its GPU times: printed if it is measured.
        if (print && index >= options.warmup + kLag) {
            const std::uint64_t measured = index - kLag - options.warmup;
            const openrac::stats::Frame& closed = *openrac::stats::history(kLag - 1);
            print_frame(closed, measured, times[index - kLag]);
        }

        // Halfway, a level change: the level uploaded again.
        if (scenario.reupload && index == options.warmup + options.frames / 2
            && !scene.reupload(level, error)) {
            return false;
        }

        // The warm-up has passed: what follows is measured.
        if (index == options.warmup) {
            out.at_warm = vkgl::submission_counters();
        }

        const double tick = static_cast<double>(shown.tick - 1) + shown.alpha;
        const auto camera = render::camera_at(walk, path, tick, ticks, aspect);
        const render::MobyFrame mobys = render::mobys_at(level, size, shown);
        const auto start = std::chrono::steady_clock::now();
        tls_heap = HeapCount{true, 0, 0};
        scene.draw(camera, mobys, index);
        const HeapCount heap = tls_heap;
        tls_heap.counting = false;
        context.window().swap();
        times[index] = ms_since(start);
        openrac::stats::gauge("bench_heap_allocs", static_cast<double>(heap.allocs));
        openrac::stats::gauge("bench_heap_bytes", static_cast<double>(heap.bytes));

        // The first picture after the upload: what a level's first frame costs.
        if (index == 0) {
            out.first_ms = times[index];
        }

        // A measured picture: its time and its allocations go into the summary.
        if (index >= options.warmup && index < pictures) {
            out.picture_ms.push_back(times[index]);
            out.heap_allocs += heap.allocs;
            out.heap_bytes += heap.bytes;
            out.draw_calls += static_cast<std::uint64_t>(scene.renderer().last_stats().draw_calls);
            out.triangles += static_cast<std::uint64_t>(scene.renderer().last_stats().triangles);
        }

        // The last measured picture is drawn: the counters stop here.
        if (index + 1 == pictures) {
            out.at_end = vkgl::submission_counters();
        }
    }

    return true;
}

// Prints the summary line of `measured`, a run of `options`.
void print_summary(const Options& options, const Measured& measured) {
    const double frames = static_cast<double>(std::max<std::size_t>(measured.picture_ms.size(), 1));
    std::string line = "{\"summary\":{\"device\":" + json_string(vkgl::device_name())
                       + ",\"driver\":" + json_string(vkgl::driver_version()) + ",\"size\":\""
                       + std::to_string(options.width) + "x" + std::to_string(options.height)
                       + "\",\"scenario\":" + json_string(options.scenario)
                       + ",\"enhanced\":" + (options.enhanced ? "true" : "false")
                       + ",\"gpu_driven\":" + (openrac::test::gpu_draw_active() ? "true" : "false")
                       + ",\"materials\":" + (vkgl::gpu_materials_active() ? "true" : "false")
                       + ",\"cache\":" + (options.warm ? "\"warm\"" : "\"cold\"")
                       + ",\"frames\":" + std::to_string(measured.picture_ms.size());
    line += ",\"picture_ms\":{\"median\":" + json_number(quantile(measured.picture_ms, 0.5))
            + ",\"p95\":" + json_number(quantile(measured.picture_ms, 0.95))
            + ",\"max\":" + json_number(quantile(measured.picture_ms, 1.0)) + "}";
    line += ",\"load_ms\":" + json_number(measured.load_ms)
            + ",\"first_picture_ms\":" + json_number(measured.first_ms);
    line += ",\"bench_heap_allocs\":" + std::to_string(measured.heap_allocs)
            + ",\"bench_heap_allocs_per_frame\":"
            + json_number(static_cast<double>(measured.heap_allocs) / frames)
            + ",\"bench_heap_bytes\":" + std::to_string(measured.heap_bytes)
            + ",\"draw_calls\":" + std::to_string(measured.draw_calls)
            + ",\"triangles\":" + std::to_string(measured.triangles);
    line += ",\"load\":" + counters_json(measured.at_open, measured.at_load);
    line += ",\"warmup\":" + counters_json(measured.at_load, measured.at_warm);
    line += ",\"counters\":" + counters_json(measured.at_warm, measured.at_end);
    line += ",\"validation_errors\":" + std::to_string(measured.at_end.validation_errors);
    std::printf("%s}}\n", line.c_str());
}

// Switches the registry entries `list` names off (OPENRAC_RENDER_OPT_OFF's syntax). False with
// `error` for a name that is no entry's.
bool switch_off(std::string_view list, std::string& error) {
    namespace render_opt = openrac::renderer::render_opt;
    const render_opt::OffList off = render_opt::parse_off_list(list);

    // A misspelt entry would measure the optimised path while saying the reference.
    if (!off.unknown.empty()) {
        error = "--opt-off names " + off.unknown.front() + ", which is no optimisation";
        return false;
    }

    // Every entry, for all.
    for (const render_opt::Entry& entry : render_opt::entries()) {
        // Off when the list says all or names it.
        if (off.all || std::ranges::find(off.off, entry.opt) != off.off.end()) {
            render_opt::set(entry.opt, false);
        }
    }

    return true;
}

// Returns the level to draw: the synthetic one of `size`, or the extracted one `options` names.
std::optional<openrac::viewer::LevelData> level_for(
    const Options& options, const render::SceneSize& size, std::string& error
) {
    // The synthetic scene, unless a level was named.
    if (options.level.empty()) {
        return render::make_level(size);
    }

    openrac::viewer::LevelData level;

    // An extracted level the maintainers point at (docs/port/RENDER_PERF.md, their commands).
    if (!openrac::viewer::load_level(options.level, level, error)) {
        return std::nullopt;
    }

    return level;
}

}  // namespace

int main(int argc, char** argv) {
    std::string error;
    const std::optional<Options> read = read_options(argc, argv, error);

    // Options it cannot read: say which and stop.
    if (!read) {
        std::fprintf(stderr, "render_bench: %s\n", error.c_str());
        return kBadOptions;
    }

    const Options& options = *read;
    const CacheFolder cache;
    // The benchmark measures the Vulkan renderer, with a pipeline cache of its own.
    set_variable("OPENRAC_GL_API", "vulkan");
    set_variable("OPENRAC_VULKAN_PIPELINE_CACHE", cache.file().string());
    openrac::gl::set_enhanced_rendering(options.enhanced);
    openrac::test::GlContext context;

    // The benchmark measures the Vulkan renderer only.
    if (!context.open(error, options.width, options.height) || !vkgl::active()) {
        std::printf("SKIP: Vulkan unavailable: %s\n", error.c_str());
        return openrac::test::kSkip;
    }

    // The references the matrix compares against, and the registry entries switched off.
    openrac::test::gpu_draw_reference(!options.gpu_driven);
    openrac::test::gpu_material_reference(!options.materials);

    // A misspelt entry stops the run.
    if (!switch_off(options.opt_off, error)) {
        std::fprintf(stderr, "render_bench: %s\n", error.c_str());
        return kBadOptions;
    }

    render::SceneSize size;
    size.seed = options.seed;
    size.triangles = options.triangles.value_or(size.triangles);
    const std::optional<openrac::viewer::LevelData> level = level_for(options, size, error);

    // A level that cannot be read is a failure, not a skip.
    if (!level) {
        std::fprintf(stderr, "render_bench: %s\n", error.c_str());
        return 1;
    }

    openrac::stats::set_enabled(openrac::stats::Client::Overlay, true);
    Measured measured;

    // A warm run draws everything once first, unprinted, with the pipelines it makes kept.
    if (options.warm && !run(context, *level, size, options, false, measured, error)) {
        std::fprintf(stderr, "render_bench: %s\n", error.c_str());
        return 1;
    }

    measured = Measured{};

    // The measured run.
    if (!run(context, *level, size, options, true, measured, error)) {
        std::fprintf(stderr, "render_bench: %s\n", error.c_str());
        return 1;
    }

    print_summary(options, measured);
    return measured.at_end.validation_errors == 0 ? 0 : 1;
}

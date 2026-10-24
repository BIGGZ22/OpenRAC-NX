// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the port measures about itself, for the debug overlay (game/common/debug_overlay.h) and the
// per-frame dump (--debug-dump): counters, timed sections and the frames they belong to. Any code
// of the port may count and time through here; nothing here draws or knows OpenGL (the renderer
// installs the GPU timers: renderer/gl_stats.h).
//
// The shape is OpenGOAL's frame profiler (game/graphics/opengl_renderer/Profiler.h; ISC License,
// Copyright (c) 2020-2026 OpenGOAL Team): a tree of named sections per frame, each with its time
// and the draw calls and triangles made inside it. Here the tree is a flat list with depths, a
// frame is one picture shown (a game tick's own, or one drawn between two ticks), and a section may
// also be timed on the GPU.
//
//     stats::count(stats::Counter::DrawCalls);        // always counted: one add
//     { stats::Scope s("mobys", stats::kGpu); ... }   // timed while someone is looking
//     stats::gauge("ui quads", quads.size());         // a value of this frame, by name
//
// Counters are always on (an add each). Sections, gauges and the frame history only work while
// enabled() (the overlay is shown or a dump is open); otherwise each costs one test of a bool.
// Everything is called from the thread that runs the game and draws.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace openrac::stats {

// ---- Counters: per frame, set to zero when a frame begins ----

enum class Counter : std::uint8_t {
    DrawCalls,
    Triangles,
    TextureUploads,      // images given to the GPU (glTexImage2D with pixels)
    TextureUploadBytes,
    BufferUploadBytes,   // vertex, index and uniform data given to the GPU
    ProgramBinds,        // changes of shader program
    FramebufferBinds,
    Count,
};

inline constexpr std::size_t kCounterCount = static_cast<std::size_t>(Counter::Count);

const char* counter_name(Counter counter);

extern std::array<std::uint64_t, kCounterCount> g_counters;

inline void count(Counter counter, std::uint64_t by = 1) {
    g_counters[static_cast<std::size_t>(counter)] += by;
}

// ---- Who is looking ----

extern bool g_enabled;

inline bool enabled() {
    return g_enabled;
}

enum class Client : std::uint8_t { Overlay, Dump };

// The overlay and the dump each ask for the measurements; they are taken while either does.
void set_enabled(Client client, bool on);

// ---- Sections: timed parts of a frame ----

inline constexpr unsigned kGpu = 1;  // Scope: time the section's GL commands on the GPU as well

struct Section {
    std::string name;
    int depth = 0;
    double cpu_ms = 0.0;
    double gpu_ms = -1.0;  // -1: not measured (not asked for, no timer queries, or not back yet)
    std::uint64_t draw_calls = 0;
    std::uint64_t triangles = 0;
};

// Times from its construction to its destruction, in the frame that is open. Sections nest.
class Scope {
public:
    explicit Scope(std::string_view name, unsigned flags = 0) {
        if (g_enabled) {
            begin(name, flags);
        }
    }

    ~Scope() {
        if (m_index >= 0) {
            end();
        }
    }

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

private:
    void begin(std::string_view name, unsigned flags);
    void end();

    int m_index = -1;
    bool m_gpu = false;
    std::uint64_t m_serial = 0;
};

// A top-level section that ends now and began where the last top-level section ended (or the frame
// began): for timing the stretches of a long function without bracing them.
void lap(std::string_view name);

// A value of the frame that is open, by name (the last one given wins).
void gauge(std::string_view name, double value);

// ---- Frames ----

enum class FrameKind : std::uint8_t {
    Tick,     // a game tick's own picture (or, without a window, the tick itself)
    Between,  // a picture drawn between two ticks, the scene part way from one to the next
    Picture,  // a full-screen picture the game shows itself (a movie's frame, a boot still)
    Held,     // the last tick drawn again while the game is paused
};

const char* frame_kind_name(FrameKind kind);

struct Frame {
    std::uint64_t serial = 0;  // counts every frame begun
    std::uint64_t tick = 0;    // the game tick it shows
    FrameKind kind = FrameKind::Tick;
    float alpha = 1.0f;        // how far from the last tick's scene to this tick's (1: all the way)
    double start_s = 0.0;      // seconds since the program's first frame
    double interval_ms = 0.0;  // to the next frame's start: the frame time
    double game_ms = 0.0;      // the game's own code since the last tick (a tick's frame only)
    bool closed = false;
    int pending_gpu = 0;       // GPU times asked for and not back yet
    std::array<std::uint64_t, kCounterCount> counters{};
    std::vector<Section> sections;
    std::vector<std::pair<std::string, double>> gauges;

    // The time of the top-level section of that name, or 0.
    double cpu_ms(std::string_view section) const;
    // All the GPU time measured in the frame, or -1 when none was.
    double gpu_ms() const;
};

// Closes the open frame and opens the next. Called once per picture shown (and once per tick
// without a window). Cheap while not enabled(): the counters are zeroed and nothing is kept.
void begin_frame(FrameKind kind, std::uint64_t tick, float alpha = 1.0f);

// Of the open frame, once they are known.
void set_frame(std::uint64_t tick, float alpha);

// The game's own code stops running (the port takes over to draw) and starts again: the time
// between a resume and the next yield is the game's, given to the next tick's frame.
void game_yielded();
void game_resumed();

// The closed frames, the newest first: history(0) is the last one closed. Null past the end.
inline constexpr std::size_t kHistory = 512;
const Frame* history(std::size_t back);
std::size_t history_size();

// ---- GPU timers: installed by the renderer (renderer/gl_stats.h) ----

struct GpuHooks {
    // Starts timing what is drawn from now for that frame's section; false if it cannot (a timer
    // is already running: they do not nest).
    bool (*begin)(std::uint64_t serial, int section) = nullptr;
    void (*end)() = nullptr;
    // Once a frame: hands over the times that came back (gpu_result).
    void (*poll)() = nullptr;
};

void set_gpu_hooks(const GpuHooks& hooks);
bool gpu_timers();  // hooks are installed
void gpu_result(std::uint64_t serial, int section, double ms);
#if defined(OPENRAC_GPU_MARKERS)
std::string gpu_section_name(std::uint64_t serial, int section);
#endif

// ---- The dump: one JSON object per frame, one per line (docs/port/DEBUGGING.md) ----

bool open_dump(const std::string& path, std::string& error);
void close_dump();  // writes the frames still held; also done when the program ends
bool dumping();

// ---- Probes: the OPENRAC_* switches read from the environment ----
//
// A switch that is on when its variable is set, as the port's probes always were, kept in a list
// so the overlay can show them and turn them on and off while the game runs:
//
//     static const bool& cam_log = stats::probe("OPENRAC_CAM_LOG", "the camera of each tick, in the log");
//
// The variable is a flag of common/env.h's table, read through env::flag; each probe's row reads
// as Truth::Set. A name the table does not hold as a flag starts off, with a warning in the log.

struct Probe {
    const char* variable;
    const char* what;
    bool on;
};

const bool& probe(const char* variable, const char* what);
std::vector<Probe*>& probes();

// ---- Shader sources: the GLSL files built into the program (cmake/Window.cmake) ----
//
// Each embedded file with its path under port/ and its text as built in, and the port/ folder the
// build read them from: what a program's sources are matched against to find the files to read
// again (renderer/gl_stats.h, reload).

struct ShaderFile {
    const char* path;  // under port/, as "renderer/shaders/direct.vert"
    const char* text;
};

int embedded_shader(const char* path, const char* text);  // returns 0: called from initialisers
int embedded_shader_root(const char* port_dir);
const std::vector<ShaderFile>& embedded_shaders();
const char* shader_root();  // "" when no shader was built in

// ---- The host process ----

struct ProcessMemory {
    std::uint64_t resident = 0;   // bytes of physical memory the process holds
    std::uint64_t footprint = 0;  // what the system charges it for, where it says (macOS); else 0
    std::uint64_t peak = 0;       // the most resident so far, where the system says; else 0
};

ProcessMemory process_memory();

// Seconds of processor time the process has used (all threads), for a CPU load figure.
double process_cpu_seconds();

}  // namespace openrac::stats

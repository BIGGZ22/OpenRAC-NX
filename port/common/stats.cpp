// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "common/stats.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <format>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "common/env.h"
#include "common/log.h"

namespace openrac::stats {

std::array<std::uint64_t, kCounterCount> g_counters{};
bool g_enabled = false;

namespace {

using Clock = std::chrono::steady_clock;

double ms_between(Clock::time_point from, Clock::time_point to) {
    return std::chrono::duration<double, std::milli>(to - from).count();
}

struct OpenSection {
    int index;
    Clock::time_point started;
    std::uint64_t draw_calls, triangles;
};

// How many frames a frame waits for its GPU times before it is written without them.
constexpr std::uint64_t kDumpWait = 16;

struct State {
    std::mutex probes_mutex;  // Guards lazy probe registration across game and render threads.
    // Each consumer can stop without disabling the others' measurements.
    std::array<bool, static_cast<std::size_t>(Client::Count)> clients{};
    // The frames, a ring by serial; frames[serial % kHistory]. `open`: the one being filled.
    std::vector<Frame> frames = std::vector<Frame>(kHistory);
    std::uint64_t next_serial = 0;  // the serial the next frame gets
    Frame* open = nullptr;
    Clock::time_point origin{};
    Clock::time_point open_started{};
    Clock::time_point top_ended{};  // where the last top-level section of the open frame ended
    std::vector<OpenSection> stack;
    // The game's own time since the last tick's frame.
    Clock::time_point game_since{};
    bool game_running = false;
    double game_ms = 0.0;
    GpuHooks gpu;
    std::FILE* dump = nullptr;
    std::uint64_t dump_next = 0;  // the serial of the next frame to write
    FrameReader reader = nullptr;
    void* reader_user = nullptr;
    std::uint64_t reader_next = 0;  // the serial of the next frame to hand the reader
};

State& state() {
    static State s;
    return s;
}

Frame* frame_of(std::uint64_t serial) {
    State& s = state();
    if (serial >= s.next_serial || s.next_serial - serial > kHistory) {
        return nullptr;
    }
    Frame& f = s.frames[serial % kHistory];
    return f.serial == serial ? &f : nullptr;
}

void write_json_string(std::string& out, std::string_view text) {
    out += '"';
    for (const char c : text) {
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (static_cast<unsigned char>(c) < 0x20) {
            std::format_to(std::back_inserter(out), "\\u{:04x}", c);
        } else {
            out += c;
        }
    }
    out += '"';
}

// One frame as a line of JSON, made in memory and written at once: hundreds of small writes, each
// taking the file's lock, cost a phone's window thread more than the frame's own counting.
void write_frame(std::FILE* out, const Frame& f) {
    thread_local std::string line;
    line.clear();
    const auto add = std::back_inserter(line);
    std::format_to(
        add,
        "{{\"frame\":{},\"tick\":{},\"kind\":\"{}\",\"alpha\":{:.3f},\"t\":{:.6f},\"frame_ms\":{:."
        "3f}",
        f.serial,
        f.tick,
        frame_kind_name(f.kind),
        static_cast<double>(f.alpha),
        f.start_s,
        f.interval_ms
    );
    if (f.kind == FrameKind::Tick) {
        std::format_to(add, ",\"game_ms\":{:.3f}", f.game_ms);
    }
    const double gpu = f.gpu_ms();
    if (gpu >= 0.0) {
        std::format_to(add, ",\"gpu_ms\":{:.3f}", gpu);
    } else {
        line += ",\"gpu_ms\":null";
    }
    for (std::size_t i = 0; i < kCounterCount; ++i) {
        std::format_to(add, ",\"{}\":{}", counter_name(static_cast<Counter>(i)), f.counters[i]);
    }
    line += ",\"sections\":[";
    for (std::size_t i = 0; i < f.sections.size(); ++i) {
        const Section& s = f.sections[i];
        line += i == 0 ? "{\"name\":" : ",{\"name\":";
        write_json_string(line, s.name);
        std::format_to(add, ",\"depth\":{},\"cpu_ms\":{:.3f}", s.depth, s.cpu_ms);
        if (s.gpu_ms >= 0.0) {
            std::format_to(add, ",\"gpu_ms\":{:.3f}", s.gpu_ms);
        }
        std::format_to(add, ",\"draw_calls\":{},\"triangles\":{}}}", s.draw_calls, s.triangles);
    }
    line += "],\"gauges\":{";
    for (std::size_t i = 0; i < f.gauges.size(); ++i) {
        if (i != 0) {
            line += ',';
        }
        write_json_string(line, f.gauges[i].first);
        std::format_to(add, ":{:.6g}", f.gauges[i].second);
    }
    line += "}}\n";
    std::fwrite(line.data(), 1, line.size(), out);
}

// Hands `take` the closed frames from serial `next` on whose GPU times are back, or that have
// waited long enough (`all`: every closed frame), moving `next` past them.
template <typename Take>
void flush_frames(std::uint64_t& next, bool all, Take&& take) {
    State& s = state();
    while (next < s.next_serial) {
        if (s.next_serial - next > kHistory) {
            next = s.next_serial - kHistory;  // fell out of the ring unread
        }
        const Frame* f = frame_of(next);
        if (f == nullptr || !f->closed) {
            break;
        }
        if (!all && f->pending_gpu > 0 && s.next_serial - next < kDumpWait) {
            break;
        }
        take(*f);
        ++next;
    }
}

// Writes the frames due to the dump and hands the reader its own.
void flush_dump(bool all) {
    State& s = state();
    if (s.dump != nullptr) {
        flush_frames(s.dump_next, all, [&s](const Frame& f) { write_frame(s.dump, f); });
    }
    // Read once: the reader may be taken away between two frames, never in the middle of one.
    if (const FrameReader reader = s.reader; reader != nullptr) {
        flush_frames(s.reader_next, all, [&s, reader](const Frame& f) {
            reader(f, s.reader_user);
        });
    }
}

void close_open(Clock::time_point now) {
    State& s = state();
    if (s.open == nullptr) {
        return;
    }
    // Sections left open (a frame begun inside one): closed with the frame.
    while (!s.stack.empty()) {
        const OpenSection& o = s.stack.back();
        s.open->sections[static_cast<std::size_t>(o.index)].cpu_ms = ms_between(o.started, now);
        s.stack.pop_back();
    }
    s.open->interval_ms = ms_between(s.open_started, now);
    s.open->counters = g_counters;
    s.open->closed = true;
    s.open = nullptr;
}

}  // namespace

const char* counter_name(Counter counter) {
    switch (counter) {
        case Counter::DrawCalls:
            return "draw_calls";
        case Counter::Triangles:
            return "triangles";
        case Counter::TextureUploads:
            return "texture_uploads";
        case Counter::TextureUploadBytes:
            return "texture_upload_bytes";
        case Counter::BufferUploadBytes:
            return "buffer_upload_bytes";
        case Counter::ProgramBinds:
            return "program_binds";
        case Counter::FramebufferBinds:
            return "framebuffer_binds";
        case Counter::Count:
            break;
    }
    return "?";
}

const char* frame_kind_name(FrameKind kind) {
    switch (kind) {
        case FrameKind::Tick:
            return "tick";
        case FrameKind::Between:
            return "between";
        case FrameKind::Picture:
            return "picture";
        case FrameKind::Held:
            return "held";
    }
    return "?";
}

void set_enabled(Client client, bool on) {
    State& s = state();
    s.clients[static_cast<std::size_t>(client)] = on;
    const bool enabled =
        std::any_of(s.clients.begin(), s.clients.end(), [](bool active) { return active; });
    if (!enabled && g_enabled) {
        close_open(Clock::now());
        s.stack.clear();
    }
    g_enabled = enabled;
}

std::string open_section() {
    const State& s = state();
    if (!g_enabled || s.open == nullptr || s.stack.empty()) {
        return {};
    }
    return s.open->sections[static_cast<std::size_t>(s.stack.back().index)].name;
}

void Scope::begin(std::string_view name, unsigned flags) {
    State& s = state();
    if (s.open == nullptr) {
        return;
    }
    m_index = static_cast<int>(s.open->sections.size());
    m_serial = s.open->serial;
    Section& section = s.open->sections.emplace_back();
    section.name.assign(name);
    section.depth = static_cast<int>(s.stack.size());
    s.stack.push_back({m_index, Clock::now(), g_counters[static_cast<std::size_t>(Counter::DrawCalls)],
                       g_counters[static_cast<std::size_t>(Counter::Triangles)]});
    if ((flags & kGpu) != 0 && s.gpu.begin != nullptr && s.gpu.begin(m_serial, m_index)) {
        m_gpu = true;
        s.open->pending_gpu++;
    }
}

void Scope::end() {
    State& s = state();
    if (m_gpu && s.gpu.end != nullptr) {
        s.gpu.end();
    }
    // The frame it was opened in may be closed by now (a frame begun inside the section).
    if (s.open == nullptr || s.open->serial != m_serial || s.stack.empty() || s.stack.back().index != m_index) {
        return;
    }
    const OpenSection o = s.stack.back();
    s.stack.pop_back();
    Section& section = s.open->sections[static_cast<std::size_t>(m_index)];
    const auto now = Clock::now();
    section.cpu_ms = ms_between(o.started, now);
    if (s.stack.empty()) {
        s.top_ended = now;
    }
    section.draw_calls = g_counters[static_cast<std::size_t>(Counter::DrawCalls)] - o.draw_calls;
    section.triangles = g_counters[static_cast<std::size_t>(Counter::Triangles)] - o.triangles;
}

void lap(std::string_view name) {
    State& s = state();
    if (!g_enabled || s.open == nullptr || !s.stack.empty()) {
        return;
    }
    const auto now = Clock::now();
    Section& section = s.open->sections.emplace_back();
    section.name.assign(name);
    section.cpu_ms = ms_between(s.top_ended, now);
    s.top_ended = now;
}

void gauge(std::string_view name, double value) {
    State& s = state();
    if (!g_enabled || s.open == nullptr) {
        return;
    }
    for (auto& [key, held] : s.open->gauges) {
        if (key == name) {
            held = value;
            return;
        }
    }
    s.open->gauges.emplace_back(std::string(name), value);
}

double Frame::cpu_ms(std::string_view section) const {
    double total = 0.0;
    for (const Section& s : sections) {
        if (s.depth == 0 && s.name == section) {
            total += s.cpu_ms;
        }
    }
    return total;
}

double Frame::gpu_ms() const {
    if (gpu_frame_ms >= 0.0) {
        return gpu_frame_ms;
    }
    double total = 0.0;
    bool any = false;
    for (const Section& s : sections) {
        if (s.gpu_ms >= 0.0) {
            total += s.gpu_ms;
            any = true;
        }
    }
    return any ? total : -1.0;
}

void begin_frame(FrameKind kind, std::uint64_t tick, float alpha) {
    State& s = state();
    if (!g_enabled) {
        g_counters.fill(0);
        if (kind == FrameKind::Tick) {
            s.game_ms = 0.0;
        }
        return;
    }
    const auto now = Clock::now();
    if (s.origin.time_since_epoch().count() == 0) {
        s.origin = now;
    }
    close_open(now);
    if (s.gpu.poll != nullptr) {
        s.gpu.poll();
    }
    flush_dump(false);
    g_counters.fill(0);
    Frame& f = s.frames[s.next_serial % kHistory];
    f.serial = s.next_serial++;
    f.tick = tick;
    f.kind = kind;
    f.alpha = alpha;
    f.start_s = std::chrono::duration<double>(now - s.origin).count();
    f.interval_ms = 0.0;
    f.game_ms = 0.0;
    f.closed = false;
    f.pending_gpu = 0;
    f.gpu_frame_ms = -1.0;
    f.counters.fill(0);
    f.sections.clear();
    f.gauges.clear();
    if (kind == FrameKind::Tick) {
        f.game_ms = s.game_ms;
        s.game_ms = 0.0;
    }
    s.open = &f;
    s.open_started = now;
    s.top_ended = now;
}

void set_frame(std::uint64_t tick, float alpha) {
    if (Frame* f = state().open) {
        f->tick = tick;
        f->alpha = alpha;
    }
}

void game_yielded() {
    State& s = state();
    if (s.game_running) {
        s.game_ms += ms_between(s.game_since, Clock::now());
        s.game_running = false;
    }
}

void game_resumed() {
    State& s = state();
    s.game_since = Clock::now();
    s.game_running = true;
}

const Frame* history(std::size_t back) {
    State& s = state();
    // The open frame is not history: the newest closed one is the one before it.
    const std::uint64_t newest = s.open != nullptr ? s.next_serial - 1 : s.next_serial;
    if (back >= newest || back >= kHistory - 1) {
        return nullptr;
    }
    const Frame* f = frame_of(newest - 1 - back);
    return f != nullptr && f->closed ? f : nullptr;
}

std::size_t history_size() {
    State& s = state();
    const std::uint64_t closed = s.open != nullptr && s.next_serial > 0 ? s.next_serial - 1 : s.next_serial;
    return static_cast<std::size_t>(closed < kHistory - 1 ? closed : kHistory - 1);
}

void set_gpu_hooks(const GpuHooks& hooks) {
    state().gpu = hooks;
}

bool gpu_timers() {
    return state().gpu.begin != nullptr;
}

#if defined(OPENRAC_GPU_MARKERS)
std::string gpu_section_name(std::uint64_t serial, int section) {
    const auto* f = frame_of(serial);
    return f && section >= 0 && std::size_t(section) < f->sections.size()
        ? f->sections[std::size_t(section)].name : std::string{};
}
#endif

void gpu_result(std::uint64_t serial, int section, double ms) {
    Frame* f = frame_of(serial);
    if (f == nullptr || section < 0 || static_cast<std::size_t>(section) >= f->sections.size()) {
        return;
    }
    f->sections[static_cast<std::size_t>(section)].gpu_ms = ms;
    if (f->pending_gpu > 0) {
        f->pending_gpu--;
    }
}

std::uint64_t gpu_frame_begin() {
    State& s = state();
    if (!g_enabled || s.open == nullptr) {
        return kNoFrame;
    }
    s.open->pending_gpu++;
    return s.open->serial;
}

void gpu_frame_result(std::uint64_t serial, double ms) {
    Frame* f = frame_of(serial);
    if (f == nullptr) {
        return;
    }
    f->gpu_frame_ms = ms;
    if (f->pending_gpu > 0) {
        f->pending_gpu--;
    }
}

bool open_dump(const std::string& path, std::string& error) {
    State& s = state();
    close_dump();
    s.dump = std::fopen(path.c_str(), "w");
    if (s.dump == nullptr) {
        error = "cannot write " + path;
        return false;
    }
    s.dump_next = s.next_serial;
    set_enabled(Client::Dump, true);
    static bool registered = false;
    if (!registered) {
        registered = true;
        std::atexit(close_dump);
    }
    return true;
}

void close_dump() {
    State& s = state();
    if (s.dump == nullptr) {
        return;
    }
    close_open(Clock::now());
    flush_frames(s.dump_next, true, [&s](const Frame& f) { write_frame(s.dump, f); });
    std::fclose(s.dump);
    s.dump = nullptr;
    set_enabled(Client::Dump, false);
}

bool dumping() {
    return state().dump != nullptr;
}

void set_reader(FrameReader reader, void* user) {
    State& s = state();
    s.reader = reader;
    s.reader_user = user;
    s.reader_next = s.next_serial;
    set_enabled(Client::Capture, reader != nullptr);
}

namespace {

// Returns whether the probe of `variable` starts on: its value read through env.h by its row's
// Truth. Every probe's variable is a flag read as Truth::Set, on when set to anything, as probe
// read it when it called getenv itself (stats_test pins both).
bool starts_on(const char* variable) {
    const std::optional<env::Var> var = env::find(variable);

    // A probe declared without its row in env.cpp's table, or naming a variable that is not a flag,
    // starts off; said once, since probe keeps one entry per variable.
    if (!var || env::variable(*var).kind != env::Kind::Flag) {
        log::warn("probe {}: not a flag of common/env.cpp's table, off", variable);
        return false;
    }

    return env::flag(*var);
}

}  // namespace

std::vector<Probe*>& probes() {
    static std::vector<Probe*> list;
    return list;
}

const bool& probe(const char* variable, const char* what) {
    const std::lock_guard lock(state().probes_mutex);
    // A variable has one probe: a second declaration is handed the first one's switch.
    for (Probe* p : probes()) {
        // The probe of that variable, by name.
        if (std::strcmp(p->variable, variable) == 0) {
            return p->on;
        }
    }
    // Kept for the life of the program: the callers hold references to `on`.
    Probe* p = new Probe{variable, what, starts_on(variable)};
    probes().push_back(p);
    return p->on;
}

namespace {

std::vector<ShaderFile>& shader_files() {
    static std::vector<ShaderFile> files;
    return files;
}

const char*& shader_root_text() {
    static const char* root = "";
    return root;
}

}  // namespace

int embedded_shader(const char* path, const char* text) {
    shader_files().push_back({path, text});
    return 0;
}

int embedded_shader_root(const char* port_dir) {
    shader_root_text() = port_dir;
    return 0;
}

const std::vector<ShaderFile>& embedded_shaders() {
    return shader_files();
}

const char* shader_root() {
    return shader_root_text();
}

}  // namespace openrac::stats

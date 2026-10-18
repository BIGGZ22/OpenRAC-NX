// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay (debug_overlay.h). Its frame-time window, with its plot, its pause and its
// single step, follows OpenGOAL's (game/graphics/opengl_renderer/debug_gui.cpp, FrameTimeRecorder;
// ISC License, Copyright (c) 2020-2026 OpenGOAL Team), and the per-pass table its profiler's
// (Profiler.cpp): name, time, draw calls, triangles.

#include "debug_overlay.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include "common/log.h"
#include "common/stats.h"
#include "settings.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "renderer/gl.h"
#include "renderer/gl_stats.h"
#include "viewer/screenshot.h"

// The editor's moby class names (cmake/Games.cmake, from editor/moby_classes.tsv).
extern const char* const openrac_moby_class_names;

namespace openrac::frontend::debug_overlay {

namespace {

constexpr std::uint32_t kRamSize = 32u * 1024 * 1024;
constexpr std::uint32_t kMobySize = 0x100;

// The sections of the panel, as OPENRAC_OVERLAY names them.
enum Section : std::size_t { kFrame, kRenderer, kMemory, kMobys, kGame, kShaders, kSectionCount };
constexpr std::array<const char*, kSectionCount> kSectionNames = {"frame", "renderer", "memory",
                                                                 "mobys", "game", "shaders"};

constexpr std::array<const char*, 19> kLevelNames = {
    "Veldin", "Novalis", "Aridia", "Kerwan", "Eudora", "Rilgar", "Blarg Station", "Umbris", "Batalia", "Gaspar",
    "Orxon", "Pokitaru", "Hoven", "Gemlik Base", "Oltanis", "Quartu", "Kalebo III", "Drek's Fleet", "Veldin (final)"};

// The pad's buttons by bit (active low in the report).
constexpr std::array<const char*, 16> kButtonNames = {"select", "L3", "R3", "start", "up", "right", "down", "left",
                                                      "L2", "R2", "L1", "R1", "triangle", "circle", "cross", "square"};

// The main memory map: one cell per 128 KB.
constexpr std::uint32_t kCellBytes = 128 * 1024;
constexpr std::size_t kCells = kRamSize / kCellBytes;

struct RamCell {
    std::uint16_t used_pages = 0;  // of 32 (4 KB each) holding anything but zero
    std::uint64_t sum = 0;
    bool changed = false;
};

// A scripted key or click (OPENRAC_KEYS).
struct ScriptedInput {
    std::uint64_t frame = 0;  // the window frame it happens at
    int held = 0;             // and that many held pictures later, while paused there
    SDL_Keycode key = 0;      // 0: a click
    float x = 0.0f, y = 0.0f;
    bool done = false;
};

struct State {
    std::vector<ScriptedInput> script;
    SDL_Window* window = nullptr;  // as last drawn into
    int held_pictures = 0;         // pictures of the tick held now
    bool wanted = false;      // shown from the start
    bool configured = false;  // the environment was read
    bool visible = false;
    bool initialised = false;  // the ImGui context exists
    std::array<bool, kSectionCount> open_at_start = {true, false, false, false, false, false};

    // The last tick, as the window gave it.
    std::string game;
    std::span<const std::uint8_t> ram;
    int level = -1;
    viewer::GameState state;
    std::uint32_t draws = 0;
    std::uint64_t tick = 0;
    Tick counts;
    std::uint16_t buttons = 0xFFFF;
    std::uint8_t analog[4] = {128, 128, 128, 128};

    // Pause, step, slow motion.
    bool paused = false;
    bool step = false;
    double stretch = 1.0;
    long long pause_at = -1;
    long long reload_at = -1;  // OPENRAC_SHADER_RELOAD: the tick to read the shaders again at
    int held_frames = 0;  // frames to hold before going on by itself (a scripted pause); 0: for good

    // The renderer's passes hidden here, by name; wireframe.
    std::set<std::string> hidden;
    bool wireframe = false;
    bool wireframe_set = false;

    // Mobys.
    std::uint32_t selected = 0;  // address; 0 none
    bool follow_hero = false;    // "inspect": the hero is selected, whichever moby he is now
    bool inspector = false;
    bool labels = false;
    bool near_hero = true;
    float radius = 30.0f;
    bool pick = false;  // a click in the game's picture selects the nearest moby

    // Memory.
    bool memory_viewer = false;
    std::uint32_t view_address = 0x00100000;
    bool view_words = false;
    std::array<RamCell, kCells> cells{};
    std::uint64_t scanned_at = ~0ull;
    std::uint32_t used_pages = 0;

    // Screenshots of the window, with the overlay.
    std::string shot_path;        // write the next frame there
    std::uint64_t shot_every = 0;  // OPENRAC_OVERLAY_SHOT=N:PREFIX
    std::string shot_prefix;
    std::uint64_t shot_from = 0;
    int shots = 0;

    std::string message;  // what the last action said (a reload, a file written)
    std::string notice;   // kept in the picture until cleared (debug_overlay::notice)
    std::string toast;    // the same for an action taken with the panel hidden, shown a moment
    double toast_until = 0.0;
    bool alt_alone = false;  // Alt went down and no other key since: its release toggles the menus
    bool developer = false;  // the launcher's developer tools (OPENRAC_DEVELOPER): the level select
    std::string dump_path = "openrac_frames.jsonl";

    // The processor's load, measured each half second.
    double cpu_seconds = 0.0;
    double cpu_wall = 0.0;
    double cpu_load = 0.0;
};

State s;

template <typename... Args>
void text(std::format_string<Args...> fmt, Args&&... args) {
    ImGui::TextUnformatted(std::format(fmt, std::forward<Args>(args)...).c_str());
}

template <typename... Args>
void text_coloured(const ImVec4& colour, std::format_string<Args...> fmt, Args&&... args) {
    ImGui::PushStyleColor(ImGuiCol_Text, colour);
    ImGui::TextUnformatted(std::format(fmt, std::forward<Args>(args)...).c_str());
    ImGui::PopStyleColor();
}

const ImVec4 kRed{1.0f, 0.35f, 0.3f, 1.0f};
const ImVec4 kGreen{0.45f, 0.9f, 0.45f, 1.0f};
const ImVec4 kDim{0.6f, 0.6f, 0.65f, 1.0f};
const ImVec4 kYellow{1.0f, 0.9f, 0.35f, 1.0f};

// ---- Reading the game's memory ----

bool in_ram(std::uint32_t address, std::uint32_t size) {
    address &= 0x01FFFFFF;
    return static_cast<std::size_t>(address) + size <= s.ram.size();
}

template <typename T>
T read(std::uint32_t address) {
    T value{};
    address &= 0x01FFFFFF;
    if (static_cast<std::size_t>(address) + sizeof(T) <= s.ram.size()) {
        std::memcpy(&value, s.ram.data() + address, sizeof(T));
    }
    return value;
}

const std::unordered_map<int, std::string>& class_names() {
    static const std::unordered_map<int, std::string> names = [] {
        std::unordered_map<int, std::string> map;
        const char* at = openrac_moby_class_names;
        while (at != nullptr && *at != '\0') {
            const char* end = std::strchr(at, '\n');
            const std::string line(at, end != nullptr ? static_cast<std::size_t>(end - at) : std::strlen(at));
            const std::size_t tab = line.find('\t');
            if (!line.empty() && line[0] != '#' && tab != std::string::npos) {
                map[std::atoi(line.c_str())] = line.substr(tab + 1);
            }
            at = end != nullptr ? end + 1 : nullptr;
        }
        return map;
    }();
    return names;
}

std::string class_name(int id) {
    const auto found = class_names().find(id);
    return found != class_names().end() ? found->second : std::string();
}

std::string function_name(std::uint32_t address) {
    if (address == 0) {
        return "none";
    }
    const char* name = openrac_guest_function_name(address);
    return name != nullptr ? std::format("{:#x} {}", address, name) : std::format("{:#x} (no C)", address);
}

const viewer::LiveMoby* hero() {
    for (const viewer::LiveMoby& m : s.state.mobys) {
        if (m.class_id == 0) {
            return &m;
        }
    }
    return nullptr;
}

const viewer::LiveMoby* selected_moby() {
    for (const viewer::LiveMoby& m : s.state.mobys) {
        if (m.address == s.selected) {
            return &m;
        }
    }
    return nullptr;
}

float distance(const viewer::LiveMoby& a, const viewer::LiveMoby& b) {
    const float dx = a.matrix[12] - b.matrix[12], dy = a.matrix[13] - b.matrix[13], dz = a.matrix[14] - b.matrix[14];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

std::string megabytes(std::uint64_t bytes) {
    return std::format("{:.1f} MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
}

// ---- The environment ----

void configure() {
    if (s.configured) {
        return;
    }
    s.configured = true;
    if (const char* developer = std::getenv("OPENRAC_DEVELOPER")) {
        s.developer = developer[0] != '\0' && developer[0] != '0';
    }
    // OPENRAC_OVERLAY: shown from the start; a list of names opens those sections ("all": every
    // one), "inspect" selects the hero and opens the inspector, "labels" names the mobys in view,
    // "memview" opens the memory viewer.
    if (const char* v = std::getenv("OPENRAC_OVERLAY"); v != nullptr && std::strcmp(v, "0") != 0) {
        s.wanted = true;
        const std::string list = v;
        const auto has = [&](const char* word) {
            std::size_t at = 0;
            const std::size_t n = std::strlen(word);
            while ((at = list.find(word, at)) != std::string::npos) {
                const bool starts = at == 0 || list[at - 1] == ',';
                const bool ends = at + n == list.size() || list[at + n] == ',';
                if (starts && ends) {
                    return true;
                }
                at += n;
            }
            return false;
        };
        bool any = false;
        std::array<bool, kSectionCount> open{};
        for (std::size_t i = 0; i < kSectionCount; ++i) {
            open[i] = has(kSectionNames[i]) || has("all");
            any = any || open[i];
        }
        if (any) {
            s.open_at_start = open;
        }
        s.labels = has("labels");
        s.wireframe = has("wireframe");
        s.memory_viewer = has("memview");
        if (has("inspect")) {
            s.inspector = true;
            s.follow_hero = true;
        }
    }
    // OPENRAC_HIDE_PASSES=NAME,...: the renderer's passes of those names are not drawn (the names of
    // the Renderer section's rows: sky, terrain, ties, shrubs, mobys, effects, hud).
    if (const char* v = std::getenv("OPENRAC_HIDE_PASSES")) {
        std::string name;
        for (const char* at = v;; ++at) {
            if (*at == ',' || *at == '\0') {
                if (!name.empty()) {
                    s.hidden.insert(name);
                }
                name.clear();
                if (*at == '\0') {
                    break;
                }
            } else {
                name += *at;
            }
        }
    }
    // OPENRAC_KEYS=FRAME[+HELD]:KEY,...: keys pressed by script at those window frames, as
    // OPENRAC_PRESS presses pad buttons: KEY is SDL's name of a key (F3, F5, Return, ...), or
    // click@X,Y for a left click at that place of the window (in its own units, not pixels).
    // +HELD: that many pictures later while the game is paused at FRAME, where frames do not count.
    if (const char* v = std::getenv("OPENRAC_KEYS")) {
        for (const char* at = v; at != nullptr && *at != '\0';) {
            ScriptedInput in;
            char* end = nullptr;
            in.frame = std::strtoull(at, &end, 10);
            if (*end == '+') {
                in.held = static_cast<int>(std::strtol(end + 1, &end, 10));
            }
            const char* comma = std::strchr(end, ',');
            if (*end == ':') {
                std::string name(end + 1, comma != nullptr ? static_cast<std::size_t>(comma - end - 1) : std::strlen(end + 1));
                if (name.rfind("click@", 0) == 0) {
                    // The click's two numbers are parted by a comma of their own.
                    comma = comma != nullptr ? std::strchr(comma + 1, ',') : nullptr;
                    std::sscanf(end + 1, "click@%f,%f", &in.x, &in.y);
                    s.script.push_back(in);
                } else if ((in.key = SDL_GetKeyFromName(name.c_str())) != 0) {
                    s.script.push_back(in);
                } else {
                    log::warn("OPENRAC_KEYS: no key is named {}", name);
                }
            }
            at = comma != nullptr ? comma + 1 : nullptr;
        }
    }
    // OPENRAC_OVERLAY_SHOT=N:PREFIX: every N ticks the window, overlay included, to PREFIX<tick>.png
    // (from OPENRAC_SHOT_FROM on, as OPENRAC_SHOT's).
    if (const char* v = std::getenv("OPENRAC_OVERLAY_SHOT")) {
        const char* colon = std::strchr(v, ':');
        s.shot_every = std::strtoull(v, nullptr, 10);
        s.shot_prefix = colon != nullptr ? colon + 1 : "overlay_";
        if (const char* from = std::getenv("OPENRAC_SHOT_FROM")) {
            s.shot_from = std::strtoull(from, nullptr, 10);
        }
    }
    // OPENRAC_PAUSE_AT=TICK[:FRAMES]: the game pauses at that tick, and goes on by itself after
    // FRAMES pictures of it when given (for scripted checks of the pause).
    if (const char* v = std::getenv("OPENRAC_PAUSE_AT")) {
        s.pause_at = std::atoll(v);
        const char* colon = std::strchr(v, ':');
        s.held_frames = colon != nullptr ? std::atoi(colon + 1) : 0;
    }
    // OPENRAC_SHADER_RELOAD=TICK: every shader program is read again from its files at that tick
    // (what the Shaders section's button does), with the result in the log.
    if (const char* v = std::getenv("OPENRAC_SHADER_RELOAD")) {
        s.reload_at = std::atoll(v);
    }
    // OPENRAC_TIME_SCALE=X: the game's speed (0.25: quarter speed), from 1/64 to 1.
    if (const char* v = std::getenv("OPENRAC_TIME_SCALE")) {
        const double scale = std::atof(v);
        if (scale > 0.0) {
            s.stretch = std::clamp(1.0 / scale, 1.0, 64.0);
        }
    }
}

void set_visible(bool on) {
    s.visible = on;
    stats::set_enabled(stats::Client::Overlay, on);
}

bool initialise(SDL_Window* window) {
    if (s.initialised) {
        return true;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;  // nothing of the overlay is written beside the game
    io.LogFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 3.0f;
    style.WindowBorderSize = 0.0f;
    style.FramePadding = ImVec2(4.0f, 2.0f);
    style.ItemSpacing = ImVec2(6.0f, 3.0f);
    if (!ImGui_ImplSDL3_InitForOpenGL(window, SDL_GL_GetCurrentContext())
        || !ImGui_ImplOpenGL3_Init(openrac::gl::is_es() ? "#version 300 es" : "#version 410 core")) {
        log::warn("debug overlay: Dear ImGui could not start");
        ImGui::DestroyContext();
        return false;
    }
    s.initialised = true;
    return true;
}

// The scripted keys and clicks due at this picture, put in SDL's queue as the system would.
void run_script(std::uint64_t frame, int held) {
    for (ScriptedInput& in : s.script) {
        if (in.done || in.frame != frame || in.held != held) {
            continue;
        }
        in.done = true;
        const SDL_WindowID window = s.window != nullptr ? SDL_GetWindowID(s.window) : 0;
        SDL_Event e{};
        if (in.key != 0) {
            e.type = SDL_EVENT_KEY_DOWN;
            e.key.windowID = window;
            e.key.key = in.key;
            e.key.scancode = SDL_GetScancodeFromKey(in.key, nullptr);
            e.key.down = true;
            SDL_PushEvent(&e);
            e.type = SDL_EVENT_KEY_UP;
            e.key.down = false;
            SDL_PushEvent(&e);
        } else {
            e.type = SDL_EVENT_MOUSE_MOTION;
            e.motion.windowID = window;
            e.motion.x = in.x;
            e.motion.y = in.y;
            SDL_PushEvent(&e);
            e = SDL_Event{};
            e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            e.button.windowID = window;
            e.button.button = SDL_BUTTON_LEFT;
            e.button.down = true;
            e.button.clicks = 1;
            e.button.x = in.x;
            e.button.y = in.y;
            SDL_PushEvent(&e);
            e.type = SDL_EVENT_MOUSE_BUTTON_UP;
            e.button.down = false;
            SDL_PushEvent(&e);
        }
    }
}

// ---- Averages over the frame history ----

struct Averages {
    int frames = 0, ticks = 0;
    double frame_ms = 0.0, worst_ms = 0.0;
    double fps = 0.0, tps = 0.0;
    double game_ms = 0.0, scene_ms = 0.0, wait_ms = 0.0;                         // per tick
    double prepare_ms = 0.0, render_ms = 0.0, present_ms = 0.0, overlay_ms = 0.0, swap_ms = 0.0;  // per frame
    double gpu_ms = -1.0;
    int gpu_frames = 0;
};

Averages averages() {
    Averages a;
    const stats::Frame* newest = stats::history(0);
    if (newest == nullptr) {
        return a;
    }
    double gpu = 0.0;
    double span = 0.0;
    for (std::size_t back = 0; back < stats::history_size(); ++back) {
        const stats::Frame* f = stats::history(back);
        if (f == nullptr || newest->start_s - f->start_s > 1.0) {
            break;
        }
        a.frames++;
        span += f->interval_ms;
        a.frame_ms += f->interval_ms;
        a.worst_ms = std::max(a.worst_ms, f->interval_ms);
        a.prepare_ms += f->cpu_ms("prepare");
        a.render_ms += f->cpu_ms("render");
        a.present_ms += f->cpu_ms("present");
        a.overlay_ms += f->cpu_ms("overlay");
        a.swap_ms += f->cpu_ms("swap");
        if (f->kind == stats::FrameKind::Tick) {
            a.ticks++;
            a.game_ms += f->game_ms;
            a.scene_ms += f->cpu_ms("scene");
        }
        a.wait_ms += f->cpu_ms("wait");
        if (const double g = f->gpu_ms(); g >= 0.0 && f->pending_gpu == 0) {
            gpu += g;
            a.gpu_frames++;
        }
    }
    if (a.frames > 0 && span > 0.0) {
        a.fps = 1000.0 * a.frames / span;
        a.tps = 1000.0 * a.ticks / span;
        a.frame_ms /= a.frames;
        a.prepare_ms /= a.frames;
        a.render_ms /= a.frames;
        a.present_ms /= a.frames;
        a.overlay_ms /= a.frames;
        a.swap_ms /= a.frames;
    }
    if (a.ticks > 0) {
        a.game_ms /= a.ticks;
        a.scene_ms /= a.ticks;
        a.wait_ms /= a.ticks;
    }
    if (a.gpu_frames > 0) {
        a.gpu_ms = gpu / a.gpu_frames;
    }
    return a;
}

void row(const char* label, const std::string& value) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(value.c_str());
}

// ---- The sections ----

void frame_section(const Shown& shown, const Averages& a) {
    const double tick_ms = shown.tick_period * 1000.0;
    text("{:.1f} ticks/s (the game's {:.0f})   {:.1f} frames/s shown", a.tps,
         shown.tick_period > 0.0 ? 1.0 / shown.tick_period : 0.0, a.fps);
    if (a.worst_ms > 1.5 * std::max(tick_ms, 1.0)) {
        text_coloured(kRed, "frame {:.2f} ms average, {:.2f} ms worst (last second)", a.frame_ms, a.worst_ms);
    } else {
        text("frame {:.2f} ms average, {:.2f} ms worst (last second)", a.frame_ms, a.worst_ms);
    }
    // The frame times, oldest left; and the game's own time of each tick.
    static std::array<float, 240> frame_ms{};
    static std::array<float, 240> game_ms{};
    const std::size_t n = std::min<std::size_t>(frame_ms.size(), stats::history_size());
    std::size_t ticks = 0;
    for (std::size_t i = 0; i < n; ++i) {
        const stats::Frame* f = stats::history(n - 1 - i);
        frame_ms[i] = f != nullptr ? static_cast<float>(f->interval_ms) : 0.0f;
    }
    for (std::size_t back = 0; back < stats::history_size() && ticks < game_ms.size(); ++back) {
        const stats::Frame* f = stats::history(back);
        if (f != nullptr && f->kind == stats::FrameKind::Tick) {
            game_ms[game_ms.size() - 1 - ticks++] = static_cast<float>(f->game_ms);
        }
    }
    const float top = static_cast<float>(std::max(2.0 * tick_ms, 10.0));
    ImGui::PlotLines("##frame", frame_ms.data(), static_cast<int>(n), 0, std::format("frame, 0-{:.0f} ms", top).c_str(),
                     0.0f, top, ImVec2(300.0f, 44.0f));
    // Scaled to its own worst: the game's code takes a small part of a tick on a PC.
    float game_top = 0.5f;
    for (std::size_t i = game_ms.size() - ticks; i < game_ms.size(); ++i) {
        game_top = std::max(game_top, game_ms[i] * 1.25f);
    }
    ImGui::PlotHistogram("##game", game_ms.data() + (game_ms.size() - ticks), static_cast<int>(ticks), 0,
                         std::format("game tick, 0-{:.1f} ms", game_top).c_str(), 0.0f, game_top, ImVec2(300.0f, 34.0f));
    if (ImGui::BeginTable("split", 2, ImGuiTableFlags_SizingFixedFit)) {
        row("game tick (the decompiled C)", std::format("{:6.2f} ms / tick", a.game_ms));
        row("scene read from memory", std::format("{:6.2f} ms / tick", a.scene_ms));
        row("blend between ticks", std::format("{:6.2f} ms / frame", a.prepare_ms));
        row("render submission (CPU)", std::format("{:6.2f} ms / frame", a.render_ms));
        if (a.gpu_ms >= 0.0) {
            row("GPU (timer queries)", std::format("{:6.2f} ms / frame", a.gpu_ms));
        } else {
            row("GPU", renderer::gl_stats::info().timer_queries ? "no result yet" : "not measured: no timer queries");
        }
        row("present + overlay", std::format("{:6.2f} + {:.2f} ms / frame", a.present_ms, a.overlay_ms));
        row("swap and frame pacing", std::format("{:6.2f} ms / frame", a.swap_ms));
        row("wait for the next tick", std::format("{:6.2f} ms / tick", a.wait_ms));
        ImGui::EndTable();
    }
    const stats::Frame* last = stats::history(0);
    text("frames per tick {:.2f}   last: {} alpha {:.3f}", a.ticks > 0 ? static_cast<double>(a.frames) / a.ticks : 0.0,
         last != nullptr ? stats::frame_kind_name(last->kind) : "-", last != nullptr ? static_cast<double>(last->alpha) : 0.0);
    text("between-tick frames {}   vsync {}   refresh {}", shown.interpolating ? "on" : "off (one frame a tick)",
         shown.vsync ? "on" : "off",
         shown.refresh_interval > 0.0
             ? std::format("{:.2f} ms ({:.0f} Hz)", shown.refresh_interval * 1000.0, 1.0 / shown.refresh_interval)
             : std::string("unknown"));
    text("frame cost before swap {:.2f} ms   process CPU {:.0f}% of a core", shown.cost * 1000.0, s.cpu_load * 100.0);
}

void renderer_section(const Shown& shown) {
    const renderer::gl_stats::Info& info = renderer::gl_stats::info();
    text("{}", info.renderer);
    text_coloured(kDim, "{} | OpenGL {} | GLSL {}", info.vendor, info.version, info.glsl);
    if (info.software) {
        text_coloured(kRed, "A SOFTWARE RASTERISER: no GPU is drawing this");
    } else {
        text_coloured(kGreen, "hardware renderer");
    }
    ImGui::SameLine();
    text_coloured(kDim, "| GPU timer queries {}", info.timer_queries ? "yes" : "no");
    const stats::Frame* f = stats::history(0);
    if (f != nullptr) {
        const auto c = [&](stats::Counter counter) { return f->counters[static_cast<std::size_t>(counter)]; };
        text("{} draw calls   {} triangles", c(stats::Counter::DrawCalls), c(stats::Counter::Triangles));
        text("{} texture uploads ({:.1f} KB)   {:.1f} KB of buffers", c(stats::Counter::TextureUploads),
             static_cast<double>(c(stats::Counter::TextureUploadBytes)) / 1024.0,
             static_cast<double>(c(stats::Counter::BufferUploadBytes)) / 1024.0);
        text("{} program changes   {} framebuffer binds", c(stats::Counter::ProgramBinds),
             c(stats::Counter::FramebufferBinds));
    }
    // The passes, averaged over the frames of the last half second whose GPU times are back.
    struct Pass {
        double cpu = 0.0, gpu = 0.0;
        std::uint64_t draws = 0, triangles = 0;
        int frames = 0, gpu_frames = 0;
    };
    std::vector<std::pair<std::string, Pass>> passes;
    const stats::Frame* newest = stats::history(0);
    for (std::size_t back = 0; back < stats::history_size(); ++back) {
        const stats::Frame* h = stats::history(back);
        if (h == nullptr || newest == nullptr || newest->start_s - h->start_s > 0.5) {
            break;
        }
        for (const stats::Section& section : h->sections) {
            if (section.depth != 1) {
                continue;
            }
            auto found = std::find_if(passes.begin(), passes.end(), [&](const auto& p) { return p.first == section.name; });
            if (found == passes.end()) {
                passes.emplace_back(section.name, Pass{});
                found = passes.end() - 1;
            }
            Pass& p = found->second;
            p.cpu += section.cpu_ms;
            p.draws += section.draw_calls;
            p.triangles += section.triangles;
            p.frames++;
            if (section.gpu_ms >= 0.0) {
                p.gpu += section.gpu_ms;
                p.gpu_frames++;
            }
        }
    }
    if (shown.renderer != nullptr
        && ImGui::BeginTable("passes", 6, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("pass");
        ImGui::TableSetupColumn("on");
        ImGui::TableSetupColumn("draws");
        ImGui::TableSetupColumn("triangles");
        ImGui::TableSetupColumn("CPU ms");
        ImGui::TableSetupColumn("GPU ms");
        ImGui::TableHeadersRow();
        for (const auto& r : shown.renderer->renderers()) {
            const std::string& name = r->name();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(name.c_str());
            ImGui::TableNextColumn();
            bool on = !s.hidden.contains(name);
            ImGui::PushID(name.c_str());
            if (ImGui::Checkbox("##on", &on)) {
                if (on) {
                    s.hidden.erase(name);
                } else {
                    s.hidden.insert(name);
                }
            }
            ImGui::PopID();
            const auto found = std::find_if(passes.begin(), passes.end(), [&](const auto& p) { return p.first == name; });
            if (found == passes.end() || found->second.frames == 0) {
                ImGui::TableNextColumn();
                ImGui::TextDisabled("not drawn");
                continue;
            }
            const Pass& p = found->second;
            ImGui::TableNextColumn();
            text("{}", p.draws / static_cast<std::uint64_t>(p.frames));
            ImGui::TableNextColumn();
            text("{}", p.triangles / static_cast<std::uint64_t>(p.frames));
            ImGui::TableNextColumn();
            text("{:.2f}", p.cpu / p.frames);
            ImGui::TableNextColumn();
            if (p.gpu_frames > 0) {
                text("{:.2f}", p.gpu / p.gpu_frames);
            } else {
                ImGui::TextDisabled("-");
            }
        }
        ImGui::EndTable();
    }
    ImGui::Checkbox("wireframe", &s.wireframe);
    text("window {} x {}   picture {} x {} at {},{}", shown.window_width, shown.window_height, shown.box_width,
         shown.box_height, shown.box_x, shown.box_y);
    text("frame buffer {} x {}   last world {} x {}", shown.frame_width, shown.frame_height, shown.last_world_width,
         shown.last_world_height);
    const renderer::gl_stats::Textures textures = renderer::gl_stats::textures();
    text("GL textures {} ({})   texture pool {} entries", textures.count, megabytes(textures.bytes),
         shown.renderer != nullptr ? const_cast<renderer::Renderer*>(shown.renderer)->textures().size() : 0);
    int used = 0;
    const std::vector<renderer::gl_stats::Program> programs = renderer::gl_stats::programs();
    for (const renderer::gl_stats::Program& p : programs) {
        used += p.binds > 0 ? 1 : 0;
    }
    text("shader programs {} ({} used this frame)", programs.size(), used);
    text("2D path {} bytes   native 2D quads {}   effect quads {}   strips {}", s.counts.packet_bytes, s.counts.ui_quads,
         s.counts.effect_quads, s.counts.strips);
    text("layers the game drew: {:#x}", s.draws);
}

void scan_ram() {
    if (s.ram.size() < kRamSize || s.scanned_at == s.tick / 25) {
        return;
    }
    const bool first = s.scanned_at == ~0ull;
    s.scanned_at = s.tick / 25;
    s.used_pages = 0;
    for (std::size_t cell = 0; cell < kCells; ++cell) {
        std::uint16_t used = 0;
        std::uint64_t sum = 0;
        const std::uint8_t* base = s.ram.data() + cell * kCellBytes;
        for (std::uint32_t page = 0; page < kCellBytes / 4096; ++page) {
            std::uint64_t any = 0;
            for (std::uint32_t i = 0; i < 4096; i += 8) {
                std::uint64_t word;
                std::memcpy(&word, base + page * 4096 + i, 8);
                any |= word;
                sum = sum * 31 + word;
            }
            used = static_cast<std::uint16_t>(used + (any != 0 ? 1 : 0));
        }
        s.cells[cell].changed = !first && sum != s.cells[cell].sum;
        s.cells[cell].sum = sum;
        s.cells[cell].used_pages = used;
        s.used_pages += used;
    }
}

void memory_section() {
    const stats::ProcessMemory host = stats::process_memory();
    text("host process: {} resident{}{}", megabytes(host.resident),
         host.footprint != 0 ? std::format(", {} footprint", megabytes(host.footprint)) : std::string(),
         host.peak != 0 ? std::format(", peak {}", megabytes(host.peak)) : std::string());
    const renderer::gl_stats::Textures textures = renderer::gl_stats::textures();
    text("GL textures: {} in {} textures", megabytes(textures.bytes), textures.count);
    scan_ram();
    text("game memory: 32 MB, {} holding data ({} of 8192 pages)",
         megabytes(static_cast<std::uint64_t>(s.used_pages) * 4096), s.used_pages);
    text_coloured(kDim, "the game's own heaps are not read yet: their headers are not identified");
    text("game stack: {:.1f} KB in use of 1024", static_cast<double>(GUEST_STACK_TOP - openrac_guest_sp) / 1024.0);
    // The map: 128 KB a cell, 64 a row (8 MB); brighter with more of it in use, orange where it
    // changed since the last look (twice a second).
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float cw = 5.0f, ch = 9.0f;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    for (std::size_t cell = 0; cell < kCells; ++cell) {
        const float x = origin.x + static_cast<float>(cell % 64) * cw;
        const float y = origin.y + static_cast<float>(cell / 64) * ch;
        const RamCell& c = s.cells[cell];
        const int level = c.used_pages == 0 ? 0 : 70 + c.used_pages * 185 / 32;
        const ImU32 colour = c.used_pages == 0 ? IM_COL32(40, 40, 48, 255)
                             : c.changed ? IM_COL32(level, level * 6 / 10, 30, 255)
                                         : IM_COL32(40, level, 70, 255);
        draw->AddRectFilled(ImVec2(x, y), ImVec2(x + cw - 1.0f, y + ch - 1.0f), colour);
    }
    ImGui::InvisibleButton("ram map", ImVec2(64.0f * cw, static_cast<float>(kCells / 64) * ch));
    if (ImGui::IsItemHovered()) {
        const ImVec2 mouse = ImGui::GetMousePos();
        const std::size_t column = static_cast<std::size_t>(std::clamp((mouse.x - origin.x) / cw, 0.0f, 63.0f));
        const std::size_t line = static_cast<std::size_t>(
            std::clamp((mouse.y - origin.y) / ch, 0.0f, static_cast<float>(kCells / 64 - 1)));
        const std::size_t cell = line * 64 + column;
        ImGui::SetTooltip("%#010x - %#010x: %d of 32 pages in use%s\nclick: open in the memory viewer",
                          static_cast<unsigned>(cell * kCellBytes), static_cast<unsigned>((cell + 1) * kCellBytes - 1),
                          static_cast<int>(s.cells[cell].used_pages), s.cells[cell].changed ? ", changing" : "");
        if (ImGui::IsItemClicked()) {
            s.view_address = static_cast<std::uint32_t>(cell * kCellBytes);
            s.memory_viewer = true;
        }
    }
    text_coloured(kDim, "0 MB at the top left, 8 MB a row; orange: changing");
    ImGui::Checkbox("memory viewer", &s.memory_viewer);
    ImGui::SameLine();
    if (ImGui::Button("write memory to a file")) {
        const std::string path = std::format("ram_{}.bin", s.tick);
        if (std::FILE* f = std::fopen(path.c_str(), "wb")) {
            std::fwrite(s.ram.data(), 1, s.ram.size(), f);
            std::fclose(f);
            s.message = "main memory written to " + path;
            log::info("{}", s.message);
        } else {
            s.message = "cannot write " + path;
        }
    }
}

void memory_viewer_window() {
    if (!s.memory_viewer) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Kept in the bottom right corner, whatever its size becomes.
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 10.0f,
                                   viewport->WorkPos.y + viewport->WorkSize.y - 10.0f),
                            ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.88f);
    if (ImGui::Begin("Game memory", &s.memory_viewer,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        static char input[16] = "100000";
        ImGui::SetNextItemWidth(90.0f);
        if (ImGui::InputText("address", input, sizeof(input),
                             ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            s.view_address = static_cast<std::uint32_t>(std::strtoul(input, nullptr, 16));
        }
        const auto go = [&](std::uint32_t address) {
            s.view_address = address & 0x01FFFFF0;
            std::snprintf(input, sizeof(input), "%X", static_cast<unsigned>(s.view_address));
        };
        ImGui::SameLine();
        if (ImGui::Button("-0x100")) {
            go(s.view_address - 0x100);
        }
        ImGui::SameLine();
        if (ImGui::Button("+0x100")) {
            go(s.view_address + 0x100);
        }
        ImGui::SameLine();
        if (ImGui::Button("follow")) {
            go(read<std::uint32_t>(s.view_address));
        }
        ImGui::SameLine();
        ImGui::Checkbox("words", &s.view_words);
        if (s.selected != 0) {
            ImGui::SameLine();
            if (ImGui::Button("selected moby")) {
                go(s.selected);
            }
        }
        const std::uint32_t base = s.view_address & 0x01FFFFF0;
        for (std::uint32_t line = 0; line < 16; ++line) {
            const std::uint32_t at = base + line * 16;
            if (!in_ram(at, 16)) {
                break;
            }
            std::string out = std::format("{:08X} ", at);
            if (s.view_words) {
                for (std::uint32_t i = 0; i < 16; i += 4) {
                    out += std::format(" {:08X}", read<std::uint32_t>(at + i));
                }
                out += "  ";
                for (std::uint32_t i = 0; i < 16; i += 4) {
                    const float f = read<float>(at + i);
                    out += std::isfinite(f) && std::fabs(f) < 1.0e7f && (std::fabs(f) > 1.0e-4f || f == 0.0f)
                               ? std::format(" {:10.3f}", f) : std::string("          .");
                }
            } else {
                for (std::uint32_t i = 0; i < 16; ++i) {
                    out += std::format(" {:02X}", read<std::uint8_t>(at + i));
                }
                out += "  ";
                for (std::uint32_t i = 0; i < 16; ++i) {
                    const char c = static_cast<char>(read<std::uint8_t>(at + i));
                    out += c >= 0x20 && c < 0x7F ? c : '.';
                }
            }
            ImGui::TextUnformatted(out.c_str());
        }
    }
    ImGui::End();
}

void mobys_section() {
    const viewer::LiveMoby* h = hero();
    text("{} live mobys, {} drawn", s.state.mobys.size(), s.counts.mobys_drawn);
    ImGui::Checkbox("near the hero", &s.near_hero);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.0f);
    ImGui::SliderFloat("radius", &s.radius, 2.0f, 200.0f, "%.0f");
    ImGui::Checkbox("labels in the picture", &s.labels);
    ImGui::SameLine();
    ImGui::Checkbox("click picks", &s.pick);
    struct Row {
        const viewer::LiveMoby* moby;
        float distance;
    };
    std::vector<Row> rows;
    for (const viewer::LiveMoby& m : s.state.mobys) {
        const float d = h != nullptr ? distance(m, *h) : 0.0f;
        if (!s.near_hero || h == nullptr || d <= s.radius) {
            rows.push_back({&m, d});
        }
    }
    std::stable_sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) { return a.distance < b.distance; });
    text("{} listed{}", rows.size(), h == nullptr ? " (no hero: all of them)" : "");
    if (ImGui::BeginTable("mobys", 8,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                          ImVec2(0.0f, 190.0f))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        for (const char* column : {"address", "class", "state", "mode", "anim", "position", "dist", "update"}) {
            ImGui::TableSetupColumn(column);
        }
        ImGui::TableHeadersRow();
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(rows.size()));
        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                const viewer::LiveMoby& m = *rows[static_cast<std::size_t>(i)].moby;
                const std::uint32_t a = m.address;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Selectable(std::format("{:06X}", a).c_str(), s.selected == a,
                                      ImGuiSelectableFlags_SpanAllColumns)) {
                    s.selected = a;
                    s.follow_hero = false;
                    s.inspector = true;
                }
                ImGui::TableNextColumn();
                const std::string name = class_name(m.class_id);
                text("{} {}", m.class_id, name);
                ImGui::TableNextColumn();
                text("{}", read<std::uint8_t>(a + 0x20));
                ImGui::TableNextColumn();
                text("{:04X}", read<std::uint16_t>(a + 0x34));
                ImGui::TableNextColumn();
                text("{}:{}", read<std::uint8_t>(a + 0x52), read<std::uint8_t>(a + 0x50));
                ImGui::TableNextColumn();
                text("{:.1f} {:.1f} {:.1f}", m.matrix[12], m.matrix[13], m.matrix[14]);
                ImGui::TableNextColumn();
                text("{:.1f}", rows[static_cast<std::size_t>(i)].distance);
                ImGui::TableNextColumn();
                const std::uint32_t update = read<std::uint32_t>(a + 0x74);
                const char* fn = update != 0 ? openrac_guest_function_name(update) : nullptr;
                ImGui::TextUnformatted(update == 0 ? "-" : fn != nullptr ? fn : std::format("{:#x}", update).c_str());
            }
        }
        ImGui::EndTable();
    }
}

void inspector_window() {
    if (!s.inspector) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Kept in the top right corner, whatever its size becomes.
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 10.0f, viewport->WorkPos.y + 10.0f),
                            ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.88f);
    if (ImGui::Begin("Moby", &s.inspector,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        const viewer::LiveMoby* m = selected_moby();
        if (m == nullptr) {
            ImGui::TextUnformatted(s.selected == 0 ? "none selected: pick one in the Mobys list"
                                                   : "the selected moby is gone");
        } else {
            const std::uint32_t a = m->address;
            const std::string name = class_name(m->class_id);
            text_coloured(kYellow, "{:#010x}   class {} {}", a, m->class_id, name.empty() ? "(no name)" : name);
            if (ImGui::BeginTable("fields", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
                float sphere[4];
                std::memcpy(sphere, s.ram.data() + a, sizeof(sphere));
                row("+00 collision sphere", std::format("{:.2f} {:.2f} {:.2f} radius {:.2f}", sphere[0], sphere[1],
                                                         sphere[2], sphere[3]));
                row("+10 position", std::format("{:.3f} {:.3f} {:.3f}", read<float>(a + 0x10), read<float>(a + 0x14),
                                                 read<float>(a + 0x18)));
                row("+20 state", std::format("{}", read<std::uint8_t>(a + 0x20)));
                row("+2C scale", std::format("{:.4f}", read<float>(a + 0x2C)));
                row("+30 draw bytes", std::format("{} {} {:#06x}", read<std::uint8_t>(a + 0x30),
                                                   read<std::uint8_t>(a + 0x31), read<std::uint16_t>(a + 0x32)));
                const std::uint16_t mode = read<std::uint16_t>(a + 0x34);
                row("+34 mode bits", std::format("{:#06x}{}{}{}{}", mode, (mode & 1) != 0 ? " hidden" : "",
                                                  (mode & 0x10) != 0 ? " glow" : "", (mode & 0x100) != 0 ? " own-rows" : "",
                                                  (mode & 0x8000) != 0 ? " mirrored" : ""));
                row("+38 light", std::format("set {} / {} blend {}   ambient {} {} {}", read<std::uint8_t>(a + 0x38),
                                              read<std::uint8_t>(a + 0x39), read<std::uint8_t>(a + 0x3A),
                                              read<std::uint8_t>(a + 0x3C), read<std::uint8_t>(a + 0x3D),
                                              read<std::uint8_t>(a + 0x3E)));
                row("+40 rotation", std::format("{:.3f} {:.3f} {:.3f}", read<float>(a + 0x40), read<float>(a + 0x44),
                                                 read<float>(a + 0x48)));
                row("+50 animation", std::format("sequence {} frame {} -> {} frame {}, t {:.3f}",
                                                  read<std::uint8_t>(a + 0x52), read<std::uint8_t>(a + 0x50),
                                                  read<std::uint8_t>(a + 0x53), read<std::uint8_t>(a + 0x51),
                                                  read<float>(a + 0x54)));
                row("+74 update", function_name(read<std::uint32_t>(a + 0x74)));
                row("+78 pvars", std::format("{:#x}", read<std::uint32_t>(a + 0x78)));
                row("+90 glow colour", std::format("{:#08x}", read<std::uint32_t>(a + 0x90) & 0xFFFFFFu));
                row("+A6 class", std::format("{}", read<std::int16_t>(a + 0xA6)));
                if (const viewer::LiveMoby* h = hero()) {
                    row("distance to the hero", std::format("{:.2f}", distance(*m, *h)));
                }
                ImGui::EndTable();
            }
            if (ImGui::Button("in the memory viewer")) {
                s.view_address = a;
                s.memory_viewer = true;
            }
            const std::uint32_t pvars = read<std::uint32_t>(a + 0x78);
            if (pvars != 0 && in_ram(pvars, 0x40)) {
                ImGui::SameLine();
                if (ImGui::Button("its pvars")) {
                    s.view_address = pvars & 0x01FFFFF0;
                    s.memory_viewer = true;
                    s.view_words = true;
                }
            }
            if (ImGui::TreeNode("all 0x100 bytes, as words and floats")) {
                for (std::uint32_t at = 0; at < kMobySize; at += 16) {
                    std::string out = std::format("+{:02X} ", at);
                    for (std::uint32_t i = 0; i < 16; i += 4) {
                        out += std::format(" {:08X}", read<std::uint32_t>(a + at + i));
                    }
                    out += " ";
                    for (std::uint32_t i = 0; i < 16; i += 4) {
                        const float f = read<float>(a + at + i);
                        out += std::isfinite(f) && std::fabs(f) < 1.0e6f && (std::fabs(f) > 1.0e-4f || f == 0.0f)
                                   ? std::format(" {:9.3f}", f) : std::string("         .");
                    }
                    ImGui::TextUnformatted(out.c_str());
                }
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

void game_section() {
    const std::string level = s.level >= 0 && s.level < 19 ? std::format("{} {}", s.level, kLevelNames[static_cast<std::size_t>(s.level)])
                                                            : std::string("none (the boot program: title, menus)");
    text("{}   level {}", s.game, level);
    text("tick {}   game mode {}", s.tick, read<std::int32_t>(0x0015F6E8));
    if (const viewer::LiveMoby* h = hero()) {
        text("hero {:.2f} {:.2f} {:.2f}   state {}", h->matrix[12], h->matrix[13], h->matrix[14],
             read<std::uint8_t>(h->address + 0x20));
    } else {
        ImGui::TextDisabled("no hero moby");
    }
    const viewer::GameState& g = s.state;
    text("camera {:.2f} {:.2f} {:.2f}", g.camera_position[0], g.camera_position[1], g.camera_position[2]);
    text("  forward {:.3f} {:.3f} {:.3f}   tan fov {:.3f} x {:.3f}", g.forward[0], g.forward[1], g.forward[2],
         g.tan_half_fov_x, g.tan_half_fov_y);
    text("  fog {:.0f} {:.0f} {:.0f} from {:.0f} to {:.0f}", g.fog_colour[0] * 255.0f, g.fog_colour[1] * 255.0f,
         g.fog_colour[2] * 255.0f, g.fog_near, g.fog_far);
    std::string held;
    for (std::size_t bit = 0; bit < kButtonNames.size(); ++bit) {
        if ((s.buttons & (1u << bit)) == 0) {
            held += (held.empty() ? "" : " ") + std::string(kButtonNames[bit]);
        }
    }
    text("pad {:#06x} {}", s.buttons, held.empty() ? "(nothing held)" : held);
    text("  left stick {} {}   right stick {} {}", s.analog[2], s.analog[3], s.analog[0], s.analog[1]);
    unsigned functions = 0;
    const unsigned long calls = openrac_guest_missing_calls(&functions);
    if (functions == 0) {
        text_coloured(kGreen, "no function without C was called");
    } else {
        text_coloured(kRed, "{} calls to {} functions without C", calls, functions);
        if (ImGui::TreeNode("the functions")) {
            unsigned long count = 0;
            for (unsigned i = 0; i < 64; ++i) {
                const char* name = openrac_guest_missing_at(i, &count);
                if (name == nullptr) {
                    break;
                }
                text("{:8} {}", count, name);
            }
            ImGui::TreePop();
        }
    }
    text_coloured(kDim, "a fault in the game's code ends the program: its report is in the log");
    if (ImGui::TreeNode("probes (the OPENRAC_* switches)")) {
        for (stats::Probe* p : stats::probes()) {
            ImGui::Checkbox(p->variable, &p->on);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", p->what);
            }
        }
        if (stats::probes().empty()) {
            ImGui::TextDisabled("none read yet");
        }
        ImGui::TreePop();
    }
}

std::string shader_root() {
    const char* named = std::getenv("OPENRAC_SHADER_DIR");
    return named != nullptr ? named : stats::shader_root();
}

void shaders_section() {
    const std::string root = shader_root();
    const std::vector<renderer::gl_stats::Program> programs = renderer::gl_stats::programs();
    if (ImGui::Button("reload all from disk")) {
        std::string message;
        const int done = renderer::gl_stats::reload_all(root, message);
        s.message = std::format("{} programs reloaded{}", done, message.empty() ? "" : "\n" + message);
    }
    ImGui::SameLine();
    text_coloured(kDim, "{}", root.empty() ? "no source folder" : root);
    if (ImGui::BeginTable("programs", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        for (const char* column : {"id", "program", "status", "binds", ""}) {
            ImGui::TableSetupColumn(column);
        }
        ImGui::TableHeadersRow();
        for (const renderer::gl_stats::Program& p : programs) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            text("{}", p.id);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(p.name.c_str());
            if (ImGui::IsItemHovered() && !p.vertex_file.empty()) {
                ImGui::SetTooltip("%s\n%s", p.vertex_file.c_str(), p.fragment_file.c_str());
            }
            ImGui::TableNextColumn();
            if (p.linked) {
                text_coloured(kGreen, "linked{}", p.reload_status.empty() ? "" : ", " + p.reload_status);
            } else {
                text_coloured(kRed, "NOT linked");
            }
            ImGui::TableNextColumn();
            text("{} ({} in all)", p.binds, p.binds_total);
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(p.id));
            if (!p.vertex_file.empty() && !p.fragment_file.empty() && ImGui::SmallButton("reload")) {
                renderer::gl_stats::reload(p.id, root, s.message);
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    const std::vector<std::string>& failures = renderer::gl_stats::failures();
    if (failures.empty()) {
        text_coloured(kGreen, "every shader compiled and every program linked");
    } else {
        text_coloured(kRed, "{} failures:", failures.size());
        for (const std::string& f : failures) {
            ImGui::TextWrapped("%s", f.c_str());
        }
    }
}

// ---- The picture: the selected moby's marker, the labels, picking ----

struct Projector {
    renderer::Mat4 clip_from_world;
    float x, y, width, height;  // the picture, in ImGui's coordinates (top left origin)

    bool project(float wx, float wy, float wz, ImVec2& out) const {
        const renderer::Mat4& m = clip_from_world;
        const float cx = m[0] * wx + m[4] * wy + m[8] * wz + m[12];
        const float cy = m[1] * wx + m[5] * wy + m[9] * wz + m[13];
        const float cw = m[3] * wx + m[7] * wy + m[11] * wz + m[15];
        if (cw <= 0.001f) {
            return false;
        }
        const float nx = cx / cw, ny = cy / cw;
        if (nx < -1.0f || nx > 1.0f || ny < -1.0f || ny > 1.0f) {
            return false;
        }
        out = ImVec2(x + (nx * 0.5f + 0.5f) * width, y + (1.0f - (ny * 0.5f + 0.5f)) * height);
        return true;
    }
};

void picture_marks(const Shown& shown) {
    if (shown.camera == nullptr || shown.box_width <= 0 || (!s.labels && !s.pick && s.selected == 0)) {
        return;
    }
    const ImGuiIO& io = ImGui::GetIO();
    const float scale = io.DisplayFramebufferScale.x > 0.0f ? io.DisplayFramebufferScale.x : 1.0f;
    Projector p;
    p.clip_from_world = renderer::multiply(shown.camera->projection, shown.camera->view);
    p.x = static_cast<float>(shown.box_x) / scale;
    p.width = static_cast<float>(shown.box_width) / scale;
    p.height = static_cast<float>(shown.box_height) / scale;
    // The box counts rows from the window's bottom, as OpenGL does; it is centred, so the same.
    p.y = static_cast<float>(shown.window_height - shown.box_y - shown.box_height) / scale;
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    const viewer::LiveMoby* h = hero();
    const bool clicked = s.pick && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.WantCaptureMouse;
    float best = 40.0f * 40.0f;
    std::uint32_t picked = 0;
    for (const viewer::LiveMoby& m : s.state.mobys) {
        ImVec2 at;
        if (!p.project(m.matrix[12], m.matrix[13], m.matrix[14], at)) {
            continue;
        }
        if (clicked) {
            const float dx = at.x - io.MousePos.x, dy = at.y - io.MousePos.y;
            if (dx * dx + dy * dy < best) {
                best = dx * dx + dy * dy;
                picked = m.address;
            }
        }
        const bool selected = m.address == s.selected;
        if (selected) {
            draw->AddCircle(at, 14.0f, IM_COL32(255, 230, 80, 255), 24, 2.0f);
        }
        if (selected || (s.labels && (h == nullptr || distance(m, *h) <= s.radius))) {
            const std::string name = class_name(m.class_id);
            const std::string label = name.empty() ? std::format("{}", m.class_id) : std::format("{} {}", m.class_id, name);
            draw->AddCircleFilled(at, 2.5f, IM_COL32(255, 255, 255, 220));
            draw->AddText(ImVec2(at.x + 7.0f, at.y - 5.0f), IM_COL32(0, 0, 0, 255), label.c_str());
            draw->AddText(ImVec2(at.x + 6.0f, at.y - 6.0f), selected ? IM_COL32(255, 230, 80, 255) : IM_COL32(255, 255, 255, 255),
                          label.c_str());
        }
    }
    if (picked != 0) {
        s.follow_hero = false;
        s.selected = picked;
        s.inspector = true;
    }
}

// ---- The panel ----

void panel(const Shown& shown) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 10.0f, viewport->WorkPos.y + 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(viewport->WorkSize.x - 20.0f, viewport->WorkSize.y - 20.0f));
    ImGui::SetNextWindowBgAlpha(0.82f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
                                   | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
                                   | ImGuiWindowFlags_NoNav;
    if (ImGui::Begin("OpenRAC debug", nullptr, flags)) {
        const Averages a = averages();
        const std::string where = s.level >= 0 && s.level < 19 ? kLevelNames[static_cast<std::size_t>(s.level)] : "boot";
        text_coloured(kYellow, "OpenRAC {}  {}  tick {}  {:.0f} tps  {:.0f} fps", s.game, where, s.tick, a.tps, a.fps);
        if (ImGui::Button(s.paused ? "resume (F5)" : "pause (F5)")) {
            s.paused = !s.paused;
            s.held_frames = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("step (F6)")) {
            s.paused = true;
            s.step = true;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(70.0f);
        static const char* const kSpeeds[] = {"1x", "1/2", "1/4", "1/8", "1/16"};
        int speed = std::clamp(static_cast<int>(std::lround(std::log2(s.stretch))), 0, 4);
        if (ImGui::Combo("speed", &speed, kSpeeds, 5)) {
            s.stretch = static_cast<double>(1 << speed);
        }
        if (s.paused) {
            ImGui::SameLine();
            text_coloured(kRed, "PAUSED");
        }
        if (ImGui::Button("screenshot (F12)")) {
            s.shot_path = std::format("openrac_window_{}.png", s.tick);
        }
        ImGui::SameLine();
        bool dumping = stats::dumping();
        if (ImGui::Checkbox("dump frames", &dumping)) {
            if (dumping) {
                std::string error;
                s.message = stats::open_dump(s.dump_path, error) ? "frames are written to " + s.dump_path : error;
            } else {
                stats::close_dump();
                s.message = "frame dump closed";
            }
        }
        if (!s.message.empty()) {
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 420.0f);
            text_coloured(kDim, "{}", s.message);
            ImGui::PopTextWrapPos();
        }
        const auto header = [](Section section, const char* label) {
            ImGui::SetNextItemOpen(s.open_at_start[section], ImGuiCond_Once);
            return ImGui::CollapsingHeader(label);
        };
        if (header(kFrame, "Frame")) {
            frame_section(shown, a);
        }
        if (header(kRenderer, "Renderer")) {
            renderer_section(shown);
        }
        if (header(kMemory, "Memory")) {
            memory_section();
        }
        if (header(kMobys, "Mobys")) {
            mobys_section();
        }
        if (header(kGame, "Game")) {
            game_section();
        }
        if (header(kShaders, "Shaders")) {
            shaders_section();
        }
        text_coloured(kDim, "F3 hide  F5 pause  F6 step  F7/F8 slower/faster  F12 screenshot");
    }
    ImGui::End();
}

// The window as it is now, overlay included, to a PNG (the back buffer, before the swap).
void write_window(const std::string& path, int width, int height) {
    using namespace openrac::gl;
    if (width <= 0 || height <= 0) {
        return;
    }
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    // OpenGL's first row is the bottom one.
    std::vector<std::uint8_t> flipped(pixels.size());
    const std::size_t stride = static_cast<std::size_t>(width) * 4;
    for (int y = 0; y < height; ++y) {
        std::memcpy(flipped.data() + static_cast<std::size_t>(y) * stride,
                    pixels.data() + static_cast<std::size_t>(height - 1 - y) * stride, stride);
    }
    for (std::size_t i = 3; i < flipped.size(); i += 4) {
        flipped[i] = 0xFF;
    }
    if (viewer::write_png(path, width, height, flipped)) {
        s.message = "window written to " + path;
        log::info("{}", s.message);
    } else {
        s.message = "cannot write " + path;
    }
}

// The menus along the window's top, as OpenGOAL has them: on Windows and Linux. macOS has its own
// menu bar, with the same menus (platform/macos_app.mm).
#if defined(__APPLE__)
constexpr bool kMenuBar = false;
#else
constexpr bool kMenuBar = true;
#endif

// OPENRAC_MENU_BAR=1 has them on macOS as well (for looking at them there).
bool menu_bar_possible() {
    static const bool forced = std::getenv("OPENRAC_MENU_BAR") != nullptr;
    return kMenuBar || forced;
}

bool menu_bar_shown() {
    return menu_bar_possible() && settings::get().menu_bar;
}

// A menu item presses the key the port already answers, as the macOS menus do.
void press(SDL_Keycode key) {
    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = key;
    event.key.down = true;
    SDL_PushEvent(&event);
    event.type = SDL_EVENT_KEY_UP;
    event.key.down = false;
    SDL_PushEvent(&event);
}

void menu_bar() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Full screen", "F11")) press(SDLK_F11);
        if (ImGui::MenuItem("Frame counter", "F2", settings::get().show_fps)) press(SDLK_F2);
        ImGui::Separator();
        if (ImGui::MenuItem("Hide these menus", "Alt")) {
            settings::get().menu_bar = false;
            settings::save();
            s.toast = "Alt shows the menus again";
            s.toast_until = ImGui::GetTime() + 4.0;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Sound")) {
        if (ImGui::MenuItem("Volume up", "F10")) press(SDLK_F10);
        if (ImGui::MenuItem("Volume down", "F9")) press(SDLK_F9);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Developer")) {
        if (ImGui::MenuItem("Debug overlay", "F3", s.visible)) press(SDLK_F3);
        ImGui::Separator();
        if (ImGui::MenuItem("Pause", "F5", s.paused)) press(SDLK_F5);
        if (ImGui::MenuItem("Step one tick", "F6")) press(SDLK_F6);
        if (ImGui::MenuItem("Slower", "F7", false, s.stretch < 16.0)) press(SDLK_F7);
        if (ImGui::MenuItem("Faster", "F8", false, s.stretch > 1.0)) press(SDLK_F8);
        ImGui::Separator();
        if (ImGui::MenuItem("Screenshot", "F12")) press(SDLK_F12);
        if (s.developer) {
            ImGui::Separator();
            if (ImGui::MenuItem("Level select", "F1")) press(SDLK_F1);
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

// What the keys of pause and speed have set, said in the picture whether the panel is shown or
// not: a game standing still or running at a quarter speed with nothing saying why is a puzzle.
void status_badge() {
    if (!s.toast.empty() && ImGui::GetTime() >= s.toast_until) {
        s.toast.clear();
    }
    std::string line;
    if (s.paused) {
        line = "PAUSED    F5 resume    F6 one tick";
    } else if (s.stretch != 1.0) {
        line = std::format("SPEED 1/{}    F8 faster    F7 slower", static_cast<int>(s.stretch));
    }
    if (line.empty() && s.toast.empty() && s.notice.empty()) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f, viewport->WorkPos.y + 10.0f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.82f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
                                   | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
                                   | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;
    if (ImGui::Begin("OpenRAC status", nullptr, flags)) {
        if (!line.empty()) {
            text_coloured(s.paused ? kRed : kYellow, "{}", line);
        }
        if (!s.toast.empty()) {
            text_coloured(kDim, "{}", s.toast);
        }
        if (!s.notice.empty()) {
            text_coloured(kRed, "{}", s.notice);
        }
    }
    ImGui::End();
}

}  // namespace

void request() {
    configure();
    s.wanted = true;
}

bool visible() {
    return s.visible;
}

bool event(const SDL_Event& e) {
    configure();
    if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
        if (e.key.key == SDLK_F3) {
            set_visible(!s.visible);
            s.wanted = false;
            return true;
        }
        // Pause, step, speed and the screenshot are their own: they work with the panel hidden,
        // and say so in the picture (status_badge).
        switch (e.key.key) {
            case SDLK_F5:
                s.paused = !s.paused;
                s.held_frames = 0;
                return true;
            case SDLK_F6:
                s.paused = true;
                s.step = true;
                return true;
            case SDLK_F7:
                s.stretch = std::min(s.stretch * 2.0, 16.0);
                return true;
            case SDLK_F8:
                s.stretch = std::max(s.stretch / 2.0, 1.0);
                return true;
            case SDLK_F12:
                s.shot_path = std::format("openrac_window_{}.png", s.tick);
                return true;
            default:
                break;
        }
    }
    if (menu_bar_possible()) {
        // Alt by itself hides and shows the menus (Alt+Return is fullscreen: another key in between
        // makes it not by itself).
        const bool alt = e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP
                             ? e.key.key == SDLK_LALT || e.key.key == SDLK_RALT
                             : false;
        if (e.type == SDL_EVENT_KEY_DOWN) {
            s.alt_alone = alt;
        } else if (e.type == SDL_EVENT_KEY_UP && alt && s.alt_alone) {
            s.alt_alone = false;
            settings::get().menu_bar = !settings::get().menu_bar;
            settings::save();
        }
    }
    if (!(s.visible || menu_bar_shown()) || !s.initialised) {
        return false;
    }
    ImGui_ImplSDL3_ProcessEvent(&e);
    const ImGuiIO& io = ImGui::GetIO();
    switch (e.type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL:
        case SDL_EVENT_MOUSE_MOTION:
            return io.WantCaptureMouse;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_TEXT_INPUT:
            return io.WantTextInput;  // typing in one of the overlay's fields
        default:
            return false;
    }
}

void tick(const Tick& t) {
    configure();
    if (s.wanted && !s.visible) {
        s.wanted = false;
        set_visible(true);
    }
    if (s.pause_at >= 0 && t.tick == static_cast<std::uint64_t>(s.pause_at)) {
        s.paused = true;
        s.pause_at = -1;
        log::info("debug overlay: paused at tick {}", t.tick);
    }
    if (s.reload_at >= 0 && t.tick == static_cast<std::uint64_t>(s.reload_at)) {
        s.reload_at = -1;
        std::string message;
        const int done = renderer::gl_stats::reload_all(shader_root(), message);
        s.message = std::format("{} programs reloaded{}", done, message.empty() ? "" : "\n" + message);
        log::info("debug overlay: {}", s.message);
    }
    stats::gauge("mobys", t.state != nullptr ? static_cast<double>(t.state->mobys.size()) : 0.0);
    stats::gauge("mobys_drawn", static_cast<double>(t.mobys_drawn));
    stats::gauge("ui_quads", static_cast<double>(t.ui_quads));
    stats::gauge("effect_quads", static_cast<double>(t.effect_quads));
    stats::gauge("packet_bytes", static_cast<double>(t.packet_bytes));
    stats::gauge("level", static_cast<double>(t.level));
    s.tick = t.tick;  // a screenshot is named after it, with the panel shown or not
    if (!s.visible) {
        return;
    }
    s.game = t.game;
    s.ram = t.ram;
    s.level = t.level;
    if (t.state != nullptr) {
        s.state = *t.state;
    }
    s.draws = t.draws;
    s.tick = t.tick;
    s.counts = t;
    if (s.follow_hero) {
        if (const viewer::LiveMoby* h = hero()) {
            s.selected = h->address;
        }
    }
}

void begin(bool first, bool held, float alpha, std::uint64_t tick_number) {
    if (!s.script.empty() && (first || held)) {
        s.held_pictures = held ? s.held_pictures + 1 : 0;
        run_script(tick_number, s.held_pictures);
    }
    if (first) {
        // The tick's frame was opened where the tick ended (main.cpp); what came since read the
        // scene from the game's memory.
        stats::set_frame(tick_number, alpha);
        stats::lap("scene");
    } else {
        stats::begin_frame(held ? stats::FrameKind::Held : stats::FrameKind::Between, tick_number, alpha);
    }
}

void before_render(renderer::Renderer* renderer) {
    using namespace openrac::gl;
    if (renderer != nullptr && !s.hidden.empty()) {
        for (const auto& r : renderer->renderers()) {
            if (s.hidden.contains(r->name())) {
                r->enabled = false;
            }
        }
    }
    if (s.wireframe && s.visible) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        s.wireframe_set = true;
    }
    stats::lap("prepare");
}

void draw(const Shown& shown) {
    using namespace openrac::gl;
    if (s.wireframe_set) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        s.wireframe_set = false;
    }
    stats::lap("present");
    s.window = shown.window;
    const bool badge = s.paused || s.stretch != 1.0 || !s.toast.empty() || !s.notice.empty();
    if (!(s.visible || menu_bar_shown() || badge || !s.shot_path.empty()) || shown.window == nullptr
        || !initialise(shown.window)) {
        return;
    }
    const stats::Scope scope("overlay");
    // The processor's load, each half second.
    if (const stats::Frame* f = stats::history(0); s.visible && f != nullptr && f->start_s - s.cpu_wall >= 0.5) {
        const double seconds = stats::process_cpu_seconds();
        s.cpu_load = s.cpu_wall > 0.0 ? (seconds - s.cpu_seconds) / (f->start_s - s.cpu_wall) : 0.0;
        s.cpu_seconds = seconds;
        s.cpu_wall = f->start_s;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, shown.window_width, shown.window_height);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    if (menu_bar_shown()) {
        menu_bar();
    }
    if (s.visible) {
        panel(shown);
        inspector_window();
        memory_viewer_window();
        picture_marks(shown);
    }
    status_badge();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    renderer::gl_stats::reset_binds();

    const stats::Frame* last = stats::history(0);
    const bool tick_frame = last == nullptr || last->kind != stats::FrameKind::Between;
    if (s.shot_every > 0 && tick_frame && s.tick % s.shot_every == 0 && s.tick >= s.shot_from && s.shots < 10000) {
        // Once a tick; and once more while paused, a few pictures in ("_held").
        static std::uint64_t written = ~0ull;
        static int held = 0;
        held = s.paused ? held + 1 : 0;
        if (written != s.tick || held == 5) {
            written = s.tick;
            write_window(std::format("{}{}{}.png", s.shot_prefix, s.tick, held == 5 ? "_held" : ""), shown.window_width,
                         shown.window_height);
            s.shots++;
        }
    }
    if (!s.shot_path.empty()) {
        write_window(s.shot_path, shown.window_width, shown.window_height);
        s.toast = "saved " + s.shot_path;
        s.toast_until = ImGui::GetTime() + 2.5;
        s.shot_path.clear();
    }
}

void pad(std::uint16_t buttons, const std::uint8_t analog[4]) {
    s.buttons = buttons;
    std::memcpy(s.analog, analog, 4);
}

bool holding() {
    if (!s.paused) {
        return false;
    }
    if (s.step) {
        s.step = false;
        return false;
    }
    if (s.held_frames > 0 && --s.held_frames == 0) {
        s.paused = false;
        log::info("debug overlay: resumed");
        return false;
    }
    return true;
}

double tick_stretch() {
    configure();
    return s.stretch;
}

void notice(const std::string& text) {
    s.notice = text;
}

void release() {
    if (s.initialised) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        s.initialised = false;
    }
}

}  // namespace openrac::frontend::debug_overlay

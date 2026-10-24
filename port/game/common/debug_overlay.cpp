// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay (debug_overlay.h): what the environment asks of it, its initialisation, the
// scripted keys, the panel, the window's screenshots and the entry points, which draw the rest from
// debug/ (debug/overlay.h). Its frame-time window, with its plot, its pause and its single step,
// follows OpenGOAL's (game/graphics/opengl_renderer/debug_gui.cpp, FrameTimeRecorder; ISC License,
// Copyright (c) 2020-2026 OpenGOAL Team), and the per-pass table its profiler's (Profiler.cpp):
// name, time, draw calls, triangles; the panel's sections are in debug/frame.cpp and
// debug/renderer.cpp.

#include "debug_overlay.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "common/env.h"
#include "common/log.h"
#include "common/stats.h"
#include "debug/overlay.h"
#include "frontend.h"
#include "imgui_gl.h"
#include "renderer/gl.h"
#include "renderer/gl_stats.h"
#include "scene/screenshot.h"
#include "settings.h"

namespace openrac::frontend::debug_overlay {

namespace {

// ---- The environment ----

// Returns the value `var` was set to as a string of its own, or nothing when it is unset. For the
// reads below that walk the value as the C library does, up to its NUL, which env::value's view
// need not have.
std::optional<std::string> env_string(env::Var var) {
    const std::optional<std::string_view> found = env::value(var);

    // An unset variable stays nothing, which differs from the empty string.
    if (!found) {
        return std::nullopt;
    }

    return std::string(*found);
}

void configure() {
    if (s.configured) {
        return;
    }
    s.configured = true;
    s.developer = env::flag(env::Var::Developer);
    // OPENRAC_OVERLAY: shown from the start; a list of names opens those sections ("all": every
    // one), "inspect" selects the hero and opens the inspector, "labels" names the mobys in view,
    // "memview" opens the memory viewer.
    if (const std::optional<std::string_view> v = env::value(env::Var::Overlay); v && *v != "0") {
        s.wanted = true;
        const std::string list(*v);
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
    if (const std::optional<std::string> v = env_string(env::Var::HidePasses)) {
        std::string name;
        for (const char* at = v->c_str();; ++at) {
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
    if (const std::optional<std::string> v = env_string(env::Var::Keys)) {
        for (const char* at = v->c_str(); at != nullptr && *at != '\0';) {
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
    if (const std::optional<std::string> v = env_string(env::Var::OverlayShot)) {
        const char* colon = std::strchr(v->c_str(), ':');
        s.shot_every = std::strtoull(v->c_str(), nullptr, 10);
        s.shot_prefix = colon != nullptr ? colon + 1 : "overlay_";

        // Read with strtoull, as OPENRAC_SHOT's is (frontend.cpp), not env::integer's strtoll.
        if (const std::optional<std::string> from = env_string(env::Var::ShotFrom)) {
            s.shot_from = std::strtoull(from->c_str(), nullptr, 10);
        }
    }
    // OPENRAC_PAUSE_AT=TICK[:FRAMES]: the game pauses at that tick, and goes on by itself after
    // FRAMES pictures of it when given (for scripted checks of the pause).
    if (const std::optional<std::string> v = env_string(env::Var::PauseAt)) {
        s.pause_at = std::atoll(v->c_str());
        const char* colon = std::strchr(v->c_str(), ':');
        s.held_frames = colon != nullptr ? std::atoi(colon + 1) : 0;
    }
    // OPENRAC_SHADER_RELOAD=TICK: every shader program is read again from its files at that tick
    // (what the Shaders section's button does), with the result in the log.
    s.reload_at = env::integer(env::Var::ShaderReload, s.reload_at);
    // OPENRAC_TIME_SCALE=X: the game's speed (0.25: quarter speed), from 1/64 to 1.
    {
        // Unset reads as 0, which, as any value not above 0, leaves the speed as it is.
        const double scale = env::number(env::Var::TimeScale, 0.0);
        if (scale > 0.0) {
            s.stretch = std::clamp(1.0 / scale, 1.0, 64.0);
        }
    }
}

// On a phone: everything as large as the screen's density asks, in the system's own typeface
// (ImGui's built-in one is a bitmap for a desktop's pixels).
void initialise_touch(SDL_Window* window) {
#if defined(__ANDROID__)
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    const float density = SDL_GetWindowPixelDensity(window);
    const float display = SDL_GetWindowDisplayScale(window);
    s.ui_scale = density > 0.0f && display > 0.0f ? std::max(display / density, 1.0f) : 1.0f;
    // The panel's buttons, boxes and headers roomier for a finger, and reached a little beyond.
    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.TouchExtraPadding = ImVec2(4.0f, 4.0f);
    style.ScrollbarSize = 18.0f;
    style.GrabMinSize = 18.0f;
    style.ScaleAllSizes(s.ui_scale);
    bool font = false;
    for (const char* path : {"/system/fonts/Roboto-Regular.ttf", "/system/fonts/DroidSans.ttf"}) {
        if (std::FILE* f = std::fopen(path, "rb")) {
            std::fclose(f);
            font = io.Fonts->AddFontFromFileTTF(path, std::round(15.0f * s.ui_scale)) != nullptr;
            break;
        }
    }
    if (!font) {
        ImFontConfig config;
        config.SizePixels = std::round(13.0f * s.ui_scale);
        io.Fonts->AddFontDefault(&config);
    }
#else
    (void)window;
#endif
}

bool initialise(SDL_Window* window) {
    if (s.initialised) {
        return true;
    }
    // ImGui's OpenGL backend loads the system's GL; a window drawn with Vulkan has none, and has the
    // renderer's GL instead (imgui_gl.h).
    s.gl_subset = SDL_GL_GetCurrentContext() == nullptr;
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
    initialise_touch(window);
    std::string error;
    const bool started = s.gl_subset
                             ? ImGui_ImplSDL3_InitForOther(window) && imgui_gl::init(error)
                             : ImGui_ImplSDL3_InitForOpenGL(window, SDL_GL_GetCurrentContext())
                                   && ImGui_ImplOpenGL3_Init(openrac::gl::is_es() ? "#version 300 es" : "#version 410 core");
    if (!started) {
        log::warn("debug overlay: Dear ImGui could not start{}", error.empty() ? "" : ": " + error);
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

// ---- The panel ----

void panel(const Shown& shown) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 10.0f, viewport->WorkPos.y + 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(viewport->WorkSize.x - 20.0f, viewport->WorkSize.y - 20.0f));
    ImGui::SetNextWindowBgAlpha(0.82f);
    // On a touch screen it has no keys to name, and a gamepad moves through it while the game is
    // paused (draw); a finger scrolls it.
    const bool keys = !drawer_possible();
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
                                   | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
                                   | (keys ? ImGuiWindowFlags_NoNav : ImGuiWindowFlags_None);
    if (ImGui::Begin("OpenRAC debug", nullptr, flags)) {
        if (!keys) {
            drag_scroll(s.panel_scrolled);
        }
        const Averages a = averages();
        const std::string where = s.level >= 0 && s.level < 19 ? kLevelNames[static_cast<std::size_t>(s.level)] : "boot";
        text_coloured(kYellow, "OpenRAC {}  {}  tick {}  {:.0f} tps  {:.0f} fps", s.game, where, s.tick, a.tps, a.fps);
        if (ImGui::Button(keys ? (s.paused ? "resume (F5)" : "pause (F5)") : (s.paused ? "resume" : "pause"))) {
            s.paused = !s.paused;
            s.held_frames = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button(keys ? "step (F6)" : "step")) {
            s.paused = true;
            s.step = true;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(dp(70.0f));
        static const char* const kSpeeds[] = {"1x", "1/2", "1/4", "1/8", "1/16"};
        int speed = std::clamp(static_cast<int>(std::lround(std::log2(s.stretch))), 0, 4);
        if (ImGui::Combo("speed", &speed, kSpeeds, 5)) {
            s.stretch = static_cast<double>(1 << speed);
        }
        if (s.paused) {
            ImGui::SameLine();
            text_coloured(kRed, "PAUSED");
        }
        if (ImGui::Button(keys ? "screenshot (F12)" : "screenshot")) {
            s.shot_path = shot_name(s.tick);
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
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + dp(420.0f));
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
        if (keys) {
            text_coloured(kDim, "F3 hide  F5 pause  F6 step  F7/F8 slower/faster  F12 screenshot");
        }
    }
    ImGui::End();
}

// The window as it is now, overlay included, to a PNG (the back buffer, before the swap). False
// when it could not be written.
bool write_window(const std::string& path, int width, int height) {
    using namespace openrac::gl;
    if (width <= 0 || height <= 0) {
        return false;
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
        return true;
    }
    s.message = "cannot write " + path;
    log::warn("{}", s.message);
    return false;
}

}  // namespace

void set_visible(bool on) {
    s.visible = on;
    stats::set_enabled(stats::Client::Overlay, on);
}

// A screenshot's file: in the working folder; on Android, whose apps run in "/" (not writable),
// in the app's own files (HOME, which the launcher sets).
std::string shot_name(std::uint64_t tick) {
    const std::string name = std::format("openrac_window_{}.png", tick);
#if defined(__ANDROID__)
    if (const std::optional<std::string> home = env::system_variable("HOME");
        home && !home->empty()) {
        std::error_code ignored;
        const std::filesystem::path dir = std::filesystem::path(*home) / "screenshots";
        std::filesystem::create_directories(dir, ignored);
        return (dir / name).string();
    }
#endif
    return name;
}

void request() {
    configure();
    s.wanted = true;
}

bool drawer_available() {
    return drawer_possible();
}

void toggle_drawer() {
    if (drawer_possible()) {
        set_drawer(!s.drawer_open);
    }
}

bool visible() {
    return s.visible;
}

bool event(const SDL_Event& e) {
    configure();
    if (drawer_possible() && drawer_event(e)) {
        return true;
    }
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
                s.shot_path = shot_name(s.tick);
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
    if (!(s.visible || menu_bar_shown() || s.drawer_open) || !s.initialised) {
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
        if (const host::LiveMoby* h = hero()) {
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
    const bool drawer_out = s.drawer_open || s.drawer_at > 0.0f || s.shot_when_shut;
    if (!(s.visible || menu_bar_shown() || badge || drawer_out || !s.shot_path.empty()) || shown.window == nullptr
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
    if (drawer_possible()) {
        // A gamepad moves through the drawer's rows while it is open, and through the panel while
        // the game stands still under it; it is the game's otherwise.
        ImGuiIO& io = ImGui::GetIO();
        const bool pad_moves = s.drawer_open || (s.visible && s.paused);
        io.ConfigFlags = pad_moves ? io.ConfigFlags | ImGuiConfigFlags_NavEnableGamepad
                                   : io.ConfigFlags & ~ImGuiConfigFlags_NavEnableGamepad;
    }
    if (!s.gl_subset) {
        ImGui_ImplOpenGL3_NewFrame();
    }
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
    if (drawer_possible()) {
        drawer(shown);
    }
    status_badge();
    ImGui::Render();
    if (s.gl_subset) {
        imgui_gl::render(ImGui::GetDrawData());
    } else {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
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
        s.toast = (write_window(s.shot_path, shown.window_width, shown.window_height) ? "saved " : "cannot save ")
                  + s.shot_path;
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

bool take_settings_changed() {
    return std::exchange(s.settings_changed, false);
}

void release() {
    if (s.initialised) {
        if (s.gl_subset) {
            imgui_gl::shutdown();
        } else {
            ImGui_ImplOpenGL3_Shutdown();
        }
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        s.initialised = false;
    }
}

}  // namespace openrac::frontend::debug_overlay

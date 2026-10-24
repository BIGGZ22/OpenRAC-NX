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
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <imgui_internal.h>  // ImGui::ClearActiveID: a finger that scrolls lets go of what it pressed
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/env.h"
#include "common/log.h"
#include "common/stats.h"
#include "debug/overlay.h"
#include "fixes.h"
#include "frontend.h"
#include "imgui_gl.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "options_menu.h"
#include "renderer/gl.h"
#include "renderer/gl_stats.h"
#include "settings.h"
#include "viewer/screenshot.h"

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

void set_visible(bool on) {
    s.visible = on;
    stats::set_enabled(stats::Client::Overlay, on);
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

// ---- The panel ----

// On Android the drawer has the menus (debug_overlay.h); OPENRAC_DRAWER=1 elsewhere too.
bool drawer_possible() {
#if defined(__ANDROID__)
    return true;
#else
    return env::flag(env::Var::Drawer);
#endif
}

// A finger dragged up or down the window being drawn scrolls it, and lets go of what it came down
// on, which is then not pressed. `scrolled`: the finger down now has scrolled.
void drag_scroll(bool& scrolled) {
    if (ImGui::IsMouseClicked(0)) {
        scrolled = false;
    }
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDown(0)
        && (scrolled || std::abs(ImGui::GetMouseDragDelta(0).y) > dp(10.0f))) {
        if (!scrolled) {
            ImGui::ClearActiveID();
        }
        scrolled = true;
        ImGui::SetScrollY(ImGui::GetScrollY() - ImGui::GetIO().MouseDelta.y);
    }
}

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

// A setting moved in the drawer or the menus: saved, and applied by the window
// (take_settings_changed). A picture shape of the player's own is the settings file's.
void change_setting(int row, int by) {
    options_menu::step(row, by);
    if (row == options_menu::kAspect && settings::get().aspect == settings::Aspect::Custom) {
        options_menu::step(row, by);
    }
    settings::save();
    s.settings_changed = true;
}

void gtao_controls() {
    auto& options = settings::get().gtao_settings;
    bool changed = false;
    ImGui::PushItemWidth(dp(170.0f));
    int quality = static_cast<int>(options.quality);
    if (ImGui::Combo("Quality", &quality, "Performance\0Quality\0")) {
        options.quality = static_cast<renderer::GtaoQuality>(quality);
        changed = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Performance is faster. Quality gives more accurate contact shading.");
    }
    changed |= ImGui::Checkbox("Half resolution", &options.half_resolution);
    changed |=
        ImGui::SliderInt("Directions", &options.slices, 1, 12, "%d", ImGuiSliderFlags_AlwaysClamp);
    changed |= ImGui::SliderInt(
        "Steps per side", &options.steps, 4, 24, "%d", ImGuiSliderFlags_AlwaysClamp
    );
    changed |= ImGui::SliderFloat(
        "Radius", &options.radius, 0.05f, 8.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp
    );
    changed |= ImGui::SliderFloat(
        "Strength multiplier", &options.strength, 0.0f, 3.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp
    );
    ImGui::PopItemWidth();
    if (ImGui::Button("Reset GTAO settings")) {
        options = renderer::GtaoSettings{};
        changed = true;
    }
    if (changed) {
        settings::save();
        s.settings_changed = true;
    }
}

void enhanced_controls() {
    auto& graphics = settings::get();
    const bool available = enhanced_graphics_available();
    if (!available) {
        ImGui::TextDisabled("Compatible probe data is required for this level.");
    }
    ImGui::BeginDisabled(!available);
    bool changed = ImGui::Checkbox("Enhanced graphics (F4)", &graphics.enhanced_graphics);
    ImGui::BeginDisabled(!graphics.enhanced_graphics);
    changed |= ImGui::Checkbox("Modern shadows", &graphics.enhanced_shadows);
    changed |= ImGui::Checkbox("Probe lighting", &graphics.probe_lighting);
    ImGui::BeginDisabled(!graphics.probe_lighting);
    changed |= ImGui::Checkbox("Probe spheres", &graphics.probe_spheres);
    if (graphics.probe_spheres) {
        ImGui::TextDisabled("Cyan: sparse   Green: dense");
        ImGui::TextDisabled("Gold: relocated   Magenta: invalid");
    }
    ImGui::PushItemWidth(dp(170.0f));
    changed |= ImGui::SliderFloat(
        "Probe GI multiplier",
        &graphics.probe_gi_multiplier,
        0.0f,
        5.0f,
        "%.2fx",
        ImGuiSliderFlags_AlwaysClamp
    );
    changed |= ImGui::SliderFloat(
        "Probe normal bias",
        &graphics.probe_normal_bias,
        0.0f,
        2.0f,
        "%.2f units",
        ImGuiSliderFlags_AlwaysClamp
    );
    changed |= ImGui::SliderFloat(
        "Probe view bias",
        &graphics.probe_view_bias,
        0.0f,
        2.0f,
        "%.2f units",
        ImGuiSliderFlags_AlwaysClamp
    );
    ImGui::PopItemWidth();
    ImGui::EndDisabled();
    changed |= ImGui::Checkbox("GTAO", &graphics.gtao);
    if (ImGui::CollapsingHeader("GTAO settings")) {
        gtao_controls();
    }
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    if (changed) {
        settings::save();
        s.settings_changed = true;
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
    return (kMenuBar || env::flag(env::Var::MenuBar)) && !drawer_possible();
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
    if (ImGui::BeginMenu("Graphics")) {
        // Each item moves its setting on, its value beside it, and leaves the menu open.
        ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
        using namespace options_menu;
        for (const int row :
             {kAspect, kHud, kRenderScale, kMsaa, kAnisotropy, kFrameRate, kVSync}) {
            const bool shown_for = row != kHud || settings::get().aspect != settings::Aspect::Standard;
            if (ImGui::MenuItem(label(row), value(row).c_str(), false, shown_for)) {
                change_setting(row, 1);
            }
        }
        ImGui::PopItemFlag();
        ImGui::EndMenu();
    }
    if (gl::enhanced_rendering_available() && ImGui::BeginMenu("Enhanced")) {
        enhanced_controls();
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

// ---- The drawer ----

// Opening it pauses the game, as an emulator's menu does; shutting it leaves the game as its "Keep
// paused" row says (paused as before it opened, unless changed there).
void set_drawer(bool open) {
    if (open == s.drawer_open) {
        return;
    }
    s.drawer_open = open;
    if (open) {
        s.keep_paused = s.paused;
        s.paused = true;
        s.held_frames = 0;
        s.drawer_focus = true;
    } else {
        s.paused = s.keep_paused;
    }
}

// A finger, at its place in the window, as ImGui's pointer.
void pointer_at(const SDL_TouchFingerEvent& f) {
    SDL_Window* window = SDL_GetWindowFromID(f.windowID);
    int w = 0;
    int h = 0;
    if (window == nullptr || !SDL_GetWindowSize(window, &w, &h)) {
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
    io.AddMousePosEvent(f.x * static_cast<float>(w), f.y * static_cast<float>(h));
}

// The drawer's events: Back and Guide open and shut it; fingers are ImGui's pointer (the system's
// mouse made from them follows only the first finger on the screen, perhaps a thumb on a stick),
// and those that land while it is open are its own, not the on-screen pad's.
bool drawer_event(const SDL_Event& e) {
    switch (e.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if (e.key.key != SDLK_AC_BACK) {
                return false;
            }
            if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
                set_drawer(!s.drawer_open);
            }
            return true;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            if (e.gbutton.button != SDL_GAMEPAD_BUTTON_GUIDE) {
                return false;
            }
            set_drawer(!s.drawer_open);
            return true;
        case SDL_EVENT_MOUSE_MOTION:
            return e.motion.which == SDL_TOUCH_MOUSEID;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            return e.button.which == SDL_TOUCH_MOUSEID;
        case SDL_EVENT_MOUSE_WHEEL:
            return e.wheel.which == SDL_TOUCH_MOUSEID;
        case SDL_EVENT_FINGER_DOWN: {
            if (e.tfinger.touchID == SDL_MOUSE_TOUCHID || !s.initialised) {
                return false;
            }
            if (!s.pointer_down && (s.drawer_open || s.visible)) {
                s.pointer = e.tfinger.fingerID;
                s.pointer_down = true;
                pointer_at(e.tfinger);
                ImGui::GetIO().AddMouseButtonEvent(0, true);
            }
            if (!s.drawer_open) {
                return false;
            }
            s.taken.push_back(e.tfinger.fingerID);
            return true;
        }
        case SDL_EVENT_FINGER_MOTION:
            if (s.pointer_down && e.tfinger.fingerID == s.pointer && s.initialised) {
                pointer_at(e.tfinger);
            }
            return std::find(s.taken.begin(), s.taken.end(), e.tfinger.fingerID) != s.taken.end();
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_CANCELED: {
            if (s.pointer_down && e.tfinger.fingerID == s.pointer && s.initialised) {
                pointer_at(e.tfinger);
                ImGuiIO& io = ImGui::GetIO();
                io.AddMouseButtonEvent(0, false);
                io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);  // a lifted finger hovers nothing
                s.pointer_down = false;
            }
            const auto it = std::find(s.taken.begin(), s.taken.end(), e.tfinger.fingerID);
            if (it == s.taken.end()) {
                return false;
            }
            s.taken.erase(it);
            return true;
        }
        default:
            return false;
    }
}

// The launcher's colours (launcher/docs/STYLE.md), so that the drawer looks like the rest of the app.
namespace look {
constexpr ImU32 kPanel = IM_COL32(0x1d, 0x1d, 0x28, 250);
constexpr ImU32 kCard = IM_COL32(0x22, 0x22, 0x2f, 255);
constexpr ImU32 kPressed = IM_COL32(0x3a, 0x2c, 0x16, 255);
constexpr ImU32 kLine = IM_COL32(156, 161, 212, 56);
constexpr ImU32 kLineStrong = IM_COL32(156, 161, 212, 90);
constexpr ImU32 kOrange = IM_COL32(0xe5, 0xa0, 0x13, 255);  // the launcher's primary buttons
constexpr ImU32 kOrangePressed = IM_COL32(0xb0, 0x70, 0x00, 255);
constexpr ImU32 kOrangeInk = IM_COL32(0x2a, 0x18, 0x00, 255);
constexpr ImU32 kAmber = IM_COL32(0xff, 0xb4, 0x43, 255);
constexpr ImU32 kGold = IM_COL32(0xff, 0xd0, 0x8a, 255);
constexpr ImU32 kText = IM_COL32(0xe8, 0xf0, 0xff, 255);
constexpr ImU32 kSoft = IM_COL32(0xaa, 0xb6, 0xdc, 255);
constexpr ImU32 kDim = IM_COL32(0x8f, 0x9a, 0xc4, 255);
constexpr ImU32 kOff = IM_COL32(0x3a, 0x3b, 0x52, 255);
constexpr ImU32 kErr = IM_COL32(0xff, 0x8a, 0x7a, 255);
}  // namespace look

// A section's rows on one card, as the launcher's settings have them: the card is drawn under the
// rows once their height is known, a hairline between each two.
void card_begin() {
    ImDrawList* list = ImGui::GetWindowDrawList();
    list->ChannelsSplit(2);
    list->ChannelsSetCurrent(1);
    s.card = {ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail().x, 0};
}

void card_end() {
    ImDrawList* list = ImGui::GetWindowDrawList();
    const ImVec2 end(s.card.top.x + s.card.width, ImGui::GetCursorScreenPos().y);
    list->ChannelsSetCurrent(0);
    list->AddRectFilled(s.card.top, end, look::kCard, dp(12.0f));
    list->AddRect(s.card.top, end, look::kLine, dp(12.0f));
    list->ChannelsMerge();
}

// A section's heading: its name, small, above its card.
void drawer_section(const char* title) {
    ImGui::Dummy(ImVec2(0.0f, dp(16.0f)));
    ImFont* font = ImGui::GetFont();
    const float size = ImGui::GetFontSize() * 0.85f;
    const ImVec2 at = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddText(font, size, ImVec2(at.x + dp(6.0f), at.y), look::kDim, title);
    ImGui::Dummy(ImVec2(0.0f, size + dp(6.0f)));
}

// A row of the card open now, as high as a thumb: true when it was pressed (a finger that dragged
// the rows along presses none). Its label at the left; `right` draws what it shows at the right,
// from the point given (the row's right edge, inset, at its middle).
template <typename Right>
bool drawer_row(const char* label, ImU32 ink, Right&& right) {
    const float height = dp(48.0f);
    const ImVec2 at = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    ImDrawList* list = ImGui::GetWindowDrawList();
    if (s.card.rows++ > 0) {
        list->AddLine(ImVec2(at.x + dp(16.0f), at.y), ImVec2(at.x + width - dp(16.0f), at.y), look::kLine);
    }
    ImGui::PushID(label);
    const bool pressed = ImGui::InvisibleButton("row", ImVec2(width, height));
    ImGui::PopID();
    const ImVec2 a(at.x + dp(3.0f), at.y + dp(3.0f));
    const ImVec2 b(at.x + width - dp(3.0f), at.y + height - dp(3.0f));
    if (ImGui::IsItemActive()) {
        list->AddRectFilled(a, b, look::kPressed, dp(9.0f));
    }
    if (ImGui::IsItemFocused() && ImGui::GetIO().NavVisible) {
        list->AddRect(a, b, look::kAmber, dp(9.0f), 0, dp(2.0f));
    }
    list->AddText(ImVec2(at.x + dp(16.0f), at.y + (height - ImGui::GetFontSize()) * 0.5f), ink, label);
    right(list, ImVec2(at.x + width - dp(16.0f), at.y + height * 0.5f));
    return pressed;
}

bool drawer_row(const char* label, ImU32 ink = look::kText) {
    return drawer_row(label, ink, [](ImDrawList*, ImVec2) {});
}

// An arrow as the launcher draws one: two strokes meeting at `tip`, pointing `dx` (-1 left, 1
// right) or, with `dx` 0, down (`dy` 1) or up (`dy` -1).
void arrow(ImDrawList* list, ImVec2 tip, float dx, float dy, ImU32 ink) {
    const float arm = dp(5.0f);
    const ImVec2 back(tip.x - dx * arm, tip.y - dy * arm);
    const ImVec2 points[3] = {ImVec2(back.x - dy * arm, back.y - dx * arm), tip,
                              ImVec2(back.x + dy * arm, back.y + dx * arm)};
    list->AddPolyline(points, 3, ink, ImDrawFlags_None, dp(2.0f));
}

// A row that turns something on and off, its switch at the right.
bool drawer_switch(const char* label, bool on) {
    return drawer_row(label, look::kText, [on](ImDrawList* list, ImVec2 right) {
        const float h = dp(22.0f);
        const ImVec2 a(right.x - dp(40.0f), right.y - h * 0.5f);
        const ImVec2 b(right.x, right.y + h * 0.5f);
        list->AddRectFilled(a, b, on ? look::kOrange : look::kOff, h * 0.5f);
        list->AddRect(a, b, look::kLineStrong, h * 0.5f);
        list->AddCircleFilled(ImVec2(on ? b.x - h * 0.5f : a.x + h * 0.5f, right.y), h * 0.5f - dp(3.0f),
                              on ? look::kOrangeInk : look::kSoft);
    });
}

// A row with its value between two arrows, as a console game's options: -1 or 1 when it was moved
// back or on. A touch on the value's left part moves it back, anywhere else on; a gamepad's or the
// keyboard's left and right move it while it has the focus, and A moves it on.
int drawer_choice(const char* label, const std::string& value, bool can_lower = true, bool can_raise = true) {
    float back_until = 0.0f;  // a touch left of this moves the value back
    const bool pressed = drawer_row(label, look::kText, [&](ImDrawList* list, ImVec2 right) {
        // The values of every row in one column, centred between its arrows.
        const float column = std::min(s.card.width * 0.48f, dp(168.0f));
        const float left = right.x - column;
        arrow(list, ImVec2(left, right.y), -1.0f, 0.0f, can_lower ? look::kAmber : look::kOff);
        arrow(list, ImVec2(right.x, right.y), 1.0f, 0.0f, can_raise ? look::kAmber : look::kOff);
        const ImVec2 size = ImGui::CalcTextSize(value.c_str());
        list->AddText(ImVec2(left + (column - size.x) * 0.5f, right.y - size.y * 0.5f), look::kGold, value.c_str());
        back_until = left + column * 0.35f;
    });
    int by = 0;
    if (pressed) {
        by = ImGui::IsMouseReleased(0) && ImGui::GetIO().MousePos.x < back_until ? -1 : 1;
    }
    if (ImGui::IsItemFocused()) {
        const auto held = [](ImGuiKey pad, ImGuiKey stick, ImGuiKey key) {
            return ImGui::IsKeyPressed(pad) || ImGui::IsKeyPressed(stick) || ImGui::IsKeyPressed(key);
        };
        if (held(ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadLStickLeft, ImGuiKey_LeftArrow)) {
            by = -1;
        } else if (held(ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadLStickRight, ImGuiKey_RightArrow)) {
            by = 1;
        }
    }
    return (by < 0 && !can_lower) || (by > 0 && !can_raise) ? 0 : by;
}

// One of the port's settings as a choice row (options_menu.h).
void setting_row(int row) {
    std::string value = options_menu::value(row);
    if (row == options_menu::kAspect && settings::get().aspect == settings::Aspect::Window) {
        value = "Fill screen";  // a window's shape, said as a phone's
    }
    if (const int by = drawer_choice(options_menu::label(row), value)) {
        change_setting(row, by);
    }
}

// The launcher's primary button, across the drawer.
bool drawer_button(const char* label) {
    const float height = dp(46.0f);
    const ImVec2 a = ImGui::GetCursorScreenPos();
    const ImVec2 b(a.x + ImGui::GetContentRegionAvail().x, a.y + height);
    const bool pressed = ImGui::InvisibleButton(label, ImVec2(b.x - a.x, height));
    ImDrawList* list = ImGui::GetWindowDrawList();
    const float round = dp(12.0f);
    list->AddRectFilled(a, b, ImGui::IsItemActive() ? look::kOrangePressed : look::kOrange, round);
    if (ImGui::IsItemFocused() && ImGui::GetIO().NavVisible) {
        list->AddRect(ImVec2(a.x - dp(3.0f), a.y - dp(3.0f)), ImVec2(b.x + dp(3.0f), b.y + dp(3.0f)), look::kGold,
                      round + dp(3.0f), 0, dp(2.0f));
    }
    const ImVec2 size = ImGui::CalcTextSize(label);
    list->AddText(ImVec2((a.x + b.x - size.x) * 0.5f, a.y + (height - size.y) * 0.5f), look::kOrangeInk, label);
    return pressed;
}

void drawer(const Shown& shown) {
    const ImGuiIO& io = ImGui::GetIO();
    const float target = s.drawer_open ? 1.0f : 0.0f;
    const float move = io.DeltaTime / 0.18f;  // out or in in under a fifth of a second
    s.drawer_at = s.drawer_at < target ? std::min(s.drawer_at + move, target) : std::max(s.drawer_at - move, target);
    if (s.drawer_at <= 0.0f) {
        if (s.shot_when_shut) {
            s.shot_when_shut = false;
            s.shot_path = shot_name(s.tick);
        }
        return;
    }
    const float ease = 1.0f - std::pow(1.0f - s.drawer_at, 3.0f);
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // The left of the screen a camera's notch keeps.
    float inset = 0.0f;
    SDL_Rect safe{};
    if (shown.window != nullptr && SDL_GetWindowSafeArea(shown.window, &safe)) {
        inset = static_cast<float>(safe.x);
    }
    const float width = std::min(viewport->Size.x * 0.6f, dp(360.0f)) + inset;
    const float x = viewport->Pos.x - width * (1.0f - ease);
    ImGui::GetBackgroundDrawList()->AddRectFilled(viewport->Pos,
                                                  ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
                                                  IM_COL32(0x12, 0x12, 0x1a, static_cast<int>(150.0f * ease)));
    // A touch beside it shuts it, as does the gamepad's B.
    if (s.drawer_open && ImGui::IsMouseReleased(0) && !s.drawer_scrolled && io.MouseClickedPos[0].x > x + width) {
        set_drawer(false);
    }
    if (s.drawer_open && ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false)) {
        set_drawer(false);
    }
    ImGui::SetNextWindowPos(ImVec2(x, viewport->Pos.y));
    ImGui::SetNextWindowSize(ImVec2(width, viewport->Size.y));
    if (s.drawer_focus) {
        ImGui::SetNextWindowFocus();
        s.drawer_focus = false;
    }
    // The launcher's panel: its ink, a thin border, 16-pixel corners on the open side; a header
    // above the rows, which scroll under it.
    const float header = dp(52.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, dp(16.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(dp(14.0f) + inset, header + dp(4.0f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, dp(4.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImGui::ColorConvertU32ToFloat4(look::kLineStrong));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                   | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("OpenRAC options", nullptr, flags)) {
        drag_scroll(s.drawer_scrolled);
        {
            ImDrawList* back = ImGui::GetWindowDrawList();
            const ImVec2 top(x, viewport->Pos.y);
            const ImVec2 bottom(x + width, viewport->Pos.y + viewport->Size.y);
            back->PushClipRect(top, bottom, false);
            back->AddRectFilled(top, bottom, look::kPanel, dp(16.0f), ImDrawFlags_RoundCornersRight);
            back->AddRect(top, bottom, look::kLine, dp(16.0f), ImDrawFlags_RoundCornersRight);
            back->PopClipRect();
        }
        {
            // The header: the wordmark, and what the game is doing while the drawer is out.
            ImDrawList* front = ImGui::GetForegroundDrawList();
            const ImVec2 top(x, viewport->Pos.y);
            const ImVec2 bottom(x + width, viewport->Pos.y + header);
            front->AddRectFilled(top, bottom, look::kPanel, dp(16.0f), ImDrawFlags_RoundCornersTopRight);
            if (ImGui::GetScrollY() > 0.0f) {
                front->AddLine(ImVec2(top.x, bottom.y), bottom, look::kLine);
            }
            ImFont* font = ImGui::GetFont();
            const float size = ImGui::GetFontSize() * 1.2f;
            const float text_y = top.y + (header - size) * 0.5f;
            const float text_x = top.x + inset + dp(18.0f);
            const float open = font->CalcTextSizeA(size, FLT_MAX, 0.0f, "Open").x;
            front->AddText(font, size, ImVec2(text_x, text_y), look::kText, "Open");
            front->AddText(font, size, ImVec2(text_x + open, text_y), look::kAmber, "RAC");
            const char* state = s.keep_paused ? "Paused" : "Paused while open";
            const ImVec2 state_size = ImGui::CalcTextSize(state);
            const ImVec2 pill_b(bottom.x - dp(16.0f), top.y + header * 0.5f + state_size.y * 0.5f + dp(4.0f));
            const ImVec2 pill_a(pill_b.x - state_size.x - dp(20.0f), pill_b.y - state_size.y - dp(8.0f));
            front->AddRectFilled(pill_a, pill_b, look::kOff, (pill_b.y - pill_a.y) * 0.5f);
            front->AddText(ImVec2(pill_a.x + dp(10.0f), pill_a.y + dp(4.0f)), look::kSoft, state);
        }

        if (drawer_button("Resume")) {
            set_drawer(false);
        }
        ImGui::SetItemDefaultFocus();  // where a gamepad starts

        drawer_section("GRAPHICS");
        card_begin();
        setting_row(options_menu::kAspect);
        if (settings::get().aspect != settings::Aspect::Standard) {
            setting_row(options_menu::kHud);  // the HUD at the 4:3 picture's middle, or out at the edges
        }
        setting_row(options_menu::kRenderScale);
        setting_row(options_menu::kMsaa);
        setting_row(options_menu::kAnisotropy);
        setting_row(options_menu::kFrameRate);
        setting_row(options_menu::kVSync);
        if (drawer_switch("Frame counter", settings::get().show_fps)) {
            change_setting(options_menu::kShowFps, 1);
        }
        card_end();

        if (gl::enhanced_rendering_available()) {
            drawer_section("ENHANCED");
            card_begin();
            enhanced_controls();
            card_end();
        }

        drawer_section("SOUND AND GAME");
        card_begin();
        const int volume = settings::get().volume;
        if (const int by = drawer_choice("Volume", std::format("{}%", volume), volume > 0, volume < 100)) {
            settings::set_volume(volume + by * 10);
            settings::save();
        }
        if (drawer_switch("Skip cutscenes", settings::get().skip_cutscenes)) {
            change_setting(options_menu::kSkipCutscenes, 1);
        }
        card_end();

        // What the port changes from the console that the player may switch back (fixes.h): on is
        // the port's fix, off the console's behaviour. The launcher's page for the game says more.
        if (fixes::count() > 0) {
            drawer_section("GAME FIXES");
            card_begin();
            for (int i = 0; i < fixes::count(); ++i) {
                if (drawer_switch(fixes::fix(i).name, fixes::on(i))) {
                    change_setting(options_menu::kRows + 1 + i, 1);
                }
            }
            card_end();
        }

        ImGui::Dummy(ImVec2(0.0f, dp(16.0f)));
        card_begin();
        const bool developer = s.drawer_developer;
        if (drawer_row("Developer tools", look::kText, [developer](ImDrawList* list, ImVec2 right) {
                arrow(list, ImVec2(right.x - dp(5.0f), right.y + (developer ? -dp(2.5f) : dp(2.5f))), 0.0f,
                      developer ? -1.0f : 1.0f, look::kSoft);
            })) {
            s.drawer_developer = !s.drawer_developer;
        }
        if (developer) {
            if (drawer_switch("Debug overlay", s.visible)) {
                set_visible(!s.visible);
            }
            if (drawer_switch("Keep paused", s.keep_paused)) {
                s.keep_paused = !s.keep_paused;
            }
            if (drawer_row("Step one tick")) {
                s.keep_paused = true;
                s.step = true;
            }
            // Back is slower, on is faster: full speed at the right end.
            const std::string speed = s.stretch == 1.0 ? "Full" : std::format("1/{}", static_cast<int>(s.stretch));
            if (const int by = drawer_choice("Speed", speed, s.stretch < 16.0, s.stretch > 1.0)) {
                s.stretch = by < 0 ? std::min(s.stretch * 2.0, 16.0) : std::max(s.stretch / 2.0, 1.0);
            }
            if (drawer_row("Screenshot")) {
                s.shot_when_shut = true;
                set_drawer(false);
            }
            if (s.developer && drawer_row("Level select")) {
                set_drawer(false);
                press(SDLK_F1);
            }
        }
        card_end();

        ImGui::Dummy(ImVec2(0.0f, dp(16.0f)));
        card_begin();
        if (drawer_row("Quit game", look::kErr)) {
            SDL_Event quit;
            SDL_zero(quit);
            quit.type = SDL_EVENT_QUIT;
            SDL_PushEvent(&quit);
        }
        card_end();

        // What the picture is drawn with: the launcher's renderer choice takes effect only when the
        // game starts, and here the player sees which one this run has.
        {
            static const std::string graphics = [] {
                const auto* name = reinterpret_cast<const char*>(openrac::gl::glGetString(openrac::gl::GL_RENDERER));
                const char* api = s.gl_subset ? "Vulkan" : openrac::gl::is_es() ? "OpenGL ES" : "OpenGL";
                return std::format("{} · {}", api, name != nullptr ? name : "unknown GPU");
            }();
            ImGui::Dummy(ImVec2(0.0f, dp(14.0f)));
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(look::kDim));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + dp(6.0f));
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextUnformatted(graphics.c_str());
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();
            ImGui::Dummy(ImVec2(0.0f, dp(12.0f)));
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(5);
}

// What the keys of pause and speed have set, said in the picture whether the panel is shown or
// not: a game standing still or running at a quarter speed with nothing saying why is a puzzle.
void status_badge() {
    if (!s.toast.empty() && ImGui::GetTime() >= s.toast_until) {
        s.toast.clear();
    }
    if (s.drawer_at > 0.0f) {
        return;  // the drawer says it
    }
    // With the drawer, without the keys: its rows change them.
    const bool keys = !drawer_possible();
    std::string line;
    if (s.paused) {
        line = keys ? "PAUSED    F5 resume    F6 one tick" : "PAUSED";
    } else if (s.stretch != 1.0) {
        line = keys ? std::format("SPEED 1/{}    F8 faster    F7 slower", static_cast<int>(s.stretch))
                    : std::format("SPEED 1/{}", static_cast<int>(s.stretch));
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

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "frontend.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>

#include "common/env.h"
#include "common/log.h"
#include "common/stats.h"
#include "debug_menu.h"
#include "debug_overlay.h"
#include "enhanced/game/scripted_moby_colour.h"
#include "enhanced/viewer/hero_shadow.h"
#include "fps_counter.h"
#include "frame_pacing.h"
#include "game/common/frontend/geometry.h"
#include "game/common/frontend/pacing.h"
#include "game/common/frontend/state.h"
#include "game/common/particle_texture_cache.h"
#include "game/rac1/game_state.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "options_menu.h"
#include "platform/gpu_driver.h"
#include "platform/input.h"
#include "platform/performance_hint.h"
#include "platform/pump.h"
#include "platform/window.h"
#include "renderer/direct.h"
#include "renderer/effects.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/gl_stats.h"
#include "renderer/renderer.h"
#include "renderer/shadows.h"
#include "settings.h"
#include "touch_overlay.h"
#include "viewer/level.h"
#include "viewer/level_renderer.h"
#include "viewer/moby_pose.h"
#include "viewer/moby_shadow.h"
#include "viewer/screenshot.h"

extern "C" int openrac_game_loaded_overlay(void);

#include <stb_image.h>

extern "C" {
extern const unsigned char openrac_icon_png[];
extern const unsigned int openrac_icon_png_size;
}

// The game's name and version, "Ratchet & Clank (PAL)" (cmake/Games.cmake).
#ifndef OPENRAC_GAME_TITLE
#define OPENRAC_GAME_TITLE "OpenRAC"
#endif

namespace openrac::frontend {

// The frontend's state (frontend/state.h).
std::unique_ptr<State> g;

// When frames reach the display (frontend/state.h).
Pacing pacing;

// Returns the value `var` was set to as a string of its own, or nothing when it is unset. For the
// reads below that hand the value to the C library (a file to open, a script for sscanf), which
// wants a string that ends in a NUL: env::value's view need not.
std::optional<std::string> env_string(env::Var var) {
    const std::optional<std::string_view> found = env::value(var);

    // An unset variable stays nothing, which differs from the empty string.
    if (!found) {
        return std::nullopt;
    }

    return std::string(*found);
}

// Returns the frame number `var` names, read as std::strtoull reads it, or nothing when it is
// unset. The frame numbers (OPENRAC_DUMP_RAM, OPENRAC_FX_DUMP, OPENRAC_SHOT_FROM) have always been
// read so: env::integer reads as std::strtoll, which differs for a negative value past -2^63
// (strtoull wraps it to a frame number, strtoll holds it to -2^63).
std::optional<std::uint64_t> env_frame(env::Var var) {
    const std::optional<std::string> text = env_string(var);
    if (!text) {
        return std::nullopt;
    }
    return std::strtoull(text->c_str(), nullptr, 10);
}

// Returns OPENRAC_SHOT read, once. EVERY is read as std::atoll read it (std::strtoll), and a
// negative one wraps to a large period, as it always did.
const Shots& shots() {
    static const Shots read = [] {
        Shots result;
        const std::optional<std::string_view> value = env::value(env::Var::Shot);

        // Without a ':' there is no path to write to, and no shot.
        const std::size_t colon = value ? value->find(':') : std::string_view::npos;
        if (colon != std::string_view::npos) {
            result.every = static_cast<std::uint64_t>(env::read_integer(value, 10, 0));
            result.prefix = std::string(value->substr(colon + 1));
        }
        return result;
    }();
    return read;
}

// Where a scripted run is (frontend/state.h).
Route g_route;

// Whether Start would skip what is playing now: a level's in-game scene (mode 2), or the flight
// between planets (the boot program's mode 6).
bool skippable() {
    return (g_route.level >= 0 && g_route.mode == 2) || (g_route.level < 0 && g_route.mode == 6);
}

// The picture's shape now: the player's setting, for the window as it is.
float picture_aspect(int window_width, int window_height) {
    return settings::aspect_ratio(settings::get(), window_width, window_height);
}

// The development switches that keep one frame a tick whatever the settings say.
bool one_frame_a_tick() {
    static const bool on = env::flag(env::Var::Uncapped) || env::flag(env::Var::NoBlend);
    return on;
}

// Whether a swap waits for a refresh: the player's vsync setting, when frames are drawn between
// ticks. With one frame a tick (the "Original" frame rate; OPENRAC_UNCAPPED, OPENRAC_NO_BLEND)
// nothing waits for the display: the tick paces the frames, and 50 of them a second each held for
// a refresh of a 60 or 120 Hz display would hold the tick up with them.
platform::SwapInterval wanted_swap() {
    const settings::Settings& s = settings::get();
    if (one_frame_a_tick() || s.frame_rate == settings::FrameRate::Original || s.vsync == settings::VSync::Off) {
        return platform::SwapInterval::Immediate;
    }
    return s.vsync == settings::VSync::Adaptive ? platform::SwapInterval::Adaptive : platform::SwapInterval::VSync;
}

platform::DisplayMode window_mode(settings::DisplayMode mode) {
    switch (mode) {
        case settings::DisplayMode::Borderless:
            return platform::DisplayMode::Borderless;
        case settings::DisplayMode::Fullscreen:
            return platform::DisplayMode::Fullscreen;
        default:
            return platform::DisplayMode::Windowed;
    }
}

// Draws `framebuffer` (width x height) into the window's box, black around it; `flip`: its first
// row is the top (an uploaded picture) rather than the bottom (a rendered frame).
void present(unsigned framebuffer, int width, int height, int window_width, int window_height, bool flip,
             float aspect = kTvAspect) {
    using namespace gl;
    const Box b = frame_box(window_width, window_height, aspect);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    const int y0 = flip ? b.y + b.height : b.y;
    const int y1 = flip ? b.y : b.y + b.height;
    glBlitFramebuffer(0, 0, width, height, b.x, y0, b.x + b.width, y1, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

// OpenRAC's icon on the window (the taskbar's, and the title bar's where there is one): the
// launcher's picture, embedded by cmake/Games.cmake.
void set_window_icon(platform::Window& window) {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* rgba = stbi_load_from_memory(openrac_icon_png, static_cast<int>(openrac_icon_png_size), &width,
                                                &height, &channels, 4);
    if (rgba == nullptr) {
        return;
    }
    window.set_icon(rgba, width, height);
    stbi_image_free(rgba);
}

#if defined(__linux__) || defined(__FreeBSD__)
static bool write_file_if_changed(const std::filesystem::path& path, const void* data, std::size_t size) {
    std::error_code ec;
    if (std::filesystem::exists(path, ec) && std::filesystem::file_size(path, ec) == size) {
        std::ifstream current(path, std::ios::binary);
        if (current) {
            std::vector<char> buffer(size);
            if (current.read(buffer.data(), static_cast<std::streamsize>(size))) {
                if (std::memcmp(buffer.data(), data, size) == 0) {
                    return false;
                }
            }
        }
    }
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        return false;
    }
    out.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    return out.good();
}

static void ensure_linux_desktop_integration(const std::string& game_id) {
    const std::optional<std::string> home = env::system_variable("HOME");
    if (!home || home->empty()) {
        return;
    }
    const std::filesystem::path home_path = *home;
    const std::filesystem::path icons_base = home_path / ".local/share/icons/hicolor";
    const std::filesystem::path app_dir = home_path / ".local/share/applications";

    bool icons_changed = false;
    bool entries_changed = false;

    // The game's icon for the desktop and taskbar (KDE Plasma, GNOME, etc.).
    // Wayland compositors identify windows by their app_id (dev.openrac.<game_id>)
    // and match them against .desktop files in ~/.local/share/applications/ and
    // icons in ~/.local/share/icons/hicolor/.
    const std::filesystem::path icon_dir = icons_base / "512x512/apps";
    const std::string app_id = "dev.openrac." + game_id;
    const std::string prog_id = "openrac-" + game_id;

    if (openrac_icon_png_size > 0) {
        icons_changed |= write_file_if_changed(icon_dir / (app_id + ".png"), openrac_icon_png, openrac_icon_png_size);
        icons_changed |= write_file_if_changed(icon_dir / (prog_id + ".png"), openrac_icon_png, openrac_icon_png_size);
    }

    // Resolve current executable path for desktop entry.
    std::error_code ec;
    std::filesystem::path exe_path = std::filesystem::canonical("/proc/self/exe", ec);
    if (ec || exe_path.empty()) {
        exe_path = prog_id;
    }

    const auto make_entry = [&](const std::string& wm_class, bool hidden) {
        std::string entry = "[Desktop Entry]\n";
        entry += "Type=Application\n";
        entry += "Name=";
        entry += OPENRAC_GAME_TITLE;
        entry += "\nComment=Play ";
        entry += OPENRAC_GAME_TITLE;
        entry += " with OpenRAC\n";
        entry += "Exec=\"";
        entry += exe_path.string();
        entry += "\"\n";
        entry += "Icon=" + app_id + "\n";
        entry += "Terminal=false\n";
        entry += "Categories=Game;\n";
        entry += "StartupNotify=true\n";
        entry += "StartupWMClass=" + wm_class + "\n";
        if (hidden) {
            entry += "NoDisplay=true\n";
        }
        return entry;
    };

    const std::string dev_entry = make_entry(app_id, false);
    const std::string prog_entry = make_entry(prog_id, true);

    entries_changed |= write_file_if_changed(app_dir / (app_id + ".desktop"), dev_entry.data(), dev_entry.size());
    entries_changed |= write_file_if_changed(app_dir / (prog_id + ".desktop"), prog_entry.data(), prog_entry.size());

    // Refresh system caches if files were added or modified
    if (icons_changed) {
        if (std::system(nullptr)) {
            std::string cmd = "gtk-update-icon-cache -f -t \"" + icons_base.string() + "\" >/dev/null 2>&1 &";
            (void)std::system(cmd.c_str());
        }
    }
    if (entries_changed || icons_changed) {
        if (std::system(nullptr)) {
            std::string cmd1 = "update-desktop-database \"" + app_dir.string() + "\" >/dev/null 2>&1 &";
            (void)std::system(cmd1.c_str());
            std::string cmd2 = "kbuildsycoca6 >/dev/null 2>&1 &";
            (void)std::system(cmd2.c_str());
        }
    }
}
#endif

bool open(const std::string& game_id, const std::filesystem::path& levels, std::string& error) {
    g = std::make_unique<State>();
    g->game = game_id;
    g->levels = levels;
    // Before SDL starts: on Wayland the app id names the program to the desktop, and a machine
    // with two graphics cards is asked for the fast one.
    const std::string app_id = "dev.openrac." + game_id;
#if defined(__linux__) || defined(__FreeBSD__)
    ensure_linux_desktop_integration(game_id);
#endif
    platform::set_app_metadata(OPENRAC_GAME_TITLE, app_id.c_str());
    platform::prefer_discrete_gpu();
    platform::prepare_gpu_driver();
#if defined(__ANDROID__)
    // Back opens and shuts the options drawer instead of leaving the game (debug_overlay.h), from
    // the first moment the window is there.
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
#endif
    g->sdl = std::make_unique<platform::Platform>(platform::kVideo | platform::kGamepad | platform::kAudio);
    if (!g->sdl->ok()) {
        error = g->sdl->error();
        return false;
    }
    platform::WindowConfig config;
    config.title = OPENRAC_GAME_TITLE;
    config.width = 1280;
    config.height = 960;
    // The game ticks 60 times a second (main.cpp, every version) and the window draws at the
    // display's, the frames between ticks moving the scene on (between), each shown at a refresh
    // when the player's vsync is on (wanted_swap).
    config.vsync = false;  // set below, as asked
    const settings::Settings& chosen = settings::get();
    if (chosen.window_width >= 320 && chosen.window_height >= 240) {
        // The window as the player left it.
        config.width = chosen.window_width;
        config.height = chosen.window_height;
    } else if (chosen.aspect != settings::Aspect::Standard && chosen.aspect != settings::Aspect::Window) {
        // A window of the picture's shape.
        config.width = static_cast<int>(std::lround(static_cast<float>(config.height) * picture_aspect(0, 0)));
    }
    config.mode = window_mode(chosen.display_mode);
    config.display = chosen.monitor;
    config.fullscreen_width = chosen.fullscreen_width;
    config.fullscreen_height = chosen.fullscreen_height;
    // The game is drawn by a graphics card or not at all: a software rasteriser shows a level at a
    // few frames a second, which looks like a broken port (OPENRAC_SOFTWARE_GL=1: run it anyway).
    config.software = env::flag(env::Var::SoftwareGl) ? platform::SoftwareRenderer::Allow
                                                      : platform::SoftwareRenderer::Refuse;
    // OPENRAC_HIDDEN=1: the window is made and drawn as always but never shown, for scripted runs
    // that should not open windows on the player's screen (several can run at once).
    config.hidden = env::flag(env::Var::Hidden);
    g->window = platform::Window::open(config, error);
    if (!g->window) {
        return false;
    }
    platform::install_app_menus(env::flag(env::Var::Developer));
    set_window_icon(*g->window);
    g->swap_asked = wanted_swap();
    g->swap = g->window->set_swap_interval(g->swap_asked);
    g->display_mode = chosen.display_mode;
    g->monitor = chosen.monitor;
    g->fullscreen_width = chosen.fullscreen_width;
    g->fullscreen_height = chosen.fullscreen_height;
    g->aspect = chosen.aspect;
    g->aspect_custom_x = chosen.aspect_custom_x;
    g->aspect_custom_y = chosen.aspect_custom_y;
    std::string missing;
    if (!gl::load(platform::Window::gl_loader(), missing)) {
        error = "OpenGL functions missing: " + missing;
        return false;
    }
    // [debug tools] The renderer's GL calls counted from here on (renderer/gl_stats.h).
    renderer::gl_stats::install(platform::Window::gl_loader());
    log::info("graphics: up to {} samples a pixel, anisotropy up to {:.0f}; the swap {}", 
              renderer::FrameBuffer::max_samples(), renderer::TexturePool::max_anisotropy(),
              g->swap == platform::SwapInterval::Immediate ? "does not wait"
              : g->swap == platform::SwapInterval::Adaptive ? "waits for the display (adaptive)"
                                                            : "waits for the display");
    return true;
}

void mobys_drawn(std::span<const std::uint8_t> ram, std::uint32_t first, int count) {
    if (!g) {
        return;
    }
    const int level = openrac_game_loaded_overlay();
    const viewer::GameAddresses& a =
        viewer::rac1_addresses(g->game, level);
    const viewer::GameState camera = viewer::read_game_state(ram, a);
    for (int i = 0; i < count; ++i) {
        g->moby_cameras.emplace_back(first + static_cast<std::uint32_t>(i) * 0x100u, camera);
    }
}

void world_drawn(std::span<const std::uint8_t> ram) {
    if (!g || g->world_camera) {
        return;
    }
    const int level = openrac_game_loaded_overlay();
    const viewer::GameAddresses& a =
        viewer::rac1_addresses(g->game, level);
    g->world_camera = viewer::read_game_state(ram, a);
}

bool show_picture(const std::uint8_t* rgba, int width, int height, float black) {
    using namespace gl;
    if (!g || width <= 0 || height <= 0) {
        return true;
    }
    SDL_Event event;
    while (platform::poll_event(event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
        if (debug_overlay::event(event)) {
            continue;  // [debug tools]
        }
        g->input.handle_event(event);
    }
    apply_mouse_capture();
    g->input.update();
    stats::begin_frame(stats::FrameKind::Picture, g->index);  // [debug tools]

    // The fade, on the bytes as the chip's black quad blends them: C * (1 - coverage).
    std::vector<std::uint8_t> pixels(rgba, rgba + static_cast<std::size_t>(width) * height * 4);
    if (black > 0.0f) {
        const float keep = black >= 1.0f ? 0.0f : 1.0f - black;
        for (std::size_t i = 0; i < pixels.size(); i += 4) {
            for (int c = 0; c < 3; ++c) {
                pixels[i + c] = static_cast<std::uint8_t>(static_cast<float>(pixels[i + c]) * keep);
            }
        }
    }
    std::string error;
    if (g->picture.width() != width || g->picture.height() != height) {
        g->picture.create(width, height, error);
    }
    glBindTexture(GL_TEXTURE_2D, g->picture.colour_texture());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    int window_width = 0;
    int window_height = 0;
    g->window->drawable_size(window_width, window_height);
    const std::uint64_t index = g->index++;
    if (shots().due(index)) {
        const std::string path = shots().prefix + std::to_string(index) + ".png";
        viewer::write_png(path, width, height, pixels);
        log::info("frame {} written to {}", index, path);
    }
    // The picture fills the frame the game's own frames fill; its first row is the top.
    present(g->picture.id(), width, height, window_width, window_height, true);
    draw_touch_pad(window_width, window_height);
    {
        // [debug tools] The overlay over a picture too (no renderer or camera then).
        const Box box = frame_box(window_width, window_height);
        debug_overlay::Shown shown{};
        shown.window = g->window->sdl();
        shown.window_width = window_width;
        shown.window_height = window_height;
        shown.box_x = box.x;
        shown.box_y = box.y;
        shown.box_width = box.width;
        shown.box_height = box.height;
        shown.frame_width = width;
        shown.frame_height = height;
        shown.vsync = g->swap != platform::SwapInterval::Immediate;
        debug_overlay::draw(shown);
    }
    g->window->swap();
    return true;
}

std::uint32_t text(int id) {
    return options_menu::text(id);
}

std::int64_t since_control() {
    return g && g_route.control >= 0 ? static_cast<std::int64_t>(g->index) - g_route.control : -1;
}

void close() {
    if (g) {
        debug_overlay::release();
        settings::save();  // the window's size, if it was changed
        fps_counter::release();
        touch_overlay::release();
        debug_menu::release();
        g->frame.release();
        g->picture.release();
        g->last_world.release();
        g->renderer.reset();
        g->scene.release();
        g.reset();
    }
}

}  // namespace openrac::frontend


// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The window's own side of the frontend, a slice of frontend/state.h: the player's display settings
// applied to the window (apply_settings), and the window's events and keys, which the game never
// sees (window_key).
//
// The window's opening (open), its icon and the Linux desktop entries are in frontend.cpp: they
// read the icon's bytes through an extern "C" declaration, which a new C++ file may only hold when
// it is named for a C contract (CODING_CONVENTIONS.md, section 14).

#include <cmath>

#include <SDL3/SDL.h>

#include "common/log.h"
#include "debug_overlay.h"
#include "frontend.h"
#include "game/common/frontend/state.h"
#include "renderer/gl.h"
#include "settings.h"

namespace openrac::frontend {

namespace {

// A window the player sizes: never on Android, where the game covers the screen whatever the
// settings say (Window::set_display_mode).
bool windowed() {
#if defined(__ANDROID__)
    return false;
#else
    return g->display_mode == settings::DisplayMode::Windowed;
#endif
}

// The window's own events, which the game never sees: the window moved to another display or its
// display changed mode (the refresh rate is read again), it was sized (the size is kept for the
// next start), it lost or got the keyboard (the player's "mute in the background").
void window_event(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
        case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            pacing.interval_stale = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            // Remembered in memory; written with the next setting saved and when the window closes.
            if (windowed() && event.window.data1 >= 320 && event.window.data2 >= 240) {
                settings::get().window_width = event.window.data1;
                settings::get().window_height = event.window.data2;
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            g->muted = event.type == SDL_EVENT_WINDOW_FOCUS_LOST;
            break;
        default:
            break;
    }
    const bool muted = g->muted && settings::get().mute_unfocused;
    settings::set_muted(muted);
}

}  // namespace

// The window as the player's settings want it: its display mode, display and fullscreen size, its
// shape, and each swap waiting for a refresh or not. Called when they may have changed; all of
// them take effect at once (the frame's size, its samples and the textures' filter follow the
// settings by themselves, in draw_kept).
void apply_settings() {
    const settings::Settings& s = settings::get();
    if (s.display_mode != g->display_mode || s.monitor != g->monitor
        || (s.display_mode == settings::DisplayMode::Fullscreen
            && (s.fullscreen_width != g->fullscreen_width
                || s.fullscreen_height != g->fullscreen_height))) {
        g->window->set_display_mode(
            window_mode(s.display_mode), s.fullscreen_width, s.fullscreen_height, s.monitor
        );
        g->display_mode = s.display_mode;
        g->monitor = s.monitor;
        g->fullscreen_width = s.fullscreen_width;
        g->fullscreen_height = s.fullscreen_height;
        pacing.interval_stale = true;
        // A change of mode may leave the driver's swap interval at its default: asked again below.
        g->swap_asked = wanted_swap() == platform::SwapInterval::Immediate
                            ? platform::SwapInterval::VSync
                            : platform::SwapInterval::Immediate;
    }
    if (s.aspect != g->aspect || s.aspect_custom_x != g->aspect_custom_x
        || s.aspect_custom_y != g->aspect_custom_y) {
        g->aspect = s.aspect;
        g->aspect_custom_x = s.aspect_custom_x;
        g->aspect_custom_y = s.aspect_custom_y;
        // A window takes the picture's new shape, at the height it has.
        int w = 0;
        int h = 0;
        if (windowed() && s.aspect != settings::Aspect::Window
            && SDL_GetWindowSize(g->window->sdl(), &w, &h) && h > 0) {
            SDL_SetWindowSize(
                g->window->sdl(),
                static_cast<int>(std::lround(static_cast<float>(h) * picture_aspect(w, h))),
                h
            );
        }
    }
    if (wanted_swap() != g->swap_asked) {
        g->swap_asked = wanted_swap();
        g->swap = g->window->set_swap_interval(g->swap_asked);
        pacing.last_swap = {};
    }
}

// The window's own keys: F11 and Alt+Return change between fullscreen and the window, F9 and F10
// turn the port's volume down and up a tenth, F2 shows or hides the frame counter,
// and F4 toggles enhanced graphics. True when the event was one of them.
bool window_key(const SDL_Event& event) {
    window_event(event);
    if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat) {
        return false;
    }
    if (event.key.key == SDLK_F2) {  // the frame counter, as the Video options' row
        settings::get().show_fps = !settings::get().show_fps;
        settings::save();
        return true;
    }
    if (event.key.key == SDLK_F4 && gl::enhanced_rendering_available()) {
        if (!enhanced_graphics_available()) {
            debug_overlay::notice(
                "Enhanced lighting requires compatible probe data for this level."
            );
            return true;
        }
        auto& graphics = settings::get();
        graphics.enhanced_graphics = !graphics.enhanced_graphics;
        settings::save();
        log::info("enhanced graphics {}", graphics.enhanced_graphics ? "on" : "off");
        return true;
    }
    if (event.key.key == SDLK_F9 || event.key.key == SDLK_F10) {
        settings::set_volume(settings::get().volume + (event.key.key == SDLK_F10 ? 10 : -10));
        settings::save();
        log::info("volume {}", settings::get().volume);
        return true;
    }
    const bool alt_return = (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER)
                            && (event.key.mod & SDL_KMOD_ALT) != 0;
    if (event.key.key != SDLK_F11 && !alt_return) {
        return false;
    }
    // Between the window and the fullscreen kind last chosen (borderless until one was).
    static settings::DisplayMode full = settings::DisplayMode::Borderless;
    settings::Settings& s = settings::get();
    if (s.display_mode == settings::DisplayMode::Windowed) {
        s.display_mode = full;
    } else {
        full = s.display_mode;
        s.display_mode = settings::DisplayMode::Windowed;
    }
    settings::save();
    apply_settings();
    return true;
}

}  // namespace openrac::frontend

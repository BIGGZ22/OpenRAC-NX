// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The player's input side of the frontend, a slice of frontend/state.h: the on-screen touch pad
// (draw_touch_pad), the cursor the mods capture (apply_mouse_capture, capture_mouse), the keyboard
// and mouse as the mods read them (keyboard_mouse, take_keyboard), and the pad the game reads each
// frame (pad), with the scripted presses of OPENRAC_PRESS and Start's skip of a scene
// (skip_after_mods). The events themselves reach platform::Input in frontend.cpp's frame loop.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>

#include <SDL3/SDL.h>

#include "common/env.h"
#include "common/log.h"
#include "debug_menu.h"
#include "debug_overlay.h"
#include "frontend.h"
#include "game/common/frontend/geometry.h"
#include "game/common/frontend/state.h"
#include "openrac/game_host.h"
#include "renderer/ui/touch_overlay.h"
#include "settings.h"

namespace openrac::frontend {

// The on-screen pad, kept inside the screen's safe area (a camera's notch at a phone's side).
void draw_touch_pad(int window_width, int window_height) {
    platform::TouchPad& touch = g->input.touch_pad();
    SDL_Rect safe{};
    int w = 0;
    int h = 0;
    if (SDL_GetWindowSafeArea(g->window->sdl(), &safe)
        && SDL_GetWindowSize(g->window->sdl(), &w, &h) && h > 0) {
        const float height = static_cast<float>(h);
        touch.set_safe_area(
            static_cast<float>(safe.x) / height, static_cast<float>(w - safe.x - safe.w) / height
        );
    }
    // Android's options are in a drawer (debug_overlay.h), opened with the pad's own button.
    touch.set_menu_button(debug_overlay::drawer_available());
    if (touch.take_menu_press()) {
        debug_overlay::toggle_drawer();
    }
    touch_overlay::draw(touch, window_width, window_height);
}

// The cursor as the mods ask for it, while nothing else has the mouse: captured only with the
// window focused and the overlay and the debug menu shut.
void apply_mouse_capture() {
    if (!g || !g->window || g->window->sdl() == nullptr) {
        return;
    }
    SDL_Window* w = g->window->sdl();
    const bool focused = (SDL_GetWindowFlags(w) & SDL_WINDOW_INPUT_FOCUS) != 0;
    const bool capture =
        g->capture_wanted && focused && !debug_overlay::visible() && !debug_menu::is_open();
    if (SDL_GetWindowRelativeMouseMode(w) != capture) {
        SDL_SetWindowRelativeMouseMode(w, capture);
    }
    g->input.set_captured(capture);
}

KeyboardMouseView keyboard_mouse() {
    if (!g) {
        return {};
    }
    KeyboardMouseView view{
        &g->input.keyboard_mouse(), debug_overlay::visible() || debug_menu::is_open()
    };
    SDL_Window* w = g->window ? g->window->sdl() : nullptr;
    if (w == nullptr) {
        return view;
    }
    // The cursor in the window's points, then in its pixels, then over the game's 4:3 picture: the
    // picture's box for the aspect setting, and the 4:3 middle of it where it is wider (where the
    // game's 2D screen, its menus, are drawn).
    float mx = 0.0f;
    float my = 0.0f;
    SDL_GetMouseState(&mx, &my);
    int points_w = 0;
    int points_h = 0;
    SDL_GetWindowSize(w, &points_w, &points_h);
    int window_width = 0;
    int window_height = 0;
    g->window->drawable_size(window_width, window_height);
    if (points_w <= 0 || points_h <= 0 || window_width <= 0 || window_height <= 0) {
        return view;
    }
    const float px = mx * static_cast<float>(window_width) / static_cast<float>(points_w);
    const float py = my * static_cast<float>(window_height) / static_cast<float>(points_h);
    const Box box =
        frame_box(window_width, window_height, picture_aspect(window_width, window_height));
    if (box.width <= 0 || box.height <= 0) {
        return view;
    }
    const float tv_width =
        std::min(static_cast<float>(box.width), static_cast<float>(box.height) * kTvAspect);
    const float tv_x =
        static_cast<float>(box.x) + (static_cast<float>(box.width) - tv_width) / 2.0f;
    view.pointer_x = (px - tv_x) / tv_width;
    view.pointer_y = (py - static_cast<float>(box.y)) / static_cast<float>(box.height);
    view.pointer_inside = SDL_GetMouseFocus() == w && view.pointer_x >= 0.0f
                          && view.pointer_x <= 1.0f && view.pointer_y >= 0.0f
                          && view.pointer_y <= 1.0f;
    return view;
}

void take_keyboard() {
    if (g) {
        g->keyboard_taken = true;
        g->input.set_keyboard_table(false);
    }
}

void capture_mouse(bool capture) {
    if (g) {
        g->capture_wanted = capture;
    }
}

void skip_after_mods(int port, std::uint16_t& buttons) {
    if (port == 0 && settings::get().skip_cutscenes && (buttons & 0x8) == 0) {
        if (openrac_game_skip_buttons != nullptr) {
            buttons = static_cast<std::uint16_t>(buttons & ~(openrac_game_skip_buttons() & 0xFFFF));
        }
        if (openrac_game_skip != nullptr) {
            openrac_game_skip();
        }
    }
}

bool pad(int port, std::uint16_t* buttons, std::uint8_t analog[4]) {
    if (!g || port < 0 || port >= platform::kPadPorts) {
        return false;
    }
    const platform::Pad& p = g->input.pad(port);
    *buttons = p.buttons;
    analog[0] = p.right_x;
    analog[1] = p.right_y;
    analog[2] = p.left_x;
    analog[3] = p.left_y;
    // OPENRAC_PRESS=FRAME:MASK:FRAMES[:LX:LY[:RX:RY]],... holds buttons (mask bits as the pad reports
    // them, 0x4000 cross, 0x8 start) and optionally the left and right sticks (0-255, 128 centred; 0
    // is left or up) for scripted checks. FRAME counts the window's frames (world, 2D and movie frames).
    // cFRAME counts from control instead (Route: the same moment in every version); kPERIOD holds the
    // buttons the first FRAMES of every PERIOD frames while what plays can be skipped and control has
    // not come yet (Start through scenes and the flight), counted on window frames; sPERIOD does the
    // same counted from the frame what plays became skippable, which is the same tick of the scene in
    // every version (k's window frames fall at other ticks of it in PAL and US); SPERIOD as s, before
    // and after control (the scenes and flights a route starts once the player has control).
    static const std::optional<std::string> press = env_string(env::Var::Press);
    if (port == 0 && press) {
        for (const char* at = press->c_str(); at && *at;) {
            unsigned long frame = 0, mask = 0, length = 5;
            unsigned int lx = 128, ly = 128, rx = 128, ry = 128;
            const char kind = *at == 'c' || *at == 'k' || *at == 's' || *at == 'S' ? *at : 0;
            const int read = std::sscanf(
                at + (kind != 0 ? 1 : 0),
                "%lu:%lx:%lu:%u:%u:%u:%u",
                &frame,
                &mask,
                &length,
                &lx,
                &ly,
                &rx,
                &ry
            );
            bool active = false;
            if (kind == 'c') {
                const std::int64_t since = static_cast<std::int64_t>(g->index) - g_route.control;
                active = g_route.control >= 0 && since >= static_cast<std::int64_t>(frame)
                         && since < static_cast<std::int64_t>(frame + length);
            } else if (kind == 'k') {
                active =
                    g_route.control < 0 && frame > 0 && skippable() && g->index % frame < length;
            } else if (kind == 's' || kind == 'S') {
                active = (kind == 'S' || g_route.control < 0) && frame > 0 && skippable()
                         && g_route.skippable_from >= 0
                         && (static_cast<std::int64_t>(g->index) - g_route.skippable_from)
                                    % static_cast<std::int64_t>(frame)
                                < static_cast<std::int64_t>(length);
            } else {
                active = g->index >= frame && g->index < frame + length;
            }
            if (read >= 2 && active) {
                *buttons = static_cast<std::uint16_t>(*buttons & ~mask);
                if (read >= 5) {
                    analog[2] = static_cast<std::uint8_t>(lx);
                    analog[3] = static_cast<std::uint8_t>(ly);
                }
                if (read >= 7) {
                    analog[0] = static_cast<std::uint8_t>(rx);
                    analog[1] = static_cast<std::uint8_t>(ry);
                }
            }
            at = std::strchr(at, ',');
            at = at ? at + 1 : nullptr;
        }
    }
    // Start skips an in-game scene the way the game itself skips it: a pressed Start also presses
    // what the game's own skip of the scene playing reads (rac1: in a cutscene the four shoulders,
    // since the game skips one on Start with L1, L2, R1 and R2 held, Start alone only once it is
    // beaten, ReRAC cutscenes_transitions.md, "skip"; in the ship's landing and take-off Cross).
    // Bits are active low: 0x8 Start, 0x4000 Cross, 0x0F00 L2 R2 L1 R1.
    // With a mod on the pad's reports, after the mods (skip_after_mods), so a Start a mod presses
    // (a key bound to it) skips as the pad's does.
    if (openrac_game_pad_report_hook == nullptr) {
        skip_after_mods(port, *buttons);
    }
    if (port == 0) {
        debug_menu::filter_pad(*buttons, openrac_game_loaded_overlay());
        debug_overlay::pad(*buttons, analog);  // [debug tools]
    }
    if (env::flag(env::Var::TraceInput)) {
        static std::uint64_t last_frame[platform::kPadPorts]{};
        if (g->index >= last_frame[port] + 25) {
            last_frame[port] = g->index;
            log::info(
                "input frame {} port {}: buttons {:#x}, right {} {}, left {} {}",
                g->index,
                port,
                *buttons,
                analog[0],
                analog[1],
                analog[2],
                analog[3]
            );
        }
    }
    return true;
}

}  // namespace openrac::frontend

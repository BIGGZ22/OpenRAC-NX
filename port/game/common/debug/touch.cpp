// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's touch handling (debug/overlay.h): whether the drawer is offered, the drawer's
// opening and shutting, and the fingers it takes as ImGui's pointer, away from the on-screen pad;
// with the drag that scrolls the drawer and the panel.

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>  // ImGui::ClearActiveID: a finger that scrolls lets go of what it pressed

#include <SDL3/SDL.h>

#include "common/env.h"
#include "debug/overlay.h"

namespace openrac::frontend::debug_overlay {

namespace {

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

}  // namespace

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
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
        && ImGui::IsMouseDown(0)
        && (scrolled || std::abs(ImGui::GetMouseDragDelta(0).y) > dp(10.0f))) {
        if (!scrolled) {
            ImGui::ClearActiveID();
        }
        scrolled = true;
        ImGui::SetScrollY(ImGui::GetScrollY() - ImGui::GetIO().MouseDelta.y);
    }
}

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

}  // namespace openrac::frontend::debug_overlay

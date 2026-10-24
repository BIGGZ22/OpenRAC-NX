// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's menu bar (debug/overlay.h): the menus along the window's top on Windows and
// Linux, whose items press the keys the port already answers or move its settings on.

#include <imgui.h>

#include <SDL3/SDL.h>

#include "common/env.h"
#include "debug/overlay.h"
#include "options_menu.h"
#include "renderer/gl.h"
#include "settings.h"

namespace openrac::frontend::debug_overlay {

namespace {

// The menus along the window's top, as OpenGOAL has them: on Windows and Linux. macOS has its own
// menu bar, with the same menus (platform/macos_app.mm).
#if defined(__APPLE__)
constexpr bool kMenuBar = false;
#else
constexpr bool kMenuBar = true;
#endif

}  // namespace

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

}  // namespace openrac::frontend::debug_overlay

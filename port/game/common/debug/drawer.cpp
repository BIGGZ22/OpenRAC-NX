// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's drawer (debug/overlay.h): the menus on a touch screen, a panel that slides
// out from the left with the port's settings, the enhanced graphics, the fixes and the developer
// tools on cards, drawn in the launcher's colours with rows as high as a thumb.

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <format>
#include <imgui.h>
#include <string>

#include <SDL3/SDL.h>

#include "debug/overlay.h"
#include "fixes.h"
#include "options_menu.h"
#include "renderer/gl.h"
#include "settings.h"

namespace openrac::frontend::debug_overlay {

namespace {

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

}  // namespace

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

}  // namespace openrac::frontend::debug_overlay

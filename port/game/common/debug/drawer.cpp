// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's drawer (debug/overlay.h): the menus on a touch screen, a panel that slides
// out from the left with the port's settings, the enhanced graphics, the touch controls, the fixes
// and the developer tools on cards, drawn in the launcher's colours (ui_colors.h) with rows as high
// as a thumb; and the bar shown while the on-screen pad's layout is edited.

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <format>
#include <imgui.h>
#include <imgui_internal.h>  // ImHashStr, ShadeVertsLinearColorGradientKeepAlpha: the eases and gradients
#include <span>
#include <string>

#include <SDL3/SDL.h>

#include "debug/overlay.h"
#include "fixes.h"
#include "frontend.h"
#include "options_menu.h"
#include "renderer/gl.h"
#include "settings.h"
#include "ui_colors.h"

namespace openrac::frontend::debug_overlay {

namespace {

// The launcher's colours: its theme, as the launcher started the game with it (ui_colors.h), so
// that the drawer looks like the rest of the app (launcher/docs/STYLE.md).
const ui_colors::Colors& ui() {
    return ui_colors::launcher();
}

// A row or button held down: the launcher's accent, faint over the card.
ImU32 pressed_fill() {
    return ui_colors::mix(ui().panel_hi, ui().amber, 0.16f);
}

// The launcher's transitions: what changes (a press, a switch, the focus) eases to its new state
// over `seconds`, nearly all the way, rather than jumping; `id` is ImGui's, one value each.
float tween(ImGuiID id, float target, float seconds) {
    auto [at, made] = s.tweens.try_emplace(id, target);
    float& v = at->second;
    if (!made) {
        v += (target - v) * (1.0f - std::exp(-ImGui::GetIO().DeltaTime * 3.0f / seconds));
        if (std::abs(target - v) < 0.002f) {
            v = target;
        }
    }
    return v;
}

// One of an item's parts that eases on its own (its press, its focus, a switch's knob).
ImGuiID item_part(const char* part) {
    return ImHashStr(part, 0, ImGui::GetItemID());
}

constexpr float kFast = 0.15f;    // --fast
constexpr float kMedium = 0.28f;  // --medium

// The focus ring a gamepad moves, around the item just drawn, easing in as it arrives.
void focus_ring(ImDrawList* list, ImVec2 a, ImVec2 b, float round) {
    const bool focused = ImGui::IsItemFocused() && ImGui::GetIO().NavVisible;
    const float shown = tween(item_part("focus"), focused ? 1.0f : 0.0f, kFast);
    if (shown > 0.0f) {
        list->AddRect(a, b, ui_colors::fade(ui().amber, shown), round, 0, dp(2.0f));
    }
}

// The launcher's main button: its gradient, a highlight along the top, its ink; darker while held.
void primary_fill(ImDrawList* list, ImVec2 a, ImVec2 b, float round, float held) {
    const int first = list->VtxBuffer.Size;
    list->AddRectFilled(a, b, IM_COL32_WHITE, round);
    const ImU32 dark = ui_colors::mix(ui().bar_bottom, ui().ink_deep, 0.18f * held);
    ImGui::ShadeVertsLinearColorGradientKeepAlpha(
        list,
        first,
        list->VtxBuffer.Size,
        ImVec2(a.x, a.y),
        ImVec2(a.x, b.y),
        ui_colors::mix(ui().bar_top, ui().ink_deep, 0.18f * held),
        dark
    );
    list->AddLine(
        ImVec2(a.x + round, a.y + 1.0f),
        ImVec2(b.x - round, a.y + 1.0f),
        IM_COL32(255, 255, 255, 90)
    );
}

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
    list->AddRectFilled(s.card.top, end, ui().panel_hi, dp(16.0f));
    list->AddRect(s.card.top, end, ui().line, dp(16.0f));
    list->ChannelsMerge();
}

// A section's heading above its card, as the launcher's Settings name theirs: in its accent's gold.
void drawer_section(const char* title) {
    ImGui::Dummy(ImVec2(0.0f, dp(18.0f)));
    ImFont* font = ImGui::GetFont();
    const float size = ImGui::GetFontSize() * 1.05f;
    const ImVec2 at = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()
        ->AddText(font, size, ImVec2(at.x + dp(4.0f), at.y), ui().gold, title);
    ImGui::Dummy(ImVec2(0.0f, size + dp(8.0f)));
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
        list->AddLine(
            ImVec2(at.x + dp(16.0f), at.y), ImVec2(at.x + width - dp(16.0f), at.y), ui().line
        );
    }
    ImGui::PushID(label);
    const bool pressed = ImGui::InvisibleButton("row", ImVec2(width, height));
    ImGui::PopID();
    const ImVec2 a(at.x + dp(3.0f), at.y + dp(3.0f));
    const ImVec2 b(at.x + width - dp(3.0f), at.y + height - dp(3.0f));
    const float held = tween(item_part("held"), ImGui::IsItemActive() ? 1.0f : 0.0f, kFast);
    if (held > 0.0f) {
        list->AddRectFilled(a, b, ui_colors::fade(pressed_fill(), held), dp(10.0f));
    }
    focus_ring(list, a, b, dp(10.0f));
    list->AddText(
        ImVec2(at.x + dp(16.0f), at.y + (height - ImGui::GetFontSize()) * 0.5f), ink, label
    );
    right(list, ImVec2(at.x + width - dp(16.0f), at.y + height * 0.5f));
    return pressed;
}

bool drawer_row(const char* label, ImU32 ink = ui().text) {
    return drawer_row(label, ink, [](ImDrawList*, ImVec2) {});
}

// An arrow as the launcher draws one: two strokes meeting at `tip`, pointing `dx` (-1 left, 1
// right) or, with `dx` 0, down (`dy` 1) or up (`dy` -1).
void arrow(ImDrawList* list, ImVec2 tip, float dx, float dy, ImU32 ink) {
    const float arm = dp(5.0f);
    const ImVec2 back(tip.x - dx * arm, tip.y - dy * arm);
    const ImVec2 points[3] =
        {ImVec2(back.x - dy * arm, back.y - dx * arm),
         tip,
         ImVec2(back.x + dy * arm, back.y + dx * arm)};
    list->AddPolyline(points, 3, ink, ImDrawFlags_None, dp(2.0f));
}

// A row that turns something on and off, its switch at the right: the launcher's, its knob
// sliding across and its track filling as it turns on.
bool drawer_switch(const char* label, bool on) {
    return drawer_row(label, ui().text, [on](ImDrawList* list, ImVec2 right) {
        const float k = tween(item_part("switch"), on ? 1.0f : 0.0f, kMedium);
        const float h = dp(26.0f);
        const ImVec2 a(right.x - dp(46.0f), right.y - h * 0.5f);
        const ImVec2 b(right.x, right.y + h * 0.5f);
        list->AddRectFilled(a, b, ui_colors::mix(ui().ink_deep, ui().brand, k), h * 0.5f);
        list->AddRect(a, b, ui_colors::mix(ui().line_strong, ui().brand, k), h * 0.5f);
        const float knob = h * 0.5f - dp(4.0f);
        const float from = a.x + h * 0.5f;
        const float to = b.x - h * 0.5f;
        list->AddCircleFilled(
            ImVec2(from + (to - from) * k, right.y), knob, ui_colors::mix(ui().dim, ui().text, k)
        );
    });
}

// A row with its value between two arrows, as a console game's options: -1 or 1 when it was moved
// back or on. A touch on the value's left part moves it back, anywhere else on; a gamepad's or the
// keyboard's left and right move it while it has the focus, and A moves it on.
int drawer_choice(
    const char* label, const std::string& value, bool can_lower = true, bool can_raise = true
) {
    float back_until = 0.0f;  // a touch left of this moves the value back
    const bool pressed = drawer_row(label, ui().text, [&](ImDrawList* list, ImVec2 right) {
        // The values of every row in one column, centred between its arrows.
        const float column = std::min(s.card.width * 0.48f, dp(168.0f));
        const float left = right.x - column;
        const ImU32 off = ui_colors::fade(ui().dim, 0.35f);
        arrow(list, ImVec2(left, right.y), -1.0f, 0.0f, can_lower ? ui().amber : off);
        arrow(list, ImVec2(right.x, right.y), 1.0f, 0.0f, can_raise ? ui().amber : off);
        const ImVec2 size = ImGui::CalcTextSize(value.c_str());
        list->AddText(
            ImVec2(left + (column - size.x) * 0.5f, right.y - size.y * 0.5f),
            ui().gold,
            value.c_str()
        );
        back_until = left + column * 0.35f;
    });
    int by = 0;
    if (pressed) {
        by = ImGui::IsMouseReleased(0) && ImGui::GetIO().MousePos.x < back_until ? -1 : 1;
    }
    if (ImGui::IsItemFocused()) {
        const auto held = [](ImGuiKey pad, ImGuiKey stick, ImGuiKey key) {
            return ImGui::IsKeyPressed(pad) || ImGui::IsKeyPressed(stick)
                   || ImGui::IsKeyPressed(key);
        };
        if (held(ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadLStickLeft, ImGuiKey_LeftArrow)) {
            by = -1;
        } else if (held(
                       ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadLStickRight, ImGuiKey_RightArrow
                   )) {
            by = 1;
        }
    }
    return (by < 0 && !can_lower) || (by > 0 && !can_raise) ? 0 : by;
}

// A percentage the player picks from `list` (settings.h), as a choice row: true when it changed.
bool drawer_listed(const char* label, int& value, std::span<const int> list) {
    const int by =
        drawer_choice(label, std::format("{}%", value), value > list.front(), value < list.back());
    if (by == 0) {
        return false;
    }
    value = settings::step_listed(list, value, by);
    return true;
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
    primary_fill(
        list, a, b, round, tween(item_part("held"), ImGui::IsItemActive() ? 1.0f : 0.0f, kFast)
    );
    focus_ring(
        list,
        ImVec2(a.x - dp(3.0f), a.y - dp(3.0f)),
        ImVec2(b.x + dp(3.0f), b.y + dp(3.0f)),
        round + dp(3.0f)
    );
    const ImVec2 size = ImGui::CalcTextSize(label);
    list->AddText(
        ImVec2((a.x + b.x - size.x) * 0.5f, a.y + (height - size.y) * 0.5f), ui().bar_ink, label
    );
    return pressed;
}

// A line of the card open now, small and dim, in place of rows that cannot be used yet.
void drawer_note(const char* line) {
    ImGui::Dummy(ImVec2(0.0f, dp(10.0f)));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + dp(16.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(ui().soft));
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + s.card.width - dp(32.0f));
    ImGui::TextUnformatted(line);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, dp(10.0f)));
    ++s.card.rows;
}

// Enhanced graphics (port/enhanced, the launcher's Mods entry) as the drawer's own rows: the
// switches and the two values a player changes, large enough for a thumb and moved through by a
// gamepad. The diagnostics (probe spheres, receiver biases, GTAO's directions and steps) are the
// debug overlay's and the desktop's Enhanced menu's. When the game cannot have it, the card says
// why and what to do instead of leaving the player to guess.
void drawer_enhanced() {
#if defined(OPENRAC_ENHANCED_RENDERING) && OPENRAC_ENHANCED_RENDERING
    auto& set = settings::get();
    drawer_section("Enhanced graphics");
    card_begin();
    if (!gl::enhanced_rendering_enabled()) {
        drawer_note(
            "Off. Switch it on under Mods in the launcher: bounced light, modern shadows and "
            "ambient occlusion, from the game's next start."
        );
        card_end();
        return;
    }
    if (!gl::enhanced_rendering_available()) {
        drawer_note(
            "It needs the Vulkan renderer, and this run draws with OpenGL. Vulkan 1.3 could not "
            "start on this device or driver."
        );
        card_end();
        return;
    }
    if (!enhanced_graphics_available()) {
        drawer_note(
            "This level's lighting is not prepared yet. In the launcher, Mods, Enhanced graphics "
            "prepares it from your levels; the game shows the original lighting until then."
        );
        card_end();
        return;
    }
    bool changed = false;
    if (drawer_switch("Enhanced graphics", set.enhanced_graphics)) {
        set.enhanced_graphics = !set.enhanced_graphics;
        changed = true;
    }
    if (set.enhanced_graphics) {
        if (drawer_switch("Modern shadows", set.enhanced_shadows)) {
            set.enhanced_shadows = !set.enhanced_shadows;
            changed = true;
        }
        if (drawer_switch("Bounced light", set.probe_lighting)) {
            set.probe_lighting = !set.probe_lighting;
            changed = true;
        }
        if (set.probe_lighting) {
            const float gi = set.probe_gi_multiplier;
            if (const int by = drawer_choice(
                    "Bounce strength", std::format("{:g}x", gi), gi > 0.05f, gi < 4.95f
                )) {
                set.probe_gi_multiplier = std::clamp(
                    std::round((gi + 0.25f * static_cast<float>(by)) * 4.0f) / 4.0f, 0.0f, 5.0f
                );
                changed = true;
            }
        }
        if (drawer_switch("Ambient occlusion", set.gtao)) {
            set.gtao = !set.gtao;
            changed = true;
        }
        if (set.gtao) {
            auto& gtao = set.gtao_settings;
            const bool quality = gtao.quality == renderer::GtaoQuality::Quality;
            if (drawer_choice(
                    "Occlusion quality", quality ? "Quality" : "Performance", quality, !quality
                )) {
                gtao.quality =
                    quality ? renderer::GtaoQuality::Performance : renderer::GtaoQuality::Quality;
                changed = true;
            }
        }
    }
    card_end();
    if (changed) {
        settings::save();
        s.settings_changed = true;
    }
#endif
}

}  // namespace

void drawer(const Shown& shown) {
    const ImGuiIO& io = ImGui::GetIO();
    // Out in the launcher's --medium, back in a little quicker, as its pages come and go.
    const float target = s.drawer_open ? 1.0f : 0.0f;
    const float move = io.DeltaTime / (s.drawer_open ? kMedium : 0.2f);
    s.drawer_at = s.drawer_at < target ? std::min(s.drawer_at + move, target)
                                       : std::max(s.drawer_at - move, target);
    if (s.drawer_at <= 0.0f) {
        if (s.shot_when_shut) {
            s.shot_when_shut = false;
            s.shot_path = shot_name(s.tick);
        }
        return;
    }
    // The launcher's --ease, cubic-bezier(0.2, 0.8, 0.2, 1): quick out of the edge, settling slowly.
    const float ease = 1.0f - std::pow(1.0f - s.drawer_at, 4.0f);
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // The left of the screen a camera's notch keeps.
    float inset = 0.0f;
    SDL_Rect safe{};
    if (shown.window != nullptr && SDL_GetWindowSafeArea(shown.window, &safe)) {
        inset = static_cast<float>(safe.x);
    }
    const float width = std::min(viewport->Size.x * 0.6f, dp(360.0f)) + inset;
    const float x = viewport->Pos.x - width * (1.0f - ease);
    ImGui::GetBackgroundDrawList()->AddRectFilled(
        viewport->Pos,
        ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
        ui_colors::fade(ui().ink_deep, 0.6f * ease)
    );
    // A touch beside it shuts it, as does the gamepad's B.
    if (s.drawer_open && ImGui::IsMouseReleased(0) && !s.drawer_scroll.dragged
        && io.MouseClickedPos[0].x > x + width) {
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
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImGui::ColorConvertU32ToFloat4(ui().line_strong));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                                   | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("OpenRAC options", nullptr, flags)) {
        drag_scroll(s.drawer_scroll);
        {
            ImDrawList* back = ImGui::GetWindowDrawList();
            const ImVec2 top(x, viewport->Pos.y);
            const ImVec2 bottom(x + width, viewport->Pos.y + viewport->Size.y);
            back->PushClipRect(top, bottom, false);
            back->AddRectFilled(top, bottom, ui().panel, dp(16.0f), ImDrawFlags_RoundCornersRight);
            back->AddRect(top, bottom, ui().line, dp(16.0f), ImDrawFlags_RoundCornersRight);
            back->PopClipRect();
        }
        {
            // The header: the wordmark, and what the game is doing while the drawer is out.
            ImDrawList* front = ImGui::GetForegroundDrawList();
            const ImVec2 top(x, viewport->Pos.y);
            const ImVec2 bottom(x + width, viewport->Pos.y + header);
            front->AddRectFilled(
                top, bottom, ui().panel, dp(16.0f), ImDrawFlags_RoundCornersTopRight
            );
            // A hairline under it once rows have scrolled beneath, fading in as they do.
            const float under = std::min(ImGui::GetScrollY() / dp(24.0f), 1.0f);
            if (under > 0.0f) {
                front->AddLine(ImVec2(top.x, bottom.y), bottom, ui_colors::fade(ui().line, under));
            }
            ImFont* font = ImGui::GetFont();
            const float size = ImGui::GetFontSize() * 1.2f;
            const float text_y = top.y + (header - size) * 0.5f;
            const float text_x = top.x + inset + dp(18.0f);
            const float open = font->CalcTextSizeA(size, FLT_MAX, 0.0f, "Open").x;
            front->AddText(font, size, ImVec2(text_x, text_y), ui().text, "Open");
            front->AddText(font, size, ImVec2(text_x + open, text_y), ui().amber, "RAC");
            const char* state = s.keep_paused ? "Paused" : "Paused while open";
            const ImVec2 state_size = ImGui::CalcTextSize(state);
            const ImVec2 pill_b(
                bottom.x - dp(16.0f), top.y + header * 0.5f + state_size.y * 0.5f + dp(4.0f)
            );
            const ImVec2 pill_a(
                pill_b.x - state_size.x - dp(20.0f), pill_b.y - state_size.y - dp(8.0f)
            );
            // A chip, as the launcher's: its deepest ink, a hairline, soft text.
            front->AddRectFilled(pill_a, pill_b, ui().ink_deep, (pill_b.y - pill_a.y) * 0.5f);
            front->AddRect(pill_a, pill_b, ui().line, (pill_b.y - pill_a.y) * 0.5f);
            front->AddText(ImVec2(pill_a.x + dp(10.0f), pill_a.y + dp(4.0f)), ui().soft, state);
        }

        if (drawer_button("Resume")) {
            set_drawer(false);
        }
        ImGui::SetItemDefaultFocus();  // where a gamepad starts

        drawer_section("Graphics");
        card_begin();
        setting_row(options_menu::kAspect);
        if (settings::get().aspect != settings::Aspect::Standard) {
            setting_row(
                options_menu::kHud
            );  // the HUD at the 4:3 picture's middle, or out at the edges
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

        drawer_enhanced();

        drawer_section("Sound and game");
        card_begin();
        const int volume = settings::get().volume;
        if (const int by =
                drawer_choice("Volume", std::format("{}%", volume), volume > 0, volume < 100)) {
            settings::set_volume(volume + by * 10);
            settings::save();
        }
        if (drawer_switch("Skip cutscenes", settings::get().skip_cutscenes)) {
            change_setting(options_menu::kSkipCutscenes, 1);
        }
        card_end();

        // The on-screen pad (platform/touch_pad.h): its size, the camera's speed, and where its
        // controls are, which the player drags about (edit_touch).
        drawer_section("Touch controls");
        card_begin();
        auto& set = settings::get();
        if (drawer_listed("Button size", set.touch_scale, settings::kTouchScales)) {
            settings::save();
        }
        if (drawer_listed("Camera speed", set.touch_camera, settings::kTouchCameraSpeeds)) {
            settings::save();
        }
        if (drawer_row("Move the controls")) {
            edit_touch(true);
        }
        if (!set.touch_layout.empty() && drawer_row("Put them back")) {
            set.touch_layout.clear();
            settings::save();
            s.touch_reset = true;
        }
        card_end();

        // What the port changes from the console that the player may switch back (fixes.h): on is
        // the port's fix, off the console's behaviour. The launcher's page for the game says more.
        if (fixes::count() > 0) {
            drawer_section("Game fixes");
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
        if (drawer_row("Developer tools", ui().text, [developer](ImDrawList* list, ImVec2 right) {
                // The arrow turns as the section opens.
                const float k = tween(item_part("open"), developer ? 1.0f : 0.0f, kMedium);
                const float dy = 1.0f - 2.0f * k;
                arrow(
                    list, ImVec2(right.x - dp(5.0f), right.y + dp(2.5f) * dy), 0.0f, dy, ui().soft
                );
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
            const std::string speed =
                s.stretch == 1.0 ? "Full" : std::format("1/{}", static_cast<int>(s.stretch));
            if (const int by = drawer_choice("Speed", speed, s.stretch < 16.0, s.stretch > 1.0)) {
                s.stretch =
                    by < 0 ? std::min(s.stretch * 2.0, 16.0) : std::max(s.stretch / 2.0, 1.0);
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
        if (drawer_row("Quit game", ui().err)) {
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
                const auto* name = reinterpret_cast<const char*>(
                    openrac::gl::glGetString(openrac::gl::GL_RENDERER)
                );
                const char* api = s.gl_subset            ? "Vulkan"
                                  : openrac::gl::is_es() ? "OpenGL ES"
                                                         : "OpenGL";
                return std::format("{} · {}", api, name != nullptr ? name : "unknown GPU");
            }();
            ImGui::Dummy(ImVec2(0.0f, dp(14.0f)));
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(ui().dim));
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

namespace {

// A button of the editing bar, the launcher's main one or a plain one: true when pressed.
bool bar_button(const char* label, float width, bool primary = false) {
    const float height = dp(44.0f);
    const ImVec2 a = ImGui::GetCursorScreenPos();
    const ImVec2 b(a.x + width, a.y + height);
    const bool pressed = ImGui::InvisibleButton(label, ImVec2(width, height));
    ImDrawList* list = ImGui::GetWindowDrawList();
    const float round = dp(12.0f);
    const float held = tween(item_part("held"), ImGui::IsItemActive() ? 1.0f : 0.0f, kFast);
    if (primary) {
        primary_fill(list, a, b, round, held);
    } else {
        list->AddRectFilled(a, b, ui_colors::mix(ui().panel_hi, pressed_fill(), held), round);
        list->AddRect(a, b, ui().line_strong, round);
    }
    focus_ring(
        list,
        ImVec2(a.x - dp(3.0f), a.y - dp(3.0f)),
        ImVec2(b.x + dp(3.0f), b.y + dp(3.0f)),
        round + dp(3.0f)
    );
    const ImVec2 size = ImGui::CalcTextSize(label, nullptr, true);
    list->AddText(
        ImVec2((a.x + b.x - size.x) * 0.5f, a.y + (height - size.y) * 0.5f),
        primary ? ui().bar_ink : ui().text,
        label,
        ImGui::FindRenderedTextEnd(label)
    );
    return pressed;
}

}  // namespace

// While the on-screen pad's layout is edited: a bar in the middle of the screen, clear of the
// controls' places, to make them smaller or larger, put them back, and finish.
void touch_edit_bar() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float gap = dp(8.0f);
    const float button = dp(52.0f);
    const float wide = dp(96.0f);
    const float width = button * 2.0f + wide * 3.0f + gap * 4.0f + dp(28.0f);
    ImGui::SetNextWindowPos(
        ImVec2(
            viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.42f
        ),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, dp(16.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(dp(14.0f), dp(12.0f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, dp(10.0f)));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(ui().panel));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(ui().line));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                                   | ImGuiWindowFlags_NoSavedSettings
                                   | ImGuiWindowFlags_AlwaysAutoResize;
    // The lines the dragged group snapped to, in the accent, under the bar.
    ImDrawList* guides = ImGui::GetBackgroundDrawList();
    const ImU32 guide = ui_colors::fade(ui().amber, 0.7f);
    if (s.touch_guides.x >= 0.0f) {
        const float x = viewport->Pos.x + viewport->Size.x * s.touch_guides.x;
        guides->AddLine(
            ImVec2(x, viewport->Pos.y),
            ImVec2(x, viewport->Pos.y + viewport->Size.y),
            guide,
            dp(1.5f)
        );
    }
    if (s.touch_guides.y >= 0.0f) {
        const float y = viewport->Pos.y + viewport->Size.y * s.touch_guides.y;
        guides->AddLine(
            ImVec2(viewport->Pos.x, y),
            ImVec2(viewport->Pos.x + viewport->Size.x, y),
            guide,
            dp(1.5f)
        );
    }
    if (ImGui::Begin("Move the controls", nullptr, flags)) {
        auto& set = settings::get();
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(ui().soft));
        ImGui::TextUnformatted("Drag a control to move it. The game is paused.");
        ImGui::PopStyleColor();
        bool resized = false;
        if (bar_button("-##smaller", button)) {
            set.touch_scale = settings::step_listed(settings::kTouchScales, set.touch_scale, -1);
            resized = true;
        }
        ImGui::SameLine();
        const std::string size = std::format("{}%##size", set.touch_scale);
        bar_button(size.c_str(), wide);
        ImGui::SameLine();
        if (bar_button("+##larger", button)) {
            set.touch_scale = settings::step_listed(settings::kTouchScales, set.touch_scale, 1);
            resized = true;
        }
        ImGui::SameLine();
        if (bar_button("Reset", wide)) {
            s.touch_reset = true;
        }
        ImGui::SameLine();
        if (bar_button("Done", wide, true)) {
            edit_touch(false);
        }
        if (resized) {
            settings::save();
        }
        s.bar_min = ImGui::GetWindowPos();
        s.bar_max =
            ImVec2(s.bar_min.x + ImGui::GetWindowSize().x, s.bar_min.y + ImGui::GetWindowSize().y);
    }
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

}  // namespace openrac::frontend::debug_overlay

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the debug overlay draws over the game's picture (debug/overlay.h): the selected moby's
// marker, the mobys' labels and the click that picks one; and the status badge, which says the
// game is paused or slowed and shows the toasts and the notice.

#include <cstdint>
#include <format>
#include <imgui.h>
#include <string>

#include "debug/overlay.h"
#include "renderer/math.h"

namespace openrac::frontend::debug_overlay {

namespace {

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

}  // namespace

void picture_marks(const Shown& shown) {
    if (shown.camera == nullptr || shown.box_width <= 0
        || (!s.labels && !s.pick && s.selected == 0)) {
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
    const bool clicked =
        s.pick && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.WantCaptureMouse;
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
            const std::string label = name.empty() ? std::format("{}", m.class_id)
                                                   : std::format("{} {}", m.class_id, name);
            draw->AddCircleFilled(at, 2.5f, IM_COL32(255, 255, 255, 220));
            draw->AddText(ImVec2(at.x + 7.0f, at.y - 5.0f), IM_COL32(0, 0, 0, 255), label.c_str());
            draw->AddText(
                ImVec2(at.x + 6.0f, at.y - 6.0f),
                selected ? IM_COL32(255, 230, 80, 255) : IM_COL32(255, 255, 255, 255),
                label.c_str()
            );
        }
    }
    if (picked != 0) {
        s.follow_hero = false;
        s.selected = picked;
        s.inspector = true;
    }
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
        line =
            keys ? std::format("SPEED 1/{}    F8 faster    F7 slower", static_cast<int>(s.stretch))
                 : std::format("SPEED 1/{}", static_cast<int>(s.stretch));
    }
    if (line.empty() && s.toast.empty() && s.notice.empty()) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f, viewport->WorkPos.y + 10.0f),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.0f)
    );
    ImGui::SetNextWindowBgAlpha(0.82f);
    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
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

}  // namespace openrac::frontend::debug_overlay

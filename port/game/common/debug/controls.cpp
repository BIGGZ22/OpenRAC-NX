// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's settings controls (debug/overlay.h), which the drawer and the menu bar share:
// a setting moved on, and the enhanced graphics' switches and sliders with GTAO's.

#include <imgui.h>

#include "debug/overlay.h"
#include "frontend.h"
#include "options_menu.h"
#include "settings.h"

namespace openrac::frontend::debug_overlay {

namespace {

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

}  // namespace

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

}  // namespace openrac::frontend::debug_overlay

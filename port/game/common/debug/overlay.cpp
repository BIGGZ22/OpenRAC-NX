// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's state and the helpers its panels share (debug/overlay.h).

#include "debug/overlay.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <format>
#include <imgui.h>
#include <string>
#include <unordered_map>

#include "openrac/guest.h"

// The editor's moby class names (cmake/Games.cmake, from editor/moby_classes.tsv).
extern const char* const openrac_moby_class_names;

namespace openrac::frontend::debug_overlay {

State s;

float dp(float v) {
    return v * s.ui_scale;
}

const ImVec4 kRed{1.0f, 0.35f, 0.3f, 1.0f};
const ImVec4 kGreen{0.45f, 0.9f, 0.45f, 1.0f};
const ImVec4 kDim{0.6f, 0.6f, 0.65f, 1.0f};
const ImVec4 kYellow{1.0f, 0.9f, 0.35f, 1.0f};

// ---- Reading the game's memory ----

bool in_ram(std::uint32_t address, std::uint32_t size) {
    address &= 0x01FFFFFF;
    return static_cast<std::size_t>(address) + size <= s.ram.size();
}


const std::unordered_map<int, std::string>& class_names() {
    static const std::unordered_map<int, std::string> names = [] {
        std::unordered_map<int, std::string> map;
        const char* at = openrac_moby_class_names;
        while (at != nullptr && *at != '\0') {
            const char* end = std::strchr(at, '\n');
            const std::string line(at, end != nullptr ? static_cast<std::size_t>(end - at) : std::strlen(at));
            const std::size_t tab = line.find('\t');
            if (!line.empty() && line[0] != '#' && tab != std::string::npos) {
                map[std::atoi(line.c_str())] = line.substr(tab + 1);
            }
            at = end != nullptr ? end + 1 : nullptr;
        }
        return map;
    }();
    return names;
}

std::string class_name(int id) {
    const auto found = class_names().find(id);
    return found != class_names().end() ? found->second : std::string();
}

std::string function_name(std::uint32_t address) {
    if (address == 0) {
        return "none";
    }
    const char* name = openrac_guest_function_name(address);
    return name != nullptr ? std::format("{:#x} {}", address, name) : std::format("{:#x} (no C)", address);
}

const viewer::LiveMoby* hero() {
    for (const viewer::LiveMoby& m : s.state.mobys) {
        if (m.class_id == 0) {
            return &m;
        }
    }
    return nullptr;
}

const viewer::LiveMoby* selected_moby() {
    for (const viewer::LiveMoby& m : s.state.mobys) {
        if (m.address == s.selected) {
            return &m;
        }
    }
    return nullptr;
}

float distance(const viewer::LiveMoby& a, const viewer::LiveMoby& b) {
    const float dx = a.matrix[12] - b.matrix[12], dy = a.matrix[13] - b.matrix[13], dz = a.matrix[14] - b.matrix[14];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

std::string megabytes(std::uint64_t bytes) {
    return std::format("{:.1f} MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
}

void row(const char* label, const std::string& value) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(value.c_str());
}

}  // namespace openrac::frontend::debug_overlay

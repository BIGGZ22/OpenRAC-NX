// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's game section: the level, the camera, the pad, the functions without C that
// were called, and the probes (debug/overlay.h).

#include <cstddef>
#include <cstdint>
#include <format>
#include <imgui.h>
#include <string>

#include "common/stats.h"
#include "debug/overlay.h"
#include "openrac/guest.h"

namespace openrac::frontend::debug_overlay {

void game_section() {
    const std::string level =
        s.level >= 0 && s.level < 19
            ? std::format("{} {}", s.level, kLevelNames[static_cast<std::size_t>(s.level)])
            : std::string("none (the boot program: title, menus)");
    text("{}   level {}", s.game, level);
    text("tick {}   game mode {}", s.tick, read<std::int32_t>(0x0015F6E8));

    if (const viewer::LiveMoby* h = hero()) {
        text(
            "hero {:.2f} {:.2f} {:.2f}   state {}",
            h->matrix[12],
            h->matrix[13],
            h->matrix[14],
            read<std::uint8_t>(h->address + 0x20)
        );
    } else {
        ImGui::TextDisabled("no hero moby");
    }

    const viewer::GameState& g = s.state;
    text(
        "camera {:.2f} {:.2f} {:.2f}",
        g.camera_position[0],
        g.camera_position[1],
        g.camera_position[2]
    );
    text(
        "  forward {:.3f} {:.3f} {:.3f}   tan fov {:.3f} x {:.3f}",
        g.forward[0],
        g.forward[1],
        g.forward[2],
        g.tan_half_fov_x,
        g.tan_half_fov_y
    );
    text(
        "  fog {:.0f} {:.0f} {:.0f} from {:.0f} to {:.0f}",
        g.fog_colour[0] * 255.0f,
        g.fog_colour[1] * 255.0f,
        g.fog_colour[2] * 255.0f,
        g.fog_near,
        g.fog_far
    );
    std::string held;

    for (std::size_t bit = 0; bit < kButtonNames.size(); ++bit) {
        if ((s.buttons & (1u << bit)) == 0) {
            held += (held.empty() ? "" : " ") + std::string(kButtonNames[bit]);
        }
    }

    text("pad {:#06x} {}", s.buttons, held.empty() ? "(nothing held)" : held);
    text(
        "  left stick {} {}   right stick {} {}", s.analog[2], s.analog[3], s.analog[0], s.analog[1]
    );
    unsigned functions = 0;
    const unsigned long calls = openrac_guest_missing_calls(&functions);

    if (functions == 0) {
        text_coloured(kGreen, "no function without C was called");
    } else {
        text_coloured(kRed, "{} calls to {} functions without C", calls, functions);

        if (ImGui::TreeNode("the functions")) {
            unsigned long count = 0;

            for (unsigned i = 0; i < 64; ++i) {
                const char* name = openrac_guest_missing_at(i, &count);

                if (name == nullptr) {
                    break;
                }

                text("{:8} {}", count, name);
            }

            ImGui::TreePop();
        }
    }

    text_coloured(kDim, "a fault in the game's code ends the program: its report is in the log");

    if (ImGui::TreeNode("probes (the OPENRAC_* switches)")) {
        for (stats::Probe* p : stats::probes()) {
            ImGui::Checkbox(p->variable, &p->on);

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", p->what);
            }
        }

        if (stats::probes().empty()) {
            ImGui::TextDisabled("none read yet");
        }

        ImGui::TreePop();
    }
}

}  // namespace openrac::frontend::debug_overlay

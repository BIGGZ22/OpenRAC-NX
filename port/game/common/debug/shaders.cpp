// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's shaders section: the GL programs, their state, and reloading them from their
// source folder (debug/overlay.h).

#include <format>
#include <imgui.h>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "common/env.h"
#include "common/stats.h"
#include "debug/overlay.h"
#include "renderer/gl_stats.h"

namespace openrac::frontend::debug_overlay {

std::string shader_root() {
    const std::optional<std::string_view> named = env::value(env::Var::ShaderDir);
    return named ? std::string(*named) : stats::shader_root();
}

void shaders_section() {
    const std::string root = shader_root();
    const std::vector<renderer::gl_stats::Program> programs = renderer::gl_stats::programs();

    if (ImGui::Button("reload all from disk")) {
        std::string message;
        const int done = renderer::gl_stats::reload_all(root, message);
        s.message =
            std::format("{} programs reloaded{}", done, message.empty() ? "" : "\n" + message);
    }

    ImGui::SameLine();
    text_coloured(kDim, "{}", root.empty() ? "no source folder" : root);

    if (ImGui::BeginTable("programs", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        for (const char* column : {"id", "program", "status", "binds", ""}) {
            ImGui::TableSetupColumn(column);
        }

        ImGui::TableHeadersRow();

        for (const renderer::gl_stats::Program& p : programs) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            text("{}", p.id);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(p.name.c_str());

            if (ImGui::IsItemHovered() && !p.vertex_file.empty()) {
                ImGui::SetTooltip("%s\n%s", p.vertex_file.c_str(), p.fragment_file.c_str());
            }

            ImGui::TableNextColumn();

            if (p.linked) {
                text_coloured(
                    kGreen, "linked{}", p.reload_status.empty() ? "" : ", " + p.reload_status
                );
            } else {
                text_coloured(kRed, "NOT linked");
            }

            ImGui::TableNextColumn();
            text("{} ({} in all)", p.binds, p.binds_total);
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(p.id));

            if (!p.vertex_file.empty() && !p.fragment_file.empty()
                && ImGui::SmallButton("reload")) {
                renderer::gl_stats::reload(p.id, root, s.message);
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    const std::vector<std::string>& failures = renderer::gl_stats::failures();

    if (failures.empty()) {
        text_coloured(kGreen, "every shader compiled and every program linked");
    } else {
        text_coloured(kRed, "{} failures:", failures.size());

        for (const std::string& f : failures) {
            ImGui::TextWrapped("%s", f.c_str());
        }
    }
}

}  // namespace openrac::frontend::debug_overlay

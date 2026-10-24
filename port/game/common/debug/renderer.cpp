// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's renderer section: the GPU, the frame's counters and the renderer's passes,
// each of which it can hide (debug/overlay.h).

#include "renderer/renderer.h"

#include <algorithm>
#include <cstdint>
#include <format>
#include <imgui.h>
#include <string>
#include <utility>
#include <vector>

#include "common/stats.h"
#include "debug/overlay.h"
#include "renderer/gl_stats.h"

namespace openrac::frontend::debug_overlay {

void renderer_section(const Shown& shown) {
    const renderer::gl_stats::Info& info = renderer::gl_stats::info();
    text("{}", info.renderer);
    text_coloured(kDim, "{} | OpenGL {} | GLSL {}", info.vendor, info.version, info.glsl);

    if (info.software) {
        text_coloured(kRed, "A SOFTWARE RASTERISER: no GPU is drawing this");
    } else {
        text_coloured(kGreen, "hardware renderer");
    }

    ImGui::SameLine();
    text_coloured(kDim, "| GPU timer queries {}", info.timer_queries ? "yes" : "no");
    const stats::Frame* f = stats::history(0);

    if (f != nullptr) {
        const auto c = [&](stats::Counter counter) {
            return f->counters[static_cast<std::size_t>(counter)];
        };
        text(
            "{} draw calls   {} triangles",
            c(stats::Counter::DrawCalls),
            c(stats::Counter::Triangles)
        );
        text(
            "{} texture uploads ({:.1f} KB)   {:.1f} KB of buffers",
            c(stats::Counter::TextureUploads),
            static_cast<double>(c(stats::Counter::TextureUploadBytes)) / 1024.0,
            static_cast<double>(c(stats::Counter::BufferUploadBytes)) / 1024.0
        );
        text(
            "{} program changes   {} framebuffer binds",
            c(stats::Counter::ProgramBinds),
            c(stats::Counter::FramebufferBinds)
        );
    }

    // The passes, averaged over the frames of the last half second whose GPU times are back.
    struct Pass {
        double cpu = 0.0, gpu = 0.0;
        std::uint64_t draws = 0, triangles = 0;
        int frames = 0, gpu_frames = 0;
    };

    std::vector<std::pair<std::string, Pass>> passes;
    const stats::Frame* newest = stats::history(0);

    for (std::size_t back = 0; back < stats::history_size(); ++back) {
        const stats::Frame* h = stats::history(back);

        if (h == nullptr || newest == nullptr || newest->start_s - h->start_s > 0.5) {
            break;
        }

        for (const stats::Section& section : h->sections) {
            if (section.depth != 1) {
                continue;
            }

            auto found = std::find_if(passes.begin(), passes.end(), [&](const auto& p) {
                return p.first == section.name;
            });

            if (found == passes.end()) {
                passes.emplace_back(section.name, Pass{});
                found = passes.end() - 1;
            }

            Pass& p = found->second;
            p.cpu += section.cpu_ms;
            p.draws += section.draw_calls;
            p.triangles += section.triangles;
            p.frames++;

            if (section.gpu_ms >= 0.0) {
                p.gpu += section.gpu_ms;
                p.gpu_frames++;
            }
        }
    }

    if (shown.renderer != nullptr
        && ImGui::BeginTable("passes", 6, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("pass");
        ImGui::TableSetupColumn("on");
        ImGui::TableSetupColumn("draws");
        ImGui::TableSetupColumn("triangles");
        ImGui::TableSetupColumn("CPU ms");
        ImGui::TableSetupColumn("GPU ms");
        ImGui::TableHeadersRow();

        for (const auto& r : shown.renderer->renderers()) {
            const std::string& name = r->name();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(name.c_str());
            ImGui::TableNextColumn();
            bool on = !s.hidden.contains(name);
            ImGui::PushID(name.c_str());

            if (ImGui::Checkbox("##on", &on)) {
                if (on) {
                    s.hidden.erase(name);
                } else {
                    s.hidden.insert(name);
                }
            }

            ImGui::PopID();
            const auto found = std::find_if(passes.begin(), passes.end(), [&](const auto& p) {
                return p.first == name;
            });

            if (found == passes.end() || found->second.frames == 0) {
                ImGui::TableNextColumn();
                ImGui::TextDisabled("not drawn");
                continue;
            }

            const Pass& p = found->second;
            ImGui::TableNextColumn();
            text("{}", p.draws / static_cast<std::uint64_t>(p.frames));
            ImGui::TableNextColumn();
            text("{}", p.triangles / static_cast<std::uint64_t>(p.frames));
            ImGui::TableNextColumn();
            text("{:.2f}", p.cpu / p.frames);
            ImGui::TableNextColumn();

            if (p.gpu_frames > 0) {
                text("{:.2f}", p.gpu / p.gpu_frames);
            } else {
                ImGui::TextDisabled("-");
            }
        }

        ImGui::EndTable();
    }

    ImGui::Checkbox("wireframe", &s.wireframe);
    text(
        "window {} x {}   picture {} x {} at {},{}",
        shown.window_width,
        shown.window_height,
        shown.box_width,
        shown.box_height,
        shown.box_x,
        shown.box_y
    );
    text(
        "frame buffer {} x {}   last world {} x {}",
        shown.frame_width,
        shown.frame_height,
        shown.last_world_width,
        shown.last_world_height
    );
    const renderer::gl_stats::Textures textures = renderer::gl_stats::textures();
    text(
        "GL textures {} ({})   texture pool {} entries",
        textures.count,
        megabytes(textures.bytes),
        shown.renderer != nullptr
            ? const_cast<renderer::Renderer*>(shown.renderer)->textures().size()
            : 0
    );
    int used = 0;
    const std::vector<renderer::gl_stats::Program> programs = renderer::gl_stats::programs();

    for (const renderer::gl_stats::Program& p : programs) {
        used += p.binds > 0 ? 1 : 0;
    }

    text("shader programs {} ({} used this frame)", programs.size(), used);
    text(
        "2D path {} bytes   native 2D quads {}   effect quads {}   strips {}",
        s.counts.packet_bytes,
        s.counts.ui_quads,
        s.counts.effect_quads,
        s.counts.strips
    );
    text("layers the game drew: {:#x}", s.draws);
}

}  // namespace openrac::frontend::debug_overlay

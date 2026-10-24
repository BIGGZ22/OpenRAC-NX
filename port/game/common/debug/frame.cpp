// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay's frame section: the tick and frame rates, the frame times' plots and where a
// frame's time goes, averaged over the last second (debug/overlay.h).

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <imgui.h>
#include <string>

#include "common/stats.h"
#include "debug/overlay.h"
#include "renderer/gl_stats.h"

namespace openrac::frontend::debug_overlay {

Averages averages() {
    Averages a;
    const stats::Frame* newest = stats::history(0);

    if (newest == nullptr) {
        return a;
    }

    double gpu = 0.0;
    double span = 0.0;

    for (std::size_t back = 0; back < stats::history_size(); ++back) {
        const stats::Frame* f = stats::history(back);

        if (f == nullptr || newest->start_s - f->start_s > 1.0) {
            break;
        }

        a.frames++;
        span += f->interval_ms;
        a.frame_ms += f->interval_ms;
        a.worst_ms = std::max(a.worst_ms, f->interval_ms);
        a.prepare_ms += f->cpu_ms("prepare");
        a.render_ms += f->cpu_ms("render");
        a.present_ms += f->cpu_ms("present");
        a.overlay_ms += f->cpu_ms("overlay");
        a.swap_ms += f->cpu_ms("swap");

        if (f->kind == stats::FrameKind::Tick) {
            a.ticks++;
            a.game_ms += f->game_ms;
            a.scene_ms += f->cpu_ms("scene");
        }

        a.wait_ms += f->cpu_ms("wait");

        if (const double g = f->gpu_ms(); g >= 0.0 && f->pending_gpu == 0) {
            gpu += g;
            a.gpu_frames++;
        }
    }

    if (a.frames > 0 && span > 0.0) {
        a.fps = 1000.0 * a.frames / span;
        a.tps = 1000.0 * a.ticks / span;
        a.frame_ms /= a.frames;
        a.prepare_ms /= a.frames;
        a.render_ms /= a.frames;
        a.present_ms /= a.frames;
        a.overlay_ms /= a.frames;
        a.swap_ms /= a.frames;
    }

    if (a.ticks > 0) {
        a.game_ms /= a.ticks;
        a.scene_ms /= a.ticks;
        a.wait_ms /= a.ticks;
    }

    if (a.gpu_frames > 0) {
        a.gpu_ms = gpu / a.gpu_frames;
    }

    return a;
}

void frame_section(const Shown& shown, const Averages& a) {
    const double tick_ms = shown.tick_period * 1000.0;
    text(
        "{:.1f} ticks/s (the game's {:.0f})   {:.1f} frames/s shown",
        a.tps,
        shown.tick_period > 0.0 ? 1.0 / shown.tick_period : 0.0,
        a.fps
    );

    if (a.worst_ms > 1.5 * std::max(tick_ms, 1.0)) {
        text_coloured(
            kRed, "frame {:.2f} ms average, {:.2f} ms worst (last second)", a.frame_ms, a.worst_ms
        );
    } else {
        text("frame {:.2f} ms average, {:.2f} ms worst (last second)", a.frame_ms, a.worst_ms);
    }

    // The frame times, oldest left; and the game's own time of each tick.
    static std::array<float, 240> frame_ms{};
    static std::array<float, 240> game_ms{};
    const std::size_t n = std::min<std::size_t>(frame_ms.size(), stats::history_size());
    std::size_t ticks = 0;

    for (std::size_t i = 0; i < n; ++i) {
        const stats::Frame* f = stats::history(n - 1 - i);
        frame_ms[i] = f != nullptr ? static_cast<float>(f->interval_ms) : 0.0f;
    }

    for (std::size_t back = 0; back < stats::history_size() && ticks < game_ms.size(); ++back) {
        const stats::Frame* f = stats::history(back);

        if (f != nullptr && f->kind == stats::FrameKind::Tick) {
            game_ms[game_ms.size() - 1 - ticks++] = static_cast<float>(f->game_ms);
        }
    }

    const float top = static_cast<float>(std::max(2.0 * tick_ms, 10.0));
    ImGui::PlotLines(
        "##frame",
        frame_ms.data(),
        static_cast<int>(n),
        0,
        std::format("frame, 0-{:.0f} ms", top).c_str(),
        0.0f,
        top,
        ImVec2(dp(300.0f), dp(44.0f))
    );
    // Scaled to its own worst: the game's code takes a small part of a tick on a PC.
    float game_top = 0.5f;

    for (std::size_t i = game_ms.size() - ticks; i < game_ms.size(); ++i) {
        game_top = std::max(game_top, game_ms[i] * 1.25f);
    }

    ImGui::PlotHistogram(
        "##game",
        game_ms.data() + (game_ms.size() - ticks),
        static_cast<int>(ticks),
        0,
        std::format("game tick, 0-{:.1f} ms", game_top).c_str(),
        0.0f,
        game_top,
        ImVec2(dp(300.0f), dp(34.0f))
    );

    if (ImGui::BeginTable("split", 2, ImGuiTableFlags_SizingFixedFit)) {
        row("game tick (the decompiled C)", std::format("{:6.2f} ms / tick", a.game_ms));
        row("scene read from memory", std::format("{:6.2f} ms / tick", a.scene_ms));
        row("blend between ticks", std::format("{:6.2f} ms / frame", a.prepare_ms));
        row("render submission (CPU)", std::format("{:6.2f} ms / frame", a.render_ms));

        if (a.gpu_ms >= 0.0) {
            row("GPU (timer queries)", std::format("{:6.2f} ms / frame", a.gpu_ms));
        } else {
            row("GPU",
                renderer::gl_stats::info().timer_queries ? "no result yet"
                                                         : "not measured: no timer queries");
        }

        row("present + overlay",
            std::format("{:6.2f} + {:.2f} ms / frame", a.present_ms, a.overlay_ms));
        row("swap and frame pacing", std::format("{:6.2f} ms / frame", a.swap_ms));
        row("wait for the next tick", std::format("{:6.2f} ms / tick", a.wait_ms));
        ImGui::EndTable();
    }

    const stats::Frame* last = stats::history(0);
    text(
        "frames per tick {:.2f}   last: {} alpha {:.3f}",
        a.ticks > 0 ? static_cast<double>(a.frames) / a.ticks : 0.0,
        last != nullptr ? stats::frame_kind_name(last->kind) : "-",
        last != nullptr ? static_cast<double>(last->alpha) : 0.0
    );
    text(
        "between-tick frames {}   vsync {}   refresh {}",
        shown.interpolating ? "on" : "off (one frame a tick)",
        shown.vsync ? "on" : "off",
        shown.refresh_interval > 0.0 ? std::format(
                                           "{:.2f} ms ({:.0f} Hz)",
                                           shown.refresh_interval * 1000.0,
                                           1.0 / shown.refresh_interval
                                       )
                                     : std::string("unknown")
    );
    text(
        "frame cost before swap {:.2f} ms   process CPU {:.0f}% of a core",
        shown.cost * 1000.0,
        s.cpu_load * 100.0
    );
}

}  // namespace openrac::frontend::debug_overlay

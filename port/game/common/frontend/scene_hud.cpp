// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Binds captured HUD data and offscreen menu objects without accessing live game memory.
#include <chrono>
#include <cmath>
#include <string>
#include <vector>

#include "frontend.h"
#include "game/common/frontend/geometry.h"
#include "game/common/frontend/scene_frame_internal.h"
#include "renderer/direct.h"
#include "renderer/gl.h"

namespace openrac::frontend {
namespace {

// Offscreen target geometry decoded from the native HUD callback arguments.
struct MobyTarget {
    std::uint64_t marker;  // Address/count marker in the owned HUD packets.
    unsigned framebuffer;  // GPU framebuffer selected by the HUD renderer.
    int size;              // Square target extent in pixels.
    float centre_x;        // Horizontal projection centre in native HUD pixels.
    float centre_y;        // Vertical projection centre in native HUD pixels.
    bool clear_depth;      // Clears depth before drawing this panel.
};

// Draws menu mobys using their captured cameras, without reading live game memory.
void draw_target_mobys(State& owner, const Kept& scene, const MobyTarget& target) {
    const std::uint32_t first = static_cast<std::uint32_t>(target.marker);
    // The HUD marker packs the first game address below an eight-bit instance count.
    const std::uint32_t count = static_cast<std::uint32_t>(target.marker >> 32) & 0xFFu;
    std::vector<scene::Instance> instances;
    const host::GameState* camera = nullptr;

    // The captured page objects occupy consecutive 256-byte moby records.
    for (const Kept::TargetMoby& moby : scene.target_mobys) {
        // Other markers draw different ranges of the captured menu objects.
        if (moby.address >= first && moby.address < first + count * 0x100u) {
            instances.push_back(moby.instance);
            camera = &moby.camera;
        }
    }

    // Incomplete captures cannot supply the panel projection.
    if (camera == nullptr || !owner.renderer || camera->focal_x <= 0.0f) {
        return;
    }

    gl::glViewport(0, 0, target.size, target.size);
    gl::glDisable(gl::GL_SCISSOR_TEST);

    // Each panel decides whether it starts a new depth layer.
    if (target.clear_depth) {
        gl::glDepthMask(gl::GL_TRUE);
        gl::glClearDepth(0.0);
        gl::glClear(gl::GL_DEPTH_BUFFER_BIT);
    }

    const float half = 0.5f * static_cast<float>(renderer::kOffscreenPixels);
    const float tan_x = half / camera->focal_x;
    const float tan_y = half / camera->focal_y;
    // Match the world camera's near/far range in the offscreen panel.
    const renderer::Mat4 projection =
        renderer::perspective_reversed(2.0f * std::atan(tan_y), tan_x / tan_y, 0.05f, 2000.0f);
    renderer::Mat4 offset = renderer::identity();
    // Matrix translation moves the axis from the square centre to the native HUD projection centre.
    offset[12] = (target.centre_x - half) / half;
    offset[13] = -(target.centre_y - half) / half;
    renderer::FrameInput panel;
    panel.camera.view = camera->view();
    panel.camera.projection = renderer::multiply(offset, projection);
    panel.camera.position = camera->camera_position;
    renderer::RenderState state{
        owner.renderer->textures(),
        renderer::RenderTarget{target.framebuffer, target.size, target.size},
        {}
    };
    owner.scene.draw_mobys(instances, panel, state);
}

}  // namespace

void set_packets(SceneFrame& frame) {
    const Kept& scene = frame.scene;
    auto& input = frame.input;
    const auto& graphics = frame.graphics;
    const bool enhanced = enhanced_graphics_available() && graphics.enhanced_graphics;
    const bool has_camera = scene.has_camera;
    const auto started = frame.output.started;
    input.gtao_settings = graphics.gtao_settings;
    // Honor the user's GTAO setting alongside probe GI and modern shadows.
    // Mesh shaders apply AO to indirect lighting before direct light is added.
    input.gtao = enhanced && graphics.gtao && has_camera && (scene.draws & 0x1Fu) != 0;
    input.frame = scene.frame;
    // Shadow direction easing follows display time rather than the number of rendered frames.
    input.seconds = std::chrono::duration<double>(started.time_since_epoch()).count();
    input.strip_vertices = scene.strip_vertices;
    input.strips = scene.strips;
    input.direct_before_world =
        std::span<const std::uint8_t>(scene.packets).first(scene.before_world);
    input.packets[static_cast<std::size_t>(renderer::Bucket::Hud)] =
        std::span<const std::uint8_t>(scene.packets).subspan(scene.before_world);
    input.effects = scene.effects;
    input.sky_shells = scene.sky_shells;
    input.sky_shell_mask = scene.sky_shell_mask;
    input.effect_uploads = scene.effect_uploads;
    input.ui_quads = scene.ui_quads;
}

void set_target_mobys(SceneFrame& frame) {
    State& owner = frame.owner;
    const Kept& scene = frame.scene;
    frame.input.target_mobys = [&owner, &scene](
                                   std::uint64_t marker,
                                   unsigned framebuffer,
                                   int size,
                                   float centre_x,
                                   float centre_y,
                                   bool clear_depth
                               ) {
        draw_target_mobys(
            owner, scene, {marker, framebuffer, size, centre_x, centre_y, clear_depth}
        );
    };
}

void retain_background(SceneFrame& frame) {
    State& owner = frame.owner;
    const Kept& scene = frame.scene;
    auto& input = frame.input;
    const bool first = frame.output.first;
    const bool has_camera = scene.has_camera;
    const int width = frame.output.width;
    const int height = frame.output.height;
    // A tick that draws no world and has no picture of its own to put under its 2D (the page
    // menu's first ticks, before its snapshot of the frame comes; the tick it closes on): the
    // console still shows the last frame there, so the 2D goes over the last world drawn, not black.
    // Only right after the world was drawn (the menu opening) or right after a menu tick with its
    // snapshot (the menu closing): other ticks without a world (a new game's captions between its
    // scenes) are drawn on black, as they come.
    constexpr std::size_t kSnapshotBytes = 0x40000;
    const bool after_world = owner.last_world_tick != 0 && scene.frame - owner.last_world_tick <= 3;

    // A newly opened page may omit the snapshot transfer for its first few ticks.
    if (first && has_camera && (scene.draws & 0x0Fu) == 0 && scene.before_world < kSnapshotBytes
        && (after_world || owner.last_tick_menu) && owner.last_world.width() == width
        && owner.last_world.height() == height) {
        owner.hold_last_world = true;
    } else if (first) {
        owner.hold_last_world = false;
    }

    // Pause-menu packet gaps reuse the retained world instead of clearing to black.
    if (owner.hold_last_world && !owner.repeat_frame && owner.last_world.width() == width
        && owner.last_world.height() == height) {
        owner.last_world.blit_to(owner.frame.id(), width, height);
        input.keep_colour = true;
    }

    // The last frame that drew its world, as the world alone (before the HUD's 2D goes over it):
    // what the page menus read back for their background, and what a tick without a world of its
    // own is drawn over.
    if (first && has_camera && (scene.draws & 0x1Fu) != 0 && width > 0 && height > 0) {
        input.before_hud = [&owner, &scene, width, height]() {
            // Size changes invalidate the retained world attachment.
            if (owner.last_world.width() != width || owner.last_world.height() != height) {
                std::string error;
                owner.last_world.create(width, height, error);
            }

            owner.frame.blit_to(owner.last_world.id(), width, height);
            owner.last_world_tick = scene.frame;
        };
    }
}

}  // namespace openrac::frontend

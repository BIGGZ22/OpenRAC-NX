// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// frontend/state.h's world recording: interpolated owned scene data, lighting, temporal resolve
// and native HUD. The level and graphics context are pinned until the worker handoff completes.
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common/log.h"
#include "common/stats.h"
#include "debug_overlay.h"
#include "frontend.h"
#include "game/common/frontend/pacing.h"
#include "game/common/frontend/scene_frame_internal.h"
#include "renderer/direct.h"
#include "renderer/gl.h"
#include "scene/moby_pose.h"

namespace openrac::frontend {
namespace {

// Creates or resets temporal history for the submitted graphics settings.
bool prepare_temporal(SceneFrame& frame) {
    State& owner = frame.owner;
    const Kept& scene = frame.scene;
    const auto& graphics = frame.graphics;
    const bool temporal_wanted = enhanced_graphics_available() && graphics.enhanced_graphics
                                 && graphics.temporal_quality != renderer::TemporalQuality::Off
                                 && enhanced::Fsr3::available() && scene.has_camera
                                 && (scene.draws & 0x1Fu) != 0;

    // SDK and pose history are absent when enhanced reconstruction is not in use.
    if (temporal_wanted && !owner.temporal) {
        owner.temporal = std::make_unique<enhanced::Fsr3>();
        // Register after GL statistics initialise, so SDK owners retire before their exit-time destruction.
        std::atexit([] {
            // A manually closed frontend has already released this owner.
            if (g) {
                // Cleanup still retires temporal history after a reported worker error.
                [[maybe_unused]] const bool finished = finish_render();
                g->temporal.reset();
            }
        });
    } else if (!temporal_wanted && owner.temporal) {
        owner.temporal->reset();
    }

    return temporal_wanted;
}

// ---- Frame configuration ----

// Recreates the target when its dimensions, samples or depth-sampling requirement changes.
void resize_target(SceneFrame& frame, int samples, bool sampleable_depth) {
    auto& target = frame.owner.frame;
    const auto& output = frame.output;

    // Minimized pictures keep their last target until a drawable size returns.
    if (output.box.width <= 0 || output.box.height <= 0) {
        return;
    }

    // Stable settings reuse the existing attachments.
    if (target.width() == output.width && target.height() == output.height
        && target.samples() == samples && (target.depth_texture() != 0) == sampleable_depth) {
        return;
    }

    std::string error;
    bool created = target.create(output.width, output.height, error, samples, sampleable_depth);

    // Some drivers advertise sample counts they cannot attach; retry without multisampling.
    if (!created && samples > 1) {
        log::warn("{}; drawing without anti-aliasing", error);
        created = target.create(
            output.width,
            output.height,
            error,
            1,
            enhanced_graphics_available() && frame.graphics.enhanced_graphics
        );
    }

    // Propagate attachment failure through the worker rather than recording against an incomplete target.
    if (!created) {
        throw std::runtime_error(error);
    }

    log::info(
        "frame {}x{} for a picture of {}x{} pixels, {} sample(s) a pixel",
        target.width(),
        target.height(),
        output.box.width,
        output.box.height,
        target.samples()
    );
}

// Sizes the render target and returns the original picture width within it.
float prepare_target(SceneFrame& frame) {
    auto& output = frame.output;
    const auto& graphics = frame.graphics;
    frame_size(
        output.box.width, output.box.height, graphics.render_scale, output.width, output.height
    );
    // Temporal reconstruction supplies antialiasing; native rendering keeps the saved MSAA preference.
    const int samples =
        frame.temporal ? 1 : std::clamp(graphics.msaa, 1, renderer::FrameBuffer::max_samples());
    const bool sampleable_depth =
        samples == 1 && enhanced_graphics_available() && graphics.enhanced_graphics;
    resize_target(frame, samples, sampleable_depth);

    // A wider picture preserves the central television framing.
    if (output.aspect > kTvAspect && output.box.width > 0 && output.box.height > 0) {
        return std::min(
            1.0f,
            kTvAspect * static_cast<float>(output.box.height) / static_cast<float>(output.box.width)
        );
    }

    return 1.0f;
}

// Applies the submitted settings and authored shadow state.
void set_lighting(SceneFrame& frame) {
    State& owner = frame.owner;
    const Kept& scene = frame.scene;
    const auto& graphics = frame.graphics;
    auto& input = frame.input;

    // Boot pictures can arrive before a world renderer exists.
    if (owner.renderer) {
        const int anisotropy = graphics.anisotropy_samples(
            gl::enhanced_rendering_available() && graphics.enhanced_graphics
        );
        owner.renderer->textures().set_anisotropy(static_cast<float>(anisotropy));
    }

    const bool enhanced = enhanced_graphics_available() && graphics.enhanced_graphics;
    input.texture_filter = enhanced ? graphics.texture_filter : renderer::TextureFilter::Normal;
    owner.scene
        .set_probe_lighting(enhanced && graphics.probe_lighting, graphics.probe_gi_multiplier);
    owner.scene.set_probe_bias(graphics.probe_normal_bias, graphics.probe_view_bias);
    owner.scene.set_probe_spheres(graphics.probe_spheres);
    input.sun_pixel_shadow = enhanced && graphics.enhanced_shadows
                             && owner.scene.sun_shadow_enabled() && scene.has_camera
                             && scene.level >= 0 && scene.level < 19;
    input.hero_pixel_shadow = enhanced && graphics.enhanced_shadows
                              && owner.scene.hero_shadow_enabled()
                              && scene.hero_airborne.has_value() && scene.has_camera
                              && scene.level >= 0 && scene.level < 19;
    input.hero_airborne = scene.hero_airborne.value_or(false);
    input.shadow_extrusion = scene.shadow_state.extrusion;

    // Disabled authored shadow state carries no usable height interval.
    if (scene.shadow_state.enabled) {
        input.shadow_heights = scene.shadow_state.heights;
    }
}

// Enables submitted world buckets while retaining native HUD drawing during loading.
void set_buckets(SceneFrame& frame) {
    State& owner = frame.owner;
    const Kept& scene = frame.scene;
    const bool has_camera = scene.has_camera;

    // Native HUD remains available even when the game submits no world.
    for (auto& bucket : owner.renderer
                            ? owner.renderer->renderers()
                            : std::span<const std::unique_ptr<renderer::BucketRenderer>>{}) {
        // Particle effects are meaningful only with a captured world camera.
        if (bucket->bucket() == renderer::Bucket::Particles) {
            // The effect quads are there only when the game drew them.
            bucket->enabled = has_camera;
        } else if (bucket->bucket() != renderer::Bucket::Hud) {
            bucket->enabled =
                has_camera && (scene.draws & (1u << static_cast<unsigned>(bucket->bucket()))) != 0;
        }
    }
}

// Configures visible buckets, the interpolated camera and probe selection.
void set_camera(SceneFrame& frame) {
    State& owner = frame.owner;
    const Kept& scene = frame.scene;
    auto& input = frame.input;
    const float picture_scale = frame.picture_scale;
    const auto state = scene.has_prev_state && frame.output.alpha < 1.0f
                           ? blend_camera(scene.prev_state, scene.state, frame.output.alpha)
                           : scene.state;
    // No camera yet (loading, fades between programs): a black frame, not the level's background.
    const bool has_camera = scene.has_camera;
    input.clear_colour =
        has_camera
            ? std::array<
                  float,
                  4>{owner.level.background[0], owner.level.background[1], owner.level.background[2], 1}
            : std::array<float, 4>{0, 0, 0, 1};
    set_buckets(frame);
    input.camera.view = state.view();
    // The game's frustum: its horizontal and vertical tangents as the game set them (the frame is
    // the TV's 4:3, which they were made for), not the window's shape. A wider picture keeps the
    // vertical one and opens the horizontal one by as much as the picture is wider: the game's
    // view is its middle.
    input.camera.projection = state.projection(
        (state.tan_half_fov_y > 0.0f ? state.tan_half_fov_x / state.tan_half_fov_y : 4.0f / 3.0f)
            / picture_scale,
        0.05f,
        2000.0f
    );
    input.camera.position = state.camera_position;
    owner.scene.update_probe_regions(input.camera);
    input.after_world_depth =
        owner.scene.compute_probe_selection()
            ? std::function<void(unsigned, int, int)>(
                  [&owner, &input](unsigned depth, int width, int height) {
                      owner.scene.select_visible_probe_regions(depth, input.camera, width, height);
                  }
              )
            : std::function<void(unsigned, int, int)>{};
}

// ---- World recording ----

// Records the submitted world and temporal resolve before native HUD drawing.
void draw_world(SceneFrame& frame) {
    State& owner = frame.owner;
    const Kept& scene = frame.scene;
    const auto& graphics = frame.graphics;
    auto& input = frame.input;
    const int width = frame.output.width;
    const int height = frame.output.height;
    const float picture_scale = frame.picture_scale;
    const bool temporal_wanted = frame.temporal;

    // Repeated pause-menu frames preserve the already completed world.
    if (owner.renderer && !owner.repeat_frame) {
        // [debug tools] The passes hidden in the overlay; the render timed (common/stats.h).
        debug_overlay::before_render(owner.renderer.get());
        const stats::Scope render_scope("render");
        renderer::RenderTarget target{owner.frame.id(), width, height, picture_scale};
        target.depth_texture = owner.frame.depth_texture();
        target.resolve_framebuffer = owner.frame.resolve_target();
        target.resolves_itself = owner.frame.resolves_itself();
        target.wide_2d = picture_scale < 1.0f && graphics.hud == settings::Hud::Edges;

        // Loading/menu snapshots bypass history; active world frames resolve before the final HUD bucket.
        if (temporal_wanted && !input.keep_colour) {
            std::string temporal_error;
            const enhanced::TemporalFrame temporal{
                input, target, owner.frame, owner.scene, owner.renderer->textures(), scene.level
            };
            const bool active = owner.temporal->begin(
                temporal, graphics.temporal_quality, graphics.temporal_sharpness, temporal_error
            );

            // The temporal module reports recoverable setup failures without throwing.
            if (!active && !temporal_error.empty()) {
                log::error("{}", temporal_error);
            }
        }

        owner.renderer->render(input, target);
    }
}
}  // namespace

void render_scene(
    State& owner,
    const Kept& snapshot,
    const settings::Settings& graphics,
    FramePresentation& output
) {
    SceneFrame frame{owner, snapshot, graphics, output, {}, {}, false, 1};
    frame.temporal = prepare_temporal(frame);
    upload_instances(frame);
    const auto scene_done = std::chrono::steady_clock::now();
    frame.picture_scale = prepare_target(frame);
    set_lighting(frame);
    blend_shadows(frame);
    set_camera(frame);
    set_packets(frame);
    set_target_mobys(frame);
    retain_background(frame);
    draw_world(frame);
    const auto recorded = std::chrono::steady_clock::now();

    // Frame-cost diagnostics deliberately stall the GPU, matching their synchronous reference behaviour.
    if (output.measure_cost) {
        gl::glFinish();
    }

    output.shown = owner.frame.resolved();
    output.camera = frame.input.camera;
    output.tick = frame.input.frame;
    output.finished = std::chrono::steady_clock::now();
    output.scene_seconds = std::chrono::duration<double>(scene_done - output.started).count();
    output.record_seconds = std::chrono::duration<double>(recorded - scene_done).count();
    output.gpu_seconds = std::chrono::duration<double>(output.finished - recorded).count();
}

}  // namespace openrac::frontend

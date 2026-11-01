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
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
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
#ifdef __SWITCH__
#include "platform/switch/render_resolution.h"
#include "platform/switch/overlay_stats.h"
#include "platform/switch/camera_options.h"
#endif

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
#ifdef __SWITCH__
    const int preset = platform::switch_video::profile_resolution(graphics.switch_resolution,
        graphics.switch_handheld_resolution, graphics.switch_docked_resolution, platform::overlay_stats::docked());
    platform::switch_video::target_size(preset, output.width, output.height);
#endif
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
    const bool has_camera = frame.has_camera;

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

// Returns the camera to draw the world from: `camera`, or when usable_camera or usable_view
// refuses it (non-finite, degenerate, out past every level: a frame of it is a black screen
// crossed by a few smeared triangles), the last one they accepted, and none before any was. The
// first frame of each run of refused ones is logged (a few runs a session), with the camera's
// numbers, so that a report carries them.
std::optional<renderer::Camera> guard_camera(
    State& owner, const host::GameState& state, const renderer::Camera& camera, std::uint64_t tick
) {
    const CameraPose pose{state.camera_position, state.forward, state.left, state.up};

    // A usable camera is drawn, and kept for frames that have none.
    if (usable_camera(pose) && usable_view(camera.view, camera.projection)) {
        if (owner.refused_camera_frames != 0) {
            log::info("camera usable again after {} refused frame(s)", owner.refused_camera_frames);
            owner.refused_camera_frames = 0;
        }
        owner.good_camera = camera;
        return camera;
    }

    // The first frame of a run is logged; a session logs at most a few runs.
    constexpr int kMostLogged = 4;
    if (owner.refused_camera_frames++ == 0 && owner.refused_camera_logs < kMostLogged) {
        ++owner.refused_camera_logs;
        log::warn(
            "camera: tick {} has an unusable camera (eye {} {} {}, forward {} {} {}, up {} {} {}, "
            "tangents {} {}); {}",
            tick,
            state.camera_position[0],
            state.camera_position[1],
            state.camera_position[2],
            state.forward[0],
            state.forward[1],
            state.forward[2],
            state.up[0],
            state.up[1],
            state.up[2],
            state.tan_half_fov_x,
            state.tan_half_fov_y,
            owner.good_camera ? "drawing the last good one" : "drawing no world"
        );
    }

    return owner.good_camera;
}

// Configures visible buckets, the interpolated camera and probe selection.
void set_camera(SceneFrame& frame) {
    State& owner = frame.owner;
    const Kept& scene = frame.scene;
    auto& input = frame.input;
    const float picture_scale = frame.picture_scale;
    auto state = scene.has_prev_state && frame.pose_alpha < 1.0f
                           ? blend_camera(scene.prev_state, scene.state, frame.pose_alpha)
                           : scene.state;
#ifdef __SWITCH__
    const float fov = platform::switch_camera::fov_scale(frame.graphics.switch_fov,
        platform::switch_camera::gameplay.load(std::memory_order_relaxed));
    state.tan_half_fov_x *= fov;
    state.tan_half_fov_y *= fov;
#endif
    renderer::Camera camera;
    camera.view = state.view();
    // The game's frustum: its horizontal and vertical tangents as the game set them (the frame is
    // the TV's 4:3, which they were made for), not the window's shape. A wider picture keeps the
    // vertical one and opens the horizontal one by as much as the picture is wider: the game's
    // view is its middle.
    camera.projection = state.projection(
        (state.tan_half_fov_y > 0.0f ? state.tan_half_fov_x / state.tan_half_fov_y : 4.0f / 3.0f)
            / picture_scale,
        0.05f,
        2000.0f
    );
    camera.position = state.camera_position;
    // No camera yet (loading, fades between programs): a black frame, not the level's background.
    // Nor one the guard refuses with no good one to draw instead. With one, the colour the game
    // clears to (the level settings' background, which a mod can change), else the extracted
    // level's.
    const std::optional<renderer::Camera> drawn =
        scene.has_camera ? guard_camera(owner, state, camera, scene.frame) : std::nullopt;
    const bool has_camera = drawn.has_value();
    frame.has_camera = has_camera;
    const auto background = scene.state.background_or(owner.level.background);
    // A leftover valid camera alone does not mean the game submitted a world.
    // Retained menu backgrounds are preserved separately by keep_colour.
    const bool draws_world=(scene.draws & 0x1Fu)!=0;
    input.clear_colour = world_clear_colour(has_camera,draws_world,background);
    if(has_camera && !draws_world && (background[0]!=0 || background[1]!=0 || background[2]!=0)) {
        static unsigned reported=0;
        if(reported++<8) log::info("transition tick {}: no world submitted; clear black instead of level colour {} {} {}",
            scene.frame,background[0],background[1],background[2]);
    }
    set_buckets(frame);
    input.camera = drawn.value_or(camera);
    owner.scene.choose_scenery(
        frame.graphics.original_lod, input.camera.position, input.camera.projection
    );
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

    // Repeated pause-menu frames preserve the already completed world; a tick drawn over the last
    // picture draws its 2D once, on its first frame (drawn again, a fade's darkening would add up).
    if (owner.renderer && !owner.repeat_frame && !(owner.overdraw && !frame.output.first)) {
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
    // At the first frame and on each change: the settings that decide the scene's GPU work, so a
    // performance log says what it measured (session 33's did not show the scenery detail).
    {
        static bool told = false;
        static std::array<int, 9> last{};
        const std::array<int, 9> now{
            graphics.original_lod ? 1 : 0, static_cast<int>(graphics.texture_filter),
            graphics.anisotropy, graphics.msaa, graphics.render_scale, graphics.switch_resolution,
            static_cast<int>(graphics.vsync), static_cast<int>(graphics.frame_rate), graphics.frame_cap
        };
        if (!told || now != last) {
            told = true;
            last = now;
            log::info(
                "graphics settings in use: scenery detail {}, texture filter {}, anisotropy {} (0 auto), "
                "samples {}, render scale {}, Switch resolution preset {}, vsync {}, frame rate mode {}, cap {}",
                graphics.original_lod ? "original draw distances" : "full detail (every tie and shrub)",
                now[1], now[2], now[3], now[4], now[5], now[6], now[7], now[8]
            );
        }
    }
    // OPENRAC_FLIGHT_POSE_AT_TICK=0 blends the flight between planets like other worlds again
    // (an A/B switch for the canopy candidate, docs/port/DKGL_SHIP_CANOPY_SOURCE_FIX_2026-10-29.md).
    static const bool flight_at_tick = [] {
        const char* v = std::getenv("OPENRAC_FLIGHT_POSE_AT_TICK");
        return v == nullptr || std::string_view(v) != "0";
    }();
    frame.pose_alpha =
        flight_at_tick ? world_pose_alpha(snapshot.world, output.alpha) : output.alpha;
    // Once a run, the log says the flight is held at the tick, so a hardware test shows it ran.
    static bool told = false;
    if (snapshot.world == -2 && !told) {
        told = true;
        log::info("frontend: flight between planets drawn {}", flight_at_tick
            ? "at the game tick (canopy candidate; OPENRAC_FLIGHT_POSE_AT_TICK=0 blends it)"
            : "blended between ticks (OPENRAC_FLIGHT_POSE_AT_TICK=0)");
    }
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

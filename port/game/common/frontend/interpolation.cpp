// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frames drawn between the game's ticks, a slice of frontend/state.h: the kept tick drawn with
// its camera, mobys, poses and shadow part way from the last tick's to its own (draw_kept); when a
// frame is due and whether one is drawn before the next tick (set_tick, interpolating,
// wants_between, between, frame_due); the swap and what it tells the pacing of the display
// (swapped); the paused game drawn again (hold); and the frame's cost switches (OPENRAC_FRAME_COST,
// OPENRAC_SLOW_FRAME). The pacing's pure arithmetic is frontend/pacing.h's; frame() in
// frontend.cpp reads each tick from the game and keeps it (Kept) for what is drawn here.
//
// Only the thread that opened the window runs any of it, as with the state it reads (state.h).

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include <SDL3/SDL.h>

#include "common/env.h"
#include "common/log.h"
#include "common/stats.h"
#include "debug_menu.h"
#include "debug_overlay.h"
#include "fps_counter.h"
#include "frame_pacing.h"
#include "frontend.h"
#include "game/common/frontend/geometry.h"
#include "game/common/frontend/pacing.h"
#include "game/common/frontend/state.h"
#include "openrac/game_host.h"
#include "platform/performance_hint.h"
#include "platform/pump.h"
#include "platform/window.h"
#include "renderer/direct.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/renderer.h"
#include "renderer/shadows.h"
#include "scene/level_renderer.h"
#include "scene/moby_pose.h"
#include "scene/screenshot.h"
#include "settings.h"

namespace openrac::frontend {

namespace {

// OPENRAC_FRAME_COST=N: every N frames drawn (default 200), in the log, what a frame cost on
// average and where: `scene` (the tick's mobys and poses moved between ticks and sent to their
// buffers, on the CPU), `submit` (the renderers' OpenGL calls, on the CPU), `gpu` (the wait for the
// graphics card to finish them: this switch calls glFinish after each frame, so that the card's
// time is not counted into a later call; it lowers the frame rate a little) and `show` (the
// frame scaled to the window, and the swap). With the draw calls and triangles of the last frame.
// frame_cost is the one; only the thread that opened the window touches it.
struct FrameCost {
    bool on = false;
    int every = 200;
    int frames = 0;
    double scene = 0.0, submit = 0.0, gpu = 0.0, show = 0.0;
    std::chrono::steady_clock::time_point mark{};

    FrameCost() {
        if (env::value(env::Var::FrameCost)) {
            on = true;
            // Read as std::atoi read it, held to int's range, where atoi's answer was undefined.
            const std::int64_t asked = std::min<std::int64_t>(
                env::integer(env::Var::FrameCost, 0), std::numeric_limits<int>::max()
            );
            every = asked > 1 ? static_cast<int>(asked) : 200;
        }
    }

    // The seconds since the last call, added to `part`.
    void lap(double& part) {
        const auto now = std::chrono::steady_clock::now();
        part += std::chrono::duration<double>(now - mark).count();
        mark = now;
    }
} frame_cost;

// [debug tools] The picture being drawn is a paused game's (hold). Only the thread that opened the
// window touches it.
bool g_held = false;

// Returns the frames a second the frames between ticks are held to (the player's cap, or the
// display's rate), 0 for no limit and with one frame a tick.
//
// The cap and refresh share one deadline schedule. A quick presentation return
// is not proof of a missed VSync: Vulkan queues presentation asynchronously.
double frame_cap() {
    if (!interpolating()) {
        return 0.0;
    }
    const settings::Settings& s = settings::get();
    const double requested =
        s.frame_rate == settings::FrameRate::Capped
            ? static_cast<double>(
                  std::clamp(s.frame_cap, settings::kFrameCapMin, settings::kFrameCapMax)
              )
            : 0.0;
    return frame_pacing::limit(
        requested,
        s.frame_rate == settings::FrameRate::Display,
        g->swap != platform::SwapInterval::Immediate,
        pacing.interval
    );
}

// The refresh a frame started now is shown at: the first after the frame is drawn.
std::chrono::steady_clock::time_point next_display(std::chrono::steady_clock::time_point now) {
    return shown_at(now, pacing.cost, pacing.interval, pacing.last_swap);
}

// How far the scene is from the last tick's to the kept one's at that refresh.
float alpha_at(std::chrono::steady_clock::time_point display) {
    return tick_alpha(display, pacing.tick, pacing.period);
}

// Swaps the window's buffers for the frame begun at `started`, and keeps what the swap tells the
// pacing: what a frame costs, the display's refresh interval when it is to be read again, the
// last swap that waited for a refresh, and when the frame cap lets the next frame start.
void swapped(std::chrono::steady_clock::time_point started) {
    using namespace std::chrono;
    const auto before = steady_clock::now();
    platform::frame_work_done();
    g->window->swap();
    const auto after = steady_clock::now();
    const double cost = duration<double>(before - started).count();
    pacing.cost = 0.8 * pacing.cost + 0.2 * cost;
    if (settings::get().show_fps) {
        fps_counter::frame_drawn(after, cost);
    }
    if (pacing.interval_stale) {
        const float rate = g->window->refresh_rate();
        pacing.interval = rate > 20.0f ? 1.0 / rate : 0.0;
        pacing.interval_stale = false;
    }
    // What a frame may take, for the system's CPU clocks (performance_hint.h): a refresh, or the
    // player's frame cap when it is lower.
    const double cap = frame_cap();
    platform::set_frame_target(cap > 0.0 ? std::max(1.0 / cap, pacing.interval) : pacing.interval);
    if (g->swap == platform::SwapInterval::Immediate || gl::is_vulkan()
        || duration<double>(after - before).count() < 0.0005) {
        // Vulkan's return is not a vblank timestamp, and an immediate GL return
        // supplies no display phase either. The shared deadline paces both;
        // never add a second refresh-length sleep after drawing the tick.
        pacing.last_swap = {};
    } else {
        pacing.last_swap = after;
    }
    // A frame rate of the player's own (and the display's, when no swap waits): the next frame is
    // due that long after this one was, so a frame held up by the game's tick is made up for by the
    // next ones; one more than a frame late starts the count again. It holds beside the swap's
    // wait: 60 frames a second on a 120 Hz display are every other refresh. Nothing sleeps here:
    // the game's tick may be due before the next frame is (frame_due).
    if (cap > 0.0) {
        const auto step = duration_cast<steady_clock::duration>(duration<double>(1.0 / cap));
        const auto now = steady_clock::now();
        pacing.due = frame_pacing::next_due(pacing.due, started, now, step);
    } else {
        pacing.due = {};
    }
    // The next frame's work counts from here: the swap, and any wait in it, is the display's.
    platform::work_resumed();
}

// OPENRAC_SLOW_FRAME=MS: every frame drawn takes that much longer, as on a graphics card too slow
// for the settings, for checking that the game's time does not slow with it.
void slow_frame() {
    static const double ms = env::number(env::Var::SlowFrame, 0.0);
    if (ms > 0.0) {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(ms));
    }
}

// The camera part way from the last tick's to this one's (a cut is not blended).
viewer::GameState blend_camera(
    const viewer::GameState& from, const viewer::GameState& to, float t
) {
    viewer::GameState c = to;
    const CameraPose pose = blend_pose(
        {from.camera_position, from.forward, from.left, from.up},
        {to.camera_position, to.forward, to.left, to.up},
        t
    );
    c.camera_position = pose.position;
    c.forward = pose.forward;
    c.left = pose.left;
    c.up = pose.up;
    return c;
}

}  // namespace

// Whether the frame cap lets a frame start now (within a millisecond: a sleep wakes that coarsely).
bool frame_due(std::chrono::steady_clock::time_point now) {
    return frame_may_start(frame_cap(), pacing.due, now);
}

// ---- The kept tick ----

bool enhanced_graphics_available() {
    return g && gl::enhanced_rendering_available() && g->scene.probe_data_available();
}

void draw_kept(bool first) {
    const auto started = std::chrono::steady_clock::now();
    const float alpha = interpolating() ? alpha_at(next_display(started)) : 1.0f;
    debug_overlay::begin(
        first, g_held, alpha, g->kept.frame
    );  // [debug tools] the frame's measurements
    pacing.last_alpha = alpha;
    frame_cost.mark = started;
    // OPENRAC_PACING: each frame shown, when (ms), how far between ticks, the refresh interval.
    static const bool& pacing_log =
        stats::probe("OPENRAC_PACING", "each frame shown and when, in the log");
    if (pacing_log) {
        static const auto t0 = started;
        log::info(
            "pace {} {:.2f} ms alpha {:.3f} {} refresh {:.2f} ms cost {:.2f} ms",
            g->kept.frame,
            std::chrono::duration<double, std::milli>(started - t0).count(),
            alpha,
            first ? "tick" : "between",
            pacing.interval * 1000.0,
            pacing.cost * 1000.0
        );
    }
    Kept& k = g->kept;
    const viewer::GameState state =
        k.has_prev_state && alpha < 1.0f ? blend_camera(k.prev_state, k.state, alpha) : k.state;
    if (!g->level.models.empty()) {
        std::vector<viewer::Instance> live = k.live;
        if (alpha < 1.0f) {
            for (std::size_t i = 0; i < live.size() && i < k.addresses.size(); ++i) {
                auto it = k.prev_matrices.find(k.addresses[i]);
                if (it != k.prev_matrices.end()) {
                    live[i].matrix = blend_matrix(it->second, live[i].matrix, alpha);
                    const auto sphere = k.prev_spheres.find(k.addresses[i]);
                    live[i].world_sphere = sphere == k.prev_spheres.end()
                                               ? std::array<float, 4>{}
                                               : viewer::moby_sphere_envelope(
                                                     sphere->second,
                                                     it->second,
                                                     k.live[i].world_sphere,
                                                     k.live[i].matrix,
                                                     live[i].matrix
                                                 );
                }
            }
        }
        if (alpha < 1.0f && !k.prev_poses.empty()) {
            // The poses too: each joint part way from the last tick's to this one's, for a moby
            // that is the same thing moving (its matrix was blended) with the same skeleton.
            std::vector<viewer::LevelScene::JointColumns> palette = k.palette;
            for (std::size_t i = 0;
                 i < live.size() && i < k.addresses.size() && i < k.joints.size();
                 ++i) {
                const auto pose = k.prev_poses.find(k.addresses[i]);
                const int first = live[i].palette;
                const int count = k.joints[i];
                const auto last = k.prev_matrices.find(k.addresses[i]);
                if (pose == k.prev_poses.end() || first < 0 || count <= 0
                    || pose->second.second != count || last == k.prev_matrices.end()
                    || !same_moby(last->second, k.live[i].matrix)
                    || static_cast<std::size_t>(first + count) > palette.size()) {
                    continue;
                }
                const viewer::LevelScene::JointColumns* from =
                    k.prev_palette.data() + pose->second.first;
                for (int j = 0; j < count; ++j) {
                    viewer::LevelScene::JointColumns& to =
                        palette[static_cast<std::size_t>(first + j)];
                    for (std::size_t c = 0; c < to.size(); ++c) {
                        to[c] = from[j][c] + (to[c] - from[j][c]) * alpha;
                    }
                }
            }
            g->scene.set_palette(palette);
        } else {
            g->scene.set_palette(k.palette);
        }
        g->scene.set_instances(viewer::Layer::Mobys, live);
    }

    if (frame_cost.on) {
        frame_cost.lap(frame_cost.scene);
    }

    int window_width = 0;
    int window_height = 0;
    g->window->drawable_size(window_width, window_height);
    const float aspect = picture_aspect(window_width, window_height);
    const Box box = frame_box(window_width, window_height, aspect);
    // The frame: the picture's box in the window, at the player's render scale, with the player's
    // samples a pixel. It is scaled to the box when it is shown (present).
    int width = 0;
    int height = 0;
    frame_size(box.width, box.height, settings::get().render_scale, width, height);
    // How much of the frame's width the game's 4:3 picture is: all of it, or its middle.
    const float picture_scale =
        aspect > kTvAspect && box.width > 0 && box.height > 0
            ? std::min(
                  1.0f, kTvAspect * static_cast<float>(box.height) / static_cast<float>(box.width)
              )
            : 1.0f;
    std::string error;
    const int samples = std::clamp(settings::get().msaa, 1, renderer::FrameBuffer::max_samples());
    const bool sampleable_depth =
        samples == 1 && enhanced_graphics_available() && settings::get().enhanced_graphics;
    if (box.width > 0 && box.height > 0
        && (g->frame.width() != width || g->frame.height() != height
            || g->frame.samples() != samples
            || (g->frame.depth_texture() != 0) != sampleable_depth)) {
        if (!g->frame.create(width, height, error, samples, sampleable_depth) && samples > 1) {
            // A card that refuses the samples it listed: the frame without them.
            log::warn("{}; drawing without anti-aliasing", error);
            g->frame.create(
                width,
                height,
                error,
                1,
                enhanced_graphics_available() && settings::get().enhanced_graphics
            );
        }
        log::info(
            "frame {}x{} for a picture of {}x{} pixels, {} sample(s) a pixel",
            g->frame.width(),
            g->frame.height(),
            box.width,
            box.height,
            g->frame.samples()
        );
    }
    renderer::FrameInput input;
    const auto& graphics = settings::get();
    if (g->renderer) {
        const int anisotropy = graphics.anisotropy_samples(
            gl::enhanced_rendering_available() && graphics.enhanced_graphics
        );
        g->renderer->textures().set_anisotropy(static_cast<float>(anisotropy));
    }
    const bool enhanced = enhanced_graphics_available() && graphics.enhanced_graphics;
    input.texture_filter = enhanced ? graphics.texture_filter : renderer::TextureFilter::Normal;
    g->scene.set_probe_lighting(enhanced && graphics.probe_lighting, graphics.probe_gi_multiplier);
    g->scene.set_probe_bias(graphics.probe_normal_bias, graphics.probe_view_bias);
    g->scene.set_probe_spheres(graphics.probe_spheres);
    input.sun_pixel_shadow = enhanced && graphics.enhanced_shadows && g->scene.sun_shadow_enabled()
                             && k.has_camera && k.level >= 0 && k.level < 19;
    input.hero_pixel_shadow = enhanced && graphics.enhanced_shadows
                              && g->scene.hero_shadow_enabled() && k.hero_airborne.has_value()
                              && k.has_camera && k.level >= 0 && k.level < 19;
    input.hero_airborne = k.hero_airborne.value_or(false);
    input.shadow_extrusion = k.shadow_state.extrusion;
    if (k.shadow_state.enabled) {
        input.shadow_heights = k.shadow_state.heights;
    }
    auto shadows = k.shadows;
    if (alpha < 1.0f && k.shadow_moby != 0 && k.shadow_moby == k.prev_shadow_moby
        && shadows.size() == k.prev_shadows.size()) {
        const auto prev = k.prev_matrices.find(k.shadow_moby);
        const auto now = std::find(k.addresses.begin(), k.addresses.end(), k.shadow_moby);
        if (prev != k.prev_matrices.end() && now != k.addresses.end()
            && same_moby(
                prev->second, k.live[static_cast<std::size_t>(now - k.addresses.begin())].matrix
            )) {
            for (std::size_t i = 0; i < shadows.size(); ++i) {
                for (int j = 0; j < 3; ++j) {
                    shadows[i].a[j] =
                        k.prev_shadows[i].a[j] + (shadows[i].a[j] - k.prev_shadows[i].a[j]) * alpha;
                    shadows[i].b[j] =
                        k.prev_shadows[i].b[j] + (shadows[i].b[j] - k.prev_shadows[i].b[j]) * alpha;
                }
                shadows[i].radius_a = k.prev_shadows[i].radius_a
                                      + (shadows[i].radius_a - k.prev_shadows[i].radius_a) * alpha;
                shadows[i].radius_b = k.prev_shadows[i].radius_b
                                      + (shadows[i].radius_b - k.prev_shadows[i].radius_b) * alpha;
            }
            if (k.prev_shadow_state.enabled && k.shadow_state.enabled) {
                for (int j = 0; j < 3; ++j) {
                    input.shadow_extrusion[j] =
                        k.prev_shadow_state.extrusion[j]
                        + (k.shadow_state.extrusion[j] - k.prev_shadow_state.extrusion[j]) * alpha;
                }
                std::array<float, 2> heights{};
                for (int j = 0; j < 2; ++j) {
                    heights[j] =
                        k.prev_shadow_state.heights[j]
                        + (k.shadow_state.heights[j] - k.prev_shadow_state.heights[j]) * alpha;
                }
                input.shadow_heights = heights;
            }
        }
    }
    input.shadow_capsules = shadows;
    // No camera yet (loading, fades between programs): a black frame, not the level's background.
    const bool has_camera = k.has_camera;
    input.clear_colour =
        has_camera
            ? std::array<
                  float,
                  4>{g->level.background[0], g->level.background[1], g->level.background[2], 1}
            : std::array<float, 4>{0, 0, 0, 1};
    for (auto& r : g->renderer ? g->renderer->renderers()
                               : std::span<const std::unique_ptr<renderer::BucketRenderer>>{}) {
        if (r->bucket() == renderer::Bucket::Particles) {
            // The effect quads are there only when the game drew them.
            r->enabled = has_camera;
        } else if (r->bucket() != renderer::Bucket::Hud) {
            r->enabled = has_camera && (k.draws & (1u << static_cast<unsigned>(r->bucket()))) != 0;
        }
    }
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
    g->scene.update_probe_regions(input.camera);
    input.after_world_depth =
        g->scene.compute_probe_selection()
            ? std::function<void(unsigned, int, int)>([&](unsigned depth, int width, int height) {
                  g->scene.select_visible_probe_regions(depth, input.camera, width, height);
              })
            : std::function<void(unsigned, int, int)>{};
    input.gtao_settings = settings::get().gtao_settings;
    // Honor the user's GTAO setting alongside probe GI and modern shadows.
    // Mesh shaders apply AO to indirect lighting before direct light is added.
    input.gtao = enhanced && graphics.gtao && has_camera && (k.draws & 0x1Fu) != 0;
    input.frame = k.frame;
    input.strip_vertices = k.strip_vertices;
    input.strips = k.strips;
    input.direct_before_world = std::span<const std::uint8_t>(k.packets).first(k.before_world);
    input.packets[static_cast<std::size_t>(renderer::Bucket::Hud)] =
        std::span<const std::uint8_t>(k.packets).subspan(k.before_world);
    input.effects = k.effects;
    input.sky_shells = k.sky_shells;
    input.sky_shell_mask = k.sky_shell_mask;
    input.effect_uploads = k.effect_uploads;
    input.ui_quads = k.ui_quads;
    // The mobys the game drew into a texture, at their markers (Kept::target_mobys): seen by the
    // camera they were drawn with, through the frame's focal lengths, about the centre of the
    // target the 2D layer was drawing into (its XYOFFSET's 2048), in its 512-pixel square.
    input.target_mobys = [&](std::uint64_t marker,
                             unsigned framebuffer,
                             int size,
                             float centre_x,
                             float centre_y,
                             bool clear_depth) {
        const std::uint32_t first = static_cast<std::uint32_t>(marker);
        const std::uint32_t count = static_cast<std::uint32_t>(marker >> 32) & 0xFFu;
        std::vector<viewer::Instance> instances;
        const viewer::GameState* camera = nullptr;
        for (const Kept::TargetMoby& t : k.target_mobys) {
            if (t.address >= first && t.address < first + count * 0x100u) {
                instances.push_back(t.instance);
                camera = &t.camera;
            }
        }
        if (camera == nullptr || !g->renderer || camera->focal_x <= 0.0f) {
            return;
        }
        using namespace openrac::gl;
        glViewport(0, 0, size, size);
        glDisable(GL_SCISSOR_TEST);
        if (clear_depth) {
            glDepthMask(GL_TRUE);
            glClearDepth(0.0);
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        const float half = 0.5f * static_cast<float>(renderer::kOffscreenPixels);
        const float tan_x = half / camera->focal_x;
        const float tan_y = half / camera->focal_y;
        renderer::Mat4 projection =
            renderer::perspective_reversed(2.0f * std::atan(tan_y), tan_x / tan_y, 0.05f, 2000.0f);
        // The axis through the centre: x on and y down from the square's middle, in device units.
        renderer::Mat4 offset = renderer::identity();
        offset[12] = (centre_x - half) / half;
        offset[13] = -(centre_y - half) / half;
        renderer::FrameInput panel;
        panel.camera.view = camera->view();
        panel.camera.projection = renderer::multiply(offset, projection);
        panel.camera.position = camera->camera_position;
        renderer::RenderState target{
            g->renderer->textures(), renderer::RenderTarget{framebuffer, size, size}, {}
        };
        g->scene.draw_mobys(instances, panel, target);
    };
    // A tick that draws no world and has no picture of its own to put under its 2D (the page
    // menu's first ticks, before its snapshot of the frame comes; the tick it closes on): the
    // console still shows the last frame there, so the 2D goes over the last world drawn, not black.
    // Only right after the world was drawn (the menu opening) or right after a menu tick with its
    // snapshot (the menu closing): other ticks without a world (a new game's captions between its
    // scenes) are drawn on black, as they come.
    constexpr std::size_t kSnapshotBytes = 0x40000;
    const bool after_world = g->last_world_tick != 0 && k.frame - g->last_world_tick <= 3;
    if (first && has_camera && (k.draws & 0x0Fu) == 0 && k.before_world < kSnapshotBytes
        && (after_world || g->last_tick_menu) && g->last_world.width() == width
        && g->last_world.height() == height) {
        g->hold_last_world = true;
    } else if (first) {
        g->hold_last_world = false;
    }
    if (g->hold_last_world && !g->repeat_frame && g->last_world.width() == width
        && g->last_world.height() == height) {
        g->last_world.blit_to(g->frame.id(), width, height);
        input.keep_colour = true;
    }
    // The last frame that drew its world, as the world alone (before the HUD's 2D goes over it):
    // what the page menus read back for their background, and what a tick without a world of its
    // own is drawn over.
    if (first && has_camera && (k.draws & 0x1Fu) != 0 && width > 0 && height > 0) {
        input.before_hud = [&]() {
            if (g->last_world.width() != width || g->last_world.height() != height) {
                std::string e;
                g->last_world.create(width, height, e);
            }
            g->frame.blit_to(g->last_world.id(), width, height);
            g->last_world_tick = k.frame;
        };
    }
    if (g->renderer && !g->repeat_frame) {
        // [debug tools] The passes hidden in the overlay; the render timed (common/stats.h).
        debug_overlay::before_render(g->renderer.get());
        const stats::Scope render_scope("render");
        renderer::RenderTarget target{g->frame.id(), width, height, picture_scale};
        target.depth_texture = g->frame.depth_texture();
        target.resolve_framebuffer = g->frame.resolve_target();
        target.resolves_itself = g->frame.resolves_itself();
        target.wide_2d = picture_scale < 1.0f && settings::get().hud == settings::Hud::Edges;
        g->renderer->render(input, target);
    }
    if (frame_cost.on) {
        frame_cost.lap(frame_cost.submit);
        gl::glFinish();
        frame_cost.lap(frame_cost.gpu);
    }
    // What is shown: the frame with one sample a pixel (a multisampled one is resolved here, once),
    // with the port's own overlays over it.
    const unsigned shown = g->frame.resolved();
    debug_menu::draw(shown, width, height);
    if (settings::get().show_fps) {
        fps_counter::draw(shown, width, height);
    }
    // OPENRAC_SHOT=FRAME:FILE.png writes that frame (for checking without looking at the screen);
    // OPENRAC_SHOT_BETWEEN also writes the frames drawn between ticks (FILE<frame>_b<n>.png).
    static const bool& shot_between =
        stats::probe("OPENRAC_SHOT_BETWEEN", "OPENRAC_SHOT also writes the frames between ticks");
    static int between_index = 0;
    between_index = first ? 0 : between_index + 1;
    // OPENRAC_SHOT_FROM=FRAME: no shots before that frame.
    static const std::uint64_t shot_from = env_frame(env::Var::ShotFrom).value_or(0);
    if ((first || shot_between) && shots().due(input.frame) && input.frame >= shot_from) {
        const std::string path = shots().prefix + std::to_string(input.frame)
                                 + (first ? std::string() : "_b" + std::to_string(between_index))
                                 + ".png";
        viewer::write_png(path, width, height, renderer::read_rgba(shown, width, height));
        log::info("frame {} written to {}", input.frame, path);
    }
    slow_frame();
    present(shown, width, height, window_width, window_height, false, aspect);
    draw_touch_pad(window_width, window_height);
    // [debug tools] The overlay, over the frame in the window.
    debug_overlay::draw(
        {g->window->sdl(),
         window_width,
         window_height,
         box.x,
         box.y,
         box.width,
         box.height,
         g->renderer.get(),
         &input.camera,
         g->frame.width(),
         g->frame.height(),
         g->last_world.width(),
         g->last_world.height(),
         g->swap != platform::SwapInterval::Immediate,
         interpolating(),
         pacing.interval,
         pacing.cost,
         pacing.period.count()}
    );
    const stats::Scope swap_scope("swap");
    swapped(started);
    if (frame_cost.on) {
        frame_cost.lap(frame_cost.show);
        if (++frame_cost.frames >= frame_cost.every) {
            const double n = frame_cost.frames * 0.001;
            const renderer::FrameStats stats =
                g->renderer ? g->renderer->last_stats() : renderer::FrameStats{};
            log::info(
                "cost tick {}: {:.3f} ms a frame over {} frames (scene {:.3f}, submit {:.3f}, "
                "gpu {:.3f}, show {:.3f}); {} draw calls, {} triangles, frame {}x{} x{}",
                g->kept.frame,
                (frame_cost.scene + frame_cost.submit + frame_cost.gpu + frame_cost.show) / n,
                frame_cost.frames,
                frame_cost.scene / n,
                frame_cost.submit / n,
                frame_cost.gpu / n,
                frame_cost.show / n,
                stats.draw_calls,
                stats.triangles,
                width,
                height,
                g->frame.samples()
            );
            frame_cost.frames = 0;
            frame_cost.scene = frame_cost.submit = frame_cost.gpu = frame_cost.show = 0.0;
        }
    }
}

// ---- Between ticks ----

void set_tick(
    std::chrono::steady_clock::time_point tick,
    std::chrono::duration<double> period,
    std::chrono::duration<double> late
) {
    pacing.tick = tick;
    pacing.period = period;
    pacing.late = late;
}

bool interpolating() {
    return !one_frame_a_tick() && settings::get().frame_rate != settings::FrameRate::Original && g
           && g->window;
}

bool wants_between() {
    return interpolating() && g->kept.has_state && g->kept.has_camera;
}

int between(std::chrono::steady_clock::time_point next_tick) {
    if (!g) {
        return 0;
    }
    // A frame only for a refresh the kept tick is still shown at (the next tick's, with the lag).
    const auto lag =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(pacing.period * kLagTicks);
    auto now = std::chrono::steady_clock::now();
    // The frame cap: a frame not due before the next tick is left to after it; one due sooner is
    // waited for here.
    if (!frame_due(now)) {
        if (pacing.due >= next_tick) {
            return 0;
        }
        std::this_thread::sleep_until(pacing.due);
        platform::work_resumed();
        now = std::chrono::steady_clock::now();
    }
    const auto display = next_display(now);
    if (display >= next_tick + lag) {
        return 0;
    }
    // Nothing waits for the display: a frame that would not be done by the next tick's time is not
    // started, so that the tick comes on time whatever a frame costs (with the swap's wait the
    // frame belongs to a refresh, and the tick after it is a little late and made up for).
    if (g->swap == platform::SwapInterval::Immediate
        && now
                   + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                       std::chrono::duration<double>(pacing.cost)
                   )
               >= next_tick) {
        return 0;
    }
    // The scene has arrived at the kept tick's: another frame would be the same picture again.
    if (!pacing.first_pending && pacing.last_alpha >= 1.0f && alpha_at(display) >= 1.0f) {
        return 0;
    }
    SDL_Event event;
    while (platform::poll_event(event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return -1;
        }
        if (window_key(event) || debug_overlay::event(event)
            || debug_menu::event(event, openrac_game_loaded_overlay())) {
            continue;
        }
        g->input.handle_event(event);
    }
    const bool dump = renderer::g_dump_draws;
    renderer::g_dump_draws = false;
    const bool first = pacing.first_pending;
    pacing.first_pending = false;
    draw_kept(first);
    renderer::g_dump_draws = dump;
    return 1;
}

// [debug tools] While the overlay holds the game (pause): the last tick drawn again, the window's
// events read, until it lets a tick through. False once the window was closed.
bool hold() {
    while (g && g->window && debug_overlay::holding()) {
        SDL_Event event;
        while (platform::poll_event(event)) {
            if (event.type == SDL_EVENT_QUIT) {
                return false;
            }
            if (window_key(event) || debug_overlay::event(event)) {
                continue;
            }
            g->input.handle_event(event);
        }
        if (debug_overlay::take_settings_changed()) {
            apply_settings();  // the drawer is open over the paused game
        }
        const auto started = std::chrono::steady_clock::now();
        const bool dump = renderer::g_dump_draws;
        renderer::g_dump_draws = false;
        g_held = true;
        draw_kept(false);
        g_held = false;
        renderer::g_dump_draws = dump;
        // No faster than the display shows them (a 120 Hz screen's drawer moves at 120), and no
        // faster than 60 a second where its rate is not known.
        const double interval = pacing.interval > 0.0 ? pacing.interval : 1.0 / 60.0;
        std::this_thread::sleep_until(
            started
            + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(interval * 0.9)
            )
        );
    }
    return true;
}

}  // namespace openrac::frontend

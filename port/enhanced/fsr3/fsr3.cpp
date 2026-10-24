// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Fsr3 setup keeps optional SDK resources in enhanced Vulkan rendering and preserves HUD resolution.
#include <algorithm>
#include <cmath>

#include "common/log.h"
#include "enhanced/fsr3/fsr3_internal.h"
#include "enhanced/fsr3/temporal_math.h"
#include "fsr3_shaders.h"
#include "renderer/gl.h"
#include "scene/level_renderer.h"

#include <FidelityFX/host/ffx_fsr3upscaler.h>

namespace openrac::enhanced {
using namespace openrac::gl;

namespace {
// Camera discontinuities need fresh accumulation; ordinary movement retains temporal history.
bool camera_cut(const renderer::Camera& current, const renderer::Camera& previous) {
    float distance_squared = 0;
    float alignment = 0;

    // Squared displacement and forward alignment distinguish a cut from ordinary camera movement.
    for (std::size_t n = 0; n < current.position.size(); ++n) {
        const float delta = current.position[n] - previous.position[n];
        distance_squared += delta * delta;
        alignment += current.view[n * 4 + 2] * previous.view[n * 4 + 2];
    }

    // Eight world units, a 45-degree turn, or a changed projection identifies a cut.
    return distance_squared > 64 || alignment < .707f
           || std::abs(current.projection[0] - previous.projection[0]) > .01f
           || std::abs(current.projection[5] - previous.projection[5]) > .01f;
}
}  // namespace

Fsr3::Fsr3() : m_state(std::make_unique<State>()) {}

Fsr3::~Fsr3() {
    release();
}

bool Fsr3::available() {
    vkgl::NativeDevice device;
    return enhanced_rendering_available() && vkgl::native_device(device) && device.storage_formats;
}

void Fsr3::set_scene(
    std::span<const scene::Instance> instances, std::span<const renderer::Mat4> palette
) {
    m_state->instances.assign(instances.begin(), instances.end());
    m_state->palette.assign(palette.begin(), palette.end());
}

bool Fsr3::State::build_shaders(std::string& error) {
    // Retain compiled helper programs across resolution and quality changes.
    if (camera_motion.id() && object_motion.id() && reactive_mask.id()) {
        return true;
    }

    return camera_motion.build(
               "temporal camera motion",
               shaders::fullscreen_vert,
               shaders::camera_motion_frag,
               error
           )
           && object_motion.build(
               "temporal object motion", shaders::motion_vert, shaders::motion_frag, error
           )
           && reactive_mask.build(
               "temporal reactive mask", shaders::fullscreen_vert, shaders::reactive_frag, error
           );
}

bool Fsr3::State::configure(
    renderer::FrameBuffer& output, std::array<int, 2> size, std::string& error
) {
    const bool resized = world.width() != size[0] || world.height() != size[1]
                         || display.width != output.width() || display.height != output.height();

    // History images can be reused only at identical input and output dimensions.
    if (world.width() != size[0] || world.height() != size[1]) {
        valid = false;

        // RGBA16F preserves signed velocity; R32F stores depth without a stencil aspect.
        if (!world.create(size[0], size[1], error, 1, true)
            || !motion.create(size[0], size[1], 0x881A, 0, error)
            || !depth.create(size[0], size[1], 0x822E, 0, error)
            || !reactive.create(size[0], size[1], GL_RGBA8, 0, error)
            || !opaque.create(size[0], size[1], GL_RGBA8, 0, error)) {
            return false;
        }
    }

    // The final colour target alone needs storage usage; the original renderer's images remain ordinary attachments.
    if (!vkgl::storage_texture(output.colour_texture()) || output_texture != output.colour_texture()
        || display.width != output.width() || display.height != output.height()) {
        valid = false;

        // Storage capability can change even when a framebuffer reuses the same texture name.
        if (!vkgl::allocate_storage_texture(
                output.colour_texture(), GL_RGBA8, output.width(), output.height()
            )) {
            error = "cannot allocate FSR output image";
            return false;
        }

        output_texture = output.colour_texture();
    }

    // Only complete setups are reported as active temporal rendering.
    if (!build_shaders(error) || !sdk.configure(size, {output.width(), output.height()}, error)) {
        return false;
    }

    // Resolution changes are useful diagnostics without a message on every displayed frame.
    if (resized) {
        log::info(
            "FSR temporal upscaling: {}x{} to {}x{}",
            size[0],
            size[1],
            output.width(),
            output.height()
        );
    }

    return true;
}

bool Fsr3::State::prepare_camera(TemporalFrame frame, float sharpness, std::string& error) {
    const std::array<int, 2> size{world.width(), world.height()};
    camera = frame.input.camera;
    dispatch.reset = !valid || level != frame.level || camera_cut(camera, old_camera);
    level = frame.level;
    const auto now = std::chrono::steady_clock::now();
    const float elapsed = std::chrono::duration<float, std::milli>(now - last_time).count();
    dispatch.milliseconds = valid ? std::clamp(elapsed, 1.0f, 250.0f) : 16.666667f;
    last_time = now;
    dispatch.sharpness = std::clamp(sharpness, 0.0f, 1.0f);
    dispatch.vertical_fov = 2 * std::atan(1 / camera.projection[5]);
    const int phases = ffxFsr3UpscalerGetJitterPhaseCount(size[0], frame.display.width());
    ffxFsr3UpscalerGetJitterOffset(&dispatch.jitter[0], &dispatch.jitter[1], jitter_index, phases);
    jitter_index = (jitter_index + 1) % phases;
    renderer::Mat4 offset = renderer::identity();
    offset[12] = 2 * dispatch.jitter[0] / static_cast<float>(size[0]);
    offset[13] = 2 * dispatch.jitter[1] / static_cast<float>(size[1]);
    frame.input.camera.projection = renderer::multiply(offset, camera.projection);
    view.raster = renderer::multiply(frame.input.camera.projection, camera.view);

    // Restore the original camera if its projection cannot be inverted.
    if (!fsr3_detail::invert(view.raster, inverse_raster)) {
        frame.input.camera = camera;
        error = "invalid temporal camera projection";
        return false;
    }

    view.current = renderer::multiply(camera.projection, camera.view);
    view.previous =
        dispatch.reset ? view.current : renderer::multiply(old_camera.projection, old_camera.view);
    view.size = size;
    return true;
}

void Fsr3::State::attach_frame(TemporalFrame frame, std::array<int, 2> size) {
    scene = &frame.scene;
    textures = &frame.textures;
    display = frame.target;
    has_opaque = false;
    frame.target.framebuffer = world.id();
    frame.target.width = size[0];
    frame.target.height = size[1];
    frame.target.depth_texture = world.depth_texture();
    frame.target.resolve_framebuffer = 0;
    frame.target.resolves_itself = false;
    frame.input.on_before_particles = [this](const auto& target) {
        capture_opaque(target);
    };
    frame.input.on_resolve_world = [this](auto& target) {
        resolve(target);
    };
}

bool Fsr3::begin(
    TemporalFrame frame, renderer::TemporalQuality quality, float sharpness, std::string& error
) {
    State& state = *m_state;

    // Unsupported or failed modes retain native rendering without per-frame retries.
    if (quality == renderer::TemporalQuality::Off || state.failed || !available()) {
        return false;
    }

    // Minimized windows and invalid settings cannot form a temporal frame.
    if (frame.target.width <= 0 || frame.target.height <= 0
        || quality >= renderer::TemporalQuality::Count || !std::isfinite(sharpness)) {
        error = "invalid temporal output or sharpening setting";
        return false;
    }

    const auto size =
        fsr3_detail::render_extent({frame.display.width(), frame.display.height()}, quality);

    // A failed setup cannot install callbacks or leave a jittered native fallback.
    if (!state.configure(frame.display, size, error)
        || !state.prepare_camera(frame, sharpness, error)) {
        state.failed = true;
        return false;
    }

    state.attach_frame(frame, size);
    return true;
}

void Fsr3::reset() {
    // Switching away from a failed mode retires it, allowing an explicit later retry.
    if (m_state->failed) {
        release();
    }

    m_state->valid = false;
}

void Fsr3::release() {
    auto& state = *m_state;
    state.sdk.release();
    state.motion.release();
    state.depth.release();
    state.reactive.release();
    state.opaque.release();
    state.world.release();
    state.camera_motion.release();
    state.object_motion.release();
    state.reactive_mask.release();
    glDeleteVertexArrays(1, &state.vao);
    glDeleteTextures(1, &state.previous_palette);
    state.vao = 0;
    state.previous_palette = 0;
    state.output_texture = 0;
    state.valid = false;
    state.failed = false;
    state.previous.clear();
}
}  // namespace openrac::enhanced

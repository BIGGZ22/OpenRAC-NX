// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Private temporal frame state shared by setup and resolve; no frontend includes this file.
#pragma once

#include <chrono>
#include <unordered_map>
#include <vector>

#include "enhanced/fsr3/frame_target.h"
#include "enhanced/fsr3/fsr3.h"
#include "enhanced/fsr3/sdk_context.h"
#include "renderer/framebuffer.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "scene/level.h"
#include "scene/motion.h"

namespace openrac::enhanced {
// Render-thread state, retaining the previous displayed frame rather than a previous simulation tick.
struct Fsr3::State {
    fsr3_detail::SdkContext sdk;        // AMD pipelines and GPU history.
    renderer::FrameBuffer world;        // Render-resolution colour and sampleable opaque depth.
    fsr3_detail::FrameTarget motion;    // RGBA16F: pixel velocity XY and reactive coverage Z.
    fsr3_detail::FrameTarget depth;     // R32F copy, avoiding depth/stencil view reinterpretation.
    fsr3_detail::FrameTarget reactive;  // R channel, particle and disocclusion reactivity.
    fsr3_detail::FrameTarget opaque;    // World colour before the particle bucket.

    renderer::Shader camera_motion;  // Static world and sky motion, or float depth copy.
    renderer::Shader object_motion;  // Rigid and skinned dynamic velocities.
    renderer::Shader reactive_mask;  // Particle colour-difference mask.
    unsigned vao = 0;                // Empty VAO for fullscreen triangles.
    unsigned previous_palette = 0;   // Previous displayed joint matrices.
    unsigned output_texture = 0;     // Display texture last made storage-capable.

    std::vector<scene::Instance> instances;  // Current displayed live mobys.
    std::vector<renderer::Mat4> palette;      // Current displayed joints, padded only on upload.
    std::unordered_map<std::uint64_t, scene::Instance>
        previous;  // Last displayed live identities.
    std::vector<scene::MotionInstance>
        velocities;  // Current/previous pairs for the velocity pass.

    renderer::Camera camera;                    // Current unjittered camera.
    renderer::Camera old_camera;                // Previous unjittered camera.
    renderer::Mat4 inverse_raster{};            // Current clip-to-world transform.
    scene::MotionView view;                    // Camera matrices and motion target size.
    renderer::RenderTarget display;             // Final target restored before HUD drawing.
    scene::LevelScene* scene = nullptr;        // Frontend-owned scene, borrowed through render().
    renderer::TexturePool* textures = nullptr;  // Renderer-owned pool, same borrow duration.

    fsr3_detail::Dispatch dispatch;  // Jitter, elapsed time, camera and reset flag for AMD.
    std::chrono::steady_clock::time_point last_time{};  // Previous displayed-frame timestamp.
    int level = -1;                                     // Last configured scene identity.
    int jitter_index = 0;     // Advanced once per displayed frame, independent of game ticks.
    bool valid = false;       // Previous camera and object poses belong to a completed frame.
    bool has_opaque = false;  // The current frame reached its preparticle snapshot hook.
    bool failed = false;      // Suppresses repeated SDK failures until resources are released.

    // Builds renderer-owned helper shaders lazily, before recording any temporal frame.
    [[nodiscard]] bool build_shaders(std::string& error);
    // Allocates render intermediates and SDK histories only when dimensions change.
    [[nodiscard]] bool configure(
        renderer::FrameBuffer& output, std::array<int, 2> size, std::string& error
    );
    // Applies a new jitter sample and establishes current/previous camera transforms.
    [[nodiscard]] bool prepare_camera(TemporalFrame frame, float sharpness, std::string& error);
    // Installs the render-resolution world target and the synchronous reconstruction callbacks.
    void attach_frame(TemporalFrame frame, std::array<int, 2> size);
    // Selects matching displayed object histories, rejecting new, replaced and teleported objects.
    void build_velocities();
    // Uploads current joints as the next displayed frame's previous palette.
    void save_palette();
    // Captures opaque colour before particles, restoring the target in the caller.
    void capture_opaque(const renderer::RenderTarget& target);
    // Generates static and dynamic velocity, and the float depth image consumed by AMD.
    void draw_motion();
    // Fills camera-only velocity and float depth before dynamic objects overwrite visible motion.
    void draw_camera_motion();
    // Builds a reactive mask from particle colour differences and new-object coverage.
    void draw_reactive();
    // Resolves world colour, retains history and changes the caller's target to display resolution.
    void resolve(renderer::RenderTarget& target);
};
}  // namespace openrac::enhanced

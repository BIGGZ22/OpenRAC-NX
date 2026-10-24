// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Enhanced Vulkan temporal reconstruction with displayed object poses and a full-resolution HUD.
#pragma once

#include <memory>
#include <span>
#include <string>

#include "renderer/math.h"
#include "renderer/temporal_settings.h"

namespace openrac::renderer {
class FrameBuffer;
class TexturePool;
struct FrameInput;
struct RenderTarget;
}  // namespace openrac::renderer

namespace openrac::scene {
class LevelScene;
struct Instance;
}  // namespace openrac::scene

namespace openrac::enhanced {
// One frame's borrowed objects, valid through the renderer's synchronous render call.
struct TemporalFrame {
    renderer::FrameInput& input;      // World camera and resolve callbacks updated by begin().
    renderer::RenderTarget& target;   // Replaced with the render-resolution world target.
    renderer::FrameBuffer& display;   // Owned by the frontend, remains the final HUD target.
    scene::LevelScene& scene;         // Uploaded level geometry, borrowed for velocity drawing.
    renderer::TexturePool& textures;  // Original material alpha textures.
    int level = -1;                   // A level change invalidates temporal history.
};

// Owns SDK histories and intermediate targets; only the render thread calls it.
class Fsr3 {
public:
    // Allocates CPU state only; GPU work starts when begin() succeeds.
    Fsr3();
    // Releases histories before the graphics context closes.
    ~Fsr3();
    Fsr3(const Fsr3&) = delete;
    Fsr3& operator=(const Fsr3&) = delete;
    // Returns whether this build and backend can offer enhanced temporal reconstruction.
    static bool available();
    // Captures the current displayed world poses; inactive callers do not copy any data.
    void set_scene(
        std::span<const scene::Instance> instances, std::span<const renderer::Mat4> palette
    );
    // Prepares jitter and world reconstruction hooks, reporting allocation or shader failures in `error`.
    [[nodiscard]] bool begin(
        TemporalFrame frame, renderer::TemporalQuality quality, float sharpness, std::string& error
    );
    // Invalidates history on loading, a skipped world, or an explicit camera cut.
    void reset();
    // Frees GPU resources while the context is current, retaining compiled shader sources.
    void release();

private:
    struct State;  // SDK context and displayed-frame histories, hidden from frontend code.
    std::unique_ptr<State> m_state;  // Owned CPU and GPU state.
};
}  // namespace openrac::enhanced

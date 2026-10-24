// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// GPU lifetime and dispatch of AMD's temporal upscaler, independent of game and HUD orchestration.
#pragma once
#include <array>
#include <memory>
#include <string>

#include "renderer/vulkan/native_compute.h"

namespace openrac::enhanced::fsr3_detail {
// Per-displayed-frame inputs; motion is unjittered, current-to-previous in render-pixel units.
struct Dispatch {
    std::array<float, 2> jitter{};    // Raster offset in render pixels.
    float milliseconds = 16.666667f;  // Elapsed presentation time, not game-tick time.
    float near_plane = 0.05f;         // Original camera near plane in game units.
    float far_plane = 2000.0f;        // Original finite far plane in game units.
    float vertical_fov = 1;           // Vertical field of view in radians.
    float sharpness = 0.2f;           // Optional RCAS strength in [0, 1].
    bool reset = true;                // First frame, camera cut or invalid history.
};

// One GPU resource context and its cached shader library, used only on the render thread.
class SdkContext {
public:
    // Creates CPU-owned shader and SDK bookkeeping; no GPU work is recorded yet.
    SdkContext();
    // Drains queued work before releasing a live SDK context; the graphics context must remain current.
    ~SdkContext();
    SdkContext(const SdkContext&) = delete;
    SdkContext& operator=(const SdkContext&) = delete;

    // Recreates GPU resources only when render/display extents change; false includes an SDK diagnostic.
    [[nodiscard]] bool configure(
        std::array<int, 2> render_size, std::array<int, 2> display_size, std::string& error
    );
    // Records colour, depth, motion, reactive mask and output in that order on the renderer's queue.
    [[nodiscard]] bool dispatch(
        std::array<unsigned, 5> textures, const Dispatch& frame, std::string& error
    );
    // Retires GPU resources while the graphics context is still current; cached shaders survive.
    void release();

private:
    struct State;  // SDK state and scratch allocation, hidden from application-facing headers.
    std::unique_ptr<State> m_state;  // Owned until this adapter is destroyed.
};
}  // namespace openrac::enhanced::fsr3_detail

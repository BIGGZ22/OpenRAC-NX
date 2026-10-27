// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Private inputs shared by world recording, scene interpolation and captured HUD callbacks.
#pragma once

#include "game/common/frontend/state.h"

namespace openrac::frontend {

// The inputs and temporary views of one owned snapshot; callbacks end before this context expires.
struct SceneFrame {
    State& owner;                        // Pinned by the frontend until the handoff completes.
    const Kept& scene;                   // Owned by the accepted render job.
    const settings::Settings& graphics;  // Settings copied with that job.
    FramePresentation& output;           // Shared result, published by the worker handoff mutex.
    renderer::FrameInput input;          // Views into scene and the shadow storage below.
    std::vector<renderer::ShadowCapsule> shadows;  // Interpolated capsule storage for input.
    bool temporal = false;    // Temporal reconstruction is active for this scene.
    float picture_scale = 1;  // Width of the original picture inside the wider frame.
};

// The camera part way from the last tick's to this one's (a cut is not blended).
host::GameState blend_camera(const host::GameState& from, const host::GameState& to, float t);

// Uploads the interpolated transforms and poses used by world and motion rendering.
void upload_instances(SceneFrame& frame);

// Interpolates capsule shadows only while their source moby retains its identity.
void blend_shadows(SceneFrame& frame);

// Binds the owned packets, effects and HUD quads for this scene.
void set_packets(SceneFrame& frame);

// Registers the native HUD callback, consumed synchronously before the owned snapshot is released.
void set_target_mobys(SceneFrame& frame);

// Retains the last world where the original menu reads its background.
void retain_background(SceneFrame& frame);

// ---- A mod's views (frontend/views.cpp, frontend.h's set_view_source) ----

// True while a mod has views: then each frame draws them, on the window's thread.
bool views_wanted();

// Asks the mod for this frame's views and draws each after the window's picture, from the same
// scene (`world`: whether this frame drew its world anew, else the last pictures are given again),
// then copies them where the mod said.
void draw_views(SceneFrame& frame, bool world);

// Tells the mod the frame that drew its views is submitted (after the window's swap).
void finish_views();

// Before the graphics context goes: the mod lets go of what it made with it, then the views'
// pictures and framebuffers go.
void release_views(State& owner);

}  // namespace openrac::frontend

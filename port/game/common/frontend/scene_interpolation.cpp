// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Interpolates owned camera, instance, joint and shadow values for one displayed frame.
#include <algorithm>
#include <array>
#include <vector>

#include "game/common/frontend/geometry.h"
#include "game/common/frontend/scene_frame_internal.h"
#include "scene/moby_pose.h"

namespace openrac::frontend {
namespace {

// Interpolates owned moby transforms and preserves stable game identities.
std::vector<scene::Instance> blend_instances(const Kept& scene, float alpha) {
    std::vector<scene::Instance> live = scene.live;

    // Game addresses provide stable live identities across grouping and interpolated display frames.
    for (std::size_t n = 0; n < live.size() && n < scene.addresses.size(); ++n) {
        live[n].temporal_id = scene.addresses[n];
    }

    // Tick frames retain their exact pose; intermediate displays blend between captured poses.
    if (alpha < 1.0f) {
        // Addresses align current instances with their previous transform.
        for (std::size_t n = 0; n < live.size() && n < scene.addresses.size(); ++n) {
            auto it = scene.prev_matrices.find(scene.addresses[n]);

            // Newly spawned mobys have no previous transform.
            if (it != scene.prev_matrices.end()) {
                live[n].matrix = blend_matrix(it->second, live[n].matrix, alpha);
                const auto sphere = scene.prev_spheres.find(scene.addresses[n]);
                live[n].world_sphere = sphere == scene.prev_spheres.end()
                                           ? std::array<float, 4>{}
                                           : moby_sphere_envelope(
                                                 sphere->second,
                                                 it->second,
                                                 scene.live[n].world_sphere,
                                                 scene.live[n].matrix,
                                                 live[n].matrix
                                             );
            }
        }
    }

    return live;
}

// Interpolates joints only for the same moby and skeleton.
std::vector<scene::LevelScene::JointColumns> blend_palette(
    const Kept& scene, const std::vector<scene::Instance>& live, float alpha
) {
    std::vector<scene::LevelScene::JointColumns> palette = scene.palette;

    // Palette ranges must still belong to the same captured moby.
    for (std::size_t n = 0;
         n < live.size() && n < scene.addresses.size() && n < scene.joints.size();
         ++n) {
        const auto pose = scene.prev_poses.find(scene.addresses[n]);
        const int first = live[n].palette;
        const int count = scene.joints[n];
        const auto last = scene.prev_matrices.find(scene.addresses[n]);

        // Reused moby slots and changed skeletons start a new pose without blending.
        if (pose == scene.prev_poses.end() || first < 0 || count <= 0
            || pose->second.second != count || last == scene.prev_matrices.end()
            || !same_moby(last->second, scene.live[n].matrix)
            || static_cast<std::size_t>(first) + static_cast<std::size_t>(count) > palette.size()) {
            continue;
        }

        const scene::LevelScene::JointColumns* from =
            scene.prev_palette.data() + pose->second.first;

        // Joint columns interpolate independently inside the same skeleton.
        for (int component = 0; component < count; ++component) {
            scene::LevelScene::JointColumns& to =
                palette[static_cast<std::size_t>(first) + static_cast<std::size_t>(component)];

            // Matrix columns share the same display fraction as their object transform.
            for (std::size_t c = 0; c < to.size(); ++c) {
                to[c] = from[component][c] + (to[c] - from[component][c]) * alpha;
            }
        }
    }

    return palette;
}

// Interpolates both capsule endpoints and radii in the same display-time fraction as the moby.
void blend_capsules(const Kept& scene, float alpha, std::vector<renderer::ShadowCapsule>& shadows) {
    // Each capsule keeps its identity while the matching skeleton is unchanged.
    for (std::size_t n = 0; n < shadows.size(); ++n) {
        // Capsule endpoints use three world-space coordinates.
        for (std::size_t component = 0; component < shadows[n].a.size(); ++component) {
            shadows[n].a[component] =
                scene.prev_shadows[n].a[component]
                + (shadows[n].a[component] - scene.prev_shadows[n].a[component]) * alpha;
            shadows[n].b[component] =
                scene.prev_shadows[n].b[component]
                + (shadows[n].b[component] - scene.prev_shadows[n].b[component]) * alpha;
        }

        shadows[n].radius_a = scene.prev_shadows[n].radius_a
                              + (shadows[n].radius_a - scene.prev_shadows[n].radius_a) * alpha;
        shadows[n].radius_b = scene.prev_shadows[n].radius_b
                              + (shadows[n].radius_b - scene.prev_shadows[n].radius_b) * alpha;
    }
}

// Interpolates authored extrusion and vertical limits when both ticks contain valid shadow state.
void blend_shadow_state(const Kept& scene, float alpha, renderer::FrameInput& input) {
    // Missing shadow state cannot supply an interpolation endpoint.
    if (!scene.prev_shadow_state.enabled || !scene.shadow_state.enabled) {
        return;
    }

    // Extrusion has three world-space coordinates.
    for (std::size_t component = 0; component < input.shadow_extrusion.size(); ++component) {
        input.shadow_extrusion[component] = scene.prev_shadow_state.extrusion[component]
                                            + (scene.shadow_state.extrusion[component]
                                               - scene.prev_shadow_state.extrusion[component])
                                                  * alpha;
    }

    auto heights = scene.shadow_state.heights;

    // Both vertical limits follow the same display fraction as the shadow direction.
    for (std::size_t component = 0; component < heights.size(); ++component) {
        heights[component] =
            scene.prev_shadow_state.heights[component]
            + (scene.shadow_state.heights[component] - scene.prev_shadow_state.heights[component])
                  * alpha;
    }

    input.shadow_heights = heights;
}

}  // namespace

host::GameState blend_camera(const host::GameState& from, const host::GameState& to, float t) {
    host::GameState c = to;
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

void upload_instances(SceneFrame& frame) {
    // Loading scenes without models have no live instance buffers.
    if (frame.owner.level.models.empty()) {
        return;
    }

    const auto live = blend_instances(frame.scene, frame.output.alpha);
    std::vector<scene::LevelScene::JointColumns> blended;
    const bool blend = frame.output.alpha < 1.0f && !frame.scene.prev_poses.empty();

    // Exact tick frames borrow the captured palette without allocating a redundant copy.
    if (blend) {
        blended = blend_palette(frame.scene, live, frame.output.alpha);
    }

    const auto& palette = blend ? blended : frame.scene.palette;
    frame.owner.scene.set_palette(palette);

    // Motion history must match the exact pose drawn into depth and colour.
    if (frame.temporal) {
        frame.owner.temporal->set_scene(live, palette);
    }

    frame.owner.scene.set_instances(scene::Layer::Mobys, live);
}

void blend_shadows(SceneFrame& frame) {
    const Kept& scene = frame.scene;
    frame.shadows = scene.shadows;
    frame.input.shadow_capsules = frame.shadows;
    const float alpha = frame.output.alpha;

    // Replaced actors or capsule layouts must not interpolate through unrelated geometry.
    if (alpha >= 1.0f || scene.shadow_moby == 0 || scene.shadow_moby != scene.prev_shadow_moby
        || frame.shadows.size() != scene.prev_shadows.size()) {
        return;
    }

    const auto previous = scene.prev_matrices.find(scene.shadow_moby);
    const auto now = std::find(scene.addresses.begin(), scene.addresses.end(), scene.shadow_moby);

    // A matching address alone does not prove that a reused moby slot still holds the same actor.
    if (previous == scene.prev_matrices.end() || now == scene.addresses.end()
        || !same_moby(
            previous->second,
            scene.live[static_cast<std::size_t>(now - scene.addresses.begin())].matrix
        )) {
        return;
    }

    blend_capsules(scene, alpha, frame.shadows);
    blend_shadow_state(scene, alpha, frame.input);
}

}  // namespace openrac::frontend

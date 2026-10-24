// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The enhanced renderer's lighting of a level, registered with the scene's lighting hook
// (scene/level_lighting.h): the probe lighting (probe_lighting.h) and the early lighting
// (early_lighting.h) of one LevelScene, behind the calls the level renderer makes. Whoever makes a
// LevelScene registers one with use_scene_lighting (the game programs, the viewer, the tests); the
// probe bake an in-memory provider hands over goes through scene_lighting.

#pragma once

#include <string>
#include <vector>

#include "enhanced/lighting/probes.h"
#include "enhanced/viewer/early_lighting.h"
#include "enhanced/viewer/probe_lighting.h"
#include "scene/level_lighting.h"

namespace openrac::scene {
class LevelScene;
}  // namespace openrac::scene

namespace openrac::lighting {

// The probe and early lighting of one LevelScene. The scene owns it once registered.
class SceneLighting final : public scene::LevelLighting {
public:
    std::string mesh_fragment(std::string source) const override;
    std::string early_fragment(std::string source) const override;
    bool early_supported() const override;

    void load(const scene::LevelData& level) override;
    void release() override;

    bool has_data() const override;
    bool active() const override;
    int mode() const override;
    void set_mode(int mode) override;
    void set_lighting(bool enabled, float multiplier) override;
    void set_bias(float normal, float view) override;
    void set_spheres(bool shown) override;
    bool compute_selection() const override;
    void select_visible_regions(
        unsigned depth, const renderer::Camera& camera, int width, int height
    ) override;
    void update_regions(const renderer::Camera& camera, float seconds) override;

    void draw_spheres(const renderer::FrameInput& input, renderer::RenderState& state) override;
    void bind(
        const renderer::Shader& shader,
        scene::Layer layer,
        bool all_static_receivers,
        const renderer::Camera* camera,
        bool depth_matched
    ) const override;
    bool render_early(
        unsigned depth,
        const renderer::FrameInput& input,
        const renderer::RenderTarget& target,
        const std::function<void(const renderer::Shader&)>& bind_lighting,
        std::string& error
    ) override;
    void bind_early(
        const renderer::Shader& shader,
        const renderer::FrameInput& input,
        const renderer::RenderState& state
    ) const override;
    bool early_matches(
        const renderer::FrameInput& input, const renderer::RenderState& state
    ) const override;

    // Uploads a probe volume an in-memory provider made (and its dense volume); the provider has
    // checked the scene's and the bake's identity. False with `error` when it cannot be uploaded.
    bool set_probe_volume(
        const ProbeVolume& volume, std::string& error, const ProbeVolume* dense = nullptr
    );

    // Uploads streamed probe regions over `volume`. False with `error` when they cannot be.
    bool set_probe_regions(
        const ProbeVolume& volume, std::vector<ProbeVolume> regions, std::string& error
    );

private:
    ProbeLighting m_probes;          // The level's probe bake on the GPU.
    EarlyLighting m_early_lighting;  // The early-lit world's per-pixel lighting.
};

// Registers a SceneLighting with `scene`, replacing what it had, and returns it.
SceneLighting& use_scene_lighting(scene::LevelScene& scene);

// Returns the SceneLighting registered with `scene`, or null when it has another or none.
SceneLighting* scene_lighting(scene::LevelScene& scene);

}  // namespace openrac::lighting

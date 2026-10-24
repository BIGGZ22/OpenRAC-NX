// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The enhanced lighting of a level, scene_lighting.h: each call of the scene's hook made on the
// probe lighting or the early lighting as the level renderer made it itself before restructure
// package P2-39b.

#include "enhanced/viewer/scene_lighting.h"

#include <memory>
#include <utility>

#include "scene/level_renderer.h"

namespace openrac::lighting {

std::string SceneLighting::mesh_fragment(std::string source) const {
    return probe_mesh_fragment(std::move(source));
}

std::string SceneLighting::early_fragment(std::string source) const {
    return probe_early_fragment(std::move(source));
}

bool SceneLighting::early_supported() const {
    return EarlyLighting::supported();
}

void SceneLighting::load(const scene::LevelData& level) {
    m_probes.load(level);
}

void SceneLighting::release() {
    m_probes.release();
    m_early_lighting.release();
}

bool SceneLighting::has_data() const {
    return m_probes.has_data();
}

bool SceneLighting::active() const {
    return m_probes.active();
}

int SceneLighting::mode() const {
    return m_probes.mode;
}

void SceneLighting::set_mode(int mode) {
    m_probes.mode = mode;
}

void SceneLighting::set_lighting(bool enabled, float multiplier) {
    m_probes.enabled = enabled;
    m_probes.gi_multiplier = multiplier;
}

void SceneLighting::set_bias(float normal, float view) {
    m_probes.normal_bias = normal;
    m_probes.view_bias = view;
}

void SceneLighting::set_spheres(bool shown) {
    m_probes.show_spheres = shown || m_probes.environment_spheres;
}

bool SceneLighting::compute_selection() const {
    return m_probes.compute_selection();
}

void SceneLighting::select_visible_regions(
    unsigned depth, const renderer::Camera& camera, int width, int height
) {
    m_probes.select_visible_regions(depth, camera, width, height);
}

void SceneLighting::update_regions(const renderer::Camera& camera, float seconds) {
    const auto clip = renderer::multiply(camera.projection, camera.view);
    m_probes.update_regions(camera.position, seconds, &clip);
}

void SceneLighting::draw_spheres(const renderer::FrameInput& input, renderer::RenderState& state) {
    m_probes.draw_spheres(input, state);
}

void SceneLighting::bind(
    const renderer::Shader& shader,
    scene::Layer layer,
    bool all_static_receivers,
    const renderer::Camera* camera,
    bool depth_matched
) const {
    m_probes.bind(shader, layer, all_static_receivers, camera, depth_matched);
}

bool SceneLighting::render_early(
    unsigned depth,
    const renderer::FrameInput& input,
    const renderer::RenderTarget& target,
    const std::function<void(const renderer::Shader&)>& bind_lighting,
    std::string& error
) {
    return m_early_lighting.render(depth, input, target, bind_lighting, error);
}

void SceneLighting::bind_early(
    const renderer::Shader& shader,
    const renderer::FrameInput& input,
    const renderer::RenderState& state
) const {
    m_early_lighting.bind(shader, input, state);
}

bool SceneLighting::early_matches(
    const renderer::FrameInput& input, const renderer::RenderState& state
) const {
    return m_early_lighting.matches(input, state);
}

bool SceneLighting::set_probe_volume(
    const ProbeVolume& volume, std::string& error, const ProbeVolume* dense
) {
    return m_probes.upload(volume, error, dense);
}

bool SceneLighting::set_probe_regions(
    const ProbeVolume& volume, std::vector<ProbeVolume> regions, std::string& error
) {
    return m_probes.upload_regions(volume, std::move(regions), error);
}

SceneLighting& use_scene_lighting(scene::LevelScene& scene) {
    auto lighting = std::make_unique<SceneLighting>();
    SceneLighting& registered = *lighting;
    scene.set_level_lighting(std::move(lighting));
    return registered;
}

SceneLighting* scene_lighting(scene::LevelScene& scene) {
    return dynamic_cast<SceneLighting*>(scene.level_lighting());
}

}  // namespace openrac::lighting

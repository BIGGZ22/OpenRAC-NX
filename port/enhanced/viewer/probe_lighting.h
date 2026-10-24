// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <chrono>
#include <filesystem>

#include "enhanced/lighting/probe_regions.h"
#include "enhanced/lighting/probes.h"
#include "enhanced/viewer/probe_selection.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "scene/level.h"

namespace openrac::scene {
std::string probe_mesh_fragment();
std::string probe_early_fragment();
std::string probe_shader_source(std::string source);

// Where the player's own bakes are (the launcher's `--probes`, bake_all.py's output): one
// level_NN folder each, holding sparse.orpg. OPENRAC_PROBE_GI, when set, names one bake instead.
// A level without a bake, or with one that does not match it, keeps the original lighting.
void set_probe_root(std::filesystem::path root);
const std::filesystem::path& probe_root();

class ProbeLighting {
public:
    bool upload(
        const lighting::ProbeVolume& volume,
        std::string& error,
        const lighting::ProbeVolume* dense = nullptr
    );
    bool upload_regions(
        const lighting::ProbeVolume& volume,
        std::vector<lighting::ProbeVolume> regions,
        std::string& error
    );
    void update_regions(
        lighting::Float3 camera, float seconds = -1, const renderer::Mat4* world_to_clip = nullptr
    );

    const auto& region_slots() const { return m_selector.slots; }

    bool compute_selection() const { return active() && m_selection.available(); }

    void select_visible_regions(
        unsigned depth, const renderer::Camera& camera, int width, int height
    ) {
        if (compute_selection()) {
            m_selection.dispatch(depth, camera, width, height);
        }
    }

    // Activation through OPENRAC_PROBE_GI, else the level's bake under probe_root(). Missing or
    // invalid assets report why and leave the original lighting active.
    void load(const LevelData& level);
    void bind(
        const renderer::Shader& shader,
        Layer layer,
        bool all_static_receivers = false,
        const renderer::Camera* camera = nullptr,
        bool depth_matched = true
    ) const;
    void release();
    // Optional placement overlay, drawn after the world's moby layer.
    void draw_spheres(const renderer::FrameInput& input, renderer::RenderState& state);

    bool has_data() const { return m_textures[0] != 0; }

    bool active() const { return enabled && has_data() && mode != 0; }

    bool enabled = true;
    float gi_multiplier = 1.0f;
    float normal_bias = 0.5f;
    float view_bias = 0.5f;
    int mode = 1;
    bool backlight = false;
    bool show_spheres = false;
    bool environment_spheres = false;
    bool sphere_xray = false;

private:
    std::array<unsigned, 3> m_textures{};
    lighting::ProbeGrid m_grid;
    lighting::Float3 m_sunlight_scale{1, 1, 1};
    bool m_terrain_vertex_albedo = false;
    std::vector<lighting::ProbeVolume> m_regions;
    std::vector<lighting::ProbeGrid> m_region_grids;
    lighting::ProbeRegionSelector m_selector;
    std::array<int, lighting::kDenseProbeSlots> m_gpu_regions{-1, -1};
    std::size_t m_region_capacity = 0;
    ProbeSelection m_selection;
    bool m_full_residency = false;
    std::vector<unsigned> m_region_offsets;
    bool m_camera_started = false;
    std::chrono::steady_clock::time_point m_region_time{};
    void upload_region(unsigned slot, int region);
    void upload_volume(const lighting::ProbeVolume& volume, std::size_t offset);
    std::size_t m_sparse_capacity = 0;
    std::size_t region_offset(unsigned slot, int region) const;

    struct Sphere {
        std::array<float, 4> position_radius;
        std::array<float, 3> colour;
        int region = -1;
        float opacity = 1;
    };

    std::vector<Sphere> m_spheres, m_visible_spheres;
    renderer::Shader m_sphere_shader;
    unsigned m_sphere_vao = 0, m_sphere_buffer = 0, m_sphere_indices = 0;
};
}  // namespace openrac::scene

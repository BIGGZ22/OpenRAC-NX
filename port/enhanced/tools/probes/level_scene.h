// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "enhanced/lighting/bake.h"
#include "scene/level.h"

namespace openrac::lighting {

struct BakeSceneExport {
    BakeScene scene;
    Float3 bounds_min{}, bounds_max{};
    std::array<std::size_t, 4> instances{};  // terrain, ties, shrubs, selected static mobys
    std::size_t automatic_liquid_triangles = 0;
    std::size_t surface_instances = 0;  // animated surfaces supplied for static transport
    std::size_t skipped_blended = 0, skipped_degenerate = 0;
    std::size_t skipped_mobys = 0, skipped_glow = 0;
    std::size_t emissive_triangles = 0;
    std::uint64_t scene_hash = 0;
};

// Static exported render meshes plus explicitly selected unskinned mobys.
// Moving mobys, stand-in boxes and sky meshes are excluded from
// occlusion/transport geometry; the exported sky panorama supplies environment.
// Activation passes analyze_liquids=false: it fingerprints the same raw inputs
// without performing bake-only liquid analysis. Existing proxy surfaces still load.
// Format 3 has no terrain light selector, so the initial experiment explicitly
// selects one light set instead of summing every set in the level.
BakeSceneExport export_probe_scene(
    const viewer::LevelData& level,
    unsigned light_set,
    Float3 environment,
    bool analyze_liquids = true
);
// Visibility moments cover 1.5 cell diagonals; transport and occupancy trace
// the full scene. Scene bounds determine allocation, not the visibility range.
ProbeGrid sparse_probe_grid(const BakeSceneExport& scene, float spacing, unsigned resolution);
ProbeGrid sparse_probe_grid(const BakeSceneExport& scene, Float3 spacing, unsigned resolution);
// Cell-aligned local bounds; transport still uses the entire exported scene.
ProbeGrid bounded_probe_grid(
    const BakeSceneExport& scene, Float3 minimum, Float3 maximum, float spacing, unsigned resolution
);
ProbeGrid bounded_probe_grid(
    const BakeSceneExport& scene,
    Float3 minimum,
    Float3 maximum,
    Float3 spacing,
    unsigned resolution
);
std::uint64_t probe_bake_settings_hash(const ProbeGrid& grid, const BakeSettings& settings);

// Fingerprint canonical source files before loading and after a persistent batch.
// This guards preparation against concurrent edits without rebuilding transport geometry.
std::uint64_t probe_input_hash(const std::filesystem::path& directory);

}  // namespace openrac::lighting

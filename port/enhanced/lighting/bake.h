// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once
#include <functional>
#include <memory>
#include <optional>

#include "enhanced/lighting/probes.h"
#include "scene/emission.h"

namespace openrac::lighting {

// Regular subtexel integration footprint; part of the bake settings identity.
inline constexpr unsigned kProbeTexelSamplesPerAxis = 2;

struct BakeTriangle {
    // Deliberately no source shading normals: transport derives a geometric
    // normal from positions and faces it toward the incoming ray.
    std::array<Float3, 3> vertices;
    Float3 diffuse{0.5f, 0.5f, 0.5f};  // linear, unlit material colour
    std::array<Float2, 3> uv{};
    int texture = -1;
    float alpha_cutoff = 0.5f;
    // Authored unlit moby multipliers, not terrain/tie prelighting.
    std::array<std::array<float, 4>, 3> multipliers{{{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}}};
    Float3 emission{};          // linear radiance; independent of diffuse reflectance
    int emission_texture = -1;  // sRGB RGB mask, alpha comes from the diffuse material
    scene::EmissionMask emission_mask = scene::EmissionMask::None;
    bool emission_vertex_colour = false;
    std::array<Float3, 3> emission_colours{{{1, 1, 1}, {1, 1, 1}, {1, 1, 1}}};
    bool terrain_prelighting = false;
    std::array<Float3, 3> authored_colours{{{1, 1, 1}, {1, 1, 1}, {1, 1, 1}}};
    float diffuse_scale = 1;
};

struct BakeTexture {
    std::uint32_t width = 0, height = 0;
    std::vector<std::uint8_t> rgba;  // sRGB colour, linear alpha, top row first
};

// Colour controls shared by ordinary material and emission-mask sampling.
struct BakeMaterialOptions {
    Float3 texture_tint{1, 1, 1};       // GS/sRGB modulation before decoding.
    bool terrain_vertex_albedo = true;  // Decode authored terrain after filtering and tinting.
};

// Barycentric texture sampling: linear RGB and alpha. Repeat wrapping, as
// exported level materials use. Optional terrain_vertex_albedo filters encoded texels, applies
// the authored vertex/material tint, then decodes once, matching the runtime.
std::array<float, 4> bake_material(
    const struct BakeScene& scene,
    const BakeTriangle& triangle,
    float u,
    float v,
    const BakeMaterialOptions& options = {}
);
Float3 bake_emission(const struct BakeScene& scene, const BakeTriangle& triangle, float u, float v);

struct BakeSun {
    Float3 direction{0, 0, -1};  // direction light travels
    Float3 irradiance{1, 1, 1};
    bool primary = true;
};

struct BakePointEmitter {
    Float3 position{};
    Float3 intensity{};  // linear radiant intensity; irradiance falls as 1 / distance squared
};

struct BakeScene {
    std::vector<BakeTriangle> triangles;
    std::vector<BakeSun> suns;
    std::vector<BakePointEmitter> point_emitters;  // emission only; no occlusion geometry
    Float3 primary_directional_colour{};           // authored linear RGB, before irradiance * pi
    std::vector<BakeTexture> textures;
    Float3 environment{0.1f, 0.1f, 0.1f};  // uniform fallback when no sky is selected
    // Linear radiance, top row +Z, longitude zero +Y increasing toward +X.
    std::uint32_t sky_width = 0, sky_height = 0;
    std::vector<Float3> sky_radiance;
};

struct BakeSettings {
    bool use_level_sky = true;
    float sky_intensity = 1.0f;
    float sky_weight = 0.7f;            // remaining environment colour comes from the primary light
    Float3 sunlight_scale{1, 1, 1};     // fixed per-level linear RGB correction, primary sun only
    // Optional world-space light travel direction; absent preserves authored primary directions.
    std::optional<Float3> sunlight_direction;
    bool terrain_vertex_albedo = true;  // preserve authored terrain shading as an artistic tint
    std::uint32_t rays_per_probe = 1024;
    std::uint32_t bounces = 3;
    std::uint32_t seed = 1;
    float ray_bias = 0.002f;  // game world units
    float distance_exponent = 32;
    float min_inside_fraction = 0.75f;  // paired odd crossings; independent of winding
    std::uint32_t relocation_iterations = 4;
    float relocation_max_fraction = 0.45f;  // radius relative to minimum grid spacing
};

// Bilinear linear-light filtering, wrapping longitude and clamping latitude.
// Sky misses blend sky_intensity * sky with primary_directional_colour using sky_weight.
// Uniform scene.environment is used if disabled or no panorama is available.
Float3 sample_bake_environment(
    const BakeScene& scene, Float3 direction, const BakeSettings& settings
);

// Prepared, immutable transport state shared by a level's sparse and dense grids.
// The source scene must remain unchanged and outlive the baker; each bake has
// independent output, random seeds, progress and worker lifetime.
class ProbeBaker {
public:
    // Validate the source and build Embree acceleration once; invalid inputs throw. THREADS: the
    // acceleration build's threads (0: every CPU thread); bake() takes its own count.
    ProbeBaker(const BakeScene& scene, const BakeSettings& settings, std::uint32_t threads = 0);
    ~ProbeBaker();
    ProbeBaker(const ProbeBaker&) = delete;
    ProbeBaker& operator=(const ProbeBaker&) = delete;

    // Bake one grid with the prepared settings; invalid grids and tracing failures throw.
    ProbeVolume bake(
        const ProbeGrid& grid,
        const std::function<void(std::size_t, std::size_t)>& progress = {},
        std::uint32_t thread_count = 0
    ) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;  // owns acceleration, not the borrowed source scene
};

// Optional Embree 4 CPU implementation, linked only into offline bake tools.
// Irradiance includes environment and surface bounce light, excluding the
// analytic sun's direct contribution at the receiver. Geometry is two sided
// for transport. Placement uses paired ray-crossing parity, not normals or
// winding. Closed shells are interpreted as solids; open sheets do not define
// an interior. Sealed air rooms require separate occupancy information.
// thread_count is an execution control, excluded from bake identity: 0 uses
// available CPU threads, 1 is serial. Output is identical across thread counts.
// Progress is monotonic, may skip counts, and always runs on the calling thread.
ProbeVolume bake_probes(
    const ProbeGrid& grid,
    const BakeScene& scene,
    const BakeSettings& settings,
    const std::function<void(std::size_t, std::size_t)>& progress = {},
    std::uint32_t thread_count = 0
);

}  // namespace openrac::lighting

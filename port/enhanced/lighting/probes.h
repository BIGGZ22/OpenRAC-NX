// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace openrac::lighting {

using Float2 = std::array<float, 2>;
using Float3 = std::array<float, 3>;

// Only the current GPU-ready layout is accepted. Older assets must be rebaked.
inline constexpr std::uint32_t kProbeFormatVersion = 3;
inline constexpr std::uint32_t kProbeBakeVersion = 1;
inline constexpr unsigned kProbeAtlasTilesX = 64;

// Baking and CPU reference sampling use linear E; files store fifth-root E.
inline constexpr float kProbeIrradianceGamma = 5.0f;
float encode_probe_irradiance(float linear);

// Z up, right handed, in game world units. Maps store irradiance (not E/pi)
// and first-hit distance moments in world units and squared world units.
struct ProbeGrid {
    std::uint32_t bake_version = kProbeBakeVersion;
    Float3 origin{};
    Float3 spacing{16, 16, 16};
    std::array<std::uint32_t, 3> dimensions{2, 2, 2};
    std::uint32_t map_resolution = 6;  // interior; one gutter texel on every side
    float max_distance = 1024;
    std::string game_version = "rac1-pal";
    std::uint32_t level = 0;
    std::uint64_t scene_hash = 0;
    std::uint64_t settings_hash = 0;

    std::size_t probe_count() const;
    std::size_t texels_per_probe() const;
    void validate() const;
};

struct Probe {
    Float3 offset{};  // relocation from nominal grid position
    bool valid = false;
};

struct ProbeVolume {
    ProbeGrid grid;
    std::vector<Probe> probes;
    // Probe index x + nx*(y + ny*z), then row-major padded octahedral maps.
    std::vector<Float3> irradiance;
    std::vector<Float2> distance_moments;
    // Runtime owns the file buffer, with GPU-ready row-major texture planes.
    // Linear maps are only expanded by offline tools/reference tests.
    std::vector<std::uint8_t> gpu_bytes;
    std::size_t gpu_offset = 0;
    std::size_t atlas_rows() const;
    std::size_t atlas_capacity() const;
    std::span<const std::uint8_t> gpu_plane(unsigned plane) const;

    static ProbeVolume allocate(const ProbeGrid& grid);
    Float3 position(std::size_t probe) const;
    void validate() const;
};

Float2 oct_encode(Float3 direction);
Float3 oct_decode(Float2 uv);
// Populate gutters after filtering the interiors. Shared layout for CPU/GPU.
void update_probe_gutters(ProbeVolume& volume);

struct ProbeSample {
    Float3 irradiance{};
    float weight = 0;
    // Fraction of normal-weighted support surviving validity and occlusion.
    float confidence = 0;
    // Available normal-weighted support before visibility rejection.
    float coverage = 0;
};

// Empty means use original lighting. Biases are world-space offsets; view_offset
// is normally the normalized direction toward the camera times its bias. This is
// the CPU reference for fragment sampling. Cell selection, interpolation and
// moments use the biased position; membership and the plane guard use position.
std::optional<ProbeSample> sample_probes(
    const ProbeVolume& volume,
    Float3 position,
    Float3 normal,
    float normal_bias = 0.02f,
    Float3 view_offset = {}
);
std::optional<ProbeSample> sample_local_probes(
    const ProbeVolume& volume, Float3 position, Float3 normal
);

// Pair validation and fixed-local sampling. Camera-selected regions
// use probe_regions.h. Validate once at activation; blend irradiance, not moments.
// Missing dense probes fade to coarse. Valid but occluded dense support stays
// dark, with a smooth low-visibility fade rather than bright coarse fallback.
void validate_probe_pair(const ProbeVolume& coarse, const ProbeVolume& dense);
float probe_boundary_weight(const ProbeGrid& grid, Float3 position);
std::optional<ProbeSample> sample_probe_volumes(
    const ProbeVolume& coarse,
    const ProbeVolume* dense,
    Float3 position,
    Float3 normal,
    float normal_bias = 0.02f,
    Float3 view_offset = {}
);

// Portable little-endian format; no C++ struct dumps or platform-size fields.
std::vector<std::uint8_t> encode_probes(const ProbeVolume& volume);
ProbeVolume decode_probes(std::span<const std::uint8_t> bytes);
// Move the read buffer into the runtime resource without unpacking map texels.
ProbeVolume decode_gpu_probes(std::vector<std::uint8_t> bytes);
bool probe_identity_matches(
    const ProbeVolume& volume,
    const std::string& game_version,
    std::uint32_t level,
    std::uint64_t scene_hash,
    std::uint64_t settings_hash
);

}  // namespace openrac::lighting

// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>

#include "enhanced/tools/probes/level_scene.h"

namespace openrac::lighting {
struct ProbeActivation {
    BakeSettings bake;
    unsigned light_set = 0;
    Float3 environment{0.1f, 0.1f, 0.1f};
};

// Per-level probe-bake.json, then the mod profile, then caller overrides.
struct LevelBakeProfile {
    ProbeActivation activation;
    Float3 sparse_spacing{256, 256, 256};
    Float3 dense_spacing{4, 4, 4};
    unsigned map_resolution = 6;
};

LevelBakeProfile load_level_bake_profile(
    const std::filesystem::path& directory, const std::filesystem::path& overrides = {}
);

// Retain GPU-ready file planes; offline diagnostics can request linear maps.
// Reject wrong game/level, stale scene and bake settings before texture upload.
// Never invokes Embree.
ProbeVolume load_level_probes(
    const std::filesystem::path& path,
    const scene::LevelData& level,
    const ProbeActivation& expected,
    bool expand_linear_maps = false
);
}  // namespace openrac::lighting

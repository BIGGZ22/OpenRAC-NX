// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "enhanced/lighting/probes.h"

namespace openrac::lighting {
inline constexpr unsigned kDenseProbeSlots = 2;

struct ProbeRegionSlot {
    int region = -1;
    float weight = 0;
};

// Visible boxes rank before offscreen boxes, then by camera distance with
// resident hysteresis. Camera distance is measured to each nominal volume box. A slot is never
// reused while its previous region still contributes light.
class ProbeRegionSelector {
public:
    std::array<ProbeRegionSlot, kDenseProbeSlots> slots{};
    float full_distance = 32, fade_distance = 96, fade_seconds = 0.5f, hysteresis = 8;
    void update(
        std::span<const ProbeGrid> regions,
        Float3 camera,
        float seconds,
        const std::array<float, 16>* world_to_clip = nullptr
    );
};

float probe_region_distance(const ProbeGrid& grid, Float3 point);
// Conservative AABB/GL clip-frustum intersection; column-major, including
// near/far for reversed depth. Missing/invalid camera matrices use distance only.
bool probe_region_in_frustum(const ProbeGrid& grid, const std::array<float, 16>& world_to_clip);

struct WeightedProbeRegion {
    const ProbeVolume* volume = nullptr;
    float weight = 0;
};

// Order-independent overlap of up to two locals, with a sparse fallback.
std::optional<ProbeSample> sample_probe_regions(
    const ProbeVolume& coarse,
    std::span<const WeightedProbeRegion> regions,
    Float3 position,
    Float3 normal
);
}  // namespace openrac::lighting

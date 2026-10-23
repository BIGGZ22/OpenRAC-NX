// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/lighting/probe_regions.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <tuple>

namespace openrac::lighting {
float probe_region_distance(const ProbeGrid& grid, Float3 point) {
    float squared = 0;
    for (unsigned a = 0; a < 3; ++a) {
        if (!std::isfinite(point[a])) {
            return std::numeric_limits<float>::infinity();
        }
        const float end = grid.origin[a] + grid.spacing[a] * float(grid.dimensions[a] - 1);
        const float d = std::max({grid.origin[a] - point[a], point[a] - end, 0.0f});
        squared += d * d;
    }
    return std::sqrt(squared);
}

bool probe_region_in_frustum(const ProbeGrid& grid, const std::array<float, 16>& clip) {
    if (!std::all_of(clip.begin(), clip.end(), [](float x) { return std::isfinite(x); })) {
        return true;  // Invalid camera: do not demote arbitrary regions.
    }
    // GL clip bounds are -w..w, also when the depth row is reversed.
    // Reject only if the entire box is outside a plane, never by its centre.
    for (unsigned axis = 0; axis < 3; ++axis) {
        for (float sign : {-1.0f, 1.0f}) {
            float maximum = clip[15] + sign * clip[12 + axis];
            for (unsigned a = 0; a < 3; ++a) {
                const float normal = clip[4 * a + 3] + sign * clip[4 * a + axis];
                const float end = grid.origin[a] + grid.spacing[a] * float(grid.dimensions[a] - 1);
                maximum += normal * (normal >= 0 ? end : grid.origin[a]);
            }
            if (maximum < -1e-4f) {
                return false;
            }
        }
    }
    return true;
}

void ProbeRegionSelector::update(
    std::span<const ProbeGrid> regions,
    Float3 camera,
    float seconds,
    const std::array<float, 16>* world_to_clip
) {
    if (!(full_distance >= 0 && fade_distance > full_distance && fade_seconds > 0 && hysteresis >= 0
          && std::isfinite(seconds) && seconds >= 0)) {
        throw std::runtime_error("invalid probe region fade settings");
    }
    std::vector<std::tuple<bool, float, int>> candidates;
    std::vector<float> targets(regions.size());
    for (std::size_t i = 0; i < regions.size(); ++i) {
        const float distance = probe_region_distance(regions[i], camera);
        if (distance >= fade_distance) {
            continue;
        }
        const float t =
            std::clamp((fade_distance - distance) / (fade_distance - full_distance), 0.0f, 1.0f);
        targets[i] = t * t * (3 - 2 * t);
        const bool resident = std::any_of(slots.begin(), slots.end(), [&](const auto& s) {
            return s.region == static_cast<int>(i);
        });
        const bool offscreen =
            world_to_clip && !probe_region_in_frustum(regions[i], *world_to_clip);
        candidates
            .emplace_back(offscreen, distance - (resident ? hysteresis : 0), static_cast<int>(i));
    }
    std::sort(candidates.begin(), candidates.end());
    if (candidates.size() > kDenseProbeSlots) {
        candidates.resize(kDenseProbeSlots);
    }
    const auto wanted = [&](int id) {
        return std::any_of(candidates.begin(), candidates.end(), [&](const auto& c) {
            return std::get<2>(c) == id;
        });
    };
    const float step = std::min(seconds / fade_seconds, 1.0f);
    for (auto& slot : slots) {
        if (slot.region < 0) {
            continue;
        }
        const float target = wanted(slot.region) ? targets[slot.region] : 0;
        slot.weight += std::clamp(target - slot.weight, -step, step);
        if (slot.weight <= 0 && !wanted(slot.region)) {
            slot = {};
        }
    }
    for (const auto& [offscreen, score, id] : candidates) {
        (void)offscreen;
        (void)score;
        if (std::any_of(slots.begin(), slots.end(), [&](const auto& s) {
                return s.region == id;
            })) {
            continue;
        }
        const auto empty =
            std::find_if(slots.begin(), slots.end(), [](const auto& s) { return s.region < 0; });
        if (empty != slots.end()) {
            *empty = {id, std::min(targets[id], step)};
        }
    }
}

std::optional<ProbeSample> sample_probe_regions(
    const ProbeVolume& coarse, std::span<const WeightedProbeRegion> regions, Float3 p, Float3 n
) {
    if (regions.size() > kDenseProbeSlots) {
        throw std::runtime_error("too many active probe regions");
    }
    auto base = sample_probes(coarse, p, n);
    ProbeSample local{};
    float total = 0;
    for (const auto& region : regions) {
        if (!region.volume || region.weight <= 0) {
            continue;
        }
        // Valid but occluded local support stays dark, never bright coarse GI.
        const auto sample = sample_local_probes(*region.volume, p, n);
        if (!sample) {
            continue;
        }
        const float c = sample->coverage;
        const float w = std::clamp(region.weight, 0.0f, 1.0f)
                        * probe_boundary_weight(region.volume->grid, p) * c * c * (3 - 2 * c);
        for (unsigned a = 0; a < 3; ++a) {
            local.irradiance[a] += sample->irradiance[a] * w;
        }
        local.weight += sample->weight * w;
        local.confidence += sample->confidence * w;
        local.coverage += sample->coverage * w;
        total += w;
    }
    if (total <= 0) {
        return base;
    }
    for (auto& e : local.irradiance) {
        e /= total;
    }
    local.weight /= total;
    local.confidence /= total;
    local.coverage /= total;
    if (!base) {
        return local;
    }
    const float blend = std::min(total, 1.0f);
    for (unsigned a = 0; a < 3; ++a) {
        base->irradiance[a] = base->irradiance[a] * (1 - blend) + local.irradiance[a] * blend;
    }
    base->weight = base->weight * (1 - blend) + local.weight * blend;
    base->confidence = base->confidence * (1 - blend) + local.confidence * blend;
    base->coverage = base->coverage * (1 - blend) + local.coverage * blend;
    return base;
}
}  // namespace openrac::lighting

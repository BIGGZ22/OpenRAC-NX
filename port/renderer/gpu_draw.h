// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace openrac::renderer {

// Native GPU instance-culling inputs. Geometry and material policy stay in
// the scene renderer; the Vulkan backend owns allocation and submission.
struct GpuCullBounds {
    std::array<float, 3> low{}, high{};
    std::uint32_t occlusion = 0x7f80;
    std::uint32_t flags = 0;  // 1: valid box, 2: UI (bypass world visibility)
};

static_assert(sizeof(GpuCullBounds) == 32);

struct GpuCullView {
    std::array<std::array<float, 4>, 6> planes{};
    std::array<std::uint8_t, 128> pvs{};
    unsigned plane_count = 0;
    bool use_pvs = false;
    bool bypass_ui = true;
    // The zero-coverage cull (cull-zero-coverage, docs/port/RENDER_PERF.md, R-19): an instance
    // whose projected bounds hold no sample position of the view is not drawn. The caller sets
    // `coverage` once it has filled in what the view's draws rasterise with.
    bool coverage = false;
    std::array<float, 16> clip{};     // The draws' view-projection, column-major.
    std::array<float, 4> viewport{};  // The draws' viewport: x, y, width, height in pixels.
    unsigned samples = 1;             // The target's samples a pixel; above 1 nothing is culled.
};

// The sub-pixel margin around projected bounds: the rasteriser snaps vertices to at least a
// sixteenth of a pixel (Vulkan's 4 bits, documented), doubled for the projection's float error.
inline constexpr float kCoverageMargin = 0.125f;

// The smallest clip-space w a bounds corner may have for the test to be made: a corner on or
// behind the eye's plane projects without limit, so such bounds are always kept.
inline constexpr float kCoverageMinimumW = 1.0e-6f;

// Returns whether an instance whose world bounds are `low` to `high` may light a sample of
// `view`: the CPU reference of the cull shader's zero-coverage test (gpu_draw.inc, which computes
// the same in the same order). Its triangles lie inside the box, so with every corner in front of
// the eye their fragments lie inside the projected corners' rectangle; with one sample a pixel
// the samples are the pixel centres (k + 0.5), so a rectangle, widened by kCoverageMargin, that
// holds no centre in x or none in y lights nothing. True whenever the test cannot be made.
inline bool covers_a_sample(
    const GpuCullView& view, const std::array<float, 3>& low, const std::array<float, 3>& high
) {
    // Off, or multisampled: samples may sit anywhere in a pixel, so every overlap could light one.
    if (!view.coverage || view.samples != 1) {
        return true;
    }

    std::array<float, 2> least{1.0e30f, 1.0e30f};
    std::array<float, 2> most{-1.0e30f, -1.0e30f};

    // The eight corners, projected to the viewport's pixels.
    for (int corner = 0; corner < 8; ++corner) {
        const float x = (corner & 1) != 0 ? high[0] : low[0];
        const float y = (corner & 2) != 0 ? high[1] : low[1];
        const float z = (corner & 4) != 0 ? high[2] : low[2];
        const auto& m = view.clip;
        const float cx = ((m[0] * x + m[4] * y) + m[8] * z) + m[12];
        const float cy = ((m[1] * x + m[5] * y) + m[9] * z) + m[13];
        const float cw = ((m[3] * x + m[7] * y) + m[11] * z) + m[15];

        // A corner at or behind the eye: the projection is unbounded, the instance kept.
        if (!(cw > kCoverageMinimumW)) {
            return true;
        }

        const std::array<float, 2> ndc{cx / cw, cy / cw};

        // Each axis to the viewport's pixels.
        for (int k = 0; k < 2; ++k) {
            const float pixel = view.viewport[k] + (ndc[k] * 0.5f + 0.5f) * view.viewport[k + 2];
            least[k] = std::min(least[k], pixel);
            most[k] = std::max(most[k], pixel);
        }
    }

    // A pixel centre k + 0.5 inside the widened rectangle, on each axis.
    for (int k = 0; k < 2; ++k) {
        const float first = std::ceil(least[k] - kCoverageMargin - 0.5f);
        const float last = std::floor(most[k] + kCoverageMargin - 0.5f);

        // None on this axis: no sample is inside.
        if (!(last >= first)) {
            return false;
        }
    }

    return true;
}

struct GpuInstanceBatch {
    std::uint64_t recording = 0;
    std::uint32_t page = 0, group = 0;

    explicit operator bool() const { return recording != 0; }
};

}  // namespace openrac::renderer

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once

#include <array>
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
};

struct GpuInstanceBatch {
    std::uint64_t recording = 0;
    std::uint32_t page = 0, group = 0;

    explicit operator bool() const { return recording != 0; }
};

}  // namespace openrac::renderer

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once
#include <cstring>

#include "common/stats.h"
#include "renderer/gl.h"
#if defined(OPENRAC_VULKAN)
#include "renderer/vulkan/vkgl.h"
#endif
namespace openrac::test {
inline void gpu_material_reference(bool legacy) {
    stats::probe("OPENRAC_GPU_MATERIALS_OFF", "Legacy material shader reference (reload shaders)");
    for (auto* probe : stats::probes()) {
        if (std::strcmp(probe->variable, "OPENRAC_GPU_MATERIALS_OFF") == 0) {
            probe->on = legacy;
        }
    }
}

inline void gpu_draw_reference(bool cpu) {
    stats::probe("OPENRAC_GPU_DRIVEN_OFF", "CPU reference for GPU draw comparisons");
    for (auto* probe : stats::probes()) {
        if (std::strcmp(probe->variable, "OPENRAC_GPU_DRIVEN_OFF") == 0) {
            probe->on = cpu;
        }
    }
}

inline bool gpu_draw_active() {
#if defined(OPENRAC_VULKAN)
    return vkgl::gpu_driven_active();
#else
    return false;
#endif
}
}  // namespace openrac::test

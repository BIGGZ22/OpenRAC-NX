// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Player choices for temporal reconstruction; availability belongs to the enhanced provider.
#pragma once

#include <cstdint>

namespace openrac::renderer {
// Render-to-display ratios follow AMD's temporal-upscaler quality presets.
enum class TemporalQuality : std::uint8_t {
    Off,
    NativeAA,
    Quality,
    Balanced,
    Performance,
    Count,
};
// Persistent keys shared by the settings file and launcher.
inline constexpr const char* kTemporalKeys[]{
    "off", "native-aa", "quality", "balanced", "performance"
};
// Menu labels in the same order as TemporalQuality.
inline constexpr const char* kTemporalNames[]{
    "Off", "Native AA", "Quality", "Balanced", "Performance"
};
}  // namespace openrac::renderer

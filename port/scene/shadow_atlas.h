// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>

namespace openrac::scene {
inline constexpr int kShadowAtlasSize = 3072;
// x, y, width, height; four sun cascades followed by the hero view.
inline constexpr std::array<std::array<int, 4>, 5> kShadowAtlasRects{
    {{0, 0, 2048, 2048},
     {2048, 0, 1024, 1024},
     {2048, 1024, 1024, 1024},
     {0, 2048, 1024, 1024},
     {1024, 2048, 512, 512}}
};
}  // namespace openrac::scene

// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
namespace openrac::game {
// The angle is an 8-bit index: evaluate its 256 possibilities once.
inline const std::array<float, 2>& particle_rotation(std::uint8_t angle) {
    static const auto table = [] {
        std::array<std::array<float, 2>, 256> values{};
        for (unsigned i = 0; i < values.size(); ++i) {
            const float theta = static_cast<float>(i) * 6.2831853f / 256.0f;
            values[i] = {std::cos(theta), std::sin(theta)};
        }
        return values;
    }();
    return table[angle];
}
}

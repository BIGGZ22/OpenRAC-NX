// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace openrac::viewer {

// RAC1's sparse baked camera grid; see docs/engine/formats/OCCLUSION.md.
// No previous-cell state: unsupported camera positions conservatively show everything.
struct Pvs {
    using Mask = std::array<std::uint8_t, 128>;
    static constexpr std::uint16_t always = 0x7f80;
    std::vector<std::uint8_t> grid;

    static Mask all_visible() {
        Mask mask;
        mask.fill(255);
        return mask;
    }

    static bool visible(const Mask& mask, std::uint16_t word) {
        const unsigned byte = word >> 8, bit = word & 255;
        // Uninitialised/malformed words must never hide an object.
        return byte >= mask.size() || !bit || (bit & (bit - 1)) || (mask[byte] & bit);
    }

    Mask select(const std::array<float, 3>& camera) const {
        Mask mask{};
        std::array<int, 3> cell;
        for (unsigned k = 0; k < 3; ++k) {
            const double c = double(camera[k]) * .25;
            if (!std::isfinite(c) || c < std::numeric_limits<int>::min() + 1.0
                || c > std::numeric_limits<int>::max() - 1.0) {
                return all_visible();
            }
            cell[k] = static_cast<int>(c);  // retail truncates, including negative coordinates
        }
        auto lookup = [&](const std::array<int, 3>& c, Mask& out) {
            auto u16 = [&](std::size_t at) -> unsigned {
                return grid[at] | (unsigned(grid[at + 1]) << 8);
            };
            if (grid.size() < 8) {
                return false;
            }
            const std::size_t masks = u16(0) | (std::size_t(u16(2)) << 16);
            if (masks < 8 || masks > grid.size()) {
                return false;
            }
            std::size_t node = 4;
            for (int axis = 2; axis >= 0; --axis) {
                if (node > masks || masks - node < 4) {
                    return false;
                }
                const auto offset = std::int64_t(c[axis]) - u16(node);
                const unsigned count = u16(node + 2);
                if (count > (masks - node - 4) / 2 || offset < 0 || offset >= count) {
                    return false;
                }
                const unsigned slot = u16(node + 4 + std::size_t(offset) * 2);
                if (axis) {
                    if (!slot) {
                        return false;
                    }
                    node = std::size_t(slot) * 4;
                } else {
                    const std::size_t at = masks + std::size_t(slot) * out.size();
                    if (slot == 0xffff || at > grid.size() || grid.size() - at < out.size()) {
                        return false;
                    }
                    for (std::size_t i = 0; i < out.size(); ++i) {
                        out[i] |= grid[at + i];
                    }
                }
            }
            return true;
        };
        if (!lookup(cell, mask)) {
            bool found = false;
            // Retail's normal fallback unions adjacent cells. Try all six conservatively.
            for (unsigned axis = 0; axis < 3; ++axis) {
                for (const int step : {-1, 1}) {
                    auto neighbour = cell;
                    neighbour[axis] += step;
                    found |= lookup(neighbour, mask);
                }
            }
            if (!found) {
                return all_visible();
            }
        }
        mask.back() |= 128;
        return mask;
    }
};

}  // namespace openrac::viewer

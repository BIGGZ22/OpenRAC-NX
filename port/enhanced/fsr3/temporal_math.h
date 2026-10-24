// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Extents and camera transforms shared by temporal rendering and its synthetic tests.
#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include "renderer/math.h"
#include "renderer/temporal_settings.h"

namespace openrac::enhanced::fsr3_detail {
// Returns AMD's preset render extent for a positive display extent, rounded down and clamped to one.
inline std::array<int, 2> render_extent(
    std::array<int, 2> display, renderer::TemporalQuality quality
) {
    // Off and native AA retain display resolution; remaining ratios are AMD's published presets.
    constexpr std::array<float, 5> kRatios{1, 1, 1.5f, 1.7f, 2};
    const auto index = std::min(static_cast<std::size_t>(quality), kRatios.size() - 1);
    return {
        std::max(1, static_cast<int>(static_cast<float>(display[0]) / kRatios[index])),
        std::max(1, static_cast<int>(static_cast<float>(display[1]) / kRatios[index]))
    };
}

// Row-major matrix and identity side by side, reduced in double precision for finite reversed depth.
using AugmentedMatrix = std::array<std::array<double, 8>, 4>;

// Places a column-major camera beside an identity matrix for pivoted row reduction.
inline AugmentedMatrix augmented(const renderer::Mat4& source) {
    AugmentedMatrix rows{};

    // The reduction operates on rows, unlike the renderer's column-major matrices.
    for (std::size_t row = 0; row < rows.size(); ++row) {
        // Preserve the camera coefficients before adding the identity block.
        for (std::size_t column = 0; column < rows.size(); ++column) {
            rows[row][column] = source[column * rows.size() + row];
        }

        rows[row][row + rows.size()] = 1;
    }

    return rows;
}

// Inverts a finite nonsingular matrix once per displayed frame; false rejects an invalid camera.
inline bool invert(const renderer::Mat4& source, renderer::Mat4& inverse) {
    AugmentedMatrix rows = augmented(source);

    // Partial pivoting avoids dividing by a small diagonal when another row has a stronger coefficient.
    for (std::size_t column = 0; column < rows.size(); ++column) {
        auto pivot = std::max_element(
            rows.begin() + static_cast<std::ptrdiff_t>(column),
            rows.end(),
            [column](const auto& a, const auto& b) {
                return std::abs(a[column]) < std::abs(b[column]);
            }
        );

        // A singular or nonfinite camera cannot produce usable world positions or motion.
        if (!std::isfinite((*pivot)[column]) || std::abs((*pivot)[column]) < 1e-12) {
            return false;
        }

        std::swap(rows[column], *pivot);
        const double scale = rows[column][column];

        // A unit pivot lets the remaining rows eliminate this column by subtraction.
        for (auto& value : rows[column]) {
            value /= scale;
        }

        // Leave the pivot row unchanged while eliminating its column from every other row.
        for (std::size_t row = 0; row < rows.size(); ++row) {
            const double factor = row == column ? 0 : rows[row][column];

            // Apply the same row operation to the camera and the augmented identity.
            for (std::size_t n = 0; n < rows[row].size(); ++n) {
                rows[row][n] -= factor * rows[column][n];
            }
        }
    }

    // The right-hand block is the inverse, returned in the renderer's column-major order.
    for (std::size_t column = 0; column < rows.size(); ++column) {
        // Converting storage order leaves the mathematical matrix unchanged.
        for (std::size_t row = 0; row < rows.size(); ++row) {
            inverse[column * rows.size() + row] =
                static_cast<float>(rows[row][column + rows.size()]);
        }
    }

    return true;
}
}  // namespace openrac::enhanced::fsr3_detail

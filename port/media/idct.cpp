// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/idct.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The inverse DCT (idct.h).

#include "media/idct.h"

#include <cmath>
#include <numbers>

namespace openrac::media {

namespace {

using Basis = std::array<std::array<float, 8>, 8>;

// basis[k][n] = C(k) / 2 * cos((2n + 1) k pi / 16), C(0) = 1 / sqrt(2), else 1.
const Basis& basis() {
    static const Basis b = [] {
        Basis t{};

        for (int k = 0; k < 8; ++k) {
            const double c = k == 0 ? 1.0 / std::numbers::sqrt2 : 1.0;

            for (int n = 0; n < 8; ++n) {
                t[k][n] = static_cast<float>(
                    c / 2.0 * std::cos((2 * n + 1) * k * std::numbers::pi / 16.0)
                );
            }
        }

        return t;
    }();
    return b;
}

}  // namespace

void inverse_dct(std::array<std::int32_t, 64>& block) {
    const Basis& b = basis();
    std::array<float, 64> tmp{};

    // Rows: tmp[v][x] = sum over u of F[v][u] * B[u][x]; all-zero rows stay zero.
    for (int v = 0; v < 8; ++v) {
        float* out = &tmp[v * 8];

        for (int u = 0; u < 8; ++u) {
            const std::int32_t c = block[v * 8 + u];

            if (c == 0) {
                continue;
            }

            const auto cf = static_cast<float>(c);

            for (int x = 0; x < 8; ++x) {
                out[x] += cf * b[u][x];
            }
        }
    }

    // Columns: f[y][x] = sum over v of tmp[v][x] * B[v][y].
    std::array<float, 64> out{};

    for (int v = 0; v < 8; ++v) {
        const float* t = &tmp[v * 8];
        bool zero = true;

        for (int x = 0; x < 8; ++x) {
            zero = zero && t[x] == 0.0f;
        }

        if (zero) {
            continue;
        }

        for (int y = 0; y < 8; ++y) {
            const float w = b[v][y];
            float* o = &out[y * 8];

            for (int x = 0; x < 8; ++x) {
                o[x] += t[x] * w;
            }
        }
    }

    for (int k = 0; k < 64; ++k) {
        block[k] = static_cast<std::int32_t>(std::floor(out[k] + 0.5f));
    }
}

}  // namespace openrac::media

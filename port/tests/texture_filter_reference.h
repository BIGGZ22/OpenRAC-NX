// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Independent scalar texture reconstruction used to check GPU filtering and original alpha.
#pragma once

#include <array>
#include <span>
#include <vector>

namespace openrac::test::texture_reference {

// Borrows one square RGBA8 image for the duration of a reference calculation.
struct Image {
    std::span<const unsigned char> pixels;  // Caller-owned source pixels.
    int size;                               // Width and height in texels.
    bool repeat_s = true;                   // Repeats U instead of clamping.
    bool repeat_t = true;                   // Repeats V instead of clamping.

    // Returns one wrapped or clamped channel, scaled to [0,1].
    double texel(int x, int y, int channel) const;
    // Returns wrapped or clamped RGB without using production shader code.
    std::array<double, 3> rgb(int x, int y) const;
    // Returns the scalar luma used by the independent reconstruction kernel.
    double luminance(int x, int y) const;
};

// Returns the quantised 2x RGB reconstruction with directional debanding and opaque alpha.
std::vector<unsigned char> edge_reference(const Image& image);

// Bilinearly samples one channel with the image's wrapping rules.
double reference(const Image& image, double u, double v, int channel);

}  // namespace openrac::test::texture_reference

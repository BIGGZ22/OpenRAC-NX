// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Scalar reconstruction and debanding oracles, independent of the production GLSL implementation.
#include "tests/texture_filter_reference.h"

#include <algorithm>
#include <cmath>

namespace openrac::test::texture_reference {
namespace {

// Coordinates and luma gradient shared by the three channels of one reconstructed texel.
struct Kernel {
    int x, y;                     // Lower source neighbour.
    double fx, fy;                // Fractional source coordinates.
    double gx, gy;                // Luma gradient.
    double magnitude, coherence;  // Gradient length and directional confidence.
};

// Computes the original scalar kernel without reading any production shader state.
Kernel kernel(const Image& image, int x, int y) {
    const double px = (x + .5) * .5 - .5, py = (y + .5) * .5 - .5;
    const int bx = static_cast<int>(std::floor(px)), by = static_cast<int>(std::floor(py));
    const double l00 = image.luminance(bx, by), l10 = image.luminance(bx + 1, by);
    const double l01 = image.luminance(bx, by + 1), l11 = image.luminance(bx + 1, by + 1);
    const double gx = l10 - l00 + l11 - l01, gy = l01 - l00 + l11 - l10;
    const double magnitude = std::hypot(gx, gy);
    const double energy =
        std::abs(l10 - l00) + std::abs(l11 - l01) + std::abs(l01 - l00) + std::abs(l11 - l10);
    return {
        bx,
        by,
        px - bx,
        py - by,
        gx,
        gy,
        magnitude,
        std::clamp(magnitude / std::max(energy, .0001), 0.0, 1.0)
    };
}

// Returns the compact two-lobe sinc weight, with its analytic value at zero.
double sinc_weight(double radius) {
    // The filter has no support outside two source texels.
    if (radius >= 2) {
        return 0;
    }
    // Avoid division by zero at the kernel's centre.
    if (radius < .0001) {
        return 1;
    }
    constexpr double kPi = 3.141592653589793;
    return std::sin(kPi * radius) * std::sin(.5 * kPi * radius)
           / (.5 * kPi * kPi * radius * radius);
}

// Integrates the 4x4 neighbourhood in the same scalar accumulation order as the original oracle.
double directional_sample(const Image& image, const Kernel& point, int channel, double fallback) {
    const double nx = point.gx / point.magnitude, ny = point.gy / point.magnitude;
    double sum = 0, total = 0;
    // Each tap uses the source wrapping rules, independent of the output cache.
    for (int j = -1; j <= 2; ++j) {
        for (int i = -1; i <= 2; ++i) {
            const double dx = i - point.fx, dy = j - point.fy;
            const double radius = std::hypot(
                (-dx * ny + dy * nx) / std::lerp(1.0, 1.5, point.coherence),
                (dx * nx + dy * ny) / std::lerp(1.0, .75, point.coherence)
            );
            const double weight = sinc_weight(radius);
            sum += image.texel(point.x + i, point.y + j, channel) * weight;
            total += weight;
        }
    }
    // A degenerate support retains bilinear sampling.
    return total > .0001 ? sum / total : fallback;
}

// Reconstructs one channel and constrains ringing to the four neighbouring source values.
unsigned char reconstructed_channel(const Image& image, const Kernel& point, int channel) {
    const double c00 = image.texel(point.x, point.y, channel);
    const double c10 = image.texel(point.x + 1, point.y, channel);
    const double c01 = image.texel(point.x, point.y + 1, channel);
    const double c11 = image.texel(point.x + 1, point.y + 1, channel);
    double value =
        std::lerp(std::lerp(c00, c10, point.fx), std::lerp(c01, c11, point.fx), point.fy);
    // Flat neighbourhoods retain bilinear sampling.
    if (point.magnitude >= .0001) {
        value = directional_sample(image, point, channel, value);
        value = std::clamp(value, std::min({c00, c10, c01, c11}), std::max({c00, c10, c01, c11}));
    }
    return static_cast<unsigned char>(std::lround(value * 255));
}

// Measures the maximum RGB channel difference for the directional debanding gate.
double distance(const std::array<double, 3>& a, const std::array<double, 3>& b) {
    return std::max({std::abs(a[0] - b[0]), std::abs(a[1] - b[1]), std::abs(a[2] - b[2])});
}

// Evaluates the cubic threshold used by the independent debanding oracle.
double smooth(double a, double b, double value) {
    const double t = std::clamp((value - a) / (b - a), 0.0, 1.0);
    return t * t * (3 - 2 * t);
}

// Unnormalised correction and total accepted weight for one reconstructed texel.
struct Adjustment {
    std::array<double, 3> sum{};  // Weighted RGB deltas.
    double total = 0;             // Weight shared by all channels.
};

// Samples both sides of four directions without blurring a larger colour discontinuity.
Adjustment adjustment(const Image& image, int x, int y) {
    constexpr int kDirections[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    const auto centre = image.rgb(x / 2, y / 2);
    Adjustment result;
    // Two radii detect the low-amplitude bands covered by the fixture.
    for (const auto& direction : kDirections) {
        for (const int radius : {2, 4}) {
            const auto a = image.rgb(x / 2 - direction[0] * radius, y / 2 - direction[1] * radius);
            const auto b = image.rgb(x / 2 + direction[0] * radius, y / 2 + direction[1] * radius);
            const double weight =
                (1
                 - smooth(
                     4.0 / 255, 10.0 / 255, std::max(distance(centre, a), distance(centre, b))
                 ))
                * smooth(.5 / 255, 2.0 / 255, distance(a, b));
            // Every RGB channel shares the same edge decision.
            for (int channel = 0; channel < 3; ++channel) {
                result.sum[channel] += ((a[channel] + b[channel]) * .5 - centre[channel]) * weight;
            }
            result.total += weight;
        }
    }
    return result;
}
}  // namespace

double Image::texel(int x, int y, int channel) const {
    x = repeat_s ? (x % size + size) % size : std::clamp(x, 0, size - 1);
    y = repeat_t ? (y % size + size) % size : std::clamp(y, 0, size - 1);
    const auto offset = (static_cast<std::size_t>(y) * size + x) * 4 + channel;
    return pixels[offset] / 255.0;
}

std::array<double, 3> Image::rgb(int x, int y) const {
    return {texel(x, y, 0), texel(x, y, 1), texel(x, y, 2)};
}

double Image::luminance(int x, int y) const {
    return .299 * texel(x, y, 0) + .587 * texel(x, y, 1) + .114 * texel(x, y, 2);
}

std::vector<unsigned char> edge_reference(const Image& image) {
    const int extent = image.size * 2;
    std::vector<unsigned char> result(static_cast<std::size_t>(extent) * extent * 4);
    // Cache quantisation happens before debanding, matching the two independent oracle stages.
    for (int y = 0; y < extent; ++y) {
        for (int x = 0; x < extent; ++x) {
            const auto point = kernel(image, x, y);
            const auto offset = (static_cast<std::size_t>(y) * extent + x) * 4;
            for (int channel = 0; channel < 3; ++channel) {
                result[offset + channel] = reconstructed_channel(image, point, channel);
            }
            result[offset + 3] = 255;
        }
    }
    // Each correction reads the original source rather than neighbouring output texels.
    for (int y = 0; y < extent; ++y) {
        for (int x = 0; x < extent; ++x) {
            const auto delta = adjustment(image, x, y);
            if (delta.total <= 0) {
                continue;
            }
            const auto offset = (static_cast<std::size_t>(y) * extent + x) * 4;
            for (int channel = 0; channel < 3; ++channel) {
                auto& value = result[offset + channel];
                value = static_cast<unsigned char>(std::lround(
                    std::clamp(value / 255.0 + delta.sum[channel] / delta.total * .5, 0.0, 1.0)
                    * 255
                ));
            }
        }
    }
    return result;
}

double reference(const Image& image, double u, double v, int channel) {
    const double x = u * image.size - .5, y = v * image.size - .5;
    const int ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y));
    const double fx = x - ix, fy = y - iy;
    return (1 - fy)
               * ((1 - fx) * image.texel(ix, iy, channel) + fx * image.texel(ix + 1, iy, channel))
           + fy
                 * ((1 - fx) * image.texel(ix, iy + 1, channel)
                    + fx * image.texel(ix + 1, iy + 1, channel));
}
}  // namespace openrac::test::texture_reference

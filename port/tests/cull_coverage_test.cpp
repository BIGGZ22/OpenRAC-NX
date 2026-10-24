// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests the zero-coverage cull's CPU reference (renderer/gpu_draw.h, covers_a_sample; ctest
// cull_coverage): bounds over a pixel centre are kept, bounds between centres are dropped, bounds
// that reach behind the eye and multisampled views are always kept, and no bounds it drops hold a
// triangle that a software rasteriser (pixel centres, edges included) lights. The cull shader of
// renderer/vulkan/gpu_draw.inc computes the same; render_equivalence_vulkan compares its frames.

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numbers>

#include "renderer/gpu_draw.h"
#include "renderer/math.h"
#include "tests/check.h"

namespace {

using openrac::renderer::GpuCullView;
using Box = std::array<float, 3>;

// The view's pixels: a 1920 by 1080 target.
constexpr float kWidth = 1920.0f;
constexpr float kHeight = 1080.0f;

// Returns a view of a camera at the origin looking down -Z with a 60 degree field, one sample a
// pixel, the zero-coverage cull on.
GpuCullView camera_view() {
    constexpr float kFovY = std::numbers::pi_v<float> / 3.0f;  // 60 degrees
    constexpr float kNear = 0.05f;
    constexpr float kFar = 4000.0f;
    GpuCullView view;
    const auto projection =
        openrac::renderer::perspective_reversed(kFovY, kWidth / kHeight, kNear, kFar);
    const auto eye = openrac::renderer::look_along({0, 0, 0}, {0, 0, -1}, {0, 1, 0});
    view.clip = openrac::renderer::multiply(projection, eye);
    view.viewport = {0.0f, 0.0f, kWidth, kHeight};
    view.coverage = true;
    return view;
}

// SplitMix64: the same sequence on every machine.
class Random {
public:
    explicit Random(std::uint64_t seed) : m_state(seed) {}

    // Returns a number from 0 to 1.
    float unit() {
        m_state += 0x9e3779b97f4a7c15ull;
        std::uint64_t z = m_state;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
        z ^= z >> 31;
        constexpr double kTwoTo53 = 9007199254740992.0;  // the double's mantissa range
        return static_cast<float>(static_cast<double>(z >> 11) / kTwoTo53);
    }

private:
    std::uint64_t m_state;
};

// Returns `point` in the view's pixels, the same projection the cull makes.
std::array<float, 2> to_pixels(const GpuCullView& view, const Box& point) {
    const auto& m = view.clip;
    const float x = ((m[0] * point[0] + m[4] * point[1]) + m[8] * point[2]) + m[12];
    const float y = ((m[1] * point[0] + m[5] * point[1]) + m[9] * point[2]) + m[13];
    const float w = ((m[3] * point[0] + m[7] * point[1]) + m[11] * point[2]) + m[15];
    return {
        view.viewport[0] + (x / w * 0.5f + 0.5f) * view.viewport[2],
        view.viewport[1] + (y / w * 0.5f + 0.5f) * view.viewport[3]
    };
}

// Returns whether the triangle `a`, `b`, `c` in pixels holds a pixel centre, its edges included:
// a rasteriser lights at most these (and a top-left rule fewer).
bool lights_a_centre(std::array<float, 2> a, std::array<float, 2> b, std::array<float, 2> c) {
    const auto edge = [](std::array<float, 2> p, std::array<float, 2> q, float x, float y) {
        return (q[0] - p[0]) * (y - p[1]) - (q[1] - p[1]) * (x - p[0]);
    };
    const int x0 = static_cast<int>(std::floor(std::fmin(a[0], std::fmin(b[0], c[0])))) - 1;
    const int x1 = static_cast<int>(std::ceil(std::fmax(a[0], std::fmax(b[0], c[0])))) + 1;
    const int y0 = static_cast<int>(std::floor(std::fmin(a[1], std::fmin(b[1], c[1])))) - 1;
    const int y1 = static_cast<int>(std::ceil(std::fmax(a[1], std::fmax(b[1], c[1])))) + 1;

    // Every pixel centre near the triangle.
    for (int y = y0; y <= y1; ++y) {
        // Along the row.
        for (int x = x0; x <= x1; ++x) {
            const float cx = static_cast<float>(x) + 0.5f;
            const float cy = static_cast<float>(y) + 0.5f;
            const float e0 = edge(a, b, cx, cy);
            const float e1 = edge(b, c, cx, cy);
            const float e2 = edge(c, a, cx, cy);

            // Inside or on every edge, for either winding.
            if ((e0 >= 0 && e1 >= 0 && e2 >= 0) || (e0 <= 0 && e1 <= 0 && e2 <= 0)) {
                return true;
            }
        }
    }

    return false;
}

// Tests that bounds over a pixel centre are kept.
void test_bounds_over_a_centre_are_kept() {
    const GpuCullView view = camera_view();
    // A box of a hundredth of a unit, 100 units ahead, on the view's axis (the image's centre,
    // between pixels 959 and 960: its rectangle reaches no centre, so it moves half a pixel).
    constexpr float kHalf = 0.005f;
    const Box low{-kHalf, -kHalf, -100.0f - kHalf};
    const Box high{kHalf, kHalf, -100.0f + kHalf};
    CHECK(openrac::renderer::covers_a_sample(view, {-1.0f, -1.0f, -11.0f}, {1.0f, 1.0f, -9.0f}));
    const float half_pixel = 100.0f * std::tan(std::numbers::pi_v<float> / 6.0f) / kHeight;
    CHECK(
        openrac::renderer::covers_a_sample(
            view,
            {low[0] + half_pixel, low[1] + half_pixel, low[2]},
            {high[0] + half_pixel, high[1] + half_pixel, high[2]}
        )
    );
}

// Tests that bounds between pixel centres are dropped: the cull culls something.
void test_bounds_between_centres_are_dropped() {
    const GpuCullView view = camera_view();
    constexpr float kHalf = 0.001f;  // a fiftieth of a pixel at 100 units
    CHECK(!openrac::renderer::covers_a_sample(
        view, {-kHalf, -kHalf, -100.0f - kHalf}, {kHalf, kHalf, -100.0f + kHalf}
    ));
}

// Tests that bounds reaching behind the eye, and every bounds of a multisampled or unfilled view,
// are kept.
void test_what_cannot_be_tested_is_kept() {
    GpuCullView view = camera_view();
    const Box low{-0.001f, -0.001f, -0.01f};
    const Box high{0.001f, 0.001f, 0.01f};
    CHECK(openrac::renderer::covers_a_sample(view, low, high));
    constexpr float kHalf = 0.001f;
    const Box far_low{-kHalf, -kHalf, -100.0f - kHalf};
    const Box far_high{kHalf, kHalf, -100.0f + kHalf};
    view.samples = 4;
    CHECK(openrac::renderer::covers_a_sample(view, far_low, far_high));
    view.samples = 1;
    view.coverage = false;
    CHECK(openrac::renderer::covers_a_sample(view, far_low, far_high));
}

// Tests on a grid of small bounds near the threshold that no bounds the reference drops holds a
// triangle a software rasteriser lights, and that it drops some.
void test_no_dropped_bounds_lights_a_centre() {
    constexpr int kBounds = 20000;
    constexpr int kTriangles = 6;  // random triangles inside each box
    const GpuCullView view = camera_view();
    Random random(7);
    int dropped = 0;
    int wrong = 0;

    // Boxes from a hundredth to two pixels wide, anywhere in the picture, 50 to 2,000 units ahead.
    for (int n = 0; n < kBounds; ++n) {
        const float distance = 50.0f + 1950.0f * random.unit();
        const float pixel = distance * 2.0f * std::tan(std::numbers::pi_v<float> / 6.0f) / kHeight;
        const float size = pixel * (0.01f + 2.0f * random.unit());
        const Box centre{
            (random.unit() - 0.5f) * distance, (random.unit() - 0.5f) * distance * 0.5f, -distance
        };
        const Box low{centre[0] - size * 0.5f, centre[1] - size * 0.5f, centre[2] - size * 0.5f};
        const Box high{centre[0] + size * 0.5f, centre[1] + size * 0.5f, centre[2] + size * 0.5f};

        // Kept: nothing to check.
        if (openrac::renderer::covers_a_sample(view, low, high)) {
            continue;
        }

        ++dropped;

        // Triangles with corners anywhere inside the dropped box.
        for (int t = 0; t < kTriangles; ++t) {
            std::array<std::array<float, 2>, 3> corners{};

            // Each corner.
            for (auto& corner : corners) {
                const Box p{
                    low[0] + (high[0] - low[0]) * random.unit(),
                    low[1] + (high[1] - low[1]) * random.unit(),
                    low[2] + (high[2] - low[2]) * random.unit()
                };
                corner = to_pixels(view, p);
            }

            wrong += lights_a_centre(corners[0], corners[1], corners[2]) ? 1 : 0;
        }
    }

    std::printf(
        "zero-coverage: %d of %d bounds dropped, %d lit triangles in them\n",
        dropped,
        kBounds,
        wrong
    );
    CHECK(dropped > 0);
    CHECK(wrong == 0);
}

}  // namespace

int main() {
    test_bounds_over_a_centre_are_kept();
    test_bounds_between_centres_are_dropped();
    test_what_cannot_be_tested_is_kept();
    test_no_dropped_bounds_lights_a_centre();
    return openrac::test::result();
}

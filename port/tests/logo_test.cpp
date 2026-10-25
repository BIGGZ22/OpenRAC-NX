// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The logo's coverage mask (renderer/ui/logo.h): its size follows the height asked for, the mark
// alone is the logo's left part, and the shapes are filled where the logo is solid and empty in
// its holes.

#include <cstdint>

#include "check.h"
#include "renderer/ui/logo.h"

namespace {

using openrac::renderer::ui::logo::draw;
using openrac::renderer::ui::logo::Mask;

int at(const Mask& m, double x, double y) {
    return m.alpha
        [static_cast<std::size_t>(y * (m.height - 1)) * m.width
         + static_cast<std::size_t>(x * (m.width - 1))];
}

void test_sizes() {
    const Mask mark = draw(120, false);
    CHECK(mark.height == 120);
    CHECK(mark.alpha.size() == static_cast<std::size_t>(mark.width) * mark.height);
    // The mark is a little wider than high (410 by 329 points).
    CHECK(mark.width > 140 && mark.width < 160);
    CHECK(mark.name_x == mark.width);
    const Mask full = draw(60, true);
    CHECK(full.height == 60);
    // The name makes the logo about three times as wide as the mark.
    CHECK(full.width > 3 * full.height && full.width < 4 * full.height);
    CHECK(full.name_x > 60 && full.name_x < 90);
    CHECK(draw(2, true).alpha.empty());
}

void test_coverage() {
    const Mask mark = draw(200, false);
    // The margin is empty, the "O"'s ring is solid and its middle is a hole.
    CHECK(mark.alpha[0] == 0);
    CHECK(at(mark, 0.97, 0.62) == 255);
    CHECK(at(mark, 0.80, 0.62) == 0);
    int edges = 0;

    for (const std::uint8_t a : mark.alpha) {
        edges += a > 0 && a < 255 ? 1 : 0;
    }

    CHECK(edges > 0);  // antialiased
}

}  // namespace

int main() {
    test_sizes();
    test_coverage();
    return openrac::test::result();
}

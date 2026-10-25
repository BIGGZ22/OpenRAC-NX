// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests program::UndrawnTicks (game/common/program/undrawn_ticks.h): which ticks keep the window's
// last picture. The expected values follow from the rule its header states.

#include <cstdint>

#include "game/common/program/undrawn_ticks.h"
#include "tests/check.h"

namespace {

using openrac::program::kMostUndrawnTicks;
using openrac::program::UndrawnTicks;

// A layer bit the game reports drawing (any OPENRAC_DRAW_* bit does).
constexpr std::uint32_t kSomeLayer = 1u;

// An undrawn tick after a drawn one keeps the picture, at most kMostUndrawnTicks in a row.
void test_undrawn_ticks_after_a_drawn_one_keep_the_picture() {
    UndrawnTicks ticks;
    CHECK(!ticks.keep(true, kSomeLayer, true));

    // Each undrawn tick of the run, up to the most, keeps it.
    for (int n = 0; n < kMostUndrawnTicks; ++n) {
        CHECK(ticks.keep(true, 0, true));
    }

    // The next undrawn tick is drawn as it comes, and so is every one after it.
    CHECK(!ticks.keep(true, 0, true));
    CHECK(!ticks.keep(true, 0, true));

    // A drawn tick starts the count again.
    CHECK(!ticks.keep(true, kSomeLayer, true));
    CHECK(ticks.keep(true, 0, true));
}

// Nothing is kept before the game has drawn, outside a level, or while it reports no draws.
void test_no_picture_is_kept_without_a_drawn_level_tick() {
    UndrawnTicks first;
    CHECK(!first.keep(true, 0, true));

    UndrawnTicks boot;
    CHECK(!boot.keep(true, kSomeLayer, false));
    CHECK(!boot.keep(true, 0, false));

    // A game that reports no draws has every layer drawn (vsync passes all bits set).
    UndrawnTicks silent;
    CHECK(!silent.keep(false, ~0u, true));
    CHECK(!silent.keep(false, ~0u, true));
}

}  // namespace

int main() {
    test_undrawn_ticks_after_a_drawn_one_keep_the_picture();
    test_no_picture_is_kept_without_a_drawn_level_tick();
    return openrac::test::result();
}

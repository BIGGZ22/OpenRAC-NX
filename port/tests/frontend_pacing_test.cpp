// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's pacing arithmetic (game/common/frontend/pacing.h): the refresh a frame is shown
// at, how far between ticks the scene is at a refresh for the PAL and NTSC tick rates, and when the
// frame cap lets a frame start. Characterisation tests: they pin what the functions returned when
// they were moved out of frontend.cpp, so that the frontend's restructure can show it changed none
// of it. Each expected value is written as the arithmetic that gives it. The clock is never read:
// every time is an offset from a fixed point.

#include <chrono>

#include "check.h"
#include "game/common/frame_pacing.h"
#include "game/common/frontend/pacing.h"

namespace {

using openrac::frontend::frame_may_start;
using openrac::frontend::kLagTicks;
using openrac::frontend::shown_at;
using openrac::frontend::tick_alpha;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

// The game's tick rates: PAL's 50 a second and NTSC's 60.
constexpr int kTickRates[] = {50, 60};

// The frame caps the player can choose from, with 0 for none.
constexpr double kCaps[] = {0.0, 30.0, 60.0, 120.0, 144.0};

// A 60 Hz display's refresh interval, in seconds.
constexpr double kRefresh = 1.0 / 60.0;

// What drawing a frame costs, in seconds (4 ms).
constexpr double kCost = 0.004;

// How far apart two alphas may be and still be the same result of double to float arithmetic.
constexpr float kTolerance = 1e-5f;

// A fixed point on the steady clock, away from its epoch (which stands for "never").
steady_clock::time_point start() {
    return steady_clock::time_point{} + std::chrono::seconds(100);
}

// `seconds` as the steady clock's duration, cut as the frontend cuts it.
steady_clock::duration span(double seconds) {
    return duration_cast<steady_clock::duration>(duration<double>(seconds));
}

// True when `a` and `b` differ by at most kTolerance.
bool near(float a, float b) {
    return (a > b ? a - b : b - a) <= kTolerance;
}

// With no refresh interval, or no swap that waited, a frame is shown when it is drawn.
void test_a_frame_without_a_refresh_is_shown_when_drawn() {
    const auto now = start();
    CHECK(shown_at(now, kCost, 0.0, now - milliseconds(5)) == now + span(kCost));
    CHECK(shown_at(now, kCost, kRefresh, steady_clock::time_point{}) == now + span(kCost));
}

// A frame is shown at the first refresh after it is drawn, counted from the last swap that waited.
void test_a_frame_is_shown_at_the_next_refresh() {
    const auto now = start();
    // Drawn 5 + 4 = 9 ms after the last swap: the first refresh, 16.7 ms after it.
    const auto swap = now - milliseconds(5);
    CHECK(shown_at(now, kCost, kRefresh, swap) == swap + span(1 * kRefresh));
    // Drawn 40 + 4 = 44 ms after it: the third refresh (44 / 16.7 = 2.64, rounded up).
    const auto old_swap = now - milliseconds(40);
    CHECK(shown_at(now, kCost, kRefresh, old_swap) == old_swap + span(3 * kRefresh));
}

// A frame drawn before the last swap's time is still shown no sooner than the refresh after it.
void test_a_frame_is_shown_one_refresh_after_the_swap_at_least() {
    const auto now = start();
    const auto swap = now + milliseconds(10);
    CHECK(shown_at(now, kCost, kRefresh, swap) == swap + span(1 * kRefresh));
}

// The scene is at the last tick until half a tick has passed, and at the kept one a tick later.
void test_the_alpha_is_held_at_the_edges_of_a_tick() {
    // PAL's tick and NTSC's.
    for (const int rate : kTickRates) {
        const duration<double> period{1.0 / rate};
        const auto tick = start();
        // Before the tick and up to the lag, 0.
        CHECK(tick_alpha(tick - milliseconds(1), tick, period) == 0.0f);
        CHECK(tick_alpha(tick, tick, period) == 0.0f);
        CHECK(tick_alpha(tick + span(kLagTicks / rate) - milliseconds(1), tick, period) == 0.0f);
        // A tick and a half on (the lag and the whole tick), and after it, 1.
        CHECK(tick_alpha(tick + span((1.0 + kLagTicks) / rate), tick, period) == 1.0f);
        CHECK(tick_alpha(tick + span(5.0 / rate), tick, period) == 1.0f);
    }
}

// Between the edges the alpha runs straight from 0 to 1 over one tick.
void test_the_alpha_runs_over_a_tick() {
    // PAL's tick and NTSC's.
    for (const int rate : kTickRates) {
        const duration<double> period{1.0 / rate};
        const auto tick = start();
        // One tick on is half a tick past the lag: 0.5.
        CHECK(near(tick_alpha(tick + span(1.0 / rate), tick, period), 0.5f));
        // A quarter of a tick past the lag: 0.25.
        CHECK(near(tick_alpha(tick + span((kLagTicks + 0.25) / rate), tick, period), 0.25f));
    }
}

// No cap, or no capped frame yet, lets every frame start.
void test_no_cap_lets_a_frame_start() {
    const auto now = start();

    // Every cap, with no frame capped yet.
    for (const double cap : kCaps) {
        CHECK(frame_may_start(cap, steady_clock::time_point{}, now));
    }

    CHECK(frame_may_start(0.0, now + std::chrono::seconds(1), now));
    CHECK(frame_may_start(-1.0, now + std::chrono::seconds(1), now));
}

// A cap holds a frame until a millisecond before its due time, for each cap the player can pick.
void test_a_cap_holds_a_frame_until_it_is_due() {
    // Each cap the player can pick.
    for (const double cap : kCaps) {
        // No cap has no schedule to hold to.
        if (cap <= 0.0) {
            continue;
        }

        const auto step = span(1.0 / cap);
        const auto started = start();
        // The first capped frame drew in 1 ms: the next is due a step after it started.
        const auto due =
            openrac::frontend::frame_pacing::next_due({}, started, started + milliseconds(1), step);
        CHECK(due == started + step);
        CHECK(!frame_may_start(cap, due, due - milliseconds(2)));
        CHECK(frame_may_start(cap, due, due - milliseconds(1)));
        CHECK(frame_may_start(cap, due, due));
        CHECK(frame_may_start(cap, due, due + step));
    }
}

}  // namespace

int main() {
    test_a_frame_without_a_refresh_is_shown_when_drawn();
    test_a_frame_is_shown_at_the_next_refresh();
    test_a_frame_is_shown_one_refresh_after_the_swap_at_least();
    test_the_alpha_is_held_at_the_edges_of_a_tick();
    test_the_alpha_runs_over_a_tick();
    test_no_cap_lets_a_frame_start();
    test_a_cap_holds_a_frame_until_it_is_due();
    return openrac::test::result();
}

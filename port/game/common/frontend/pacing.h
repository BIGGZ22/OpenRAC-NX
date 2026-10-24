// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's pacing arithmetic for the frames drawn between ticks: the refresh a frame reaches,
// how far from one tick to the next the scene drawn for that refresh is, and whether the frame cap
// lets a frame start. Arithmetic on what it is given only: frontend.cpp passes in its pacing state
// and the cap it works out from the settings (frame_cap), so that tests/frontend_pacing_test.cpp
// can pin each result. The cap's own arithmetic and its schedule are frame_pacing.h's.

#pragma once

#include <chrono>

namespace openrac::frontend {

// The frames between ticks show the game half a tick late, so that the next tick is ready before
// the refresh that needs it.
constexpr double kLagTicks = 0.5;

// The refresh a frame started at `now` is shown at: the first after the frame is drawn. `cost`:
// what drawing a frame takes, in seconds. `interval`: the display's refresh interval in seconds, 0
// when the display does not say. `last_swap`: the last swap that waited for a refresh, the clock's
// epoch when none did. Without an interval or such a swap, the time the frame is drawn by.
std::chrono::steady_clock::time_point shown_at(
    std::chrono::steady_clock::time_point now,
    double cost,
    double interval,
    std::chrono::steady_clock::time_point last_swap
);

// How far the scene is from the last tick's to the kept one's at the refresh `display`, from 0 to
// 1: `tick` is when the kept tick came, `period` the game's tick length, and the scene runs
// kLagTicks behind.
float tick_alpha(
    std::chrono::steady_clock::time_point display,
    std::chrono::steady_clock::time_point tick,
    std::chrono::duration<double> period
);

// Whether the frame cap lets a frame start at `now` (within a millisecond: a sleep wakes that
// coarsely). `cap`: the frames a second it allows, 0 or less for none. `due`: when the cap lets the
// next frame start (frame_pacing::next_due), the clock's epoch before the first capped frame.
bool frame_may_start(
    double cap, std::chrono::steady_clock::time_point due, std::chrono::steady_clock::time_point now
);

}  // namespace openrac::frontend

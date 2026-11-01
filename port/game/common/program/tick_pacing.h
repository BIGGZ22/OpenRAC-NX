// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The game's tick schedule (program/vsync.cpp), as a plain function the host tests can check: each
// tick is due one period after the last one was due, so time spent in a tick is not added to the
// game's time; a tick more than `slack` late starts the schedule again from now (the game slows down
// rather than running a burst of ticks to catch up a long stall); and a new tick rate (the game's
// time base changed: 60 or 50 ticks a second) starts it again from now too, so the change neither
// bursts nor waits out the old period.

#pragma once

#include <chrono>

namespace openrac::program::tick_pacing {

using Clock = std::chrono::steady_clock;

// When the next tick is due, given when the last was due (zero: none yet), the time now, the period
// at the rate now, whether the rate has just changed, and the most lateness caught up.
inline Clock::time_point next_due(Clock::time_point last_due, Clock::time_point now, Clock::duration period,
                                  bool rate_changed, Clock::duration slack = std::chrono::milliseconds(100)) {
    Clock::time_point from = last_due;
    if (last_due.time_since_epoch().count() == 0 || now > last_due + slack || rate_changed) {
        from = now;
    }
    return from + period;
}

// The period of `rate` ticks a second.
inline Clock::duration period_of(int rate) {
    return std::chrono::duration_cast<Clock::duration>(std::chrono::microseconds(1000000 / rate));
}

}  // namespace openrac::program::tick_pacing

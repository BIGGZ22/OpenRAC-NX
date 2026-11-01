// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's pacing arithmetic (pacing.h).

#include "game/common/frontend/pacing.h"

#include <algorithm>
#include <cmath>

namespace openrac::frontend {

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::steady_clock;

steady_clock::time_point shown_at(
    steady_clock::time_point now, double cost, double interval, steady_clock::time_point last_swap
) {
    const auto ready = now + duration_cast<steady_clock::duration>(duration<double>(cost));

    // Without a refresh to count from, the frame is shown when it is drawn.
    if (interval <= 0.0 || last_swap.time_since_epoch().count() == 0) {
        return ready;
    }

    const double since = duration<double>(ready - last_swap).count();
    const double n = std::max(1.0, std::ceil(since / interval));
    return last_swap + duration_cast<steady_clock::duration>(duration<double>(n * interval));
}

float tick_alpha(
    steady_clock::time_point display, steady_clock::time_point tick, duration<double> period
) {
    const double t = duration<double>(display - tick).count() / period.count() - kLagTicks;
    return static_cast<float>(std::clamp(t, 0.0, 1.0));
}

bool frame_may_start(double cap, steady_clock::time_point due, steady_clock::time_point now) {
    return cap <= 0.0 || due.time_since_epoch().count() == 0
           || now + std::chrono::milliseconds(1) >= due;
}

steady_clock::duration tick_frame_tolerance(double cap, duration<double> tick_period, bool after_between) {
    const steady_clock::duration strict = std::chrono::milliseconds(1);
    if (cap <= 0.0 || after_between || tick_period.count() <= 0.0) {
        return strict;
    }
    const double step = 1.0 / cap;
    if (step > tick_period.count() + 0.00025) {
        return strict;  // a cap under the tick rate: it chooses the ticks drawn
    }
    return std::max(strict, duration_cast<steady_clock::duration>(duration<double>(step * 0.5)));
}

bool tick_frame_may_start(
    double cap,
    steady_clock::time_point due,
    steady_clock::time_point now,
    duration<double> tick_period,
    bool after_between
) {
    return cap <= 0.0 || due.time_since_epoch().count() == 0
           || now + tick_frame_tolerance(cap, tick_period, after_between) >= due;
}

}  // namespace openrac::frontend

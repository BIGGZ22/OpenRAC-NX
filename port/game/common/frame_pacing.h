// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once

#include <algorithm>
#include <chrono>

namespace openrac::frontend::frame_pacing {

// Zero means unlimited. VSync cannot present faster than the current display;
// explicit caps below that rate still apply. Unknown refresh leaves the cap alone.
inline double limit(double requested_hz, bool follow_display, bool vsync, double refresh_interval) {
    if (refresh_interval > 0.0 && (follow_display || vsync)) {
        const double display_hz = 1.0 / refresh_interval;
        return requested_hz > 0.0 ? std::min(requested_hz, display_hz) : display_hz;
    }
    return requested_hz;
}

// Schedule from the frame's start, not from a potentially asynchronous present
// return. The caller can defer a tick's picture until this deadline instead of
// sleeping after every picture and delaying the next simulation tick.
inline std::chrono::steady_clock::time_point next_due(
    std::chrono::steady_clock::time_point previous,
    std::chrono::steady_clock::time_point started,
    std::chrono::steady_clock::time_point finished,
    std::chrono::steady_clock::duration step
) {
    return previous.time_since_epoch().count() == 0 || finished > previous + 2 * step
               ? started + step : previous + step;
}

}  // namespace openrac::frontend::frame_pacing

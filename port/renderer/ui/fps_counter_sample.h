// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Displayed-frame averages: elapsed time includes every intervening simulation tick and wait.
#pragma once

#include <algorithm>

namespace openrac::renderer::ui::fps_counter {

// One averaging window, counting displayed intervals rather than simulation ticks.
struct FrameSample {
    double seconds = 0;      // Total time between completed presentations.
    double cpu_seconds = 0;  // Elapsed time outside swap calls, including other waits.
    int frames = 0;          // Completed presentation intervals.

    // Adds one displayed interval; `swap_seconds` is the swap call ending that interval.
    void add(double interval_seconds, double swap_seconds) {
        // A first presentation or an invalid interval cannot contribute an FPS sample.
        if (interval_seconds <= 0) {
            return;
        }

        seconds += interval_seconds;
        cpu_seconds += interval_seconds - std::clamp(swap_seconds, 0.0, interval_seconds);
        ++frames;
    }

    // Returns presentations per second, or zero before the first complete interval.
    double fps() const { return seconds > 0 ? frames / seconds : 0; }

    // Returns the mean whole-frame milliseconds, including swap and pacing waits.
    double frame_ms() const { return frames > 0 ? seconds * 1000.0 / frames : 0; }

    // Returns elapsed milliseconds per displayed frame with only swap time removed.
    double cpu_ms() const { return frames > 0 ? cpu_seconds * 1000.0 / frames : 0; }
};

}  // namespace openrac::renderer::ui::fps_counter

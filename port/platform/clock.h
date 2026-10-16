// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Time: a monotonic clock, and the frame pacer that holds the game loop to its
// tick rate. The port runs every game at 60 Hz (the PAL game in its own 60 Hz
// mode); the game's own timing (animation steps, the frame skip it applies when
// a frame runs long) assumes that rate, so the port paces the loop to it
// whatever the monitor's refresh. Vsync, when on, only
// keeps the picture from tearing.

#pragma once

#include <cstdint>
#include <functional>

namespace openrac::platform {

using Nanoseconds = std::uint64_t;

// Nanoseconds since some fixed point (SDL's tick counter); never goes back.
Nanoseconds now_ns();

// Sleep for about this long; SDL's precise delay spins the last stretch.
void sleep_ns(Nanoseconds duration);

enum class VideoStandard {
    Pal,
    Ntsc
};

// A rate as an exact fraction, so a long run does not drift: PAL 50/1,
// NTSC 60000/1001.
struct Rate {
    std::uint64_t numerator;
    std::uint64_t denominator;

    double hz() const { return static_cast<double>(numerator) / static_cast<double>(denominator); }
};

Rate field_rate(VideoStandard standard);

class FramePacer {
public:
    // The clock the pacer reads and sleeps on; tests pass a fake one.
    struct Clock {
        std::function<Nanoseconds()> now;
        std::function<void(Nanoseconds)> sleep;
    };

    static Clock system_clock();

    explicit FramePacer(Rate rate, Clock clock = system_clock());

    // Wait until the next field is due, then return how many fields have
    // passed since the previous call: 1 when the frame was on time, more when
    // it ran long (the game's vsync counter advances by that much). A frame
    // late by more than kResyncFields starts the schedule again from now
    // rather than racing to catch up.
    int wait();

    // Start the schedule again from now (after a load or a pause).
    void reset();

    Rate rate() const { return m_rate; }

    // The deadline of field `index` after the start of the schedule.
    Nanoseconds deadline(std::uint64_t index) const;

    static constexpr std::uint64_t kResyncFields = 8;

private:
    Rate m_rate;
    Clock m_clock;
    Nanoseconds m_start = 0;
    std::uint64_t m_index = 0;  // fields since m_start
};

}  // namespace openrac::platform

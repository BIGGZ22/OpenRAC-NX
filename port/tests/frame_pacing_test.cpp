// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#include <chrono>
#include <cmath>

#include "game/common/frame_pacing.h"
#include "tests/check.h"

int main() {
    using namespace openrac;
    using namespace std::chrono;
    using frontend::frame_pacing::limit;
    using frontend::frame_pacing::next_due;
    CHECK(limit(120, false, true, 1.0 / 120) == 120);
    CHECK(limit(120, false, true, 1.0 / 60) == 60);
    CHECK(limit(120, false, true, 1.0 / 100) == 100);
    CHECK(limit(30, false, true, 1.0 / 120) == 30);
    CHECK(limit(120, false, false, 1.0 / 60) == 120);
    CHECK(limit(0, true, false, 1.0 / 120) == 120);
    CHECK(limit(0, false, true, 1.0 / 120) == 120);
    CHECK(limit(0, false, false, 1.0 / 60) == 0);
    CHECK(limit(120, false, true, 0) == 120);
    CHECK(limit(0, true, true, 0) == 0);

    // Rendering and simulation share a thread. Quick present calls must leave
    // the next tick runnable, rather than appending a whole refresh wait.
    for (int refresh : {60, 100, 120, 144}) {
        const double cap = limit(120, false, true, 1.0 / refresh);
        const auto step = duration_cast<steady_clock::duration>(duration<double>(1.0 / cap));
        auto now = steady_clock::time_point{} + seconds(1);
        const auto start = now;
        steady_clock::time_point due{};
        int frames = 0;
        for (int tick = 0; tick < 60; ++tick) {
            const auto tick_time = start + duration_cast<steady_clock::duration>(duration<double>(tick / 60.0));
            const auto next_tick = start + duration_cast<steady_clock::duration>(duration<double>((tick + 1) / 60.0));
            now = std::max(now, tick_time) + milliseconds(1);
            while (now < next_tick) {
                if (due > now) now = due;
                if (now >= next_tick) break;
                const auto frame_start = now;
                now += milliseconds(2);  // draw, submit and return without waiting for vblank
                due = next_due(due, frame_start, now, step);
                ++frames;
            }
        }
        CHECK(std::abs(frames - static_cast<int>(cap)) <= 1);
    }
    const auto start = steady_clock::time_point{} + seconds(1);
    CHECK(next_due({}, start, start + milliseconds(2), milliseconds(10)) == start + milliseconds(10));
    CHECK(next_due(start, start + milliseconds(3), start + milliseconds(5), milliseconds(10))
          == start + milliseconds(10));
    CHECK(next_due(start, start + seconds(1), start + seconds(1) + milliseconds(2), milliseconds(10))
          == start + seconds(1) + milliseconds(10));
    return test::result();
}

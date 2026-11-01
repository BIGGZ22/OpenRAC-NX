// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// game/common/program/tick_pacing.h, over a simulated clock: the game's time against wall time. At
// 60 and at 50 ticks a second the wall time of N ticks is N periods whatever each tick's work (under
// a period); a change of rate takes effect from the next tick without a burst or a wait; a tick that
// runs late by less than the slack is caught up (the average stays right), a stall past it starts the
// schedule again (the game time lost, no burst after); a machine too slow runs the game slow, never
// fast.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>

#include "game/common/program/tick_pacing.h"
#include "tests/check.h"

namespace tp = openrac::program::tick_pacing;
using namespace std::chrono;
using Clock = tp::Clock;

namespace {

struct Sim {
    Clock::time_point now{seconds(1000)};
    Clock::time_point due{};  // zero: no tick yet
    Clock::duration min_gap = hours(1);  // the shortest gap between two tick starts
    Clock::time_point last_start{};

    // One tick: `work` of game time spent, then paced as vsync() does (next_due, then sleep_until).
    void tick(Clock::duration work, int rate, bool rate_changed = false) {
        if (last_start.time_since_epoch().count() != 0) {
            min_gap = std::min(min_gap, now - last_start);
        }
        last_start = now;
        now += work;
        due = tp::next_due(due, now, tp::period_of(rate), rate_changed);
        if (now < due) {
            now = due;  // sleep_until; past due, the next tick starts at once (the catch-up)
        }
    }
};

double secs(Clock::duration d) {
    return duration<double>(d).count();
}

}  // namespace

int main() {
    // 60 a second: 600 ticks of 5 ms work take 10 s.
    {
        Sim s;
        const auto start = s.now;
        for (int i = 0; i < 600; ++i) {
            s.tick(milliseconds(5), 60);
        }
        CHECK(secs(s.now - start) > 9.99 && secs(s.now - start) < 10.01);
    }
    // 50 a second (the game's 50 Hz time base): 500 ticks take 10 s (at 60 they took 8.3 s: 1.2 times
    // too fast).
    {
        Sim s;
        const auto start = s.now;
        for (int i = 0; i < 500; ++i) {
            s.tick(milliseconds(5), 50);
        }
        CHECK(secs(s.now - start) > 9.99 && secs(s.now - start) < 10.01);
    }
    // 50 then 60 (the level start after the title): 250 ticks at 50 (5 s), 300 at 60 (5 s); no tick
    // sooner than 15 ms after the last (no burst at the change); 10 s in all.
    {
        Sim s;
        const auto start = s.now;
        for (int i = 0; i < 250; ++i) {
            s.tick(milliseconds(4), 50);
        }
        for (int i = 0; i < 300; ++i) {
            s.tick(milliseconds(4), 60, i == 0);
        }
        CHECK(secs(s.now - start) > 9.97 && secs(s.now - start) < 10.03);
        CHECK(s.min_gap >= milliseconds(15));
    }
    // Late ticks inside the slack are caught up: every 10th tick takes 40 ms, the others 5 ms; 600
    // ticks still take 10 s (the ticks after a slow one start at once).
    {
        Sim s;
        const auto start = s.now;
        for (int i = 0; i < 600; ++i) {
            s.tick(milliseconds(i % 10 == 9 ? 40 : 5), 60);
        }
        CHECK(secs(s.now - start) > 9.99 && secs(s.now - start) < 10.05);
    }
    // A stall past the slack (a 500 ms load): the schedule starts again after it; the lost time is
    // not run back as a burst (the next ticks are a period apart).
    {
        Sim s;
        for (int i = 0; i < 60; ++i) {
            s.tick(milliseconds(5), 60);
        }
        s.tick(milliseconds(500), 60);
        const auto after = s.now;
        s.min_gap = hours(1);
        for (int i = 0; i < 60; ++i) {
            s.tick(milliseconds(5), 60);
        }
        CHECK(secs(s.now - after) > 0.99 && secs(s.now - after) < 1.01);
        CHECK(s.min_gap >= milliseconds(16));
    }
    // Too slow for the rate (25 ms of work a tick at 60): a tick every 25 ms, never faster than the
    // rate: the game runs slow, not fast.
    {
        Sim s;
        const auto start = s.now;
        for (int i = 0; i < 400; ++i) {
            s.tick(milliseconds(25), 60);
        }
        CHECK(secs(s.now - start) >= 400 * 0.025 - 0.001);
    }
    CHECK(tp::period_of(60) == microseconds(16666));
    CHECK(tp::period_of(50) == microseconds(20000));
    return openrac::test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// platform/switch/clock_choice.h: only Nintendo's clock steps, never above each official maximum,
// the GPU held to 460.8 MHz handheld and 768 MHz docked, values between steps taken down, and 0 (or
// too low) leaving a clock alone.

#include "platform/switch/clock_choice.h"
#include "tests/check.h"

namespace c = openrac::platform::clock_choice;

int main() {
    // Nothing asked: nothing set.
    {
        const c::Choice x = c::choose(0, 0, 0, true);
        CHECK(x.cpu_hz == 0 && x.gpu_hz == 0 && x.mem_hz == 0);
    }
    // The official maximum, docked.
    {
        const c::Choice x = c::choose(1785, 768, 1600, true);
        CHECK(x.cpu_hz == 1785000000u && x.gpu_hz == 768000000u && x.mem_hz == 1600000000u);
    }
    // Above the maximum: held to it (no overclock past Nintendo's own steps).
    {
        const c::Choice x = c::choose(2295, 1267, 2133, true);
        CHECK(x.cpu_hz == 1785000000u && x.gpu_hz == 768000000u && x.mem_hz == 1600000000u);
    }
    // Handheld: the GPU at most 460.8 MHz, whatever is asked.
    {
        CHECK(c::choose(0, 768, 0, false).gpu_hz == 460800000u);
        CHECK(c::choose(0, 460, 0, false).gpu_hz == 460800000u);  // 460.8 asked as 460
        CHECK(c::choose(0, 384, 0, false).gpu_hz == 384000000u);
        CHECK(c::choose(1785, 0, 0, false).cpu_hz == 1785000000u);  // the CPU is not tied to the mode
    }
    // Between steps: the step below.
    {
        CHECK(c::choose(1500, 0, 0, true).cpu_hz == 1428000000u);
        CHECK(c::choose(0, 700, 0, true).gpu_hz == 691200000u);
        CHECK(c::choose(0, 0, 1500, true).mem_hz == 1331200000u);
    }
    // Below the lowest step offered: left alone.
    {
        CHECK(c::choose(800, 200, 1000, true).cpu_hz == 0);
        CHECK(c::choose(800, 200, 1000, true).gpu_hz == 0);
        CHECK(c::choose(800, 200, 1000, true).mem_hz == 0);
    }
    // Every step is reachable by its own MHz figure.
    for (const auto hz : c::kCpuHz) {
        CHECK(c::choose(hz / 1000000u, 0, 0, true).cpu_hz == hz);
    }
    for (const auto hz : c::kGpuHz) {
        CHECK(c::choose(0, hz / 1000000u, 0, true).gpu_hz == hz);
    }
    for (const auto hz : c::kMemHz) {
        CHECK(c::choose(0, 0, hz / 1000000u, true).mem_hz == hz);
    }
    static_assert(c::choose(1785, 768, 1600, true).gpu_hz == 768000000u);
    // The raised profile: 1785 / 768 / 1600 whatever the mode (no handheld cap for it).
    static_assert(c::raised_profile().cpu_hz == 1785000000u && c::raised_profile().gpu_hz == 768000000u
                  && c::raised_profile().mem_hz == 1600000000u);
    return openrac::test::result();
}

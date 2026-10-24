// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Counter intervals from arithmetic, including skipped simulation ticks and swap waits.
#include <cmath>

#include "renderer/ui/fps_counter_sample.h"
#include "tests/check.h"

namespace {
namespace counter = openrac::renderer::ui::fps_counter;

// Fifteen intervals in a quarter second yield 60 FPS and reciprocal frame milliseconds.
void test_whole_frame_time_agrees_with_fps() {
    counter::FrameSample sample;

    for (int n = 0; n < 15; ++n) {
        sample.add(1.0 / 60, 1.0 / 60 - .012);
    }

    CHECK(sample.frames == 15);
    CHECK(std::abs(sample.fps() - 60) < 1e-9);
    CHECK(std::abs(sample.frame_ms() - 1000.0 / 60) < 1e-9);
    CHECK(std::abs(sample.cpu_ms() - 12) < 1e-9);
}

// Several simulation ticks before one presentation remain one displayed-frame CPU sample.
void test_skipped_ticks_do_not_dilute_cpu_time() {
    counter::FrameSample sample;
    // Three intervening ticks and rendering take 30 ms, followed by a 10 ms swap.
    sample.add(.030 + .010, .010);
    CHECK(sample.frames == 1);
    CHECK(std::abs(sample.cpu_ms() - 30) < 1e-9);
    CHECK(std::abs(sample.frame_ms() - 40) < 1e-9);
    CHECK(std::abs(sample.fps() - 25) < 1e-9);
}

// Uneven frames use total elapsed time rather than averaging instantaneous FPS values.
void test_uneven_intervals_use_one_displayed_frame_denominator() {
    counter::FrameSample sample;
    sample.add(.010, .004);
    sample.add(.020, .007);
    sample.add(.030, .006);
    CHECK(std::abs(sample.fps() - 50) < 1e-9);
    CHECK(std::abs(sample.frame_ms() - 20) < 1e-9);
    CHECK(std::abs(sample.cpu_ms() - (6.0 + 13.0 + 24.0) / 3) < 1e-9);
}

// Warm-up has no interval, and swap is bounded to avoid negative CPU time.
void test_warm_up_and_swap_bounds() {
    counter::FrameSample sample;
    sample.add(0, .010);
    CHECK(sample.frames == 0);
    CHECK(sample.fps() == 0 && sample.frame_ms() == 0 && sample.cpu_ms() == 0);
    sample.add(.010, .020);
    CHECK(sample.cpu_ms() == 0);
    sample = {};
    sample.add(.010, -.010);
    CHECK(std::abs(sample.cpu_ms() - 10) < 1e-9);
}
}  // namespace

int main() {
    test_whole_frame_time_agrees_with_fps();
    test_skipped_ticks_do_not_dilute_cpu_time();
    test_uneven_intervals_use_one_displayed_frame_denominator();
    test_warm_up_and_swap_bounds();
    return openrac::test::result();
}

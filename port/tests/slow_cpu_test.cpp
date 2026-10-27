// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The simulated slow processor (common/slow_cpu.h): a thread's work since its last pad padded to
// `factor` times as long, nothing while off, and the factor kept in its range.

#include <chrono>
#if !defined(_WIN32)
#include <ctime>
#endif

#include "check.h"
#include "common/slow_cpu.h"

namespace {

namespace slow_cpu = openrac::slow_cpu;
using Clock = std::chrono::steady_clock;

[[maybe_unused]] double ms_since(Clock::time_point t) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t).count();
}

#if !defined(_WIN32)
// This thread's processor time in milliseconds.
double thread_ms() {
    timespec now{};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now);
    return static_cast<double>(now.tv_sec) * 1e3 + static_cast<double>(now.tv_nsec) * 1e-6;
}

// Keeps this thread busy for `ms` of its own processor time, however loaded the machine is.
void work(double ms) {
    const double start = thread_ms();
    volatile unsigned sink = 0;
    while (thread_ms() - start < ms) {
        sink = sink + 1;
    }
}
#endif

}  // namespace

int main() {
    CHECK(slow_cpu::factor() == 1);
#if defined(_WIN32)
    CHECK(!slow_cpu::available());
    slow_cpu::set_factor(4);
    CHECK(slow_cpu::factor() == 1);
#else
    CHECK(slow_cpu::available());
    slow_cpu::set_factor(0);
    CHECK(slow_cpu::factor() == 1);
    slow_cpu::set_factor(99);
    CHECK(slow_cpu::factor() == 8);

    // Three times slower: 20 ms of work padded by about 40 ms.
    slow_cpu::set_factor(3);
    slow_cpu::pad();  // the first call only starts counting
    work(20.0);
    auto start = Clock::now();
    slow_cpu::pad();
    const double padded = ms_since(start);
    CHECK(padded > 38.0 && padded < 400.0);  // 40 ms of wall time, however loaded

    // The spin is not padded again: a call right after pads almost nothing.
    start = Clock::now();
    slow_cpu::pad();
    CHECK(ms_since(start) < 10.0);

    // Off: no padding at all.
    slow_cpu::set_factor(1);
    work(10.0);
    start = Clock::now();
    slow_cpu::pad();
    CHECK(ms_since(start) < 5.0);
#endif
    return openrac::test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/clock.h"

#include <utility>

#ifdef __SWITCH__
#include <chrono>
#include <thread>
#else
#include <SDL3/SDL_timer.h>
#endif

namespace openrac::platform {

Nanoseconds now_ns() {
#ifdef __SWITCH__
    return static_cast<Nanoseconds>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
#else
    return SDL_GetTicksNS();
#endif
}

void sleep_ns(Nanoseconds duration) {
#ifdef __SWITCH__
    std::this_thread::sleep_for(std::chrono::nanoseconds(duration));
#else
    SDL_DelayPrecise(duration);
#endif
}

Rate field_rate(VideoStandard standard) {
    switch (standard) {
        case VideoStandard::Pal:
            return {50, 1};
        case VideoStandard::Ntsc:
            return {60000, 1001};
    }
    return {50, 1};
}

FramePacer::Clock FramePacer::system_clock() {
    return {now_ns, sleep_ns};
}

FramePacer::FramePacer(Rate rate, Clock clock) : m_rate(rate), m_clock(std::move(clock)) {
    reset();
}

void FramePacer::reset() {
    m_start = m_clock.now();
    m_index = 0;
}

Nanoseconds FramePacer::deadline(std::uint64_t index) const {
    // index * 1e9 * den / num, exact in integers: a run of a year at 60 Hz is
    // about 2e9 fields, and 2e9 * 1e9 * 1001 overflows 64 bits, so the whole
    // seconds are taken out first.
    const std::uint64_t whole = index / m_rate.numerator;
    const std::uint64_t part = index % m_rate.numerator;
    const std::uint64_t ns = whole * 1'000'000'000ull * m_rate.denominator
                             + part * 1'000'000'000ull * m_rate.denominator / m_rate.numerator;
    return m_start + ns;
}

int FramePacer::wait() {
    const Nanoseconds now = m_clock.now();
    std::uint64_t due = m_index + 1;
    if (deadline(due) > now) {
        m_clock.sleep(deadline(due) - now);
        m_index = due;
        return 1;
    }
    // Late: no sleep, and every deadline that has gone by counts, so the
    // game's field counter and the schedule agree.
    std::uint64_t passed = 1;
    while (deadline(due + 1) <= now) {
        ++due;
        ++passed;
        if (passed > kResyncFields) {
            // Far behind (a breakpoint, a slow load): forget the old schedule.
            m_start = now;
            m_index = 0;
            return static_cast<int>(passed);
        }
    }
    m_index = due;
    return static_cast<int>(passed);
}

}  // namespace openrac::platform

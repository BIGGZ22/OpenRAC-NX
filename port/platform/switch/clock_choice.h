// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Which CPU, GPU and memory clocks the Switch build may ask for (platform/switch/clocks.cpp), as a
// plain function the host tests can check. Only Nintendo's own clock steps are offered, and none
// above the official maximum of each: CPU 1785 MHz (the boost mode games use while loading), GPU
// 768 MHz docked and 460.8 MHz handheld, memory 1600 MHz. These are the clocks retail games run
// at somewhere; nothing here goes past them (that would need the OC suites and their risks). A
// value between steps is taken down to the step below it; a value under the lowest step offered,
// or 0, means "leave this clock as the system has it".

#pragma once

#include <array>
#include <cstdint>

namespace openrac::platform::clock_choice {

inline constexpr std::array<std::uint32_t, 7> kCpuHz = {
    1020000000u, 1224000000u, 1326000000u, 1428000000u, 1581000000u, 1683000000u, 1785000000u,
};
inline constexpr std::array<std::uint32_t, 7> kGpuHz = {
    307200000u, 384000000u, 460800000u, 537600000u, 614400000u, 691200000u, 768000000u,
};
inline constexpr std::array<std::uint32_t, 2> kMemHz = {1331200000u, 1600000000u};

// The GPU's ceiling: 768 MHz docked, 460.8 MHz handheld (the most any official profile gives a
// handheld Switch; the console's power budget off the dock is the reason).
inline constexpr std::uint32_t kGpuDockedMaxHz = 768000000u;
inline constexpr std::uint32_t kGpuHandheldMaxHz = 460800000u;

// The step at or below `mhz` megahertz in `steps`, no higher than `cap_hz`; 0 when `mhz` is 0 or
// below the lowest step (leave the clock alone).
template <std::size_t N>
constexpr std::uint32_t step_at_or_below(const std::array<std::uint32_t, N>& steps, std::uint32_t mhz,
                                         std::uint32_t cap_hz) {
    if (mhz == 0) {
        return 0;
    }
    const std::uint64_t asked = static_cast<std::uint64_t>(mhz) * 1000000u;
    std::uint32_t best = 0;
    for (const std::uint32_t hz : steps) {
        // A step of 460.8 MHz is asked for as 460 (or 461): within a megahertz counts.
        if (hz <= asked + 999999u && hz <= cap_hz && hz > best) {
            best = hz;
        }
    }
    return best;
}

struct Choice {
    std::uint32_t cpu_hz = 0, gpu_hz = 0, mem_hz = 0;  // 0: leave as the system has it
};

// The clocks to set for what was asked (MHz, 0 = not asked) in the current mode.
constexpr Choice choose(std::uint32_t cpu_mhz, std::uint32_t gpu_mhz, std::uint32_t mem_mhz, bool docked) {
    Choice c;
    c.cpu_hz = step_at_or_below(kCpuHz, cpu_mhz, kCpuHz.back());
    c.gpu_hz = step_at_or_below(kGpuHz, gpu_mhz, docked ? kGpuDockedMaxHz : kGpuHandheldMaxHz);
    c.mem_hz = step_at_or_below(kMemHz, mem_mhz, kMemHz.back());
    return c;
}

// The raised profile (OPENRAC_SWITCH_CLOCK_PROFILE=raised): CPU 1785, GPU 768, memory 1600 MHz in
// BOTH modes, as the user asked. These are each domain's highest official step, but not a profile
// any retail game runs steadily: 1785 is the CPU's boost (loading) clock, and 768 MHz is the GPU's
// docked clock, above the 460.8 MHz any official handheld profile gives; handheld, the console
// draws more power than Nintendo budgets for there. Opt-in only; hardware validation pending.
constexpr Choice raised_profile() {
    return Choice{kCpuHz.back(), kGpuDockedMaxHz, kMemHz.back()};
}

}  // namespace openrac::platform::clock_choice

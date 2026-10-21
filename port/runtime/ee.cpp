// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the C hostgen writes from assembly needs at run time (openrac/ee.h):
// the vector unit's registers, and the reports of traps and lost jumps.

#include "openrac/ee.h"

#include <cfenv>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "common/log.h"

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <xmmintrin.h>
#define OPENRAC_EE_SSE 1
#endif

using openrac::log::warn;

// The EE's FPU and VU0 round every operation toward zero (they have no other mode) and have no
// denormals: one is read as zero and none is made. The game's code relies on both (a counter
// stepped by 0.1, the animation code's 12582912.0 that removes the fraction), so its thread runs
// so. Constants the compiler folds stay rounded to nearest, as the console's compiler folded
// them too.
extern "C" void openrac_ee_float_mode(int round_nearest, int keep_denormals) {
    if (!round_nearest) {
        std::fesetround(FE_TOWARDZERO);
    }
    if (keep_denormals) {
        return;
    }
#if defined(OPENRAC_EE_SSE)
    _mm_setcsr(_mm_getcsr() | 0x8040u);  // flush to zero (bit 15) and denormals are zero (bit 6)
#elif defined(__aarch64__) || defined(_M_ARM64)
    std::uint64_t fpcr;
    __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));
    fpcr |= std::uint64_t{1} << 24;  // FZ: denormal inputs and results are zero
    __asm__ __volatile__("msr fpcr, %0" : : "r"(fpcr));
#endif
}

// vf0 reads (0, 0, 0, 1) and is never written.
extern "C" openrac_vu0_state openrac_vu0 = {{{0.0f, 0.0f, 0.0f, 1.0f}}, {}, {}, 0.0f, 0.0f, 0.0f, 0.0f};

namespace {
std::mutex g_lock;
std::set<std::pair<std::string, uint32_t>> g_seen;

bool first(const char* function, uint32_t at) {
    std::scoped_lock hold(g_lock);
    return g_seen.emplace(function, at).second;
}
}  // namespace

extern "C" void openrac_ee_break(const char* function, uint32_t address) {
    const char* strict = std::getenv("OPENRAC_STRICT");
    if (strict != nullptr && strict[0] == '1') {
        openrac::log::fatalf("{} reached the trap at {:08X}", function, address);
    }
    if (first(function, address)) {
        warn("{} reached the trap at {:08X} (translated from its assembly); going on", function, address);
    }
}

extern "C" void openrac_ee_lost(const char* function, uint32_t target) {
    if (first(function, target | 1u)) {
        warn("{} jumped through a register to {:08X}, which its tables do not list; returning", function, target);
    }
}

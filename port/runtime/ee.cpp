// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the C hostgen writes from assembly needs at run time (openrac/ee.h):
// the vector unit's registers, and the reports of traps and lost jumps.

#include "openrac/ee.h"

#include <cstdlib>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "common/log.h"

using openrac::log::warn;

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

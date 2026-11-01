// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A diagnostic interval read from the environment ("every N frames"): the value, never the
// variable's presence, decides. Unset, empty, 0, negative or not a number: off (0). A positive N:
// N, held to [lo, hi]. Leading spaces and a sign are read, the digits up to the first other
// character are the number (as std::atoi reads it, without its undefined overflow), so "120" and
// "120 " are 120, "x" and "" are off.
//
// Why (session 55): OPENRAC_DK_COUNTERS was read as max(1, atoi(v)) when set, so "=0" logged every
// frame, and the upload-shape count was on whenever the variable existed. Header-only, no
// allocation: renderer/dkgl and the tests include it.

#pragma once

#include <climits>
#include <cstdint>

namespace openrac::env_interval {

// The interval `v` asks for (std::getenv's result, nullptr when unset): 0 for off, otherwise held
// to [lo, hi] (lo at least 1).
constexpr int parse(const char* v, int lo = 1, int hi = INT_MAX) {
    if (lo < 1) {
        lo = 1;
    }
    if (hi < lo) {
        hi = lo;
    }
    if (v == nullptr) {
        return 0;
    }
    while (*v == ' ' || *v == '\t' || *v == '\n' || *v == '\r' || *v == '\f' || *v == '\v') {
        ++v;
    }
    bool negative = false;
    if (*v == '+' || *v == '-') {
        negative = *v == '-';
        ++v;
    }
    std::int64_t n = 0;
    bool digits = false;
    for (; *v >= '0' && *v <= '9'; ++v) {
        digits = true;
        if (n <= INT_MAX) {
            n = n * 10 + (*v - '0');
        }
    }
    if (!digits || negative || n <= 0) {
        return 0;
    }
    if (n < lo) {
        return lo;
    }
    return n > hi ? hi : static_cast<int>(n);
}

// Whether `v` turns the diagnostic on (a positive interval).
constexpr bool on(const char* v) {
    return parse(v) > 0;
}

}  // namespace openrac::env_interval

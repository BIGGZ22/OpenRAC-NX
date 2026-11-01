// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// common/env_interval.h: a diagnostic interval is off for unset, empty, 0, negative and non-numbers,
// and N for a positive N (session 55: OPENRAC_DK_COUNTERS=0 logged every frame). With the reading
// the dkgl counters use, the per-frame schedule (a line when frames % every == 0) never fires when
// off, and an off value never turns the upload-shape count on.

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>

#include "common/env_interval.h"
#include "tests/check.h"

namespace ei = openrac::env_interval;

namespace {

// dkgl_core.cpp's schedule: how many of `frames` frames write the counter lines.
int lines_over(const char* value, int frames) {
    const int every = ei::parse(value);
    int lines = 0;
    for (std::uint64_t f = 1; f <= static_cast<std::uint64_t>(frames); ++f) {
        if (every > 0 && f % static_cast<std::uint64_t>(every) == 0) {
            ++lines;
        }
    }
    return lines;
}

}  // namespace

int main() {
    // Off.
    CHECK(ei::parse(nullptr) == 0);
    CHECK(ei::parse("") == 0);
    CHECK(ei::parse("0") == 0);
    CHECK(ei::parse("00") == 0);
    CHECK(ei::parse("-1") == 0);
    CHECK(ei::parse("-120") == 0);
    CHECK(ei::parse("off") == 0);
    CHECK(ei::parse("x120") == 0);
    CHECK(ei::parse(" ") == 0);
    CHECK(ei::parse("+") == 0);
    CHECK(ei::parse("-0") == 0);
    CHECK(!ei::on(nullptr) && !ei::on("") && !ei::on("0") && !ei::on("-5"));
    // On, the value kept.
    CHECK(ei::parse("1") == 1);
    CHECK(ei::parse("120") == 120);
    CHECK(ei::parse("+30") == 30);
    CHECK(ei::parse(" 30") == 30);
    CHECK(ei::parse("30 ") == 30);
    CHECK(ei::parse("30x") == 30);  // as std::atoi read it
    CHECK(ei::on("1") && ei::on("120"));
    // Agrees with std::atoi wherever atoi is positive and defined.
    for (const char* v : {"1", "7", "30", "120", "3600", " 42", "99abc"}) {
        CHECK(ei::parse(v) == std::atoi(v));
    }
    // Overflow holds at the top, never wraps to off or negative.
    CHECK(ei::parse("99999999999999999999") == INT_MAX);
    CHECK(ei::parse("2147483648") == INT_MAX);
    CHECK(ei::parse("2147483647") == INT_MAX);
    // Bounds, for the 30..3600 diagnostics: a positive value is held to them, off stays off.
    CHECK(ei::parse("5", 30, 3600) == 30);
    CHECK(ei::parse("100000", 30, 3600) == 3600);
    CHECK(ei::parse("120", 30, 3600) == 120);
    CHECK(ei::parse("0", 30, 3600) == 0);
    CHECK(ei::parse("-3", 30, 3600) == 0);
    CHECK(ei::parse("100", 0, 50) == 50);  // lo under 1 read as 1
    CHECK(ei::parse("100", 200, 50) == 200);  // hi under lo read as lo
    // The counters' schedule over 600 frames: 0 lines off (it was 600 for "0": every frame), 5 at 120.
    CHECK(lines_over(nullptr, 600) == 0);
    CHECK(lines_over("", 600) == 0);
    CHECK(lines_over("0", 600) == 0);
    CHECK(lines_over("-1", 600) == 0);
    CHECK(lines_over("120", 600) == 5);
    CHECK(lines_over("1", 600) == 600);
    // A constant expression: usable in a static initialiser with no allocation.
    static_assert(ei::parse("60") == 60 && ei::parse("0") == 0 && ei::parse(nullptr) == 0);
    return openrac::test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The pool of common/parallel.h: every piece run once, calls one after another reusing the
// threads, none and one piece, and an exception thrown by a piece thrown again by the call.

#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "check.h"
#include "common/parallel.h"

namespace {

namespace parallel = openrac::parallel;

void each_once() {
    for (int round = 0; round < 200; ++round) {
        const std::size_t count = static_cast<std::size_t>(round % 37);
        std::vector<std::atomic<int>> runs(count);
        parallel::for_each(count, [&](std::size_t i) { runs[i].fetch_add(1); });
        bool once = true;
        for (const auto& r : runs) {
            once = once && r.load() == 1;
        }
        CHECK(once);
    }
}

void results_in_place() {
    std::vector<std::size_t> squares(1000);
    parallel::for_each(squares.size(), [&](std::size_t i) { squares[i] = i * i; });
    bool right = true;
    for (std::size_t i = 0; i < squares.size(); ++i) {
        right = right && squares[i] == i * i;
    }
    CHECK(right);
}

void rethrows() {
    std::atomic<int> ran{0};
    bool caught = false;
    try {
        parallel::for_each(64, [&](std::size_t i) {
            ran.fetch_add(1);
            if (i == 17) {
                throw std::runtime_error("piece 17");
            }
        });
    } catch (const std::runtime_error&) {
        caught = true;
    }
    CHECK(caught);
    CHECK(ran.load() == 64);  // the others still ran
    // And the pool still works after it.
    std::atomic<int> after{0};
    parallel::for_each(10, [&](std::size_t) { after.fetch_add(1); });
    CHECK(after.load() == 10);
}

}  // namespace

int main() {
    CHECK(parallel::workers() <= 3);
    each_once();
    results_in_place();
    rethrows();
    return openrac::test::result();
}

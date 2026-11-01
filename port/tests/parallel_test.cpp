// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The pool of common/parallel.h: every piece run once, calls one after another reusing the
// threads, none and one piece, and an exception thrown by a piece thrown again by the call.

#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <thread>
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

// A call from inside a piece (the pool is busy with the outer call) runs on its own caller, and
// every piece of both runs once.
void nested() {
    std::vector<std::atomic<int>> outer(12);
    std::vector<std::atomic<int>> inner(12 * 9);
    parallel::for_each(outer.size(), [&](std::size_t i) {
        outer[i].fetch_add(1);
        parallel::for_each(9, [&](std::size_t k) { inner[i * 9 + k].fetch_add(1); });
    });
    bool once = true;
    for (const auto& r : outer) once = once && r.load() == 1;
    for (const auto& r : inner) once = once && r.load() == 1;
    CHECK(once);
}

// Two threads calling at once (the game's and the render worker's): one has the threads, the
// other runs alone; neither loses or repeats a piece.
void concurrent() {
    std::vector<std::atomic<int>> a(5000), b(5000);
    std::thread other([&] {
        for (int round = 0; round < 20; ++round) {
            parallel::for_each(b.size() / 20, [&](std::size_t i) { b[round * (b.size() / 20) + i].fetch_add(1); });
        }
    });
    for (int round = 0; round < 20; ++round) {
        parallel::for_each(a.size() / 20, [&](std::size_t i) { a[round * (a.size() / 20) + i].fetch_add(1); });
    }
    other.join();
    bool once = true;
    for (const auto& r : a) once = once && r.load() == 1;
    for (const auto& r : b) once = once && r.load() == 1;
    CHECK(once);
}

// Many short calls, as a tick's poses are (OPENRAC_POOL_STATS counts them when set).
void many_calls() {
    std::atomic<std::size_t> total{0};
    for (int round = 0; round < 400; ++round) {
        parallel::for_each(180, [&](std::size_t) { total.fetch_add(1); });
    }
    CHECK(total.load() == 400u * 180u);
}

}  // namespace

int main() {
    CHECK(parallel::workers() <= 3);
    each_once();
    results_in_place();
    rethrows();
    nested();
    concurrent();
    many_calls();
    return openrac::test::result();
}

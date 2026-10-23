// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The joining thread (common/joining_thread.h): its owner going, or being assigned over, waits for
// its thread. The expected values are the counts each test's threads add.

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <utility>
#include <vector>

#include "check.h"
#include "common/joining_thread.h"

namespace {

// The threads each test starts.
constexpr int kThreads = 4;

// Tests that destroying the owners waits for every thread's function to return.
void test_the_owner_going_waits_for_its_thread() {
    std::atomic<int> done{0};
    {
        std::vector<openrac::JoiningThread> threads;
        threads.reserve(kThreads);
        for (int n = 0; n < kThreads; ++n) {
            threads.emplace_back([&done] { done.fetch_add(1); });
        }
    }
    CHECK(done.load() == kThreads);
}

// Tests that the arguments reach the function, as std::thread passes them.
void test_the_arguments_reach_the_function() {
    std::atomic<int> sum{0};
    {
        const openrac::JoiningThread thread([&sum](int a, int b) { sum.fetch_add(a + b); }, 2, 3);
    }
    CHECK(sum.load() == 2 + 3);
}

// Tests that assigning over a running object joins its thread and takes the other's.
void test_assigning_over_one_joins_its_thread_first() {
    std::atomic<bool> first_done{false};
    std::atomic<bool> second_done{false};
    openrac::JoiningThread first([&first_done] { first_done.store(true); });
    openrac::JoiningThread second([&second_done] { second_done.store(true); });
    first = std::move(second);
    CHECK(first_done.load());
    first.join();
    CHECK(second_done.load());
}

// Tests that a moved thread is joined by its new owner, and joining twice or none returns.
void test_a_moved_thread_is_joined_by_its_new_owner() {
    openrac::JoiningThread empty;
    empty.join();
    std::atomic<bool> done{false};
    openrac::JoiningThread running([&done] { done.store(true); });
    openrac::JoiningThread taken(std::move(running));
    taken.join();
    CHECK(done.load());
    taken.join();
}

// Runs the tests in the order they are written; false when one threw.
bool run_tests() {
    try {
        test_the_owner_going_waits_for_its_thread();
        test_the_arguments_reach_the_function();
        test_assigning_over_one_joins_its_thread_first();
        test_a_moved_thread_is_joined_by_its_new_owner();
        return true;
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "joining_thread: %s\n", failure.what());
        return false;
    }
}

}  // namespace

int main() {
    return run_tests() ? openrac::test::result() : EXIT_FAILURE;
}

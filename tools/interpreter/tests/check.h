// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The whole test framework: two macros and a table of functions.
 *
 * A check that fails prints its file, line and expression and the run goes on, so one run shows
 * every failure. `run_tests` prints one line for each test and a total, and returns the exit code
 * for `main`. Tests run one after another on one thread, so the counters need no lock.
 */

#pragma once

#include <cstdio>

/** Number of checks that failed so far, over all tests. */
inline int g_failures = 0;

/** Number of checks run so far, over all tests. */
inline int g_checks = 0;

/**
 * Checks that a condition holds, and counts the check; a failure prints where and what.
 *
 * It is a macro because the message needs `__FILE__`, `__LINE__` and the text of the expression,
 * which only the caller's line has.
 *
 * @param cond The condition that must be true.
 */
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        g_checks++;                                                                                \
        if (!(cond)) {                                                                             \
            g_failures++;                                                                          \
            std::printf("  FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond);                        \
        }                                                                                          \
    } while (0)

/**
 * Checks that two values are equal, and counts the check; a failure prints both in hexadecimal.
 *
 * It is a macro for the same reason as `CHECK`. Both values must convert to `unsigned long long`
 * and should be of the same type, so write the type on the literal.
 *
 * @param a The actual value.
 * @param b The expected value.
 */
#define CHECK_EQ(a, b)                                                                             \
    do {                                                                                           \
        g_checks++;                                                                                \
        auto va = (a);                                                                             \
        auto vb = (b);                                                                             \
        if (!(va == vb)) {                                                                         \
            g_failures++;                                                                          \
            std::printf(                                                                           \
                "  FAILED %s:%d: %s == %s (%llx vs %llx)\n",                                       \
                __FILE__,                                                                          \
                __LINE__,                                                                          \
                #a,                                                                                \
                #b,                                                                                \
                static_cast<unsigned long long>(va),                                               \
                static_cast<unsigned long long>(vb)                                                \
            );                                                                                     \
        }                                                                                          \
    } while (0)

/** One row of a test table: the name that is printed, and the function that runs the test. */
struct TestCase {
    /** The name printed on the line for this test. */
    const char* name;

    /** The test; it reports its failures through `CHECK` and takes no arguments. */
    void (*run)();
};

/**
 * Runs every test of a table in order and prints the result of each.
 *
 * @tparam N Number of tests in the table.
 * @param tests The table.
 * @return 0 when every check passed, 1 when any failed; fit to return from `main`.
 */
template <std::size_t N>
int run_tests(const TestCase (&tests)[N]) {
    // In table order, so the output reads like the table.
    for (const TestCase& t : tests) {
        int before = g_failures;
        t.run();

        // A test passed when no check failed while it ran.
        std::printf("%s %s\n", g_failures == before ? "ok    " : "FAILED", t.name);
    }

    std::printf("%d checks, %d failed\n", g_checks, g_failures);

    // The exit code is 1 when any check of any test failed.
    return g_failures ? 1 : 0;
}

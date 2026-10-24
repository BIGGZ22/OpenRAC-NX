// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The C-boundary wrapper (common/c_boundary.h): a body that returns passes through unreported; one
// that throws a std::exception or anything else is reported as `<where>: <what>` and the call
// returns the failure value; a long message is cut; the default reporter writes to the log. The
// expected values are the messages the wrapper's comment documents, built from what each test
// throws.

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "check.h"
#include "common/c_boundary.h"
#include "common/log.h"

namespace {

namespace c_boundary = openrac::c_boundary;

// What the tests' bodies return when nothing throws, and their failure value.
constexpr int kPlayed = 7;
constexpr int kFailed = -1;

// A thrown value that is not a std::exception, as a library or a mod may throw.
struct NotAStdException {};

// Throws a NotAStdException: what the wrapper reports as an unknown exception.
[[noreturn]] void throw_not_a_std_exception() {
    // Throwing a type outside std::exception's tree is what this test is for.
    // NOLINTNEXTLINE(bugprone-std-exception-baseclass)
    throw NotAStdException{};
}

// The reports record_report heard, in order.
using Reports = std::vector<std::string>;

// Where record_report keeps what it hears: the reporter is a plain function pointer with no user
// pointer. Each test that passes record_report points it at its own list first.
Reports* g_reports = nullptr;

// Keeps `message` in the current test's list: the reporter the tests pass.
void record_report(const char* message) {
    g_reports->emplace_back(message);
}

// Tests that a body that returns gives its value back and reports nothing.
void test_a_body_that_returns_passes_through() {
    // The where is a string_view already: making one from a C string may throw, the call may not.
    constexpr std::string_view kWhere = "sound";
    static_assert(noexcept(c_boundary::call(kWhere, kFailed, [] { return kPlayed; })));

    Reports reports;
    g_reports = &reports;
    const int played = c_boundary::call("sound", kFailed, [] { return kPlayed; }, record_report);
    CHECK(played == kPlayed);
    CHECK(reports.empty());
}

// Tests that a std::exception is reported with its what() and the failure value is returned.
void test_a_std_exception_is_reported_and_returns_the_failure_value() {
    Reports reports;
    g_reports = &reports;
    const int played = c_boundary::call(
        "sound",
        kFailed,
        []() -> int { throw std::runtime_error("cannot play voice 3"); },
        record_report
    );
    CHECK(played == kFailed);
    CHECK(reports.size() == 1);
    CHECK(reports.front() == "sound: cannot play voice 3");
}

// Tests that a thrown value that is not a std::exception is reported as an unknown exception.
void test_a_non_std_exception_is_reported_as_unknown() {
    Reports reports;
    g_reports = &reports;
    const int played = c_boundary::call(
        "sound", kFailed, []() -> int { throw_not_a_std_exception(); }, record_report
    );
    CHECK(played == kFailed);
    CHECK(reports.size() == 1);
    CHECK(reports.front() == "sound: unknown exception");
}

// Tests that a pointer entry point's failure value can be written as nullptr.
void test_a_pointer_failure_value_is_returned() {
    Reports reports;
    g_reports = &reports;
    const char* const name = c_boundary::call(
        "mods",
        nullptr,
        []() -> const char* { throw std::runtime_error("no mod 2"); },
        record_report
    );
    CHECK(name == nullptr);
    CHECK(reports.size() == 1);
    CHECK(reports.front() == "mods: no mod 2");
}

// Tests that a body with no value runs, and that its exception is reported and stops there.
void test_a_void_body_runs_and_its_exception_is_reported() {
    Reports reports;
    g_reports = &reports;
    bool ran = false;
    c_boundary::call("window", [&ran] { ran = true; }, record_report);
    CHECK(ran);
    CHECK(reports.empty());

    c_boundary::call("window", [] { throw std::logic_error("no window yet"); }, record_report);
    c_boundary::call("window", [] { throw_not_a_std_exception(); }, record_report);
    CHECK(reports.size() == 2);
    CHECK(reports.at(0) == "window: no window yet");
    CHECK(reports.at(1) == "window: unknown exception");
}

// Tests that a message longer than the buffer is cut to kMessageBytes, terminator included.
void test_a_long_message_is_cut_to_the_buffer() {
    Reports reports;
    g_reports = &reports;
    const std::string what(c_boundary::kMessageBytes * 2, 'x');
    c_boundary::call("w", [&what] { throw std::runtime_error(what); }, record_report);
    CHECK(reports.size() == 1);
    CHECK(reports.front().size() == c_boundary::kMessageBytes - 1);
    CHECK(reports.front().starts_with("w: xxx"));
}

// Kept by keep_line: whether the log's last lines hold the line the default reporter wrote.
struct LogSearch {
    std::string wanted;  // The text the line ends with.
    bool found = false;  // True once a line ending with it was seen.
};

// Marks `context` (a LogSearch) found when the line ends with its text; log::recent_lines calls it.
void keep_line(const char* text, std::size_t length, void* context) {
    auto* search = static_cast<LogSearch*>(context);
    const std::string line(text, length);
    search->found = search->found || line.ends_with(search->wanted);
}

// Tests that with no reporter named, and with a null one, the report goes to the log as an error.
void test_the_default_and_a_null_reporter_write_to_the_log() {
    const int played = c_boundary::call("mixer", kFailed, []() -> int {
        throw std::runtime_error("default reporter");
    });
    CHECK(played == kFailed);
    c_boundary::call("mixer", [] { throw std::runtime_error("null reporter"); }, nullptr);

    LogSearch default_line{"mixer: default reporter"};
    openrac::log::recent_lines(keep_line, &default_line);
    CHECK(default_line.found);

    LogSearch null_line{"mixer: null reporter"};
    openrac::log::recent_lines(keep_line, &null_line);
    CHECK(null_line.found);
}

// Runs the tests in the order they are written; false when one threw.
bool run_tests() {
    try {
        test_a_body_that_returns_passes_through();
        test_a_std_exception_is_reported_and_returns_the_failure_value();
        test_a_non_std_exception_is_reported_as_unknown();
        test_a_pointer_failure_value_is_returned();
        test_a_void_body_runs_and_its_exception_is_reported();
        test_a_long_message_is_cut_to_the_buffer();
        test_the_default_and_a_null_reporter_write_to_the_log();
        return true;
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "c_boundary: %s\n", failure.what());
        return false;
    }
}

}  // namespace

int main() {
    return run_tests() ? openrac::test::result() : EXIT_FAILURE;
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac::Result<T> (common/result.h): a value or an Error through each accessor, the wrong side
// throwing, copies and moves, and Result<void>. The expected values are the ones each test puts
// in, and std::expected's documented behaviour, which Result follows.

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <format>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "check.h"
#include "common/result.h"

namespace {

using openrac::Error;
using openrac::Result;
using openrac::unexpected;

// The count parse_count accepts and the tests read back.
constexpr int kCount = 12;

// Returns `raw` as a count, or an error for a negative one: the form a public function takes.
Result<int> parse_count(int raw) {
    // A negative count is the malformed input this sample rejects.
    if (raw < 0) {
        return unexpected(Error{std::format("negative count {}", raw)});
    }

    return raw;
}

// Checks that `raw` is not negative: the form a function with no value takes.
Result<void> check_count(int raw) {
    // A negative count is the malformed input this sample rejects.
    if (raw < 0) {
        return unexpected(Error{std::format("negative count {}", raw)});
    }

    return {};
}

// Tests that a value reads back through has_value, operator bool, value, operator* and operator->.
void test_a_value_reads_back_through_every_accessor() {
    const Result<int> count = parse_count(kCount);
    CHECK(count.has_value());
    CHECK(static_cast<bool>(count));
    CHECK(count.value() == kCount);
    CHECK(*count == kCount);

    const Result<std::string> name = std::string("veldin");
    CHECK(name->size() == std::string("veldin").size());
}

// Tests that an error made with unexpected() holds its message and no value.
void test_an_error_holds_its_message() {
    const Result<int> count = parse_count(-1);
    CHECK(!count.has_value());
    CHECK(!static_cast<bool>(count));
    CHECK(count.error().message == "negative count -1");
}

// Tests that value() on an error and error() on a value throw, as std::expected's value() does.
void test_the_wrong_side_throws() {
    const Result<int> failed = parse_count(-1);
    const Result<int> counted = parse_count(kCount);
    bool value_threw = false;
    bool error_threw = false;
    try {
        static_cast<void>(failed.value());
    } catch (const std::bad_variant_access&) {
        value_threw = true;
    }
    try {
        static_cast<void>(counted.error());
    } catch (const std::bad_variant_access&) {
        error_threw = true;
    }
    CHECK(value_threw);
    CHECK(error_threw);
}

// Tests that a value can be changed in place through value() and operator*.
void test_a_value_changes_in_place() {
    Result<std::string> name = std::string("kerwan");
    name.value() += " city";
    *name += " hall";
    CHECK(*name == "kerwan city hall");
}

// Tests that a move-only value moves into a Result and out of it, and the Result is move-only.
void test_a_move_only_value_moves_through() {
    static_assert(!std::is_copy_constructible_v<Result<std::unique_ptr<int>>>);
    static_assert(std::is_nothrow_move_constructible_v<Result<std::unique_ptr<int>>>);

    Result<std::unique_ptr<int>> held = std::make_unique<int>(kCount);
    Result<std::unique_ptr<int>> moved = std::move(held);
    const std::unique_ptr<int> out = std::move(moved).value();
    CHECK(out != nullptr);
    CHECK(*out == kCount);
}

// Tests that a copy of a Result is its own value or error, and moving one moves its error out.
void test_copies_and_moves_keep_the_side_and_the_contents() {
    const Result<std::string> name = std::string("novalis");
    Result<std::string> name_copy = name;
    *name_copy += " prime";
    CHECK(name_copy.has_value());
    CHECK(*name_copy == "novalis prime");
    CHECK(*name == "novalis");

    Result<std::string> failed = unexpected(Error{"cannot read bank 3"});
    const Result<std::string> failed_copy = failed;
    CHECK(!failed_copy.has_value());
    CHECK(failed_copy.error().message == "cannot read bank 3");

    const Error error = std::move(failed).error();
    CHECK(error.message == "cannot read bank 3");
}

// Tests that Result<void> is success when made empty, and holds the error otherwise.
void test_a_void_result_is_success_or_an_error() {
    const Result<void> passed = check_count(kCount);
    CHECK(passed.has_value());
    CHECK(static_cast<bool>(passed));
    passed.value();
    *passed;

    const Result<void> failed = check_count(-2);
    CHECK(!failed.has_value());
    CHECK(!static_cast<bool>(failed));
    CHECK(failed.error().message == "negative count -2");
}

// Tests that Result<void>'s value() on an error and error() on success throw, as Result<T>'s do.
void test_a_void_result_throws_on_the_wrong_side() {
    const Result<void> passed = check_count(kCount);
    const Result<void> failed = check_count(-1);
    bool value_threw = false;
    bool error_threw = false;
    try {
        failed.value();
    } catch (const std::bad_variant_access&) {
        value_threw = true;
    }
    try {
        static_cast<void>(passed.error());
    } catch (const std::bad_variant_access&) {
        error_threw = true;
    }
    CHECK(value_threw);
    CHECK(error_threw);
}

// Tests that Result<void> copies its error and moves it out.
void test_a_void_result_copies_and_moves_its_error() {
    Result<void> failed = unexpected(Error{"cannot open movie 7"});
    const Result<void> failed_copy = failed;
    CHECK(failed_copy.error().message == "cannot open movie 7");

    const Error error = std::move(failed).error();
    CHECK(error.message == "cannot open movie 7");
}

// Runs the tests in the order they are written; false when one threw.
bool run_tests() {
    try {
        test_a_value_reads_back_through_every_accessor();
        test_an_error_holds_its_message();
        test_the_wrong_side_throws();
        test_a_value_changes_in_place();
        test_a_move_only_value_moves_through();
        test_copies_and_moves_keep_the_side_and_the_contents();
        test_a_void_result_is_success_or_an_error();
        test_a_void_result_throws_on_the_wrong_side();
        test_a_void_result_copies_and_moves_its_error();
        return true;
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "result: %s\n", failure.what());
        return false;
    }
}

}  // namespace

int main() {
    return run_tests() ? openrac::test::result() : EXIT_FAILURE;
}

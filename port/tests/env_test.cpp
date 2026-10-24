// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The environment variables' table and reads (common/env.h). The readings are characterisation
// tests: they pin what the code that called getenv for each variable did before it moved onto
// env.h (a value starting with 1 for OPENRAC_STRICT, strtoul in hexadecimal for
// OPENRAC_DUMP_TEXTURE, ...), and each test names the code it was taken from. The table is checked
// against docs/port/DEBUGGING.md, which must document every variable it holds and no other.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "check.h"
#include "common/env.h"

namespace {

using openrac::env::Kind;
using openrac::env::Truth;
using openrac::env::Var;

namespace env = openrac::env;

// One value of a variable (nothing for unset) and whether a flag reads it as on.
struct FlagCase {
    std::optional<std::string_view> value;  // The value, or nothing for an unset variable.
    bool on;                                // Whether the flag is on with it.
};

// One value of a variable (nothing for unset) and the number it reads as.
template <typename Number>
struct NumberCase {
    std::optional<std::string_view> value;  // The value, or nothing for an unset variable.
    Number expected;                        // What it reads as.
};

// The fallback the integer cases pass, which only an unset variable returns.
constexpr std::int64_t kIntegerFallback = 42;

// The fallback the number cases pass, which only an unset variable returns.
constexpr double kNumberFallback = 2.0;

// The bases a Kind::Integer and a Kind::Hex are read in.
constexpr int kDecimal = 10;
constexpr int kHexadecimal = 16;

// Sets the process's variable `name` to `value`, or unsets it when `value` is null.
void put_variable(const char* name, const char* value) {
#if defined(_WIN32)
    // Windows unsets a variable that is given the empty string.
    CHECK(_putenv_s(name, value != nullptr ? value : "") == 0);
#else
    // POSIX has a call of its own for unsetting.
    if (value == nullptr) {
        CHECK(unsetenv(name) == 0);
        return;
    }

    CHECK(setenv(name, value, 1) == 0);
#endif
}

// Checks that a flag read by `truth` is on for exactly the cases that say so.
void check_truth(Truth truth, std::initializer_list<FlagCase> cases) {
    for (const FlagCase& one : cases) {
        CHECK(env::reads_as_on(truth, one.value) == one.on);
    }
}

// Tests that the values are read once, at the first read, and a later change is not seen.
void test_the_values_are_read_once_at_the_first_read() {
    // The first test to read, so the snapshot is taken from what is set here.
    put_variable("OPENRAC_STRICT", "1x");
    put_variable("OPENRAC_DUMP_TEXTURE", "0x1A");
    put_variable("OPENRAC_SCRIPT_BUDGET_MS", "2.5");
    put_variable("OPENRAC_GL_API", "vulkan");
    put_variable("OPENRAC_TRACE_MISSING", nullptr);
    put_variable("OPENRAC_SHOT_FROM", nullptr);
    CHECK(env::flag(Var::Strict));
    CHECK(env::integer(Var::DumpTexture, -1) == 0x1A);
    CHECK(env::number(Var::ScriptBudgetMs, kNumberFallback) == 2.5);
    CHECK(env::value(Var::GlApi) == "vulkan");
    CHECK(std::strcmp(env::value(Var::GlApi)->data(), "vulkan") == 0);  // ends in a NUL, for C
    CHECK(!env::flag(Var::TraceMissing));
    CHECK(!env::value(Var::TraceMissing).has_value());
    CHECK(env::integer(Var::ShotFrom, kIntegerFallback) == kIntegerFallback);

    // Changed after the first read: the program goes on with what it found.
    put_variable("OPENRAC_TRACE_MISSING", "1");
    put_variable("OPENRAC_GL_API", "es");
    CHECK(!env::flag(Var::TraceMissing));
    CHECK(env::value(Var::GlApi) == "vulkan");
}

// Tests that a variable the port does not own is read again at each call.
void test_a_system_variable_is_read_at_each_call() {
    constexpr const char* kName = "OPENRAC_ENV_TEST_SYSTEM_VARIABLE";
    put_variable(kName, nullptr);
    CHECK(!env::system_variable(kName).has_value());
    put_variable(kName, "first");
    CHECK(env::system_variable(kName) == "first");
    put_variable(kName, "second");
    CHECK(env::system_variable(kName) == "second");
    put_variable(kName, nullptr);
}

// Tests that a Set flag is on when set to anything, as `getenv(name) != nullptr` was (guest.cpp's
// traces, direct.cpp, pump.cpp's SDL_getenv).
void test_a_set_flag_is_on_whatever_its_value() {
    check_truth(
        Truth::Set, {{std::nullopt, false}, {"", true}, {"0", true}, {"1", true}, {"off", true}}
    );
}

// Tests that a LeadingOne flag needs a value starting with 1, as `env != nullptr && env[0] == '1'`
// was (OPENRAC_STRICT in guest.cpp and ee.cpp, OPENRAC_VULKAN_VALIDATION in window.cpp).
void test_a_leading_one_flag_needs_a_value_starting_with_one() {
    check_truth(
        Truth::LeadingOne,
        {{std::nullopt, false},
         {"", false},
         {"1", true},
         {"10", true},
         {"1x", true},
         {"0", false},
         {"01", false},
         {" 1", false},
         {"yes", false}}
    );
}

// Tests that a One flag needs the value 1 alone, as gl.cpp's `std::string_view(enhanced) == "1"`
// for OPENRAC_ENHANCED and probe_lighting.cpp's OPENRAC_PROBE_BACKLIGHT were.
void test_a_one_flag_needs_the_value_one() {
    check_truth(
        Truth::One, {{std::nullopt, false}, {"", false}, {"1", true}, {"10", false}, {" 1", false}}
    );
}

// Tests that a NotEmpty flag is on for any value but the empty string, as kernel.c's
// OPENRAC_TRACE_SIF is.
void test_a_not_empty_flag_needs_a_value() {
    check_truth(Truth::NotEmpty, {{std::nullopt, false}, {"", false}, {"0", true}, {"x", true}});
}

// Tests that a NotEmptyOrZero flag is off for the empty string and 0, as debug_menu.cpp's
// OPENRAC_DEVELOPER is (`v[0] != '\0' && std::strcmp(v, "0") != 0`).
void test_a_not_empty_or_zero_flag_is_off_for_zero() {
    check_truth(
        Truth::NotEmptyOrZero,
        {{std::nullopt, false}, {"", false}, {"0", false}, {"00", true}, {"01", true}, {"1", true}}
    );
}

// Tests that an UnlessZero flag is on unless set to 0, as frontend.cpp's OPENRAC_ANIM is
// (`v == nullptr || std::strcmp(v, "0") != 0`).
void test_an_unless_zero_flag_is_on_unless_zero() {
    check_truth(
        Truth::UnlessZero,
        {{std::nullopt, true}, {"", true}, {"0", false}, {"00", true}, {" 0", true}, {"off", true}}
    );
}

// Tests that a variable of another kind never reads as an on flag.
void test_a_variable_that_is_not_a_flag_is_never_on() {
    check_truth(Truth::NotAFlag, {{std::nullopt, false}, {"", false}, {"1", false}});
}

// Tests that a decimal integer reads as std::strtoll reads it, and unset as the fallback.
void test_a_decimal_integer_reads_as_strtoll() {
    const NumberCase<std::int64_t> cases[] = {
        {std::nullopt, kIntegerFallback},
        {"", 0},
        {"12", 12},
        {"  12", 12},
        {"+7", 7},
        {"-3", -3},
        {"12abc", 12},
        {"1.9", 1},
        {"0x10", 0},
        {"abc", 0},
        {"99999999999999999999", std::numeric_limits<std::int64_t>::max()},
    };
    for (const NumberCase<std::int64_t>& one : cases) {
        CHECK(env::read_integer(one.value, kDecimal, kIntegerFallback) == one.expected);
    }
}

// Tests that a hexadecimal integer reads as texture_pool.cpp's `std::strtoul(dump, nullptr, 16)`
// for OPENRAC_DUMP_TEXTURE: the same number for every value a block number can equal.
void test_a_hex_integer_reads_as_strtoul_did() {
    const NumberCase<std::int64_t> cases[] = {
        {std::nullopt, kIntegerFallback},
        {"", 0},
        {"1a", 0x1A},
        {"0x1A", 0x1A},
        {"1A0", 0x1A0},
        {"10", 0x10},
        {"g", 0},
        {"-0", 0},
    };
    for (const NumberCase<std::int64_t>& one : cases) {
        CHECK(env::read_integer(one.value, kHexadecimal, kIntegerFallback) == one.expected);
    }

    // strtoul wrapped a negative value to a number past any block; strtoll keeps it below them.
    CHECK(env::read_integer("-1", kHexadecimal, kIntegerFallback) < 0);
}

// Tests that a number reads as std::atof and std::strtod read it, and unset as the fallback.
void test_a_number_reads_as_strtod() {
    const NumberCase<double> cases[] = {
        {std::nullopt, kNumberFallback},
        {"", 0.0},
        {"0.25", 0.25},
        {"2", 2.0},
        {" 1.5ms", 1.5},
        {"-0.5", -0.5},
        {"1e-3", 1e-3},
        {"abc", 0.0},
    };
    for (const NumberCase<double>& one : cases) {
        CHECK(env::read_number(one.value, kNumberFallback) == one.expected);
    }
}

// Tests that the variables runtime/, platform/ and renderer/ read keep the readings their getenv
// calls had.
void test_the_moved_variables_keep_their_readings() {
    const auto reads_as = [](Var var, Kind kind, Truth truth) {
        return env::variable(var).kind == kind && env::variable(var).truth == truth;
    };
    CHECK(reads_as(Var::Strict, Kind::Flag, Truth::LeadingOne));
    CHECK(reads_as(Var::VulkanValidation, Kind::Flag, Truth::LeadingOne));
    CHECK(reads_as(Var::TraceMissing, Kind::Flag, Truth::Set));
    CHECK(reads_as(Var::TraceRelocation, Kind::Flag, Truth::Set));
    CHECK(reads_as(Var::UiResidual, Kind::Flag, Truth::Set));
    CHECK(reads_as(Var::ShowMissing, Kind::Flag, Truth::Set));
    CHECK(reads_as(Var::NoPumpFiber, Kind::Flag, Truth::Set));
    CHECK(reads_as(Var::DumpTexture, Kind::Hex, Truth::NotAFlag));
    CHECK(reads_as(Var::Enhanced, Kind::Flag, Truth::One));
    CHECK(reads_as(Var::ProbeGi, Kind::Path, Truth::NotAFlag));
    CHECK(reads_as(Var::GlApi, Kind::Text, Truth::NotAFlag));
    CHECK(reads_as(Var::Gpu, Kind::Text, Truth::NotAFlag));
    CHECK(reads_as(Var::GlHideExtensions, Kind::Text, Truth::NotAFlag));
    CHECK(reads_as(Var::VulkanLibrary, Kind::Path, Truth::NotAFlag));
}

// Keeps Vulkan rendering threaded by default while retaining an explicit synchronous reference.
void test_render_worker_is_on_unless_disabled() {
    const env::Variable& row = env::variable(Var::RenderThread);
    CHECK(row.kind == Kind::Flag);
    CHECK(row.truth == Truth::UnlessZero);
    CHECK(row.when_unset == "on (Vulkan)");
    check_truth(row.truth, {{std::nullopt, true}, {"", true}, {"1", true}, {"0", false}});
}

// Returns whether `name` is OPENRAC_ and then upper-case words joined by underscores.
bool is_variable_name(std::string_view name) {
    constexpr std::string_view kPrefix = "OPENRAC_";

    // Every variable of the port's carries the prefix, and something after it.
    if (!name.starts_with(kPrefix) || name.size() == kPrefix.size()) {
        return false;
    }

    for (const char c : name) {
        // Upper snake case: capitals, digits and underscores only.
        if (!(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9') && c != '_') {
            return false;
        }
    }
    return true;
}

// Tests that the table's rows are in order, well named, and described in one line each.
void test_the_table_is_in_order_and_complete() {
    std::string_view previous;
    std::size_t place = 0;
    for (const env::Variable& row : env::all()) {
        CHECK(static_cast<std::size_t>(row.var) == place);
        CHECK(&env::variable(row.var) == &row);
        CHECK(env::find(row.name) == row.var);
        CHECK(is_variable_name(row.name));
        CHECK(previous < row.name);
        CHECK((row.kind == Kind::Flag) == (row.truth != Truth::NotAFlag));
        CHECK(!row.when_unset.empty());
        CHECK(!row.description.empty());
        CHECK(row.description.find('\n') == std::string_view::npos);
        previous = row.name;
        ++place;
    }
    CHECK(place == static_cast<std::size_t>(Var::Write) + 1);

    // find takes the whole name, exactly.
    CHECK(!env::find("OPENRAC_NOT_A_VARIABLE").has_value());
    CHECK(!env::find("TRACE_CD").has_value());
    CHECK(!env::find("openrac_trace_cd").has_value());
    CHECK(!env::find("").has_value());
}

// Returns DEBUGGING.md, found from this file's place in the repository (port/tests/).
std::filesystem::path debugging_page() {
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "docs"
           / "port" / "DEBUGGING.md";
}

// Returns the variables a row of DEBUGGING.md's tables opens with, under its "Environment
// variables" headings; the build's own (### Build-time) are not the program's and are left out.
std::set<std::string> documented_variables(const std::filesystem::path& page) {
    std::ifstream file(page);
    CHECK(file.is_open());
    constexpr std::string_view kRowStart = "| `OPENRAC_";

    // The name starts after "| `" and ends at the next backquote.
    constexpr std::size_t kNameStart = 3;

    std::set<std::string> names;
    bool in_variables = false;
    std::string line;
    while (std::getline(file, line)) {
        // A heading of the page's own sections starts or ends the variables' part.
        if (line.starts_with("## ")) {
            in_variables = line.starts_with("## Environment variables");
        } else if (line.starts_with("### ")) {
            // The build-time variables come after the program's, under a heading of their own.
            in_variables = in_variables && line != "### Build-time";
        }

        // Only a table row that opens with a variable's name documents it.
        if (!in_variables || !line.starts_with(kRowStart)) {
            continue;
        }

        const std::size_t end = line.find('`', kNameStart);
        names.insert(line.substr(kNameStart, end - kNameStart));
    }
    return names;
}

// Returns the names of `names` that `others` lacks, in order.
std::vector<std::string> lacking(
    const std::set<std::string>& names, const std::set<std::string>& others
) {
    std::vector<std::string> missing;
    std::ranges::set_difference(names, others, std::back_inserter(missing));
    return missing;
}

// Tests that DEBUGGING.md has a row for every variable of the table, and none for another.
void test_debugging_md_documents_the_table() {
    const std::set<std::string> documented = documented_variables(debugging_page());
    std::set<std::string> in_table;
    for (const env::Variable& row : env::all()) {
        in_table.emplace(row.name);
    }

    // Each difference is named, so that the failure says which row to write or remove.
    const std::vector<std::string> undocumented = lacking(in_table, documented);
    for (const std::string& name : undocumented) {
        std::fprintf(stderr, "env: DEBUGGING.md has no row for %s\n", name.c_str());
    }
    const std::vector<std::string> unknown = lacking(documented, in_table);
    for (const std::string& name : unknown) {
        std::fprintf(
            stderr, "env: DEBUGGING.md documents %s, which the table lacks\n", name.c_str()
        );
    }

    CHECK(undocumented.empty());
    CHECK(unknown.empty());
}

// Runs the tests in the order they are written; false when one threw.
bool run_tests() {
    try {
        test_the_values_are_read_once_at_the_first_read();
        test_a_system_variable_is_read_at_each_call();
        test_a_set_flag_is_on_whatever_its_value();
        test_a_leading_one_flag_needs_a_value_starting_with_one();
        test_a_one_flag_needs_the_value_one();
        test_a_not_empty_flag_needs_a_value();
        test_a_not_empty_or_zero_flag_is_off_for_zero();
        test_an_unless_zero_flag_is_on_unless_zero();
        test_a_variable_that_is_not_a_flag_is_never_on();
        test_a_decimal_integer_reads_as_strtoll();
        test_a_hex_integer_reads_as_strtoul_did();
        test_a_number_reads_as_strtod();
        test_the_moved_variables_keep_their_readings();
        test_render_worker_is_on_unless_disabled();
        test_the_table_is_in_order_and_complete();
        test_debugging_md_documents_the_table();
        return true;
    } catch (const std::exception& failure) {
        std::fprintf(stderr, "env: %s\n", failure.what());
        return false;
    }
}

}  // namespace

int main() {
    return run_tests() ? openrac::test::result() : EXIT_FAILURE;
}

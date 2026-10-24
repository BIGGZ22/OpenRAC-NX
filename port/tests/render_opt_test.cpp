// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests the optimisation registry (renderer/render_opt.h). One program, two ctest tests:
//
//   render_opt              the table and the parsing of OPENRAC_RENDER_OPT_OFF, no GPU; it runs in
//                           the headless preset too
//   render_opt_environment  --environment: the variable as ctest sets it, read and applied
//
// The expected values come from the table itself.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

#include "common/log.h"
#include "renderer/render_opt.h"
#include "tests/check.h"

namespace {

namespace render_opt = openrac::renderer::render_opt;
using render_opt::Opt;

// The text ctest gives OPENRAC_RENDER_OPT_OFF for render_opt_environment (cmake/Render.cmake).
constexpr std::string_view kUnknownInEnvironment = "no-such-optimisation";

// ---- The table ----

// Tests that every entry has a name of its own, at its Opt's place, and that find returns it.
void test_each_entry_has_a_name_of_its_own() {
    const auto entries = render_opt::entries();
    CHECK(entries.size() == render_opt::kOptCount);

    // Each entry at its place, findable, and named unlike every entry after it.
    for (std::size_t n = 0; n < entries.size(); ++n) {
        CHECK(static_cast<std::size_t>(entries[n].opt) == n);
        CHECK(!entries[n].name.empty());
        CHECK(entries[n].name != "all");
        CHECK(render_opt::find(entries[n].name) == &entries[n]);

        // The names after this one, so each pair is compared once.
        for (std::size_t other = n + 1; other < entries.size(); ++other) {
            CHECK(entries[n].name != entries[other].name);
        }
    }

    CHECK(render_opt::find("") == nullptr);
    CHECK(render_opt::find("Registry-Selftest") == nullptr);
}

// Tests that the self-test entry reads back as the table holds it: on by default, exact frames.
void test_the_selftest_entry_reads_back() {
    const render_opt::Entry& selftest = render_opt::entry(Opt::RegistrySelftest);
    CHECK(selftest.name == "registry-selftest");
    CHECK(selftest.default_on);
    CHECK(selftest.tolerance.exact());
    CHECK(selftest.tolerance == render_opt::Tolerance{});
    CHECK(!selftest.description.empty());
}

// Tests that a tolerance other than zero is not exact.
void test_a_tolerance_other_than_zero_is_not_exact() {
    CHECK(!(render_opt::Tolerance{0.0002, 0}.exact()));
    CHECK(!(render_opt::Tolerance{0.0, 2}.exact()));
}

// ---- Parsing OPENRAC_RENDER_OPT_OFF ----

// Tests that one name switches its entry off and nothing else.
void test_a_name_is_parsed() {
    const render_opt::OffList list = render_opt::parse_off_list("registry-selftest");
    CHECK(list.off.size() == 1);
    CHECK(!list.off.empty() && list.off[0] == Opt::RegistrySelftest);
    CHECK(!list.all);
    CHECK(list.unknown.empty());
}

// Tests that a list with spaces, empty items and a repeated name lists each entry once.
void test_a_list_is_parsed() {
    const render_opt::OffList list =
        render_opt::parse_off_list(" registry-selftest ,, registry-selftest,\t");
    CHECK(list.off.size() == 1);
    CHECK(!list.all);
    CHECK(list.unknown.empty());
}

// Tests that the word all says every entry, alone or in a list.
void test_all_is_parsed() {
    CHECK(render_opt::parse_off_list("all").all);
    CHECK(render_opt::parse_off_list("registry-selftest, all").all);
    CHECK(!render_opt::parse_off_list("ALL").all);
}

// Tests that an unknown name is kept apart, in order, and switches nothing.
void test_an_unknown_name_is_kept_apart() {
    const render_opt::OffList list =
        render_opt::parse_off_list("first-unknown,registry-selftest,second-unknown");
    CHECK(list.unknown.size() == 2);
    CHECK(list.unknown.size() == 2 && list.unknown[0] == "first-unknown");
    CHECK(list.unknown.size() == 2 && list.unknown[1] == "second-unknown");
    CHECK(list.off.size() == 1);
    CHECK(render_opt::parse_off_list("").off.empty());
    CHECK(render_opt::parse_off_list(" , ").unknown.empty());
}

// ---- The switches ----

// Tests that set switches an entry over what the run began with, and reset puts it back.
void test_set_and_reset() {
    const bool began = render_opt::enabled(Opt::RegistrySelftest);
    render_opt::set(Opt::RegistrySelftest, !began);
    CHECK(render_opt::enabled(Opt::RegistrySelftest) == !began);
    render_opt::reset();
    CHECK(render_opt::enabled(Opt::RegistrySelftest) == began);
}

// Marks the text sought as found when the recent log line `text` holds it; `context` is the
// text and the mark (log::recent_lines' callback).
void find_in_line(const char* text, std::size_t length, void* context) {
    auto* wanted = static_cast<std::pair<std::string_view, bool>*>(context);
    wanted->second =
        wanted->second
        || std::string_view(text, length).find(wanted->first) != std::string_view::npos;
}

// Tests that the variable ctest sets is applied: the self-test entry off, the unknown name warned
// of, set and reset over it as over a default.
int test_the_environment_is_applied() {
    CHECK(!render_opt::enabled(Opt::RegistrySelftest));
    std::pair<std::string_view, bool> wanted{kUnknownInEnvironment, false};
    openrac::log::recent_lines(find_in_line, &wanted);
    CHECK(wanted.second);
    render_opt::set(Opt::RegistrySelftest, true);
    CHECK(render_opt::enabled(Opt::RegistrySelftest));
    render_opt::reset();
    CHECK(!render_opt::enabled(Opt::RegistrySelftest));
    return openrac::test::result();
}

}  // namespace

int main(int argc, char** argv) {
    const std::string_view mode = argc > 1 ? argv[1] : "";

    // The variable as ctest sets it for render_opt_environment.
    if (mode == "--environment") {
        return test_the_environment_is_applied();
    }

    test_each_entry_has_a_name_of_its_own();
    test_the_selftest_entry_reads_back();
    test_a_tolerance_other_than_zero_is_not_exact();
    test_a_name_is_parsed();
    test_a_list_is_parsed();
    test_all_is_parsed();
    test_an_unknown_name_is_kept_apart();
    test_set_and_reset();
    return openrac::test::result();
}

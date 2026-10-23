// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The probes of common/stats.h: the OPENRAC_* switches a module declares with stats::probe, which
// the overlay lists and turns on and off. These are characterisation tests, written before probe
// moved onto common/env.h: they pin what it did when it called getenv itself (a switch is on when
// its variable is set, whatever the value, and each variable has one probe).

#include <cstdlib>
#include <cstring>

#include "check.h"
#include "common/stats.h"

namespace {

namespace stats = openrac::stats;

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

// Returns the listed probe of `variable`, or null when none is listed.
stats::Probe* listed(const char* variable) {
    // The list is in the order the probes were declared, one per variable.
    for (stats::Probe* probe : stats::probes()) {
        // The probe of that variable, by name, as the overlay and the tests' references find it.
        if (std::strcmp(probe->variable, variable) == 0) {
            return probe;
        }
    }
    return nullptr;
}

// Returns how many listed probes are of `variable`.
int times_listed(const char* variable) {
    int count = 0;
    // Every listed probe, since a variable listed twice is what this counts.
    for (const stats::Probe* probe : stats::probes()) {
        // One of that variable, by name.
        if (std::strcmp(probe->variable, variable) == 0) {
            ++count;
        }
    }
    return count;
}

// Sets the variables the tests probe. Run first, before anything reads the environment, and each
// variable is probed once only, since a probe keeps what it read the first time.
void set_the_variables() {
    put_variable("OPENRAC_CAM_LOG", "1");
    put_variable("OPENRAC_FPS", "0");
    put_variable("OPENRAC_HITCH", "00");
    put_variable("OPENRAC_DEBUG", nullptr);
    put_variable("OPENRAC_PACING", "1");
    put_variable("OPENRAC_SHOT_BETWEEN", nullptr);
#if !defined(_WIN32)
    // Windows cannot hold a variable set to the empty string.
    put_variable("OPENRAC_PVS_OFF", "");
#endif
}

// Tests that a switch is on when its variable is set, whatever the value: 1, 0, 00 and empty.
void test_a_set_switch_is_on_whatever_its_value() {
    CHECK(stats::probe("OPENRAC_CAM_LOG", "the camera, in the log"));
    CHECK(stats::probe("OPENRAC_FPS", "the ticks per second, in the log"));
    CHECK(stats::probe("OPENRAC_HITCH", "each slow tick, in the log"));
#if !defined(_WIN32)
    CHECK(stats::probe("OPENRAC_PVS_OFF", "no baked camera visibility"));
#endif
}

// Tests that a switch is off when its variable is unset.
void test_an_unset_switch_is_off() {
    CHECK(!stats::probe("OPENRAC_DEBUG", "the game's state, in the log"));
}

// Tests that a second probe of a variable is the first: the same switch, listed once, keeping the
// first description.
void test_a_variable_has_one_probe() {
    const bool& first = stats::probe("OPENRAC_PACING", "each frame shown, in the log");
    const bool& second = stats::probe("OPENRAC_PACING", "another description");

    CHECK(&first == &second);
    CHECK(times_listed("OPENRAC_PACING") == 1);

    const stats::Probe* probe = listed("OPENRAC_PACING");
    CHECK(probe != nullptr && std::strcmp(probe->what, "each frame shown, in the log") == 0);
}

// Tests that turning a listed probe on and off, as the overlay does, is what its callers then read.
void test_the_list_switches_a_probe() {
    const bool& between = stats::probe("OPENRAC_SHOT_BETWEEN", "the frames between ticks");
    stats::Probe* probe = listed("OPENRAC_SHOT_BETWEEN");
    CHECK(probe != nullptr);

    // The rest of the test needs the listed probe.
    if (probe == nullptr) {
        return;
    }

    probe->on = true;
    CHECK(between);
    probe->on = false;
    CHECK(!between);
}

}  // namespace

int main() {
    set_the_variables();
    test_a_set_switch_is_on_whatever_its_value();
    test_an_unset_switch_is_off();
    test_a_variable_has_one_probe();
    test_the_list_switches_a_probe();
    return openrac::test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The probes of common/stats.h: the OPENRAC_* switches a module declares with stats::probe, which
// the overlay lists and turns on and off. These are characterisation tests, written before probe
// moved onto common/env.h: they pin what it did when it called getenv itself (a switch is on when
// its variable is set, whatever the value, and each variable has one probe). The tests after them
// came with the move: each probe's variable is a flag of the table read as set, and a name the
// table does not hold as a flag starts off.

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>

#include "check.h"
#include "common/env.h"
#include "common/stats.h"

namespace {

namespace env = openrac::env;
namespace stats = openrac::stats;

// The variables the port's modules declare through stats::probe, all 31: what probe accepted
// while it called getenv, on when set to anything, is what each row of the table must keep.
constexpr std::array<std::string_view, 31> kProbedVariables = {
    "OPENRAC_CAMERA_INSTANCE_CULL_OFF",
    "OPENRAC_CAM_LOG",
    "OPENRAC_DEBUG",
    "OPENRAC_DEPTH_REUSE_OFF",
    "OPENRAC_EARLY_LIGHTING_OFF",
    "OPENRAC_FPS",
    "OPENRAC_GPU_DRIVEN_OFF",
    "OPENRAC_GPU_MATERIALS_OFF",
    "OPENRAC_HERO_SHADOW",
    "OPENRAC_HITCH",
    "OPENRAC_MOBY_SPHERE_CULL_OFF",
    "OPENRAC_OPAQUE_COLOUR_BATCH_OFF",
    "OPENRAC_OPAQUE_SPECIALISE_OFF",
    "OPENRAC_PACING",
    "OPENRAC_PROBE_COMPUTE_OFF",
    "OPENRAC_PROBE_TILED_REFERENCE",
    "OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE",
    "OPENRAC_PVS_OFF",
    "OPENRAC_SHADOW_ATLAS_OFF",
    "OPENRAC_SHADOW_BATCH_OFF",
    "OPENRAC_SHADOW_FILTER_REFERENCE",
    "OPENRAC_SHADOW_GATHER_OFF",
    "OPENRAC_SHADOW_LAYER_REFERENCE",
    "OPENRAC_SHADOW_SOLID_OFF",
    "OPENRAC_SHOT_BETWEEN",
    "OPENRAC_SUN_INSTANCE_CULL_OFF",
    "OPENRAC_SUN_RECEIVER_CULL_OFF",
    "OPENRAC_SUN_TIMING_DETAIL",
    "OPENRAC_VK_EFFECT_UPLOAD_REUSE_OFF",
    "OPENRAC_VK_PARTICLE_TEXTURE_REUSE_OFF",
    "OPENRAC_VK_UNIFORM_REUSE_OFF",
};

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
    put_variable("OPENRAC_NOT_IN_THE_TABLE", "1");
    put_variable("OPENRAC_SHOT_FROM", "1");
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

// Tests that every probed variable is a flag of the table that reads as on when set to anything.
void test_each_probed_variable_is_a_flag_read_as_set() {
    // Each of them, since a probe added with another reading is what this would find.
    for (const std::string_view name : kProbedVariables) {
        const std::optional<env::Var> var = env::find(name);
        CHECK(var.has_value());

        // The row's reading needs the row.
        if (!var) {
            continue;
        }

        CHECK(env::variable(*var).kind == env::Kind::Flag);
        CHECK(env::variable(*var).truth == env::Truth::Set);
    }
}

// Tests that a set variable the table lacks, or holds as no flag, starts its probe off, listed.
void test_a_name_that_is_no_flag_of_the_table_starts_off() {
    CHECK(!stats::probe("OPENRAC_NOT_IN_THE_TABLE", "a probe without its row"));
    CHECK(times_listed("OPENRAC_NOT_IN_THE_TABLE") == 1);
    CHECK(!stats::probe("OPENRAC_SHOT_FROM", "a variable of another kind"));
}

// Counter measurements remain independent of overlay and dump ownership.
void test_frame_counter_measurements_have_their_own_client() {
    stats::set_enabled(stats::Client::FrameCounter, true);
    CHECK(stats::enabled());
    stats::set_enabled(stats::Client::Overlay, true);
    stats::set_enabled(stats::Client::FrameCounter, false);
    CHECK(stats::enabled());
    stats::set_enabled(stats::Client::Dump, true);
    stats::set_enabled(stats::Client::Overlay, false);
    CHECK(stats::enabled());
    stats::set_enabled(stats::Client::Dump, false);
    CHECK(!stats::enabled());
}

// The frames dump (a capture's .jsonl): one line of JSON a frame, its names escaped.
void test_the_dump_writes_a_line_a_frame() {
    const std::string path = "stats_test_dump.jsonl";
    std::string error;
    CHECK(stats::open_dump(path, error));
    stats::begin_frame(stats::FrameKind::Tick, 7);
    {
        const stats::Scope part("a\"b");
        stats::gauge("g\\x", 1.5);
    }
    stats::begin_frame(stats::FrameKind::Between, 7);
    stats::close_dump();
    std::FILE* in = std::fopen(path.c_str(), "r");
    CHECK(in != nullptr);
    std::string text;
    if (in != nullptr) {
        char buffer[4096];
        std::size_t n = 0;
        while ((n = std::fread(buffer, 1, sizeof buffer, in)) > 0) {
            text.append(buffer, n);
        }
        std::fclose(in);
    }
    std::remove(path.c_str());
    const std::string_view first = std::string_view(text).substr(0, text.find('\n') + 1);
    CHECK(first.starts_with("{\"frame\":"));
    CHECK(first.find(",\"tick\":7,\"kind\":\"tick\",\"alpha\":1.000,") != std::string_view::npos);
    CHECK(first.find("{\"name\":\"a\\\"b\",\"depth\":0,\"cpu_ms\":") != std::string_view::npos);
    CHECK(first.find("\"gauges\":{\"g\\\\x\":1.5}") != std::string_view::npos);
    CHECK(first.ends_with("}}\n"));
}

}  // namespace

int main() {
    test_the_dump_writes_a_line_a_frame();
    test_frame_counter_measurements_have_their_own_client();
    set_the_variables();
    test_a_set_switch_is_on_whatever_its_value();
    test_an_unset_switch_is_off();
    test_a_variable_has_one_probe();
    test_the_list_switches_a_probe();
    test_each_probed_variable_is_a_flag_read_as_set();
    test_a_name_that_is_no_flag_of_the_table_starts_off();
    return openrac::test::result();
}

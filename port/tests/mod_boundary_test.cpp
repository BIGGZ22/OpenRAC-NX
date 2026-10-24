// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The script runtime's C entry point (openrac/script_binding.h) stops an exception at the C boundary
// (common/c_boundary.h, CODING_CONVENTIONS.md section 13.4): a script mod hooked through the real
// loader onto a hook point of the test's own, whose call throws. The throw is reported to the log as
// `openrac_script_dispatch: <what>`, the entry point returns 0 (the contract's "call the original as
// it is"), and the game goes on with its original's result. Before the wrapper, the exception reached
// the C thunk, and the program ended in std::terminate.
//
// openrac_script_hook_id and openrac_game_fix go through the same wrapper, but nothing a test can
// reach makes their bodies throw (only an allocation in the log could); mod_scripts_test and
// game_fixes_test keep their answers when nothing throws.

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "check.h"
#include "common/log.h"
#include "mods.h"
#include "openrac/memory.h"
#include "openrac/script_binding.h"

namespace fs = std::filesystem;
using namespace openrac;

namespace {

// What the hook point's call throws, and the report the log is to have of it.
constexpr std::string_view kThrown = "the hook point's call threw";
constexpr std::string_view kReport = "openrac_script_dispatch: the hook point's call threw";

// The hook point, as hostgen's hooks.c writes one: the game's function, and the slot a mod fills.
void* g_slot = nullptr;

int throwing_game(int x) {
    return x + 1;
}

int throwing(int x) {
    if (g_slot != nullptr) {
        return reinterpret_cast<int (*)(int)>(g_slot)(x);
    }

    return throwing_game(x);
}

const mods::HookPoint kHooks[] = {
    {"Test_Throw", "func_00100000", "i32(i32)", &g_slot, reinterpret_cast<void*>(throwing_game)},
};

// The binding, as sdk/tools/gen_binding.py writes one, for one i32(i32) hook point whose call to
// its original throws instead of calling it.
const std::uint8_t kKinds[] = {OPENRAC_SV_I32};
const char* const kNames[] = {"x"};
openrac_script_link g_links[OPENRAC_SCRIPT_LINKS];

// The binding's openrac_script_call_fn: throws, as a hook point's call that fails might.
void throwing_call(void*, const openrac_script_value*, openrac_script_value*) {
    throw std::runtime_error(std::string(kThrown));
}

// The hook point's entry in the binding, defined below: it names the thunks, which name it.
const openrac_script_point& point();

// The link function of place `Place` in the chain: the script runtime's, else the original as it is.
template <int Place>
int thunk(int x) {
    const openrac_script_value args[1] = {{x, 0.0}};
    openrac_script_value result{0, 0.0};

    if (openrac_script_dispatch(&point(), &g_links[Place], args, &result) == 0) {
        return reinterpret_cast<int (*)(int)>(g_links[Place].original)(x);
    }

    return static_cast<int>(result.i);
}

void* const kThunks[OPENRAC_SCRIPT_LINKS] = {
    reinterpret_cast<void*>(thunk<0>),
    reinterpret_cast<void*>(thunk<1>),
    reinterpret_cast<void*>(thunk<2>),
    reinterpret_cast<void*>(thunk<3>),
    reinterpret_cast<void*>(thunk<4>),
    reinterpret_cast<void*>(thunk<5>),
    reinterpret_cast<void*>(thunk<6>),
    reinterpret_cast<void*>(thunk<7>),
};

const openrac_script_point kPoints[] = {
    {"Test_Throw", "i32(i32)", 1, kKinds, kNames, OPENRAC_SV_I32, g_links, kThunks, throwing_call},
};

const openrac_script_point& point() {
    return kPoints[0];
}

const char* const kDiscs[] = {"one"};

// The binding of the test's game: one disc, one hook point, no aliases, fields or mobys.
openrac_script_binding binding() {
    openrac_script_binding b{};
    b.game = "test";
    b.disc_count = 1;
    b.discs = kDiscs;
    b.point_count = 1;
    b.points = kPoints;
    return b;
}

void write(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << text;
}

// A script mod that takes the hook point before its original, and the launcher's list with it on.
void install(const fs::path& plugins) {
    write(plugins / "before" / "main.luau", "openrac.before(\"Test_Throw\", function(x) end)\n");
    write(
        plugins / "before" / "openrac-mod.json", R"({"format": 2, "id": "before", "name": "Before",
        "version": "1.0.0", "type": "script", "api": "1.1", "scripts": ["main.luau"],
        "games": {"test": ["one"]}, "hooks": ["Test_Throw"], "settings": []})"
    );
    write(
        plugins / "plugins.json", R"({"format": 1, "mods": [{"id": "before", "enabled": true}]})"
    );
}

// How many of the log's recent lines contain `text`.
int lines_with(std::string_view text) {
    struct Search {
        std::string_view text;
        int found = 0;
    } search{text, 0};

    log::recent_lines(
        [](const char* line, std::size_t length, void* context) {
            auto* s = static_cast<Search*>(context);

            if (std::string_view(line, length).find(s->text) != std::string_view::npos) {
                ++s->found;
            }
        },
        &search
    );
    return search.found;
}

// Tests that a throw from the hook point's call is reported, gives the failure value, and leaves
// the game going on with the original.
void test_a_throwing_call_is_reported(const fs::path& plugins) {
    install(plugins);
    const openrac_script_binding b = binding();
    mods::Host host;
    host.game = "test";
    host.version = "one";
    host.openrac_version = "0.0.0";
    host.hooks = kHooks;
    host.hook_count = 1;
    host.list = plugins / "plugins.json";
    host.now = "2026-10-23T00:00:00+00:00";
    host.scripts = &b;
    CHECK(mods::load(host) == "before 1.0.0");
    CHECK(g_slot == kThunks[0]);
    CHECK(lines_with(kReport) == 0);

    // The entry point itself: the failure value, and one report.
    const openrac_script_value args[1] = {{4, 0.0}};
    openrac_script_value result{0, 0.0};
    CHECK(openrac_script_dispatch(&point(), &g_links[0], args, &result) == 0);
    CHECK(lines_with(kReport) == 1);

    // Through the hook point: the original runs as it is, and the game has its result.
    CHECK(throwing(4) == 5);
    CHECK(lines_with(kReport) == 2);

    mods::reset();
    CHECK(g_slot == nullptr && throwing(4) == 5);
}

}  // namespace

int main() {
    runtime::Memory::create();
    const fs::path dir = fs::temp_directory_path()
                         / ("openrac_mod_boundary_test_" + std::to_string(std::random_device{}()));
    fs::remove_all(dir);
    test_a_throwing_call_is_reported(dir / "plugins");
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    return test::result();
}

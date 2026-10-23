// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The mod loader (game/common/mods.h) with real mods: mod_loader_test_mod.c built twice, as
// test-a and test-b, installed into a plugins folder of the test's own with hook points of its own
// (one, Test_Add, written as hostgen's hooks.c writes them). Load, hook chaining in load order,
// settings, frame and pad callbacks, a mod's own refusal, an unknown hook point and a wrong
// signature, status.json, and the slots left null when nothing loads.

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "check.h"
#include "common/json.h"
#include "mod_manifest.h"
#include "mods.h"
#include "openrac/guest.h"
#include "openrac/script_binding.h"

namespace fs = std::filesystem;
using namespace openrac;

namespace {

// The hook point, as hooks.c writes one.
void* g_slot_add = nullptr;
int add_game(int x) {
    return x + 1;
}
int add(int x) {
    if (g_slot_add != nullptr) {
        return reinterpret_cast<int (*)(int)>(g_slot_add)(x);
    }
    return add_game(x);
}
const mods::HookPoint kHooks[] = {
    {"Test_Add", "func_00100000", "i32(i32)", &g_slot_add, reinterpret_cast<void*>(add_game)},
};

// The function the mods find by name (api->function): what they report.
std::vector<int> g_records;
extern "C" void test_record(int v) {
    g_records.push_back(v);
}

void (*g_pad_hook)(int, unsigned char*, int) = nullptr;

void write(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << text;
}

std::string read(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream s;
    s << in.rdbuf();
    return s.str();
}

const char* library_name() {
#if defined(_WIN32)
    return "test.dll";
#elif defined(__APPLE__)
    return "libtest.dylib";
#else
    return "libtest.so";
#endif
}

// Installs test-<which> into `plugins`: its library copied in, its manifest.
void install(const fs::path& plugins, const std::string& which, const char* built, const std::string& api = "1.3") {
    const fs::path dir = plugins / ("test-" + which);
    fs::create_directories(dir / "lib");
    fs::copy_file(built, dir / "lib" / library_name(), fs::copy_options::overwrite_existing);
    write(dir / "openrac-mod.json", R"({"format": 2, "id": "test-)" + which + R"(", "name": "Test", "version": "1.0.0",
        "api": ")" + api + R"(", "games": {"test": ["one"]}, "hooks": ["Test_Add"],
        "plugin": {")" + std::string(mods::platform_key()) + R"(": "lib/)" + library_name() + R"("},
        "settings": [{"key": "tag", "name": "Tag", "type": "int", "default": 0},
                     {"key": "behaviour", "name": "Behaviour", "type": "choice", "default": "ok",
                      "choices": [{"value": "ok", "name": "OK"}, {"value": "fail", "name": "Fail"},
                                  {"value": "unknown_hook", "name": "Unknown"},
                                  {"value": "bad_signature", "name": "Bad"},
                                  {"value": "input", "name": "Input"}]},
                     {"key": "jump", "name": "Jump", "type": "keybind", "default": "Space"}]})");
}

mods::Host host(const fs::path& plugins) {
    mods::Host h;
    h.game = "test";
    h.version = "one";
    h.openrac_version = "0.0.0";
    h.hooks = kHooks;
    h.hook_count = 1;
    h.list = plugins / "plugins.json";
    h.pad_hook = &g_pad_hook;
    h.now = "2026-10-21T00:00:00+00:00";
    return h;
}

// The state status.json gives a mod, and its reason.
std::pair<std::string, std::string> status_of(const fs::path& plugins, const std::string& id) {
    json::Value v;
    std::string error;
    if (!json::parse(read(plugins / "status.json"), v, error)) {
        return {"unreadable: " + error, ""};
    }
    for (const auto& m : v["mods"].items()) {
        if (m["id"].string() == id) {
            return {m["state"].string(), m["reason"].is_string() ? m["reason"].string() : ""};
        }
    }
    return {"absent", ""};
}

void two_mods_chain_in_load_order(const fs::path& plugins) {
    write(plugins / "plugins.json", R"({"format": 1, "mods": [
        {"id": "test-a", "enabled": true, "settings": {"tag": 1}},
        {"id": "test-b", "enabled": true, "settings": {"tag": 2}}]})");
    g_records.clear();
    const std::string line = mods::load(host(plugins));
    CHECK(line == "test-a 1.0.0, test-b 1.0.0");
    CHECK(mods::loaded_count() == 2);
    // b loaded last, so it runs first: b(a(game)) = ((0 + 1) * 10 + 1) * 10 + 2.
    CHECK(add(0) == 112);
    CHECK(status_of(plugins, "test-a").first == "loaded");
    CHECK(status_of(plugins, "test-b").first == "loaded");
    CHECK(read(plugins / "status.json").find("\"hook_points\": [\"Test_Add\"]") != std::string::npos);
    // Frame callbacks in load order; a hook taken in one (after load) is refused (1000 + 3).
    CHECK(mods::g_frame_callbacks);
    mods::frame();
    CHECK((g_records == std::vector<int>{1, 1003, 2, 1003}));
    // Pad callbacks, in load order, on the report.
    CHECK(g_pad_hook != nullptr);
    unsigned char report[18] = {10};
    g_pad_hook(0, report, 18);
    CHECK(report[0] == 13);
    mods::reset();
    CHECK(g_slot_add == nullptr && add(0) == 1 && g_pad_hook == nullptr && !mods::g_frame_callbacks);
}

void a_mod_that_fails_is_taken_back(const fs::path& plugins) {
    for (const char* behaviour : {"fail", "unknown_hook", "bad_signature"}) {
        write(plugins / "plugins.json", std::string(R"({"format": 1, "mods": [
            {"id": "test-a", "settings": {"tag": 1}},
            {"id": "test-b", "settings": {"tag": 2, "behaviour": ")") + behaviour + R"("}}]})");
        g_records.clear();
        const std::string line = mods::load(host(plugins));
        CHECK(line == "test-a 1.0.0 (1 not loaded: plugins/status.json)");
        CHECK(add(0) == 11);  // a's only: (0 + 1) * 10 + 1
        const auto [state, reason] = status_of(plugins, "test-b");
        CHECK(state == "failed");
        if (std::string(behaviour) == "fail") {
            CHECK(reason == "this mod refuses on purpose");
        } else if (std::string(behaviour) == "unknown_hook") {
            CHECK(reason.find("no hook point Test_Nothing") != std::string::npos);
        } else {
            CHECK(reason.find("is i32(i32) here, not void(i32)") != std::string::npos);
        }
        mods::frame();
        CHECK((g_records == std::vector<int>{1, 1003}));  // a's callback only
        mods::reset();
        CHECK(g_slot_add == nullptr);
    }
}

void refused_before_opening(const fs::path& plugins) {
    install(plugins, "b", OPENRAC_TEST_MOD_B, "2.0");
    write(plugins / "plugins.json", R"({"format": 1, "mods": [
        {"id": "test-a", "enabled": false}, {"id": "test-b"}, {"id": "test-c"}]})");
    CHECK(mods::load(host(plugins)) == "none (2 not loaded: plugins/status.json)");
    CHECK(g_slot_add == nullptr);
    CHECK(status_of(plugins, "test-a").first == "absent");  // off: not written
    CHECK(status_of(plugins, "test-b").second.find("needs interface 2.0") != std::string::npos);
    CHECK(status_of(plugins, "test-c").second.find("not installed") != std::string::npos);
    mods::reset();
    install(plugins, "b", OPENRAC_TEST_MOD_B);
}

void purist_and_no_list(const fs::path& plugins) {
    mods::Host h = host(plugins);
    h.purist = true;
    CHECK(mods::load(h) == "none (purist mode)");
    CHECK(g_slot_add == nullptr);
    mods::reset();
    h = host(plugins / "nowhere");
    CHECK(mods::load(h) == "none");
    mods::reset();
}

// The library's id must be its manifest's.
void a_library_of_another_id(const fs::path& plugins) {
    fs::copy_file(OPENRAC_TEST_MOD_A, plugins / "test-b" / "lib" / library_name(), fs::copy_options::overwrite_existing);
    write(plugins / "plugins.json", R"({"format": 1, "mods": [{"id": "test-b"}]})");
    CHECK(mods::load(host(plugins)) == "none (1 not loaded: plugins/status.json)");
    CHECK(status_of(plugins, "test-b").second.find("its library is mod \"test-a\"") != std::string::npos);
    mods::reset();
    install(plugins, "b", OPENRAC_TEST_MOD_B);
}

// A player's install, with no checkout anywhere: the launcher's per-user install folder
// (~/OpenRAC by default), the game's set-up folder in it, a mod as the launcher installs one from its
// .zip (the SDK's layout, lib/<platform>/<file>), the list named on the command line as the launcher
// names it, and the program started in an unrelated folder. Nothing is looked for beside the program
// or in a source tree: an installed launcher's own folder (an AppImage, an .app) is read-only.
void an_installed_launchers_layout(const fs::path& root) {
    const fs::path data = root / "home" / "OpenRAC" / "active" / "test" / "data";
    const fs::path plugins = data / "plugins";
    const fs::path mod = plugins / "test-a";
    const fs::path lib = fs::path("lib") / mods::platform_key() / library_name();
    fs::create_directories(mod / lib.parent_path());
    fs::copy_file(OPENRAC_TEST_MOD_A, mod / lib, fs::copy_options::overwrite_existing);
    write(mod / "openrac-mod.json", R"({"format": 2, "id": "test-a", "name": "Test", "version": "1.0.0",
        "api": "1.3", "games": {"test": ["one"]}, "hooks": ["Test_Add"],
        "plugin": {")" + std::string(mods::platform_key()) + R"(": ")" + lib.generic_string() + R"("},
        "settings": [{"key": "tag", "name": "Tag", "type": "int", "default": 0}]})");
    write(plugins / "plugins.json", R"({"format": 1, "mods": [{"id": "test-a", "enabled": true, "settings": {"tag": 4}}]})");
    const fs::path elsewhere = root / "somewhere-else";
    fs::create_directories(elsewhere);
    const fs::path before = fs::current_path();
    fs::current_path(elsewhere);
    const std::string line = mods::load(host(plugins));
    fs::current_path(before);
    CHECK(line == "test-a 1.0.0");
    CHECK(add(0) == 14);
    CHECK(status_of(plugins, "test-a").first == "loaded");
    // Only status.json is written, beside the list; nothing in the folder the program ran in.
    CHECK(fs::is_empty(elsewhere));
    mods::reset();
}

// The keyboard and mouse (interface 1.2): the mod takes the keyboard during load, reads the
// state the host gives for the tick, and asks for the cursor (let go while the overlay is open).
openrac_input g_input{};
std::uint32_t g_taken = 0;
int g_captures = 0;
bool g_captured = false;

void input_reaches_the_mod(const fs::path& plugins) {
    write(plugins / "plugins.json", R"({"format": 1, "mods": [
        {"id": "test-a", "enabled": true, "settings": {"behaviour": "input", "jump": "KeyQ"}}]})");
    mods::Host h = host(plugins);
    g_input = openrac_input{};
    g_input.size = sizeof g_input;
    g_taken = 0;
    g_captures = 0;
    h.input = [] { return static_cast<const openrac_input*>(&g_input); };
    h.take_input = [](std::uint32_t devices) { g_taken |= devices; };
    h.capture_mouse = [](bool capture) {
        ++g_captures;
        g_captured = capture;
    };
    CHECK(mods::load(h) == "test-a 1.0.0");
    CHECK(g_taken == OPENRAC_INPUT_KEYBOARD);
    CHECK(g_captures == 1 && g_captured);
    g_input.keys[26] = OPENRAC_KEY_HELD;
    g_input.mouse_dx = 7.0f;
    g_input.overlay = 1;
    unsigned char report[18] = {0};
    CHECK(g_pad_hook != nullptr);
    g_pad_hook(0, report, 18);
    CHECK(report[1] == 0x57 && report[2] == 7);
    CHECK(g_captures == 2 && !g_captured);  // the overlay is open: it lets the cursor go
    mods::reset();

    // Without a host's input (a test, no window) the state is all released, never null.
    write(plugins / "plugins.json", R"({"format": 1, "mods": [
        {"id": "test-a", "enabled": true, "settings": {"behaviour": "input"}}]})");
    CHECK(mods::load(host(plugins)) == "test-a 1.0.0");
    unsigned char quiet[18] = {0, 9, 9};
    g_pad_hook(0, quiet, 18);
    CHECK(quiet[1] == 0 && quiet[2] == 0);
    mods::reset();
}

}  // namespace

// A hook point renamed after a mod was written (docs/port/MODS.md, section 4): the decompilation now
// calls the function Test_Add_Renamed, and the game's binding keeps Test_Add as an alias of its
// permanent id, test.add. The mod still asks for Test_Add, and binds; status.json names the id.
void an_old_name_still_binds(const fs::path& plugins) {
    static const mods::HookPoint renamed[] = {
        {"Test_Add_Renamed", "func_00100000", "i32(i32)", &g_slot_add, reinterpret_cast<void*>(add_game)},
    };
    static const openrac_script_alias aliases[] = {
        {"Test_Add", "test.add"},
        {"Test_Add_Renamed", "test.add"},
        {"func_00100000", "test.add"},
    };
    static const char* const discs[] = {"one"};
    static const openrac_script_binding binding = {"test", 1, discs, 0, nullptr, 0, nullptr, nullptr, 3, aliases};
    CHECK(std::string(openrac_script_hook_id(&binding, "Test_Add")) == "test.add");
    CHECK(std::string(openrac_script_hook_id(&binding, "func_00100000")) == "test.add");
    CHECK(std::string(openrac_script_hook_id(&binding, "test.add")) == "test.add");
    CHECK(std::string(openrac_script_hook_id(&binding, "Unknown")) == "Unknown");
    CHECK(std::string(openrac_script_hook_id(nullptr, "Test_Add")) == "Test_Add");
    write(plugins / "plugins.json", R"({"format": 1, "mods": [{"id": "test-a", "enabled": true, "settings": {"tag": 5}}]})");
    mods::Host h = host(plugins);
    h.hooks = renamed;
    h.scripts = &binding;
    CHECK(mods::load(h) == "test-a 1.0.0");
    CHECK(add(0) == 15);
    CHECK(status_of(plugins, "test-a").first == "loaded");
    const std::string status = read(plugins / "status.json");
    CHECK(status.find("\"hooks\": [\"test.add\"]") != std::string::npos);
    CHECK(status.find("\"hook_points\": [\"test.add\"]") != std::string::npos);
    mods::reset();
}

int main() {
    const fs::path dir = fs::temp_directory_path() / ("openrac_mod_loader_test_" + std::to_string(std::rand()));
    fs::remove_all(dir);
    const fs::path plugins = dir / "plugins";
    install(plugins, "a", OPENRAC_TEST_MOD_A);
    install(plugins, "b", OPENRAC_TEST_MOD_B);
    static const openrac_fn_entry functions[] = {
        {0x00100100u, reinterpret_cast<openrac_host_fn>(test_record), "Test_Record"},
    };
    openrac_guest_register(OPENRAC_OVERLAY_EXE, functions, 1);
    two_mods_chain_in_load_order(plugins);
    a_mod_that_fails_is_taken_back(plugins);
    refused_before_opening(plugins);
    purist_and_no_list(plugins);
    a_library_of_another_id(plugins);
    an_installed_launchers_layout(dir);
    an_old_name_still_binds(plugins);
    input_reaches_the_mod(plugins);
    std::error_code e;
    fs::remove_all(dir, e);
    return test::result();
}

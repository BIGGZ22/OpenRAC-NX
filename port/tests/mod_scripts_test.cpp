// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Script mods (game/common/mod_scripts.h, docs/port/MODS.md section 11) through the real loader,
// with a game of the test's own: two hook points written as hostgen's hooks.c writes them, its
// binding generated from tests/script_binding_test.json (script_binding_test.c), and the native test
// mod of mod_loader_test (mod_loader_test_mod.c) beside the scripts. The sandbox, the budget, an
// error attributed to its mod with the original still run, one chain shared with a native mod in
// both orders, before and after, the pad's report, the fields, require(), the installed launcher's
// layout, and no Luau state while no script mod is on.

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "check.h"
#include "mod_manifest.h"
#include "mod_scripts.h"
#include "mods.h"
#include "openrac/guest.h"
#include "openrac/memory.h"
#include "openrac/script_binding.h"
#include "viewer/json.h"

namespace fs = std::filesystem;
using namespace openrac;

namespace {

// The hook points, as hooks.c writes them.
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
void* g_slot_scale = nullptr;
float scale_game(float x, unsigned by) {
    return x * static_cast<float>(by);
}
float scale(float x, unsigned by) {
    if (g_slot_scale != nullptr) {
        return reinterpret_cast<float (*)(float, unsigned)>(g_slot_scale)(x, by);
    }
    return scale_game(x, by);
}
const mods::HookPoint kHooks[] = {
    {"Test_Add", "func_00100000", "i32(i32)", &g_slot_add, reinterpret_cast<void*>(add_game)},
    {"Test_Scale", "func_00100100", "f32(f32,u32)", &g_slot_scale, reinterpret_cast<void*>(scale_game)},
};

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

// A script mod: its manifest and its files (name -> text).
void install_script(const fs::path& plugins, const std::string& id,
                    const std::vector<std::pair<std::string, std::string>>& files,
                    const std::string& settings = "") {
    const fs::path dir = plugins / id;
    fs::remove_all(dir);
    std::string scripts;
    for (const auto& [name, text] : files) {
        write(dir / name, text);
        if (name.find('/') == std::string::npos) {
            scripts += (scripts.empty() ? "\"" : ", \"") + name + "\"";
        }
    }
    write(dir / "openrac-mod.json", R"({"format": 2, "id": ")" + id + R"(", "name": "Script", "version": "1.0.0",
        "type": "script", "api": "1.1", "scripts": [)" + scripts + R"(], "games": {"test": ["one"]},
        "hooks": ["Test_Add", "Test_Scale"], "settings": [)" + settings + "]}");
}

// The native test mod (mod_loader_test_mod.c), as test-a.
void install_native(const fs::path& plugins) {
    const fs::path dir = plugins / "test-a";
    fs::create_directories(dir / "lib");
    fs::copy_file(OPENRAC_TEST_MOD_A, dir / "lib" / library_name(), fs::copy_options::overwrite_existing);
    write(dir / "openrac-mod.json", R"({"format": 2, "id": "test-a", "name": "Test", "version": "1.0.0",
        "api": "1.2", "games": {"test": ["one"]}, "hooks": ["Test_Add"],
        "plugin": {")" + std::string(mods::platform_key()) + R"(": "lib/)" + library_name() + R"("},
        "settings": [{"key": "tag", "name": "Tag", "type": "int", "default": 0},
                     {"key": "behaviour", "name": "Behaviour", "type": "choice", "default": "ok",
                      "choices": [{"value": "ok", "name": "OK"}, {"value": "fail", "name": "Fail"}]}]})");
}

void list(const fs::path& plugins, const std::string& mods_json) {
    write(plugins / "plugins.json", R"({"format": 1, "mods": [)" + mods_json + "]}");
}

mods::Host host(const fs::path& plugins) {
    mods::Host h;
    h.game = "test";
    h.version = "one";
    h.openrac_version = "0.0.0";
    h.hooks = kHooks;
    h.hook_count = 2;
    h.list = plugins / "plugins.json";
    h.pad_hook = &g_pad_hook;
    h.now = "2026-10-21T00:00:00+00:00";
    h.scripts = openrac_game_script_binding();
    return h;
}

std::pair<std::string, std::string> status_of(const fs::path& plugins, const std::string& id) {
    viewer::json::Value v;
    std::string error;
    if (!viewer::json::parse(read(plugins / "status.json"), v, error)) {
        return {"unreadable: " + error, ""};
    }
    for (const auto& m : v["mods"].items()) {
        if (m["id"].string() == id) {
            return {m["state"].string(), m["reason"].is_string() ? m["reason"].string() : ""};
        }
    }
    return {"absent", ""};
}

std::int32_t counter() {
    std::int32_t v;
    std::memcpy(&v, G(0x00100000u), 4);
    return v;
}

void no_script_mod_no_state(const fs::path& plugins) {
    install_native(plugins);
    list(plugins, R"({"id": "test-a", "enabled": true, "settings": {"tag": 1}})");
    CHECK(mods::load(host(plugins)) == "test-a 1.0.0");
    CHECK(mods::scripts::state_count() == 0);
    CHECK(add(0) == 11);
    mods::reset();
}

void the_sandbox(const fs::path& plugins) {
    install_script(plugins, "sandbox", {{"main.luau", R"(
assert(io == nil, "io")
assert(os.execute == nil and os.getenv == nil and os.remove == nil, "os")
assert(type(os.clock()) == "number", "os.clock")
assert(loadstring == nil and getfenv == nil and setfenv == nil and load == nil and dofile == nil, "loaders")
assert(debug.info == nil and debug.traceback ~= nil, "debug")
assert(not pcall(require, "../outside"), "require outside the folder")
assert(not pcall(require, "/etc/passwd"), "require of an absolute path")
assert(not pcall(function() openrac.extra = 1 end), "openrac is read-only")
assert(not pcall(function() string.rep = nil end), "the libraries are read-only")
local util = require("lib/util")
assert(util.twice(4) == 8, "require of the mod's own file")
assert(require("lib/util") == util, "a module runs once")
openrac.log("sandbox fine on", openrac.game, openrac.version, openrac.api)
)"},
                                        {"lib/util.luau", "return { twice = function(x) return x * 2 end }"}});
    list(plugins, R"({"id": "sandbox", "enabled": true})");
    CHECK(mods::load(host(plugins)) == "sandbox 1.0.0");
    CHECK(status_of(plugins, "sandbox").first == "loaded");
    if (status_of(plugins, "sandbox").first != "loaded") {
        std::fprintf(stderr, "sandbox: %s\n", status_of(plugins, "sandbox").second.c_str());
    }
    CHECK(mods::scripts::state_count() == 1);
    mods::reset();
    CHECK(mods::scripts::state_count() == 0);
}

void the_budget(const fs::path& plugins) {
    mods::scripts::set_budget_ms(20.0);
    // At load: the mod does not load.
    install_script(plugins, "spin", {{"main.luau", "while true do end"}});
    list(plugins, R"({"id": "spin", "enabled": true})");
    mods::load(host(plugins));
    auto [state, reason] = status_of(plugins, "spin");
    CHECK(state == "failed");
    CHECK(reason.find("time budget") != std::string::npos);
    CHECK(g_slot_add == nullptr);
    CHECK(mods::scripts::state_count() == 0);
    mods::reset();
    // In a hook: it stops there, and the hook passes to the game from then on.
    install_script(plugins, "spin", {{"main.luau", R"(
openrac.hook("Test_Add", function(original, x)
    if x == 7 then while true do end end
    return original(x) + 100
end)
)"}});
    mods::load(host(plugins));
    CHECK(status_of(plugins, "spin").first == "loaded");
    CHECK(add(1) == 102);
    CHECK(add(7) == 8);  // stopped before its original ran: the game's runs
    const auto stopped = status_of(plugins, "spin");
    CHECK(stopped.first == "failed");
    CHECK(stopped.second.find("time budget") != std::string::npos);
    CHECK(add(1) == 2);  // falls through now
    mods::reset();
    mods::scripts::set_budget_ms(2.0);
}

void an_error_is_the_mods(const fs::path& plugins) {
    install_script(plugins, "broken", {{"main.luau", R"(
openrac.hook("Test_Add", function(original, x)
    local r = original(x)
    if x == 3 then error("boom at " .. x) end
    return r * 2
end)
)"}});
    list(plugins, R"({"id": "broken", "enabled": true})");
    mods::load(host(plugins));
    CHECK(add(1) == 4);
    CHECK(add(3) == 4);  // its original ran before the error: that result stands
    const auto s = status_of(plugins, "broken");
    CHECK(s.first == "failed");
    CHECK(s.second.find("boom at 3") != std::string::npos);
    CHECK(s.second.find("hook test.add") != std::string::npos);
    CHECK(add(5) == 6);
    mods::reset();
}

void one_chain_with_native_mods(const fs::path& plugins) {
    install_native(plugins);
    install_script(plugins, "double", {{"main.luau", R"(
openrac.hook("Test_Add", function(original, x) return original(x) * 2 end)
)"}});
    // The script loaded last runs first: double(a(game)) = ((0 + 1) * 10 + 1) * 2.
    list(plugins, R"({"id": "test-a", "enabled": true, "settings": {"tag": 1}}, {"id": "double", "enabled": true})");
    CHECK(mods::load(host(plugins)) == "test-a 1.0.0, double 1.0.0");
    CHECK(add(0) == 22);
    mods::reset();
    // The other way round: a(double(game)) = ((0 + 1) * 2) * 10 + 1.
    list(plugins, R"({"id": "double", "enabled": true}, {"id": "test-a", "enabled": true, "settings": {"tag": 1}})");
    CHECK(mods::load(host(plugins)) == "double 1.0.0, test-a 1.0.0");
    CHECK(add(0) == 21);
    mods::reset();
    // Two scripts on one point, and floats: twice(half(game)).
    install_script(plugins, "half", {{"main.luau", R"(
openrac.hook("Test_Scale", function(original, x, by) return original(x, by) / 2 end)
)"}});
    install_script(plugins, "twice", {{"main.luau", R"(
openrac.hook("Test_Scale", function(original, x, by) return original(x + 1, by) * 4 end)
)"}});
    list(plugins, R"({"id": "half", "enabled": true}, {"id": "twice", "enabled": true})");
    mods::load(host(plugins));
    CHECK(scale(1.5f, 2) == 10.0f);  // twice: half(2.5, 2) * 4 = (5 / 2) * 4
    mods::reset();
    CHECK(g_slot_scale == nullptr && scale(1.5f, 2) == 3.0f);
}

void before_after_pad_fields(const fs::path& plugins) {
    std::memset(G(0x00100000u), 0, 8);
    const float value = 2.5f;
    std::memcpy(G(0x00100004u), &value, 4);
    install_script(plugins, "watch", {{"main.luau", R"(
openrac.before("Test_Add", function(x) openrac.fields.counter += x end)
openrac.after("Test_Add", function(x) openrac.write("i32", 0x00100008, x * 3) end)
openrac.on_pad(function(port, report)
    if port == 0 then buffer.writeu8(report, 0, buffer.readu8(report, 0) + openrac.setting("step")) end
end)
openrac.on_frame(function()
    assert(openrac.fields.nested.value == 2.5)
    assert(not pcall(function() openrac.fields.nested.value = 1 end), "read-only")
    assert(not pcall(openrac.read, "u32", 0x10000000), "outside RAM")
    openrac.fields.counter += 1000
end)
)"}}, R"({"key": "step", "name": "Step", "type": "int", "default": 5})");
    list(plugins, R"({"id": "watch", "enabled": true})");
    mods::load(host(plugins));
    CHECK(add(4) == 5);
    CHECK(counter() == 4);
    std::int32_t after;
    std::memcpy(&after, G(0x00100008u), 4);
    CHECK(after == 12);
    unsigned char report[18] = {10};
    CHECK(g_pad_hook != nullptr);
    g_pad_hook(0, report, 18);
    CHECK(report[0] == 15);
    mods::frame();
    CHECK(counter() == 1004);
    CHECK(status_of(plugins, "watch").first == "loaded");
    mods::reset();
}

// A player's install, no checkout: ~/OpenRAC/active/<game>/data/plugins under a temporary folder,
// the list named as the launcher names it, the program started elsewhere.
// The keyboard and mouse (interface 1.2) from a script: binds by name, the mouse, the window, the
// keyboard taken while the scripts first run, the cursor asked for.
openrac_input g_input{};
std::uint32_t g_taken = 0;
bool g_captured = false;

void keyboard_and_mouse(const fs::path& plugins) {
    install_script(plugins, "kbm", {{"main.luau", R"(
openrac.take_input("keyboard")
assert(not pcall(openrac.key, "NoSuchKey"), "an unknown name is an error")
openrac.on_pad(function(port, report)
    local dx, dy, wx, wy = openrac.mouse()
    local focused, captured, overlay = openrac.window()
    buffer.writeu8(report, 1, if openrac.key("KeyW") then 1 else 0)
    buffer.writeu8(report, 2, if openrac.pressed("MouseLeft") then 1 else 0)
    buffer.writeu8(report, 3, dx)
    buffer.writeu8(report, 4, if openrac.pressed("WheelDown") then 1 else 0)
    buffer.writeu8(report, 5, if focused and not overlay then 1 else 0)
    openrac.capture_mouse(focused)
end)
)"}});
    list(plugins, R"({"id": "kbm", "enabled": true})");
    mods::Host h = host(plugins);
    g_input = openrac_input{};
    g_input.size = sizeof g_input;
    h.input = [] { return static_cast<const openrac_input*>(&g_input); };
    h.take_input = [](std::uint32_t devices) { g_taken |= devices; };
    h.capture_mouse = [](bool capture) { g_captured = capture; };
    CHECK(mods::load(h) == "kbm 1.0.0");
    CHECK(g_taken == OPENRAC_INPUT_KEYBOARD);
    g_input.keys[26] = OPENRAC_KEY_HELD;
    g_input.mouse_pressed = OPENRAC_MOUSE_LEFT;
    g_input.mouse_dx = 9.0f;
    g_input.wheel_y = -1.0f;
    g_input.focused = 1;
    unsigned char report[18] = {0};
    g_pad_hook(0, report, 18);
    CHECK(report[1] == 1 && report[2] == 1 && report[3] == 9 && report[4] == 1 && report[5] == 1);
    CHECK(g_captured);
    mods::reset();
}

void an_installed_launchers_layout(const fs::path& root) {
    const fs::path plugins = root / "home" / "OpenRAC" / "active" / "test" / "data" / "plugins";
    install_script(plugins, "plus", {{"main.luau", R"(
openrac.hook("Test_Add", function(original, x) return original(x) + 40 end)
)"}});
    list(plugins, R"({"id": "plus", "enabled": true})");
    const fs::path elsewhere = root / "somewhere-else";
    fs::create_directories(elsewhere);
    const fs::path before = fs::current_path();
    fs::current_path(elsewhere);
    CHECK(mods::load(host(plugins)) == "plus 1.0.0");
    fs::current_path(before);
    CHECK(add(1) == 42);
    CHECK(fs::is_empty(elsewhere));
    mods::reset();
}

// A hook point by its permanent id, and by a name it had before a rename (the test table's Old_Add,
// an alias of test.add): both bind to the same chain (docs/port/MODS.md, section 4).
void ids_and_old_names(const fs::path& plugins) {
    install_script(plugins, "renamed", {{"main.luau", R"(
openrac.hook("Old_Add", function(original, x) return original(x) + 100 end)
openrac.hook("test.add", function(original, x) return original(x) * 3 end)
)"}});
    list(plugins, R"({"id": "renamed", "enabled": true})");
    CHECK(mods::load(host(plugins)) == "renamed 1.0.0");
    // The later hook runs first: (x * 3 of what the earlier gives) = ((1 + 1) + 100) * 3.
    CHECK(add(1) == 306);
    CHECK(read(plugins / "status.json").find("\"test.add\"") != std::string::npos);
    mods::reset();
}

void refused_without_a_binding(const fs::path& plugins) {
    install_script(plugins, "plus", {{"main.luau", "openrac.log('hi')"}});
    list(plugins, R"({"id": "plus", "enabled": true})");
    mods::Host h = host(plugins);
    h.scripts = nullptr;
    mods::load(h);
    const auto s = status_of(plugins, "plus");
    CHECK(s.first == "refused");
    CHECK(s.second.find("runs no scripts") != std::string::npos);
    CHECK(mods::scripts::state_count() == 0);
    mods::reset();
}

}  // namespace

int main() {
    runtime::Memory::create();
    const fs::path dir = fs::temp_directory_path() / ("openrac_mod_scripts_test_" + std::to_string(std::rand()));
    fs::remove_all(dir);
    const fs::path plugins = dir / "plugins";
    no_script_mod_no_state(plugins);
    the_sandbox(plugins);
    the_budget(plugins);
    an_error_is_the_mods(plugins);
    one_chain_with_native_mods(plugins);
    before_after_pad_fields(plugins);
    an_installed_launchers_layout(dir);
    ids_and_old_names(plugins);
    keyboard_and_mouse(plugins);
    refused_without_a_binding(plugins);
    std::error_code e;
    fs::remove_all(dir, e);
    return test::result();
}

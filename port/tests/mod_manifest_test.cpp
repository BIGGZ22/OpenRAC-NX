// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Native mods' manifests, the launcher's list and the checks that decide what loads
// (game/common/mod_manifest.h; docs/port/MODS.md, sections 3 and 5). Sources: RFC 8259 for the JSON
// the shared cases are written back to.
//
// The shared cases (sdk/tests/manifests/cases.json) pin today's verdicts of parse_manifest and
// valid_id, which the launcher's and the SDK's checks are held to as well: where the three disagree,
// the file records each one's verdict and why. They are characterisation tests, written before the
// restructure, so a refactor of any of the three cannot move a rule unseen.

#include <cstdio>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "check.h"
#include "common/json.h"
#include "mod_manifest.h"

using namespace openrac;
using namespace openrac::mods;

namespace {

// A manifest for rac1's two discs with a library for every platform; `extra` adds fields.
std::string manifest(const std::string& id, const std::string& version = "1.0.0", const std::string& extra = "",
                     const std::string& api = "1.0") {
    return R"({"format": 2, "id": ")" + id + R"(", "name": "A mod", "version": ")" + version + R"(",
        "api": ")" + api + R"(", "games": {"rac1": ["pal", "ntsc"]},
        "plugin": {"macos-arm64": "lib/a.dylib", "macos-x86_64": "lib/a.dylib", "linux-x86_64": "lib/a.so",
                   "linux-aarch64": "lib/a.so", "windows-x86_64": "lib/a.dll", "windows-arm64": "lib/a.dll"})"
           + extra + "}";
}

Candidate candidate(const std::string& text, const std::string& id = "") {
    Candidate c;
    std::string error;
    c.has_manifest = parse_manifest(text, c.manifest, error);
    c.manifest_error = error;
    c.entry.id = id.empty() ? c.manifest.id : id;
    return c;
}

Target rac1_pal() {
    return {"rac1", "pal", platform_key(), 1, 0};
}

std::vector<Decision> plan_all(const std::vector<Candidate>& list, const Target& target = rac1_pal()) {
    return plan(list, target, [](const std::string&, const std::string&) { return true; });
}

void versions() {
    CHECK(parse_version("1.2.3").has_value());
    CHECK(!parse_version("1.2").has_value());
    CHECK(!parse_version("1.02.3").has_value());
    CHECK(!parse_version("1.2.3-").has_value());
    CHECK(compare(*parse_version("1.2.3"), *parse_version("1.10.0")) < 0);
    CHECK(compare(*parse_version("2.0.0"), *parse_version("1.99.99")) > 0);
    CHECK(compare(*parse_version("1.0.0-beta"), *parse_version("1.0.0")) < 0);
    CHECK(compare(*parse_version("1.0.0"), *parse_version("1.0.0")) == 0);
    CHECK(valid_id("lock-strafe"));
    CHECK(!valid_id("Lock-Strafe"));
    CHECK(!valid_id("lock--strafe"));
    CHECK(!valid_id("-lock"));
    CHECK(!valid_id("../x"));
}

void manifests() {
    Manifest m;
    std::string error;
    CHECK(parse_manifest(manifest("lock-strafe", "1.2.0",
                                  R"(, "hooks": ["Pad_ProcessReport"], "author": "us",
          "settings": [{"key": "mode", "name": "Mode", "type": "choice", "default": "lock",
                        "choices": [{"value": "lock", "name": "Lock"}, {"value": "hold", "name": "Hold"}]},
                       {"key": "speed", "name": "Speed", "type": "float", "default": 1.0, "min": 0.5, "max": 2},
                       {"key": "count", "name": "Count", "type": "int", "default": 3, "min": 0, "max": 9},
                       {"key": "trace", "name": "Trace", "type": "bool", "default": false},
                       {"key": "label", "name": "Label", "type": "string", "default": "hi", "max_length": 4}],
          "requires": [{"id": "base-lib", "version": ">=1.1.0"}],
          "conflicts": [{"id": "other-strafe", "reason": "both take L2"}],
          "load": {"after": ["camera"], "before": []})"),
                         m, error));
    CHECK(error.empty());
    CHECK(m.id == "lock-strafe" && m.version.minor == 2 && m.api_major == 1 && m.api_minor == 0);
    CHECK(m.games.at("rac1").size() == 2);
    CHECK(m.settings.size() == 5 && m.settings[0].choices.size() == 2 && m.settings[0].fallback.s == "lock");
    CHECK(m.requirements.size() == 1 && m.requirements[0].op == ">=");
    CHECK(m.conflicts.size() == 1 && m.conflicts[0].reason == "both take L2");
    CHECK(m.load_after.size() == 1);

    // Refused: a data pack, a newer format, a bad id, a plugin without "api", a bad default, a
    // path out of the mod, an unknown platform, an http homepage.
    CHECK(!parse_manifest(R"({"format": 1, "game": "rac1"})", m, error) && error.find("data pack") != std::string::npos);
    CHECK(!parse_manifest(R"({"format": 3})", m, error) && error.find("update OpenRAC") != std::string::npos);
    CHECK(!parse_manifest(manifest("Bad_Id"), m, error));
    CHECK(!parse_manifest(R"({"format": 2, "id": "a", "name": "A", "version": "1.0.0", "games": {"rac1": ["pal"]},
                               "plugin": {"linux-x86_64": "a.so"}})",
                          m, error));
    CHECK(!parse_manifest(manifest("a", "1.0.0",
                                   R"(, "settings": [{"key": "n", "name": "N", "type": "int", "default": 12, "max": 9}])"),
                          m, error));
    CHECK(!parse_manifest(manifest("a", "1.0.0",
                                   R"(, "settings": [{"key": "c", "name": "C", "type": "choice", "default": "x",
                                       "choices": [{"value": "y", "name": "Y"}]}])"),
                          m, error));
    CHECK(!parse_manifest(R"({"format": 2, "id": "a", "name": "A", "version": "1.0.0", "api": "1.0",
                               "games": {"rac1": ["pal"]}, "plugin": {"linux-x86_64": "../a.so"}})",
                          m, error));
    CHECK(!parse_manifest(R"({"format": 2, "id": "a", "name": "A", "version": "1.0.0", "api": "1.0",
                               "games": {"rac1": ["pal"]}, "plugin": {"amiga-68k": "a.so"}})",
                          m, error));
    CHECK(!parse_manifest(manifest("a", "1.0.0", R"(, "homepage": "http://example.org")"), m, error));
    CHECK(!parse_manifest(manifest("a", "1.0.0", "", "1"), m, error));
}

void settings() {
    Manifest m;
    std::string error;
    CHECK(parse_manifest(manifest("a", "1.0.0",
                                  R"(, "settings": [
          {"key": "mode", "name": "Mode", "type": "choice", "default": "lock",
           "choices": [{"value": "lock", "name": "Lock"}, {"value": "hold", "name": "Hold"}]},
          {"key": "count", "name": "Count", "type": "int", "default": 3, "min": 0, "max": 9},
          {"key": "speed", "name": "Speed", "type": "float", "default": 1.0, "min": 0.5, "max": 2},
          {"key": "trace", "name": "Trace", "type": "bool", "default": false}])"),
                         m, error));
    std::vector<ListEntry> list;
    CHECK(parse_list(R"({"format": 1, "mods": [{"id": "a", "enabled": true,
        "settings": {"mode": "hold", "count": 42, "speed": 1.5, "trace": "yes", "other": 1}}]})",
                     list, error));
    CHECK(list.size() == 1);
    std::vector<std::string> notes;
    const auto values = resolve_settings(m, list[0].settings, notes);
    CHECK(values.at("mode").s == "hold");     // as written
    CHECK(values.at("count").i == 3);         // out of range: the default
    CHECK(values.at("speed").f == 1.5);       // as written
    CHECK(values.at("trace").b == false);     // of the wrong type: the default
    CHECK(notes.size() == 3);                 // count, trace, and "other", which is not the mod's
    // Missing values are the defaults, without a note.
    notes.clear();
    const auto defaults = resolve_settings(m, {}, notes);
    CHECK(defaults.at("mode").s == "lock" && defaults.at("count").i == 3 && notes.empty());
}

// Key bindings (interface 1.2): names of openrac/keys.h, at most `binds` of them, none twice.
void keybinds() {
    std::string out;
    CHECK(parse_binds("KeyW", 2, out) && out == "KeyW");
    CHECK(parse_binds("KeyW, ArrowUp", 2, out) && out == "KeyW,ArrowUp");
    CHECK(parse_binds("", 2, out) && out.empty());
    CHECK(parse_binds("MouseRight,WheelUp", 2, out) && out == "MouseRight,WheelUp");
    CHECK(!parse_binds("KeyW,KeyW", 2, out));        // twice
    CHECK(!parse_binds("KeyW,KeyA,KeyS", 2, out));   // three
    CHECK(!parse_binds("KeyW,KeyA", 1, out));        // over its "binds"
    CHECK(!parse_binds("W", 2, out));                // not a name of the table
    Manifest m;
    std::string error;
    CHECK(parse_manifest(manifest("a", "1.0.0", R"(, "settings": [
          {"key": "jump", "name": "Jump", "type": "keybind", "default": "Space"},
          {"key": "fire", "name": "Fire", "type": "keybind", "default": "MouseLeft,KeyF", "binds": 2},
          {"key": "pause", "name": "Pause", "type": "keybind", "default": "Escape", "binds": 1}])"),
                         m, error));
    CHECK(m.settings.size() == 3 && m.settings[0].type == SettingType::Keybind && m.settings[2].binds == 1);
    CHECK(!parse_manifest(manifest("a", "1.0.0", R"(, "settings": [
          {"key": "jump", "name": "Jump", "type": "keybind", "default": "Spacebar"}])"), m, error));
    CHECK(!parse_manifest(manifest("a", "1.0.0", R"(, "settings": [
          {"key": "jump", "name": "Jump", "type": "keybind", "default": "Space", "binds": 3}])"), m, error));
    CHECK(!parse_manifest(manifest("a", "1.0.0", R"(, "settings": [
          {"key": "n", "name": "N", "type": "int", "default": 1, "binds": 1}])"), m, error));
    CHECK(parse_manifest(manifest("a", "1.0.0", R"(, "settings": [
          {"key": "jump", "name": "Jump", "type": "keybind", "default": "Space"},
          {"key": "pause", "name": "Pause", "type": "keybind", "default": "Escape", "binds": 1}])"), m, error));
    std::vector<ListEntry> list;
    CHECK(parse_list(R"({"format": 1, "mods": [{"id": "a", "enabled": true,
        "settings": {"jump": "KeyQ, Digit1", "pause": "KeyP,KeyO"}}]})", list, error));
    std::vector<std::string> notes;
    const auto values = resolve_settings(m, list[0].settings, notes);
    CHECK(values.at("jump").s == "KeyQ,Digit1");  // as written, normalized
    CHECK(values.at("pause").s == "Escape");      // two where one is taken: the default
    CHECK(notes.size() == 1);
}

void presets() {
    // A preset fills settings of any type; each value must be one its setting takes.
    const std::string settings = R"(, "settings": [
          {"key": "damage", "name": "Damage", "type": "float", "default": 1.0, "min": 0.25, "max": 4.0},
          {"key": "count", "name": "Count", "type": "int", "default": 3, "min": 1, "max": 30},
          {"key": "mode", "name": "Mode", "type": "choice", "default": "a",
           "choices": [{"value": "a", "name": "A"}, {"value": "b", "name": "B"}]},
          {"key": "chaos", "name": "Chaos", "type": "bool", "default": false},
          {"key": "jump", "name": "Jump", "type": "keybind", "default": "Space"}])";
    const auto with = [&](const std::string& presets) {
        Manifest m;
        std::string error;
        return parse_manifest(manifest("a", "1.0.0", settings + R"(, "presets": )" + presets, "1.2"), m, error);
    };
    CHECK(with(R"([{"id": "hard", "name": "Hard",
                    "values": {"damage": 2.5, "count": 30, "mode": "b", "chaos": true, "jump": "KeyJ"}},
                   {"id": "default", "name": "Retail", "values": {}}])"));
    CHECK(with("[]"));
    CHECK(!with(R"([{"id": "x", "name": "X", "values": {"damage": 5.0}}])"));     // out of range
    CHECK(!with(R"([{"id": "x", "name": "X", "values": {"count": 1.5}}])"));      // not an integer
    CHECK(!with(R"([{"id": "x", "name": "X", "values": {"mode": "c"}}])"));       // not a choice
    CHECK(!with(R"([{"id": "x", "name": "X", "values": {"chaos": 1}}])"));        // not a bool
    CHECK(!with(R"([{"id": "x", "name": "X", "values": {"jump": "Nope"}}])"));    // not a key
    CHECK(!with(R"([{"id": "x", "name": "X", "values": {"nothing": 1}}])"));      // no such setting
    CHECK(!with(R"([{"id": "X!", "name": "X", "values": {}}])"));
    CHECK(!with(R"([{"id": "x", "name": "X", "values": {}}, {"id": "x", "name": "Y", "values": {}}])"));
    CHECK(!with(R"({"x": 1})"));
}

void lists() {
    std::vector<ListEntry> list;
    std::string error;
    CHECK(parse_list(R"({"format": 1, "mods": [{"id": "a"}, {"id": "b", "enabled": false}, {"id": "a"}]})", list,
                     error));
    CHECK(list.size() == 2 && list[0].enabled && !list[1].enabled);  // the first "a" counts
    CHECK(!parse_list(R"({"format": 2, "mods": []})", list, error));
    CHECK(!parse_list(R"({"format": 1, "mods": [{"id": "../a"}]})", list, error));
    CHECK(!parse_list("not json", list, error));
}

void checks() {
    // Game, disc, interface version, platform.
    {
        auto d = plan_all({candidate(manifest("a"))});
        CHECK(d.size() == 1 && d[0].load);
        Target ntsc = rac1_pal();
        ntsc.version = "ntsc";
        CHECK(plan_all({candidate(manifest("a"))}, ntsc)[0].load);
        Target other = rac1_pal();
        other.version = "jp";
        d = plan_all({candidate(manifest("a"))}, other);
        CHECK(!d[0].load && d[0].reason.find("not for this disc") != std::string::npos);
        Target rac2 = rac1_pal();
        rac2.game = "rac2";
        CHECK(!plan_all({candidate(manifest("a"))}, rac2)[0].load);
        d = plan_all({candidate(manifest("a", "1.0.0", "", "2.0"))});
        CHECK(!d[0].load && d[0].reason.find("needs interface 2.0") != std::string::npos);
        d = plan_all({candidate(manifest("a", "1.0.0", "", "1.3"))});
        CHECK(!d[0].load && d[0].reason.find("update OpenRAC") != std::string::npos);
        Target newer = rac1_pal();
        newer.api_minor = 4;
        CHECK(plan_all({candidate(manifest("a", "1.0.0", "", "1.3"))}, newer)[0].load);
        Target amiga = rac1_pal();
        amiga.platform = "amiga-68k";
        d = plan_all({candidate(manifest("a"))}, amiga);
        CHECK(!d[0].load && d[0].reason.find("no library for this system") != std::string::npos);
        d = plan({candidate(manifest("a"))}, rac1_pal(), [](const std::string&, const std::string&) { return false; });
        CHECK(!d[0].load && d[0].reason.find("missing") != std::string::npos);
        d = plan_all({candidate(manifest("a"), "b")});
        CHECK(!d[0].load && d[0].reason.find("folder") != std::string::npos);
        Candidate broken;
        broken.entry.id = "c";
        broken.manifest_error = "its openrac-mod.json: bad";
        d = plan_all({broken});
        CHECK(!d[0].load && d[0].reason == "its openrac-mod.json: bad");
    }
    // Requirements: on, earlier in the list, of a version that satisfies.
    {
        const auto base = candidate(manifest("base", "1.2.0"));
        const auto user = candidate(manifest("user", "1.0.0", R"(, "requires": [{"id": "base", "version": ">=1.1.0"}])"));
        auto d = plan_all({base, user});
        CHECK(d[0].load && d[1].load);
        d = plan_all({user, base});
        CHECK(!d[0].load && d[0].reason.find("before it") != std::string::npos && d[1].load);
        d = plan_all({user});
        CHECK(!d[0].load && d[0].reason.find("needs mod base") != std::string::npos);
        const auto old = candidate(manifest("base", "1.0.0"));
        d = plan_all({old, user});
        CHECK(!d[1].load && d[1].reason.find(">=1.1.0") != std::string::npos);
        const auto exact = candidate(manifest("user", "1.0.0", R"(, "requires": [{"id": "base", "version": "=1.2.0"}])"));
        CHECK(plan_all({base, exact})[1].load);
        // A requirement refused for its own reasons takes its dependants with it.
        auto refused = candidate(manifest("base", "1.2.0", "", "2.0"));
        d = plan_all({refused, user});
        CHECK(!d[0].load && !d[1].load);
    }
    // Conflicts, from either side; the later one is refused.
    {
        const auto a = candidate(manifest("a", "1.0.0", R"(, "conflicts": [{"id": "b", "reason": "both take L2"}])"));
        const auto b = candidate(manifest("b"));
        auto d = plan_all({a, b});
        CHECK(d[0].load && !d[1].load && d[1].reason == "conflicts with mod a: both take L2");
        d = plan_all({b, a});
        CHECK(d[0].load && !d[1].load && d[1].reason.find("conflicts with mod b") != std::string::npos);
        // A conflicting mod that is itself refused conflicts with nothing.
        auto a_refused = candidate(manifest("a", "1.0.0", R"(, "conflicts": [{"id": "b"}])", "9.0"));
        d = plan_all({a_refused, b});
        CHECK(!d[0].load && d[1].load);
    }
    // Load hints are warnings only.
    {
        const auto x = candidate(manifest("x", "1.0.0", R"(, "load": {"after": ["y"]})"));
        const auto y = candidate(manifest("y"));
        auto d = plan_all({x, y});
        CHECK(d[0].load && d[0].warnings.size() == 1 && d[1].load);
        d = plan_all({y, x});
        CHECK(d[1].load && d[1].warnings.empty());
    }
}

// Script mods (MODS.md, section 11): "type": "script", their "scripts", interface 1.1, no library.
void scripts() {
    auto script = [](const std::string& extra, const std::string& api = "1.1") {
        return R"({"format": 2, "id": "s", "name": "S", "version": "1.0.0", "type": "script", "api": ")" + api
               + R"(", "games": {"rac1": ["pal"]})" + extra + "}";
    };
    Manifest m;
    std::string error;
    CHECK(parse_manifest(script(R"(, "scripts": ["main.luau", "more/other.luau"])"), m, error));
    CHECK(m.script && m.scripts.size() == 2 && m.scripts[1] == "more/other.luau" && m.plugin.empty());
    CHECK(!parse_manifest(script(""), m, error));                                   // no scripts
    CHECK(!parse_manifest(script(R"(, "scripts": [])"), m, error));
    CHECK(error.find("scripts") != std::string::npos);
    CHECK(!parse_manifest(script(R"(, "scripts": ["main.lua"])"), m, error));      // not .luau
    CHECK(!parse_manifest(script(R"(, "scripts": ["../main.luau"])"), m, error));  // outside the mod
    CHECK(!parse_manifest(script(R"(, "scripts": ["/main.luau"])"), m, error));
    CHECK(!parse_manifest(script(R"(, "scripts": ["a\\main.luau"])"), m, error));
    CHECK(!parse_manifest(script(R"(, "scripts": ["main.luau"])", "1.0"), m, error));  // scripts need 1.1
    CHECK(error.find("1.1") != std::string::npos);
    CHECK(!parse_manifest(script(R"(, "scripts": ["main.luau"], "plugin": {"linux-x86_64": "lib/a.so"})"), m, error));
    CHECK(!parse_manifest(R"({"format": 2, "id": "s", "name": "S", "version": "1.0.0", "type": "other",
        "games": {"rac1": ["pal"]}})", m, error));
    CHECK(!parse_manifest(manifest("n", "1.0.0", R"(, "scripts": ["main.luau"])"), m, error));  // a native's
    CHECK(parse_manifest(manifest("n", "1.0.0", R"(, "type": "native")"), m, error) && !m.script);
    CHECK(script_path("main.luau") && script_path("lib/util.luau"));
    CHECK(!script_path("lib/./util.luau") && !script_path("lib//util.luau") && !script_path("c:/x.luau"));

    // The plan: a port with scripts loads it when its files are there; one without refuses it; a port
    // of interface 1.0 refuses it for its version.
    Target with = rac1_pal();
    with.api_minor = 1;
    with.scripts = true;
    const Candidate c = candidate(script(R"(, "scripts": ["main.luau"])"));
    CHECK(c.has_manifest);
    auto d = plan({c}, with, [](const std::string&, const std::string& path) { return path == "main.luau"; });
    CHECK(d[0].load);
    d = plan({c}, with, [](const std::string&, const std::string&) { return false; });
    CHECK(!d[0].load && d[0].reason.find("main.luau is missing") != std::string::npos);
    Target without = with;
    without.scripts = false;
    d = plan_all({c}, without);
    CHECK(!d[0].load && d[0].reason.find("runs no scripts") != std::string::npos);
    Target old = with;
    old.api_minor = 0;
    d = plan_all({c}, old);
    CHECK(!d[0].load && d[0].reason.find("update OpenRAC") != std::string::npos);
}

void platform() {
    const std::string p = platform_key();
    CHECK(p == "macos-arm64" || p == "macos-x86_64" || p == "linux-x86_64" || p == "linux-aarch64"
          || p == "windows-x86_64" || p == "windows-arm64");
}

// ---- The shared cases ----

// Returns the shared cases' file, found from this file's place in the repository (port/tests/), so
// the build needs no setting for it.
std::filesystem::path shared_cases_path() {
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "sdk"
           / "tests" / "manifests" / "cases.json";
}

// Returns `text` as a JSON string: the quote, the backslash and the control characters escaped, the
// other bytes as they are (UTF-8).
std::string json_string(std::string_view text) {
    // JSON's control characters are the bytes below a space, which a string may not hold (documented).
    constexpr unsigned char kFirstPrintable = 0x20;
    std::string out = "\"";
    for (const char c : text) {
        const auto byte = static_cast<unsigned char>(c);
        // The quote and the backslash take a backslash before them.
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (byte < kFirstPrintable) {
            // A control character is written as \u and its code in four hex digits (documented).
            out += std::format("\\u{:04x}", byte);
        } else {
            out += c;
        }
    }
    return out + "\"";
}

// Returns `v` written as JSON text again, for parse_manifest, which reads text. Members come out in
// key order and numbers in their shortest form; neither changes a verdict, since the port's reader
// keeps members by key and every number as a double.
std::string json_text(const json::Value& v) {
    // A container's items and members are joined by commas, with no space.
    std::string out;
    switch (v.type()) {
        case json::Value::Type::Null:  // the literal null
            return "null";
        case json::Value::Type::Bool:  // the literal true or false
            return v.boolean() ? "true" : "false";
        case json::Value::Type::Number:  // the double's shortest form
            return std::format("{}", v.number());
        case json::Value::Type::String:  // quoted and escaped
            return json_string(v.string());
        case json::Value::Type::Array:  // each item in turn
            for (const json::Value& item : v.items()) {
                out += (out.empty() ? "" : ",") + json_text(item);
            }
            return "[" + out + "]";
        case json::Value::Type::Object:  // each member in key order
            for (const auto& [key, member] : v.members()) {
                out += (out.empty() ? "" : ",") + json_string(key) + ":" + json_text(member);
            }
            return "{" + out + "}";
    }
    return "null";
}

// Returns the port's verdict a shared case expects: its own where the three checks disagree.
bool expected(const json::Value& c) {
    const json::Value& valid = c["valid"];
    // A verdict all three share is a bool; where they disagree, an object holds each one's.
    return valid.type() == json::Value::Type::Bool ? valid.boolean() : valid["port"].boolean();
}

// Returns the shared cases, or null when the file cannot be read (a failed check says so).
json::Value shared_cases() {
    json::Value cases;
    std::string error;
    const bool read = json::parse_file(shared_cases_path().string(), cases, error);
    // Say why the file was not read: a missing file and a broken one are fixed differently.
    if (!read) {
        std::fprintf(stderr, "%s: %s\n", shared_cases_path().string().c_str(), error.c_str());
    }
    CHECK(read);
    return cases;
}

// Every shared id gets the verdict the file gives the port.
void test_shared_ids_keep_their_verdicts() {
    const json::Value cases = shared_cases();
    // A list that is missing reads as empty and would check nothing.
    CHECK(!cases["ids"].items().empty());
    for (const json::Value& c : cases["ids"].items()) {
        const bool valid = valid_id(c["id"].string());
        // Name the id whose verdict moved; the check alone says only that one did.
        if (valid != expected(c)) {
            std::fprintf(
                stderr,
                "shared id \"%s\": expected %s\n",
                c["id"].string().c_str(),
                expected(c) ? "valid" : "invalid"
            );
        }
        CHECK(valid == expected(c));
    }
}

// Every shared manifest gets the verdict the file gives the port.
void test_shared_manifests_keep_their_verdicts() {
    const json::Value cases = shared_cases();
    // A list that is missing reads as empty and would check nothing.
    CHECK(!cases["manifests"].items().empty());
    for (const json::Value& c : cases["manifests"].items()) {
        Manifest m;
        std::string error;
        const bool valid = parse_manifest(json_text(c["manifest"]), m, error);
        // Name the case whose verdict moved, with the port's reason when it refused it.
        if (valid != expected(c)) {
            std::fprintf(
                stderr,
                "shared manifest \"%s\": expected %s (%s)\n",
                c["name"].string().c_str(),
                expected(c) ? "valid" : "invalid",
                valid ? "accepted" : error.c_str()
            );
        }
        CHECK(valid == expected(c));
    }
}

}  // namespace

int main() {
    versions();
    manifests();
    settings();
    keybinds();
    presets();
    lists();
    checks();
    scripts();
    platform();
    test_shared_ids_keep_their_verdicts();
    test_shared_manifests_keep_their_verdicts();
    return test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Native mods' files, read and checked (docs/port/MODS.md, sections 3 and 5): a mod's manifest
// (openrac-mod.json, format 2), the launcher's list (plugins.json) and the checks that decide which
// mods the port opens, in which order. Nothing here opens a library or touches the game: mods.cpp
// does that, and tests/mod_manifest_test.cpp holds these rules to the design. The launcher applies
// the same rules (launcher/core/src/plugins.rs).

#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "common/json.h"

namespace openrac::mods {

// MAJOR.MINOR.PATCH with an optional -suffix; a version with a suffix comes before the same without.
struct Version {
    int major = 0;
    int minor = 0;
    int patch = 0;
    std::string suffix;
};
std::optional<Version> parse_version(std::string_view text);
int compare(const Version& a, const Version& b);

// Keybind (interface 1.2): up to `binds` names of openrac/keys.h, "KeyW,ArrowUp", kept as a string.
enum class SettingType { Bool, Int, Float, Choice, String, Keybind };

// A setting's value, of its declaration's type.
struct SettingValue {
    SettingType type = SettingType::Bool;
    bool b = false;
    std::int64_t i = 0;
    double f = 0.0;
    std::string s;  // a choice's value or a string
};

struct Setting {
    std::string key;
    std::string name;
    SettingType type = SettingType::Bool;
    SettingValue fallback;  // the manifest's "default"
    std::optional<double> min, max;
    std::vector<std::string> choices;  // the values of a choice
    std::size_t max_length = 0;        // a string's (0: no limit)
    int binds = 2;                     // a keybind's most binds
};

// A keybind's text as names of openrac/keys.h, split at commas or spaces: false when one is no
// name, one is there twice or there are more than `most`; else `normalized` is them comma-joined.
bool parse_binds(std::string_view text, int most, std::string& normalized);

struct Requirement {
    std::string id;
    std::string op;  // "", ">=" or "="
    Version version;
};

struct Conflict {
    std::string id;
    std::string reason;
};

struct Manifest {
    int format = 0;
    std::string id, name, version_text, author, description, license, homepage;
    Version version;
    bool has_api = false;
    int api_major = 0, api_minor = 0;
    std::map<std::string, std::vector<std::string>> games;  // game -> its versions (discs)
    std::map<std::string, std::string> plugin;               // platform -> path inside the mod
    bool script = false;                                     // "type": "script" (MODS.md, section 11)
    std::vector<std::string> scripts;                        // a script mod's .luau files, run in order
    std::vector<std::string> hooks;
    std::vector<Setting> settings;
    std::vector<Requirement> requirements;  // the manifest's "requires"
    std::vector<Conflict> conflicts;
    std::vector<std::string> load_after, load_before;
    // "vulkan_device": true (interface 1.4): the mod makes the Vulkan device with the port
    // (openrac_api.setup_vulkan), so the mods load before the window opens. Ignored on a script
    // mod or one of an older interface.
    bool vulkan_device = false;
};

// A manifest's text checked by the rules of MODS.md section 3. False, and why, when one is broken.
bool parse_manifest(std::string_view text, Manifest& out, std::string& error);

bool valid_id(std::string_view id);

// A path inside a mod to one of its .luau files: relative, with /, no empty, "." or ".." part.
bool script_path(std::string_view path);

// One mod of the launcher's list, in its order. `settings` keeps the values as written (JSON
// values of the object), checked against the manifest by resolve_settings.
struct ListEntry {
    std::string id;
    bool enabled = true;
    std::map<std::string, json::Value> settings;  // key -> its value as written
};
bool parse_list(std::string_view text, std::vector<ListEntry>& out, std::string& error);

// The values a mod gets: each declared setting's from the list where it is of the right type and
// in range, its default otherwise (a line in `notes` says so).
std::map<std::string, SettingValue> resolve_settings(
    const Manifest& manifest,
    const std::map<std::string, json::Value>& written,
    std::vector<std::string>& notes
);

// This build's platform key: "macos-arm64", "linux-x86_64", "windows-x86_64", ...
const char* platform_key();

// What the port is, for the checks.
struct Target {
    std::string game;     // "rac1"
    std::string version;  // "pal"
    std::string platform;  // platform_key()
    int api_major = 0;
    int api_minor = 0;
    bool scripts = false;  // a script binding for this game (script mods can run)
};

// A mod the list has on, with its manifest (or why it could not be read).
struct Candidate {
    ListEntry entry;
    bool has_manifest = false;
    Manifest manifest;
    std::string manifest_error;
};

// The answer for one candidate: open it, or refused and why.
struct Decision {
    std::string id;
    bool load = false;
    std::string reason;
    std::vector<std::string> warnings;  // load hints not met, hooks shared
};

// The checks of MODS.md section 5, step 2, in the list's order: the manifest, the game and the disc,
// the interface's version, a library for this platform (`has_file` says whether the mod's path
// exists), the dependencies (on, loading, earlier in the list, of a version that satisfies) and the
// conflicts (with any mod on that loads, either side's list).
std::vector<Decision> plan(const std::vector<Candidate>& enabled, const Target& target,
                           const std::function<bool(const std::string& id, const std::string& path)>& has_file);

}  // namespace openrac::mods

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Native mods' manifests and the launcher's list, read and checked (mod_manifest.h).

#include "mod_manifest.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <set>

#include "openrac/keys.h"

namespace openrac::mods {
namespace json = viewer::json;

namespace {

bool is_bool(const json::Value& v) {
    return v.type() == json::Value::Type::Bool;
}

bool is_integer(const json::Value& v) {
    return v.is_number() && std::isfinite(v.number()) && std::floor(v.number()) == v.number()
           && std::fabs(v.number()) < 9.0e15;
}

bool digits(std::string_view s, int& out) {
    if (s.empty() || s.size() > 9 || (s.size() > 1 && s[0] == '0')) {
        return false;
    }
    int v = 0;
    for (char c : s) {
        if (c < '0' || c > '9') {
            return false;
        }
        v = v * 10 + (c - '0');
    }
    out = v;
    return true;
}

// A string member, required or not.
bool text(const json::Value& o, std::string_view key, bool required, std::string& out, std::string& error) {
    const json::Value& v = o[key];
    if (v.is_null() && !required) {
        return true;
    }
    if (!v.is_string() || (required && v.string().empty())) {
        error = std::format("\"{}\" must be a{} string", key, required ? " non-empty" : "");
        return false;
    }
    out = v.string();
    return true;
}

bool id_list(const json::Value& v, std::string_view what, std::vector<std::string>& out, std::string& error) {
    if (v.is_null()) {
        return true;
    }
    if (!v.is_array()) {
        error = std::format("\"{}\" must be a list", what);
        return false;
    }
    for (const json::Value& item : v.items()) {
        if (!item.is_string() || item.string().empty()) {
            error = std::format("\"{}\" must hold names", what);
            return false;
        }
        out.push_back(item.string());
    }
    return true;
}

const char* type_name(SettingType t) {
    switch (t) {
        case SettingType::Bool:
            return "bool";
        case SettingType::Int:
            return "int";
        case SettingType::Float:
            return "float";
        case SettingType::Choice:
            return "choice";
        case SettingType::String:
            return "string";
        case SettingType::Keybind:
            return "keybind";
    }
    return "?";
}

// A value of a setting's type from JSON, within its limits; false when it is not one.
bool value_of(const Setting& s, const json::Value& v, SettingValue& out) {
    out = SettingValue{};
    out.type = s.type;
    switch (s.type) {
        case SettingType::Bool:
            if (!is_bool(v)) {
                return false;
            }
            out.b = v.boolean();
            return true;
        case SettingType::Int:
            if (!is_integer(v) || (s.min && v.number() < *s.min) || (s.max && v.number() > *s.max)) {
                return false;
            }
            out.i = static_cast<std::int64_t>(v.number());
            return true;
        case SettingType::Float:
            if (!v.is_number() || !std::isfinite(v.number()) || (s.min && v.number() < *s.min)
                || (s.max && v.number() > *s.max)) {
                return false;
            }
            out.f = v.number();
            return true;
        case SettingType::Choice:
            if (!v.is_string() || std::find(s.choices.begin(), s.choices.end(), v.string()) == s.choices.end()) {
                return false;
            }
            out.s = v.string();
            return true;
        case SettingType::String:
            if (!v.is_string() || (s.max_length != 0 && v.string().size() > s.max_length)) {
                return false;
            }
            out.s = v.string();
            return true;
        case SettingType::Keybind:
            return v.is_string() && parse_binds(v.string(), s.binds, out.s);
    }
    return false;
}

bool parse_setting(const json::Value& v, Setting& s, std::string& error) {
    if (!v.is_object()) {
        error = "each setting must be an object";
        return false;
    }
    if (!text(v, "key", true, s.key, error) || !text(v, "name", true, s.name, error)) {
        return false;
    }
    for (char c : s.key) {
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) {
            error = std::format("setting key \"{}\" may hold only a-z, 0-9 and _", s.key);
            return false;
        }
    }
    std::string type;
    if (!text(v, "type", true, type, error)) {
        return false;
    }
    if (type == "bool") {
        s.type = SettingType::Bool;
    } else if (type == "int") {
        s.type = SettingType::Int;
    } else if (type == "float") {
        s.type = SettingType::Float;
    } else if (type == "choice") {
        s.type = SettingType::Choice;
    } else if (type == "string") {
        s.type = SettingType::String;
    } else if (type == "keybind") {
        s.type = SettingType::Keybind;
    } else {
        error = std::format("setting \"{}\": unknown type \"{}\"", s.key, type);
        return false;
    }
    for (std::string_view bound : {"min", "max"}) {
        const json::Value& b = v[bound];
        if (b.is_null()) {
            continue;
        }
        if (!b.is_number() || (s.type != SettingType::Int && s.type != SettingType::Float)) {
            error = std::format("setting \"{}\": \"{}\" is for a number", s.key, bound);
            return false;
        }
        (bound == "min" ? s.min : s.max) = b.number();
    }
    if (s.min && s.max && *s.min > *s.max) {
        error = std::format("setting \"{}\": min is above max", s.key);
        return false;
    }
    if (s.type == SettingType::Choice) {
        const json::Value& choices = v["choices"];
        if (!choices.is_array() || choices.items().size() < 2) {
            error = std::format("setting \"{}\": a choice needs two or more \"choices\"", s.key);
            return false;
        }
        for (const json::Value& c : choices.items()) {
            if (!c.is_object() || !c["value"].is_string() || !c["name"].is_string()) {
                error = std::format("setting \"{}\": each choice is {{\"value\", \"name\"}}", s.key);
                return false;
            }
            s.choices.push_back(c["value"].string());
        }
    }
    if (const json::Value& m = v["max_length"]; !m.is_null()) {
        if (!is_integer(m) || m.number() < 0 || s.type != SettingType::String) {
            error = std::format("setting \"{}\": \"max_length\" is a count, for a string", s.key);
            return false;
        }
        s.max_length = static_cast<std::size_t>(m.number());
    }
    if (const json::Value& n = v["binds"]; !n.is_null()) {
        if (!is_integer(n) || n.number() < 1 || n.number() > 2 || s.type != SettingType::Keybind) {
            error = std::format("setting \"{}\": \"binds\" is 1 or 2, for a keybind", s.key);
            return false;
        }
        s.binds = static_cast<int>(n.number());
    }
    if (!value_of(s, v["default"], s.fallback)) {
        error = std::format("setting \"{}\": \"default\" must be a {} within its limits", s.key, type_name(s.type));
        return false;
    }
    return true;
}

bool parse_requirement(const json::Value& v, Requirement& r, std::string& error) {
    if (!v.is_object() || !v["id"].is_string() || !valid_id(v["id"].string())) {
        error = "each of \"requires\" is {\"id\", \"version\"} with a mod's id";
        return false;
    }
    r.id = v["id"].string();
    const json::Value& want = v["version"];
    if (want.is_null()) {
        return true;
    }
    std::string_view w = want.is_string() ? std::string_view(want.string()) : std::string_view();
    if (w.starts_with(">=")) {
        r.op = ">=";
        w.remove_prefix(2);
    } else if (w.starts_with("=")) {
        r.op = "=";
        w.remove_prefix(1);
    }
    const auto version = parse_version(w);
    if (r.op.empty() || !version) {
        error = std::format("requires \"{}\": \"version\" is \">=X.Y.Z\" or \"=X.Y.Z\"", r.id);
        return false;
    }
    r.version = *version;
    return true;
}

bool satisfies(const Version& have, const Requirement& r) {
    if (r.op.empty()) {
        return true;
    }
    const int c = compare(have, r.version);
    return r.op == "=" ? c == 0 : c >= 0;
}

}  // namespace

bool parse_binds(std::string_view text, int most, std::string& normalized) {
    normalized.clear();
    std::vector<int> seen;
    std::size_t i = 0;
    while (i < text.size()) {
        if (text[i] == ',' || text[i] == ' ') {
            ++i;
            continue;
        }
        std::size_t end = i;
        while (end < text.size() && text[end] != ',' && text[end] != ' ') {
            ++end;
        }
        const std::string_view name = text.substr(i, end - i);
        const int bind = openrac_bind_from_name_n(name.data(), name.size());
        if (bind == OPENRAC_BIND_NONE || std::find(seen.begin(), seen.end(), bind) != seen.end()
            || static_cast<int>(seen.size()) >= most) {
            normalized.clear();
            return false;
        }
        seen.push_back(bind);
        normalized += (normalized.empty() ? "" : ",") + std::string(name);
        i = end;
    }
    return true;
}

bool script_path(std::string_view p) {
    if (p.size() < 6 || !p.ends_with(".luau") || p.front() == '/' || p.find('\\') != std::string_view::npos
        || p.find(':') != std::string_view::npos) {
        return false;
    }
    std::size_t start = 0;
    while (start <= p.size()) {
        const std::size_t end = std::min(p.find('/', start), p.size());
        const std::string_view part = p.substr(start, end - start);
        if (part.empty() || part == "." || part == "..") {
            return false;
        }
        start = end + 1;
    }
    return true;
}

std::optional<Version> parse_version(std::string_view text) {
    Version v;
    const std::size_t dash = text.find('-');
    if (dash != std::string_view::npos) {
        v.suffix = std::string(text.substr(dash + 1));
        if (v.suffix.empty()) {
            return std::nullopt;
        }
        text = text.substr(0, dash);
    }
    const std::size_t a = text.find('.');
    const std::size_t b = a == std::string_view::npos ? a : text.find('.', a + 1);
    if (a == std::string_view::npos || b == std::string_view::npos || !digits(text.substr(0, a), v.major)
        || !digits(text.substr(a + 1, b - a - 1), v.minor) || !digits(text.substr(b + 1), v.patch)) {
        return std::nullopt;
    }
    return v;
}

int compare(const Version& a, const Version& b) {
    if (a.major != b.major) {
        return a.major < b.major ? -1 : 1;
    }
    if (a.minor != b.minor) {
        return a.minor < b.minor ? -1 : 1;
    }
    if (a.patch != b.patch) {
        return a.patch < b.patch ? -1 : 1;
    }
    if (a.suffix == b.suffix) {
        return 0;
    }
    if (a.suffix.empty() || b.suffix.empty()) {
        return a.suffix.empty() ? 1 : -1;  // 1.0.0-beta before 1.0.0
    }
    return a.suffix < b.suffix ? -1 : 1;
}

bool valid_id(std::string_view id) {
    if (id.empty() || id.size() > 64 || id.front() == '-' || id.back() == '-') {
        return false;
    }
    char last = 0;
    for (char c : id) {
        const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-';
        if (!ok || (c == '-' && last == '-')) {
            return false;
        }
        last = c;
    }
    return true;
}

bool parse_manifest(std::string_view source, Manifest& m, std::string& error) {
    m = Manifest{};
    json::Value v;
    if (!json::parse(source, v, error)) {
        return false;
    }
    if (!v.is_object()) {
        error = "the manifest is not a JSON object";
        return false;
    }
    if (!is_integer(v["format"])) {
        error = "\"format\" is missing";
        return false;
    }
    m.format = static_cast<int>(v["format"].number());
    if (m.format == 1) {
        error = "a format-1 manifest is a data pack (editor/MODDING.md), not a native mod";
        return false;
    }
    if (m.format != 2) {
        error = std::format("format {} is not known to this OpenRAC (it reads 2): update OpenRAC", m.format);
        return false;
    }
    if (!text(v, "id", true, m.id, error) || !text(v, "name", true, m.name, error)
        || !text(v, "version", true, m.version_text, error) || !text(v, "author", false, m.author, error)
        || !text(v, "description", false, m.description, error) || !text(v, "license", false, m.license, error)
        || !text(v, "homepage", false, m.homepage, error)) {
        return false;
    }
    if (!valid_id(m.id)) {
        error = std::format("id \"{}\" may hold only a-z, 0-9 and single hyphens, at most 64 characters", m.id);
        return false;
    }
    if (const auto version = parse_version(m.version_text)) {
        m.version = *version;
    } else {
        error = std::format("version \"{}\" is not MAJOR.MINOR.PATCH", m.version_text);
        return false;
    }
    if (!m.homepage.empty() && !m.homepage.starts_with("https://")) {
        error = "\"homepage\" must be an https:// address";
        return false;
    }
    const json::Value& games = v["games"];
    if (!games.is_object() || games.members().empty()) {
        error = "\"games\" must name at least one game";
        return false;
    }
    for (const auto& [game, versions] : games.members()) {
        std::vector<std::string> list;
        if (!id_list(versions, "games", list, error) || list.empty()) {
            error = std::format("\"games\": {} must list its versions (discs)", game);
            return false;
        }
        m.games[game] = list;
    }
    const json::Value& plugin = v["plugin"];
    if (!plugin.is_null()) {
        if (!plugin.is_object() || plugin.members().empty()) {
            error = "\"plugin\" must map platforms to libraries";
            return false;
        }
        static const std::set<std::string, std::less<>> kPlatforms = {
            "macos-arm64", "macos-x86_64", "linux-x86_64", "linux-aarch64", "windows-x86_64", "windows-arm64"};
        for (const auto& [platform, path] : plugin.members()) {
            if (!kPlatforms.contains(platform)) {
                error = std::format("\"plugin\": unknown platform \"{}\"", platform);
                return false;
            }
            if (!path.is_string() || path.string().empty() || path.string().front() == '/'
                || path.string().find("..") != std::string::npos || path.string().find('\\') != std::string::npos) {
                error = std::format("\"plugin\": {}'s library must be a path inside the mod, with /", platform);
                return false;
            }
            m.plugin[platform] = path.string();
        }
    }
    const json::Value& api = v["api"];
    if (!api.is_null()) {
        int major = 0;
        int minor = 0;
        const std::string a = api.is_string() ? api.string() : std::string();
        const std::size_t dot = a.find('.');
        if (dot == std::string::npos || !digits(std::string_view(a).substr(0, dot), major)
            || !digits(std::string_view(a).substr(dot + 1), minor)) {
            error = "\"api\" is MAJOR.MINOR, as \"1.0\"";
            return false;
        }
        m.has_api = true;
        m.api_major = major;
        m.api_minor = minor;
    } else if (!m.plugin.empty()) {
        error = "a mod with a \"plugin\" must say which \"api\" it was built for";
        return false;
    }
    // A script mod (MODS.md, section 11): .luau files run by the port's Luau, no library.
    const json::Value& type = v["type"];
    if (!type.is_null()) {
        if (!type.is_string() || (type.string() != "native" && type.string() != "script")) {
            error = "\"type\" is \"native\" or \"script\"";
            return false;
        }
        m.script = type.string() == "script";
    }
    const json::Value& scripts = v["scripts"];
    if (!scripts.is_null() && !m.script) {
        error = "\"scripts\" are for a mod of \"type\": \"script\"";
        return false;
    }
    if (m.script) {
        if (!m.plugin.empty()) {
            error = "a script mod has no \"plugin\"";
            return false;
        }
        if (!m.has_api) {
            error = "a script mod must say which \"api\" it was written for (1.1 or later)";
            return false;
        }
        if (m.api_major == 1 && m.api_minor < 1) {
            error = "script mods need interface 1.1 or later (\"api\": \"1.1\")";
            return false;
        }
        if (!scripts.is_array() || scripts.items().empty()) {
            error = "a script mod must list its \"scripts\" (\"main.luau\", ...)";
            return false;
        }
        for (const json::Value& s : scripts.items()) {
            const std::string p = s.is_string() ? s.string() : std::string();
            if (!script_path(p)) {
                error = std::format("\"scripts\": \"{}\" must be a .luau file inside the mod, with /", p);
                return false;
            }
            m.scripts.push_back(p);
        }
    }
    if (!id_list(v["hooks"], "hooks", m.hooks, error)) {
        return false;
    }
    const json::Value& settings = v["settings"];
    if (!settings.is_null()) {
        if (!settings.is_array()) {
            error = "\"settings\" must be a list";
            return false;
        }
        std::set<std::string> keys;
        for (const json::Value& item : settings.items()) {
            Setting s;
            if (!parse_setting(item, s, error)) {
                return false;
            }
            if (!keys.insert(s.key).second) {
                error = std::format("setting \"{}\" is declared twice", s.key);
                return false;
            }
            m.settings.push_back(std::move(s));
        }
    }
    if (const json::Value& r = v["requires"]; !r.is_null()) {
        if (!r.is_array()) {
            error = "\"requires\" must be a list";
            return false;
        }
        for (const json::Value& item : r.items()) {
            Requirement req;
            if (!parse_requirement(item, req, error)) {
                return false;
            }
            m.requirements.push_back(std::move(req));
        }
    }
    if (const json::Value& c = v["conflicts"]; !c.is_null()) {
        if (!c.is_array()) {
            error = "\"conflicts\" must be a list";
            return false;
        }
        for (const json::Value& item : c.items()) {
            if (!item.is_object() || !item["id"].is_string() || !valid_id(item["id"].string())) {
                error = "each of \"conflicts\" is {\"id\", \"reason\"} with a mod's id";
                return false;
            }
            m.conflicts.push_back({item["id"].string(), item["reason"].is_string() ? item["reason"].string() : ""});
        }
    }
    if (const json::Value& load = v["load"]; !load.is_null()) {
        if (!load.is_object() || !id_list(load["after"], "load.after", m.load_after, error)
            || !id_list(load["before"], "load.before", m.load_before, error)) {
            if (error.empty()) {
                error = "\"load\" is {\"after\": [...], \"before\": [...]}";
            }
            return false;
        }
    }
    return true;
}

bool parse_list(std::string_view source, std::vector<ListEntry>& out, std::string& error) {
    out.clear();
    json::Value v;
    if (!json::parse(source, v, error)) {
        return false;
    }
    if (!v.is_object() || !is_integer(v["format"])) {
        error = "the list has no \"format\"";
        return false;
    }
    if (v["format"].number() != 1) {
        error = std::format("the list is of format {} (this OpenRAC reads 1): update OpenRAC", v["format"].number());
        return false;
    }
    const json::Value& mods = v["mods"];
    if (!mods.is_array()) {
        error = "\"mods\" must be a list";
        return false;
    }
    std::set<std::string> seen;
    for (const json::Value& item : mods.items()) {
        if (!item.is_object() || !item["id"].is_string() || !valid_id(item["id"].string())) {
            error = "each of \"mods\" is {\"id\", \"enabled\", \"settings\"} with a mod's id";
            return false;
        }
        ListEntry e;
        e.id = item["id"].string();
        if (!seen.insert(e.id).second) {
            continue;  // the first place counts
        }
        e.enabled = item["enabled"].is_null() || item["enabled"].boolean(false);
        if (const json::Value& s = item["settings"]; s.is_object()) {
            for (const auto& [key, value] : s.members()) {
                e.settings[key] = value;
            }
        }
        out.push_back(std::move(e));
    }
    return true;
}

std::map<std::string, SettingValue> resolve_settings(const Manifest& manifest,
                                                     const std::map<std::string, json::Value>& written,
                                                     std::vector<std::string>& notes) {
    std::map<std::string, SettingValue> out;
    for (const Setting& s : manifest.settings) {
        SettingValue value = s.fallback;
        if (auto it = written.find(s.key); it != written.end()) {
            if (!value_of(s, it->second, value)) {
                value = s.fallback;
                notes.push_back(std::format("setting \"{}\" is not a {} within its limits: its default instead",
                                            s.key, type_name(s.type)));
            }
        }
        out[s.key] = value;
    }
    for (const auto& [key, value] : written) {
        (void)value;
        if (!out.contains(key)) {
            notes.push_back(std::format("setting \"{}\" is not one of the mod's: left out", key));
        }
    }
    return out;
}

const char* platform_key() {
#if defined(__aarch64__) || defined(_M_ARM64)
#if defined(__APPLE__)
    return "macos-arm64";
#elif defined(_WIN32)
    return "windows-arm64";
#else
    return "linux-aarch64";
#endif
#else
#if defined(__APPLE__)
    return "macos-x86_64";
#elif defined(_WIN32)
    return "windows-x86_64";
#else
    return "linux-x86_64";
#endif
#endif
}

std::vector<Decision> plan(const std::vector<Candidate>& enabled, const Target& target,
                           const std::function<bool(const std::string& id, const std::string& path)>& has_file) {
    std::vector<Decision> out;
    // Ids that load, with their version, as the list goes on.
    std::map<std::string, Version> loading;
    std::map<std::string, std::size_t> position;
    for (std::size_t i = 0; i < enabled.size(); ++i) {
        position[enabled[i].entry.id] = i;
    }
    auto refuse = [&](Decision& d, std::string reason) {
        d.load = false;
        d.reason = std::move(reason);
    };
    for (const Candidate& c : enabled) {
        Decision d;
        d.id = c.entry.id;
        d.load = true;
        const Manifest& m = c.manifest;
        if (!c.has_manifest) {
            refuse(d, c.manifest_error.empty() ? "its openrac-mod.json cannot be read" : c.manifest_error);
        } else if (m.id != c.entry.id) {
            refuse(d, std::format("its manifest's id is \"{}\", not its folder's", m.id));
        } else if (!m.games.contains(target.game)) {
            refuse(d, std::format("it is not for this game ({})", target.game));
        } else if (const auto& discs = m.games.at(target.game);
                   std::find(discs.begin(), discs.end(), target.version) == discs.end()) {
            refuse(d, std::format("it is not for this disc ({}/{})", target.game, target.version));
        } else if (!m.script && m.plugin.empty()) {
            refuse(d, "it has no library: data mods are packs (editor/MODDING.md)");
        } else if (m.script && !target.scripts) {
            refuse(d, std::format("this OpenRAC runs no scripts for {}", target.game));
        } else if (m.api_major != target.api_major) {
            refuse(d, std::format("needs interface {}.{}; this OpenRAC has {}.{}", m.api_major, m.api_minor,
                                  target.api_major, target.api_minor));
        } else if (m.api_minor > target.api_minor) {
            refuse(d, std::format("needs interface {}.{}; this OpenRAC has {}.{}: update OpenRAC", m.api_major,
                                  m.api_minor, target.api_major, target.api_minor));
        } else if (m.script) {
            for (const std::string& s : m.scripts) {
                if (!has_file(m.id, s)) {
                    refuse(d, std::format("its script {} is missing", s));
                    break;
                }
            }
        } else if (!m.plugin.contains(target.platform)) {
            refuse(d, std::format("it has no library for this system ({})", target.platform));
        } else if (!has_file(m.id, m.plugin.at(target.platform))) {
            refuse(d, std::format("its library {} is missing", m.plugin.at(target.platform)));
        }
        if (d.load) {
            for (const Requirement& r : m.requirements) {
                const auto it = loading.find(r.id);
                if (it == loading.end()) {
                    if (position.contains(r.id) && position[r.id] > position[d.id]) {
                        refuse(d, std::format("needs mod {} to load before it: move it below {}", r.id, r.id));
                    } else {
                        refuse(d, std::format("needs mod {}{}", r.id,
                                              r.op.empty() ? "" : std::format(" {}{}.{}.{}", r.op, r.version.major,
                                                                              r.version.minor, r.version.patch)));
                    }
                    break;
                }
                if (!satisfies(it->second, r)) {
                    refuse(d, std::format("needs mod {} {}{}.{}.{}", r.id, r.op, r.version.major, r.version.minor,
                                          r.version.patch));
                    break;
                }
            }
        }
        if (d.load) {
            // Conflicts: this mod's own list against every mod that loads, and theirs against it.
            for (const Candidate& other : enabled) {
                if (other.entry.id == d.id || !loading.contains(other.entry.id)) {
                    continue;
                }
                auto lists = [](const Manifest& a, const std::string& id) -> const Conflict* {
                    for (const Conflict& x : a.conflicts) {
                        if (x.id == id) {
                            return &x;
                        }
                    }
                    return nullptr;
                };
                const Conflict* x = lists(m, other.entry.id);
                if (x == nullptr && other.has_manifest) {
                    x = lists(other.manifest, d.id);
                }
                if (x != nullptr) {
                    refuse(d, std::format("conflicts with mod {}{}", other.entry.id,
                                          x->reason.empty() ? "" : ": " + x->reason));
                    break;
                }
            }
        }
        if (d.load) {
            for (const std::string& a : m.load_after) {
                if (position.contains(a) && position[a] > position[d.id]) {
                    d.warnings.push_back(std::format("asks to load after {}, which is below it", a));
                }
            }
            for (const std::string& b : m.load_before) {
                if (position.contains(b) && position[b] < position[d.id]) {
                    d.warnings.push_back(std::format("asks to load before {}, which is above it", b));
                }
            }
            loading[d.id] = m.version;
        }
        out.push_back(std::move(d));
    }
    return out;
}

}  // namespace openrac::mods

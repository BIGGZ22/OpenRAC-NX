// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The native mod loader (mods.h, docs/port/MODS.md).

#include "mods.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

#include "common/c_boundary.h"
#include "common/crash.h"
#include "common/log.h"
#include "mod_library.h"
#include "mod_manifest.h"
#include "mod_scripts.h"
#include "openrac/guest.h"
#include "openrac/openrac_mod.h"

namespace fs = std::filesystem;
using namespace openrac;

// One loaded mod: the handle openrac_mod.h's functions are given.
struct openrac_mod {
    std::string id;
    std::string version;
    std::string file;  // the library's file name (a script mod's: its first script)
    bool script = false;  // a script mod (mod_scripts.h): no library
    void* library = nullptr;
    const void* base = nullptr;
    mods::Manifest manifest;
    std::map<std::string, mods::SettingValue> settings;
    std::string failure;  // fail()'s reason, or a hook point refused
    bool hook_refused = false;
    std::vector<std::pair<void**, void*>> taken;  // each slot it filled, and what was there before
    std::vector<std::string> hooks;               // the hook points it took
    openrac_mod_unload_fn unload = nullptr;
};

namespace {

// openrac_script_hook_id's body: the binding's aliases are sorted, so by halves.
const char* permanent_hook_id(const openrac_script_binding* binding, const char* name) {
    if (binding == nullptr || name == nullptr || binding->aliases == nullptr) {
        return name;
    }
    int low = 0;
    int high = binding->alias_count - 1;
    while (low <= high) {
        const int mid = low + (high - low) / 2;
        const int c = std::strcmp(binding->aliases[mid].alias, name);
        if (c == 0) {
            return binding->aliases[mid].id;
        }
        if (c < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return name;
}

}  // namespace

// A hook point's permanent id from any of its names (openrac/script_binding.h). An exception is
// reported to the log and gives `name` back, the contract's answer for a name with no alias
// (CODING_CONVENTIONS.md, section 13.4).
extern "C" const char* openrac_script_hook_id(
    const openrac_script_binding* binding, const char* name
) {
    return c_boundary::call("openrac_script_hook_id", name, [&] {
        return permanent_hook_id(binding, name);
    });
}

namespace openrac::mods {

bool g_frame_callbacks = false;

namespace {

struct FrameCallback {
    openrac_frame_fn fn;
    void* user;
    openrac_mod* mod;
};
struct PadCallback {
    openrac_pad_fn fn;
    void* user;
    openrac_mod* mod;
};

const Host* g_host = nullptr;

// A hook point's id from one of its names, through the game's binding (none: the name).
const char* hook_id(const Host& host, const char* name) {
    return openrac_script_hook_id(host.scripts, name);
}
openrac_mod* g_loading = nullptr;  // the mod whose openrac_mod_load runs: hooks are taken then only
std::vector<std::unique_ptr<openrac_mod>> g_loaded;
std::vector<FrameCallback> g_frames;
std::vector<PadCallback> g_pads;
// The views and the Vulkan device setup (interface 1.4): one mod's each, none while its mod is null.
openrac_view_source g_views{};
openrac_mod* g_views_mod = nullptr;
openrac_vulkan_setup g_vulkan{};
openrac_mod* g_vulkan_mod = nullptr;
std::vector<crash::ModImage> g_images;
openrac_api g_api;

// ---- The interface's functions ----

void api_log(openrac_mod* mod, int level, const char* text) {
    const std::string_view id = mod != nullptr ? std::string_view(mod->id) : std::string_view("?");
    const char* t = text != nullptr ? text : "";
    switch (level) {
        case OPENRAC_LOG_DEBUG:
            log::debug("[mod {}] {}", id, t);
            break;
        case OPENRAC_LOG_WARN:
            log::warn("[mod {}] {}", id, t);
            break;
        case OPENRAC_LOG_ERROR:
            log::error("[mod {}] {}", id, t);
            break;
        default:
            log::info("[mod {}] {}", id, t);
            break;
    }
}

void api_fail(openrac_mod* mod, const char* reason) {
    if (mod != nullptr && reason != nullptr) {
        mod->failure = reason;
    }
}

const SettingValue* setting(openrac_mod* mod, const char* key, SettingType a, SettingType b) {
    if (mod == nullptr || key == nullptr) {
        return nullptr;
    }
    const auto it = mod->settings.find(key);
    if (it == mod->settings.end() || (it->second.type != a && it->second.type != b)) {
        return nullptr;
    }
    return &it->second;
}

int api_setting_bool(openrac_mod* mod, const char* key) {
    const SettingValue* v = setting(mod, key, SettingType::Bool, SettingType::Bool);
    return v != nullptr && v->b ? 1 : 0;
}

int64_t api_setting_int(openrac_mod* mod, const char* key) {
    const SettingValue* v = setting(mod, key, SettingType::Int, SettingType::Int);
    return v != nullptr ? v->i : 0;
}

double api_setting_float(openrac_mod* mod, const char* key) {
    const SettingValue* v = setting(mod, key, SettingType::Float, SettingType::Int);
    if (v == nullptr) {
        return 0.0;
    }
    return v->type == SettingType::Int ? static_cast<double>(v->i) : v->f;
}

const char* api_setting_string(openrac_mod* mod, const char* key) {
    const SettingValue* v = setting(mod, key, SettingType::String, SettingType::Choice);
    if (v == nullptr) {
        v = setting(mod, key, SettingType::Keybind, SettingType::Keybind);
    }
    return v != nullptr ? v->s.c_str() : "";
}

int api_hook(openrac_mod* mod, const char* name, const char* signature, void* replacement, void** original) {
    if (mod == nullptr || mod != g_loading || g_host == nullptr) {
        return OPENRAC_HOOK_TOO_LATE;
    }
    if (name == nullptr || replacement == nullptr || original == nullptr) {
        return OPENRAC_HOOK_UNKNOWN;
    }
    // Asked for by its id or any alias (an earlier name, a symbol): the hook point's id decides.
    const char* id = hook_id(*g_host, name);
    for (int i = 0; i < g_host->hook_count; ++i) {
        const HookPoint& p = g_host->hooks[i];
        if (p.name == nullptr || std::strcmp(hook_id(*g_host, p.name), id) != 0) {
            continue;
        }
        if (signature == nullptr || std::strcmp(p.signature, signature) != 0) {
            mod->hook_refused = true;
            mod->failure = std::format("hook point {} is {} here, not {}", name, p.signature,
                                       signature != nullptr ? signature : "(none)");
            return OPENRAC_HOOK_SIGNATURE;
        }
        // Detours' way: the replacement gets what was there (an earlier mod's, or the game's C).
        void* before = *p.slot;
        *original = before != nullptr ? before : p.original;
        mod->taken.emplace_back(p.slot, before);
        *p.slot = replacement;
        mod->hooks.emplace_back(id);
        const auto listed = [&](const std::string& h) { return std::strcmp(hook_id(*g_host, h.c_str()), id) == 0; };
        if (std::none_of(mod->manifest.hooks.begin(), mod->manifest.hooks.end(), listed)) {
            log::warn("mod {} takes hook point {}, which its manifest's \"hooks\" does not list", mod->id, name);
        }
        return OPENRAC_HOOK_OK;
    }
    mod->hook_refused = true;
    mod->failure = std::format("this OpenRAC has no hook point {} for {}/{}", name, g_host->game, g_host->version);
    return OPENRAC_HOOK_UNKNOWN;
}

void run_pads(int port, unsigned char* report, int size) {
    for (const PadCallback& c : g_pads) {
        c.fn(c.user, port, report, size);
    }
}

int api_on_frame(openrac_mod* mod, openrac_frame_fn fn, void* user) {
    if (mod == nullptr || mod != g_loading || fn == nullptr) {
        return OPENRAC_HOOK_TOO_LATE;
    }
    g_frames.push_back({fn, user, mod});
    return OPENRAC_HOOK_OK;
}

int api_on_pad(openrac_mod* mod, openrac_pad_fn fn, void* user) {
    if (mod == nullptr || mod != g_loading || fn == nullptr) {
        return OPENRAC_HOOK_TOO_LATE;
    }
    g_pads.push_back({fn, user, mod});
    return OPENRAC_HOOK_OK;
}

void* api_function(const char* name) {
    return reinterpret_cast<void*>(openrac_guest_function_named(name));
}

void* api_function_at(openrac_gaddr address) {
    return reinterpret_cast<void*>(openrac_guest_function(address));
}

openrac_gaddr api_relocate_data(openrac_gaddr address) {
    return OPENRAC_DATA(address);
}

openrac_gaddr api_relocate_level_data(int level, openrac_gaddr address) {
    return openrac_relocate_level_data(level, address);
}

int api_loaded_level(void) {
    return g_host != nullptr && g_host->loaded_level != nullptr ? g_host->loaded_level() : -1;
}

openrac_gaddr api_stack_push(uint32_t size, openrac_gaddr* saved) {
    if (saved != nullptr) {
        *saved = openrac_guest_sp;
    }
    return openrac_guest_frame(size);
}

void api_stack_pop(openrac_gaddr saved) {
    openrac_guest_sp = saved;
}

const openrac_input* api_input(void) {
    static openrac_input none = [] {
        openrac_input i{};
        i.size = sizeof(openrac_input);
        return i;
    }();
    const openrac_input* in = g_host != nullptr && g_host->input != nullptr ? g_host->input() : nullptr;
    return in != nullptr ? in : &none;
}

int api_take_input(openrac_mod* mod, uint32_t devices) {
    if (mod == nullptr || mod != g_loading) {
        return OPENRAC_HOOK_TOO_LATE;
    }
    devices &= OPENRAC_INPUT_KEYBOARD | OPENRAC_INPUT_MOUSE;
    if (devices != 0 && g_host != nullptr && g_host->take_input != nullptr) {
        g_host->take_input(devices);
    }
    if ((devices & OPENRAC_INPUT_KEYBOARD) != 0) {
        log::info("mod {} takes the keyboard: the port's own keys no longer press pad 0's buttons", mod->id);
    }
    return OPENRAC_HOOK_OK;
}

void api_capture_mouse(openrac_mod* mod, int capture) {
    (void)mod;
    if (g_host != nullptr && g_host->capture_mouse != nullptr) {
        g_host->capture_mouse(capture != 0);
    }
}

const openrac_graphics* api_graphics(void) {
    static openrac_graphics none = [] {
        openrac_graphics n{};
        n.size = sizeof(openrac_graphics);
        return n;
    }();
    const openrac_graphics* graphics =
        g_host != nullptr && g_host->graphics != nullptr ? g_host->graphics() : nullptr;
    return graphics != nullptr ? graphics : &none;
}

// A structure the mod gives with its own size first, copied into `out`: what a mod built against
// a smaller one does not have stays zero. False when it is not even the size and the user pointer.
template <class T>
bool copy_sized(const T* from, T& out) {
    if (from == nullptr || from->size < offsetof(T, user) + sizeof(void*)) {
        return false;
    }
    out = T{};
    std::memcpy(&out, from, std::min<std::size_t>(from->size, sizeof(T)));
    out.size = sizeof(T);
    return true;
}

int api_on_views(openrac_mod* mod, const openrac_view_source* source) {
    if (mod == nullptr || mod != g_loading) {
        return OPENRAC_HOOK_TOO_LATE;
    }
    if (g_views_mod != nullptr) {
        log::warn("mod {} asks for views, which mod {} has", mod->id, g_views_mod->id);
        return OPENRAC_HOOK_TAKEN;
    }
    openrac_view_source s;
    if (!copy_sized(source, s) || s.begin == nullptr || s.end == nullptr) {
        return OPENRAC_HOOK_UNKNOWN;
    }
    g_views = s;
    g_views_mod = mod;
    return OPENRAC_HOOK_OK;
}

int api_setup_vulkan(openrac_mod* mod, const openrac_vulkan_setup* setup) {
    if (mod == nullptr || mod != g_loading) {
        return OPENRAC_HOOK_TOO_LATE;
    }
    if (g_vulkan_mod != nullptr) {
        log::warn(
            "mod {} asks to make the Vulkan device, which mod {} makes", mod->id, g_vulkan_mod->id
        );
        return OPENRAC_HOOK_TAKEN;
    }
    openrac_vulkan_setup s;
    if (!copy_sized(setup, s)) {
        return OPENRAC_HOOK_UNKNOWN;
    }
    if (!mod->manifest.vulkan_device) {
        log::warn(
            "mod {} makes the Vulkan device, but its manifest has no \"vulkan_device\": true; "
            "the window may open before it loads",
            mod->id
        );
    }
    g_vulkan = s;
    g_vulkan_mod = mod;
    return OPENRAC_HOOK_OK;
}

void fill_api(const Host& host) {
    static std::string game, version, openrac_version;
    game = host.game;
    version = host.version;
    openrac_version = host.openrac_version;
    g_api = openrac_api{};
    g_api.size = sizeof(openrac_api);
    g_api.api_major = OPENRAC_MOD_API_MAJOR;
    g_api.api_minor = OPENRAC_MOD_API_MINOR;
    g_api.game = game.c_str();
    g_api.version = version.c_str();
    g_api.openrac_version = openrac_version.c_str();
    g_api.platform = platform_key();
    g_api.memory = openrac_guest_base;
    g_api.log = api_log;
    g_api.fail = api_fail;
    g_api.setting_bool = api_setting_bool;
    g_api.setting_int = api_setting_int;
    g_api.setting_float = api_setting_float;
    g_api.setting_string = api_setting_string;
    g_api.hook = api_hook;
    g_api.on_frame = api_on_frame;
    g_api.on_pad = api_on_pad;
    g_api.function = api_function;
    g_api.function_at = api_function_at;
    g_api.relocate_data = api_relocate_data;
    g_api.relocate_level_data = api_relocate_level_data;
    g_api.loaded_level = api_loaded_level;
    g_api.stack_push = api_stack_push;
    g_api.stack_pop = api_stack_pop;
    g_api.input = api_input;
    g_api.take_input = api_take_input;
    g_api.capture_mouse = api_capture_mouse;
    g_api.graphics = api_graphics;
    g_api.on_views = api_on_views;
    g_api.setup_vulkan = api_setup_vulkan;
}

// ---- status.json ----

std::string quoted(std::string_view s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (c < 0x20) {
                    out += std::format("\\u{:04x}", c);
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out + "\"";
}

struct Outcome {
    std::string id;
    std::string version;
    std::string state;  // loaded, refused, failed
    std::string reason;
    std::vector<std::string> hooks;
};

void write_status(const Host& host, const std::vector<Outcome>& outcomes) {
    const fs::path folder = host.list.parent_path();
    std::error_code e;
    if (folder.empty() || !fs::is_directory(folder, e)) {
        return;
    }
    std::ostringstream o;
    o << "{\n  \"format\": 1,\n";
    o << "  \"time\": " << quoted(host.now) << ",\n";
    o << "  \"game\": " << quoted(host.game) << ", \"version\": " << quoted(host.version)
      << ", \"platform\": " << quoted(platform_key()) << ",\n";
    o << "  \"api\": " << quoted(std::format("{}.{}", OPENRAC_MOD_API_MAJOR, OPENRAC_MOD_API_MINOR))
      << ", \"openrac\": " << quoted(host.openrac_version) << ",\n";
    o << "  \"purist\": " << (host.purist ? "true" : "false") << ",\n";
    o << "  \"mods\": [";
    for (std::size_t i = 0; i < outcomes.size(); ++i) {
        const Outcome& m = outcomes[i];
        o << (i == 0 ? "\n" : ",\n") << "    {\"id\": " << quoted(m.id);
        if (!m.version.empty()) {
            o << ", \"version\": " << quoted(m.version);
        }
        o << ", \"state\": " << quoted(m.state);
        if (!m.reason.empty()) {
            o << ", \"reason\": " << quoted(m.reason);
        }
        if (!m.hooks.empty()) {
            o << ", \"hooks\": [";
            for (std::size_t h = 0; h < m.hooks.size(); ++h) {
                o << (h == 0 ? "" : ", ") << quoted(m.hooks[h]);
            }
            o << "]";
        }
        o << "}";
    }
    o << (outcomes.empty() ? "],\n" : "\n  ],\n");
    o << "  \"hook_points\": [";
    for (int i = 0; i < host.hook_count; ++i) {
        o << (i == 0 ? "" : ", ") << quoted(hook_id(host, host.hooks[i].name));
    }
    o << "]\n}\n";
    // Written beside and moved into place, so the launcher never reads half a file.
    const fs::path path = folder / "status.json";
    const fs::path temporary = folder / "status.json.tmp";
    {
        std::ofstream f(temporary, std::ios::binary | std::ios::trunc);
        f << o.str();
        if (!f) {
            log::warn("mods: {} cannot be written", log::tidy(path.string()));
            return;
        }
    }
    fs::rename(temporary, path, e);
    if (e) {
        log::warn("mods: {} cannot be written ({})", log::tidy(path.string()), e.message());
    }
}

bool read_text(const fs::path& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return false;
    }
    std::ostringstream s;
    s << f.rdbuf();
    out = s.str();
    return true;
}

// What status.json says of each mod of the list, kept for a script mod that stops later.
std::vector<Outcome> g_outcomes;

// A script mod stopped while the game ran (an error, its budget): status.json says so from now on.
// Its hooks pass to their originals and its callbacks stop (mod_scripts.cpp).
void script_stopped(openrac_mod* mod, const std::string& reason) {
    if (mod == nullptr || g_host == nullptr) {
        return;
    }
    for (Outcome& o : g_outcomes) {
        if (o.id == mod->id) {
            o.state = "failed";
            o.reason = "stopped while the game ran: " + reason;
        }
    }
    write_status(*g_host, g_outcomes);
}

// Takes back what a mod did in a load that failed: its hooks (it was the last to take any), its
// callbacks.
void undo(openrac_mod* mod) {
    for (auto it = mod->taken.rbegin(); it != mod->taken.rend(); ++it) {
        *it->first = it->second;
    }
    mod->taken.clear();
    mod->hooks.clear();
    std::erase_if(g_frames, [&](const FrameCallback& c) { return c.mod == mod; });
    std::erase_if(g_pads, [&](const PadCallback& c) { return c.mod == mod; });
    if (g_views_mod == mod) {
        g_views = {};
        g_views_mod = nullptr;
    }
    if (g_vulkan_mod == mod) {
        g_vulkan = {};
        g_vulkan_mod = nullptr;
    }
}

// Opens and loads one mod that passed the checks; false and why when it does not load.
bool load_one(const fs::path& folder, const Candidate& c, std::unique_ptr<openrac_mod>& out, std::string& state,
              std::string& reason) {
    auto mod = std::make_unique<openrac_mod>();
    mod->id = c.manifest.id;
    mod->version = c.manifest.version_text;
    mod->manifest = c.manifest;
    std::vector<std::string> notes;
    mod->settings = resolve_settings(c.manifest, c.entry.settings, notes);
    for (const std::string& n : notes) {
        log::warn("mod {}: {}", mod->id, n);
    }
    if (c.manifest.script) {
        mod->script = true;
        mod->file = c.manifest.scripts.front();
        scripts::Mod s;
        s.handle = mod.get();
        s.id = mod->id;
        s.folder = folder / c.manifest.id;
        s.scripts = c.manifest.scripts;
        s.settings = mod->settings;
        s.game = g_host->game;
        s.version = g_host->version;
        s.platform = platform_key();
        s.api = std::format("{}.{}", OPENRAC_MOD_API_MAJOR, OPENRAC_MOD_API_MINOR);
        const scripts::Services services{api_hook,  api_on_frame,  api_on_pad,        api_log,
                                         script_stopped, api_input, api_take_input, api_capture_mouse};
        std::string why;
        g_loading = mod.get();
        const bool loaded = scripts::load(s, g_host->scripts, services, why);
        g_loading = nullptr;
        if (!loaded || mod->hook_refused) {
            undo(mod.get());
            scripts::close(mod.get());
            state = "failed";
            reason = !mod->failure.empty() ? mod->failure : why;
            return false;
        }
        out = std::move(mod);
        return true;
    }
    const fs::path path = folder / c.manifest.id / fs::path(c.manifest.plugin.at(platform_key()));
    mod->file = path.filename().string();
    std::string error;
    mod->library = library::open(path, error);
    if (mod->library == nullptr) {
        state = "failed";
        reason = "its library cannot be opened: " + error;
        return false;
    }
    auto query = reinterpret_cast<openrac_mod_query_fn>(library::symbol(mod->library, OPENRAC_MOD_QUERY));
    auto load = reinterpret_cast<openrac_mod_load_fn>(library::symbol(mod->library, OPENRAC_MOD_LOAD));
    mod->unload = reinterpret_cast<openrac_mod_unload_fn>(library::symbol(mod->library, OPENRAC_MOD_UNLOAD));
    auto give_up = [&](std::string why) {
        library::close(mod->library);
        state = "failed";
        reason = std::move(why);
        return false;
    };
    if (query == nullptr || load == nullptr) {
        return give_up("its library does not export openrac_mod_query and openrac_mod_load");
    }
    openrac_mod_info info{};
    info.size = sizeof(info);
    query(&info);
    if (info.api_major != static_cast<uint32_t>(c.manifest.api_major)
        || info.api_minor != static_cast<uint32_t>(c.manifest.api_minor)) {
        return give_up(std::format("its library was built for interface {}.{}, its manifest says {}.{}",
                                   info.api_major, info.api_minor, c.manifest.api_major, c.manifest.api_minor));
    }
    if (info.id == nullptr || mod->id != info.id) {
        return give_up(std::format("its library is mod \"{}\", its manifest {}", info.id != nullptr ? info.id : "",
                                   mod->id));
    }
    mod->base = library::base_of(reinterpret_cast<const void*>(load));
    g_loading = mod.get();
    const int result = load(&g_api, mod.get());
    g_loading = nullptr;
    // A hook point asked for and not there (or of another signature) fails the load even when the
    // mod carries on: it would run with part of itself missing.
    if (result != 0 || mod->hook_refused) {
        undo(mod.get());
        state = "failed";
        reason = !mod->failure.empty() ? mod->failure : std::format("its load returned {}", result);
        library::close(mod->library);
        return false;
    }
    out = std::move(mod);
    return true;
}

}  // namespace

std::string load(const Host& host) {
    if (host.purist) {
        log::info("mods: purist mode (--no-mods), none loaded");
        return "none (purist mode)";
    }
    std::string text;
    if (!read_text(host.list, text)) {
        return "none";
    }
    std::vector<ListEntry> list;
    std::string error;
    if (!parse_list(text, list, error)) {
        log::warn("mods: {} cannot be read ({}): none loaded", log::tidy(host.list.string()), error);
        return "none";
    }
    static Host kept;
    kept = host;
    g_host = &kept;
    const fs::path folder = host.list.parent_path();
    std::vector<Candidate> enabled;
    for (const ListEntry& e : list) {
        if (!e.enabled) {
            continue;
        }
        Candidate c;
        c.entry = e;
        std::string manifest;
        if (!read_text(folder / e.id / "openrac-mod.json", manifest)) {
            c.manifest_error = "it is not installed (no openrac-mod.json in its folder)";
        } else {
            c.has_manifest = parse_manifest(manifest, c.manifest, c.manifest_error);
            if (!c.has_manifest) {
                c.manifest_error = "its openrac-mod.json: " + c.manifest_error;
            }
        }
        enabled.push_back(std::move(c));
    }
    if (enabled.empty()) {
        write_status(kept, {});
        return "none";
    }
    fill_api(kept);
    const Target target{host.game, host.version, platform_key(), OPENRAC_MOD_API_MAJOR, OPENRAC_MOD_API_MINOR,
                        host.scripts != nullptr};
    const auto decisions = plan(enabled, target, [&](const std::string& id, const std::string& path) {
        std::error_code e;
        return fs::is_regular_file(folder / id / fs::path(path), e);
    });
    std::vector<Outcome> outcomes;
    std::vector<std::string> not_loaded;  // refused or failed here: what requires them goes too
    for (std::size_t i = 0; i < enabled.size(); ++i) {
        const Candidate& c = enabled[i];
        const Decision& d = decisions[i];
        Outcome o{c.entry.id, c.has_manifest ? c.manifest.version_text : "", "loaded", "", {}};
        for (const std::string& w : d.warnings) {
            log::warn("mod {}: {}", d.id, w);
        }
        if (!d.load) {
            o.state = "refused";
            o.reason = d.reason;
        } else {
            for (const Requirement& r : c.manifest.requirements) {
                if (std::find(not_loaded.begin(), not_loaded.end(), r.id) != not_loaded.end()) {
                    o.state = "refused";
                    o.reason = std::format("needs mod {}, which did not load", r.id);
                    break;
                }
            }
            if (o.state == "loaded") {
                std::unique_ptr<openrac_mod> mod;
                if (load_one(folder, c, mod, o.state, o.reason)) {
                    o.hooks = mod->hooks;
                    log::info("mod {} {} loaded ({}{})", mod->id, mod->version, mod->file,
                              mod->hooks.empty() ? std::string() : ", hooks " + [&] {
                                  std::string s;
                                  for (const std::string& h : mod->hooks) {
                                      s += (s.empty() ? "" : " ") + h;
                                  }
                                  return s;
                              }());
                    g_loaded.push_back(std::move(mod));
                }
            }
        }
        if (o.state != "loaded") {
            not_loaded.push_back(o.id);
            log::warn("mod {} not loaded: {}", o.id, o.reason);
        }
        outcomes.push_back(std::move(o));
    }
    g_frame_callbacks = !g_frames.empty();
    if (host.pad_hook != nullptr) {
        *host.pad_hook = g_pads.empty() ? nullptr : run_pads;
    }
    if (g_views_mod != nullptr && host.set_views != nullptr) {
        log::info("mod {} draws views of the game", g_views_mod->id);
        host.set_views(&g_views);
    }
    if (g_vulkan_mod != nullptr && host.set_vulkan_setup != nullptr) {
        log::info("mod {} makes the Vulkan device", g_vulkan_mod->id);
        host.set_vulkan_setup(&g_vulkan);
    }
    g_outcomes = outcomes;
    write_status(kept, outcomes);
    g_images.clear();
    for (const auto& m : g_loaded) {
        g_images.push_back({m->id.c_str(), m->version.c_str(), m->file.c_str(), m->base});
    }
    crash::set_mods(g_images.data(), static_cast<int>(g_images.size()));
    std::string line;
    for (const auto& m : g_loaded) {
        line += (line.empty() ? "" : ", ") + m->id + " " + m->version;
    }
    const std::size_t refused = enabled.size() - g_loaded.size();
    if (line.empty()) {
        line = "none";
    }
    if (refused > 0) {
        line += std::format(" ({} not loaded: plugins/status.json)", refused);
    }
    return line;
}

bool makes_vulkan_device(const fs::path& list_path, bool purist) {
    std::string text;
    std::vector<ListEntry> list;
    std::string error;
    if (purist || !read_text(list_path, text) || !parse_list(text, list, error)) {
        return false;
    }
    const fs::path folder = list_path.parent_path();
    for (const ListEntry& e : list) {
        std::string manifest_text;
        Manifest manifest;
        if (e.enabled && read_text(folder / e.id / "openrac-mod.json", manifest_text)
            && parse_manifest(manifest_text, manifest, error) && manifest.vulkan_device) {
            return true;
        }
    }
    return false;
}

void unload() {
    for (auto it = g_loaded.rbegin(); it != g_loaded.rend(); ++it) {
        if ((*it)->unload != nullptr) {
            (*it)->unload();
        }
    }
}

void run_frame_callbacks() {
    for (const FrameCallback& c : g_frames) {
        c.fn(c.user);
    }
}

int loaded_count() {
    return static_cast<int>(g_loaded.size());
}

void reset() {
    unload();
    for (auto it = g_loaded.rbegin(); it != g_loaded.rend(); ++it) {
        undo(it->get());
        if ((*it)->library != nullptr) {
            library::close((*it)->library);
        }
    }
    scripts::reset();
    g_outcomes.clear();
    g_loaded.clear();
    g_frames.clear();
    g_pads.clear();
    g_frame_callbacks = false;
    if (g_host != nullptr && g_host->pad_hook != nullptr) {
        *g_host->pad_hook = nullptr;
    }
    if (g_host != nullptr && g_host->set_views != nullptr) {
        g_host->set_views(nullptr);
    }
    if (g_host != nullptr && g_host->set_vulkan_setup != nullptr) {
        g_host->set_vulkan_setup(nullptr);
    }
    crash::set_mods(nullptr, 0);
    g_images.clear();
    g_host = nullptr;
}

}  // namespace openrac::mods

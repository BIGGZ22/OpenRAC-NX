// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Script mods in Luau (mod_scripts.h, docs/port/MODS.md section 11).

#include "mod_scripts.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <format>
#include <fstream>
#include <memory>
#include <sstream>

#include "common/c_boundary.h"
#include "common/crash.h"
#include "common/env.h"
#include "lua.h"
#include "luacode.h"
#include "lualib.h"
#include "openrac/guest.h"
#include "openrac/keys.h"

namespace fs = std::filesystem;

namespace openrac::mods::scripts {

namespace {

constexpr std::uint32_t kRamSize = 32u * 1024u * 1024u;  // what read and write reach: game RAM

using Clock = std::chrono::steady_clock;

double g_budget_ms = [] {
    const double v = env::number(env::Var::ScriptBudgetMs, 2.0);
    if (v > 0.0) {
        return v;
    }
    return 2.0;
}();

struct State;

// The time a script has run in the call under way: the time in the game's own code (an original
// it calls) does not count.
struct Budget {
    double used = 0.0;  // seconds, before `start`
    Clock::time_point start;
    bool running = false;
};

// A place in a hook point's chain this mod holds.
struct Link {
    State* state = nullptr;
    const openrac_script_point* point = nullptr;
    openrac_script_link* link = nullptr;
    int fn = LUA_NOREF;  // the script's function
    enum class Mode { Wrap, Before, After } mode = Mode::Wrap;
    // The call under way (calls nest: the game may reach the hook point again inside it).
    const openrac_script_value* args = nullptr;
    openrac_script_value result{};
    bool called = false;  // the original ran in this call
};

struct Callback {
    State* state = nullptr;
    int fn = LUA_NOREF;
};

struct State {
    Mod mod;
    Services services;
    const openrac_script_binding* binding = nullptr;
    int disc = -1;  // the binding's index of this disc
    lua_State* L = nullptr;
    lua_State* T = nullptr;  // the sandboxed thread every script of the mod runs in
    int thread_ref = LUA_NOREF;
    bool loading = false;
    bool stopped = false;
    std::string stop_reason;
    Budget budget;
    std::string where;  // what the call under way is: "load", "hook Hero_TurnToHeading", ...
    std::vector<std::unique_ptr<Link>> links;
    std::vector<std::unique_ptr<Callback>> callbacks;
};

std::vector<std::unique_ptr<State>> g_states;
const char* g_running = nullptr;  // the script mod running now, for a crash report

State* state_of(lua_State* L) {
    return static_cast<State*>(lua_callbacks(L)->userdata);
}

double elapsed(const Budget& b) {
    return b.used + (b.running ? std::chrono::duration<double>(Clock::now() - b.start).count() : 0.0);
}

void interrupt(lua_State* L, int gc) {
    if (gc >= 0) {
        return;
    }
    State* s = state_of(L);
    if (s != nullptr && s->budget.running && elapsed(s->budget) * 1000.0 > g_budget_ms) {
        luaL_error(L, "ran over its time budget (%g ms)", g_budget_ms);
    }
}

// The message and the traceback of an error, as the handler lua_pcall calls gives them.
int traceback(lua_State* L) {
    const char* message = lua_tostring(L, -1);
    std::string text = message != nullptr ? message : "an error";
    text += "\n";
    text += lua_debugtrace(L);
    lua_pushlstring(L, text.data(), text.size());
    return 1;
}

void log_line(State* s, int level, const std::string& text) {
    s->services.log(s->mod.handle, level, text.c_str());
}

// The mod stops: its hooks pass straight to their originals, its callbacks no longer run.
void stop(State* s, const std::string& reason) {
    if (s->stopped) {
        return;
    }
    s->stopped = true;
    s->stop_reason = reason;
    log_line(s, OPENRAC_LOG_ERROR, "stopped: " + reason);
    if (!s->loading && s->services.stopped != nullptr) {
        s->services.stopped(s->mod.handle, reason);
    }
}

// Runs the function under nargs arguments on s->T with a fresh budget; false (and the mod stopped)
// on an error. Calls nest, each with its own budget and the outer one's put back after.
bool call(State* s, int nargs, int nresults, const std::string& where) {
    lua_State* T = s->T;
    const int base = lua_gettop(T) - nargs;  // the function's index
    lua_pushcfunction(T, traceback, "traceback");
    lua_insert(T, base);
    const Budget saved = s->budget;
    const std::string saved_where = s->where;
    const char* saved_running = g_running;
    s->budget = Budget{0.0, Clock::now(), true};
    s->where = where;
    g_running = s->mod.id.c_str();
    const int status = lua_pcall(T, nargs, nresults, base);
    g_running = saved_running;
    s->budget = saved;
    s->where = saved_where;
    lua_remove(T, base);
    if (status != LUA_OK) {
        const char* message = lua_tostring(T, -1);
        std::string text = message != nullptr ? message : "an error";
        lua_pop(T, 1);
        const std::string first = text.substr(0, text.find('\n'));
        log_line(s, OPENRAC_LOG_ERROR, std::format("script error in {}: {}", where, text));
        stop(s, std::format("{}: {}", where, first));
        return false;
    }
    return true;
}

// ---- Values ----

void push_value(lua_State* L, std::uint8_t kind, const openrac_script_value& v) {
    if (kind == OPENRAC_SV_F32) {
        lua_pushnumber(L, v.f);
    } else {
        lua_pushnumber(L, static_cast<double>(v.i));
    }
}

openrac_script_value to_value(lua_State* L, int idx, std::uint8_t kind) {
    openrac_script_value v{0, 0.0};
    const double n = luaL_checknumber(L, idx);
    if (kind == OPENRAC_SV_F32) {
        v.f = n;
    } else if (kind == OPENRAC_SV_U64 && n >= 9.2e18) {
        v.i = static_cast<std::int64_t>(static_cast<std::uint64_t>(n));  // above int64's range
    } else {
        v.i = static_cast<std::int64_t>(n);
    }
    return v;
}

std::size_t kind_size(std::uint8_t kind) {
    switch (kind) {
        case OPENRAC_SV_I8:
        case OPENRAC_SV_U8:
            return 1;
        case OPENRAC_SV_I16:
        case OPENRAC_SV_U16:
            return 2;
        case OPENRAC_SV_I64:
        case OPENRAC_SV_U64:
            return 8;
        default:
            return 4;
    }
}

bool kind_named(const char* name, std::uint8_t& kind) {
    static const std::pair<const char*, std::uint8_t> kinds[] = {
        {"u8", OPENRAC_SV_U8},   {"i8", OPENRAC_SV_I8},   {"u16", OPENRAC_SV_U16}, {"i16", OPENRAC_SV_I16},
        {"u32", OPENRAC_SV_U32}, {"i32", OPENRAC_SV_I32}, {"f32", OPENRAC_SV_F32},
    };
    for (const auto& [n, k] : kinds) {
        if (std::strcmp(n, name) == 0) {
            kind = k;
            return true;
        }
    }
    return false;
}

void check_address(lua_State* L, std::uint64_t address, std::size_t size) {
    if (address + size > kRamSize) {
        luaL_error(L, "address 0x%llx is outside game RAM", static_cast<unsigned long long>(address));
    }
}

void push_memory(lua_State* L, std::uint8_t kind, std::uint32_t address) {
    check_address(L, address, kind_size(kind));
    const std::uint8_t* p = openrac_guest_base + address;
    switch (kind) {
        case OPENRAC_SV_U8:
            lua_pushnumber(L, *p);
            break;
        case OPENRAC_SV_I8:
            lua_pushnumber(L, static_cast<std::int8_t>(*p));
            break;
        case OPENRAC_SV_U16: {
            std::uint16_t v;
            std::memcpy(&v, p, 2);
            lua_pushnumber(L, v);
            break;
        }
        case OPENRAC_SV_I16: {
            std::int16_t v;
            std::memcpy(&v, p, 2);
            lua_pushnumber(L, v);
            break;
        }
        case OPENRAC_SV_U32: {
            std::uint32_t v;
            std::memcpy(&v, p, 4);
            lua_pushnumber(L, v);
            break;
        }
        case OPENRAC_SV_I32: {
            std::int32_t v;
            std::memcpy(&v, p, 4);
            lua_pushnumber(L, v);
            break;
        }
        default: {
            float v;
            std::memcpy(&v, p, 4);
            lua_pushnumber(L, v);
            break;
        }
    }
}

void write_memory(lua_State* L, std::uint8_t kind, std::uint32_t address, double value) {
    check_address(L, address, kind_size(kind));
    std::uint8_t* p = openrac_guest_base + address;
    if (kind == OPENRAC_SV_F32) {
        const float v = static_cast<float>(value);
        std::memcpy(p, &v, 4);
        return;
    }
    const auto n = static_cast<std::int64_t>(value);
    const auto u = static_cast<std::uint32_t>(n);
    std::memcpy(p, &u, kind_size(kind));  // little-endian, as the console and every host of the port
}

// ---- openrac.fields and openrac.pvars ----

// A node of the fields' tree: a leaf is one field, the rest tables of them.
struct FieldNode {
    std::map<std::string, std::unique_ptr<FieldNode>> children;
    const openrac_script_field* field = nullptr;
};

struct FieldTree {
    FieldNode root;
};

std::map<const openrac_script_binding*, std::unique_ptr<FieldTree>> g_trees;

FieldNode* tree_of(const openrac_script_binding* b) {
    auto& t = g_trees[b];
    if (!t) {
        t = std::make_unique<FieldTree>();
        for (int i = 0; i < b->field_count; ++i) {
            const openrac_script_field& f = b->fields[i];
            FieldNode* node = &t->root;
            std::string_view path = f.path;
            while (!path.empty()) {
                const std::size_t dot = path.find('.');
                auto& child = node->children[std::string(path.substr(0, dot))];
                if (!child) {
                    child = std::make_unique<FieldNode>();
                }
                node = child.get();
                path = dot == std::string_view::npos ? std::string_view() : path.substr(dot + 1);
            }
            node->field = &f;
        }
    }
    return &t->root;
}

void push_node(lua_State* L, FieldNode* node) {
    *static_cast<FieldNode**>(lua_newuserdata(L, sizeof(FieldNode*))) = node;
    luaL_getmetatable(L, "openrac.fields");
    lua_setmetatable(L, -2);
}

std::uint32_t field_address(lua_State* L, State* s, const openrac_script_field* f) {
    const std::uint32_t a = s->disc >= 0 ? f->addresses[s->disc] : 0;
    if (a == 0) {
        luaL_error(L, "field %s is not on this disc", f->path);
    }
    return a;
}

int fields_index(lua_State* L) {
    FieldNode* node = *static_cast<FieldNode**>(luaL_checkudata(L, 1, "openrac.fields"));
    const char* key = luaL_checkstring(L, 2);
    const auto it = node->children.find(key);
    if (it == node->children.end()) {
        luaL_error(L, "no field %s", key);
    }
    FieldNode* child = it->second.get();
    if (child->field != nullptr) {
        push_memory(L, child->field->kind, field_address(L, state_of(L), child->field));
    } else {
        push_node(L, child);
    }
    return 1;
}

int fields_newindex(lua_State* L) {
    FieldNode* node = *static_cast<FieldNode**>(luaL_checkudata(L, 1, "openrac.fields"));
    const char* key = luaL_checkstring(L, 2);
    const auto it = node->children.find(key);
    if (it == node->children.end() || it->second->field == nullptr) {
        luaL_error(L, "no field %s to set", key);
    }
    const openrac_script_field* f = it->second->field;
    if (f->readonly) {
        luaL_error(L, "field %s is read-only", f->path);
    }
    write_memory(L, f->kind, field_address(L, state_of(L), f), luaL_checknumber(L, 3));
    return 0;
}

struct Pvars {
    const openrac_script_class* cls;
    std::uint32_t base;
};

const openrac_script_pvar* pvar_named(lua_State* L, Pvars* p, const char* key) {
    for (int i = 0; i < p->cls->pvar_count; ++i) {
        if (std::strcmp(p->cls->pvars[i].name, key) == 0) {
            return &p->cls->pvars[i];
        }
    }
    luaL_error(L, "a %s has no pvar %s", p->cls->name, key);
    return nullptr;
}

int pvars_index(lua_State* L) {
    auto* p = static_cast<Pvars*>(luaL_checkudata(L, 1, "openrac.pvars"));
    const char* key = luaL_checkstring(L, 2);
    if (std::strcmp(key, "class") == 0) {
        lua_pushstring(L, p->cls->name);
        return 1;
    }
    const openrac_script_pvar* v = pvar_named(L, p, key);
    push_memory(L, v->kind, p->base + v->offset);
    return 1;
}

int pvars_newindex(lua_State* L) {
    auto* p = static_cast<Pvars*>(luaL_checkudata(L, 1, "openrac.pvars"));
    const openrac_script_pvar* v = pvar_named(L, p, luaL_checkstring(L, 2));
    if (v->readonly) {
        luaL_error(L, "pvar %s of a %s is read-only", v->name, p->cls->name);
    }
    write_memory(L, v->kind, p->base + v->offset, luaL_checknumber(L, 3));
    return 0;
}

// openrac.pvars(moby): its class's pvars by name, or nil (a class the catalogue does not have).
int api_pvars(lua_State* L) {
    State* s = state_of(L);
    const openrac_script_moby* m = s->binding->moby;
    const auto moby = static_cast<std::uint32_t>(luaL_checknumber(L, 1));
    if (m == nullptr || moby == 0) {
        lua_pushnil(L);
        return 1;
    }
    check_address(L, moby, std::max(m->class_offset, m->pvars_offset) + 4u);
    push_memory(L, m->class_kind, moby + m->class_offset);
    const auto number = static_cast<std::int32_t>(lua_tonumber(L, -1));
    lua_pop(L, 1);
    for (int i = 0; i < m->class_count; ++i) {
        if (m->classes[i].number == number) {
            std::uint32_t base;
            std::memcpy(&base, openrac_guest_base + moby + m->pvars_offset, 4);
            auto* p = static_cast<Pvars*>(lua_newuserdata(L, sizeof(Pvars)));
            *p = {&m->classes[i], base};
            luaL_getmetatable(L, "openrac.pvars");
            lua_setmetatable(L, -2);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

// ---- The hook points ----

int call_original(lua_State* L) {
    auto* l = static_cast<Link*>(lua_tolightuserdata(L, lua_upvalueindex(1)));
    if (l == nullptr || l->args == nullptr) {
        luaL_error(L, "original() is for the hook's call only");
    }
    const openrac_script_point* p = l->point;
    std::vector<openrac_script_value> args(static_cast<std::size_t>(std::max(1, p->arg_count)));
    if (lua_gettop(L) == 0) {
        std::copy(l->args, l->args + p->arg_count, args.begin());  // original(): the call's own
    } else {
        for (int i = 0; i < p->arg_count; ++i) {
            args[static_cast<std::size_t>(i)] = to_value(L, i + 1, p->arg_kinds[i]);
        }
    }
    State* s = l->state;
    // The game's own time is not the script's.
    Budget& b = s->budget;
    b.used = elapsed(b);
    b.running = false;
    const char* saved_running = g_running;
    g_running = nullptr;
    openrac_script_value r{0, 0.0};
    p->call(l->link->original, args.data(), &r);
    g_running = saved_running;
    b.start = Clock::now();
    b.running = true;
    l->called = true;
    l->result = r;
    if (p->result_kind == OPENRAC_SV_VOID) {
        return 0;
    }
    push_value(L, p->result_kind, r);
    return 1;
}

int take_hook(lua_State* L, Link::Mode mode) {
    State* s = state_of(L);
    const char* name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    if (!s->loading) {
        luaL_error(L, "hooks are taken while the mod's scripts first run");
    }
    const openrac_script_point* point = nullptr;
    // By its id or any alias: the binding's points are named by their ids.
    const char* id = openrac_script_hook_id(s->binding, name);
    for (int i = 0; i < s->binding->point_count; ++i) {
        if (std::strcmp(s->binding->points[i].name, id) == 0) {
            point = &s->binding->points[i];
        }
    }
    if (point == nullptr) {
        luaL_error(L, "there is no hook point %s for %s", name, s->binding->game);
    }
    int free_link = -1;
    for (int k = 0; k < OPENRAC_SCRIPT_LINKS; ++k) {
        if (point->links[k].runtime == nullptr) {
            free_link = k;
            break;
        }
    }
    if (free_link < 0) {
        luaL_error(L, "hook point %s already has %d script mods on it", name, OPENRAC_SCRIPT_LINKS);
    }
    auto link = std::make_unique<Link>();
    link->state = s;
    link->point = point;
    link->link = &point->links[free_link];
    link->mode = mode;
    lua_pushvalue(L, 2);
    link->fn = lua_ref(L, -1);
    lua_pop(L, 1);
    void* original = nullptr;
    const int result = s->services.hook(s->mod.handle, point->name, point->signature,
                                        point->thunks[free_link], &original);
    if (result != OPENRAC_HOOK_OK) {
        lua_unref(L, link->fn);
        luaL_error(L, "hook point %s refused (%d)", name, result);
    }
    link->link->original = original;
    link->link->runtime = link.get();
    s->links.push_back(std::move(link));
    return 0;
}

int api_hook(lua_State* L) {
    return take_hook(L, Link::Mode::Wrap);
}
int api_before(lua_State* L) {
    return take_hook(L, Link::Mode::Before);
}
int api_after(lua_State* L) {
    return take_hook(L, Link::Mode::After);
}

// ---- Callbacks ----

void frame_callback(void* user) {
    auto* c = static_cast<Callback*>(user);
    State* s = c->state;
    if (s->stopped) {
        return;
    }
    lua_getref(s->T, c->fn);
    call(s, 0, 0, "on_frame");
}

void pad_callback(void* user, int port, std::uint8_t* report, int size) {
    auto* c = static_cast<Callback*>(user);
    State* s = c->state;
    if (s->stopped || size <= 0) {
        return;
    }
    lua_State* T = s->T;
    lua_getref(T, c->fn);
    lua_pushnumber(T, port);
    void* buffer = lua_newbuffer(T, static_cast<std::size_t>(size));
    std::memcpy(buffer, report, static_cast<std::size_t>(size));
    lua_pushvalue(T, -1);
    const int ref = lua_ref(T, -1);  // kept to read back after the call
    lua_pop(T, 1);
    if (call(s, 2, 0, "on_pad")) {
        lua_getref(T, ref);
        std::size_t len = 0;
        if (void* after = lua_tobuffer(T, -1, &len); after != nullptr && len == static_cast<std::size_t>(size)) {
            std::memcpy(report, after, len);
        }
        lua_pop(T, 1);
    }
    lua_unref(T, ref);
}

int add_callback(lua_State* L, bool pad) {
    State* s = state_of(L);
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (!s->loading) {
        luaL_error(L, "callbacks are set while the mod's scripts first run");
    }
    auto c = std::make_unique<Callback>();
    c->state = s;
    lua_pushvalue(L, 1);
    c->fn = lua_ref(L, -1);
    lua_pop(L, 1);
    const int result = pad ? s->services.on_pad(s->mod.handle, pad_callback, c.get())
                           : s->services.on_frame(s->mod.handle, frame_callback, c.get());
    if (result != OPENRAC_HOOK_OK) {
        luaL_error(L, "the callback was refused (%d)", result);
    }
    s->callbacks.push_back(std::move(c));
    return 0;
}

int api_on_frame(lua_State* L) {
    return add_callback(L, false);
}
int api_on_pad(lua_State* L) {
    return add_callback(L, true);
}

// ---- The rest of openrac ----

std::string joined(lua_State* L) {
    std::string out;
    for (int i = 1; i <= lua_gettop(L); ++i) {
        std::size_t len = 0;
        const char* s = luaL_tolstring(L, i, &len);
        out += (i == 1 ? "" : " ") + std::string(s, len);
        lua_pop(L, 1);
    }
    return out;
}

int api_log(lua_State* L) {
    log_line(state_of(L), OPENRAC_LOG_INFO, joined(L));
    return 0;
}

int api_warn(lua_State* L) {
    log_line(state_of(L), OPENRAC_LOG_WARN, joined(L));
    return 0;
}

int api_setting(lua_State* L) {
    State* s = state_of(L);
    const auto it = s->mod.settings.find(luaL_checkstring(L, 1));
    if (it == s->mod.settings.end()) {
        lua_pushnil(L);
        return 1;
    }
    const SettingValue& v = it->second;
    switch (v.type) {
        case SettingType::Bool:
            lua_pushboolean(L, v.b);
            break;
        case SettingType::Int:
            lua_pushnumber(L, static_cast<double>(v.i));
            break;
        case SettingType::Float:
            lua_pushnumber(L, v.f);
            break;
        default:
            lua_pushlstring(L, v.s.data(), v.s.size());
            break;
    }
    return 1;
}

// ---- The keyboard and mouse (interface 1.2) ----

const openrac_input* input_of(lua_State* L) {
    State* s = state_of(L);
    return s->services.input != nullptr ? s->services.input() : nullptr;
}

int bind_named(lua_State* L) {
    std::size_t len = 0;
    const char* name = luaL_checklstring(L, 1, &len);
    const int bind = openrac_bind_from_name_n(name, len);
    if (bind == OPENRAC_BIND_NONE) {
        luaL_error(L, "no key, mouse button or wheel direction is named \"%s\"", name);
    }
    return bind;
}

int api_key(lua_State* L) {
    const int bind = bind_named(L);
    lua_pushboolean(L, openrac_bind_held(input_of(L), bind));
    return 1;
}

int api_pressed(lua_State* L) {
    const int bind = bind_named(L);
    lua_pushboolean(L, openrac_bind_pressed(input_of(L), bind));
    return 1;
}

int api_mouse(lua_State* L) {
    const openrac_input* in = input_of(L);
    lua_pushnumber(L, in != nullptr ? in->mouse_dx : 0.0);
    lua_pushnumber(L, in != nullptr ? in->mouse_dy : 0.0);
    lua_pushnumber(L, in != nullptr ? in->wheel_x : 0.0);
    lua_pushnumber(L, in != nullptr ? in->wheel_y : 0.0);
    return 4;
}

int api_window(lua_State* L) {
    const openrac_input* in = input_of(L);
    lua_pushboolean(L, in != nullptr && in->focused != 0);
    lua_pushboolean(L, in != nullptr && in->captured != 0);
    lua_pushboolean(L, in != nullptr && in->overlay != 0);
    return 3;
}

int api_pointer(lua_State* L) {
    const openrac_input* in = input_of(L);
    const bool has = in != nullptr && in->size >= offsetof(openrac_input, reserved2);
    lua_pushnumber(L, has ? in->pointer_x : 0.0);
    lua_pushnumber(L, has ? in->pointer_y : 0.0);
    lua_pushboolean(L, has && in->pointer_inside != 0);
    return 3;
}

int api_take_input(lua_State* L) {
    State* s = state_of(L);
    if (!s->loading) {
        luaL_error(L, "take_input is called while the mod's scripts first run");
    }
    uint32_t devices = 0;
    for (int i = 1; i <= lua_gettop(L); ++i) {
        const std::string_view d = luaL_checkstring(L, i);
        if (d == "keyboard") {
            devices |= OPENRAC_INPUT_KEYBOARD;
        } else if (d == "mouse") {
            devices |= OPENRAC_INPUT_MOUSE;
        } else {
            luaL_error(L, "take_input: \"keyboard\" or \"mouse\"");
        }
    }
    if (s->services.take_input == nullptr || s->services.take_input(s->mod.handle, devices) != OPENRAC_HOOK_OK) {
        luaL_error(L, "take_input was refused");
    }
    return 0;
}

int api_capture_mouse(lua_State* L) {
    State* s = state_of(L);
    if (s->services.capture_mouse != nullptr) {
        s->services.capture_mouse(s->mod.handle, lua_toboolean(L, 1) ? 1 : 0);
    }
    return 0;
}

int api_read(lua_State* L) {
    std::uint8_t kind = 0;
    if (!kind_named(luaL_checkstring(L, 1), kind)) {
        luaL_error(L, "read: the type is one of u8 i8 u16 i16 u32 i32 f32");
    }
    const double a = luaL_checknumber(L, 2);
    if (a < 0 || a >= kRamSize) {
        luaL_error(L, "read: address %f is outside game RAM", a);
    }
    push_memory(L, kind, static_cast<std::uint32_t>(a));
    return 1;
}

int api_write(lua_State* L) {
    std::uint8_t kind = 0;
    if (!kind_named(luaL_checkstring(L, 1), kind)) {
        luaL_error(L, "write: the type is one of u8 i8 u16 i16 u32 i32 f32");
    }
    const double a = luaL_checknumber(L, 2);
    if (a < 0 || a >= kRamSize) {
        luaL_error(L, "write: address %f is outside game RAM", a);
    }
    write_memory(L, kind, static_cast<std::uint32_t>(a), luaL_checknumber(L, 3));
    return 0;
}

bool read_file(const fs::path& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return false;
    }
    std::ostringstream s;
    s << f.rdbuf();
    out = s.str();
    return true;
}

// Compiles and loads one of the mod's files onto T's stack as a function; false and the message there.
bool load_chunk(State* s, lua_State* T, const std::string& rel) {
    std::string source;
    if (!read_file(s->mod.folder / fs::path(rel), source)) {
        lua_pushstring(T, std::format("{} cannot be read", rel).c_str());
        return false;
    }
    std::size_t size = 0;
    char* bytecode = luau_compile(source.data(), source.size(), nullptr, &size);
    const std::string chunk = "=" + s->mod.id + "/" + rel;
    const int status = luau_load(T, chunk.c_str(), bytecode, size, 0);
    std::free(bytecode);
    return status == 0;
}

// require(name): another .luau file of the mod's own folder ("util", "lib/util"), run once.
int api_require(lua_State* L) {
    State* s = state_of(L);
    std::string name = luaL_checkstring(L, 1);
    if (!name.ends_with(".luau")) {
        name += ".luau";
    }
    if (!script_path(name)) {
        luaL_error(L, "require: %s is not a file of the mod's own folder", name.c_str());
    }
    lua_getfield(L, LUA_REGISTRYINDEX, "openrac.modules");
    lua_getfield(L, -1, name.c_str());
    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 1);
    if (!load_chunk(s, L, name)) {
        lua_error(L);
    }
    lua_call(L, 0, 1);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushboolean(L, 1);
    }
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, name.c_str());
    return 1;
}

void set_function(lua_State* L, const char* name, lua_CFunction fn) {
    lua_pushcfunction(L, fn, name);
    lua_setfield(L, -2, name);
}

void set_string(lua_State* L, const char* name, const std::string& value) {
    lua_pushlstring(L, value.data(), value.size());
    lua_setfield(L, -2, name);
}

// The standard library a script keeps (MODS.md, section 11): no getfenv or setfenv, of os only
// clock, of debug only traceback; print goes to the log.
void restrict_library(lua_State* L) {
    lua_pushnil(L);
    lua_setglobal(L, "getfenv");
    lua_pushnil(L);
    lua_setglobal(L, "setfenv");
    lua_pushnil(L);
    lua_setglobal(L, "loadstring");
    lua_pushnil(L);
    lua_setglobal(L, "dofile");
    lua_pushnil(L);
    lua_setglobal(L, "load");
    lua_getglobal(L, "os");
    lua_getfield(L, -1, "clock");
    lua_newtable(L);
    lua_insert(L, -2);
    lua_setfield(L, -2, "clock");
    lua_setglobal(L, "os");
    lua_pop(L, 1);
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_newtable(L);
    lua_insert(L, -2);
    lua_setfield(L, -2, "traceback");
    lua_setglobal(L, "debug");
    lua_pop(L, 1);
    lua_pushcfunction(L, api_log, "print");
    lua_setglobal(L, "print");
    lua_pushcfunction(L, api_require, "require");
    lua_setglobal(L, "require");
}

void push_openrac(lua_State* L, State* s) {
    lua_newtable(L);
    set_string(L, "game", s->mod.game);
    set_string(L, "version", s->mod.version);
    set_string(L, "platform", s->mod.platform);
    set_string(L, "api", s->mod.api);
    set_function(L, "log", api_log);
    set_function(L, "warn", api_warn);
    set_function(L, "setting", api_setting);
    set_function(L, "hook", api_hook);
    set_function(L, "before", api_before);
    set_function(L, "after", api_after);
    set_function(L, "on_frame", api_on_frame);
    set_function(L, "on_pad", api_on_pad);
    set_function(L, "read", api_read);
    set_function(L, "write", api_write);
    set_function(L, "key", api_key);
    set_function(L, "pressed", api_pressed);
    set_function(L, "mouse", api_mouse);
    set_function(L, "window", api_window);
    set_function(L, "pointer", api_pointer);
    set_function(L, "take_input", api_take_input);
    set_function(L, "capture_mouse", api_capture_mouse);
    if (s->binding->moby != nullptr) {
        set_function(L, "pvars", api_pvars);
    }
    push_node(L, tree_of(s->binding));
    lua_setfield(L, -2, "fields");
}

void make_metatables(lua_State* L) {
    luaL_newmetatable(L, "openrac.fields");
    lua_pushcfunction(L, fields_index, "fields.__index");
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, fields_newindex, "fields.__newindex");
    lua_setfield(L, -2, "__newindex");
    lua_pushstring(L, "the game's fields");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newmetatable(L, "openrac.pvars");
    lua_pushcfunction(L, pvars_index, "pvars.__index");
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, pvars_newindex, "pvars.__newindex");
    lua_setfield(L, -2, "__newindex");
    lua_pushstring(L, "a moby's pvars");
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "openrac.modules");
}

void free_links(State* s) {
    for (const auto& l : s->links) {
        if (l->link != nullptr && l->link->runtime == l.get()) {
            l->link->runtime = nullptr;
            l->link->original = nullptr;
        }
    }
    s->links.clear();
}

void close_state(State* s) {
    free_links(s);
    s->callbacks.clear();
    if (s->L != nullptr) {
        lua_close(s->L);
        s->L = nullptr;
        s->T = nullptr;
    }
}

}  // namespace

bool load(const Mod& mod, const openrac_script_binding* binding, const Services& services, std::string& reason) {
    if (binding == nullptr) {
        reason = "this OpenRAC runs no scripts for this game";
        return false;
    }
    auto s = std::make_unique<State>();
    s->mod = mod;
    s->services = services;
    s->binding = binding;
    for (int i = 0; i < binding->disc_count; ++i) {
        if (mod.version == binding->discs[i]) {
            s->disc = i;
        }
    }
    State* st = s.get();
    g_states.push_back(std::move(s));
    crash::set_running_script(&g_running);
    lua_State* L = luaL_newstate();
    st->L = L;
    lua_callbacks(L)->userdata = st;
    lua_callbacks(L)->interrupt = interrupt;
    luaL_openlibs(L);
    restrict_library(L);
    make_metatables(L);
    push_openrac(L, st);
    lua_setglobal(L, "openrac");
    luaL_sandbox(L);
    st->T = lua_newthread(L);
    st->thread_ref = lua_ref(L, -1);
    lua_pop(L, 1);
    luaL_sandboxthread(st->T);
    st->loading = true;
    bool ok = true;
    for (const std::string& script : mod.scripts) {
        if (!load_chunk(st, st->T, script)) {
            const char* message = lua_tostring(st->T, -1);
            reason = std::format("{} does not compile: {}", script, message != nullptr ? message : "");
            lua_pop(st->T, 1);
            log_line(st, OPENRAC_LOG_ERROR, reason);
            ok = false;
            break;
        }
        if (!call(st, 0, 0, script)) {
            reason = st->stop_reason;
            ok = false;
            break;
        }
    }
    st->loading = false;
    if (!ok) {
        close(mod.handle);
        return false;
    }
    return true;
}

void close(openrac_mod* handle) {
    for (auto it = g_states.begin(); it != g_states.end(); ++it) {
        if ((*it)->mod.handle == handle) {
            close_state(it->get());
            g_states.erase(it);
            return;
        }
    }
}

void reset() {
    for (auto& s : g_states) {
        close_state(s.get());
    }
    g_states.clear();
    g_running = nullptr;
}

int state_count() {
    int n = 0;
    for (const auto& s : g_states) {
        n += s->L != nullptr ? 1 : 0;
    }
    return n;
}

void set_budget_ms(double ms) {
    g_budget_ms = ms;
}

namespace {

// A link's call under way (Link's args, result and called), put back as it was when a call of the
// link ends, by any way out, an exception's included: calls nest, and the outer call goes on with
// its own.
class SavedCall {
public:
    explicit SavedCall(Link& link)
        : m_link(&link),
          m_args(link.args),
          m_result(link.result),
          m_called(link.called) {}

    ~SavedCall() {
        m_link->args = m_args;
        m_link->result = m_result;
        m_link->called = m_called;
    }

    SavedCall(const SavedCall&) = delete;
    SavedCall& operator=(const SavedCall&) = delete;
    SavedCall(SavedCall&&) = delete;
    SavedCall& operator=(SavedCall&&) = delete;

private:
    Link* m_link;
    const openrac_script_value* m_args;
    openrac_script_value m_result;
    bool m_called;
};

// openrac_script_dispatch's body: runs the script of `link` for one call of its hook point. May
// throw (a hook point's call, Luau outside a protected call, an allocation); the entry point stops
// it there.
int dispatch(
    const openrac_script_point* point,
    openrac_script_link* link,
    const openrac_script_value* args,
    openrac_script_value* result
) {
    auto* l = static_cast<Link*>(link->runtime);
    if (l == nullptr || l->state->stopped || l->state->T == nullptr) {
        return 0;
    }
    State* s = l->state;
    lua_State* T = s->T;
    // Calls nest: what the outer call of this link had is put back after.
    const SavedCall saved(*l);
    l->args = args;
    l->called = false;
    l->result = openrac_script_value{0, 0.0};
    const std::string where = std::format("hook {}", point->name);
    bool handled = true;
    if (l->mode == Link::Mode::After) {
        point->call(link->original, args, result);
        l->called = true;
    }
    lua_getref(T, l->fn);
    int nargs = point->arg_count;
    if (l->mode == Link::Mode::Wrap) {
        lua_pushlightuserdata(T, l);
        lua_pushcclosurek(T, call_original, "original", 1, nullptr);
        ++nargs;
    }
    for (int i = 0; i < point->arg_count; ++i) {
        push_value(T, point->arg_kinds[i], args[i]);
    }
    const int nresults = l->mode == Link::Mode::Wrap && point->result_kind != OPENRAC_SV_VOID ? 1 : 0;
    const bool ok = call(s, nargs, nresults, where);
    if (l->mode == Link::Mode::Wrap) {
        if (!ok) {
            // A script that stopped before its original ran: the original still runs (the thunk
            // calls it); after it did, its result stands.
            handled = l->called;
            *result = l->result;
        } else if (nresults == 1) {
            if (lua_isnil(T, -1)) {
                *result = l->result;
            } else if (lua_isnumber(T, -1)) {
                *result = point->result_kind == OPENRAC_SV_F32
                              ? openrac_script_value{0, lua_tonumber(T, -1)}
                              : openrac_script_value{static_cast<int64_t>(lua_tonumber(T, -1)), 0.0};
            } else {
                *result = l->result;
            }
            lua_pop(T, 1);
        }
    } else if (l->mode == Link::Mode::Before) {
        point->call(link->original, args, result);
    }
    return handled ? 1 : 0;
}

}  // namespace

}  // namespace openrac::mods::scripts

// The runtime's side of every link's function (openrac/script_binding.h). An exception from the
// call is reported to the log, and the link's original is called as it is (0): the contract's
// value for no script there (CODING_CONVENTIONS.md, section 13.4).
extern "C" int openrac_script_dispatch(
    const openrac_script_point* point,
    openrac_script_link* link,
    const openrac_script_value* args,
    openrac_script_value* result
) {
    return openrac::c_boundary::call("openrac_script_dispatch", 0, [&] {
        return openrac::mods::scripts::dispatch(point, link, args, result);
    });
}

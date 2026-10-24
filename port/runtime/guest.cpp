// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The runtime side of guest.h: the game stack, string literals, the table of
// functions by code address, and the list of functions the game called that
// have no C yet.

#include "openrac/guest.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <execinfo.h>
#endif

#include "common/env.h"
#include "common/log.h"
#include "openrac/memory.h"

extern "C" {
gaddr openrac_guest_sp = GUEST_STACK_TOP;
}

namespace {

using openrac::log::error;
using openrac::log::fatalf;
using openrac::log::info;
using openrac::log::warn;

// ---- String literals ----

std::mutex g_strings_lock;
std::unordered_map<std::string, gaddr> g_strings;
gaddr g_strings_next = GUEST_STRINGS;

// ---- Functions by address ----

struct Entry {
    openrac_host_fn fn;
    const char* name;
};

using Table = std::unordered_map<gaddr, Entry>;

Table g_exe;
std::map<int, Table> g_overlays;
// Functions registered at two or more addresses of one program: tiny functions the catalogue
// folded into one name (their bytes are the same but for the address they call), each place a
// different function. A call through one of their addresses goes where its code calls.
std::unordered_map<openrac_host_fn, int> g_ambiguous;
// Each game function's thunk (openrac_guest_call_regs), by its host function.
std::unordered_map<openrac_host_fn, openrac_regs_thunk> g_thunks;
int g_overlay = OPENRAC_OVERLAY_EXE;
thread_local gaddr g_unknown_target = 0;

bool strict() {
    return openrac::env::flag(openrac::env::Var::Strict);
}

int (*g_overlay_source)(void) = nullptr;

const Entry* find(gaddr address) {
    if (g_overlay_source != nullptr) {
        g_overlay = g_overlay_source();
    }
    if (g_overlay != OPENRAC_OVERLAY_EXE) {
        auto level = g_overlays.find(g_overlay);
        if (level != g_overlays.end()) {
            auto it = level->second.find(address);
            if (it != level->second.end()) {
                return &it->second;
            }
        }
    }
    auto it = g_exe.find(address);
    return it == g_exe.end() ? nullptr : &it->second;
}

// What a call through an unknown code address runs: it logs and returns, so
// that bring-up can go on (OPENRAC_STRICT=1 stops instead).
// What a callback that is only `jr ra` does: nothing, with 0 in v0.
std::uint64_t returns_nothing() {
    return 0;
}

void unknown_function() {
    static std::mutex lock;
    static std::map<gaddr, int> seen;
    std::scoped_lock hold(lock);
    const int calls = ++seen[g_unknown_target];
    // [verbose log] It goes on being called: said again at 10, 100, 1000... calls.
    if (calls >= 10 && openrac::log::verbose_on()) {
        int n = calls;
        while (n % 10 == 0) {
            n /= 10;
        }
        if (n == 1) {
            openrac::log::verbose("call to code address {:#010x} again: {} calls (overlay {})", g_unknown_target,
                                  calls, g_overlay);
        }
    }
    if (calls == 1) {
        error(
            "call to code address {:#010x}, which no function of the port has (overlay {})",
            g_unknown_target,
            g_overlay
        );
        // Who made the call: the translated functions above this one.
        for (int depth = 0; depth < 4; ++depth) {
            char who[256];
            openrac_host_caller(depth, who, sizeof(who));
            error("  from {}", who);
        }
    }
}

// ---- Missing functions ----

std::mutex g_missing_lock;
std::map<std::string, unsigned long> g_missing;
bool g_stop_on_missing = false;
// The sites hostgen's C counts in (guest.h, openrac_reach_site), each once called; a stub's by
// name too, since its calls are on the site from the second on (g_missing has the first).
openrac_reach_site* g_reached = nullptr;
std::map<std::string, const openrac_reach_site*> g_stub_sites;

// A function's calls without C: openrac_guest_missing's count, and its stub's site's after the
// first (which openrac_guest_missing counted).
unsigned long missing_count(const std::string& name, unsigned long direct) {
    const auto it = g_stub_sites.find(name);
    return it == g_stub_sites.end() ? direct : direct - 1 + it->second->calls;
}

void json_string(std::FILE* f, const char* s) {
    std::fputc('"', f);
    for (; s != nullptr && *s != 0; ++s) {
        if (*s == '"' || *s == '\\') {
            std::fputc('\\', f);
        }
        std::fputc(static_cast<unsigned char>(*s) < 0x20 ? ' ' : *s, f);
    }
    std::fputc('"', f);
}

}  // namespace

extern "C" {

gaddr openrac_guest_frame(uint32_t size) {
    const gaddr sp = (openrac_guest_sp - size) & ~0xFu;
    if (sp < GUEST_STACK_BOTTOM || sp > openrac_guest_sp) {
        fatalf("the game stack is full ({} bytes asked)", size);
    }
    openrac_guest_sp = sp;
    return sp;
}

gaddr openrac_guest_static(uint32_t size) {
    static std::mutex lock;
    static gaddr next = GUEST_STATICS;
    std::scoped_lock hold(lock);
    const gaddr at = next;
    if (at + size > GUEST_STATICS + GUEST_STATICS_SIZE) {
        fatalf("the statics region is full");
    }
    next = (at + size + 15) & ~15u;
    return at;
}

gaddr openrac_guest_string(const char* s, size_t size) {
    std::scoped_lock hold(g_strings_lock);
    std::string key(s, size);
    auto it = g_strings.find(key);
    if (it != g_strings.end()) {
        return it->second;
    }
    const gaddr at = g_strings_next;
    if (at + size > GUEST_STRINGS + GUEST_STRINGS_SIZE) {
        fatalf("the string region is full");
    }
    std::memcpy(G(at), s, size);
    g_strings_next = (at + static_cast<gaddr>(size) + 3) & ~3u;
    g_strings.emplace(std::move(key), at);
    return at;
}

void openrac_guest_register(int overlay, const openrac_fn_entry* entries, size_t count) {
    Table& table = overlay == OPENRAC_OVERLAY_EXE ? g_exe : g_overlays[overlay];
    std::unordered_map<openrac_host_fn, int> places;
    for (size_t i = 0; i < count; i++) {
        if (++places[entries[i].fn] == 2) {
            g_ambiguous[entries[i].fn] = 1;
        }
    }
    for (size_t i = 0; i < count; i++) {
        auto [it, added] = table.emplace(entries[i].address, Entry{entries[i].fn, entries[i].name});
        if (!added && it->second.fn != entries[i].fn) {
            warn(
                "two functions at {:#010x} in overlay {}: {} and {}; keeping the first",
                entries[i].address,
                overlay,
                it->second.name,
                entries[i].name
            );
        }
    }
}

void openrac_guest_register_thunks(const openrac_thunk_entry* entries, size_t count) {
    for (size_t i = 0; i < count; i++) {
        g_thunks.emplace(entries[i].fn, entries[i].thunk);
    }
}

uint64_t openrac_guest_call_regs(gaddr address, const uint64_t* ints, const float* floats, float* result,
                                 int float_result) {
    const openrac_host_fn fn = openrac_guest_function(address);
    *result = 0.0f;
    if (auto it = g_thunks.find(fn); it != g_thunks.end()) {
        return it->second(ints, floats, result);
    }
    // The port's own function (a library replacement, a stub): by the cast, as on System V.
    using F8 = float;
    using U8 = uint64_t;
    if (float_result) {
        *result = reinterpret_cast<float (*)(U8, U8, U8, U8, U8, U8, U8, U8, F8, F8, F8, F8, F8, F8, F8, F8)>(fn)(
            ints[0], ints[1], ints[2], ints[3], ints[4], ints[5], ints[6], ints[7], floats[0], floats[1], floats[2],
            floats[3], floats[4], floats[5], floats[6], floats[7]
        );
        return 0;
    }
    return reinterpret_cast<U8 (*)(U8, U8, U8, U8, U8, U8, U8, U8, F8, F8, F8, F8, F8, F8, F8, F8)>(fn)(
        ints[0], ints[1], ints[2], ints[3], ints[4], ints[5], ints[6], ints[7], floats[0], floats[1], floats[2],
        floats[3], floats[4], floats[5], floats[6], floats[7]
    );
}

void openrac_guest_set_overlay(int overlay) {
    g_overlay = overlay;
}

int openrac_guest_overlay(void) {
    return g_overlay;
}

openrac_host_fn openrac_guest_function(gaddr address) {
    if (const Entry* e = find(address)) {
        // A folded wrapper (addiu sp, -16; sq ra; jal T; nop; lq ra; jr ra; addiu sp, 16): this
        // place's own target, read from the program in memory.
        for (int depth = 0; depth < 4 && g_ambiguous.count(e->fn) != 0; ++depth) {
            std::uint32_t w[7];
            std::memcpy(w, G(address), sizeof(w));
            if (w[0] != 0x27BDFFF0u || w[1] != 0x7FBF0000u || (w[2] >> 26) != 3 || w[3] != 0
                || w[4] != 0x7BBF0000u || w[5] != 0x03E00008u || w[6] != 0x27BD0010u) {
                break;
            }
            const gaddr target = (address & 0xF0000000u) | ((w[2] & 0x03FFFFFFu) << 2);
            const Entry* t = find(target);
            if (t == nullptr) {
                break;
            }
            address = target;
            e = t;
        }
        openrac_guest_last_entry = address;
        openrac_guest_last_fn = e->fn;
        return e->fn;
    }
    // A leaf that only returns (jr ra; and in its delay slot nothing, or v0 = 0): the tail of
    // another function, which the game points at as a callback that does nothing.
    if (address + 8 <= 0x02000000u && (address & 3) == 0) {
        std::uint32_t w[2];
        std::memcpy(w, G(address), sizeof(w));
        const bool returns_zero = w[1] == 0 || w[1] == 0x0000102Du || w[1] == 0x00001025u || w[1] == 0x00001021u;
        if (w[0] == 0x03E00008u && returns_zero) {
            return reinterpret_cast<openrac_host_fn>(&returns_nothing);
        }
    }
    if (strict()) {
        fatalf(
            "call to code address {:#010x}, which no function of the port has (overlay {})",
            address,
            g_overlay
        );
    }
    g_unknown_target = address;
    return unknown_function;
}

gaddr openrac_guest_nearest(const uint32_t* places, int count, gaddr caller) {
    // places: (overlay + 1) << 24 | address; the loaded program's place nearest the caller (the
    // caller and the copy it calls come from one object file, so they sit together).
    const int overlay = g_overlay_source != nullptr ? g_overlay_source() : g_overlay;
    gaddr best = 0;
    gaddr fallback = 0;
    for (int i = 0; i < count; ++i) {
        const int o = static_cast<int>(places[i] >> 24) - 1;
        const gaddr a = places[i] & 0x00FFFFFFu;
        if (fallback == 0 || o == OPENRAC_OVERLAY_EXE) {
            fallback = a;
        }
        if (o == overlay) {
            const auto distance = [caller](gaddr x) { return x > caller ? x - caller : caller - x; };
            if (best == 0 || distance(a) < distance(best)) {
                best = a;
            }
        }
    }
    return best != 0 ? best : fallback;
}

gaddr openrac_folded_call(const uint32_t* places, int count, gaddr caller, uint32_t size, int k) {
    const int overlay = g_overlay_source != nullptr ? g_overlay_source() : g_overlay;
    if (size == 0 || caller == 0 || caller + size > 0x02000000u) {
        return openrac_guest_nearest(places, count, caller);
    }
    static std::mutex lock;
    // The places come in a compound literal (automatic storage): keyed by content, not address.
    static std::map<std::tuple<int, gaddr, std::uint32_t, int, int>, gaddr> found;
    std::scoped_lock hold(lock);
    const auto key = std::make_tuple(overlay, caller, places[0], count, k);
    if (auto it = found.find(key); it != found.end()) {
        return it->second;
    }
    gaddr result = 0;
    int seen = 0;
    gaddr only = 0;  // the one place every call in the caller's code goes to, if they agree
    bool several = false;
    for (std::uint32_t i = 0; i * 4 < size && result == 0; ++i) {
        std::uint32_t w;
        std::memcpy(&w, G(caller + i * 4), 4);
        if ((w >> 26) != 3) {
            continue;
        }
        const gaddr target = (caller & 0xF0000000u) | ((w & 0x03FFFFFFu) << 2);
        for (int p = 0; p < count; ++p) {
            const int o = static_cast<int>(places[p] >> 24) - 1;
            if ((places[p] & 0x00FFFFFFu) == target && (o == overlay || o == OPENRAC_OVERLAY_EXE)) {
                if (seen++ == k) {
                    result = target;
                }
                several = several || (only != 0 && only != target);
                only = target;
                break;
            }
        }
    }
    // The compiler merges the same code at the ends of several branches (cross-jumping), so the
    // caller's code can have fewer calls than its C: a call past the last one goes where they all
    // go when they agree (rac1: level 10's swoop flyer bobs with the place its other calls use).
    if (result == 0 && only != 0 && !several) {
        result = only;
    }
    if (result == 0) {
        result = openrac_guest_nearest(places, count, caller);
    }
    found.emplace(key, result);
    return result;
}

gaddr openrac_folded_address(const uint32_t* places, int count, gaddr caller, uint32_t size, int k,
                             gaddr fallback) {
    const int overlay = g_overlay_source != nullptr ? g_overlay_source() : g_overlay;
    if (size == 0 || caller == 0 || caller + size > 0x02000000u) {
        return fallback;
    }
    static std::mutex lock;
    static std::map<std::tuple<int, gaddr, std::uint32_t, int, int>, gaddr> found;
    std::scoped_lock hold(lock);
    const auto key = std::make_tuple(overlay, caller, places[0], count, k);
    if (auto it = found.find(key); it != found.end()) {
        return it->second;
    }
    // The upper halves the code's lui instructions put in each register, as it reads in order.
    std::uint32_t high[32] = {};
    bool known[32] = {};
    gaddr result = 0;
    gaddr other = 0;      // a function's address the code forms that is none of the places
    bool called = false;  // the code calls one of the places
    int seen = 0;
    for (std::uint32_t i = 0; i * 4 < size && result == 0; ++i) {
        std::uint32_t w;
        std::memcpy(&w, G(caller + i * 4), 4);
        const std::uint32_t op = w >> 26;
        const std::uint32_t rs = (w >> 21) & 31;
        const std::uint32_t rt = (w >> 16) & 31;
        if (op == 0x03 && !called) {  // jal: a C calling through the address calls one of them
            const gaddr target = (caller & 0xF0000000u) | ((w & 0x03FFFFFFu) << 2);
            for (int p = 0; p < count && !called; ++p) {
                called = (places[p] & 0x00FFFFFFu) == target;
            }
            continue;
        }
        if (op == 0x0F) {  // lui
            high[rt] = (w & 0xFFFFu) << 16;
            known[rt] = true;
            continue;
        }
        if ((op != 0x09 && op != 0x0D) || !known[rs]) {  // addiu, ori
            continue;
        }
        const gaddr formed = op == 0x09 ? high[rs] + static_cast<std::uint32_t>(static_cast<std::int16_t>(w & 0xFFFFu))
                                        : high[rs] | (w & 0xFFFFu);
        bool family = false;
        for (int p = 0; p < count; ++p) {
            const int o = static_cast<int>(places[p] >> 24) - 1;
            if ((places[p] & 0x00FFFFFFu) == formed && (o == overlay || o == OPENRAC_OVERLAY_EXE)) {
                if (seen++ == k) {
                    result = formed;
                }
                family = true;
                break;
            }
        }
        if (!family && other == 0 && find(formed) != nullptr) {
            other = formed;
        }
    }
    if (result == 0) {
        // The copy forms no address of the family but another function's: it takes a function
        // of its own (Gaspar's lava draw was one), which the fallback is not. A place for
        // split_places.tsv. (A C that calls through the address, where the code has a jal to one
        // of them, gets a copy of the same function: nothing to say.)
        if (other != 0 && !called) {
            warn(
                "the copy at {:#x} (overlay {}) takes {:#x}, not one its C names; it gets {:#x}",
                caller,
                overlay,
                other,
                fallback
            );
        }
        result = fallback;
    }
    found.emplace(key, result);
    return result;
}

const char* openrac_guest_function_name(gaddr address) {
    const Entry* e = find(address);
    return e == nullptr ? nullptr : e->name;
}

openrac_host_fn openrac_guest_function_named(const char* name) {
    if (name == nullptr) {
        return nullptr;
    }
    std::string_view want(name);
    if (want.starts_with("game_")) {
        want.remove_prefix(5);
    }
    auto search = [&](const Table& table) -> openrac_host_fn {
        for (const auto& [address, e] : table) {
            if (e.name != nullptr && want == e.name) {
                return e.fn;
            }
        }
        return nullptr;
    };
    if (openrac_host_fn fn = search(g_exe)) {
        return fn;
    }
    for (const auto& [overlay, table] : g_overlays) {
        if (openrac_host_fn fn = search(table)) {
            return fn;
        }
    }
    return nullptr;
}

void openrac_guest_missing(const char* name) {
    {
        std::scoped_lock hold(g_missing_lock);
        if (g_missing[name]++ != 0) {
            return;
        }
    }
    // OPENRAC_TRACE_MISSING: the host stack of the first call to each function without C.
    if (openrac::env::flag(openrac::env::Var::TraceMissing)) {
        void* frames[16];
        warn("first call to {}:", name);
#if defined(_WIN32)
        const auto count = CaptureStackBackTrace(0, 16, frames, nullptr);
        for (unsigned i = 0; i < count; ++i) {
            std::fprintf(stderr, "%p\n", frames[i]);
        }
#else
        const int count = backtrace(frames, 16);
        backtrace_symbols_fd(frames, count, 2);
#endif
    }
    if (g_stop_on_missing) {
        error("the game called {}, which has no C in the port yet; stopping here", name);
        std::exit(2);
    }
    warn("{} has no C in the port yet; it does nothing", name);
    // [verbose log] Who called it first.
    if (openrac::log::verbose_on()) {
        for (int depth = 0; depth < 4; ++depth) {
            char who[256];
            openrac_host_caller(depth, who, sizeof(who));
            openrac::log::verbose("  from {}", who);
        }
    }
}

unsigned long openrac_guest_missing_calls(unsigned* functions) {
    std::scoped_lock hold(g_missing_lock);
    unsigned long calls = 0;
    for (const auto& [name, count] : g_missing) {
        calls += missing_count(name, count);
    }
    if (functions != nullptr) {
        *functions = static_cast<unsigned>(g_missing.size());
    }
    return calls;
}

const char* openrac_guest_missing_at(unsigned index, unsigned long* calls) {
    std::scoped_lock hold(g_missing_lock);
    if (index >= g_missing.size()) {
        return nullptr;
    }
    auto it = g_missing.begin();
    std::advance(it, index);
    if (calls != nullptr) {
        *calls = missing_count(it->first, it->second);
    }
    return it->first.c_str();  // a map's keys do not move
}

void openrac_guest_reach_first(openrac_reach_site* site) {
    const bool stub = site->kind != nullptr && std::strcmp(site->kind, "zero-args") != 0;
    {
        std::scoped_lock hold(g_missing_lock);
        site->next = g_reached;
        g_reached = site;
        if (stub) {
            g_stub_sites.emplace(site->name, site);
        }
    }
    if (stub) {
        openrac_guest_missing(site->name);  // logged, counted once, the stop on missing
    }
}

int openrac_guest_write_reach(const char* path, const char* game, long frames) {
    // Written beside and moved in place: a run stopped half way through leaves the last whole one.
    const std::string part = std::string(path) + ".part";
    std::FILE* f = std::fopen(part.c_str(), "w");
    if (f == nullptr) {
        return 0;
    }
    std::scoped_lock hold(g_missing_lock);
    std::fputs("{\"game\": ", f);
    json_string(f, game);
    std::fprintf(f, ", \"frames\": %ld, \"sites\": [", frames);
    bool first = true;
    for (const openrac_reach_site* s = g_reached; s != nullptr; s = s->next) {
        std::fputs(first ? "\n  {\"name\": " : ",\n  {\"name\": ", f);
        first = false;
        json_string(f, s->name);
        std::fputs(", \"kind\": ", f);
        json_string(f, s->kind);
        if (s->detail != nullptr) {
            std::fputs(", \"detail\": ", f);
            json_string(f, s->detail);
        }
        std::fprintf(f, ", \"calls\": %lu}", s->calls);
    }
    // Hand C's own calls to openrac_guest_missing, with no site of hostgen's.
    for (const auto& [name, count] : g_missing) {
        if (g_stub_sites.count(name) != 0) {
            continue;
        }
        std::fputs(first ? "\n  {\"name\": " : ",\n  {\"name\": ", f);
        first = false;
        json_string(f, name.c_str());
        std::fprintf(f, ", \"kind\": \"missing\", \"calls\": %lu}", count);
    }
    std::fputs("\n]}\n", f);
    const bool ok = std::fclose(f) == 0;
    if (!ok) {
        return 0;
    }
#if defined(_WIN32)
    // The Windows CRT's rename refuses an existing destination. Replace it in
    // one operation so periodic reports refresh without discarding the last
    // complete report if the move fails.
    return MoveFileExA(part.c_str(), path, MOVEFILE_REPLACE_EXISTING) != 0 ? 1 : 0;
#else
    return std::rename(part.c_str(), path) == 0 ? 1 : 0;
#endif
}

void openrac_guest_stop_on_missing(int stop) {
    g_stop_on_missing = stop != 0;
}

void openrac_host_caller(int depth, char* text, size_t size) {
    if (size == 0) {
        return;
    }
    std::snprintf(text, size, "?");
    void* frames[16];
    if (depth < 0 || depth > 13) {
        return;
    }
    // Frame 0 is this function, 1 its caller, 2 who called that (depth 0).
    const int want = depth + 2;
#if defined(_WIN32)
    if (CaptureStackBackTrace(static_cast<DWORD>(want), 1, frames, nullptr) != 1) {
        return;
    }
    // Symbols need the debugger's library; the module and the offset in it are the same from
    // run to run, and a symbolizer names them with the program's PDB.
    HMODULE module = nullptr;
    char path[MAX_PATH];
    if (GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            static_cast<LPCSTR>(frames[0]), &module
        ) != 0
        && GetModuleFileNameA(module, path, static_cast<DWORD>(sizeof(path))) != 0) {
        const char* file = std::strrchr(path, '\\');
        const auto offset = static_cast<std::size_t>(
            static_cast<const char*>(frames[0]) - reinterpret_cast<const char*>(module)
        );
        std::snprintf(text, size, "%s+%#zx", file != nullptr ? file + 1 : path, offset);
    } else {
        std::snprintf(text, size, "%p", frames[0]);
    }
#else
    if (backtrace(frames, want + 1) <= want) {
        return;
    }
    char** names = backtrace_symbols(frames + want, 1);
    if (names != nullptr) {
        std::snprintf(text, size, "%s", names[0]);
        std::free(names);
    }
#endif
}

void openrac_guest_set_overlay_source(int (*current)(void)) {
    g_overlay_source = current;
}

void openrac_guest_report_missing(void) {
    std::scoped_lock hold(g_missing_lock);
    if (g_missing.empty()) {
        return;
    }
    std::vector<std::pair<std::string, unsigned long>> calls;
    for (const auto& [name, count] : g_missing) {
        calls.emplace_back(name, missing_count(name, count));
    }
    std::sort(calls.begin(), calls.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    warn("{} functions without C were called:", calls.size());
    for (const auto& [name, count] : calls) {
        warn("  {:>8}  {}", count, name);
    }
}

}  // extern "C"

// ---- Per-level relocation (guest.h) ----

namespace {

struct LevelMap {
    std::vector<std::pair<gaddr, std::int32_t>> data;  // executable address, delta; sorted
    std::unordered_map<gaddr, gaddr> code;              // executable function -> level copy
    std::unordered_map<gaddr, gaddr> searched;          // found by search_pair (0: none)
};

gaddr g_relocate_gp = 0;
std::vector<std::uint8_t> g_exe_code;
gaddr g_exe_base = 0;
std::map<int, LevelMap> g_level_maps;
std::mutex g_relocate_lock;

int current_overlay() {
    if (g_overlay_source != nullptr) {
        g_overlay = g_overlay_source();
    }
    return g_overlay;
}

std::uint32_t exe_word(gaddr at) {
    std::uint32_t w = 0;
    if (at >= g_exe_base && at + 4 <= g_exe_base + g_exe_code.size()) {
        std::memcpy(&w, g_exe_code.data() + (at - g_exe_base), 4);
    }
    return w;
}

std::uint32_t game_word(gaddr at) {
    std::uint32_t w;
    std::memcpy(&w, G(at), 4);
    return w;
}

// Pairs the address operands of a function's executable code and its copy: known register values
// (lui, then addiu or ori), loads and stores through them or through $gp. The two codes are the
// same instructions apart from addresses, so every pair computed in step is a correspondence.
// Past the first `own` bytes (a piece's code run on into the next pieces, piece_bytes) only an
// address whose high half was formed before them is paired: the later pieces pair the rest at their
// own places.
using WordReader = std::uint32_t (*)(int program, gaddr at);

std::uint32_t exe_reader(int, gaddr at) {
    return exe_word(at);
}

void pair_code(WordReader reference, int program, gaddr exe, gaddr level, std::uint32_t size,
               gaddr low, gaddr high, std::map<gaddr, std::int32_t>& out, std::uint32_t own = UINT32_MAX) {
    std::uint32_t ve[32] = {}, vl[32] = {};
    bool known[32] = {};
    std::uint32_t origin[32] = {};  // where a known value's high half was formed
    // A branch's target gets the registers as the branch leaves them (after its delay slot), so a
    // value formed in a delay slot (lui in a branch-likely's slot, the addiu at the target) pairs.
    struct Registers {
        std::uint32_t ve[32], vl[32], origin[32];
        bool known[32];
    };
    std::map<std::uint32_t, Registers> at_target;
    std::uint32_t pending_from = 0, pending_target = 0;
    bool pending = false;
    // A call leaves only the callee-saved registers, but its delay slot runs before it: the
    // compiler often forms an argument's address there (jal f; addiu $a1, $a1, %lo(x)), so the
    // registers are forgotten after the slot, not at the jal.
    const auto forget_call = [&known] {
        for (int r = 1; r < 32; ++r) {
            if (!((r >= 16 && r <= 23) || r == 28 || r == 29 || r == 30)) {
                known[r] = false;
            }
        }
    };
    std::uint32_t call_slot = 0;
    bool call = false;
    for (std::uint32_t i = 0; i + 4 <= size; i += 4) {
        if (call && i == call_slot + 4) {
            forget_call();
            call = false;
        }
        if (pending && i == pending_from + 8) {
            Registers& r = at_target[pending_target];
            std::memcpy(r.ve, ve, sizeof(ve));
            std::memcpy(r.vl, vl, sizeof(vl));
            std::memcpy(r.origin, origin, sizeof(origin));
            std::memcpy(r.known, known, sizeof(known));
            pending = false;
        }
        if (auto t = at_target.find(i); t != at_target.end()) {
            for (int r = 0; r < 32; ++r) {
                if (t->second.known[r]) {
                    ve[r] = t->second.ve[r];
                    vl[r] = t->second.vl[r];
                    origin[r] = t->second.origin[r];
                    known[r] = true;
                }
            }
        }
        const std::uint32_t we = reference(program, exe + i), wl = game_word(level + i);
        const std::uint32_t op = we >> 26;
        if (op != (wl >> 26)) {
            break;  // not the same code any more (the copy ends, or another function)
        }
        const bool branch = (op >= 0x04 && op <= 0x07) || (op >= 0x14 && op <= 0x17) || op == 0x01
                            || (op == 0x11 && ((we >> 21) & 31) == 8);
        if (branch) {
            const std::int32_t offset = static_cast<std::int16_t>(we & 0xFFFF);
            const std::int64_t target = static_cast<std::int64_t>(i) + 4 + static_cast<std::int64_t>(offset) * 4;
            if (target > static_cast<std::int64_t>(i) + 4 && target < static_cast<std::int64_t>(size)) {
                pending = true;
                pending_from = i;
                pending_target = static_cast<std::uint32_t>(target);
            }
        }
        const std::uint32_t rs = (we >> 21) & 31, rt = (we >> 16) & 31;
        const std::int32_t se = static_cast<std::int16_t>(we & 0xFFFF);
        const std::int32_t sl = static_cast<std::int16_t>(wl & 0xFFFF);
        if (op == 0x0F) {  // lui
            ve[rt] = (we & 0xFFFF) << 16;
            vl[rt] = (wl & 0xFFFF) << 16;
            origin[rt] = i;
            known[rt] = true;
            continue;
        }
        const bool load = (op >= 0x20 && op <= 0x27) || op == 0x37 || op == 0x1E;
        const bool store = (op >= 0x28 && op <= 0x2F) || op == 0x3F || op == 0x1F;
        const bool fpu = op == 0x31 || op == 0x39;  // lwc1, swc1: an address, no GPR written
        const bool add = op == 0x09 || op == 0x19 || op == 0x0D;  // addiu, daddiu, ori
        std::uint32_t ae = 0, al = 0, formed = i;
        bool have = false;
        if ((load || store || fpu || add) && rs == 28 && g_relocate_gp != 0) {
            ae = g_relocate_gp + se;
            al = g_relocate_gp + sl;
            have = true;
        } else if ((load || store || fpu || add) && known[rs]) {
            ae = op == 0x0D ? (ve[rs] | (we & 0xFFFF)) : ve[rs] + se;
            al = op == 0x0D ? (vl[rs] | (wl & 0xFFFF)) : vl[rs] + sl;
            formed = origin[rs];
            have = true;
        }
        // What the instruction writes is no longer a known value (unless it is the address just
        // formed by addiu/ori); a call leaves only the callee-saved registers.
        const std::uint32_t funct = we & 0x3F, rd = (we >> 11) & 31;
        if (op == 0x00 && (funct == 0x21 || funct == 0x25 || funct == 0x2D) && rt == 0 && rd != 0) {
            // A move (addu, or, daddu with $zero): the value goes along.
            ve[rd] = ve[rs];
            vl[rd] = vl[rs];
            origin[rd] = origin[rs];
            known[rd] = known[rs];
        } else if (op == 0x00 || op == 0x1C) {
            known[rd] = false;
            if (op == 0x00 && funct == 0x09) {  // jalr: as jal
                call = true;
                call_slot = i + 4;
            }
        } else if (op == 0x03) {  // jal: the registers are forgotten after its delay slot
            call = true;
            call_slot = i + 4;
        } else if (add) {
            known[rt] = have;
            if (have) {
                ve[rt] = ae;
                vl[rt] = al;
                origin[rt] = formed;
            }
        } else if (load || (op >= 0x08 && op <= 0x0E) || op == 0x18) {
            known[rt] = false;
        }
        if (!have || (i >= own && formed >= own)) {
            continue;
        }
        if (ae - low < high - low) {
            out.emplace(ae, static_cast<std::int32_t>(al - ae));
        }
    }
}

// The functions of a program in address order: a function's code is up to the next one.
using Functions = std::vector<std::pair<gaddr, openrac_host_fn>>;

std::uint32_t piece_size(const Functions& fns, std::size_t k) {
    return k + 1 < fns.size() ? std::min<std::uint32_t>(fns[k + 1].first - fns[k].first, 0x8000) : 0x400;
}

// jr, j, b (beq or bgez on $zero), eret: the code after it (and its delay slot) is not reached
// from it.
bool jumps_away(std::uint32_t w) {
    const std::uint32_t op = w >> 26;
    return (op == 0x00 && (w & 0x3F) == 0x08) || op == 0x02 || (op == 0x04 && ((w >> 16) & 0x3FF) == 0)
           || (op == 0x01 && ((w >> 16) & 0x3FF) == 0x01) || w == 0x42000018u;
}

// The bytes to pair for fns[k], and how many of them are its own: a function whose code runs on
// into the next one (it does not end in a jump and its delay slot, then nops) is one the catalogue
// cut where a branch or a table gave the next address a start (games/rac1's
// config/overlays/joined.tsv), so its registers go on into the pieces after it. rac1: the pulse
// channels' update (func_L00_001EAD88) forms the high half of its table D_L00_00166100 in its first
// piece and adds the low half in the third; paired piece by piece, the table had no pair and moved
// with the one below it, 0x20-0x40 bytes off in levels 4, 11 and 16.
std::pair<std::uint32_t, std::uint32_t> piece_bytes(WordReader reader, int program, const Functions& fns,
                                                    std::size_t k) {
    const std::uint32_t own = piece_size(fns, k);
    std::uint32_t size = own;
    for (std::size_t j = k; j + 1 < fns.size() && size < 0x8000; ++j) {
        const std::uint32_t piece = piece_size(fns, j);
        std::uint32_t last = piece / 4;
        while (last > 0 && reader(program, fns[j].first + 4 * (last - 1)) == 0) {
            --last;  // nops after the end
        }
        if (last == 0 || jumps_away(reader(program, fns[j].first + 4 * (last - 1)))
            || (last >= 2 && jumps_away(reader(program, fns[j].first + 4 * (last - 2))))) {
            break;
        }
        size = std::min<std::uint32_t>(size + piece_size(fns, j + 1), 0x8000);
    }
    return {size, own};
}

// Pairs each function of a reference program (`own`: its places and host functions, in address
// order) with its copies in the loaded program: code addresses into `code`, globals into `out`
// (the first pair of an address kept). A function the catalogue folded has several copies in one
// program (rac1: the three text measurers, one per font, each on its own glyph table), and only one
// of them is the reference's: paired with the wrong one, every use of its global in the level is
// that copy's (the regular font's table read as the large font's). The copies are taken in address
// order, so the result does not hang on a hash table's order:
// - as many places of the function in the reference as copies: the k-th with the k-th;
// - otherwise, each copy's pairs that every copy agrees on (an address all of them form the same
//   way), and none of the others: those are each copy's own, which another function's pairs or
//   openrac_data_in_copy give; the code address goes to the lowest copy.
// Each function's code is what piece_bytes gives (its registers run on into the catalogue pieces
// after it).
void pair_programs(WordReader reference, int program, const Functions& own,
                   std::unordered_map<openrac_host_fn, std::vector<gaddr>> copies, gaddr low, gaddr high,
                   bool skip_same_place, std::unordered_map<gaddr, gaddr>& code,
                   std::map<gaddr, std::int32_t>& out) {
    std::unordered_map<openrac_host_fn, std::size_t> own_count, own_rank;
    for (const auto& [address, fn] : own) {
        ++own_count[fn];
    }
    for (auto& [fn, places] : copies) {
        std::sort(places.begin(), places.end());
    }
    for (std::size_t k = 0; k < own.size(); ++k) {
        auto copy = copies.find(own[k].second);
        const std::size_t rank = own_rank[own[k].second]++;
        if (copy == copies.end() || copy->second.empty()) {
            continue;
        }
        const std::vector<gaddr>& places = copy->second;
        const auto [size, first] = piece_bytes(reference, program, own, k);
        const gaddr at = own[k].first;
        if (places.size() == 1 || places.size() == own_count[own[k].second]) {
            const gaddr place = places.size() == 1 ? places[0] : places[rank];
            if (!(skip_same_place && place == at)) {
                code.emplace(at, place);
            }
            pair_code(reference, program, at, place, size, low, high, out, first);
            continue;
        }
        if (!(skip_same_place && places[0] == at)) {
            code.emplace(at, places[0]);
        }
        std::map<gaddr, std::int32_t> agreed;
        pair_code(reference, program, at, places[0], size, low, high, agreed, first);
        for (std::size_t c = 1; c < places.size() && !agreed.empty(); ++c) {
            std::map<gaddr, std::int32_t> other;
            pair_code(reference, program, at, places[c], size, low, high, other, first);
            for (auto it = agreed.begin(); it != agreed.end();) {
                auto o = other.find(it->first);
                it = o != other.end() && o->second == it->second ? std::next(it) : agreed.erase(it);
            }
        }
        for (const auto& pair : agreed) {
            out.emplace(pair);
        }
    }
}

LevelMap& level_map(int overlay) {
    auto found = g_level_maps.find(overlay);
    if (found != g_level_maps.end()) {
        return found->second;
    }
    LevelMap& map = g_level_maps[overlay];
    auto level = g_overlays.find(overlay);
    if (level == g_overlays.end()) {
        return map;
    }
    // The executable's functions in address order, for their sizes.
    Functions exe;
    exe.reserve(g_exe.size());
    for (const auto& [address, entry] : g_exe) {
        exe.emplace_back(address, entry.fn);
    }
    std::sort(exe.begin(), exe.end());
    std::unordered_map<openrac_host_fn, std::vector<gaddr>> copies;
    for (const auto& [address, entry] : level->second) {
        copies[entry.fn].push_back(address);
    }
    std::map<gaddr, std::int32_t> pairs;
    pair_programs(exe_reader, 0, exe, std::move(copies), openrac_relocate_low, openrac_relocate_high, true,
                  map.code, pairs);
    map.data.assign(pairs.begin(), pairs.end());
    info("level program {}: {} functions and {} global addresses relocated", overlay, map.code.size(),
         map.data.size());
    return map;
}

// ---- Level to level ----
//
// A function several levels' programs carry is written with the addresses of the level its name
// gives (func_L05_...: level 5's). Its globals and the functions whose addresses it takes are
// looked up the same way as an executable function's, by pairing its code in that level's program
// (read from the player's data, through openrac_guest_set_level_programs) with its copy in the
// level loaded.

struct Chunk {
    gaddr base;
    std::vector<std::uint8_t> bytes;
};

void (*g_level_program_source)(int level) = nullptr;
std::map<int, std::vector<Chunk>> g_level_programs;
std::map<std::pair<int, int>, LevelMap> g_level_pair_maps;  // (from, loaded)
constexpr gaddr kLevelLow = 0x0015F000u, kLevelHigh = 0x00400000u;

const std::vector<Chunk>& level_program(int level) {
    auto found = g_level_programs.find(level);
    if (found == g_level_programs.end()) {
        g_level_programs[level];
        if (g_level_program_source != nullptr) {
            g_level_program_source(level);  // fills g_level_programs[level]
        }
        found = g_level_programs.find(level);
    }
    return found->second;
}

std::uint32_t level_reader(int level, gaddr at) {
    for (const Chunk& c : level_program(level)) {
        if (at >= c.base && at + 4 <= c.base + c.bytes.size()) {
            std::uint32_t w;
            std::memcpy(&w, c.bytes.data() + (at - c.base), 4);
            return w;
        }
    }
    return 0;
}

LevelMap& level_pair_map(int from, int loaded) {
    const auto key = std::make_pair(from, loaded);
    auto found = g_level_pair_maps.find(key);
    if (found != g_level_pair_maps.end()) {
        return found->second;
    }
    LevelMap& map = g_level_pair_maps[key];
    auto source = g_overlays.find(from);
    auto level = g_overlays.find(loaded);
    if (source == g_overlays.end() || level == g_overlays.end() || level_program(from).empty()) {
        return map;
    }
    Functions own;
    own.reserve(source->second.size());
    for (const auto& [address, entry] : source->second) {
        own.emplace_back(address, entry.fn);
    }
    std::sort(own.begin(), own.end());
    std::unordered_map<openrac_host_fn, std::vector<gaddr>> copies;
    for (const auto& [address, entry] : level->second) {
        copies[entry.fn].push_back(address);
    }
    std::map<gaddr, std::int32_t> pairs;
    pair_programs(level_reader, from, own, std::move(copies), kLevelLow, kLevelHigh, false, map.code, pairs);
    map.data.assign(pairs.begin(), pairs.end());
    info("level program {} for level {}'s functions: {} functions and {} global addresses relocated",
         loaded, from, map.code.size(), map.data.size());
    // Which function asked first (the place a level's copy of another level's code starts running).
    for (int depth = 1; depth < 4; ++depth) {
        char who[256];
        openrac_host_caller(depth, who, sizeof(who));
        info("  for {}", who);
    }
    return map;
}

gaddr relocate_by(const LevelMap& map, gaddr address) {
    auto it = std::upper_bound(map.data.begin(), map.data.end(), std::make_pair(address, INT32_MAX));
    if (it == map.data.begin() || address - std::prev(it)->first > 0x100) {
        // OPENRAC_TRACE_RELOCATION: each level address no paired one is near, once.
        if (openrac::env::flag(openrac::env::Var::TraceRelocation)) {
            static std::map<gaddr, bool> told;
            if (!told[address]) {
                told[address] = true;
                warn("level address {:#x} has no paired address within 0x100 below it; kept", address);
            }
        }
        return address;
    }
    --it;
    return address + static_cast<gaddr>(it->second);
}

// An instruction with its address fields cleared (the immediates of lui, addiu, ori, the loads and
// stores; the targets of j and jal), for comparing the same code in two programs.
std::uint32_t without_addresses(std::uint32_t w) {
    const std::uint32_t op = w >> 26;
    if (op == 0x0F || op == 0x09 || op == 0x0D || op == 0x19 || (op >= 0x20 && op <= 0x2F) || op == 0x31
        || op == 0x39 || op == 0x37 || op == 0x3F || op == 0x1E || op == 0x1F) {
        return w & 0xFFFF0000u;
    }
    if (op == 0x02 || op == 0x03) {
        return w & 0xFC000000u;
    }
    return w;
}

// An executable address no paired function reaches (code the catalogue does not list in the level,
// such as the hand-written routines): the executable code that forms it (a lui and the instruction
// completing it), found again in the loaded level's program by its instructions, and paired there.
// 0 when that fails.
gaddr search_pair(gaddr address, int overlay) {
    const auto& chunks = level_program(overlay);
    const std::size_t words = g_exe_code.size() / 4;
    if (chunks.empty() || words == 0) {
        return 0;
    }
    const auto exe_at = [](std::size_t i) {
        std::uint32_t w;
        std::memcpy(&w, g_exe_code.data() + i * 4, 4);
        return w;
    };
    constexpr std::size_t kBefore = 6, kLength = 24;
    for (std::size_t i = 0; i < words; ++i) {
        const std::uint32_t w = exe_at(i);
        if ((w >> 26) != 0x0F) {
            continue;
        }
        const std::uint32_t rt = (w >> 16) & 31;
        const std::uint32_t hi = (w & 0xFFFF) << 16;
        bool forms = false;
        for (std::size_t j = i + 1; j < std::min(words, i + 12) && !forms; ++j) {
            const std::uint32_t u = exe_at(j);
            const std::uint32_t op = u >> 26;
            const bool use = op == 0x09 || op == 0x0D || (op >= 0x20 && op <= 0x2F) || op == 0x31 || op == 0x39
                             || op == 0x37 || op == 0x3F || op == 0x1E || op == 0x1F;
            if (use && ((u >> 21) & 31) == rt) {
                const gaddr a = op == 0x0D ? (hi | (u & 0xFFFF)) : hi + static_cast<std::uint32_t>(static_cast<std::int16_t>(u & 0xFFFF));
                forms = a == address;
            }
        }
        if (!forms) {
            continue;
        }
        const std::size_t start = i >= kBefore ? i - kBefore : 0;
        const std::size_t length = std::min(kLength, words - start);
        std::uint32_t pattern[kLength];
        for (std::size_t k = 0; k < length; ++k) {
            pattern[k] = without_addresses(exe_at(start + k));
        }
        gaddr found = 0;
        int matches = 0;
        for (const Chunk& c : chunks) {
            const std::size_t n = c.bytes.size() / 4;
            for (std::size_t p = 0; p + length <= n && matches < 2; ++p) {
                std::size_t k = 0;
                for (; k < length; ++k) {
                    std::uint32_t lw;
                    std::memcpy(&lw, c.bytes.data() + (p + k) * 4, 4);
                    if (without_addresses(lw) != pattern[k]) {
                        break;
                    }
                }
                if (k == length) {
                    found = c.base + static_cast<gaddr>(p * 4);
                    ++matches;
                }
            }
        }
        if (openrac::env::flag(openrac::env::Var::TraceRelocation)) {
            info("  {:#x}: formed at {:#x} in the executable, {} matches in the level", address,
                 g_exe_base + static_cast<gaddr>(i * 4), matches);
        }
        if (matches != 1) {
            continue;
        }
        std::map<gaddr, std::int32_t> pairs;
        pair_code(exe_reader, 0, g_exe_base + static_cast<gaddr>(start * 4), found, static_cast<std::uint32_t>(length * 4),
                  openrac_relocate_low, openrac_relocate_high, pairs);
        if (auto it = pairs.find(address); it != pairs.end()) {
            return address + static_cast<gaddr>(it->second);
        }
    }
    return 0;
}

}  // namespace

extern "C" {

gaddr openrac_relocate_low = 0, openrac_relocate_high = 0;

void openrac_guest_set_relocation(gaddr low, gaddr high, gaddr gp, const uint8_t* exe_code,
                                  gaddr exe_base, uint32_t exe_size) {
    openrac_relocate_low = low;
    openrac_relocate_high = high;
    g_relocate_gp = gp;
    g_exe_code.assign(exe_code, exe_code + exe_size);
    g_exe_base = exe_base;
}

gaddr openrac_relocate_data(gaddr address) {
    const int overlay = current_overlay();
    if (overlay == OPENRAC_OVERLAY_EXE) {
        return address;
    }
    // $gp's value is the same in every program: C that names it means the register (rac1: D_00166D00
    // as a thread's gpReg, initAll's movie decoder thread), not the executable's datum that starts
    // there (gp+0, which a level's program keeps elsewhere: func_00218A78's `sw $24, 0($gp)` is
    // `sw $24, -0x5A80($gp)` in level 0's). The pairs have both, and the datum's came first.
    if (address == g_relocate_gp && address != 0) {
        return address;
    }
    std::scoped_lock hold(g_relocate_lock);
    const LevelMap& map = level_map(overlay);
    // A global's own address is what the code names (offsets are added after): exact pairs first;
    // otherwise the nearest pair below it, if close (inside the same object).
    auto it = std::upper_bound(map.data.begin(), map.data.end(), std::make_pair(address, INT32_MAX));
    if (it == map.data.begin() || address - std::prev(it)->first > 0x100) {
        LevelMap& found = level_map(overlay);
        auto s = found.searched.find(address);
        if (s == found.searched.end()) {
            s = found.searched.emplace(address, search_pair(address, overlay)).first;
            if (openrac::env::flag(openrac::env::Var::TraceRelocation)) {
                warn("executable address {:#x} has no paired address within 0x100 below it in level {}; {}",
                     address, overlay, s->second != 0 ? std::format("found by its code at {:#x}", s->second) : "kept");
            }
        }
        return s->second != 0 ? s->second : address;
    }
    --it;
    return address + static_cast<gaddr>(it->second);
}

gaddr openrac_relocate_code(gaddr address) {
    const int overlay = current_overlay();
    if (overlay == OPENRAC_OVERLAY_EXE) {
        return address;
    }
    std::scoped_lock hold(g_relocate_lock);
    const LevelMap& map = level_map(overlay);
    auto it = map.code.find(address);
    if (it != map.code.end()) {
        return it->second;
    }
    // A function whose copy in the level's program the catalogue has as one of the level's own (its
    // code differs: rac1's music callbacks, func_00217970 that music_StartTrackBody hands the sound
    // library is func_L00_00267130 in level 0's program): what the level's copy of the code forms where
    // the executable's forms this address (an exact pair of their operands), when a function is there.
    auto d = std::lower_bound(map.data.begin(), map.data.end(), std::make_pair(address, INT32_MIN));
    if (d != map.data.end() && d->first == address) {
        const gaddr there = address + static_cast<gaddr>(d->second);
        auto level = g_overlays.find(overlay);
        if (level != g_overlays.end() && level->second.count(there) != 0) {
            return there;
        }
    }
    return address;
}

void openrac_guest_set_level_programs(void (*load)(int level)) {
    g_level_program_source = load;
}

void openrac_guest_add_level_program(int level, gaddr base, const uint8_t* bytes, uint32_t size) {
    g_level_programs[level].push_back(Chunk{base, std::vector<std::uint8_t>(bytes, bytes + size)});
}

gaddr openrac_relocate_level_data(int from, gaddr address) {
    const int overlay = current_overlay();
    if (overlay == from || overlay == OPENRAC_OVERLAY_EXE || address - kLevelLow >= kLevelHigh - kLevelLow) {
        return address;
    }
    std::scoped_lock hold(g_relocate_lock);
    return relocate_by(level_pair_map(from, overlay), address);
}

gaddr openrac_guest_last_entry = 0;
openrac_host_fn openrac_guest_last_fn = nullptr;

// Which lui each register's upper half came from, as a function's code runs straight through: a
// register move (addu, daddu or or with $zero) carries it along, and any other write forgets it.
struct UpperHalves {
    std::uint32_t at[32] = {};
    bool set[32] = {};

    void step(std::uint32_t w, std::uint32_t i) {
        const std::uint32_t op = w >> 26, rs = (w >> 21) & 31, rt = (w >> 16) & 31, rd = (w >> 11) & 31;
        if (op == 0x0F) {
            at[rt] = i;
            set[rt] = true;
        } else if (op == 0x00) {
            const std::uint32_t funct = w & 0x3F;
            if ((funct == 0x21 || funct == 0x25 || funct == 0x2D) && rt == 0) {
                at[rd] = at[rs];
                set[rd] = set[rs];
            } else if (funct != 0x08 && funct != 0x0C && funct != 0x0D && funct != 0x11 && funct != 0x13
                       && (funct < 0x18 || funct > 0x1F)) {
                set[rd] = false;
            }
        } else if ((op >= 0x08 && op <= 0x0E) || (op >= 0x20 && op <= 0x27) || op == 0x1A || op == 0x1B
                   || op == 0x1E || op == 0x37) {
            set[rt] = false;
        }
        set[0] = false;
    }
};

// The place of `canon`'s function (a level function at `canon` in level `from`'s program) in the
// program loaded now, when it has exactly one there: a copy called directly from C knows no place
// of its own (openrac_guest_entry gives 0), and this is the one its code would run at.
gaddr own_place(int from, gaddr canon, int overlay) {
    static std::map<std::tuple<int, gaddr, int>, gaddr> found;
    const auto key = std::make_tuple(from, canon, overlay);
    if (auto it = found.find(key); it != found.end()) {
        return it->second;
    }
    gaddr place = 0;
    auto source = g_overlays.find(from);
    auto loaded = g_overlays.find(overlay);
    if (source != g_overlays.end() && loaded != g_overlays.end()) {
        auto entry = source->second.find(canon);
        if (entry != source->second.end()) {
            int count = 0;
            for (const auto& [address, e] : loaded->second) {
                if (e.fn == entry->second.fn) {
                    place = address;
                    ++count;
                }
            }
            if (count != 1) {
                place = 0;
            }
        }
    }
    found.emplace(key, place);
    return place;
}

gaddr openrac_data_in_copy(int from, gaddr canon, uint32_t size, gaddr self, gaddr target) {
    const int overlay = current_overlay();
    // from -1: an executable function's global, the executable's own address relocated.
    const gaddr fallback = from < 0 ? OPENRAC_DATA(target) : openrac_relocate_level_data(from, target);
    if (self == 0 && from >= 0 && overlay != from) {
        std::scoped_lock hold(g_relocate_lock);
        self = own_place(from, canon, overlay);
    }
    if (self == 0 || (self == canon && overlay == from)) {
        return fallback;
    }
    static std::map<std::tuple<int, int, gaddr, gaddr, gaddr>, gaddr> found;
    std::scoped_lock hold(g_relocate_lock);
    const auto key = std::make_tuple(from, overlay, canon, self, target);
    if (auto it = found.find(key); it != found.end()) {
        return it->second;
    }
    const auto canon_word = [&](gaddr at) -> std::uint32_t {
        if (from < 0) {
            return exe_word(at);
        }
        if (overlay == from) {
            std::uint32_t w;
            std::memcpy(&w, G(at), 4);
            return w;
        }
        return level_reader(from, at);
    };
    const auto self_word = [&](gaddr at) {
        std::uint32_t w;
        std::memcpy(&w, G(at), 4);
        return w;
    };
    // addiu, ori, and the loads and stores (lb .. sw, lwc1, swc1, ld, sd, lq, sq) that take %lo.
    const auto completes = [](std::uint32_t op) {
        return op == 0x09 || op == 0x0D || (op >= 0x20 && op <= 0x2B) || op == 0x31 || op == 0x39
               || op == 0x37 || op == 0x3F || op == 0x1E || op == 0x1F;
    };
    const auto formed = [](std::uint32_t hi, std::uint32_t lo) -> gaddr {
        const std::uint32_t upper = (hi & 0xFFFFu) << 16;
        const std::uint32_t imm = lo & 0xFFFFu;
        return (lo >> 26) == 0x0D ? (upper | imm) : upper + static_cast<std::uint32_t>(static_cast<std::int16_t>(imm));
    };
    gaddr result = fallback;
    std::uint32_t best = 0x101;
    UpperHalves upper;
    for (std::uint32_t i = 0; i * 4 < size; ++i) {
        const std::uint32_t w = canon_word(canon + i * 4);
        const std::uint32_t op = w >> 26, rs = (w >> 21) & 31;
        if (completes(op) && upper.set[rs]) {
            const gaddr at = formed(canon_word(canon + upper.at[rs] * 4), w);
            const std::uint32_t distance = at > target ? at - target : target - at;
            const std::uint32_t hi = self_word(self + upper.at[rs] * 4), lo = self_word(self + i * 4);
            if (distance <= 0x100 && distance < best && (hi >> 26) == 0x0F && (lo >> 26) == op) {
                best = distance;
                result = formed(hi, lo) + (target - at);
                if (distance == 0) {
                    break;
                }
            }
        } else if (completes(op) && op != 0x0D && rs == 28 && g_relocate_gp != 0) {
            // A small global, reached from $gp (the same in every program): the copy at `self` has
            // its own offset in the same instruction (rac1: Drek's Fleet's three window-glass
            // callbacks, one function folded, each drawing its own panes from its own tables).
            const gaddr at = g_relocate_gp + static_cast<std::uint32_t>(static_cast<std::int16_t>(w & 0xFFFFu));
            const std::uint32_t distance = at > target ? at - target : target - at;
            const std::uint32_t lo = self_word(self + i * 4);
            if (distance <= 0x100 && distance < best && (lo >> 26) == op && ((lo >> 21) & 31) == 28) {
                best = distance;
                result = g_relocate_gp + static_cast<std::uint32_t>(static_cast<std::int16_t>(lo & 0xFFFFu))
                         + (target - at);
                if (distance == 0) {
                    break;
                }
            }
        }
        upper.step(w, i);
    }
    found.emplace(key, result);
    return result;
}

// openrac_code_in_copy's and openrac_code_formed's search: where the copy at canon (level from's
// program, `size` bytes) forms target, or another place of target's function, read at the same
// instructions of the copy at self; fallback when it forms neither.
static gaddr code_in_copy_at(int from, int overlay, gaddr canon, uint32_t size, gaddr self, gaddr target,
                             gaddr fallback) {
    static std::map<std::tuple<int, int, gaddr, gaddr, gaddr>, gaddr> found;
    std::scoped_lock hold(g_relocate_lock);
    const auto key = std::make_tuple(from, overlay, canon, self, target);
    if (auto it = found.find(key); it != found.end()) {
        return it->second;
    }
    // Where the copy at canon forms target: a lui and the addiu or ori that completes it.
    const auto canon_word = [&](gaddr at) -> std::uint32_t {
        if (overlay == from) {
            std::uint32_t w;
            std::memcpy(&w, G(at), 4);
            return w;
        }
        return level_reader(from, at);
    };
    const auto self_word = [&](gaddr at) {
        std::uint32_t w;
        std::memcpy(&w, G(at), 4);
        return w;
    };
    const auto formed = [](std::uint32_t hi, std::uint32_t lo) -> gaddr {
        const std::uint32_t upper = (hi & 0xFFFFu) << 16;
        const std::uint32_t imm = lo & 0xFFFFu;
        return (lo >> 26) == 0x0D ? (upper | imm) : upper + static_cast<std::uint32_t>(static_cast<std::int16_t>(imm));
    };
    gaddr result = fallback;
    bool formed_here = false;
    // A function the C takes the address of and calls through it, where the compiler called it
    // directly: the copy at canon has a jal to target and forms no address. The copy at self calls
    // its own function at the same instruction (rac1: func_L00_0029D988 calls the hero update
    // through a pointer; in Novalis that is func_L01_00228E08, not level 0's func_L00_002076E8).
    std::uint32_t jal_at = size;
    // The function at target in the program of from: a place the catalogue folded into it is the
    // same function at another address, which the code may form instead (rac1: level 0's
    // func_L00_002C9820 registers the billboard draw at 0x2C9158, folded into func_L00_002ADE90,
    // and forms only that address). Searched for when the code forms target itself nowhere.
    openrac_host_fn target_fn = nullptr;
    const Table* from_table = nullptr;
    if (auto t = g_overlays.find(from); t != g_overlays.end()) {
        from_table = &t->second;
        if (auto e = from_table->find(target); e != from_table->end()) {
            target_fn = e->second.fn;
        }
    }
    for (int folded = 0; folded < 2 && !formed_here; ++folded) {
        if (folded && target_fn == nullptr) {
            break;
        }
        UpperHalves upper;
        for (std::uint32_t i = 0; i * 4 < size; upper.step(canon_word(canon + i * 4), i), ++i) {
            const std::uint32_t w = canon_word(canon + i * 4);
            const std::uint32_t op = w >> 26, rs = (w >> 21) & 31;
            if (!folded && op == 3 && jal_at == size && ((w & 0x03FFFFFFu) << 2) == (target & 0x0FFFFFFFu)) {
                jal_at = i;
            }
            if ((op != 0x09 && op != 0x0D) || !upper.set[rs]) {
                continue;
            }
            const gaddr at = formed(canon_word(canon + upper.at[rs] * 4), w);
            bool match = at == target;
            if (!match && folded) {
                auto e = from_table->find(at);
                match = e != from_table->end() && e->second.fn == target_fn;
            }
            if (match) {
                const std::uint32_t hi = self_word(self + upper.at[rs] * 4), lo = self_word(self + i * 4);
                if ((hi >> 26) == 0x0F && (lo >> 26) == op) {
                    result = formed(hi, lo);
                }
                formed_here = true;
                break;
            }
        }
    }
    if (!formed_here && jal_at != size) {
        const std::uint32_t mine = self_word(self + jal_at * 4);
        if ((mine >> 26) == 3) {
            result = (self & 0xF0000000u) | ((mine & 0x03FFFFFFu) << 2);
        }
    }
    found.emplace(key, result);
    return result;
}

gaddr openrac_code_in_copy(int from, gaddr canon, uint32_t size, gaddr self, gaddr target) {
    const int overlay = current_overlay();
    const gaddr fallback = openrac_relocate_level_code(from, target);
    if (self == 0 && from >= 0 && overlay != from) {
        std::scoped_lock hold(g_relocate_lock);
        self = own_place(from, canon, overlay);
    }
    if (self == 0 || (self == canon && overlay == from)) {
        return fallback;
    }
    return code_in_copy_at(from, overlay, canon, size, self, target, fallback);
}

gaddr openrac_code_formed(int from, gaddr canon, uint32_t size, gaddr self, gaddr target, gaddr fallback) {
    const int overlay = current_overlay();
    if (self == 0 && overlay == from) {
        self = canon;  // the code its name is written for: which of the places it forms itself
    } else if (self == 0 && from >= 0) {
        std::scoped_lock hold(g_relocate_lock);
        self = own_place(from, canon, overlay);
    }
    if (self == 0) {
        return fallback;
    }
    return code_in_copy_at(from, overlay, canon, size, self, target, fallback);
}

gaddr openrac_call_in_copy(int from, gaddr canon, uint32_t size, gaddr self, gaddr target) {
    const int overlay = current_overlay();
    const gaddr fallback = from < 0 ? OPENRAC_CODE(target) : openrac_relocate_level_code(from, target);
    if (self == 0 && from >= 0 && overlay != from) {
        std::scoped_lock hold(g_relocate_lock);
        self = own_place(from, canon, overlay);
    }
    if (self == 0 || self == canon) {
        return fallback;
    }
    static std::map<std::tuple<int, int, gaddr, gaddr, gaddr>, gaddr> found;
    std::scoped_lock hold(g_relocate_lock);
    const auto key = std::make_tuple(from, overlay, canon, self, target);
    if (auto it = found.find(key); it != found.end()) {
        return it->second;
    }
    const auto canon_word = [&](gaddr at) -> std::uint32_t {
        if (from < 0) {
            return exe_word(at);
        }
        if (overlay == from) {
            std::uint32_t w;
            std::memcpy(&w, G(at), 4);
            return w;
        }
        return level_reader(from, at);
    };
    gaddr result = fallback;
    for (std::uint32_t i = 0; i * 4 < size; ++i) {
        const std::uint32_t w = canon_word(canon + i * 4);
        if ((w >> 26) == 3 && ((w & 0x03FFFFFFu) << 2) == (target & 0x0FFFFFFFu)) {
            std::uint32_t mine;
            std::memcpy(&mine, G(self + i * 4), 4);
            if ((mine >> 26) == 3) {
                result = (self & 0xF0000000u) | ((mine & 0x03FFFFFFu) << 2);
            }
            break;
        }
    }
    found.emplace(key, result);
    if (openrac::env::flag(openrac::env::Var::TraceRelocation)) {
        std::fprintf(stderr, "call in copy %08X+%08X: %08X -> %08X\n", canon, self, target, result);
    }
    return result;
}

gaddr openrac_hook_in_copy(gaddr exe_fn, uint32_t size, int home, gaddr home_fn, gaddr stub, uint32_t k) {
    const int overlay = current_overlay();
    const gaddr fallback = OPENRAC_CODE(stub);
    if (overlay == OPENRAC_OVERLAY_EXE || overlay < 0) {
        return fallback;
    }
    const gaddr self = overlay == home ? home_fn : openrac_relocate_level_code(home, home_fn);
    if (self == 0 || (self == home_fn && overlay != home)) {
        return fallback;  // no place of this function in the loaded program
    }
    static std::map<std::tuple<int, gaddr, gaddr, std::uint32_t>, gaddr> found;
    std::scoped_lock hold(g_relocate_lock);
    const auto key = std::make_tuple(overlay, exe_fn, stub, k);
    if (auto it = found.find(key); it != found.end()) {
        return it->second;
    }
    // The K-th call of the stub in the C is the K-th jal to it in the executable's code, unless the
    // compiler merged calls the C makes on two paths into one jal (rac1: GameStateUpdate,
    // func_00230A90, ends both ship states in the same func_0022DD68 .. func_001E97A8 run): a K past
    // the jals takes the first, as every jal to one stub goes to the same function in a level's copy.
    gaddr result = fallback;
    gaddr first = 0;
    std::uint32_t seen = 0;
    for (std::uint32_t i = 0; i * 4 < size; ++i) {
        const std::uint32_t w = exe_word(exe_fn + i * 4);
        if ((w >> 26) != 3 || ((w & 0x03FFFFFFu) << 2) != (stub & 0x0FFFFFFFu)) {
            continue;
        }
        std::uint32_t mine;
        std::memcpy(&mine, G(self + i * 4), 4);
        const gaddr target = (mine >> 26) == 3 ? (self & 0xF0000000u) | ((mine & 0x03FFFFFFu) << 2) : fallback;
        if (seen == 0) {
            first = target;
        }
        if (seen++ == k) {
            result = target;
            break;
        }
    }
    if (seen <= k && first != 0) {
        result = first;
    }
    found.emplace(key, result);
    if (openrac::env::flag(openrac::env::Var::TraceRelocation)) {
        std::fprintf(stderr, "hook in copy %08X #%u: %08X -> %08X (level %d)\n", exe_fn, k, stub, result, overlay);
    }
    return result;
}

gaddr openrac_relocate_level_code(int from, gaddr address) {
    const int overlay = current_overlay();
    if (overlay == from || overlay == OPENRAC_OVERLAY_EXE) {
        return address;
    }
    std::scoped_lock hold(g_relocate_lock);
    LevelMap& map = level_pair_map(from, overlay);
    auto it = map.code.find(address);
    if (it != map.code.end()) {
        return it->second;
    }
    // A function the catalogue lists in `from` only, whose copy in the loaded level it names
    // otherwise (two fingerprints of the same code): the loaded level's function whose instructions,
    // without their address fields, are the same. Searched once, kept either way.
    auto known = map.searched.find(address);
    if (known != map.searched.end()) {
        return known->second != 0 ? known->second : address;
    }
    gaddr twin = 0;
    auto source = g_overlays.find(from);
    auto level = g_overlays.find(overlay);
    if (source != g_overlays.end() && level != g_overlays.end() && source->second.count(address) != 0) {
        gaddr next = 0;
        for (const auto& [other, entry] : source->second) {
            if (other > address && (next == 0 || other < next)) {
                next = other;
            }
        }
        const std::uint32_t size = next != 0 ? std::min<std::uint32_t>(next - address, 0x2000) : 0x100;
        for (const auto& [place, entry] : level->second) {
            bool same = size >= 8;
            for (std::uint32_t off = 0; same && off < size; off += 4) {
                std::uint32_t w;
                std::memcpy(&w, G(place + off), 4);
                same = without_addresses(w) == without_addresses(level_reader(from, address + off));
            }
            if (same) {
                twin = place;
                break;
            }
        }
        if (openrac::env::flag(openrac::env::Var::TraceRelocation)) {
            std::fprintf(stderr, "level %d's %08X in level %d: %08X\n", from, address, overlay, twin);
        }
    }
    map.searched.emplace(address, twin);
    return twin != 0 ? twin : address;
}

}  // extern "C"

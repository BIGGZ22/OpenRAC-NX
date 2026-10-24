// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac-<game>: one of the games, native. Every game's program is this
// file and those of program/, linked with that game's translated C and its description
// (openrac_game, generated from port/game/<id>/hostgen.json).
//
//   openrac-<id> --data <install>/active/<game>/data [--cards DIR] [--frames N] [--stop-on-missing]
//                [--level N] [--skip-movies] [--mods DIR] [--settings FILE] [--plugins FILE]
//                [--probes DIR] [--no-mods]
//
// The data folder is what the extractor made from the player's disc
// (tools/extractor.py; the launcher's "Set up from your disc"). The program
// does what the console's loader and the game's start-up code did, then runs
// the game's own main, translated from its decompilation:
//
//   1. game memory, laid out as the console's (port/runtime);
//   2. the executable's data copied to its addresses, from the player's copy
//      of it (its code bytes come along and are never run);
//   3. the game's functions registered by code address, with the level
//      program that is loaded deciding which ones a level address means;
//   4. the game's main.
//
// A function that has no C yet (still assembly in the decompilation, or a
// library not replaced yet) is logged and returns, and the game carries on;
// --stop-on-missing stops at the first one instead and says which, for checks.
//
// This file is the program's start: the command line read, the session's log opened, the
// executable loaded, the window and the sound opened, the mods loaded, the game's main run. The
// rest is in program/: options (the command line and default paths), vsync (each frame's end),
// sound, movie, levels (the level programs, --check-levels) and game_host (game_host.h's host
// side), with program.h for what they share.

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "common/build_info.h"
#include "common/crash.h"
#include "common/env.h"
#include "common/log.h"
#include "common/stats.h"
#include "common/system_info.h"
#include "mods.h"
#include "openrac/ee.h"
#include "openrac/elf.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "openrac/memory.h"
#include "openrac/sound_engine.h"
#include "program/program.h"

#ifdef OPENRAC_FRONTEND
#include "debug_overlay.h"
#include "enhanced/viewer/probe_lighting.h"
#include "frontend.h"
#include "platform/input.h"
#include "platform/pump.h"
#include "renderer/gl.h"
#include "settings.h"
#endif
#if defined(OPENRAC_ANDROID_ACTIVITY)
// The game as Android's activity loads it (libopenrac-<id>.so): main is SDL_main, which SDL's
// activity calls with the arguments the launcher gave it. The executable beside it
// (libopenrac-<id>-cli.so) keeps its main.
#include <SDL3/SDL_main.h>
#endif

extern "C" {
const char* openrac_game_disc_image = nullptr;
const char* openrac_game_card_dir = nullptr;
const char* openrac_game_mods_dir = nullptr;
int openrac_game_language = 1;
int openrac_game_no_card = 0;
int openrac_game_start_level = -1;
void (*openrac_game_on_frame)(unsigned frame) = nullptr;
int (*openrac_game_bolts)(void) = nullptr;
int (*openrac_game_skip_buttons)(void) = nullptr;
void (*openrac_game_skip)(void) = nullptr;
int (*openrac_game_mode)(uint32_t* mode) = nullptr;
int (*openrac_game_hero_position)(float xyz[3]) = nullptr;
int (*openrac_game_camera_position)(float xyz[3]) = nullptr;
int (*openrac_game_updating_moby)(openrac_game_moby* out) = nullptr;
int (*openrac_game_hero)(float position[3], float rotation[3], int32_t* state) = nullptr;
const openrac_front_end* openrac_game_front_end = nullptr;

// The hook points of this disc's build, for mods (hostgen's hooks.c; mods.h's HookPoint is its
// layout).
extern const openrac::mods::HookPoint openrac_game_hook_points[];
extern const int openrac_game_hook_point_count;
}

namespace {

namespace fs = std::filesystem;
using namespace openrac;
using namespace openrac::program;

}  // namespace

namespace openrac::program {

Options g_options;
long g_frame = 0;
gaddr g_chain = 0;
bool g_vsync_since_kick = true;
bool g_window = false;
std::chrono::steady_clock::time_point g_next_frame;
unsigned g_draws = 0;
bool g_reports_draws = false;

}  // namespace openrac::program

namespace {

// The moby being updated, asked by the report itself (common/crash.h, set_moby_source).
bool updating_moby(crash::Moby& out) {
    openrac_game_moby m{};
    if (openrac_game_updating_moby == nullptr || openrac_game_updating_moby(&m) == 0) {
        return false;
    }
    out.address = m.address;
    out.oclass = m.oclass;
    for (int i = 0; i < 3; ++i) {
        out.position[i] = m.position[i];
    }
    out.has_uid = m.uid >= 0;
    out.uid = m.uid;
    return true;
}

// This run's log (common/log.h: the earlier runs' moved along) and its header; crash reports go
// to <data>/crashes (common/crash.h).
void open_session(int argc, char** argv) {
    const long offset = crash::local_offset();
    char started[40];
    crash::format_time(static_cast<long long>(std::time(nullptr)), offset, started, sizeof(started));
    if (!log::open_session(g_options.data, program_name(), 5)) {
        log::warn("the log cannot be written in {}", g_options.data.string());
    }
    crash::configure(g_options.data / "crashes", program_name(), started);
    crash::set_moby_source(updating_moby);
    const char* level = log::level() <= log::Level::Debug     ? "debug"
                        : log::level() <= log::Level::Verbose ? "verbose"
                                                              : "info (OPENRAC_LOG=verbose, or verbose_log = 1 in the settings, for more)";
    log::header("==== OpenRAC session ====");
    log::header(std::format("session   {}", started));
    log::header(std::format("openrac   {}, commit {}", build_info::version(), build_info::commit()));
    log::header(std::format("game      {}: {}, disc {}", openrac_game.id, openrac_game.title, openrac_game.serial));
    log::header(std::format("system    {}, {} of memory", system_info::os(), system_info::memory()));
    log::header(std::format("cpu       {}", system_info::cpu()));
    log::header(std::format("command   {}", command_line(argc, argv)));
    log::header(std::format("log       {}", level));
    log::header("=========================");
}

// The keyboard and mouse as a mod reads them (openrac_mod.h's openrac_input), from the frontend's
// state for the tick about to run; null without a window.
const openrac_input* mod_input() {
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        const frontend::KeyboardMouseView view = frontend::keyboard_mouse();
        const auto* km = static_cast<const platform::KeyboardMouse*>(view.state);
        if (km != nullptr) {
            static openrac_input in{};
            in.size = sizeof in;
            in.tick = km->tick;
            static_assert(sizeof in.keys == platform::kKeyCount);
            std::memcpy(in.keys, km->keys.data(), sizeof in.keys);
            in.mouse_held = km->mouse_held;
            in.mouse_pressed = km->mouse_pressed;
            in.mouse_dx = km->mouse_dx;
            in.mouse_dy = km->mouse_dy;
            in.wheel_x = km->wheel_x;
            in.wheel_y = km->wheel_y;
            in.focused = km->focused ? 1 : 0;
            in.captured = km->captured ? 1 : 0;
            in.overlay = view.overlay ? 1 : 0;
            in.pointer_x = view.pointer_x;
            in.pointer_y = view.pointer_y;
            in.pointer_inside = view.pointer_inside ? 1 : 0;
            return &in;
        }
    }
#endif
    return nullptr;
}

// The native mods the launcher's list has on (mods.h, docs/port/MODS.md), bound to this disc's hook
// points (hostgen's hooks.c), once the game's functions are registered and before it runs; the
// session header says which loaded.
void load_mods() {
    mods::Host host;
    host.game = openrac_game.game;
    host.version = game_version();
    host.openrac_version = build_info::version();
    host.hooks = openrac_game_hook_points;
    host.hook_count = openrac_game_hook_point_count;
    host.list = g_options.plugins;
    host.purist = g_options.no_mods;
    host.loaded_level = openrac_game_loaded_overlay;
    host.pad_hook = &openrac_game_pad_report_hook;
    host.scripts = openrac_game_script_binding();  // generated (sdk/tools/gen_binding.py), or none
    host.input = mod_input;
#ifdef OPENRAC_FRONTEND
    host.take_input = [](std::uint32_t devices) {
        if ((devices & OPENRAC_INPUT_KEYBOARD) != 0 && g_window) {
            frontend::take_keyboard();
        }
    };
    host.capture_mouse = [](bool capture) {
        if (g_window) {
            frontend::capture_mouse(capture);
        }
    };
#endif
    // "2026-10-21 14:02:11 +0200" as "2026-10-21T14:02:11+02:00".
    char when[40];
    crash::format_time(static_cast<long long>(std::time(nullptr)), crash::local_offset(), when, sizeof(when));
    std::string now = when;
    if (now.size() == 25) {
        now = now.substr(0, 10) + "T" + now.substr(11, 8) + now.substr(20, 3) + ":" + now.substr(23, 2);
    }
    host.now = now;
    log::header(std::format("mods      {}", mods::load(host)));
    std::atexit([] { mods::unload(); });
}

}  // namespace

#if defined(_WIN32)
// winmm's timer resolution, without windows.h (its macros clash with the port's names).
extern "C" __declspec(dllimport) unsigned int __stdcall timeBeginPeriod(unsigned int);
#pragma comment(lib, "winmm.lib")
#endif

namespace {

// The game's code as the EE's floating point unit runs it: every operation rounds toward zero
// (the EE has no other mode). A counter the game steps by 0.1 lands on the other side of zero
// when rounded to nearest (a moby of rac1's level 6 shrank to a negative scale). Nor has it
// denormals: they are zero (docs/port/HARDWARE_SEMANTICS.md, item 1).
// OPENRAC_ROUND_NEAREST=1 and OPENRAC_DENORMALS=1 keep the host's ways, to compare.
int run_game_as_the_ee() {
    crash::prepare_thread();  // the game may run on a thread of its own (platform/pump.h)
    openrac_ee_float_mode(env::flag(env::Var::RoundNearest), env::flag(env::Var::Denormals));
    return openrac_game_run();
}

}  // namespace

int main(int argc, char** argv) {
#if defined(_WIN32)
    // Windows sleeps in steps of 15.6 ms unless a program asks for 1 ms: the wait for the next
    // 20 ms tick would overshoot by up to a step, and the frames stagger.
    timeBeginPeriod(1);
#endif
    g_options = parse(argc, argv);
    if (g_options.check_levels) {
#ifdef OPENRAC_FRONTEND
        return check_levels(g_options.levels.empty() ? default_levels(g_options.data) : levels_or_flat(g_options.levels));
#else
        log::fatalf("--check-levels needs a build with the window (it reads the levels as the window does)");
#endif
    }
    // The session's log first, so that a game that cannot start says why in it (on Android the
    // only place the player can read it, through the launcher).
    open_session(argc, argv);
    const fs::path iso = g_options.data / "iso_data" / openrac_game.game;
    const fs::path exe = iso / openrac_game.serial;
    const fs::path disc = iso / "disc.iso";
    if (!fs::exists(exe) || !fs::exists(disc)) {
        log::fatalf(
            "{} has no {} and disc.iso: set the game up from your disc first "
            "(python3 tools/extractor.py <image> --game {} --proj-path {})",
            iso.string(),
            openrac_game.serial,
            openrac_game.game,
            g_options.data.string()
        );
    }
    log::info("{}: {}", program_name(), openrac_game.title);

    runtime::Memory::create();
    runtime::install_crash_handler();
    // The threads reading a track are stopped before the program exits, whichever way it ends
    // (a window closed, a run of so many frames, an error): a reader that is still working
    // when the globals it uses go is a crash at quit.
    std::atexit(openrac_sound_stop_readers);

    std::vector<std::uint8_t> file;
    runtime::ElfImage image;
    std::string reason;
    if (!runtime::read_file(exe, &file) || !runtime::read_elf(file, &image, &reason)
        || !runtime::load_elf(file, image, &reason)) {
        log::fatalf("{}: {}", exe.string(), reason.empty() ? "cannot be read" : reason);
    }
    log::info(
        "{}: {} segments loaded, entry {:#x}",
        openrac_game.serial,
        image.segments.size(),
        image.entry
    );

    static std::string disc_path = disc.string();
    std::error_code made;
    fs::create_directories(g_options.cards, made);
    static std::string cards_path = g_options.cards.string();
    openrac_game_disc_image = disc_path.c_str();
    openrac_game_card_dir = cards_path.c_str();
    static std::string mods_path = g_options.mods.string();
    if (!mods_path.empty()) {
        openrac_game_mods_dir = mods_path.c_str();
        log::info("edited levels from {}", mods_path);
    }

#ifdef OPENRAC_FRONTEND
    if (g_options.window) {
        std::string why;
        settings::load(g_options.settings);
        log::header("settings  " + settings::summary(), false);
        if (settings::get().verbose_log && log::level() > log::Level::Verbose) {
            log::set_level(log::Level::Verbose);
            log::info("the settings ask for the detailed log (verbose_log = 1)");
        }
        // Enhanced rendering is chosen now, before the window makes its first shader: the
        // setting (the launcher's Mods entry), else OPENRAC_ENHANCED. Off, none of it is made.
        if (settings::get().enhanced_rendering) {
            gl::set_enhanced_rendering(true);
        }
        if (gl::enhanced_rendering_enabled()) {
            viewer::set_probe_root(g_options.probes);
            log::info("enhanced rendering on (Vulkan only); probe bakes from {}",
                      g_options.probes.empty() ? std::string("nowhere") : g_options.probes.string());
        }
        fs::path levels = g_options.levels.empty() ? default_levels(g_options.data) : levels_or_flat(g_options.levels);
        if (!frontend::open(openrac_game.id, levels, why)) {
            log::fatalf("no window: {}", why);
        }
        g_window = true;
        if (g_options.debug_overlay) {
            frontend::debug_overlay::request();
        }
        // The game's sounds (openrac/sound_engine.h), rendered on the output's thread; with
        // OPENRAC_AUDIO_TICKED, by the game's ticks instead (audio_tick).
        if (audio_ticked()) {
            openrac_stream_read_at_once(1);
            log::info("OPENRAC_AUDIO_TICKED: no sound device; the sounds advance with the game's ticks");
        } else {
            open_sound();
            std::atexit(close_sound);
        }
    }
#endif

    // [debug tools] --debug-dump FILE (or OPENRAC_DEBUG_DUMP=FILE): each frame's measurements, with
    // or without a window.
    if (g_options.debug_dump.empty()) {
        if (const std::optional<std::string_view> dump = env::value(env::Var::DebugDump)) {
            g_options.debug_dump = *dump;
        }
    }
    if (!g_options.debug_dump.empty()) {
        std::string why;
        if (stats::open_dump(g_options.debug_dump.string(), why)) {
            log::info("frame measurements are written to {}", g_options.debug_dump.string());
        } else {
            log::warn("--debug-dump: {}", why);
        }
    }

    openrac_game_register_functions();
    load_mods();
    openrac_guest_set_overlay_source(openrac_game_loaded_overlay);
    openrac_guest_set_level_programs(load_level_program);
    openrac_guest_stop_on_missing(g_options.keep_going ? 0 : 1);
    std::atexit(finish);

    // What the console's start-up code did before main: the stack and heap are
    // the runtime's, .bss was zeroed by the loader.
    int ran = 0;
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        // Beside the window's event pump, so the game goes on while the system holds the pump (a
        // window being dragged or sized on Windows: platform/pump.h).
        ran = platform::run_pumped([](void*) { return run_game_as_the_ee(); }, nullptr);
    } else {
        ran = run_game_as_the_ee();
    }
#else
    ran = run_game_as_the_ee();
#endif
    if (!ran) {
        log::error(
            "{}'s main is not known yet: port/game/{}/hostgen.json has no \"entry\"",
            openrac_game.title,
            openrac_game.id
        );
        return 2;
    }
    return 0;
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac-<game>: one of the games, native. Every game's program is this
// file, linked with that game's translated C and its description
// (openrac_game, generated from port/game/<id>/hostgen.json).
//
//   openrac-<id> --data <install>/active/<game>/data [--cards DIR] [--frames N] [--stop-on-missing]
//                [--level N] [--skip-movies] [--mods DIR] [--settings FILE] [--plugins FILE] [--probes DIR] [--no-mods]
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

#include <algorithm>
#include <cfenv>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include "common/build_info.h"
#include "common/crash.h"
#include "common/env.h"
#include "common/log.h"
#include "common/system_info.h"
#include "mods.h"
#ifdef OPENRAC_HAS_DISC
#include "assets/disc/disc.h"
#endif
#include "common/stats.h"
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
#include "platform/audio.h"
#include "platform/input.h"
#include "platform/performance_hint.h"
#include "platform/pump.h"
#include "renderer/direct.h"
#include "renderer/gl.h"
#include "renderer/texture.h"
#include "settings.h"
#include "viewer/level.h"
#include "viewer/moby_pose.h"
#endif
#ifdef OPENRAC_MOVIES
#include "media/movie.h"
#include "platform/audio.h"
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

struct Options {
    fs::path data;
    fs::path cards;
    long frames = -1;
    bool keep_going = true;
    bool window = false;
    bool skip_movies = false;  // development probes; not movie-playback validation
    fs::path levels;  // the extracted levels the window draws (level_00, ...)
    fs::path mods;  // levels edited in Godot and packed (editor/pack.py), read in place of the disc's
    fs::path settings;  // the player's settings (settings.h); in OpenRAC's data folder unless given
    fs::path plugins;   // the native mods' list (mods.h): <data>/plugins/plugins.json unless given
    fs::path probes;    // the player's probe bakes, a level_NN folder each (enhanced rendering)
    bool no_mods = false;  // purist mode: no native mod is loaded (docs/port/MODS.md)
    fs::path debug_dump;  // each frame's measurements, a JSON object a line (docs/port/DEBUGGING.md)
    bool debug_overlay = false;  // the debug overlay shown from the start (F3 shows and hides it)
    bool check_levels = false;  // read every extracted level as the window would, say what is wrong, and stop
};

Options g_options;

// Returns the value `var` was set to as a string of its own, or nothing when it is unset. For the
// reads below that hand the value to the C library (a script for sscanf, a file to write), which
// wants a string that ends in a NUL: env::value's view need not.
std::optional<std::string> env_string(env::Var var) {
    const std::optional<std::string_view> found = env::value(var);

    // An unset variable stays nothing, which differs from the empty string.
    if (!found) {
        return std::nullopt;
    }

    return std::string(*found);
}
}  // namespace

namespace openrac::program {

long g_frame = 0;
gaddr g_chain = 0;
bool g_vsync_since_kick = true;
bool g_window = false;

}  // namespace openrac::program

namespace {

std::chrono::steady_clock::time_point g_next_frame;
// The game's ticks a second: 60 for every version. The PAL game runs in its own 60 Hz mode
// (options_menu.cpp keeps it there), so its time base is a sixtieth of a second too.
int tick_rate() {
    return 60;
}

std::string program_name() {
    return std::string("openrac-") + openrac_game.id;
}

[[noreturn]] void usage(const std::string& why) {
    log::error("{}", why);
    log::error(
        "usage: {} --data <install>/active/{}/data [--cards DIR] [--frames N] [--stop-on-missing] [--level N] [--skip-movies] [--mods DIR] [--settings FILE] [--plugins FILE] [--probes DIR] [--no-mods] [--debug-overlay] [--debug-dump FILE]",
        program_name(),
        openrac_game.game
    );
    std::exit(64);
}

// OpenRAC's folder in the player's data home, as the launcher finds it
// (launcher/core/src/saves.rs, get_memcard_dir).
fs::path data_home() {
#if defined(__APPLE__)
    const std::optional<std::string> home = env::system_variable("HOME");
    fs::path base = fs::path(home.value_or(".")) / "Library/Application Support/OpenRAC";
#else
    fs::path base;
    if (const std::optional<std::string> xdg = env::system_variable("XDG_DATA_HOME");
        xdg && !xdg->empty()) {
        base = fs::path(*xdg) / "openrac";
    } else {
        const std::optional<std::string> home = env::system_variable("HOME");
        base = fs::path(home.value_or(".")) / ".local/share/openrac";
    }
#endif
    return base;
}

// Where the first memory card lives unless --cards says otherwise: the folder
// the launcher's save manager reads and backs up: <data home>/openrac/memcard/<serial>.
fs::path default_cards() {
    return data_home() / "memcard" / openrac_game.serial;
}

// The player's settings unless --settings says otherwise: <data home>/openrac/settings/<id>.ini.
fs::path default_settings() {
    return data_home() / "settings" / (std::string(openrac_game.id) + ".ini");
}

// This program's version: "rac1-pal" -> pal.
std::string game_version() {
    const std::string id = openrac_game.id;
    const auto dash = id.find('-');
    return dash == std::string::npos ? id : id.substr(dash + 1);
}

// Each version's levels sit in a folder of their own (<data>/port/<version>, the checkout's
// assets/port/<game>/<version>). PAL's levels extracted before that layout sit straight in the
// folder above (the only version that had them) and are read there until extracted again, whether
// the folder came from --levels or the default.
fs::path levels_or_flat(const fs::path& own) {
    std::error_code ec;
    const fs::path flat = own.parent_path();
    if (game_version() == "pal" && own.filename() == "pal" && !fs::exists(own / "level_99" / "manifest.json", ec) &&
        fs::exists(flat / "level_99" / "manifest.json", ec)) {
        log::warn("levels: reading the ones extracted before each version had a folder, in {} (extract again to "
                   "move them into {})",
                   flat.string(), own.string());
        return flat;
    }
    return own;
}

// Where the levels are without --levels.
fs::path default_levels(const fs::path& data) {
    return levels_or_flat(data / "port" / game_version());
}

Options parse(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        auto value = [&]() -> std::string {
            if (i + 1 >= argc) {
                usage("missing value for " + a);
            }
            return argv[++i];
        };
        if (a == "--data") {
            o.data = value();
        } else if (a == "--cards") {
            o.cards = value();
        } else if (a == "--no-card") {
            openrac_game_no_card = 1;
        } else if (a == "--window") {
            o.window = true;
        } else if (a == "--mods") {
            o.mods = value();
        } else if (a == "--settings") {
            o.settings = value();
        } else if (a == "--plugins") {
            o.plugins = value();
        } else if (a == "--probes") {
            o.probes = value();
        } else if (a == "--no-mods") {
            o.no_mods = true;
        } else if (a == "--levels") {
            o.levels = value();
        } else if (a == "--frames") {
            o.frames = std::stol(value());
        } else if (a == "--level") {
            openrac_game_start_level = std::stoi(value());
        } else if (a == "--skip-movies") {
            o.skip_movies = true;
        } else if (a == "--keep-going") {
            o.keep_going = true;
        } else if (a == "--stop-on-missing") {
            o.keep_going = false;
        } else if (a == "--debug-dump") {
            o.debug_dump = value();
        } else if (a == "--debug-overlay") {
            o.debug_overlay = true;
        } else if (a == "--check-levels") {
            o.check_levels = true;
        } else if (a == "--help" || a == "-h") {
            usage(program_name() + ": " + openrac_game.title + ", native");
        } else {
            usage("unknown option " + a);
        }
    }
    if (o.data.empty()) {
        if (const std::optional<std::string_view> data = env::value(env::Var::Data)) {
            o.data = *data;
        } else {
            usage(
                std::string("no data folder: pass --data, the folder the extractor wrote for ")
                + openrac_game.game
            );
        }
    }
    if (o.cards.empty()) {
        o.cards = default_cards();
    }
    if (o.settings.empty()) {
        o.settings = default_settings();
    }
    if (o.plugins.empty()) {
        o.plugins = o.data / "plugins" / "plugins.json";
    }
    return o;
}

// The command line as it was typed, quoted where needed, with the home folder as ~.
std::string command_line(int argc, char** argv) {
    std::string line;
    for (int i = 0; i < argc; ++i) {
        std::string a = i == 0 ? program_name() : std::string(argv[i]);
        if (a.find_first_of(" \"'") != std::string::npos || a.empty()) {
            a = "\"" + a + "\"";
        }
        line += (i == 0 ? "" : " ") + a;
    }
    return line;
}

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

// What a crash report says the game was doing (common/crash.h): sampled each frame.
void sample_state() {
    crash::GameState s;
    s.frame = g_frame;
    const int level = openrac_game_loaded_overlay();
    s.level = level == OPENRAC_OVERLAY_EXE ? -1 : level;
    if (openrac_game_mode != nullptr) {
        s.has_mode = openrac_game_mode(&s.mode) != 0;
    }
    if (openrac_game_hero_position != nullptr) {
        s.has_hero = openrac_game_hero_position(s.hero) != 0;
    }
    if (openrac_game_camera_position != nullptr) {
        s.has_camera = openrac_game_camera_position(s.camera) != 0;
    }
    crash::set_state(s);
}

// [testing only] OPENRAC_CRASH_TEST=KIND:FRAME stops the program on purpose at that frame, to check
// the crash report: segv (an access to game memory that is not mapped), host (to a host address
// that is not), abort, fpe (a raised SIGFPE), fatal (log::fatal), trap (the game's trap).
void crash_test() {
    static const std::optional<std::string> test = env_string(env::Var::CrashTest);

    // Unset, no test is asked for.
    if (!test) {
        return;
    }

    char kind[16] = {};
    long frame = 0;
    if (std::sscanf(test->c_str(), "%15[a-z]:%ld", kind, &frame) < 1 || g_frame != frame) {
        return;
    }
    log::warn("OPENRAC_CRASH_TEST: stopping on purpose ({})", kind);
    const std::string k = kind;
    if (k == "segv") {
        *static_cast<volatile std::uint32_t*>(G(0x0F000000u)) = 1;
    } else if (k == "host") {
        *reinterpret_cast<volatile std::uint32_t*>(static_cast<std::uintptr_t>(16)) = 1;
    } else if (k == "abort") {
        std::abort();
    } else if (k == "fpe") {
        std::raise(SIGFPE);
    } else if (k == "fatal") {
        log::fatalf("OPENRAC_CRASH_TEST: a fatal error on purpose");
    } else if (k == "trap") {
        openrac_game_crash("the game raised a trap (a failed assertion of its own): OPENRAC_CRASH_TEST");
    }
}

// OPENRAC_REACH=FILE: the functions without C (and the zero-filled calls) this run reached, with
// their calls (guest.h, openrac_reach_site; port/tools/regress reach). Written at exit and every
// 600 frames, so a run that crashes still leaves most of it.
void write_reach() {
    static const std::optional<std::string> path = env_string(env::Var::Reach);
    if (path && !path->empty()
        && openrac_guest_write_reach(path->c_str(), openrac_game.id, g_frame) == 0) {
        log::warn("OPENRAC_REACH: cannot write {}", *path);
    }
}

// OPENRAC_AUDIO_TICKED=1: no sound device. The game's sounds advance by one tick's samples each
// tick (rendered and dropped), and streams are read when they start, so every answer the game
// gets back from the sound engine (a voice's state, a stream's position) lands on the same tick, however
// fast the machine runs it: port/tools/regress's runs repeat. A device pulls at the real clock's
// rate, which an uncapped run outpaces by a varying amount.
bool audio_ticked() {
    return env::flag(env::Var::AudioTicked);
}

void audio_tick() {
#ifdef OPENRAC_FRONTEND
    if (!audio_ticked() || !g_window) {
        return;
    }
    static std::vector<std::int16_t> samples(static_cast<std::size_t>(platform::kAudioRate / tick_rate()) * 2);
    openrac_sound_render(samples.data(), samples.size() / 2);
#endif
}

void finish() {
    write_reach();
    openrac_guest_report_missing();
    log::info("{} frames", g_frame);
}

#ifdef OPENRAC_FRONTEND
// The game's sounds (openrac/sound_engine.h), rendered on the output's thread.
std::unique_ptr<platform::AudioOutput> g_sound;
long g_sound_retry_frame = 0;

// Every way out runs this (at exit, or in leave): the output's thread stops before the globals it
// renders from go (its callback ran on into their destruction, a crash at quit).
void close_sound() {
    g_sound.reset();
}

// The window was closed: the player quit, or Android ended the game's activity (closed from
// Recents, or the launcher's Stop). On a desktop that is std::exit. On Android the game is often
// in the background then, its surface already gone, and exit's destructors would tear the renderer
// and the window down against a driver that lost it, while SDL's onDestroy frees its own state a
// second after asking the game to quit; a close from Recents was written down as a crash. There
// the game runs exit's handlers itself and ends at once; the system frees the rest.
[[noreturn]] void leave() {
#if defined(__ANDROID__)
    finish();
    stats::close_dump();
    close_sound();
    openrac_sound_stop_readers();
    std::_Exit(0);
#else
    // The threads reading a track are stopped first, while the globals they use are still
    // there: a reader that is still working when the program exits is a crash at quit.
    openrac_sound_stop_readers();
    std::exit(0);
#endif
}

// REPORT: whether a device that cannot be opened is logged (the first try is; the retries,
// every few seconds on a machine without sound, are not).
void open_sound(bool report = true) {
    std::string why;
    g_sound = platform::AudioOutput::open(why, [](std::span<std::int16_t> samples) {
        openrac_sound_render(samples.data(), samples.size() / 2);
        // The player's volume (settings.h): at 0 the output is silence itself.
        if (const float gain = settings::output_gain(); gain < 1.0f) {
            for (std::int16_t& sample : samples) {
                sample = static_cast<std::int16_t>(static_cast<float>(sample) * gain);
            }
        }
    });
    if (!g_sound && report) {
        log::warn("no sound: {}", why);
    }
}

// Once a frame: a device that stalled is opened again, and one that could not be opened is tried
// again every few seconds (a phone's audio can be busy with a call or another app at the start).
void watch_sound() {
    if (!g_window || audio_ticked()) {
        return;
    }
    if (!g_sound) {
        if (g_frame >= g_sound_retry_frame) {
            g_sound_retry_frame = g_frame + 300;
            open_sound(false);
            if (g_sound) {
                log::info("sound is back");
            }
        }
        return;
    }
    std::string why;
    static bool reported = false;
    if (!g_sound->watch(why)) {
        if (!reported) {
            log::warn("sound stopped and the device could not be opened again: {}", why);
            reported = true;
        }
    } else if (reported) {
        log::info("sound is back");
        reported = false;
    }
}
#endif

}  // namespace

namespace openrac::program {

unsigned g_draws = 0;
bool g_reports_draws = false;

// OPENRAC_WRITE=FRAME:ADDRESS:VALUE[:FRAMES],...: a 32-bit word written into the game's memory at
// each vertical blank from FRAME for FRAMES blanks (default 1), hexadecimal address and value; the
// old interpreter's --write, for getting somewhere directly (0x15EE84, the level being played, set
// around a new game's transition loads that level). cFRAME counts the window's frames from control
// instead, as OPENRAC_PRESS's c entries do (port/tools/regress: the same moment in every version).
static void apply_writes() {
    static const std::optional<std::string> script = env_string(env::Var::Write);

    // Unset, nothing is written.
    if (!script) {
        return;
    }

    std::int64_t since = -1;
#ifdef OPENRAC_FRONTEND
    since = g_window ? frontend::since_control() : -1;
#endif
    for (const char* at = script->c_str(); at && *at;) {
        unsigned long frame = 0, address = 0, value = 0, length = 1;
        const bool from_control = *at == 'c';
        const std::int64_t now = from_control ? since : static_cast<std::int64_t>(g_frame);
        if (std::sscanf(at + (from_control ? 1 : 0), "%lu:%lx:%lx:%lu", &frame, &address, &value, &length) >= 3
            && now >= 0 && now >= static_cast<std::int64_t>(frame)
            && now < static_cast<std::int64_t>(frame + length)) {
            const std::uint32_t word = static_cast<std::uint32_t>(value);
            std::memcpy(G(static_cast<gaddr>(address)), &word, 4);
        }
        at = std::strchr(at, ',');
        at = at ? at + 1 : nullptr;
    }
}

// [debug tools] The game's tick length, stretched by the overlay's slow motion (debug_overlay.h).
static std::chrono::microseconds tick_period() {
    auto period = std::chrono::microseconds(1000000 / tick_rate());
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        period = std::chrono::microseconds(
            std::llround(static_cast<double>(period.count()) * frontend::debug_overlay::tick_stretch()));
    }
#endif
    return period;
}

int vsync() {
    // [debug tools] The game's own code stops here and goes on when this returns: its time is the
    // tick's (common/stats.h). The tick's frame of measurements begins.
    stats::game_yielded();
    struct Resume {
        ~Resume() { stats::game_resumed(); }
    } resume;
    stats::begin_frame(stats::FrameKind::Tick, static_cast<std::uint64_t>(g_frame));
    g_vsync_since_kick = true;
    apply_writes();
    // With a window, the renderer draws the frame from the game's memory; without one, a frame is
    // only paced, at the game's frame rate.
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        // The tick about to be drawn was due at the time the last one set (frames between ticks
        // count from it).
        const auto period = tick_period();
        const auto now = std::chrono::steady_clock::now();
        // How late the tick is, for the window to leave a frame out rather than fall further behind
        // (not after a long stop, a level loading: the count below starts again from now).
        const bool counted = g_next_frame.time_since_epoch().count() != 0
                             && now <= g_next_frame + std::chrono::milliseconds(100);
        frontend::set_tick(g_next_frame.time_since_epoch().count() == 0 || now > g_next_frame + period
                               ? now : g_next_frame, period,
                           counted && now > g_next_frame ? std::chrono::duration<double>(now - g_next_frame)
                                                         : std::chrono::duration<double>(0.0));
        const auto* ram = runtime::Memory::get().base();
        const std::uint32_t draws = g_reports_draws ? g_draws : ~0u;
        g_draws = 0;
        if (!frontend::frame(std::span<const std::uint8_t>(ram, 32u * 1024 * 1024), g_chain, draws)) {
            leave();
        }
        watch_sound();
    }
#endif
    g_frame++;
    audio_tick();
    sample_state();
    crash_test();
    mods::frame();
    // The level being played, once per change: the launcher reads this line for Discord's presence.
    static int shown_level = OPENRAC_OVERLAY_EXE;
    static int shown_bolts = -1;
    static long bolts_frame = 0;
    if (const int level = openrac_game_loaded_overlay(); level != shown_level) {
        shown_level = level;
        shown_bolts = -1;
        if (level != OPENRAC_OVERLAY_EXE) {
            log::info("now playing level {}", level);
        }
    }
    // Its bolts too, when they change, at most every 600 frames: Discord takes few updates.
    if (shown_level != OPENRAC_OVERLAY_EXE && openrac_game_bolts != nullptr &&
        (shown_bolts < 0 || g_frame >= bolts_frame + 600)) {
        if (const int bolts = openrac_game_bolts(); bolts >= 0 && bolts != shown_bolts) {
            shown_bolts = bolts;
            bolts_frame = g_frame;
            log::info("bolts {}", bolts);
        }
    }
    if (openrac_game_on_frame != nullptr) {
        openrac_game_on_frame(static_cast<unsigned>(g_frame));
    }
    if (g_frame % 600 == 0) {
        write_reach();
    }
    if (g_options.frames >= 0 && g_frame >= g_options.frames) {
        std::exit(0);  // finish() runs at exit
    }
    using namespace std::chrono;
    const auto now = steady_clock::now();
    // OPENRAC_FPS: the frames per second over each 300 frames, in the log. OPENRAC_UNCAPPED: no wait
    // for the next frame's time (the game then runs as fast as the machine allows: a measure, not a
    // way to play, since the game's tick is its frame).
    static const bool& fps = stats::probe("OPENRAC_FPS", "the ticks per second over each 300 ticks, in the log");
    static const bool uncapped = env::flag(env::Var::Uncapped);
    static steady_clock::time_point fps_from = now;
    // OPENRAC_WATCH=ADDRESS:COUNT[:i],...: COUNT floats (or words, with i) from each ADDRESS, each
    // frame, in the log.
    static const std::optional<std::string> watch = env_string(env::Var::Watch);
    if (watch) {
        std::string line;
        for (const char* at = watch->c_str(); at && *at;) {
            unsigned long address = 0, count = 1;
            char kind = 'f';
            std::sscanf(at, "%lx:%lu:%c", &address, &count, &kind);
            for (unsigned long i = 0; i < count && i < 32; ++i) {
                std::uint32_t w;
                std::memcpy(&w, G(static_cast<gaddr>(address + 4 * i)), 4);
                float f;
                std::memcpy(&f, &w, 4);
                line += kind == 'i' ? std::format(" {}", static_cast<std::int32_t>(w)) : std::format(" {:.4f}", f);
            }
            line += " |";
            at = std::strchr(at, ',');
            at = at ? at + 1 : nullptr;
        }
        log::info("frame {} watch{}", g_frame, line);
    }
    // OPENRAC_GUARD=FRAME:ADDRESS:BYTES[:SKIP]: from FRAME on, the first write (after SKIP) into
    // those bytes of game memory ends the program with the host's stack (runtime::guard_writes), to find who corrupts it.
    static const std::optional<std::string> guard = env_string(env::Var::Guard);
    if (guard) {
        unsigned long from = 0, address = 0, bytes = 4, skip = 0;
        std::sscanf(guard->c_str(), "%lu:%lx:%lx:%lu", &from, &address, &bytes, &skip);
        if (g_frame >= from) {
            runtime::guard_writes(static_cast<gaddr>(address), static_cast<std::uint32_t>(bytes),
                                  static_cast<std::uint32_t>(skip));
        }
    }
    // OPENRAC_HITCH: each frame that took more than twice the frame time, with how long, in the log.
    static const bool& hitch = stats::probe("OPENRAC_HITCH", "each tick that took over 1.25 tick times, in the log");
    static steady_clock::time_point last = now;
    if (hitch) {
        const double ms = duration<double, std::milli>(now - last).count();
        if (ms > 1.25 * 1000.0 / tick_rate()) {
            log::info("frame {}: {:.1f} ms", g_frame, ms);
        }
    }
    last = now;
    if (fps && g_frame % 300 == 0) {
        const double seconds = duration<double>(now - fps_from).count();
        if (seconds > 0.0) {
            log::info("frame {}: {:.1f} frames per second", g_frame, 300.0 / seconds);
        }
        fps_from = now;
    }
    if (uncapped) {
        return static_cast<int>(g_frame & 1);
    }
    if (g_next_frame.time_since_epoch().count() == 0 || now > g_next_frame + milliseconds(100)) {
        g_next_frame = now;
    }
    const auto period = tick_period();
    g_next_frame += period;
#ifdef OPENRAC_FRONTEND
    // Until the next tick is due, frames at the display's rate (each swap waits for a refresh),
    // the scene moved on from the last tick's to this one's by when each is shown.
    if (g_window && frontend::wants_between()) {
        for (int drawn; (drawn = frontend::between(g_next_frame)) != 0;) {
            if (drawn < 0) {
                leave();
            }
        }
    }
#endif
    {
        const stats::Scope wait("wait");  // [debug tools]
        std::this_thread::sleep_until(g_next_frame);
    }
#ifdef OPENRAC_FRONTEND
    openrac::platform::work_resumed();  // the next tick's work counts from here (performance_hint.h)
    // [debug tools] A paused game stays here, its last tick drawn again (frontend.h, hold).
    if (g_window && !frontend::hold()) {
        leave();
    }
#endif
    return static_cast<int>(g_frame & 1);
}

/*
 * The movie player, after ReRAC's movie mode (crates/rc-game/src/movie_player.rs,
 * crates/rc-engine/src/movie_render.rs; ISC License, Copyright (c) 2026 ReRAC contributors):
 * the audio is the clock. Each vertical blank hands the device one blank's worth of the movie's
 * 48 kHz audio, and the picture shown is the one whose time holds the sample being heard (what has
 * been handed over less what the device still holds), so picture and sound cannot drift apart.
 * When the video ends (or is skipped) the last picture stays up for the console's FadeToBlack(4).
 */
int play_movie(uint32_t lsn, uint32_t bytes, int channel, int start_skips) {
    if (g_options.skip_movies) {
        log::info("movie at sector {}: skipped by --skip-movies (development probe)", lsn);
        return 1;  // the same result as the movie player's Start-skip path
    }
#if defined(OPENRAC_FRONTEND) && defined(OPENRAC_MOVIES)
    if (!g_window || openrac_game_disc_image == nullptr || bytes == 0) {
        return 0;
    }
    std::vector<std::uint8_t> file(bytes);
    {
        std::FILE* disc = std::fopen(openrac_game_disc_image, "rb");
        if (disc == nullptr) {
            return 0;
        }
#if defined(_WIN32)
        // Windows' C library has no fseeko; _fseeki64 is its 64-bit seek.
        const bool sought = _fseeki64(disc, static_cast<long long>(lsn) * 2048, SEEK_SET) == 0;
#else
        const bool sought = fseeko(disc, static_cast<off_t>(lsn) * 2048, SEEK_SET) == 0;
#endif
        const bool read = sought && std::fread(file.data(), 1, file.size(), disc) == file.size();
        std::fclose(disc);
        if (!read) {
            log::warn("movie at sector {}: the disc image is too short", lsn);
            return 0;
        }
    }
    std::optional<media::MoviePlayer> player;
    try {
        player.emplace(media::Movie::open(file, static_cast<std::uint8_t>(channel)));
    } catch (const std::exception& e) {
        log::warn("movie at sector {}: {}", lsn, e.what());
        return 0;
    }
    const media::VideoSequence& sequence = player->movie().sequence();
    log::info(
        "movie at sector {}: {}x{}, {}/{} fps, audio channel {}", lsn, sequence.width, sequence.height,
        sequence.fps_num, sequence.fps_den, player->movie().audio_channel() ? int(*player->movie().audio_channel()) : -1
    );
    std::string error;
    auto output = platform::AudioOutput::open(error);
    if (!output) {
        log::warn("movie: no sound ({})", error);
    }
    const int rate = tick_rate();
    const std::size_t per_blank = static_cast<std::size_t>(platform::kAudioRate / rate);
    std::vector<std::int16_t> samples(per_blank * 2);
    std::vector<std::uint8_t> rgba;
    std::shared_ptr<const media::VideoFrame> shown;
    std::uint16_t previous = 0xFFFF;
    int skipped = 0;
    auto pace = [&] {
        using namespace std::chrono;
        const auto now = steady_clock::now();
        if (g_next_frame.time_since_epoch().count() == 0 || now > g_next_frame + milliseconds(100)) {
            g_next_frame = now;
        }
        g_next_frame += microseconds(1000000 / rate);
        std::this_thread::sleep_until(g_next_frame);
        g_frame++;
    };
    for (std::uint64_t blank = 0;; ++blank) {
        if (output) {
            output->set_gain(settings::output_gain());
            const std::size_t got = player->read_audio(samples);
            if (got > 0) {
                output->queue(std::span<const std::int16_t>(samples.data(), got * 2));
            }
        }
        const std::uint64_t handed = (blank + 1) * per_blank;
        const std::uint64_t held = output ? static_cast<std::uint64_t>(output->queued_frames()) : 0;
        const double heard = static_cast<double>(handed > held ? handed - held : 0) / platform::kAudioRate;
        auto frame = player->frame_at(heard);
        if (frame) {
            shown = frame;
        }
        if (player->video_finished()) {
            break;
        }
        if (shown) {
            media::frame_to_rgba(*shown, rgba);
            if (!frontend::show_picture(rgba.data(), int(shown->width), int(shown->height), 0.0f)) {
                leave();
            }
        }
        // Start skips when pressed during the movie: what is held as it starts is not a press.
        std::uint16_t buttons = 0xFFFF;
        std::uint8_t analog[4];
        openrac_game_pad_through_mods(0, &buttons, analog);  // a mod's key bound to Start skips too
        if (blank == 0) {
            previous = buttons;
        }
        const bool start_now = (buttons & 0x0008) == 0;
        const bool start_before = (previous & 0x0008) == 0;
        previous = buttons;
        if (start_skips && start_now && !start_before) {
            log::info("movie at sector {}: Start pressed (pad {:#06x})", lsn, buttons);
            skipped = 1;
            break;
        }
        pace();
    }
    // FadeToBlack(4) over the last picture, then its closing black blank.
    for (int step = 0; step <= 4 && shown; ++step) {
        frontend::show_picture(rgba.data(), int(shown->width), int(shown->height), step >= 4 ? 1.0f : float(step + 1) / 4.0f);
        pace();
    }
    if (output) {
        output->pause(true);
    }
    log::info("movie at sector {}: {}", lsn, skipped ? "skipped" : "played");
    return skipped;
#else
    (void)lsn;
    (void)bytes;
    (void)channel;
    (void)start_skips;
    return 0;
#endif
}

}  // namespace openrac::program

// A level's program straight from the player's image (<data>/iso_data/<game>/disc.iso, kept by the
// set-up), for rac1, whose disc layout is known. The image is opened once.
static bool level_program_from_disc(int level, std::vector<std::uint8_t>* bytes) {
#ifdef OPENRAC_HAS_DISC
    static std::optional<assets::disc::Disc> disc;
    static bool tried = false;
    if (!tried) {
        tried = true;
        const fs::path image = g_options.data / "iso_data" / openrac_game.game / "disc.iso";
        if (std::string(openrac_game.game) == "rac1") {
            try {
                disc.emplace(assets::disc::Disc::open(image, assets::disc::rac1_layout()));
            } catch (const std::exception& e) {
                log::warn("the level programs cannot be read from {}: {}", image.string(), e.what());
            }
        }
    }
    if (!disc) {
        return false;
    }
    try {
        assets::disc::LevelFiles files = disc->level(static_cast<std::uint32_t>(level));
        if (files.overlay) {
            *bytes = std::move(*files.overlay);
            return true;
        }
    } catch (const std::exception& e) {
        log::warn("level {}'s program cannot be read from the disc image: {}", level, e.what());
    }
#else
    (void)level;
    (void)bytes;
#endif
    return false;
}

// A level's program: the pieces the game's ParseBin copies in, each a header (load address, size,
// a word, entry) and its bytes, while the entry stays the same. For the runtime's level-to-level
// relocation: a function several levels share reads its globals where the loaded level keeps
// them, which the runtime finds by pairing the function's code in the two programs. Without it
// such a function reads the address of the level its name gives, where the loaded level has
// something else: the arctangent's tables were zero in a level, so the camera's angles were zero
// and the stick moved the hero along the world's axes whatever way the camera faced.
// From the extractor's copy when there is one (decompiler_out/<game>/levels/NN/overlay.bin), else
// from the player's image.
static void load_level_program(int level) {
    char name[8];
    std::snprintf(name, sizeof(name), "%02d", level);
    const fs::path path =
        g_options.data / "decompiler_out" / openrac_game.game / "levels" / name / "overlay.bin";
    std::vector<std::uint8_t> bytes;
    if (!runtime::read_file(path, &bytes) && !level_program_from_disc(level, &bytes)) {
        log::error("level {}'s program is in neither {} nor the disc image: the functions levels share are not "
                   "relocated, and the game will misbehave (the hero's heading ignores the camera)",
                   level, path.string());
        return;
    }
    log::verbose("level {}'s program: {} bytes", level, bytes.size());
    std::uint32_t entry = 0;
    for (std::size_t at = 0; at + 16 <= bytes.size();) {
        std::uint32_t head[4];
        std::memcpy(head, bytes.data() + at, sizeof(head));
        if (entry == 0) {
            entry = head[3];
        }
        if (head[3] != entry || at + 16 + head[1] > bytes.size()) {
            break;
        }
        openrac_guest_add_level_program(level, head[0], bytes.data() + at + 16, head[1]);
        at += 16 + head[1];
    }
}

#if defined(_WIN32)
// winmm's timer resolution, without windows.h (its macros clash with the port's names).
extern "C" __declspec(dllimport) unsigned int __stdcall timeBeginPeriod(unsigned int);
#pragma comment(lib, "winmm.lib")
#endif

#ifdef OPENRAC_FRONTEND
// --check-levels: every level the window draws from (level_00 to level_18, the flight world 98 and
// the title's 99) is read as the window reads it, without a window. A level that cannot be read,
// or that has nothing to draw, is named with the reason; when all are whole, `verified-<format>.json`
// (the extracted levels' format, viewer::kManifestFormat) is written beside them, which the launcher
// asks for before it offers Play. A game started over levels that are missing or cut short shows
// its HUD over a black world and says nothing else; levels of an older format fail the check (their
// manifests are refused), so the launcher reads them from the disc again.
static int check_levels(const fs::path& levels) {
    const fs::path stamp = levels / std::format("verified-{}.json", viewer::kManifestFormat);
    std::error_code ignored;
    fs::remove(stamp, ignored);
    fs::remove(levels / "verified.json", ignored);  // the stamp before formats were named
    std::vector<int> ids;
    for (int i = 0; i < 19; ++i) {
        ids.push_back(i);
    }
    ids.push_back(98);
    ids.push_back(99);
    int bad = 0;
    int newer = 0;  // levels of a newer format than this game reads: the game is out of date
    for (const int id : ids) {
        const fs::path dir = levels / std::format("level_{:02d}", id);
        viewer::LevelData level;
        std::string error;
        if (!viewer::load_level(dir, level, error)) {
            log::error("level {:02d}: {}", id, error);
            ++bad;
            newer += error.find("newer than this game") != std::string::npos;
            continue;
        }
        // The flight world has only its ships' textures; every other one has ground to stand on.
        if (id != 98 && (level.vertices.empty() || level.models.empty())) {
            log::error("level {:02d}: it has nothing to draw ({} models, {} vertices)", id, level.models.size(),
                       level.vertices.size());
            ++bad;
            continue;
        }
        if (level.missing_images > 0) {
            log::error("level {:02d}: {} of its textures are missing", id, level.missing_images);
            ++bad;
            continue;
        }
        log::info("level {:02d}: {} models, {} vertices, {} textures", id, level.models.size(), level.vertices.size(),
                  level.images.size());
    }
    if (newer > 0) {
        log::error("{} of {} levels in {} are of a newer format than this game reads: build the game again (the "
                   "launcher's \"Build the game\"; tools/build_port.py)",
                   newer, ids.size(), levels.string());
        return 1;
    }
    if (bad > 0) {
        log::error("{} of {} levels in {} cannot be drawn: read them from the disc again (the launcher's "
                   "\"Get the levels ready\"; editor/extract.py port --replace)",
                   bad, ids.size(), levels.string());
        return 1;
    }
    if (std::FILE* f = std::fopen(stamp.string().c_str(), "wb")) {
        const std::string text = std::format(
            "{{\"format\": {}, \"levels\": {}, \"program\": \"{}\"}}\n",
            viewer::kManifestFormat,
            ids.size(),
            program_name()
        );
        std::fwrite(text.data(), 1, text.size(), f);
        std::fclose(f);
    } else {
        log::error("cannot write {}", stamp.string());
        return 1;
    }
    log::info("all {} levels can be drawn: {}", ids.size(), stamp.string());
    return 0;
}
#endif

// The game's code as the EE's floating point unit runs it: every operation rounds toward zero
// (the EE has no other mode). A counter the game steps by 0.1 lands on the other side of zero
// when rounded to nearest (a moby of rac1's level 6 shrank to a negative scale). Nor has it
// denormals: they are zero (docs/port/HARDWARE_SEMANTICS.md, item 1).
// OPENRAC_ROUND_NEAREST=1 and OPENRAC_DENORMALS=1 keep the host's ways, to compare.
static int run_game_as_the_ee() {
    crash::prepare_thread();  // the game may run on a thread of its own (platform/pump.h)
    openrac_ee_float_mode(env::flag(env::Var::RoundNearest), env::flag(env::Var::Denormals));
    return openrac_game_run();
}

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

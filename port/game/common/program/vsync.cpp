// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The game's vertical blank (program::vsync, openrac_game_vsync's body in program/game_host.cpp):
// the end of each of the game's frames, where the program does its per-frame work. With a window
// the frame is drawn and paced to the display; without one it is only paced. Each frame also
// samples what a crash report says, runs the mods, logs the level and bolts the launcher reads,
// applies the debugging variables (OPENRAC_WRITE, _WATCH, _GUARD, _FPS, _HITCH, _CRASH_TEST,
// docs/port/DEBUGGING.md), and ends the program at --frames or when the window closes (finish,
// leave).

#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>

#include "common/crash.h"
#include "common/env.h"
#include "common/log.h"
#include "common/stats.h"
#include "mods.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "openrac/memory.h"
#include "openrac/sound_engine.h"
#include "program/program.h"

#ifdef OPENRAC_FRONTEND
#include "debug_overlay.h"
#include "frontend.h"
#include "platform/performance_hint.h"
#endif

namespace openrac::program {
namespace {

// ---- Each frame's helpers ----

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
        openrac_game_crash(
            "the game raised a trap (a failed assertion of its own): OPENRAC_CRASH_TEST"
        );
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

// OPENRAC_WRITE=FRAME:ADDRESS:VALUE[:FRAMES],...: a 32-bit word written into the game's memory at
// each vertical blank from FRAME for FRAMES blanks (default 1), hexadecimal address and value; the
// old interpreter's --write, for getting somewhere directly (0x15EE84, the level being played, set
// around a new game's transition loads that level). cFRAME counts the window's frames from control
// instead, as OPENRAC_PRESS's c entries do (port/tools/regress: the same moment in every version).
void apply_writes() {
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

        if (std::sscanf(
                at + (from_control ? 1 : 0), "%lu:%lx:%lx:%lu", &frame, &address, &value, &length
            ) >= 3
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
std::chrono::microseconds tick_period() {
    auto period = std::chrono::microseconds(1000000 / tick_rate());
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        period = std::chrono::microseconds(
            std::llround(
                static_cast<double>(period.count()) * frontend::debug_overlay::tick_stretch()
            )
        );
    }
#endif
    return period;
}

}  // namespace

// ---- The clock and the end ----

int tick_rate() {
    return 60;
}

void finish() {
    write_reach();
    openrac_guest_report_missing();
    log::info("{} frames", g_frame);
}

#ifdef OPENRAC_FRONTEND
[[noreturn]] void leave() {
    // The sound's stream goes before the window, whose end is SDL's (Platform: SDL_Quit). Left to
    // the exit, its destructor freed the stream into SDL's freed state: the heap was corrupted
    // and the program faulted on its way out.
    close_sound();
    // Join rendering while the window, profiler and GPU owners still exist.
    if (g_window) {
        frontend::close();
    }
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
#endif

// ---- The vertical blank ----

int vsync() {
    // [debug tools] The game's own code stops here and goes on when this returns: its time is the
    // tick's (common/stats.h). The tick's frame of measurements begins.
    stats::game_yielded();
    // Render snapshots own their values, so next-tick writes precede the overlapping CPU capture.
    apply_writes();
#ifdef OPENRAC_FRONTEND
    // Prepare owned CPU data before waiting; graphics and profiling storage still transfer at completion.
    if (g_window) {
        const auto* ram = runtime::Memory::get().base();
        const std::span<const std::uint8_t> memory(ram, GUEST_RAM_SIZE);

        // Only a completed handoff may present or begin the next graphics measurement frame.
        if (!frontend::prepare_frame(memory) || !frontend::finish_render()) {
            leave();
        }
    }
#endif

    struct Resume {
        ~Resume() { stats::game_resumed(); }
    } resume;

    stats::begin_frame(stats::FrameKind::Tick, static_cast<std::uint64_t>(g_frame));
    g_vsync_since_kick = true;
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
        frontend::set_tick(
            g_next_frame.time_since_epoch().count() == 0 || now > g_next_frame + period
                ? now
                : g_next_frame,
            period,
            counted && now > g_next_frame ? std::chrono::duration<double>(now - g_next_frame)
                                          : std::chrono::duration<double>(0.0)
        );
        const auto* ram = runtime::Memory::get().base();
        const std::uint32_t draws = g_reports_draws ? g_draws : ~0u;
        g_draws = 0;

        if (!frontend::frame(
                std::span<const std::uint8_t>(ram, 32u * 1024 * 1024), g_chain, draws
            )) {
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
    if (shown_level != OPENRAC_OVERLAY_EXE && openrac_game_bolts != nullptr
        && (shown_bolts < 0 || g_frame >= bolts_frame + 600)) {
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
#ifdef OPENRAC_FRONTEND
        // A fixed-length run can stop with its last owned snapshot still recording.
        if (g_window) {
            leave();
        }
#endif
        std::exit(0);  // finish() runs at exit
    }

    using namespace std::chrono;
    const auto now = steady_clock::now();
    // OPENRAC_FPS: the frames per second over each 300 frames, in the log. OPENRAC_UNCAPPED: no
    // wait for the next frame's time (the game then runs as fast as the machine allows: a measure,
    // not a way to play, since the game's tick is its frame).
    static const bool& fps =
        stats::probe("OPENRAC_FPS", "the ticks per second over each 300 ticks, in the log");
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
                line += kind == 'i' ? std::format(" {}", static_cast<std::int32_t>(w))
                                    : std::format(" {:.4f}", f);
            }

            line += " |";
            at = std::strchr(at, ',');
            at = at ? at + 1 : nullptr;
        }

        log::info("frame {} watch{}", g_frame, line);
    }

    // OPENRAC_GUARD=FRAME:ADDRESS:BYTES[:SKIP]: from FRAME on, the first write (after SKIP) into
    // those bytes of game memory ends the program with the host's stack (runtime::guard_writes), to
    // find who corrupts it.
    static const std::optional<std::string> guard = env_string(env::Var::Guard);

    if (guard) {
        unsigned long from = 0, address = 0, bytes = 4, skip = 0;
        std::sscanf(guard->c_str(), "%lu:%lx:%lx:%lu", &from, &address, &bytes, &skip);

        // The guard is set from frame FROM on. g_frame counts up from 0, so the cast keeps its
        // value and the comparison is the one the implicit conversion made.
        if (static_cast<unsigned long>(g_frame) >= from) {
            runtime::guard_writes(
                static_cast<gaddr>(address),
                static_cast<std::uint32_t>(bytes),
                static_cast<std::uint32_t>(skip)
            );
        }
    }

    // OPENRAC_HITCH: each frame that took over 1.25 frame times, with how long, in the log.
    static const bool& hitch =
        stats::probe("OPENRAC_HITCH", "each tick that took over 1.25 tick times, in the log");
    static steady_clock::time_point hitch_from = now;

    if (hitch) {
        const double ms = duration<double, std::milli>(now - hitch_from).count();

        if (ms > 1.25 * 1000.0 / tick_rate()) {
            log::info("frame {}: {:.1f} ms", g_frame, ms);
        }
    }

    hitch_from = now;

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
        std::optional<stats::Scope> wait;
        bool render_pending = false;
#ifdef OPENRAC_FRONTEND
        render_pending = g_window && frontend::render_pending();
#endif
        // The render job owns the profiler's section stack while the game can sleep or simulate.
        if (!render_pending) {
            wait.emplace("wait");
        }
        std::this_thread::sleep_until(g_next_frame);
    }
#ifdef OPENRAC_FRONTEND
    openrac::platform::
        work_resumed();  // the next tick's work counts from here (performance_hint.h)

    // [debug tools] A paused game stays here, its last tick drawn again (frontend.h, hold).
    if (g_window && !frontend::hold()) {
        leave();
    }
#endif
    return static_cast<int>(g_frame & 1);
}

}  // namespace openrac::program

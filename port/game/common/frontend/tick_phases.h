// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// OPENRAC_TICK_PHASES=N (30..3600; off by default): where the game thread's time goes, tick by
// tick, logged every N ticks as a mean and a 95th percentile per phase. Diagnostic only: it reads
// the steady clock a few times a tick and changes nothing about what is drawn or when.
//
// A tick on the game thread is the game's own code (game: from vsync's return to its next call)
// and then vsync, which is split into:
//   prepare      frontend::prepare_frame, the next frame's CPU work while a render job is out
//   capture      frontend::frame less what is nested in it: reading the tick from the game's memory
//   render       drawing the frame on this thread (render_scene), or with the render worker only
//                handing it the snapshot
//   worker wait  waiting for the render worker to finish the frame before (finish_render)
//   present      present_frame: the frame to the screen, with the UI over it, and the swap
//   between      frames between ticks less what is nested in them, and pacing's own waits there
//   sleep        vsync's sleep until the next tick is due
//   other        the rest of vsync (sound, mods, counters)
// The phases add up to the whole tick. Only the game thread records: render_scene on the render
// worker is not timed here (OPENRAC_CPU_TIMING has it), and the GPU's time is not CPU time.

#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <string>

#include "common/log.h"
#include "common/timing_samples.h"

namespace openrac::frontend::tick_phases {

enum Phase : int {
    Game,
    Prepare,
    Capture,
    Render,
    WorkerWait,
    Present,
    Between,
    Sleep,
    Other,
    kPhases
};

inline constexpr std::array<const char*, kPhases> kNames = {
    "game", "prepare", "capture", "render", "worker wait", "present", "between", "sleep", "other"
};

using Clock = std::chrono::steady_clock;

// Ticks a report covers, or 0 when OPENRAC_TICK_PHASES is not set.
inline int every() {
    static const int n = [] {
        const char* v = std::getenv("OPENRAC_TICK_PHASES");
        const int asked = v != nullptr ? std::atoi(v) : 0;
        const int set = asked > 0 ? std::clamp(asked, 30, 3600) : 0;
        if (set != 0) {
            log::info(
                "frontend: tick phases every {} ticks (OPENRAC_TICK_PHASES; game-thread wall time "
                "per phase, diagnostic only)",
                set
            );
        }
        return set;
    }();
    return n;
}

struct Tally {
    std::array<double, kPhases> tick{};  // this tick's milliseconds, by phase
    std::array<TimingSamples, kPhases> samples;
    TimingSamples whole;     // game + vsync: one tick's whole time
    TimingSamples presents;  // presents a tick
    unsigned tick_presents = 0;
    Clock::time_point entered{};  // vsync's start this tick
    Clock::time_point left{};     // vsync's return last tick
};

// Game thread only.
inline Tally& tally() {
    static Tally t;
    return t;
}

inline double ms(Clock::duration d) {
    return std::chrono::duration<double, std::milli>(d).count();
}

inline void add(Phase phase, double milliseconds) {
    if (every() != 0) {
        tally().tick[phase] += milliseconds;
    }
}

// What the nested phases took so far this tick: an enclosing span counts only its own time.
inline double nested() {
    const auto& t = tally().tick;
    return t[Render] + t[WorkerWait] + t[Present];
}

inline void count_present() {
    if (every() != 0) {
        ++tally().tick_presents;
    }
}

// Times one phase.
class Timer {
public:
    explicit Timer(Phase phase) : m_phase(phase), m_on(every() != 0) {
        if (m_on) {
            m_start = Clock::now();
        }
    }
    ~Timer() {
        if (m_on) {
            add(m_phase, ms(Clock::now() - m_start));
        }
    }
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

private:
    Phase m_phase;
    bool m_on;
    Clock::time_point m_start{};
};

// Times a span that holds nested phases (render, worker wait, present), less those.
class Span {
public:
    explicit Span(Phase phase) : m_phase(phase), m_on(every() != 0) {
        if (m_on) {
            m_nested = nested();
            m_start = Clock::now();
        }
    }
    ~Span() {
        if (m_on) {
            const double inside = ms(Clock::now() - m_start) - (nested() - m_nested);
            add(m_phase, std::max(0.0, inside));
        }
    }
    Span(const Span&) = delete;
    Span& operator=(const Span&) = delete;

private:
    Phase m_phase;
    bool m_on;
    double m_nested = 0.0;
    Clock::time_point m_start{};
};

// The report line for the ticks tallied so far, then cleared.
inline std::string report_line(Tally& t) {
    const double whole = t.whole.mean();
    std::string line = std::format(
        "tick phases over {} ticks (game thread, ms mean/p95): whole tick {:.3f}/{:.3f} "
        "({:.1f} ticks a second)",
        t.whole.values.size(),
        whole,
        t.whole.p95(),
        whole > 0.0 ? 1000.0 / whole : 0.0
    );
    for (int p = 0; p < kPhases; ++p) {
        line += std::format(
            "; {} {:.3f}/{:.3f}", kNames[static_cast<std::size_t>(p)], t.samples[p].mean(),
            t.samples[p].p95()
        );
    }
    line += std::format(
        "; presents a tick {:.2f} (render is this thread's drawing, or only the handoff with the "
        "render worker; phases overlap nothing on this thread and add up to the whole tick)",
        t.presents.mean()
    );
    for (auto& s : t.samples) {
        s.clear();
    }
    t.whole.clear();
    t.presents.clear();
    return line;
}

// vsync's start: the game's own code ran since vsync last returned.
inline void enter() {
    if (every() == 0) {
        return;
    }
    Tally& t = tally();
    const auto now = Clock::now();
    if (t.left.time_since_epoch().count() != 0) {
        t.tick[Game] += ms(now - t.left);
    }
    t.entered = now;
}

// vsync's return: the tick is complete. What vsync spent outside the measured phases is "other".
inline void leave() {
    if (every() == 0) {
        return;
    }
    Tally& t = tally();
    const auto now = Clock::now();
    if (t.entered.time_since_epoch().count() != 0) {
        const double inside = ms(now - t.entered);
        double measured = 0.0;
        for (int p = 0; p < kPhases; ++p) {
            if (p != Game && p != Other) {
                measured += t.tick[p];
            }
        }
        t.tick[Other] += std::max(0.0, inside - measured);
        // The first tick has no game time before it: left out rather than counted short.
        if (t.left.time_since_epoch().count() != 0) {
            for (int p = 0; p < kPhases; ++p) {
                t.samples[p].add(t.tick[p]);
            }
            t.whole.add(t.tick[Game] + inside);
            t.presents.add(static_cast<double>(t.tick_presents));
        }
    }
    t.tick.fill(0.0);
    t.tick_presents = 0;
    t.left = now;
    if (t.whole.values.size() >= static_cast<std::size_t>(every())) {
        log::info("{}", report_line(t));
    }
}

// vsync's whole body, from its first line to its return.
class Vsync {
public:
    Vsync() { enter(); }
    ~Vsync() { leave(); }
    Vsync(const Vsync&) = delete;
    Vsync& operator=(const Vsync&) = delete;
};

}  // namespace openrac::frontend::tick_phases

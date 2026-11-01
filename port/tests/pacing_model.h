// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A model of the game thread's tick and its frames (program/vsync.cpp with frontend frame.cpp
// draw_tick_frame, interpolation.cpp between/swapped/finish_render and the render worker), over a
// simulated clock, for tests/frontend_pacing_test.cpp. It calls the real pacing functions
// (frame_pacing::next_due, frame_may_start, tick_frame_may_start, tick_pacing::next_due); the rest
// is the order of the real code, written out. Frames between ticks are left out (declined), as in
// the clean configuration (OPENRAC_NO_BETWEEN_WHEN_BEHIND=1, the 60 Hz tick at a cap of 60), so
// every picture is a tick's own.

#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "game/common/frame_pacing.h"
#include "game/common/frontend/pacing.h"
#include "game/common/program/tick_pacing.h"

namespace pacing_model {

using Clock = std::chrono::steady_clock;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::milliseconds;

struct Config {
    double cap = 60.0;                     // frames a second, 0 for none
    bool worker = true;                    // OPENRAC_DK_RENDER_THREAD=1
    bool behind_experiment = true;         // OPENRAC_NO_BETWEEN_WHEN_BEHIND=1
    bool reference = false;                // OPENRAC_TICK_FRAME_PACING_REFERENCE=1: the old order
    bool even_present = false;             // OPENRAC_EVEN_PRESENT=1: a finished frame's swap held to a tick after the last
    std::function<int(int)> rate = [](int) { return 60; };              // tick rate of tick i
    std::function<Clock::duration(int)> work = [](int) { return milliseconds(5); };  // tick i's game work
    std::function<Clock::duration(int)> render = [](int) { return milliseconds(4); };  // its frame's render job
    Clock::duration cost = milliseconds(1);  // Pacing::cost, the window thread's share of a frame
};

struct Result {
    int ticks = 0, drawn = 0, deferred = 0, deferred_drawn = 0;
    int lost_superseded = 0, lost_worker = 0, lost_late = 0, lost_behind = 0;
    std::vector<Clock::time_point> presents;   // each swap, in order
    std::vector<int> present_ticks;            // the tick each swap showed
    std::vector<Clock::time_point> tick_starts;  // each tick's game work start
    std::vector<Clock::time_point> starts;       // each frame's start (draw_kept), what the cap governs

    int lost() const { return deferred - deferred_drawn; }
    // The gaps between swaps (or, `of_starts`, frame starts), in ms, from the `from`th on.
    std::vector<double> gaps(std::size_t from = 0, bool of_starts = false) const {
        const auto& t = of_starts ? starts : presents;
        std::vector<double> g;
        for (std::size_t i = std::max<std::size_t>(from, 1); i < t.size(); ++i) {
            g.push_back(duration<double, std::milli>(t[i] - t[i - 1]).count());
        }
        return g;
    }
};

inline Result run(const Config& c, int ticks) {
    namespace tp = openrac::program::tick_pacing;
    namespace fp = openrac::frontend::frame_pacing;
    Result r;
    Clock::time_point now = Clock::time_point{} + std::chrono::seconds(1000);
    Clock::time_point tick_due{};  // vsync's g_next_frame
    Clock::time_point cap_due{};   // Pacing::due
    struct Pending { Clock::time_point started, done; int tick; };
    std::optional<Pending> pending;
    bool first_pending = false;
    int last_rate = 0;

    const auto present = [&](Clock::time_point started, int tick) {
        r.presents.push_back(now);
        r.present_ticks.push_back(tick);
        if (c.cap > 0.0) {
            const auto step = duration_cast<Clock::duration>(duration<double>(1.0 / c.cap));
            cap_due = fp::next_due(cap_due, started, now, step);
        } else {
            cap_due = {};
        }
    };
    // draw_kept then, for a synchronous frame or one finished at once, the swap.
    const auto draw = [&](int tick, bool finish_now) {
        const auto started = now;
        r.starts.push_back(started);
        if (c.worker) {
            pending = Pending{started, now + c.render(tick), tick};
            now += microseconds(300);  // hand-off
            if (finish_now) {
                now = std::max(now, pending->done);
                present(pending->started, pending->tick);
                pending.reset();
            }
        } else {
            now += c.render(tick);
            present(started, tick);
        }
    };

    for (int i = 0; i < ticks; ++i) {
        const int rate = c.rate(i);
        const bool rate_changed = last_rate != 0 && rate != last_rate;
        last_rate = rate;
        const auto period = tp::period_of(rate);
        r.tick_starts.push_back(now);
        now += c.work(i);
        // vsync(): finish_render, the last tick's frame swapped.
        if (pending) {
            now = std::max(now, pending->done);
            present(pending->started, pending->tick);
            pending.reset();
        }
        // set_tick
        const auto behind = tick_due.time_since_epoch().count() != 0 && now > tick_due
                                ? duration<double>(now - tick_due) : duration<double>(0.0);
        ++r.ticks;
        // draw_tick_frame
        const bool may = c.reference
            ? openrac::frontend::frame_may_start(c.cap, cap_due, now)
            : openrac::frontend::tick_frame_may_start(c.cap, cap_due, now, period, false);
        if (!may) {
            first_pending = true;
            ++r.deferred;
        } else {
            first_pending = false;
            draw(i, false);
            ++r.drawn;
        }
        // The schedule, then between(g_next_frame).
        tick_due = tp::next_due(tick_due, now, period, rate_changed);
        const auto next_tick = tick_due;
        [&] {
            const bool behind_declines = c.behind_experiment && behind > duration<double>(period) * 0.5;
            if (c.reference && behind_declines) {
                if (first_pending) ++r.lost_behind;
                return;
            }
            if (pending) {
                if (pending->done > next_tick) {
                    if (first_pending) ++r.lost_worker;
                    return;  // ready_until(next_tick) false
                }
                now = std::max(now, pending->done);
                if (c.even_present && !r.presents.empty()) {
                    const auto even = r.presents.back() + period - milliseconds(1);
                    now = std::max(now, std::min(even, next_tick));
                }
                present(pending->started, pending->tick);
                pending.reset();
            }
            if (!first_pending) {
                return;  // no frames between ticks in this model
            }
            if (!openrac::frontend::frame_may_start(c.cap, cap_due, now)) {
                if (cap_due >= next_tick) {
                    ++r.lost_superseded;
                    return;
                }
                now = cap_due;  // sleep_until(due)
            }
            if (now + c.cost >= next_tick) {
                ++r.lost_late;
                return;
            }
            first_pending = false;
            draw(i, true);
            ++r.drawn;
            ++r.deferred_drawn;
        }();
        now = std::max(now, tick_due);  // sleep_until(g_next_frame)
    }
    return r;
}

// A fixed pseudo-random sequence (no clock, the same on every host).
struct Lcg {
    std::uint32_t s;
    explicit Lcg(std::uint32_t seed) : s(seed) {}
    std::uint32_t next() { s = s * 1664525u + 1013904223u; return s >> 8; }
    // A whole number of microseconds in [lo, hi].
    Clock::duration us(int lo, int hi) { return microseconds(lo + static_cast<int>(next() % std::uint32_t(hi - lo + 1))); }
};

inline double p95(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    return v[static_cast<std::size_t>(0.95 * static_cast<double>(v.size() - 1))];
}
inline double maximum(const std::vector<double>& v) { return v.empty() ? 0.0 : *std::max_element(v.begin(), v.end()); }
inline double minimum(const std::vector<double>& v) { return v.empty() ? 0.0 : *std::min_element(v.begin(), v.end()); }
inline double mean(const std::vector<double>& v) {
    double s = 0; for (double x : v) s += x; return v.empty() ? 0.0 : s / static_cast<double>(v.size());
}

}  // namespace pacing_model

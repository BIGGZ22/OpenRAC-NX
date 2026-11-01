// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frontend's pacing arithmetic (game/common/frontend/pacing.h): the refresh a frame is shown
// at, how far between ticks the scene is at a refresh for the PAL and NTSC tick rates, and when the
// frame cap lets a frame start. Characterisation tests: they pin what the functions returned when
// they were moved out of frontend.cpp, so that the frontend's restructure can show it changed none
// of it. Each expected value is written as the arithmetic that gives it. The clock is never read:
// every time is an offset from a fixed point.

#include <chrono>
#include <cstdio>
#include <memory>

#include "check.h"
#include "game/common/frame_pacing.h"
#include "game/common/frontend/pacing.h"
#include "pacing_model.h"

namespace {

using openrac::frontend::frame_may_start;
using openrac::frontend::kLagTicks;
using openrac::frontend::shown_at;
using openrac::frontend::tick_alpha;
using openrac::frontend::tick_frame_may_start;
using openrac::frontend::tick_frame_tolerance;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

// The game's tick rates: PAL's 50 a second and NTSC's 60.
constexpr int kTickRates[] = {50, 60};

// The frame caps the player can choose from, with 0 for none.
constexpr double kCaps[] = {0.0, 30.0, 60.0, 120.0, 144.0};

// A 60 Hz display's refresh interval, in seconds.
constexpr double kRefresh = 1.0 / 60.0;

// What drawing a frame costs, in seconds (4 ms).
constexpr double kCost = 0.004;

// How far apart two alphas may be and still be the same result of double to float arithmetic.
constexpr float kTolerance = 1e-5f;

// A fixed point on the steady clock, away from its epoch (which stands for "never").
steady_clock::time_point start() {
    return steady_clock::time_point{} + std::chrono::seconds(100);
}

// `seconds` as the steady clock's duration, cut as the frontend cuts it.
steady_clock::duration span(double seconds) {
    return duration_cast<steady_clock::duration>(duration<double>(seconds));
}

// True when `a` and `b` differ by at most kTolerance.
bool near(float a, float b) {
    return (a > b ? a - b : b - a) <= kTolerance;
}

// With no refresh interval, or no swap that waited, a frame is shown when it is drawn.
void test_a_frame_without_a_refresh_is_shown_when_drawn() {
    const auto now = start();
    CHECK(shown_at(now, kCost, 0.0, now - milliseconds(5)) == now + span(kCost));
    CHECK(shown_at(now, kCost, kRefresh, steady_clock::time_point{}) == now + span(kCost));
}

// A frame is shown at the first refresh after it is drawn, counted from the last swap that waited.
void test_a_frame_is_shown_at_the_next_refresh() {
    const auto now = start();
    // Drawn 5 + 4 = 9 ms after the last swap: the first refresh, 16.7 ms after it.
    const auto swap = now - milliseconds(5);
    CHECK(shown_at(now, kCost, kRefresh, swap) == swap + span(1 * kRefresh));
    // Drawn 40 + 4 = 44 ms after it: the third refresh (44 / 16.7 = 2.64, rounded up).
    const auto old_swap = now - milliseconds(40);
    CHECK(shown_at(now, kCost, kRefresh, old_swap) == old_swap + span(3 * kRefresh));
}

// A frame drawn before the last swap's time is still shown no sooner than the refresh after it.
void test_a_frame_is_shown_one_refresh_after_the_swap_at_least() {
    const auto now = start();
    const auto swap = now + milliseconds(10);
    CHECK(shown_at(now, kCost, kRefresh, swap) == swap + span(1 * kRefresh));
}

// The scene is at the last tick until half a tick has passed, and at the kept one a tick later.
void test_the_alpha_is_held_at_the_edges_of_a_tick() {
    // PAL's tick and NTSC's.
    for (const int rate : kTickRates) {
        const duration<double> period{1.0 / rate};
        const auto tick = start();
        // Before the tick and up to the lag, 0.
        CHECK(tick_alpha(tick - milliseconds(1), tick, period) == 0.0f);
        CHECK(tick_alpha(tick, tick, period) == 0.0f);
        CHECK(tick_alpha(tick + span(kLagTicks / rate) - milliseconds(1), tick, period) == 0.0f);
        // A tick and a half on (the lag and the whole tick), and after it, 1.
        CHECK(tick_alpha(tick + span((1.0 + kLagTicks) / rate), tick, period) == 1.0f);
        CHECK(tick_alpha(tick + span(5.0 / rate), tick, period) == 1.0f);
    }
}

// Between the edges the alpha runs straight from 0 to 1 over one tick.
void test_the_alpha_runs_over_a_tick() {
    // PAL's tick and NTSC's.
    for (const int rate : kTickRates) {
        const duration<double> period{1.0 / rate};
        const auto tick = start();
        // One tick on is half a tick past the lag: 0.5.
        CHECK(near(tick_alpha(tick + span(1.0 / rate), tick, period), 0.5f));
        // A quarter of a tick past the lag: 0.25.
        CHECK(near(tick_alpha(tick + span((kLagTicks + 0.25) / rate), tick, period), 0.25f));
    }
}

// No cap, or no capped frame yet, lets every frame start.
void test_no_cap_lets_a_frame_start() {
    const auto now = start();

    // Every cap, with no frame capped yet.
    for (const double cap : kCaps) {
        CHECK(frame_may_start(cap, steady_clock::time_point{}, now));
    }

    CHECK(frame_may_start(0.0, now + std::chrono::seconds(1), now));
    CHECK(frame_may_start(-1.0, now + std::chrono::seconds(1), now));
}

// A cap holds a frame until a millisecond before its due time, for each cap the player can pick.
void test_a_cap_holds_a_frame_until_it_is_due() {
    // Each cap the player can pick.
    for (const double cap : kCaps) {
        // No cap has no schedule to hold to.
        if (cap <= 0.0) {
            continue;
        }

        const auto step = span(1.0 / cap);
        const auto started = start();
        // The first capped frame drew in 1 ms: the next is due a step after it started.
        const auto due =
            openrac::frontend::frame_pacing::next_due({}, started, started + milliseconds(1), step);
        CHECK(due == started + step);
        CHECK(!frame_may_start(cap, due, due - milliseconds(2)));
        CHECK(frame_may_start(cap, due, due - milliseconds(1)));
        CHECK(frame_may_start(cap, due, due));
        CHECK(frame_may_start(cap, due, due + step));
    }
}


// ---- Session 55: the tick's own frame against the frame cap ----

// The tick frame's tolerance: half a cap frame when the cap is at least the tick rate and no frame
// was drawn between ticks since; the 1 ms otherwise.
void test_the_tick_frame_tolerance() {
    const duration<double> t60{1.0 / 60}, t50{1.0 / 50};
    const auto ms1 = steady_clock::duration(milliseconds(1));
    CHECK(tick_frame_tolerance(60.0, t60, false) == span(1.0 / 120));
    CHECK(tick_frame_tolerance(60.0, duration<double>(0.016666), false) == span(1.0 / 120));  // tick_pacing's 16666 us
    CHECK(tick_frame_tolerance(60.0, t50, false) == span(1.0 / 120));
    CHECK(tick_frame_tolerance(120.0, t60, false) == span(1.0 / 240));
    CHECK(tick_frame_tolerance(144.0, t50, false) == span(1.0 / 288));
    CHECK(tick_frame_tolerance(30.0, t60, false) == ms1);  // a cap under the tick rate chooses ticks
    CHECK(tick_frame_tolerance(30.0, t50, false) == ms1);
    CHECK(tick_frame_tolerance(60.0, t60, true) == ms1);   // a frame between ticks took the slot
    CHECK(tick_frame_tolerance(0.0, t60, false) == ms1);
    CHECK(tick_frame_tolerance(60.0, duration<double>(0.0), false) == ms1);
    CHECK(tick_frame_tolerance(1000.0, t60, false) == ms1);  // never under the 1 ms

    const auto now = start();
    CHECK(tick_frame_may_start(0.0, now + std::chrono::seconds(1), now, t60, false));
    CHECK(tick_frame_may_start(60.0, steady_clock::time_point{}, now, t60, false));
    CHECK(tick_frame_may_start(60.0, now + milliseconds(8), now, t60, false));
    CHECK(!tick_frame_may_start(60.0, now + milliseconds(9), now, t60, false));
    CHECK(!tick_frame_may_start(60.0, now + milliseconds(8), now, t60, true));
    CHECK(!tick_frame_may_start(30.0, now + milliseconds(2), now, t60, false));
    CHECK(tick_frame_may_start(30.0, now + milliseconds(1), now, t60, false));
}

namespace pm = pacing_model;

// The busiest second's frame starts (what the cap holds; a second's window holds both ends, so a
// steady 60 a second counts 61).
int most_starts_in_a_second(const pm::Result& r) {
    int most = 0;
    std::size_t j = 0;
    for (std::size_t i = 0; i < r.starts.size(); ++i) {
        while (r.starts[i] - r.starts[j] >= std::chrono::seconds(1)) {
            ++j;
        }
        most = std::max(most, static_cast<int>(i - j + 1));
    }
    return most;
}

// The shortest gap between two frame starts, in ms.
double closest_starts(const pm::Result& r) {
    return pm::minimum(r.gaps(1, true));
}

// Session 54's gameplay, as the log shows it: cap 60, ticks 60, vsync 0, the render thread, between
// frames declined (behind), each tick's work 2.5-11 ms, its render job 3-5 ms with a 12 ms one now
// and then, and a 40 ms tick every 150 (a hitch: the cap's schedule then starts from a late frame).
pm::Config session54(bool reference, std::uint32_t seed, int hitch_ms = 40) {
    pm::Config c;
    c.reference = reference;
    auto work = std::make_shared<pm::Lcg>(seed);
    auto render = std::make_shared<pm::Lcg>(seed + 100);
    c.work = [work, hitch_ms](int i) {
        return i % 150 == 75 ? steady_clock::duration(milliseconds(hitch_ms)) : work->us(2500, 11000);
    };
    c.render = [render](int i) { return i % 37 == 0 ? steady_clock::duration(milliseconds(12)) : render->us(3000, 5000); };
    return c;
}

// The old rules reproduce what session 54 counted: tick frames held back after a hitch, some never
// drawn (declined behind), the others drawn late. The new ones hold back only the tick run at once
// after a late one, whose frame the next tick's replaces; none is declined; the swaps are more even.
void test_session54_tick_frames_held_back_and_lost() {
    for (const std::uint32_t seed : {1u, 2u, 3u}) {
        const pm::Result old = pm::run(session54(true, seed), 3600);
        const pm::Result now = pm::run(session54(false, seed), 3600);
        std::printf("session54 model seed %u: reference drawn %d deferred %d lost %d (behind %d) p95 %.1f ms; "
                    "new drawn %d deferred %d lost %d (next tick %d) p95 %.1f ms\n",
                    seed, old.drawn, old.deferred, old.lost(), old.lost_behind, pm::p95(old.gaps(60)),
                    now.drawn, now.deferred, now.lost(), now.lost_superseded, pm::p95(now.gaps(60)));
        CHECK(old.lost_behind > 0);             // the drops the log counted, reproduced
        CHECK(old.deferred > 600);              // and the held-back frames drawn late
        CHECK(now.lost_behind == 0);
        CHECK(now.lost_worker == 0 && now.lost_late == 0);
        CHECK(now.deferred <= 2 * (3600 / 150) + 2);  // the catch-up ticks after each hitch only
        CHECK(now.lost() == now.lost_superseded);
        CHECK(now.lost() <= old.lost() + 2);
        CHECK(pm::p95(now.gaps(60)) < pm::p95(old.gaps(60)) - 5.0);
        CHECK(most_starts_in_a_second(now) <= 61);  // never above the cap
        CHECK(closest_starts(now) >= 1000.0 / 120 - 2.0);
        // The game's time is the tick schedule's either way: the same ticks in the same time.
        CHECK(now.tick_starts.back() - now.tick_starts.front() == old.tick_starts.back() - old.tick_starts.front());
    }
    // A shorter hitch (25 ms): the old rules still lose frames to the decline; the new lose at most
    // one in the whole run.
    for (const std::uint32_t seed : {1u, 2u}) {
        const pm::Result old = pm::run(session54(true, seed, 25), 3600);
        const pm::Result now = pm::run(session54(false, seed, 25), 3600);
        std::printf("session54 model, 25 ms hitches, seed %u: lost %d -> %d, deferred %d -> %d\n", seed,
                    old.lost(), now.lost(), old.deferred, now.deferred);
        CHECK(old.lost() >= 3);
        CHECK(now.lost() <= 1);
        CHECK(now.lost() < old.lost());
    }
}

// Without hitches, every tick frame is drawn at once, render thread or not, and the game runs at
// 60 ticks a second.
void test_cap_60_ticks_60_every_frame_drawn() {
    for (const bool worker : {true, false}) {
        pm::Config c = session54(false, 7);
        auto work = std::make_shared<pm::Lcg>(7);
        c.work = [work](int) { return work->us(2500, 11000); };
        c.worker = worker;
        const pm::Result r = pm::run(c, 3600);
        CHECK(r.drawn == 3600 && r.lost() == 0);
        if (worker) {
            CHECK(r.deferred == 0);
        }
        CHECK(most_starts_in_a_second(r) <= 61);
        CHECK(closest_starts(r) >= 1000.0 / 120 - 2.0);
        // 3599 tick periods of tick_pacing's 16666 us, the slow ticks caught up.
        const double wall = duration<double>(r.tick_starts.back() - r.tick_starts.front()).count();
        CHECK(wall > 3599 * 0.016666 - 0.01 && wall < 3599 * 0.016666 + 0.05);
    }
}

// PAL's 50 ticks under a cap of 60: every tick frame drawn, 50 a second, never two inside half a
// cap frame; render thread on and off.
void test_cap_60_ticks_50() {
    for (const bool worker : {true, false}) {
        pm::Config c = session54(false, 11);
        auto work = std::make_shared<pm::Lcg>(11);
        c.work = [work](int) { return work->us(2500, 14000); };
        c.rate = [](int) { return 50; };
        c.worker = worker;
        const pm::Result r = pm::run(c, 3000);
        CHECK(r.drawn == 3000 && r.lost() == 0);
        CHECK(most_starts_in_a_second(r) <= 51);
        CHECK(closest_starts(r) >= 1000.0 / 120 - 2.0);
    }
}

// A cap of 30 under 60 ticks keeps 30 a second (every other tick), with no burst.
void test_cap_30_ticks_60() {
    for (const bool reference : {false, true}) {
        for (const bool worker : {true, false}) {
            pm::Config c = session54(reference, 13);
            auto work = std::make_shared<pm::Lcg>(13);
            c.work = [work](int) { return work->us(2500, 9000); };
            c.cap = 30.0;
            c.worker = worker;
            const pm::Result r = pm::run(c, 3600);  // 60 s
            CHECK(r.drawn >= 1790 && r.drawn <= 1801);
            CHECK(most_starts_in_a_second(r) <= 31);
            CHECK(closest_starts(r) >= 1000.0 / 60 - 1.0);
        }
    }
}

// The game's time base changing from 50 to 60 ticks a second (the first level after the title),
// under a cap of 60: every tick frame drawn, no burst at the change, each rate's time kept.
void test_50_then_60_ticks() {
    pm::Config c = session54(false, 17);
    auto work = std::make_shared<pm::Lcg>(17);
    c.work = [work](int) { return work->us(2500, 10000); };
    c.rate = [](int i) { return i < 500 ? 50 : 60; };
    const pm::Result r = pm::run(c, 1100);
    CHECK(r.drawn == 1100 && r.lost() == 0);
    CHECK(closest_starts(r) >= 1000.0 / 120 - 2.0);
    CHECK(most_starts_in_a_second(r) <= 61);
    const double wall50 = duration<double>(r.tick_starts[500] - r.tick_starts[0]).count();
    const double wall60 = duration<double>(r.tick_starts[1099] - r.tick_starts[500]).count();
    CHECK(wall50 > 9.99 && wall50 < 10.03);   // 500 ticks at 50
    CHECK(wall60 > 9.97 && wall60 < 10.01);   // 599 ticks at 60
}

// A load or a pause (a 500 ms stop): the tick schedule and the cap start again after it; no burst
// of ticks or frames to catch up, and every frame after it drawn.
void test_a_stop_brings_no_burst() {
    for (const bool reference : {false, true}) {
        pm::Config c = session54(reference, 19);
        auto work = std::make_shared<pm::Lcg>(19);
        c.work = [work](int i) { return i == 600 ? steady_clock::duration(milliseconds(500)) : work->us(2500, 8000); };
        const pm::Result r = pm::run(c, 1200);
        CHECK(most_starts_in_a_second(r) <= 61);
        for (std::size_t i = 603; i < r.tick_starts.size(); ++i) {
            CHECK(r.tick_starts[i] - r.tick_starts[i - 1] >= milliseconds(16));
        }
        if (!reference) {
            CHECK(r.drawn == 1200);
        }
    }
}

// Delayed frames (the render job past the next tick now and then): no frame lost to the cap, none
// two inside half a cap frame.
void test_delayed_render_jobs() {
    pm::Config c = session54(false, 23);
    auto work = std::make_shared<pm::Lcg>(23);
    c.work = [work](int) { return work->us(2500, 9000); };
    c.render = [](int i) { return steady_clock::duration(milliseconds(i % 10 == 0 ? 20 : 4)); };
    const pm::Result r = pm::run(c, 1200);
    CHECK(r.lost() == 0);
    CHECK(most_starts_in_a_second(r) <= 61);
    CHECK(closest_starts(r) >= 1000.0 / 120 - 2.0);
}

// OPENRAC_EVEN_PRESENT=1 makes the swaps of session 54's model more even still, with no frame lost.
void test_even_present() {
    pm::Config c = session54(false, 29);
    const pm::Result plain = pm::run(c, 3600);
    c.even_present = true;
    const pm::Result even = pm::run(c, 3600);
    std::printf("even present: p95 %.1f -> %.1f ms, min %.1f -> %.1f ms\n", pm::p95(plain.gaps(60)),
                pm::p95(even.gaps(60)), pm::minimum(plain.gaps(60)), pm::minimum(even.gaps(60)));
    CHECK(even.drawn == plain.drawn);
    CHECK(pm::p95(even.gaps(60)) < pm::p95(plain.gaps(60)));
    CHECK(pm::minimum(even.gaps(60)) > pm::minimum(plain.gaps(60)));
    CHECK(most_starts_in_a_second(even) <= 61);
}

}  // namespace

int main() {
    test_a_frame_without_a_refresh_is_shown_when_drawn();
    test_a_frame_is_shown_at_the_next_refresh();
    test_a_frame_is_shown_one_refresh_after_the_swap_at_least();
    test_the_alpha_is_held_at_the_edges_of_a_tick();
    test_the_alpha_runs_over_a_tick();
    test_no_cap_lets_a_frame_start();
    test_a_cap_holds_a_frame_until_it_is_due();
    test_the_tick_frame_tolerance();
    test_session54_tick_frames_held_back_and_lost();
    test_cap_60_ticks_60_every_frame_drawn();
    test_cap_60_ticks_50();
    test_cap_30_ticks_60();
    test_50_then_60_ticks();
    test_a_stop_brings_no_burst();
    test_delayed_render_jobs();
    test_even_present();
    return openrac::test::result();
}

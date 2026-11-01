// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// OPENRAC_TICK_PHASES (port/game/common/frontend/tick_phases.h): the game thread's tick split into
// phases that add up to the whole tick, with a span's nested phases counted once, in their own
// rows. Timed spins stand in for the work; the checks allow for a busy host's preemption.

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>

#include "game/common/frontend/tick_phases.h"
#include "tests/check.h"

namespace {

namespace phases = openrac::frontend::tick_phases;

// Spins rather than sleeps: a sleep wakes as coarsely as the host's timer (15.6 ms steps on
// Windows), which broke the bounds below there; a spin ends within microseconds of its time.
void work(double ms) {
    const auto until = std::chrono::steady_clock::now()
                       + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                           std::chrono::duration<double, std::milli>(ms)
                       );
    while (std::chrono::steady_clock::now() < until) {
    }
}

// One tick as vsync runs it: the game's code, then vsync's phases.
void tick() {
    work(3.0);  // the game's own code
    const phases::Vsync vsync;
    {
        const phases::Timer prepare(phases::Prepare);
        work(1.0);
    }
    {
        // frontend::frame: a worker wait and a present nested in it, then its own reading, then
        // the drawing.
        const phases::Span capture(phases::Capture);
        {
            const phases::Timer wait(phases::WorkerWait);
            work(0.5);
        }
        {
            const phases::Timer present(phases::Present);
            phases::count_present();
            work(1.0);
        }
        work(1.0);
        {
            const phases::Timer render(phases::Render);
            work(2.0);
        }
    }
    {
        const phases::Timer sleep(phases::Sleep);
        work(0.5);
    }
    work(0.5);  // the rest of vsync: other
}

}  // namespace

int main() {
    // Off: nothing is tallied.
    CHECK(phases::every() == 0 || std::getenv("OPENRAC_TICK_PHASES") != nullptr);
    if (phases::every() == 0) {
        tick();
        CHECK(phases::tally().whole.values.empty());
        // The rest needs OPENRAC_TICK_PHASES=30 (ctest runs it so too).
        return openrac::test::result();
    }
    CHECK(phases::every() == 30);

    // 30 ticks make a report and clear the tally; the first has no game time before it and is not
    // counted, so 29 ticks after it leave one short of the report.
    for (int i = 0; i < 30; ++i) {
        tick();
    }
    phases::Tally& t = phases::tally();
    CHECK(t.whole.values.size() == 29);
    double sum = 0.0;
    for (int p = 0; p < phases::kPhases; ++p) {
        sum += t.samples[p].mean();
    }
    // The phases add up to the whole tick.
    CHECK(std::abs(sum - t.whole.mean()) < 0.01);
    // A span counts only its own time: capture is its 1 ms, not the 3.5 ms nested in it.
    CHECK(t.samples[phases::Capture].mean() >= 0.9 && t.samples[phases::Capture].mean() < 2.0);
    CHECK(t.samples[phases::Render].mean() >= 1.9 && t.samples[phases::Render].mean() < 3.5);
    CHECK(t.samples[phases::WorkerWait].mean() >= 0.4 && t.samples[phases::WorkerWait].mean() < 1.5);
    CHECK(t.samples[phases::Present].mean() >= 0.9 && t.samples[phases::Present].mean() < 2.0);
    CHECK(t.samples[phases::Game].mean() >= 2.9 && t.samples[phases::Game].mean() < 4.5);
    CHECK(t.samples[phases::Other].mean() >= 0.4 && t.samples[phases::Other].mean() < 1.5);
    CHECK(t.presents.mean() == 1.0);

    const std::string line = phases::report_line(t);
    CHECK(line.find("tick phases over 29 ticks") != std::string::npos);
    CHECK(line.find("; worker wait ") != std::string::npos);
    CHECK(line.find("presents a tick 1.00") != std::string::npos);
    CHECK(t.whole.values.empty());
    return openrac::test::result();
}

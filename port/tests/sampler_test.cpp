// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The capture's function sampler (common/sampler.h): a thread that noted itself is sampled while
// sampling runs, and the report names it with its samples; nothing is reported when it did not run
// or the thread was left out.

#include <chrono>
#include <string>

#include "check.h"
#include "common/sampler.h"

namespace {

namespace sampler = openrac::sampler;

// Keeps the processor busy for about `ms` of wall time.
void work(double ms) {
    const auto start = std::chrono::steady_clock::now();
    volatile unsigned sink = 0;
    while (
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count()
        < ms
    ) {
        sink = sink + 1;
    }
}

}  // namespace

int main() {
    CHECK(sampler::stop().empty());  // never started
    sampler::note_thread("early");   // nothing runs: not sampled
    if (!sampler::available()) {
        sampler::start();
        CHECK(sampler::stop().empty());
        return openrac::test::result();
    }
    sampler::start();
    sampler::note_thread("test");
    sampler::note_thread("test");  // once per start, however often it is called
    work(300.0);
    const std::string report = sampler::stop();
    CHECK(report.find("## Hot functions") != std::string::npos);
    CHECK(report.find("test thread, ") != std::string::npos);
    CHECK(report.find("early") == std::string::npos);
    CHECK(report.find('%') != std::string::npos);
    CHECK(sampler::stop().empty());  // stopped already

    // Started again: the thread is sampled anew.
    sampler::start();
    sampler::note_thread("again");
    work(100.0);
    CHECK(sampler::stop().find("again thread, ") != std::string::npos);

    // Left out (the simulated slow processor's spin): no sample is taken, so nothing is reported.
    sampler::start();
    sampler::note_thread("spin");
    sampler::leave_out(true);
    work(100.0);
    sampler::leave_out(false);
    CHECK(sampler::stop().empty());
    return openrac::test::result();
}

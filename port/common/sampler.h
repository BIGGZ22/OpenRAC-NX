// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Where a thread's processor time goes, function by function, for a performance capture: while it
// runs, each thread that has said so (the window thread, the render thread) is interrupted once a
// millisecond of its own processor time, and the function it was in is counted. The capture's
// report ends with each thread's hottest functions and their share of its samples: the time spent
// in the function itself, not in what it called. Named from the libraries' symbol tables (the
// game's translated functions are there), so no other tool or computer is needed. Linux and
// Android only (a per-thread processor-time timer and SIGPROF); empty elsewhere.

#pragma once

#include <string>

namespace openrac::sampler {

// Whether this system can sample.
bool available();

// Starts sampling the threads that call note_thread from now on. Does nothing when it runs.
void start();

// Called by a thread once a frame: sampled from then on while sampling runs. `name` ("window",
// "render") must outlive the sampling (a literal).
void note_thread(const char* name);

// Leaves the calling thread's samples out while `out` is set: the simulated slow processor's spin
// (common/slow_cpu.h), which is no work of the game's.
void leave_out(bool out);

// Stops sampling and returns the report's section of hot functions, Markdown; empty when it did not
// run or took no sample.
std::string stop();

}  // namespace openrac::sampler

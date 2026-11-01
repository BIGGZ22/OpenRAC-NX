// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The Switch's CPU, GPU and memory clocks (clocks.cpp). Always: read back and logged at start and
// once a minute (`switch clocks` lines), with the focus, the mode and the system's performance
// configuration. Only when env.txt asks:
//  - OPENRAC_SWITCH_CLOCK_PROFILE=raised: CPU 1785, GPU 768, memory 1600 MHz, handheld and docked;
//  - or per domain, OPENRAC_SWITCH_CPU_MHZ / _GPU_MHZ / _MEM_MHZ: Nintendo's steps, the GPU held to
//    460.8 MHz handheld (clock_choice.h);
// requested while the game has the focus, verified by read-back, requested again when the system
// (or another manager) resets them, with a backoff once that keeps happening
// (OPENRAC_SWITCH_CLOCK_KEEP=1: every 2 s instead of 30 s), and put back when the game loses the
// focus (HOME, sleep, an applet), on a mode change and at exit.

#pragma once

namespace openrac::platform::switch_clocks {

// Reads the settings, logs the clocks, requests them if asked and watches the focus. Once per
// start/stop pair; the platform calls it when it starts.
void start();

// Scheduled read-backs (after a request: 0.25, 1, 3, 10 s, then every 5 s in focus), resets answered,
// refused requests and put-backs retried, and the minute log. Call from the event pump (the thread
// the applet hooks run on); cheap between due times.
void tick();

// Puts the clocks back (best effort, bounded) and stops. The platform calls it when it ends, and
// std::exit through atexit.
void stop();

}  // namespace openrac::platform::switch_clocks

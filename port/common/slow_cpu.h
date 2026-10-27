// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A slower processor, simulated for testing on a fast device (the drawer's and the Developer menu's
// "Simulate slow CPU"): each thread that asks spins, where its work ends, for (factor - 1) times
// the processor time it used since it last asked, so its work takes `factor` times as long, as on
// a device that much slower, and the frame pacing, the ticks' catching up and a capture behave as
// they would there. Every part slows alike; a weak device's caches and memory differ, and the GPU
// is not slowed. Only where a thread's processor time can be read (Linux, Android, macOS).

#pragma once

namespace openrac::slow_cpu {

// Whether this system can simulate it; set_factor does nothing elsewhere.
bool available();

// 1: off (the default); 2 or more: that many times slower.
void set_factor(int factor);
int factor();

// On the window thread and the render thread where their work ends, at least once a frame: spins
// for (factor - 1) times this thread's processor time since its last call (none the first time,
// or while off).
void pad();

}  // namespace openrac::slow_cpu

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Telling the system how long each frame's work took and how long it may take, so that it keeps
// the CPU fast enough for the game. A phone's CPU governor sees a game thread that works a few
// milliseconds and then waits on the display, and lowers its clocks until frames are late; Android
// 12 and later (API 31; here through the NDK's APerformanceHint, API 33) take a hint session
// instead (Android's "Adaptive Performance Framework"). On a desktop, and where Android has no
// hint sessions, these do nothing.

#pragma once

namespace openrac::platform {

// The time a frame may take, in seconds (the display's refresh interval). Starts the hint session
// for the calling thread, the one that runs the game and draws, the first time it is called.
void set_frame_target(double seconds);

// A wait has just ended (for the display, the frame cap or the next tick): the next frame's work
// counts from now.
void work_resumed();

// A frame's work is done, its swap about to wait for the display: the time since work_resumed() is
// reported as the frame's work.
void frame_work_done();

}  // namespace openrac::platform

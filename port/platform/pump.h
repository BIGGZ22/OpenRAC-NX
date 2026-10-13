// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The window's event pump, kept apart from the program that draws in the window.
//
// On Windows, while the player drags the window by its title bar or sizes it (or holds a menu
// open), the system runs a loop of its own inside the call that reads the window's messages, and
// that call does not return until the mouse button is released. A program that reads its events
// and draws its frames in one loop stands still for as long: the picture froze while the window
// was dragged.
//
// run_pumped runs the program beside the pump on the same thread (a fiber on Windows): the
// program hands over to the pump where it used to read the system's events (poll_event), and
// while the system holds the pump in its loop, SDL's live-resize timer (every 10 ms: an expose
// event, seen by an event watch) hands back to the program for one more frame. Nothing runs at
// the same time as anything else, so the program's SDL and OpenGL calls stay on the thread that
// made the window, as both want. Elsewhere there is no such loop, and run_pumped only calls the
// program.
#pragma once

union SDL_Event;

namespace openrac::platform {

// Runs `program(argument)` to its end and returns its result. The video subsystem is up and the
// calling thread is the one that made the window. A program that leaves through exit() does not
// come back, as before.
int run_pumped(int (*program)(void* argument), void* argument);

// SDL_PollEvent for a program under run_pumped: the next event in SDL's queue, the system's
// events read into it first when it is empty. False when there is none.
bool poll_event(SDL_Event& event);

}  // namespace openrac::platform

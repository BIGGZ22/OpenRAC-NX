// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frame-rate counter (the player's `show_fps` setting): how many frames a second the window
// shows and what drawing one costs, in the picture's top left corner. Counted over the frames
// actually shown, so it tells a frame cap, a display's refresh and a slow frame apart from the
// game's own tick rate, which never changes.
#pragma once

#include <chrono>

namespace openrac::frontend::fps_counter {

// A frame was drawn and is about to be shown; `cost`: the seconds drawing it took before its swap.
void frame_drawn(std::chrono::steady_clock::time_point now, double cost);

// Draws the counter over a framebuffer of one sample a pixel, width x height. The context is
// current.
void draw(unsigned framebuffer, int width, int height);

// Frees its GL objects (the context is still current).
void release();

}  // namespace openrac::frontend::fps_counter

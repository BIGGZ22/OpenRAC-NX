// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The player's settings for a native game: what the port adds to the game's own options (the
// picture's shape, how many frames are drawn, fullscreen). They are kept in a small text file,
// `key = value` a line, beside the memory cards (`--settings FILE` names another), read once at
// start and written whenever one changes.
#pragma once

#include <filesystem>
#include <string>

namespace openrac::settings {

// The picture's shape. The game draws for a 4:3 television; a wider picture shows more of the
// world to the sides (the vertical field of view is the game's), with the 2D layer where it was.
enum class Aspect {
    Standard,   // 4:3, the game's own
    Wide16x10,  // 16:10
    Wide16x9,   // 16:9
    Wide21x9,   // 21:9
    Window,     // whatever the window's shape is (never narrower than 4:3)
    Count,
};

// How many frames are drawn. The game's tick stays its own (50 a second for PAL): the frames
// between two ticks move the camera, the mobys and their poses from one tick to the next.
enum class FrameRate {
    Original,   // one frame a tick
    Display,    // the display's refresh rate, each frame waiting for a refresh
    Cap60,
    Cap120,
    Cap144,
    Cap240,
    Unlimited,  // as many as the machine draws, no wait
    Count,
};

struct Settings {
    Aspect aspect = Aspect::Standard;
    FrameRate frame_rate = FrameRate::Display;
    bool fullscreen = false;
};

// The settings in use.
Settings& get();

// Reads `file` (missing or unreadable: the defaults) and remembers it for save().
void load(const std::filesystem::path& file);

// Writes the settings to the file load() was given; nothing without one.
void save();

// Width over height of the picture for a window of that size.
float aspect_ratio(Aspect aspect, int window_width, int window_height);

// Frames a second a rate caps at; 0 when it has no number of its own.
int frame_cap(FrameRate rate);

// What the options page shows for a value.
const char* name(Aspect aspect);
const char* name(FrameRate rate);

}  // namespace openrac::settings

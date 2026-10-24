// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The debug overlay: a panel in the window's corner, over the game, that says what the port is
// doing: the frame's times, the renderer, memory, the mobys, the game's state and the shader
// programs, with a moby inspector, a memory viewer, pause, single step and slow motion.
// docs/port/DEBUGGING.md describes all of it.
//
// F3 shows and hides it (OPENRAC_OVERLAY or --debug-overlay: shown from the start). Nothing of it
// exists until it is first shown: no ImGui context, no fonts, no measurements beyond the counters
// (common/stats.h). It is drawn with Dear ImGui over the finished frame in the window itself, at
// the window's resolution, so OPENRAC_SHOT's pictures of the game's frame stay without it;
// OPENRAC_OVERLAY_SHOT writes the window with it.
//
// The window (frontend.cpp) calls in at a few marked places and hands over what it already has;
// the overlay reads the game's memory itself and changes none of it.

#pragma once

#include <cstdint>
#include <span>
#include <string>

#include <SDL3/SDL.h>

#include "game/common/profile.h"
#include "renderer/math.h"
#include "renderer/renderer.h"

namespace openrac::frontend::debug_overlay {

// Shown from the start (--debug-overlay), as OPENRAC_OVERLAY does.
void request();

bool visible();

// A window event: true if the overlay took it (its keys, or the mouse and keyboard while they are
// over one of its windows), so the game does not see it.
bool event(const SDL_Event& event);

// Once a tick, from frame(): what the game holds now.
struct Tick {
    const char* game = "";
    std::span<const std::uint8_t> ram;
    int level = -1;                              // the level program loaded; -1 the boot program
    const host::GameState* state = nullptr;      // the camera the tick is drawn from, the mobys
    std::uint32_t draws = 0;                     // the layers the game drew (bit n: renderer::Bucket n)
    std::uint64_t tick = 0;                      // the window's frame counter
    std::size_t packet_bytes = 0, ui_quads = 0, effect_quads = 0, strips = 0, mobys_drawn = 0;
};

void tick(const Tick& tick);

// At the start of each picture drawn (draw_kept): opens the frame for the measurements
// (common/stats.h). `first`: the tick's own picture; `held`: drawn again while paused.
void begin(bool first, bool held, float alpha, std::uint64_t tick);

// Before the renderer draws: the passes hidden in the overlay are turned off, wireframe is set.
void before_render(renderer::Renderer* renderer);

// After the frame was put in the window and before the swap: draws the overlay, if shown.
struct Shown {
    SDL_Window* window = nullptr;
    int window_width = 0, window_height = 0;     // the drawable, in pixels
    int box_x = 0, box_y = 0, box_width = 0, box_height = 0;  // the game's picture in it
    const renderer::Renderer* renderer = nullptr;
    const renderer::Camera* camera = nullptr;    // the picture's view and projection
    int frame_width = 0, frame_height = 0;       // the frame buffer the game is drawn in
    int last_world_width = 0, last_world_height = 0;
    bool vsync = false, interpolating = false;
    double refresh_interval = 0.0;               // seconds; 0 unknown
    double cost = 0.0;                           // seconds: what a frame costs before its swap
    double tick_period = 0.0;                    // seconds
};

void draw(const Shown& shown);

// Pad 0 as the game is about to read it (buttons active low, right then left stick).
void pad(std::uint16_t buttons, const std::uint8_t analog[4]);

// ---- Pause, single step, slow motion ----

// True while the game is paused: the window then draws the last tick again instead of letting the
// game run (frontend.cpp, hold). A step asked for lets one tick through.
bool holding();

// How much longer a tick lasts than the game's own (1: full speed; 4: quarter speed).
double tick_stretch();

// ---- The drawer: the window's options on a touch screen ----
//
// On Android the menus along the window's top (View, Graphics, Sound, Developer), which want a
// mouse and Alt to bring them back, are a drawer instead, as an emulator's: it slides in from the
// left over the paused game, its rows large enough for a thumb and moved through by a gamepad.
// The on-screen pad's options button opens it, as do Back and a gamepad's Guide button; Back,
// Resume, the gamepad's B or a touch beside it closes it.
// OPENRAC_DRAWER=1 has it on other systems too (for looking at it there), in place of the menus.
bool drawer_available();
void toggle_drawer();

// True once after the drawer or the menus changed one of the port's settings (saved already): the
// window applies it (frontend.cpp, apply_settings).
bool take_settings_changed();

// A line kept in the picture until it is cleared (an empty text): something the player has to
// know and can do something about, such as a level whose scenery could not be read.
void notice(const std::string& text);

// GL objects and the ImGui context, before the window's context goes.
void release();

}  // namespace openrac::frontend::debug_overlay

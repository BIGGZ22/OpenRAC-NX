// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The window of a native game: once a frame, what the game holds in its memory is drawn by the
// port's renderer, and the keyboard and controllers are read for the game's pad library.
//
// Only built where the window libraries are (port/cmake/Window.cmake); the program runs without a
// window otherwise. What a game's memory means (where its camera and objects are) is the game's:
// so far Ratchet & Clank (PAL), through viewer/game_state.h, with each level's geometry from the
// extractor's port export (editor/port.py).

#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>

#include "openrac/game_host.h"

namespace openrac::frontend {

// Opens the window. `levels` holds the extracted levels (level_00, level_01, ...).
bool open(const std::string& game_id, const std::filesystem::path& levels, std::string& error);

// Draws a frame from the game's main memory and reads the input; false once the window was closed.
// The game's world renderers are drawing now: the camera they draw with is kept for this frame's
// world (a menu may set its own camera afterwards to draw its frame objects).
void world_drawn(std::span<const std::uint8_t> ram);

// The game drew `count` mobys from `first` with the camera it holds now (the page menu's frame
// objects, drawn with the menu's camera): they are drawn as that camera saw them.
void mobys_drawn(std::span<const std::uint8_t> ram, std::uint32_t first, int count);

// A world effect quad the game drew this frame (openrac_game_effect_quad).
void effect_quad(std::span<const std::uint8_t> ram, const openrac_game_quad& quad);
// A sky shell the game drew this frame, with its matrix (openrac_game_sky_shell).
void sky_shell(int shell, const float* matrix);

// A quad of the 2D layer drawn natively (openrac_game_ui_quad): its index for the marker, or -1
// when the 2D layer is replayed from the game's packets.
int ui_quad(const openrac_game_ui_rect& quad);

// How far the HUD's side-anchored elements go out past the game's picture, in its 512ths of that
// picture's width (openrac_game_hud_side): the player's "HUD: Edges" in a wider picture, else 0.
int hud_side();

// How many times wider than 4:3 the picture is, at least 1 (openrac_game_picture_wider): what the
// frame's horizontal tangent is the game's times (render's projection, picture_scale).
float picture_wider();

// A sky sprite the game drew this frame (openrac_game_sky_sprite).
void sky_sprite(const std::uint8_t* record, std::uint64_t tex0);

// A live particle the game drew this frame (openrac_game_particle).
void particle(std::span<const std::uint8_t> ram, const std::uint8_t* record, std::uint32_t pixels,
              std::uint32_t clut, int log2_side);

// `draws`: the layers the game's renderers drew this frame (bit n = renderer::Bucket n; all bits
// when the game does not report them).
bool frame(std::span<const std::uint8_t> ram, std::uint32_t chain, std::uint32_t draws);

// Frames between the game's ticks: the window draws at the display's rate, the camera and mobys
// moved from the last tick's to the latest's by when each frame is shown (OPENRAC_UNCAPPED or
// OPENRAC_NO_BLEND: one frame a tick). set_tick: when the tick about to be drawn was due, and the
// tick's length. interpolating: it is on; wants_between: the latest tick has a world to move;
// between: draws one if a refresh before `next_tick` (plus the lag) still shows this tick, and
// reads the window's events: 1 drawn, 0 none due, -1 the window was closed.
// `late`: how long after its time the tick came (a machine that draws slower than the game ticks):
// frame() leaves the frames of ticks that come more than a tick late undrawn.
void set_tick(std::chrono::steady_clock::time_point tick, std::chrono::duration<double> period,
              std::chrono::duration<double> late = std::chrono::duration<double>(0.0));
bool interpolating();

// The game's text lookup, asked for an id of the port's own (the rows it adds to the game's
// options, options_menu.h): the string in game memory, or 0.
std::uint32_t text(int id);
bool wants_between();
int between(std::chrono::steady_clock::time_point next_tick);

// An image the game sent to the GS outside the display list (its library's image transfer): kept
// for every renderer the window makes, as the chip keeps it in its memory. `base` in 256-byte
// blocks, `width_units` in 64 pixels, `pixels` in raster order in `psm`.
void upload_image(
    std::uint32_t base,
    std::uint32_t width_units,
    std::uint8_t psm,
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t width,
    std::uint32_t height,
    std::span<const std::uint8_t> pixels
);

// The game reading a PSMCT32 rectangle of its draw buffer back (sceGsExecStoreImage): answered
// with the last frame drawn, scaled to the chip's pixels. False if the rectangle is not in the
// draw buffer (nothing written).
bool store_image(std::uint32_t base, std::uint32_t width_units, std::uint8_t psm, int x, int y,
                 int width, int height, std::span<std::uint8_t> out);

// Shows one full-screen picture the game puts straight into its display buffer (a movie's frame,
// a boot still), RGBA rows top to bottom, with `black` (0 to 1) of black over it (the game's
// FadeToBlack), and reads the input. Counts as a frame (shots, OPENRAC_PRESS). False once the window
// was closed.
bool show_picture(const std::uint8_t* rgba, int width, int height, float black);

// Window frames since the player got control in a scripted run (OPENRAC_PRESS's and OPENRAC_WRITE's
// c entries), -1 before.
std::int64_t since_control();

// The pad of a port as the game's pad library reads it: buttons active low, then the right and
// left sticks (x, y each).
bool pad(int port, std::uint16_t* buttons, std::uint8_t analog[4]);

// While the debug overlay holds the game (its pause): draws the last tick again and reads the
// window's events until the overlay lets a tick through. False once the window was closed.
bool hold();

void close();

}  // namespace openrac::frontend

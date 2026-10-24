// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the files of a game's program share, and nothing outside the program includes: main.cpp's
// state that the host side of game_host.h (program/game_host.cpp) reads and writes, and the two
// long bodies of game_host.h's entry points that main.cpp still holds.
//
// The state is file-global, as it was inside main.cpp: the split of main.cpp into program/ gathers
// it into state of its own (CODING_CONVENTIONS.md, sections 11.4 and 24). Only the game's thread,
// the one that runs the game's main, reads or writes it.

#pragma once

#include <cstdint>

#include "openrac/guest.h"

namespace openrac::program {

// The game's frames so far: one for each vertical blank it waited for (openrac_game_vsync), and
// one for each blank of a movie.
extern long g_frame;

// The display list sent this frame: the game address of its first DMA tag
// (openrac_game_dma_send), which the window draws from.
extern gaddr g_chain;

// A vertical blank was waited for since the last frame was sent: a frame sent with none ends at
// the send instead (openrac_game_dma_send).
extern bool g_vsync_since_kick;

// The program has a window (--window) and the renderer draws the game's frames; false without one
// (no window asked for, or a program built without it).
extern bool g_window;

// What the game's renderers drew since the last frame (openrac_game_draw: OPENRAC_DRAW_* bits),
// cleared at each vertical blank.
extern unsigned g_draws;

// The game reports what it draws (it called openrac_game_draw once): until it has, the window draws
// every layer.
extern bool g_reports_draws;

// openrac_game_vsync's body (game_host.h): ends the game's frame. Draws it in the window (or only
// paces it without one), runs the per-frame work of the program (the sound, the crash report's
// state, the mods, the debug variables), and waits for the next tick's time. Returns the field the
// console would report, 0 or 1 by turns. Ends the program when the window closes, or at --frames.
int vsync();

// openrac_game_play_movie's body (game_host.h): plays the PSS movie of `bytes` bytes at sector
// `lsn` from the player's disc image in the window, with the ADPCM channel `channel`, blocking as
// the game's own player does; Start skips it when `start_skips`. Returns 1 if skipped (and with
// --skip-movies), 0 if played to the end or when there is no window or no movie support to play it.
int play_movie(std::uint32_t lsn, std::uint32_t bytes, int channel, int start_skips);

}  // namespace openrac::program

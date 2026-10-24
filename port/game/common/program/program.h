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

#include <chrono>
#include <cstdint>

#include "openrac/guest.h"
#include "program/options.h"

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

// The command line, read by main before anything else runs (program/options.h).
extern Options g_options;

// When the game's next tick is due: vsync waits for it, and the movie player paces its blanks
// by it. Zero until the first tick.
extern std::chrono::steady_clock::time_point g_next_frame;

// The game's ticks a second: 60 for every version. The PAL game runs in its own 60 Hz mode
// (options_menu.cpp keeps it there), so its time base is a sixtieth of a second too.
int tick_rate();

// Ends the run's records: OPENRAC_REACH's file written, the functions without C that were reached
// logged, and the frame count. main has it run at exit.
void finish();

#ifdef OPENRAC_FRONTEND
// The window was closed: the player quit, or Android ended the game's activity (closed from
// Recents, or the launcher's Stop). On a desktop that is std::exit. On Android the game is often
// in the background then, its surface already gone, and exit's destructors would tear the renderer
// and the window down against a driver that lost it, while SDL's onDestroy frees its own state a
// second after asking the game to quit; a close from Recents was written down as a crash. There
// the game runs exit's handlers itself and ends at once; the system frees the rest.
[[noreturn]] void leave();
#endif

// OPENRAC_AUDIO_TICKED=1: no sound device. The game's sounds advance by one tick's samples each
// tick (rendered and dropped), and streams are read when they start, so every answer the game
// gets back from the sound engine (a voice's state, a stream's position) lands on the same tick, however
// fast the machine runs it: port/tools/regress's runs repeat. A device pulls at the real clock's
// rate, which an uncapped run outpaces by a varying amount.
bool audio_ticked();

// Renders one tick's samples and drops them, with OPENRAC_AUDIO_TICKED and a window (audio_ticked).
void audio_tick();

#ifdef OPENRAC_FRONTEND
// Every way out runs this (at exit, or in leave): the output's thread stops before the globals it
// renders from go (its callback ran on into their destruction, a crash at quit).
void close_sound();

// REPORT: whether a device that cannot be opened is logged (the first try is; the retries,
// every few seconds on a machine without sound, are not).
void open_sound(bool report = true);

// Once a frame: a device that stalled is opened again, and one that could not be opened is tried
// again every few seconds (a phone's audio can be busy with a call or another app at the start).
void watch_sound();
#endif

// A level's program: the pieces the game's ParseBin copies in, each a header (load address, size,
// a word, entry) and its bytes, while the entry stays the same. For the runtime's level-to-level
// relocation: a function several levels share reads its globals where the loaded level keeps
// them, which the runtime finds by pairing the function's code in the two programs. Without it
// such a function reads the address of the level its name gives, where the loaded level has
// something else: the arctangent's tables were zero in a level, so the camera's angles were zero
// and the stick moved the hero along the world's axes whatever way the camera faced.
// From the extractor's copy when there is one (decompiler_out/<game>/levels/NN/overlay.bin), else
// from the player's image.
void load_level_program(int level);

#ifdef OPENRAC_FRONTEND
// --check-levels: every level the window draws from (level_00 to level_18, the flight world 98 and
// the title's 99) is read as the window reads it, without a window. A level that cannot be read,
// or that has nothing to draw, is named with the reason; when all are whole, `verified-<format>.json`
// (the extracted levels' format, viewer::kManifestFormat) is written beside them, which the launcher
// asks for before it offers Play. A game started over levels that are missing or cut short shows
// its HUD over a black world and says nothing else; levels of an older format fail the check (their
// manifests are refused), so the launcher reads them from the disc again.
int check_levels(const fs::path& levels);
#endif

}  // namespace openrac::program

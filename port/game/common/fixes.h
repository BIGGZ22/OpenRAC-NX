// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The game fixes: what the port changes from the console's behaviour that a player may switch back.
// Each is an entry of kind "fix" in the game's list of differences (port/game/<game>/differences.json),
// a setting of its own in the settings file (`fix_... = 1`, settings.cpp), a row of the Options
// page's "Game fixes" group (options_menu.cpp) and a switch on the launcher's page for the game.
// docs/port/SETTINGS.md section 8 says how one is added.
//
// The table is the program's disc's fixes, written at configure time from that file
// (cmake/Differences.cmake). The code a fix changes asks for its flag once
// (openrac_game_fix, game_host.h) and reads it through the pointer from then on.
#pragma once

namespace openrac::fixes {

struct Fix {
    const char* key;   // the setting's key: fix_wide_moby_effects
    const char* name;  // the Options row's label: Wide-screen effects
    bool on;           // the default: on is the port's fix, off the console's behaviour
};

// The most a game may have (the flags are a fixed array, so a pointer to one stays good).
inline constexpr int kMax = 32;

// The program's fixes, in the file's order.
int count();
const Fix& fix(int index);

// The index of the fix with that setting key, or -1.
int find(const char* key);

// Whether a fix is on, and setting it (an index out of range: false, nothing).
bool on(int index);
void set(int index, bool on);

// Every fix back to its default (settings::load does this before reading the file).
void reset();

// The flag the code reads: stays at the same address for the program's life.
const unsigned char* flag(int index);

}  // namespace openrac::fixes

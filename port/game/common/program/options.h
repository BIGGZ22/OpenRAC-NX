// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The game program's command line and the paths it reads by default: what each option sets
// (Options), the folders the launcher keeps a player's cards, settings and levels in, and the
// command line as the session's log shows it. main.cpp parses the command line once, before
// anything else, into program.h's g_options.

#pragma once

#include <filesystem>
#include <string>

namespace openrac::program {

namespace fs = std::filesystem;

// What the command line asked for, with the defaults parse fills in.
struct Options {
    fs::path data;
    fs::path cards;
    long frames = -1;
    bool keep_going = true;
    bool window = false;
    bool skip_movies = false;  // development probes; not movie-playback validation
    fs::path levels;           // the extracted levels the window draws (level_00, ...)
    fs::path
        mods;  // levels edited in Godot and packed (editor/pack.py), read in place of the disc's
    fs::path settings;  // the player's settings (settings.h); in OpenRAC's data folder unless given
    fs::path plugins;   // the native mods' list (mods.h): <data>/plugins/plugins.json unless given
    fs::path probes;    // the player's probe bakes, a level_NN folder each (enhanced rendering)
    bool no_mods = false;  // purist mode: no native mod is loaded (docs/port/MODS.md)
    fs::path
        debug_dump;  // each frame's measurements, a JSON object a line (docs/port/DEBUGGING.md)
    bool debug_overlay = false;  // the debug overlay shown from the start (F3 shows and hides it)
    bool check_levels =
        false;  // read every extracted level as the window would, say what is wrong, and stop
};

// Returns this program's name, openrac-<id> (openrac-rac1-pal).
std::string program_name();

// This program's version: "rac1-pal" -> pal.
std::string game_version();

// Each version's levels sit in a folder of their own (<data>/port/<version>, the checkout's
// assets/port/<game>/<version>). PAL's levels extracted before that layout sit straight in the
// folder above (the only version that had them) and are read there until extracted again, whether
// the folder came from --levels or the default.
fs::path levels_or_flat(const fs::path& own);

// Where the levels are without --levels.
fs::path default_levels(const fs::path& data);
// On-device installs keep the original ISO in place, with a small path record.
fs::path installed_disc(const fs::path& data);

// Reads the command line into Options, with the defaults where an option is not given. Prints the
// usage and ends the program (exit code 64) on an unknown option, a missing value, or no data
// folder. --no-card and --level set game_host.h's openrac_game_no_card and
// openrac_game_start_level.
Options parse(int argc, char** argv);

// The command line as it was typed, quoted where needed, with the home folder as ~.
std::string command_line(int argc, char** argv);

}  // namespace openrac::program

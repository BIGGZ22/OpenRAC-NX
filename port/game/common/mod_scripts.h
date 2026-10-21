// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Script mods (docs/port/MODS.md, section 11): each one's .luau files run in a sandboxed Luau state
// of its own, with the `openrac` table as their only way into the game. The loader (mods.cpp) calls
// load() for a script mod while its load is under way, as it calls a native mod's openrac_mod_load:
// the script's hooks and callbacks go through the same functions a native mod's do, so script and
// native mods share one chain per hook point, in the list's order. The game's binding
// (openrac/script_binding.h, generated) gives the functions the hook points' slots hold and the
// fields exposed by name. Nothing here runs, and no Luau state exists, until a script mod loads.

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "mod_manifest.h"
#include "openrac/openrac_mod.h"
#include "openrac/script_binding.h"

namespace openrac::mods::scripts {

// What the loader lends the scripts: the same functions a native mod gets (mods.cpp), and what to
// do when a script mod stops on an error or its budget.
struct Services {
    int (*hook)(openrac_mod* mod, const char* name, const char* signature, void* replacement, void** original);
    int (*on_frame)(openrac_mod* mod, openrac_frame_fn fn, void* user);
    int (*on_pad)(openrac_mod* mod, openrac_pad_fn fn, void* user);
    void (*log)(openrac_mod* mod, int level, const char* text);
    void (*stopped)(openrac_mod* mod, const std::string& reason);
};

struct Mod {
    openrac_mod* handle = nullptr;
    std::string id;
    std::filesystem::path folder;      // the mod's own folder: its scripts, and what require() reads
    std::vector<std::string> scripts;  // the manifest's, run in order
    std::map<std::string, SettingValue> settings;
    std::string game, version, platform, api;
};

// Makes the mod's state and runs its scripts, which take their hooks and callbacks as they run.
// False, and why, when a script does not compile, raises an error or runs over its budget.
bool load(const Mod& mod, const openrac_script_binding* binding, const Services& services, std::string& reason);

// Closes one mod's state and frees its places in the chains (after the loader has put the slots it
// took back): a mod whose load failed.
void close(openrac_mod* handle);

// Closes every state (tests, and the loader's reset).
void reset();

// How many Luau states are open: 0 while no script mod is on.
int state_count();

// The time a script may run each time it is called, in milliseconds (OPENRAC_SCRIPT_BUDGET_MS,
// default 2); tests set it.
void set_budget_ms(double ms);

}  // namespace openrac::mods::scripts

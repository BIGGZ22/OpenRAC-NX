// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The port's settings (settings.h) in the game's own options: the rows of the front end's Video
// options page, drawn and navigated by the game's widgets in the game's font, in place of the game's
// own rows (PAL's "Video mode", 50 or 60 Hz, which means nothing to a window). For a game whose
// program describes its front end (game_host.h, openrac_front_end): Ratchet & Clank, every disc.
#pragma once

#include <cstdint>
#include <string>

namespace openrac::frontend::options_menu {

// The rows: one per setting, whichever menu shows them (the game's options pages here, the
// overlay's menus and drawer, debug_overlay.cpp).
enum Row {
    kDisplayMode,
    kMonitor,
    kResolution,
    kAspect,
    kHud,
    kRenderScale,
    kMsaa,
    kAnisotropy,
    kVSync,
    kFrameRate,
    kFrameCap,
    kShowFps,
    kMuteUnfocused,
    kSkipCutscenes,
    kRows
};

// A row's name, and its setting's value now, as the player reads them.
const char* label(int row);
std::string value(int row);

// A row moved to its next value (by = 1) or the one before (by = -1), within what the graphics
// card offers. Not saved (settings::save) and not applied: the window applies what changed.
void step(int row, int by);

// Once a frame: while the boot program runs (the title and its menus), the Video options page
// shows the port's rows, four at a time, moving with the cursor. Cross or Right moves the row under
// the cursor to its next value, Left to the one before. True when a setting changed (it is saved
// already; the window applies it at once: none needs the game started again).
bool update(int loaded_overlay);

// The items the Options list is given in place of the game's own (with the Video page's added), or 0
// while it has the game's.
std::uint32_t options_items();

// The game's text lookup, asked for an id of the port's own: the string in game memory, or 0 for
// an id that is the game's.
std::uint32_t text(int id);

}  // namespace openrac::frontend::options_menu

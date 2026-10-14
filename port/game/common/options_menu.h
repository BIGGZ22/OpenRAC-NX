// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The port's settings (settings.h) in the game's own options: the rows of the front end's Video
// options page, drawn and navigated by the game's widgets in the game's font, in place of the game's
// own "Video mode" row (50 or 60 Hz, which means nothing to a window). So far for Ratchet & Clank
// (PAL).
#pragma once

#include <cstdint>

namespace openrac::frontend::options_menu {

// Once a frame: while the boot program runs (the title and its menus), the Video options page
// shows the port's rows, four at a time, moving with the cursor. Cross or Right moves the row under
// the cursor to its next value, Left to the one before. True when a setting changed (it is saved
// already; the window applies it at once: none needs the game started again).
bool update(const char* game_id, int loaded_overlay);

// The game's text lookup, asked for an id of the port's own: the string in game memory, or 0 for
// an id that is the game's.
std::uint32_t text(int id);

}  // namespace openrac::frontend::options_menu

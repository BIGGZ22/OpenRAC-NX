// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The port's developer features (the launcher's developer mode: OPENRAC_DEVELOPER=1; without it
// the menus are exactly the game's). In Ratchet & Clank the front end's Options list gets a
// "Planets" entry, a page of the game's own making that lists the levels and starts the one
// picked. As a fallback, a level selector the port draws itself over the title and main menu:
// F1 (or Select on a pad) opens it while the boot program runs; Up and Down pick a level,
// Return / Space / Cross starts it, F1 / Backspace / Triangle closes it. The level is started the
// way --level N with OPENRAC_DIRECT does (game/rac1/host/boot.c): NewGameInit for that level,
// then the menus closed and the start flag set, so the game loads it as a new game's first level.
//
// What it needs of the game is the game's front end (game_host.h, openrac_front_end); a game
// without one gets neither. OPENRAC_DEBUG_MENU=FRAME opens it at that window frame (for scripted
// checks).

#pragma once

#include <cstdint>

#include <SDL3/SDL.h>

namespace openrac::frontend::debug_menu {

// A window event: true if the menu took it (the game does not see it).
bool event(const SDL_Event& event, int loaded_overlay);

// Once a frame, before the game's frame is drawn: runs a level start the menu asked for.
void update(int loaded_overlay, std::uint64_t frame);

// The game's pad 0 as the game will read it: while the menu is open its buttons move the menu
// and the game sees none pressed (active low: 0xFFFF).
void filter_pad(std::uint16_t& buttons, int loaded_overlay);

bool is_open();

// Draws the menu over the finished frame (`framebuffer`, `width` x `height` pixels: the game's
// 512 x 448 picture scaled), if it is open.
void draw(unsigned framebuffer, int width, int height);

// GL objects, before the context goes.
void release();

}  // namespace openrac::frontend::debug_menu

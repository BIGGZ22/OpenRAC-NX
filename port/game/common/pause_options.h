// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Ratchet & Clank: which pause menu page is its Options page (HelpDesk, Save, Load, Sound, Camera,
// Subtitles, Quit Game), whose list options_menu.cpp gives a Video item. Header only, so the decision
// is tested without the game (tests/pause_options_test.cpp).
//
// The pause menu is level data: MenuPage (0x88 bytes: seq[14]; +0x38 back, the parent page; +0x3C
// state, which update_menu_system makes menu_system.state while the page is current; +0x40 focus,
// the screen with the input; +0x44 screens[14]) and its MenuScreen widgets (+0x00 update, +0x04
// draw, +0x08 enter, +0x0C leave; from +0x30 each kind's own data). Only a text list (the game's
// update_menu_entry_actions and draw_menu_text_list) has its items at +0x34: an item grid keeps its
// margins there (floats such as 0.08 and 0.1), so the focus's +0x34 is a list only on a page known to
// be a list's. The widgets' functions are each level program's own copies, at other addresses in
// every level and version, so the kind is told by the page's state: the Options page's is 8 on both discs
// (seen in levels 1, 3, 8, 10 and 16; the pause menu's item grids' are 3 to 6, its fifth page's 7, the Camera
// page's 13), as the game itself tells its pages by it (update_menu_grid_selection: state 3).
#pragma once

#include <cstdint>

namespace openrac::frontend::pause_options {

constexpr std::uint32_t kPageState = 0x3C;
constexpr std::uint32_t kPageFocus = 0x40;
constexpr std::int32_t kOptionsState = 8;  // menu_system.state while the pause Options page is current

// The pause Options page's list screen, when `page` (menu_system.current) is that page and its focus
// is set; 0 for any other page. `read` reads a 32-bit word of game memory.
template <class Read32>
std::uint32_t options_list(Read32 read, std::uint32_t page) {
    if (page == 0 || static_cast<std::int32_t>(read(page + kPageState)) != kOptionsState) {
        return 0;
    }
    return read(page + kPageFocus);
}

}  // namespace openrac::frontend::pause_options

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

// The Options page's picture box: a resource stream screen (the game's update_menu_resource_stream,
// US FUN_0021fdc8, PAL func_00220DF0: +0x30 its entries, {sector, sector count} each, +0x34 its
// flags, +0x48 and +0x4C its two buffers) that reads entries[list.selected] from the disc into a
// buffer, one picture per Options item. Its flags pick no other index (fixed entry, level, grid cell,
// save slot, ...), so the index is the list's. A list given more items than the game's (the port's
// Video) has to give this table as many entries, or the items after it show the next item's picture
// and the last one reads past the table: on Quit Game a 446-sector read of whatever follows (PAL
// 0x138140 + 0x38) into the buffer, whose picture header asks for 0x10000 quadwords, the 16-bit
// DMA count wraps to 0 and the VIF's DIRECT 0 (65536 quadwords) swallows the rest of the 2D list: the
// Options list's labels.
constexpr std::uint32_t kStreamEntries = 0x30;
constexpr std::uint32_t kStreamFlags = 0x34;
constexpr std::uint32_t kStreamBuffer = 0x48;
constexpr std::uint32_t kStreamEntryBytes = 8;
// Flags that make the stream's index something else than the focus list's selection.
constexpr std::uint32_t kStreamOtherIndex = 0x1 | 0x2 | 0x4 | 0x8 | 0x100 | 0x400 | 0x1000;
constexpr std::uint32_t kMaxSectors = 0x800;  // a picture's: a few sectors (PAL's are 6 to 8)

// The screen of `page` (the Options page) that streams a picture for each of the `count` items of
// the list `list`, or 0. Told by its words: an entries table of `count` plausible entries, no flag
// for another index, a buffer.
template <class Read32>
std::uint32_t options_stream(Read32 read, std::uint32_t page, std::uint32_t list, int count) {
    if (page == 0 || count <= 0) {
        return 0;
    }
    for (std::uint32_t at = 0x44; at < 0x7C; at += 4) {
        const std::uint32_t screen = read(page + at);
        if (screen == 0 || screen == list) {
            continue;
        }
        const std::uint32_t entries = read(screen + kStreamEntries);
        if (entries < 0x100000u || entries >= 0x2000000u || (entries & 3) != 0
            || (read(screen + kStreamFlags) & kStreamOtherIndex) != 0 || read(screen + kStreamBuffer) == 0) {
            continue;
        }
        bool plausible = true;
        for (int i = 0; i < count && plausible; ++i) {
            const std::uint32_t entry = entries + kStreamEntryBytes * static_cast<std::uint32_t>(i);
            const std::uint32_t sectors = read(entry + 4);
            plausible = read(entry) != 0 && sectors != 0 && sectors <= kMaxSectors;
        }
        if (plausible) {
            return screen;
        }
    }
    return 0;
}

// The stream's entries for the port's list: the game's `count` entries (two words each) with an empty
// one (no sectors: the stream reads nothing and keeps the picture it has) at `before`, where the list
// has its Video item. `out` holds 2 * (count + 1) words.
inline void entries_with_video(const std::uint32_t* game, int count, int before, std::uint32_t* out) {
    for (int i = 0, o = 0; i < count; ++i, ++o) {
        if (i == before) {
            out[2 * o] = 0;
            out[2 * o + 1] = 0;
            ++o;
        }
        out[2 * o] = game[2 * i];
        out[2 * o + 1] = game[2 * i + 1];
    }
    if (before >= count) {
        out[2 * count] = 0;
        out[2 * count + 1] = 0;
    }
}

}  // namespace openrac::frontend::pause_options

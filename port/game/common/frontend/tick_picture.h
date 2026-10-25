// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What a tick's frames start from (frontend/frame.cpp's choose_repeat_frame): a cleared frame, the
// last picture standing unchanged, or the last picture with the tick's 2D drawn over it. On the
// console the screen shows the frame buffer the game last drew; a tick that draws no world into it
// leaves that picture there, and the 2D it draws goes over it. The page menus' darkening fades
// work so: the vendor's opening darkens the displayed frame by a quarter a tick for four ticks
// while the world stops. Drawn from a cleared frame, those ticks showed the level's background
// colour for a frame (a cyan flash on Pokitaru).

#pragma once

#include <cstddef>
#include <cstdint>

namespace openrac::frontend {

// The world layers a tick can draw (sky, terrain, ties, shrubs, mobys: Kept::draws' bits 0 to 4).
inline constexpr std::uint32_t kWorldLayers = 0x1Fu;
// A 2D list at least this long holds a page menu's read-back picture of the frame.
inline constexpr std::size_t kMenuSnapshotBytes = 0x40000;
// The ticks in a row after a page menu closes whose last picture stands (frontend/state.h).
inline constexpr int kMostMenuGapTicks = 3;

// How a tick's frames start.
enum class TickPicture {
    Draw,      // cleared, the world and the 2D drawn as always
    Repeat,    // the last picture stands; nothing is drawn
    Overdraw,  // the last picture stays and the tick's 2D goes over it, once a tick
};

// What decides it, of one tick.
struct TickFacts {
    std::uint32_t draws = 0;       // the world layers drawn (Kept::draws)
    bool has_camera = false;       // a world camera was captured
    bool in_level = false;         // a level program is loaded (not the boot program)
    std::size_t packets = 0;       // the 2D list's bytes
    std::size_t before_world = 0;  // of those, the bytes before the world
    bool has_picture = false;      // a frame has been drawn to stand or go over
};

// Decides each tick's start from it and the ticks before. A value, used by the game's thread.
class TickPictures {
public:
    [[nodiscard]] TickPicture choose(const TickFacts& tick) {
        const bool world = (tick.draws & kWorldLayers) != 0;
        const bool menu_list = tick.before_world >= kMenuSnapshotBytes;
        const bool menu_gap = !world && menu_list;
        m_menu_gap_ticks = menu_gap ? m_menu_gap_ticks + 1 : 0;
        // A world with no 2D list after it is a list the game has not finished: the first tick of
        // the vendor's fly-in and of a scene (no list at all), the first world tick after a fade's
        // ticks without one (the flight's take-off: only the fade's clears, before the world).
        // Drawn, it showed the world unfaded for a frame between the fade's black.
        const bool nothing_after_world = tick.packets <= tick.before_world;
        const bool unfinished = tick.has_camera && world
                                && (tick.packets == 0 || (nothing_after_world && !m_last_world));
        // A level's tick with a camera and no world, whose list is not a page menu's: the picture
        // shown stays under its 2D.
        const bool over = !world && !menu_list && tick.has_camera && tick.in_level;
        m_last_world = world;
        if (!tick.has_picture) {
            return TickPicture::Draw;
        }
        if ((menu_gap && m_menu_gap_ticks <= kMostMenuGapTicks) || unfinished) {
            return TickPicture::Repeat;
        }
        return over ? TickPicture::Overdraw : TickPicture::Draw;
    }

private:
    int m_menu_gap_ticks = 0;  // ticks in a row with no world under a page menu's list
    bool m_last_world = true;  // the tick before drew a world layer
};

}  // namespace openrac::frontend

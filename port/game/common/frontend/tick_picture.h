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
    int level = -1;                // the level program loaded (-1: the boot program)
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
        // Another level program loaded forgets this level's pictures: what was shown last is
        // another program's (the loading picture, when a save is loaded in play, whose camera is
        // still in the game's memory), not this level's frame.
        if (tick.level != m_level) {
            m_world_shown = m_world_shown && m_level == kNoLevel;
            m_level = tick.level;
        }
        // A level's tick with a camera and no world, whose list is not a page menu's: the picture
        // shown stays under its 2D, while that picture is a world's. Over a page menu's (the
        // in-game Load Game page, whose loaded save's story card comes while the old level and
        // its camera are still in memory) it starts from a cleared frame, as from the title.
        const bool over = !world && !menu_list && tick.has_camera && tick.in_level && m_world_shown;
        m_last_world = world;
        TickPicture picture = TickPicture::Draw;
        if (tick.has_picture
            && ((menu_gap && m_menu_gap_ticks <= kMostMenuGapTicks) || unfinished)) {
            picture = TickPicture::Repeat;
        } else if (tick.has_picture && over) {
            picture = TickPicture::Overdraw;
        }
        // A drawn tick's picture is a world's when it drew one; the others keep the picture shown.
        if (picture == TickPicture::Draw) {
            m_world_shown = world;
        }
        return picture;
    }

private:
    static constexpr int kNoLevel = -2;  // no tick chosen yet

    int m_menu_gap_ticks = 0;   // ticks in a row with no world under a page menu's list
    bool m_last_world = true;   // the tick before drew a world layer
    int m_level = kNoLevel;     // the level program of the tick before
    bool m_world_shown = true;  // the picture shown is a world drawn in that program
};

}  // namespace openrac::frontend

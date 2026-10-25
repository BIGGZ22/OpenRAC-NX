// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Which ticks of a level keep the window's last picture (program/vsync.cpp). On a tick where the
// game draws nothing between drawn ones (rac1: the tick a skipped landing ends on), the console
// shows its back buffer, which still holds the last whole frame; the window draws nothing that
// tick and keeps what it shows. Drawn with no layers, the world was gone for a frame (a flash).

#pragma once

#include <cstdint>

namespace openrac::program {

// The most ticks in a row that keep the last picture: a loading card or a menu over black, which
// the game draws with no layers for longer, is drawn as it comes after them.
inline constexpr int kMostUndrawnTicks = 2;

// Counts a level's ticks on which the game drew nothing, from a tick on which it drew.
//
// A value, copied freely. vsync's one instance is used by the game's thread only.
class UndrawnTicks {
public:
    // Returns whether this tick keeps the last picture instead of drawing: the game reports what
    // it draws (`reports_draws`), drew no layer this tick (`draws`, OPENRAC_DRAW_* bits, all set
    // while it reports nothing), drew on the tick before or on the run of kept ticks before it,
    // that run is shorter than kMostUndrawnTicks, and a level is loaded (`in_level`).
    [[nodiscard]] bool keep(bool reports_draws, std::uint32_t draws, bool in_level) {
        const bool kept =
            reports_draws && draws == 0 && m_last_drew && m_kept < kMostUndrawnTicks && in_level;

        m_kept = kept ? m_kept + 1 : 0;
        m_last_drew = kept || draws != 0;
        return kept;
    }

private:
    bool m_last_drew = false;  // The tick before drew a layer, or kept the picture.
    int m_kept = 0;            // The ticks in a row that kept the picture.
};

}  // namespace openrac::program

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Deadlocked's profile for NTSC-U (game/common/profile.h): nothing of its memory is read yet, as
// for a game with no profile, but the frame it shows is the display list it sends through VIF1
// DMA, without Ratchet & Clank's two alternating list buffers. It defines game_profile() for
// openrac-rac4-ntsc.

#include "game/common/profile.h"

#include <string_view>

namespace openrac::game::rac4 {
namespace {

// The game's profile: the base's answers, and the display list it sends.
class Profile final : public host::GameProfile {
public:
    bool sends_display_list() const override { return true; }
};

// The one instance, a constant for the program's life.
const Profile kProfile;

}  // namespace
}  // namespace openrac::game::rac4

namespace openrac::host {

const GameProfile& game_profile(std::string_view /*game*/) {
    return game::rac4::kProfile;
}

}  // namespace openrac::host

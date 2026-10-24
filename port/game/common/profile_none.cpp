// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The profile of a game with none of its own (game/common/profile.h): the base GameProfile, which
// knows nothing of the game's memory, so the host reads no game state from it. The camera stays
// where GameState starts it, no moby is live, and no display list is known to be shown. A game
// gets a profile with port/game/<id>/profile.cpp or port/game/<game>/profile.cpp, which
// cmake/Games.cmake links in this file's place, as it does for script_binding_none.c.

#include <string_view>

#include "game/common/profile.h"

namespace openrac::host {
namespace {

// The one instance, a constant for the program's life.
const GameProfile kNoProfile;

}  // namespace

const GameProfile& game_profile(std::string_view /*game*/) {
    return kNoProfile;
}

}  // namespace openrac::host

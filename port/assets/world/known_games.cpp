// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Implements known_games.h: the gate every world reader passes its game through. The check is one
// comparison today; it lives in one place so that adding a sequel's layouts means changing it here.

#include "assets/world/known_games.h"

#include "assets/bytes.h"

namespace openrac::assets {

// Only rac1's layouts are known (see the header).
bool world_layout_known(Game game) {
    return game == Game::Rac1;
}

void require_world_layout(Game game, std::string_view what) {
    // A reader that guessed another game's layout would decode garbage, so refuse.
    if (!world_layout_known(game)) {
        fail("{}: the {} layout is not known yet (only rac1's is)", what, game_name(game));
    }
}

}  // namespace openrac::assets

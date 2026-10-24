// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The profile of a game with none of its own (game/common/profile.h): it knows nothing of the
// game's memory, so the host reads no game state from it. The camera stays where GameState starts
// it, no moby is live, and no display list is known to be shown (the frontend then draws the list
// the game sends, or none). A game gets a profile with port/game/<game>/profile.cpp, which
// cmake/Games.cmake links in this file's place, as it does for script_binding_none.c.

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

#include "game/common/profile.h"
#include "renderer/math.h"

namespace openrac::host {
namespace {

// The profile that reports no game state.
class NoProfile final : public GameProfile {
public:
    GameState read_state(std::span<const std::uint8_t> /*ram*/, int /*level*/) const override {
        return {};
    }

    std::optional<LiveMoby> read_moby(
        std::span<const std::uint8_t> /*ram*/, std::uint32_t /*moby*/
    ) const override {
        return std::nullopt;
    }

    bool beyond_draw_distance(
        const std::uint8_t* /*moby*/,
        const renderer::Vec3& /*eye*/,
        const renderer::Vec3& /*forward*/
    ) const override {
        return false;
    }

    std::uint32_t shown_chain(std::span<const std::uint8_t> /*ram*/, int /*level*/) const override {
        return 0;
    }
};

// The one instance, a constant for the program's life.
const NoProfile kNoProfile;

}  // namespace

const GameProfile& game_profile(std::string_view /*game*/) {
    return kNoProfile;
}

}  // namespace openrac::host

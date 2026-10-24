// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Ratchet & Clank's profiles (game/common/profile.h): one per disc, PAL (SCES-50916) and NTSC-U
// (SCUS-97199), each answering the host program with game_state.h's reads over its own tables of
// addresses (game_state.cpp), the boot program's and each level program's, and with the
// executable's addresses the frontend reads for the hero's shadow and state.
//
// It defines game_profile() for the programs that link it, and so picks which disc's profile a
// game id names, as rac1_addresses() does.

#include "game/common/profile.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

#include "game/rac1/game_state.h"
#include "renderer/math.h"

namespace openrac::game::rac1 {
namespace {

// The number of level programs, each with its own copy of the engine's globals and so its own
// table in game_state.h.
constexpr int kLevels = 19;

// What one disc's executable keeps at addresses of its own, outside game_state.h's tables.
struct ExecutableAddresses {
    std::uint32_t shadow_directions;  // The four directions the hero's shadow is cast along.
    std::uint32_t hero_state;         // The hero's record (games/rac1's D_0013F450 on PAL).
};

// Each disc's executable addresses: PAL's as games/rac1 has them, NTSC-U's the same data in
// SCUS-97199 (rac1-ntsc's rac1_version.h pairs the hero's record).
constexpr ExecutableAddresses kPalExecutable{0x001CAE00u, 0x0013F450u};
constexpr ExecutableAddresses kNtscExecutable{0x001CAA80u, 0x0013F350u};

// One disc's profile: its boot program's table, its level programs' tables and its executable's
// addresses. Ratchet & Clank's levels are extracted for the port, and its frame is one of two list
// buffers it alternates (the defaults of host::GameProfile).
class Profile final : public host::GameProfile {
public:
    // Makes the profile of the disc whose tables are `boot`, `levels` and `executable`, which
    // outlive it.
    constexpr Profile(
        const GameAddresses& boot,
        const GameAddresses (&levels)[kLevels],
        const ExecutableAddresses& executable
    ) noexcept
        : m_boot(boot),
          m_levels(levels),
          m_executable(executable) {}

    host::GameState read_state(std::span<const std::uint8_t> ram, int level) const override {
        return read_game_state(ram, addresses(level));
    }

    std::optional<host::LiveMoby> read_moby(
        std::span<const std::uint8_t> ram, std::uint32_t moby
    ) const override {
        return read_live_moby(ram, moby);
    }

    bool beyond_draw_distance(
        const std::uint8_t* moby, const renderer::Vec3& eye, const renderer::Vec3& forward
    ) const override {
        return moby_beyond_draw_distance(moby, eye, forward);
    }

    std::uint32_t shown_chain(std::span<const std::uint8_t> ram, int level) const override {
        return rac1::shown_chain(ram, addresses(level));
    }

    std::uint32_t shadow_directions() const override { return m_executable.shadow_directions; }

    std::uint32_t hero_state() const override { return m_executable.hero_state; }

    bool has_levels() const override { return true; }

private:
    // Returns the table of the program loaded: a level's, or the boot program's when `level` is
    // not one (-1 before the first level).
    const GameAddresses& addresses(int level) const {
        // Only a level number has a table of its own; the title and the menus run the boot program.
        if (level >= 0 && level < kLevels) {
            return m_levels[level];
        }

        return m_boot;
    }

    const GameAddresses& m_boot;               // The boot program's table, a constant.
    const GameAddresses (&m_levels)[kLevels];  // The level programs' tables, constants.
    const ExecutableAddresses& m_executable;   // The executable's addresses, a constant.
};

// The two discs' profiles, constants for the program's life.
const Profile kPalProfile{kRac1Pal, kRac1PalLevels, kPalExecutable};
const Profile kNtscProfile{kRac1Ntsc, kRac1NtscLevels, kNtscExecutable};

}  // namespace
}  // namespace openrac::game::rac1

namespace openrac::host {

const GameProfile& game_profile(std::string_view game) {
    // The NTSC-U disc's id picks its tables; any other id reads as PAL, as rac1_addresses() does.
    if (game == "rac1-ntsc") {
        return game::rac1::kNtscProfile;
    }

    return game::rac1::kPalProfile;
}

}  // namespace openrac::host

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the renderers need of a running Ratchet & Clank, read from its memory: the camera and the
// live mobys, as host::GameState (game/common/profile.h). rac1's profile (profile.cpp) answers the
// host program with these reads; the viewer and its tests call them directly.
//
// The memory is the game's 32 MB of main memory as the native port holds it (or a picture of it
// saved to a file, for working on the renderers without running the game). The addresses are the
// game's own, documented beside each field in game_state.cpp; they differ by version, so each
// version has a table. Nothing here draws.

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "game/common/profile.h"
#include "renderer/math.h"

namespace openrac::game::rac1 {

/** Where one version keeps what the renderers read. */
struct GameAddresses {
    std::uint32_t camera_position;  // 3 floats, game units
    std::uint32_t camera_rows;      // 3 rows of 4 floats: forward, left, up (world vectors)
    std::uint32_t view_context;     // UpdateViewContext's block; +0xB0 the horizontal FOV tangent
    std::uint32_t moby_first;       // pointer to the first moby
    std::uint32_t moby_end;         // pointer past the last
    float vertical_factor;          // the vertical tangent over the horizontal one (TV aspect)
    std::uint32_t chain_bases;      // the two display-list buffers (VU1_swapChain alternates them)
    std::uint32_t chain_index;      // which one is being built; the other is the frame shown
};

/** Ratchet & Clank, PAL (SCES-50916): the boot program's (title, menus) and each level's. */
extern const GameAddresses kRac1Pal;
extern const GameAddresses kRac1PalLevels[19];
/** Ratchet & Clank, NTSC-U (SCUS-97199): the same. */
extern const GameAddresses kRac1Ntsc;
extern const GameAddresses kRac1NtscLevels[19];
/** The table of a port game ("rac1-pal", "rac1-ntsc") for the program loaded: a level's, or the
 *  boot program's when level is not one. An unknown game reads as PAL. */
const GameAddresses& rac1_addresses(std::string_view game, int level);

/**
 * The table for the program loaded now: each level's program has its own copy of the engine, with
 * its globals at other addresses. The level number is in resident memory (0x15EE84, -1 before
 * the first level).
 */
const GameAddresses& rac1_pal_addresses(std::span<const std::uint8_t> ram);

/**
 * Reads the state from main memory.
 *
 * @param ram The 32 MB of main memory.
 * @param a The version's addresses.
 * @return The state; empty moby list when the table pointers are not inside memory.
 */
host::GameState read_game_state(std::span<const std::uint8_t> ram, const GameAddresses& a);

/** The moby at address m as the renderers place it; none when its position or scale is unusable. */
std::optional<host::LiveMoby> read_live_moby(std::span<const std::uint8_t> ram, std::uint32_t m);

/**
 * Whether MobyProc leaves a moby out by its distance, as ReRAC's moby_lod.rs replays it: the
 * bounding sphere (+0x00: x y z r in 1/1024 units) has its centre deeper along the view than the
 * draw distance (+0x32, s16 units), or lies wholly behind the eye. The side planes are left out:
 * the window's picture can be wider than the game's.
 *
 * @param moby The moby's 0x100 bytes.
 * @param eye The camera's position, game units.
 * @param forward The camera's view direction (unit).
 */
bool moby_beyond_draw_distance(const std::uint8_t* moby, const renderer::Vec3& eye, const renderer::Vec3& forward);

/** The display list of the frame being shown (the buffer VU1_swapChain retired last). */
std::uint32_t shown_chain(std::span<const std::uint8_t> ram, const GameAddresses& a);

/** Reads a 32 MB memory picture from a file. */
bool load_memory(const std::filesystem::path& path, std::vector<std::uint8_t>& out, std::string& error);

}  // namespace openrac::game::rac1

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * What the renderers need of a running Ratchet & Clank, read from its memory: the camera and the
 * live mobys.
 *
 * The memory is the game's 32 MB of main memory as the native port holds it (or a picture of it
 * saved to a file, for working on the renderers without running the game). The addresses are the
 * game's own, documented beside each field in game_state.cpp; they differ by version, so each
 * version has a table. Nothing here draws.
 */

#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <string>
#include <vector>

#include "renderer/math.h"

namespace openrac::viewer {

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

/** A moby as the renderers place it. */
struct LiveMoby {
    std::uint32_t address = 0;  // the moby in game memory (its animation fields, its class)
    int class_id = 0;
    renderer::Mat4 matrix = renderer::identity();  // game axes, the moby's scale included
};

/** The game's state at one frame. */
struct GameState {
    renderer::Vec3 camera_position{};
    renderer::Vec3 forward{1, 0, 0}, left{0, 1, 0}, up{0, 0, 1};
    float tan_half_fov_x = 0.63f, tan_half_fov_y = 0.48f;
    /** The focal lengths in the view context, in the chip's pixels (+0xC0 and +0xD4, a 32nd of
     *  them): 256 / 0.63 and 208 / 0.488 for the frame. 0 when unreadable. */
    float focal_x = 0.0f, focal_y = 0.0f;
    std::vector<LiveMoby> mobys;
    /** The fog UpdateFog left in the view context (+0x218): FOGCOL (display bytes, 0..1), the
     *  depths it starts and ends at (raw units, game units x 1024) and the GS fog value F at those
     *  depths (255 = none). F is linear in camera depth, clamped between the two. */
    std::array<float, 3> fog_colour{0, 0, 0};
    float fog_near = 0.0f, fog_far = 1.0f, fog_near_f = 255.0f, fog_far_f = 255.0f;

    /** The camera's view matrix, in the renderer's convention (looking down -Z, Y up). */
    renderer::Mat4 view() const;

    /** A reversed-depth projection with the game's field of view, for a target's aspect. */
    renderer::Mat4 projection(float aspect, float near_z, float far_z) const;
};

/**
 * Reads the state from main memory.
 *
 * @param ram The 32 MB of main memory.
 * @param a The version's addresses.
 * @return The state; empty moby list when the table pointers are not inside memory.
 */
GameState read_game_state(std::span<const std::uint8_t> ram, const GameAddresses& a);

/** The moby at address m as the renderers place it; none when its position or scale is unusable. */
std::optional<LiveMoby> read_live_moby(std::span<const std::uint8_t> ram, std::uint32_t m);

/** The display list of the frame being shown (the buffer VU1_swapChain retired last). */
std::uint32_t shown_chain(std::span<const std::uint8_t> ram, const GameAddresses& a);

/** Reads a 32 MB memory picture from a file. */
bool load_memory(const std::filesystem::path& path, std::vector<std::uint8_t>& out, std::string& error);

}  // namespace openrac::viewer

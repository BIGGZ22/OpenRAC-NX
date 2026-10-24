// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the host program asks of the game it runs, in C++: a game version's profile. The host's
// shared code (the frontend, the debug overlay) knows no game (CODING_CONVENTIONS.md, section 15);
// where it needs a game's memory read (the camera, the live mobys, the display list shown), it
// asks the profile, and the game's folder answers with its own addresses and layouts.
//
// One profile per game version, which picks the version's tables as rac1_version.h does for C. A
// game's program links one definition of game_profile() (cmake/Games.cmake): its version's own
// (port/game/<id>/profile.cpp), else its game's (port/game/<game>/profile.cpp, rac1's for both of
// its discs), else game/common/profile_none.cpp, which reports no game state. The C side of what a
// game gives the host is openrac/game_host.h.

#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "renderer/math.h"

namespace openrac::host {

// A moby as the renderers place it.
struct LiveMoby {
    std::uint32_t address = 0;         // The moby in game memory (its animation fields, its class).
    int class_id = 0;                  // Its class, as the game numbers them.
    std::uint16_t occlusion = 0x7f80;  // Its occlusion word, as the game keeps it.
    std::array<float, 4> world_sphere{};  // World centre and radius, game units; radius 0: none.
    renderer::Mat4 matrix = renderer::identity();  // Game axes, the moby's scale included.
};

// The game's state at one frame: the camera the world is drawn from, and the live mobys.
struct GameState {
    renderer::Vec3 camera_position{};  // The eye, in game units.
    renderer::Vec3 forward{1, 0, 0};   // The unit axis the camera looks along.
    renderer::Vec3 left{0, 1, 0};      // The unit axis to its left.
    renderer::Vec3 up{0, 0, 1};        // The unit axis above it.
    float tan_half_fov_x = 0.63f;      // The tangent of half the horizontal field of view.
    float tan_half_fov_y = 0.48f;      // The tangent of half the vertical one.

    // The focal lengths in the view context, in the chip's pixels (+0xC0 and +0xD4, a 32nd of
    // them): 256 / 0.63 and 208 / 0.488 for the frame. 0 when unreadable.
    float focal_x = 0.0f;
    float focal_y = 0.0f;

    // The live mobys, in the order the game's table holds them.
    std::vector<LiveMoby> mobys;

    // The fog UpdateFog left in the view context (+0x218): FOGCOL (display bytes, 0..1), the
    // depths it starts and ends at (raw units, game units x 1024) and the GS fog value F at those
    // depths (255 = none). F is linear in camera depth, clamped between the two.
    std::array<float, 3> fog_colour{0, 0, 0};
    float fog_near = 0.0f;      // The depth the fog starts at.
    float fog_far = 1.0f;       // The depth it ends at.
    float fog_near_f = 255.0f;  // F at the near depth.
    float fog_far_f = 255.0f;   // F at the far depth.

    // Returns the camera's view matrix, in the renderer's convention (looking down -Z, Y up).
    renderer::Mat4 view() const { return renderer::look_along(camera_position, forward, up); }

    // Returns a reversed-depth projection with the game's vertical field of view; the horizontal
    // one follows `aspect`, the target's width over its height.
    renderer::Mat4 projection(float aspect, float near_z, float far_z) const {
        return renderer::perspective_reversed(
            2.0f * std::atan(tan_half_fov_y), aspect, near_z, far_z
        );
    }
};

// What one game version tells the host program about its memory.
//
// The base answers as a game nothing is known of: no game state (the camera as GameState starts
// it, no live moby), no display list known to be shown, no shadows, no hero, no extracted levels.
// A game's profile derives from it and overrides what its folder knows; a game with none uses the
// base itself (game/common/profile_none.cpp). Each profile is a constant of its game's folder,
// made once and never changed: the host keeps a reference to it for the program's life, and calls
// it from the window's thread only.
class GameProfile {
public:
    virtual ~GameProfile() = default;

    // Reads the camera and the live mobys from the game's 32 MB of main memory, `ram`. `level` is
    // the level program loaded now (-1 for the boot program, the title and the menus): each has its
    // globals at other addresses. A table pointer outside memory leaves the moby list empty.
    virtual GameState read_state(std::span<const std::uint8_t> /*ram*/, int /*level*/) const {
        return {};
    }

    // Reads the moby at game address `moby` as the renderers place it; none when its position or
    // scale is unusable.
    virtual std::optional<LiveMoby> read_moby(
        std::span<const std::uint8_t> /*ram*/, std::uint32_t /*moby*/
    ) const {
        return std::nullopt;
    }

    // Whether the game's moby update leaves the moby whose record starts at `moby` undrawn by its
    // distance from the camera at `eye`, looking along the unit axis `forward` (game units).
    virtual bool beyond_draw_distance(
        const std::uint8_t* /*moby*/,
        const renderer::Vec3& /*eye*/,
        const renderer::Vec3& /*forward*/
    ) const {
        return false;
    }

    // Returns the game address of the display list of the frame being shown: the one of the
    // game's two list buffers it is not building; 0 when none is known. `level` as for
    // read_state(). Not asked of a game that sends_display_list().
    virtual std::uint32_t shown_chain(std::span<const std::uint8_t> /*ram*/, int /*level*/) const {
        return 0;
    }

    // Whether the frame shown is the display list the game sends each tick, rather than one of
    // two list buffers it alternates (shown_chain()).
    virtual bool sends_display_list() const { return false; }

    // Returns the executable's address of the four shadow directions the hero's shadow is cast
    // along (relocated to the loaded program's copy when read); 0 for a game without them.
    virtual std::uint32_t shadow_directions() const { return 0; }

    // Returns the executable's address of the hero's state record (relocated when read); 0 for a
    // game without one.
    virtual std::uint32_t hero_state() const { return 0; }

    // Whether the port has the game's levels extracted (editor/port.py), so that a level whose
    // scenery cannot be read is something missing, not a game with no scenery yet.
    virtual bool has_levels() const { return false; }
};

// Returns the profile of the game version `game` (its port id, "rac1-pal"), which lives as long as
// the program. Defined once in each game's program, by the game's folder.
const GameProfile& game_profile(std::string_view game);

}  // namespace openrac::host

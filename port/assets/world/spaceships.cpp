// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "assets/world/spaceships.h"

#include "assets/world/known_games.h"

namespace openrac::assets {

SpaceshipFile read_spaceship_file(Game game, ByteView file) {
    require_world_layout(game, "spaceships file");
    const std::size_t ship = file.u32_at(0);
    const std::size_t texture = file.u32_at(4);
    const std::size_t extra = file.u32_at(8);
    const std::size_t extra_texture = file.u32_at(12);

    if (!(ship < extra && extra < texture && texture < extra_texture
          && extra_texture < file.size())) {
        fail(
            "spaceships file: unexpected block order {:#x} {:#x} {:#x} {:#x}",
            ship,
            texture,
            extra,
            extra_texture
        );
    }

    auto texture_list = [&](std::size_t at, u32 side) {
        if (file.u32_at(at) != 1 || file.u32_at(at + 4) != 0x10) {
            fail("spaceships texture list at {:#x}: not one PIF", at);
        }

        const PifImage pif = read_pif(game, file, at + 0x10);

        if (pif.width != side || pif.height != side) {
            fail(
                "spaceships texture at {:#x}: {}x{}, expected {}x{}",
                at,
                pif.width,
                pif.height,
                side,
                side
            );
        }

        return pif;
    };
    return {
        file.sub(ship, extra - ship),
        file.sub(extra, texture - extra),
        texture_list(texture, 256),
        texture_list(extra_texture, 128),
    };
}

}  // namespace openrac::assets

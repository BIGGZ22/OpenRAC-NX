// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

// Implements spaceships.h: splits a `spaceships` file into its two classes and two textures. The
// header's four offsets must be in the order the loader lays the blocks out, and each texture list
// must hold one PIF of the expected size, or the read fails.

#include "assets/world/spaceships.h"

#include "assets/world/known_games.h"

namespace openrac::assets {

// Class blocks are sliced between consecutive offsets; textures are parsed as PIFs.
SpaceshipFile read_spaceship_file(Game game, ByteView file) {
    require_world_layout(game, "spaceships file");
    const std::size_t ship = file.u32_at(0);
    const std::size_t texture = file.u32_at(4);
    const std::size_t extra = file.u32_at(8);
    const std::size_t extra_texture = file.u32_at(12);

    // The blocks are stored class, extra class, texture, extra texture; any other order means the
    // file is not a spaceships file, and the slices below would overlap.
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

    // Reads one texture list: a count of 1, the offset 0x10 of the first entry, then a PIF.
    auto texture_list = [&](std::size_t at, u32 side) {
        // Only the one-PIF form is known (seen in the loader's texture setup).
        if (file.u32_at(at) != 1 || file.u32_at(at + 4) != 0x10) {
            fail("spaceships texture list at {:#x}: not one PIF", at);
        }

        const PifImage pif = read_pif(game, file, at + 0x10);

        // The ship's texture is 256 square and the extra one 128 square; a different size means the
        // list was read from the wrong place.
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

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/pif.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The PIF header and its bounds.

#include "assets/world/pif.h"

#include <bit>
#include <cstring>

#include "assets/world/known_games.h"

namespace openrac::assets {

PifImage read_pif(Game game, ByteView bytes, std::size_t at) {
    require_world_layout(game, "PIF image");
    const ByteView magic = bytes.sub(at, 4, "PIF magic");

    if (std::memcmp(magic.data(), "2FIP", 4) != 0) {
        fail("no PIF at {:#x}", at);
    }

    PifImage pif;
    pif.width = bytes.u32_at(at + 8);
    pif.height = bytes.u32_at(at + 0xc);
    pif.format = bytes.u32_at(at + 0x10);

    if (pif.format != kPifFormatPsmt8) {
        fail("PIF at {:#x}: format {:#x} is not PSMT8", at, pif.format);
    }

    if (!std::has_single_bit(pif.width) || !std::has_single_bit(pif.height) || pif.width > 1024
        || pif.height > 1024) {
        fail("PIF at {:#x}: {}x{}", at, pif.width, pif.height);
    }

    pif.palette = bytes.sub(at + kPifPaletteOffset, 0x400, "PIF palette");
    pif.pixels =
        bytes.sub(at + kPifPixelsOffset, std::size_t{pif.width} * pif.height, "PIF pixels");
    return pif;
}

}  // namespace openrac::assets

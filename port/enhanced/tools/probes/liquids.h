// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "assets/bytes.h"
#include "enhanced/lighting/bake.h"

namespace openrac::lighting {
// Derive static transport from the reviewed NTSC Gaspar liquid grid and its
// animated FX textures. Inputs are ordinary decoded level sections, no game run.
std::vector<BakeTriangle> gaspar_liquid_triangles(
    assets::ByteView overlay, assets::ByteView index, assets::ByteView core
);
}  // namespace openrac::lighting

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The world renderers' per-version profiles: RAC1 known in both regions,
// the later games reported as not known rather than guessed.

#include "renderer/world/profile.h"

#include "tests/check.h"

using namespace openrac::assets;
using namespace openrac::renderer::world;

int main() {
    const auto pal = world_profile(Game::Rac1, Region::Pal);
    CHECK(pal && pal->frame_height == 448.0f && pal->frame_y_ratio == 0.756f);
    CHECK(pal && pal->constants.tfrag_far_cull == 512000.0f);
    const auto ntsc = world_profile(Game::Rac1, Region::NtscU);
    CHECK(ntsc && ntsc->frame_height == 416.0f && ntsc->frame_y_ratio == 0.775f);
    for (Game g : {Game::Rac2, Game::Rac3, Game::Rac4}) {
        CHECK(!world_profile(g, Region::NtscU));
    }
    const GameVersion* v = find_version("rac1-pal");
    CHECK(v != nullptr && world_profile(*v).has_value());
    return openrac::test::result();
}

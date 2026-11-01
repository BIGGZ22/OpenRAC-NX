// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The order the mobys' colour passes draw in (LevelScene::draw_layer) and which of them write depth.
// The GS draws every moby face with TEST 0x5360b (renderer/world/gs_pass.h): a fragment whose alpha
// reaches 0x60 writes Z, the others colour only. The port draws the solid faces first, then the
// glow faces (MobyProc's glow list, mode bit 0x10) in two draws split at that reference, so that a
// near glow face hides the far ones behind it, then the translucent texels, the blended mobys and
// the metal packets, which write no depth. Drawn in one blended draw without depth, the parked
// ship's front thrusters' glow showed through its rear thrusters' light bars on Pokitaru.

#pragma once

#include <array>
#include <cmath>
#include <string_view>

namespace openrac::scene {

// mesh.vert can emit glow only in its live-lighting branch. Unknown values
// remain eligible so this optimization never culls uncertain shader inputs.
inline bool moby_instance_may_glow(float light_mode, float glow_word) {
    return !std::isfinite(light_mode) || !std::isfinite(glow_word)
           || (std::abs(light_mode)>0.5f && glow_word>0.5f);
}

// Whether a level image is a moby texture, whichever way the level was imported: the GLB export
// names them ".../textures/moby_NNNN.png", the Switch's import straight from the disc
// "native/1/N" (scene/disc_import.cpp: table 1 of decode_level_textures is the mobys', and a moby
// class's materials take their textures from group 1). LevelScene gives the cutout ones one level
// and bilinear filtering, as the GS samples them (the Gadgetron hologram's lettering); that test
// knew only the GLB name, so on the Switch every cutout moby texture kept its mipmap chain.
inline bool moby_texture_path(std::string_view path) {
    return path.find("/textures/moby_") != std::string_view::npos || path.starts_with("native/1/");
}

// The mobys' colour passes, by the number draw_layer and mesh.frag give them.
enum MobyPass : int {
    kMobySolid = 0,
    kMobyGlow = 1,
    kMobyTranslucent = 2,
    kMobyEffects = 3,
    kMobyMetal = 4
};

// One draw of the mobys' colour passes.
struct MobyPassStep {
    int pass = kMobySolid;  // the MobyPass drawn
    int glow_split =
        0;  // mesh.frag's glow_split: 1 the glow texels at or above the reference, 2 the rest
    bool depth_write = false;
};

// The draws in order.
inline constexpr std::array<MobyPassStep, 6> kMobyPassSteps{{
    {kMobySolid, 0, true},
    {kMobyGlow, 1, true},
    {kMobyGlow, 2, false},
    {kMobyTranslucent, 0, false},
    {kMobyEffects, 0, false},
    {kMobyMetal, 0, false},
}};

}  // namespace openrac::scene

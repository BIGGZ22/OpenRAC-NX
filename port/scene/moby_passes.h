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

namespace openrac::scene {

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

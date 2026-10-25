// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests scene::kMobyPassSteps (scene/moby_passes.h): the mobys' colour passes write depth as the
// GS's TEST 0x5360b does, so a near glow face hides the far ones (the parked ship's thrusters).

#include <cstddef>

#include "renderer/world/gs_pass.h"
#include "scene/moby_passes.h"
#include "tests/check.h"

namespace {

using namespace openrac::scene;

// The solid faces write depth; the glow faces at or above the reference write it before the rest
// of the glow and every blended pass, which write none.
void test_glow_writes_depth_before_the_blended_passes() {
    int glow_draws = 0;
    bool depth_written_after_blended = false;
    bool blended_seen = false;
    for (const MobyPassStep& step : kMobyPassSteps) {
        if (step.pass == kMobyGlow) {
            ++glow_draws;
            CHECK(step.depth_write == (step.glow_split == 1));
        } else if (step.pass == kMobySolid) {
            CHECK(step.depth_write);
            CHECK(step.glow_split == 0);
        } else {
            CHECK(!step.depth_write);
        }
        blended_seen |= !step.depth_write;
        depth_written_after_blended |= blended_seen && step.depth_write;
    }
    CHECK(glow_draws == 2);
    CHECK(!depth_written_after_blended);
    CHECK(kMobyPassSteps[0].pass == kMobySolid);
    CHECK(kMobyPassSteps[1].pass == kMobyGlow && kMobyPassSteps[1].glow_split == 1);
}

// The split is the mobys' TEST_1 reference: alpha GEQUAL 0x60 writes Z, the rest colour only.
void test_the_reference_is_the_moby_test() {
    constexpr auto test =
        openrac::renderer::world::Test1::decode(openrac::renderer::world::test1::kWorld);
    CHECK(test.is_z_write_split());
    CHECK(test.aref == 0x60);
}

}  // namespace

int main() {
    test_glow_writes_depth_before_the_blended_passes();
    test_the_reference_is_the_moby_test();
    return openrac::test::result();
}

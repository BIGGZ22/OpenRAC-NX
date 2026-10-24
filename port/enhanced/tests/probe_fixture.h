// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "enhanced/tests/gpu_probe_fixture.h"
#include "scene/level_renderer.h"

namespace openrac::test {
// Shadow/AO kernel fixtures still need a valid level probe resource. Keep
// sampling off for tests comparing authored lighting rather than diffuse GI.
inline bool shadow_probe_fixture(scene::LevelScene& scene, std::string& error) {
    if (scene.probe_data_available()) {
        return true;
    }
    lighting::ProbeGrid grid;
    grid.origin = {-2048, -2048, -2048};
    grid.spacing = {4096, 4096, 4096};
    auto volume = lighting::ProbeVolume::allocate(grid);
    for (auto& p : volume.probes) {
        p.valid = true;
    }
    if (!scene.set_probe_volume(openrac::test::gpu_probes(volume), error)) {
        return false;
    }
    scene.set_probe_view(0);
    return true;
}
}  // namespace openrac::test

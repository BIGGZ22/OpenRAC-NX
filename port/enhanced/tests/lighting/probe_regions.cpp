// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/lighting/probe_regions.h"

#include <cmath>
#include <limits>

#include "renderer/math.h"
#include "tests/check.h"
using namespace openrac::lighting;

int main() {
    ProbeGrid grid;
    grid.spacing = {4, 4, 4};
    grid.dimensions = {5, 5, 5};
    std::vector<ProbeGrid> grids(5, grid);
    for (unsigned i = 0; i < grids.size(); ++i) {
        grids[i].origin[0] = float(i) * 300;
    }
    CHECK(probe_region_distance(grid, {8, 8, 8}) == 0);
    CHECK(probe_region_distance(grid, {48, 8, 8}) == 32);
    CHECK(std::abs(probe_region_distance(grid, {19, 20, 16}) - 5) < 0.0001f);
    ProbeRegionSelector selector;
    selector.update(grids, {8, 8, 8}, 0.1f);
    CHECK(selector.slots[0].region == 0 && selector.slots[0].weight == 0.2f);
    selector.update(grids, {8, 8, 8}, 1);
    CHECK(selector.slots[0].weight == 1);
    // Teleports retire the old resident gradually before recycling its slot.
    selector.update(grids, {908, 8, 8}, 0.1f);
    CHECK(selector.slots[0].region == 0 && selector.slots[0].weight > 0);
    CHECK(selector.slots[1].region == 3);
    selector.update(grids, {908, 8, 8}, 1);
    CHECK(selector.slots[0].region == -1 && selector.slots[1].region == 3);
    selector.update(grids, {4000, 0, 0}, 0.1f);
    CHECK(selector.slots[1].weight > 0 && selector.slots[1].weight < 1);
    selector.update(grids, {4000, 0, 0}, 1);
    CHECK(selector.slots[0].region == -1 && selector.slots[1].region == -1);
    // Three coincident candidates select exactly two with stable IDs.
    grids.assign(3, grid);
    selector.update(grids, {8, 8, 8}, 1);
    CHECK(selector.slots[0].region == 0 && selector.slots[1].region == 1);
    selector.update(grids, {8.01f, 8, 8}, 0.1f);
    CHECK(selector.slots[0].region == 0 && selector.slots[1].region == 1);
    selector.update(grids, {80, 8, 8}, 1);  // 64 units outside the AABB: halfway fade
    CHECK(std::abs(selector.slots[0].weight - 0.5f) < 0.0001f);
    selector.update(grids, {std::numeric_limits<float>::quiet_NaN(), 0, 0}, 1);
    CHECK(selector.slots[0].region == -1 && selector.slots[1].region == -1);

    // Real reversed-depth perspective: partially visible boxes are eligible
    // even when their centres are offscreen; near/far and rear boxes reject.
    const auto projection = openrac::renderer::perspective_reversed(1.570796327f, 1, 1, 1000);
    const auto clip = openrac::renderer::multiply(
        projection, openrac::renderer::look_along({0, 0, 0}, {1, 0, 0}, {0, 0, 1})
    );
    ProbeGrid front = grid, rear = grid, side = grid, edge = grid, far = grid;
    front.origin = {30, -8, -8};
    rear.origin = {-20, -8, -8};
    side.origin = {20, 60, -8};
    edge.origin = {30, 40, -8};
    far.origin = {1200, -8, -8};
    CHECK(probe_region_in_frustum(front, clip));
    CHECK(!probe_region_in_frustum(rear, clip));
    CHECK(!probe_region_in_frustum(side, clip));
    CHECK(probe_region_in_frustum(edge, clip));
    CHECK(!probe_region_in_frustum(far, clip));
    ProbeGrid inside = grid;
    inside.origin = {-8, -8, -8};
    CHECK(probe_region_in_frustum(inside, clip));
    ProbeGrid before_near = grid;
    before_near.origin = {0.1f, -0.1f, -0.1f};
    before_near.spacing = {0.1f, 0.1f, 0.1f};
    CHECK(!probe_region_in_frustum(before_near, clip));
    auto bad_clip = clip;
    bad_clip[0] = std::numeric_limits<float>::quiet_NaN();
    CHECK(probe_region_in_frustum(rear, bad_clip));

    ProbeGrid farther_front = front;
    farther_front.origin[0] = 60;
    grids = {rear, side, front, farther_front};
    selector = {};
    selector.update(grids, {0, 0, 0}, 1);
    CHECK(selector.slots[0].region == 0 && selector.slots[1].region == 2);
    selector.update(grids, {0, 0, 0}, 0.1f, &clip);
    CHECK(selector.slots[0].region == 0 && selector.slots[0].weight > 0);  // Safe retirement.
    CHECK(selector.slots[1].region == 2 && selector.slots[1].weight == 1);
    selector.update(grids, {0, 0, 0}, 1, &clip);
    CHECK(selector.slots[0].region == 3 && selector.slots[1].region == 2);
    CHECK(selector.slots[0].weight > 0 && selector.slots[0].weight < 1);  // Distance fade retained.
    const auto turned = openrac::renderer::multiply(
        projection, openrac::renderer::look_along({0, 0, 0}, {-1, 0, 0}, {0, 0, 1})
    );
    selector.update(grids, {0, 0, 0}, 1, &turned);
    CHECK(selector.slots[0].region == 0);  // Turning in place brings the rear region in.
    CHECK(selector.slots[1].region == 2);  // Offscreen fallback uses the spare slot.
    selector.update(grids, {0, 0, 0}, 0.01f, &turned);
    CHECK(selector.slots[0].region == 0 && selector.slots[1].region == 2);

    ProbeGrid coarse_grid;
    auto coarse = ProbeVolume::allocate(coarse_grid);
    auto a = ProbeVolume::allocate(grid), b = ProbeVolume::allocate(grid);
    for (auto* v : {&coarse, &a, &b}) {
        for (auto& p : v->probes) {
            p.valid = true;
        }
        for (auto& m : v->distance_moments) {
            m = {1024, 1024 * 1024};
        }
    }
    for (auto& e : coarse.irradiance) {
        e = {1, 0, 0};
    }
    for (auto& e : a.irradiance) {
        e = {0, 1, 0};
    }
    for (auto& e : b.irradiance) {
        e = {0, 0, 1};
    }
    std::array<WeightedProbeRegion, 2> regions{{{&a, 0.25f}, {&b, 0.25f}}};
    auto sample = sample_probe_regions(coarse, regions, {8, 8, 8}, {0, 0, 1});
    CHECK(sample && sample->irradiance == (Float3{0.5f, 0.25f, 0.25f}));
    std::swap(regions[0], regions[1]);
    CHECK(
        sample_probe_regions(coarse, regions, {8, 8, 8}, {0, 0, 1})->irradiance
        == sample->irradiance
    );
    regions[0].weight = regions[1].weight = 1;
    CHECK(
        sample_probe_regions(coarse, regions, {8, 8, 8}, {0, 0, 1})->irradiance
        == (Float3{0, 0.5f, 0.5f})
    );
    for (auto& p : b.probes) {
        p.valid = false;
    }
    CHECK(
        sample_probe_regions(coarse, regions, {8, 8, 8}, {0, 0, 1})->irradiance == (Float3{0, 1, 0})
    );
    for (auto& m : a.distance_moments) {
        m = {0.001f, 0.000001f};
    }
    CHECK(sample_probe_regions(coarse, regions, {8, 8, 8}, {0, 0, 1})->irradiance == Float3{});
    regions[0].weight = regions[1].weight = 0;
    CHECK(
        sample_probe_regions(coarse, regions, {8, 8, 8}, {0, 0, 1})->irradiance == (Float3{1, 0, 0})
    );
    return openrac::test::result();
}

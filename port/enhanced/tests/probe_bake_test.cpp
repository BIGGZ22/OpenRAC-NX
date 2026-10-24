// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Grid placement, winding and format validation fixtures.
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

#include "enhanced/tests/probe_bake_fixtures.h"
#include "tests/check.h"

using namespace openrac::lighting;
using namespace openrac::lighting::bake_test;

namespace {

// Verify relocation validation before any worker starts.
void relocation_limits() {
    ProbeGrid grid;
    grid.map_resolution = 2;
    BakeSettings settings;
    settings.rays_per_probe = 16;
    settings.relocation_max_fraction = 0.45f;
    CHECK(!encode_probes(bake_probes(grid, BakeScene{}, settings, {}, 1)).empty());
    settings.relocation_max_fraction = 0.46f;
    bool started = false, rejected = false;

    try {
        bake_probes(grid, BakeScene{}, settings, [&](auto, auto) { started = true; }, 1);
    } catch (const std::runtime_error&) {
        rejected = true;
    }

    CHECK(rejected);
    CHECK(!started);  // Reject unsupported offsets before tracing the scene.
}

// Verify dense visibility ranges suppress the closed side of a doorway.
void doorway_visibility_range() {
    BakeScene scene;
    scene.environment = {1, 1, 1};
    // A wall with open sky just above its edge; the probe-to-receiver segment
    // hits the wall, while the angular moment filter also sees some sky rays.
    scene.triangles =
        {{{{{1, -100, -100}, {1, 100, -100}, {1, 100, 0.5f}}}, {0, 0, 0}},
         {{{{1, -100, -100}, {1, 100, 0.5f}, {1, -100, 0.5f}}}, {0, 0, 0}}};
    ProbeGrid grid;
    grid.spacing = {4, 4, 4};
    BakeSettings settings;
    settings.rays_per_probe = 1024;
    auto distant = bake_probes(grid, scene, settings);
    grid.max_distance = 1.5f * std::sqrt(48.0f);
    auto local = bake_probes(grid, scene, settings);
    CHECK(local.irradiance == distant.irradiance);  // full-scene transport is unchanged

    for (std::size_t i = 1; i < local.probes.size(); ++i) {
        local.probes[i].valid = distant.probes[i].valid = false;
    }

    const auto before = sample_probes(distant, {3, 0, 0}, {0, 0, 1});
    const auto after = sample_probes(local, {3, 0, 0}, {0, 0, 1});
    CHECK(before && after);

    if (before && after) {
        CHECK(before->irradiance[0] > 0.1f);
        // The 2x2 angular footprint softens this wall/sky boundary; retain substantial cap-based suppression.
        CHECK(after->irradiance[0] < before->irradiance[0] * 0.6f);
    }
}

}  // namespace

// Run grid placement and format checks.
int main() {
    relocation_limits();
    doorway_visibility_range();
    {
        BakeScene scene;
        BakeTriangle t{};
        t.diffuse = {1, 1, 1};
        t.multipliers = {{{1, 0, 0, 0}, {0, 1, 0, 1}, {0, 0, 1, 1}}};
        auto c = bake_material(scene, t, .25f, .5f);
        CHECK((c == std::array<float, 4>{.25f, .25f, .5f, .75f}));
        scene.textures.push_back({1, 1, {255, 255, 255, 128}});
        t.texture = 0;
        c = bake_material(scene, t, .25f, .5f);
        CHECK(std::abs(c[3] - .75f * 128 / 255) < .00001f);
        CHECK(c[0] == .25f && c[2] == .5f);
        t.multipliers.fill({2, 2, 2, 2});
        c = bake_material(scene, t, 0, 0);
        CHECK(c[0] == 1 && c[3] == 1);  // GS multipliers can exceed unity
    }
    ProbeGrid grid;
    grid.map_resolution = 4;
    grid.max_distance = 100;
    BakeScene scene;
    scene.environment = {1, 0.5f, 0.25f};
    BakeSettings settings;
    settings.rays_per_probe = 512;
    const auto empty = bake_probes(grid, scene, settings);

    for (auto& probe : empty.probes) {
        CHECK(probe.valid);
    }

    for (auto e : empty.irradiance) {
        CHECK(std::abs(e[0] - std::numbers::pi_v<float>) < 0.02f);
        CHECK(std::abs(e[1] - e[0] * 0.5f) < 0.0001f);
    }

    for (auto m : empty.distance_moments) {
        CHECK(m[0] == 100);
        CHECK(m[1] == 10000);
    }

    CHECK(encode_probes(empty) == encode_probes(bake_probes(grid, scene, settings)));
    // A huge black floor blocks half the environment. A downward-facing
    // receiver above it must receive much less light than in the empty scene.
    scene.triangles =
        {{{{{-1000, -1000, -1}, {1000, -1000, -1}, {1000, 1000, -1}}}, {0, 0, 0}},
         {{{{-1000, -1000, -1}, {1000, 1000, -1}, {-1000, 1000, -1}}}, {0, 0, 0}}};
    const auto blocked = bake_probes(grid, scene, settings);
    const auto sample = sample_probes(blocked, {8, 8, 8}, {0, 0, -1});
    CHECK(sample.has_value());

    if (sample) {
        CHECK(sample->irradiance[0] < 1.0f);
    }

    // A moment's finite storage range must not clip geometry out of the
    // transport scene and incorrectly turn it into an environment miss.
    for (auto& triangle : scene.triangles) {
        for (auto& vertex : triangle.vertices) {
            vertex[2] = -200;
        }
    }

    const auto distant = bake_probes(grid, scene, settings);
    const auto distant_sample = sample_probes(distant, {8, 8, 8}, {0, 0, -1});
    CHECK(distant_sample.has_value());

    if (distant_sample) {
        CHECK(distant_sample->irradiance[0] < 1.0f);
    }

    for (auto& triangle : scene.triangles) {
        for (auto& vertex : triangle.vertices) {
            vertex[2] = -1;
        }
    }

    // No environment: a white floor bounces the analytic sun, but an empty
    // scene receives no sun because direct sunlight is shaded at runtime.
    scene.environment = {0, 0, 0};
    scene.suns.push_back({{0, 0, -1}, {1, 1, 1}});

    for (auto& triangle : scene.triangles) {
        triangle.diffuse = {1, 1, 1};
    }

    const auto bounce = bake_probes(grid, scene, settings);
    const auto indirect = sample_probes(bounce, {8, 8, 8}, {0, 0, -1});
    CHECK(indirect.has_value());

    if (indirect) {
        CHECK(indirect->irradiance[0] > 0.5f);
    }

    // Transport uses a geometric normal facing the incident ray, not source
    // normals or winding. Reverse all or just one floor triangle, preserving
    // its vertex/UV associations. Placement uses the normal production policy.
    auto transport_settings = settings;
    const auto reference = bake_probes(grid, scene, transport_settings);

    for (unsigned mask : {1u, 2u, 3u}) {
        auto reversed = scene;

        for (unsigned i = 0; i < reversed.triangles.size(); ++i) {
            if ((mask & (1u << i)) != 0) {
                std::swap(reversed.triangles[i].vertices[1], reversed.triangles[i].vertices[2]);
                std::swap(reversed.triangles[i].uv[1], reversed.triangles[i].uv[2]);
            }
        }

        const auto actual = bake_probes(grid, reversed, transport_settings);

        for (std::size_t i = 0; i < reference.irradiance.size(); ++i) {
            for (unsigned c = 0; c < 3; ++c) {
                CHECK(std::abs(actual.irradiance[i][c] - reference.irradiance[i][c]) < 0.0001f);
            }

            for (unsigned c = 0; c < 2; ++c) {
                CHECK(
                    std::abs(actual.distance_moments[i][c] - reference.distance_moments[i][c])
                    < 0.001f
                );
            }
        }
    }

    scene.triangles.clear();
    const auto sun_only = bake_probes(grid, scene, settings);

    for (auto e : sun_only.irradiance) {
        CHECK(e[0] == 0 && e[1] == 0 && e[2] == 0);
    }

    // Texture colours are decoded before filtering; alpha stays linear.
    scene.textures.push_back({1, 1, {128, 64, 255, 0}});
    BakeTriangle textured{};
    textured.diffuse = {1, 1, 1};
    textured.texture = 0;
    textured.uv = {Float2{-2.5f, 3.5f}, Float2{-2.5f, 3.5f}, Float2{-2.5f, 3.5f}};
    const auto material = bake_material(scene, textured, 0.2f, 0.3f);
    CHECK(std::abs(material[0] - 0.21586f) < 0.0001f);
    CHECK(std::abs(material[1] - 0.05127f) < 0.0001f);
    CHECK(material[2] == 1 && material[3] == 0);
    // A completely transparent floor must neither block environment light
    // nor appear in the first-hit distance moments.
    scene.environment = {1, 0.5f, 0.25f};
    scene.suns.clear();
    scene.triangles =
        {{{{{-1000, -1000, -1}, {1000, -1000, -1}, {1000, 1000, -1}}}, {1, 1, 1}},
         {{{{-1000, -1000, -1}, {1000, 1000, -1}, {-1000, 1000, -1}}}, {1, 1, 1}}};

    for (auto& triangle : scene.triangles) {
        triangle.texture = 0;
    }

    const auto transparent = bake_probes(grid, scene, settings);
    CHECK(transparent.irradiance == empty.irradiance);
    CHECK(transparent.distance_moments == empty.distance_moments);
    // Transparent closed geometry must not classify empty space as solid.
    auto transparent_box = scene;
    transparent_box.triangles = box({-10, -10, -10}, {10, 10, 10});

    for (auto& triangle : transparent_box.triangles) {
        triangle.texture = 0;
    }

    const auto transparent_occupancy = bake_probes(grid, transparent_box, settings);
    equivalent(empty, transparent_occupancy);
    // Seeing the back of a small open sheet does not put an otherwise open
    // sky probe inside geometry. The fraction includes environment misses.
    scene.triangles = {{{{{-4, -4, 20}, {4, -4, 20}, {0, 4, 20}}}, {0, 0, 0}}};
    const auto open_sheet = bake_probes(grid, scene, settings);

    for (const auto& probe : open_sheet.probes) {
        CHECK(probe.valid);
    }

    // An open floor cannot establish solid occupancy, from either side.
    scene.triangles =
        {{{{{-1000, -1000, 1}, {1000, -1000, 1}, {1000, 1000, 1}}}, {0, 0, 0}},
         {{{{-1000, -1000, 1}, {1000, 1000, 1}, {-1000, 1000, 1}}}, {0, 0, 0}}};
    const auto open_floor = bake_probes(grid, scene, settings);

    for (const auto& probe : open_floor.probes) {
        CHECK(probe.valid);
        CHECK((probe.offset == Float3{}));
    }

    winding_invariant(grid, scene, settings);
    // A nominal probe inside a closed slab can move through its nearest
    // surface, but not beyond its cell's relocation radius. Bake moments
    // at the relocated position, rather than retaining underground hits.
    scene.triangles = box({-1000, -1000, -100}, {1000, 1000, 1});
    settings.relocation_iterations = 0;
    const auto underground = bake_probes(grid, scene, settings);
    CHECK(!underground.probes[0].valid);
    winding_invariant(grid, scene, settings);
    settings.relocation_iterations = 4;
    const auto relocated = bake_probes(grid, scene, settings);
    CHECK(relocated.probes[0].valid);
    CHECK(relocated.position(0)[2] > 1);
    CHECK(relocated.position(0)[2] < grid.spacing[2] * settings.relocation_max_fraction);
    CHECK(encode_probes(relocated) == encode_probes(bake_probes(grid, scene, settings)));
    winding_invariant(grid, scene, settings);
    scene.triangles = box({-1000, -1000, -100}, {1000, 1000, grid.spacing[2]});
    const auto unreachable = bake_probes(grid, scene, settings);
    CHECK(!unreachable.probes[0].valid);
    CHECK((unreachable.probes[0].offset == Float3{}));
    winding_invariant(grid, scene, settings);
    // Nested shells represent wall thickness: the central cavity is air,
    // while the layer between the inner and outer shells is solid.
    scene.triangles = box({-10, -10, -10}, {10, 10, 10});
    const auto inner = box({-5, -5, -5}, {5, 5, 5});
    scene.triangles.insert(scene.triangles.end(), inner.begin(), inner.end());
    settings.relocation_iterations = 0;
    CHECK(bake_probes(grid, scene, settings).probes[0].valid);
    winding_invariant(grid, scene, settings);
    auto wall_grid = grid;
    wall_grid.origin = {6, 0, 0};
    CHECK(!bake_probes(wall_grid, scene, settings).probes[0].valid);
    winding_invariant(wall_grid, scene, settings);
    return openrac::test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Cutout, sunlight and emitter occlusion regression fixtures.
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

#include "enhanced/tests/probe_bake_fixtures.h"

using namespace openrac::lighting;
using namespace openrac::lighting::bake_test;

namespace {
// The first GI hit supplies moments, but its distance cap must never clip transport.
void test_capped_first_hit_transport() {
    ProbeGrid grid;
    grid.spacing = {1, 1, 1};
    grid.max_distance = 1.5f * std::sqrt(3.0f);  // relocated-cell visibility range
    BakeSettings settings;
    settings.rays_per_probe = 128;
    settings.relocation_iterations = 0;
    BakeScene scene;
    scene.environment = {1, .5f, .25f};

    for (const unsigned bounces : {0u, 2u}) {
        settings.bounces = bounces;
        scene.triangles.clear();
        const auto empty = bake_probes(grid, scene, settings, {}, 1);
        scene.triangles = {
            {{{{-10000, -10000, -20}, {10000, -10000, -20}, {10000, 10000, -20}}}, {.5f, .5f, .5f}},
            {{{{-10000, -10000, -20}, {10000, 10000, -20}, {-10000, 10000, -20}}}, {.5f, .5f, .5f}}
        };
        const auto distant = bake_probes(grid, scene, settings, {}, 1);
        CHECK(distant.distance_moments == empty.distance_moments);
        CHECK(distant.irradiance != empty.irradiance);
        // A closer alpha hole must not replace the accepted first hit.
        const auto floor = scene.triangles;
        scene.textures = {{1, 1, {255, 255, 255, 0}}};

        for (auto triangle : floor) {
            triangle.texture = 0;

            for (auto& vertex : triangle.vertices) {
                vertex[2] = -1;
            }

            scene.triangles.push_back(triangle);
        }

        const auto transparent = bake_probes(grid, scene, settings, {}, 1);
        CHECK(transparent.irradiance == distant.irradiance);
        CHECK(transparent.distance_moments == distant.distance_moments);
        // A near opaque hit must retain its distance through later GI bounces.
        scene.triangles = floor;

        for (auto& triangle : scene.triangles) {
            for (auto& vertex : triangle.vertices) {
                vertex[2] = -1;
            }
        }

        const auto near = bake_probes(grid, scene, settings, {}, 1);
        CHECK(near.distance_moments != empty.distance_moments);

        for (const auto& moments : near.distance_moments) {
            CHECK(moments[0] <= grid.max_distance);
            CHECK(moments[1] <= grid.max_distance * grid.max_distance);
        }
    }
}

// Opaque caching and alpha-only queries must preserve blockers and transparent surfaces.
void test_alpha_visibility() {
    ProbeGrid grid;
    grid.spacing = {1, 1, 1};
    BakeSettings settings;
    settings.rays_per_probe = 128;
    settings.bounces = 0;  // Isolate visibility from the surface's RGB response.
    BakeScene scene;
    scene.triangles = box({-10, -10, 2}, {10, 10, 3});
    const auto blocked = encode_probes(bake_probes(grid, scene, settings));
    BakeScene empty;
    const auto clear = encode_probes(bake_probes(grid, empty, settings));
    CHECK(blocked != clear);
    scene.textures.push_back({2, 1, {255, 0, 128, 255, 0, 128, 255, 255}});

    for (auto& triangle : scene.triangles) {
        triangle.texture = 0;
        triangle.uv = {{{-1.25f, 2.75f}, {3.5f, -.25f}, {.125f, 1.5f}}};
    }

    CHECK(encode_probes(bake_probes(grid, scene, settings)) == blocked);

    // Unchanged texel alpha is not sufficient when authored vertex alpha fades the surface.
    for (auto& triangle : scene.triangles) {
        triangle.multipliers[0][3] = .1f;
        triangle.multipliers[1][3] = .3f;
        triangle.multipliers[2][3] = .49f;
    }

    CHECK(encode_probes(bake_probes(grid, scene, settings)) == clear);

    for (auto& triangle : scene.triangles) {
        triangle.multipliers[0][3] = 1;
        triangle.multipliers[1][3] = 1;
        triangle.multipliers[2][3] = 1;
    }

    // Adjacent 8-bit alpha values lie on opposite sides of the half-alpha cutoff.
    for (unsigned alpha : {127u, 128u}) {
        scene.textures[0].rgba[3] = scene.textures[0].rgba[7] = static_cast<std::uint8_t>(alpha);
        const auto actual = encode_probes(bake_probes(grid, scene, settings));
        CHECK(actual == (alpha == 127 ? clear : blocked));
    }

    // Wrapped UVs must address the correct texel inside the traversal callback.
    scene.textures[0].rgba[3] = 0;
    scene.textures[0].rgba[7] = 255;

    for (const float u : {-.75f, 1.75f}) {
        for (auto& triangle : scene.triangles) {
            triangle.uv = {{{u, -.5f}, {u, -.5f}, {u, -.5f}}};
        }

        CHECK(encode_probes(bake_probes(grid, scene, settings)) == (u < 0 ? clear : blocked));
    }
}

// Only guaranteed zero-alpha surfaces may be omitted from acceleration.
void test_fully_transparent_geometry() {
    ProbeGrid grid;
    BakeSettings settings;
    settings.rays_per_probe = 128;
    settings.bounces = 0;
    const auto bake = [&](const BakeScene& scene) {
        return encode_probes(bake_probes(grid, scene, settings));
    };
    BakeScene scene;
    scene.triangles = box({-10, -10, 2}, {10, 10, 3});
    const auto blocked = bake(scene);
    const auto clear = bake(BakeScene{});
    CHECK(blocked != clear);
    scene.textures.push_back({1, 1, {255, 255, 255, 0}});

    for (auto& triangle : scene.triangles) {
        triangle.texture = 0;
    }

    CHECK(bake(scene) == clear);

    for (auto& triangle : scene.triangles) {
        triangle.alpha_cutoff = 0;  // Alpha zero passes a zero cutoff.
    }

    CHECK(bake(scene) == blocked);
    scene.textures[0].rgba[3] = 255;

    for (auto& triangle : scene.triangles) {
        triangle.alpha_cutoff = .5f;

        for (auto& colour : triangle.multipliers) {
            colour[3] = 0;
        }
    }

    CHECK(bake(scene) == clear);

    for (auto& triangle : scene.triangles) {
        triangle.texture = -1;  // Untextured geometry is accepted independently of alpha.
    }

    CHECK(bake(scene) == blocked);
}

// Multiple rejected surfaces retain the nearest accepted hit and capped moments.
void test_layered_alpha_visibility() {
    ProbeGrid grid;
    grid.spacing = {1, 1, 1};
    BakeSettings settings;
    settings.rays_per_probe = 128;
    settings.bounces = 0;
    BakeScene layered;
    layered.triangles = box({-10, -10, 2}, {10, 10, 3});
    const auto blocked = encode_probes(bake_probes(grid, layered, settings));
    const auto clear = encode_probes(bake_probes(grid, BakeScene{}, settings));
    layered.textures.push_back({1, 1, {255, 255, 255, 0}});

    for (unsigned layer = 0; layer < 6; ++layer) {
        const float z = .2f + static_cast<float>(layer) * .25f;

        for (auto triangle : box({-10, -10, z}, {10, 10, z + .1f})) {
            triangle.texture = 0;

            if (layer % 2 == 0) {
                std::swap(triangle.vertices[1], triangle.vertices[2]);
            }

            layered.triangles.push_back(triangle);
        }
    }

    CHECK(encode_probes(bake_probes(grid, layered, settings)) == blocked);

    // With no accepted blocker, filtering must restore the unbounded escape.
    for (auto& triangle : layered.triangles) {
        triangle.texture = 0;
    }

    CHECK(encode_probes(bake_probes(grid, layered, settings)) == clear);
}

// Sun visibility remains unbounded and uses the same alpha cutoff as transport.
void test_sun_visibility() {
    ProbeGrid grid;
    grid.spacing = {.1f, .1f, .1f};  // Keep all receivers below the roof.
    grid.origin = {0, 0, 2};
    grid.max_distance = 1;  // The storage cap must not clip the sun query.
    BakeSettings settings;
    settings.rays_per_probe = 128;
    settings.bounces = 1;
    BakeScene scene;
    scene.environment = {};
    scene.suns = {{{0, 0, -1}, {1, .5f, .25f}, true}};
    scene.triangles = box({-100, -100, -1}, {100, 100, 0});

    for (auto& floor : scene.triangles) {
        floor.diffuse = {.5f, .5f, .5f};
    }

    const auto lit = bake_probes(grid, scene, settings);
    CHECK(std::any_of(lit.irradiance.begin(), lit.irradiance.end(), [](auto c) {
        return c[0] > 0;
    }));
    scene.textures.push_back({1, 1, {255, 255, 255, 127}});

    for (auto roof : box({-200, -200, 4}, {200, 200, 5})) {
        roof.diffuse = {};
        roof.texture = 0;
        scene.triangles.push_back(roof);
    }

    CHECK(bake_probes(grid, scene, settings).irradiance == lit.irradiance);
    scene.textures[0].rgba[3] = 128;
    const auto blocked = bake_probes(grid, scene, settings);
    CHECK(std::all_of(blocked.irradiance.begin(), blocked.irradiance.end(), [](auto c) {
        return c == Float3{};
    }));
}

// Emitter visibility ignores geometry beyond the light and filters near blockers.
void test_emitter_visibility(bool area) {
    ProbeGrid grid;
    grid.spacing = {.1f, .1f, .1f};  // Keep all receivers below the near blocker.
    BakeSettings settings;
    settings.rays_per_probe = 128;
    settings.bounces = 0;
    BakeScene scene;
    scene.environment = {};

    if (area) {
        // A small surface exercises sampled endpoints instead of a point light.
        BakeTriangle lamp{};
        lamp.vertices = {Float3{-.1f, -.1f, 2}, Float3{.1f, -.1f, 2}, Float3{0, .1f, 2}};
        lamp.emission = {1, .5f, .25f};
        scene.triangles.push_back(lamp);
    } else {
        scene.point_emitters.push_back({{0, 0, 2}, {1, .5f, .25f}});
    }

    const auto lit = bake_probes(grid, scene, settings);
    CHECK(std::any_of(lit.irradiance.begin(), lit.irradiance.end(), [](auto c) {
        return c[0] > 0;
    }));
    BakeTriangle blocker{};
    blocker.vertices = {Float3{-10, -10, 3}, Float3{10, -10, 3}, Float3{0, 10, 3}};
    scene.triangles.push_back(blocker);
    CHECK(bake_probes(grid, scene, settings).irradiance == lit.irradiance);
    scene.textures.push_back({1, 1, {255, 255, 255, 127}});
    auto& surface = scene.triangles.back();
    surface.texture = 0;

    for (auto& p : surface.vertices) {
        p[2] = 1;  // Move the blocker between the probe and light.
    }

    CHECK(bake_probes(grid, scene, settings).irradiance == lit.irradiance);
    std::swap(surface.vertices[1], surface.vertices[2]);
    CHECK(bake_probes(grid, scene, settings).irradiance == lit.irradiance);
    scene.textures[0].rgba[3] = 128;
    const auto blocked = bake_probes(grid, scene, settings);
    CHECK(std::all_of(blocked.irradiance.begin(), blocked.irradiance.end(), [](auto c) {
        return c == Float3{};
    }));
}

}  // namespace

// Run the independent visibility fixtures.
int main() {
    test_capped_first_hit_transport();
    test_alpha_visibility();
    test_fully_transparent_geometry();
    test_layered_alpha_visibility();
    test_sun_visibility();
    test_emitter_visibility(false);
    test_emitter_visibility(true);
    return openrac::test::result();
}

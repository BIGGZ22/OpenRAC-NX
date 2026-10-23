// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Authored albedo, sky and emission transport regression fixtures.
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

#include "enhanced/tests/probe_bake_fixtures.h"

using namespace openrac::lighting;
using namespace openrac::lighting::bake_test;

namespace {
// Verify authored terrain albedo.
void authored_terrain_albedo() {
    BakeScene scene;
    scene.textures.push_back({2, 1, {51, 102, 153, 255, 204, 153, 102, 255}});
    BakeTriangle triangle;
    triangle.texture = 0;
    triangle.uv = {{{.5f, .5f}, {.5f, .5f}, {.5f, .5f}}};
    triangle.diffuse = {.8f, 1, .5f};
    triangle.terrain_prelighting = true;
    triangle.authored_colours = {{{0, 1, .5f}, {1, 2, 1.5f}, {1, 2, 1.5f}}};
    const auto sampled = bake_material(scene, triangle, .25f, .25f, {1, 1, 1}, true);
    const auto decode = [](float x) {
        return std::pow((x + .055f) / 1.055f, 2.4f);
    };
    const Float3 expected{decode(.2f), decode(.75f), decode(.25f)};

    for (unsigned c = 0; c < 3; ++c) {
        CHECK(std::abs(sampled[c] - expected[c]) < 1e-6f);
    }

    triangle.diffuse_scale = .25f;
    const auto reduced = bake_material(scene, triangle, .25f, .25f, {1, 1, 1}, true);

    for (unsigned c = 0; c < 3; ++c) {
        CHECK(std::abs(reduced[c] - expected[c] * .25f) < 1e-6f);
    }

    triangle.emission = {1, .5f, .25f};
    triangle.emission_texture = 0;
    const auto emission = bake_emission(scene, triangle, .25f, .25f);
    triangle.authored_colours = {};
    CHECK(bake_emission(scene, triangle, .25f, .25f) == emission);
}

// Verify sunlight bounce correction.
void sunlight_bounce_correction() {
    BakeScene scene;
    scene.environment = {};
    scene.triangles =
        {{{{{-100, -100, 0}, {100, -100, 0}, {100, 100, 0}}}, {.5f, .5f, .5f}},
         {{{{-100, -100, 0}, {100, 100, 0}, {-100, 100, 0}}}, {.5f, .5f, .5f}}};
    scene.suns = {{{0, 0, -1}, {1, 2, 3}, true}, {{0, 0, -1}, {.3f, .2f, .1f}, false}};
    scene.point_emitters.push_back({{0, 0, 4}, {.4f, .5f, .6f}});
    ProbeGrid grid;
    grid.origin = {0, 0, 2};
    grid.spacing = {1, 1, 1};
    BakeSettings settings;
    settings.rays_per_probe = 128;
    settings.bounces = 1;
    const auto base = bake_probes(grid, scene, settings);
    settings.sunlight_scale = {0, 0, 0};
    const auto off = bake_probes(grid, scene, settings);
    settings.sunlight_scale = {.5f, .25f, .75f};
    const auto corrected = bake_probes(grid, scene, settings);
    scene.suns.erase(scene.suns.begin());
    const auto no_primary = bake_probes(grid, scene, settings);
    CHECK(off.irradiance == no_primary.irradiance);
    CHECK(base.distance_moments == corrected.distance_moments);
    bool sun_bounced = false;

    for (std::size_t i = 0; i < base.irradiance.size(); ++i) {
        for (unsigned c = 0; c < 3; ++c) {
            const float difference = base.irradiance[i][c] - off.irradiance[i][c];
            sun_bounced |= difference > .001f;
            CHECK(
                std::abs(
                    corrected.irradiance[i][c] - off.irradiance[i][c]
                    - difference * settings.sunlight_scale[c]
                )
                < 1e-5f
            );
        }
    }

    CHECK(sun_bounced);
}

// Verify directional sky.
void directional_sky() {
    BakeScene scene;
    BakeSettings settings;
    settings.sky_weight = 0.3f;  // Explicit primary-dominant mix for this fixture.
    settings.rays_per_probe = 512;
    scene.sky_width = 4;
    scene.sky_height = 2;
    scene.sky_radiance =
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {0, 0, 2}, {0, 0, 2}, {0, 0, 2}, {0, 0, 2}};
    const float z = std::sqrt(.5f);
    const Float3 centres[] = {{.5f, .5f, z}, {.5f, -.5f, z}, {-.5f, -.5f, z}, {-.5f, .5f, z}};

    for (unsigned i = 0; i < 4; ++i) {
        const auto value = sample_bake_environment(scene, centres[i], settings);

        for (unsigned c = 0; c < 3; ++c) {
            CHECK(std::abs(value[c] - 0.3f * scene.sky_radiance[i][c]) < .00001f);
        }
    }

    auto pole = sample_bake_environment(scene, {0, 0, -1}, settings);
    CHECK(std::abs(pole[2] - 0.6f) < .00001f);
    auto a = sample_bake_environment(scene, {.000001f, 1, .5f}, settings);
    auto b = sample_bake_environment(scene, {-.000001f, 1, .5f}, settings);

    for (unsigned c = 0; c < 3; ++c) {
        CHECK(std::abs(a[c] - b[c]) < .00001f);
    }

    settings.sky_intensity = 2;
    CHECK(std::abs(sample_bake_environment(scene, centres[0], settings)[0] - 0.6f) < .00001f);
    scene.primary_directional_colour = {0.2f, 0.4f, 0.6f};
    const auto mixed = sample_bake_environment(scene, centres[0], settings);
    CHECK(std::abs(mixed[0] - 0.74f) < .00001f);
    CHECK(std::abs(mixed[1] - 0.28f) < .00001f);
    CHECK(std::abs(mixed[2] - 0.42f) < .00001f);
    // Primary tint changes the environment, never the analytic sun source.
    scene.suns.push_back({{0, 0, -1}, {9, 8, 7}});
    CHECK(sample_bake_environment(scene, centres[0], settings) == mixed);
    scene.suns.clear();
    settings.sky_intensity = 1;
    settings.sky_weight = 0.7f;
    const auto sky_dominant = sample_bake_environment(scene, centres[0], settings);
    CHECK(std::abs(sky_dominant[0] - 0.76f) < .00001f);
    CHECK(std::abs(sky_dominant[1] - 0.12f) < .00001f);
    CHECK(std::abs(sky_dominant[2] - 0.18f) < .00001f);
    settings.sky_weight = 0.3f;
    settings.use_level_sky = false;
    CHECK(sample_bake_environment(scene, {1, 0, 0}, settings) == scene.environment);
    settings.use_level_sky = true;
    settings.sky_intensity = 1;
    scene.primary_directional_colour = {};

    for (unsigned i = 0; i < 8; ++i) {
        scene.sky_radiance[i] = i < 4 ? Float3{0, 0, 1} : Float3{1, 0, 0};
    }

    ProbeGrid grid;
    const auto volume = bake_probes(grid, scene, settings);
    auto up = sample_probes(volume, {8, 8, 8}, {0, 0, 1});
    auto down = sample_probes(volume, {8, 8, 8}, {0, 0, -1});
    CHECK(up && down);

    if (up && down) {
        CHECK(up->irradiance[2] > 2 * up->irradiance[0]);
        CHECK(down->irradiance[0] > 2 * down->irradiance[2]);
    }

    for (auto& pixel : scene.sky_radiance) {
        pixel = {};
    }

    scene.suns.push_back({{0, 0, -1}, {10, 10, 10}});
    const auto sun_only = bake_probes(grid, scene, settings);

    for (const auto& e : sun_only.irradiance) {
        CHECK((e == Float3{}));
    }

    scene.sky_radiance.pop_back();
    bool rejected = false;

    try {
        bake_probes(grid, scene, settings);
    } catch (const std::runtime_error&) {
        rejected = true;
    }

    CHECK(rejected);
}

// Verify point emission.
void point_emission() {
    ProbeGrid grid;
    grid.map_resolution = 4;
    BakeSettings settings;
    settings.rays_per_probe = 64;
    settings.bounces = 0;
    BakeScene scene;
    scene.environment = {};
    const auto dark = bake_probes(grid, scene, settings);
    scene.point_emitters.push_back({{0, 0, 2}, {2, 1, .2f}});
    const auto lit = bake_probes(grid, scene, settings);
    const unsigned at = 2 * 6 + 2;

    for (unsigned y = 0; y < grid.map_resolution; ++y) {
        for (unsigned x = 0; x < grid.map_resolution; ++x) {
            const auto texel = (y + 1) * (grid.map_resolution + 2) + x + 1;
            const auto cosine = texel_up_cosine(x, y, grid.map_resolution);

            for (unsigned c = 0; c < 3; ++c) {
                const float expected = scene.point_emitters[0].intensity[c] * cosine / 4;
                CHECK(std::abs(lit.irradiance[texel][c] - expected) < .00001f);
            }
        }
    }

    const auto centre = oct_decode({1.5f / 4, 1.5f / 4});
    CHECK(std::abs(lit.irradiance[at][0] - 2 * centre[2] / 4) > .001f);
    CHECK(lit.distance_moments == dark.distance_moments);
    BakeTriangle blocker{};
    blocker.vertices = {Float3{-4, -4, 1}, Float3{4, -4, 1}, Float3{0, 4, 1}};
    blocker.diffuse = {};
    scene.triangles.push_back(blocker);
    const auto blocked = bake_probes(grid, scene, settings);
    CHECK(blocked.irradiance[at][0] == 0);
    scene.triangles[0]
        .vertices = {Float3{-100, -100, -1}, Float3{100, -100, -1}, Float3{0, 100, -1}};
    scene.triangles[0].diffuse = {1, 1, 1};
    settings.rays_per_probe = 512;
    const auto no_bounce = bake_probes(grid, scene, settings);
    settings.bounces = 1;
    const auto bounced = bake_probes(grid, scene, settings);
    CHECK(no_bounce.irradiance[7][0] == 0);  // downward texel cannot see the point directly
    CHECK(bounced.irradiance[7][0] > 0);     // point -> white floor -> probe
    CHECK(bounced.distance_moments == no_bounce.distance_moments);
}

// Verify emissive transport.
void emissive_transport() {
    ProbeGrid grid;
    grid.map_resolution = 4;
    BakeSettings settings;
    settings.rays_per_probe = 2048;
    settings.bounces = 0;
    BakeScene scene;
    scene.environment = {};
    BakeTriangle lamp{};
    lamp.vertices = {Float3{-.05f, -.05f, 2}, Float3{.05f, -.05f, 2}, Float3{0, .05f, 2}};
    lamp.diffuse = {};
    lamp.emission = {0, 2, 0};
    scene.triangles.push_back(lamp);
    const auto direct = bake_probes(grid, scene, settings);
    const unsigned x = 1, y = 1, side = 6;
    const auto at = (y + 1) * side + x + 1;
    // Small far triangle: E approaches Le * area * cos(receiver) / distanceÃ‚Â².
    const float expected = 2 * .005f * texel_up_cosine(x, y, grid.map_resolution) / 4;
    CHECK(std::abs(direct.irradiance[at][1] / expected - 1) < .03f);
    CHECK(direct.irradiance[at][0] == 0 && direct.irradiance[at][2] == 0);
    settings.bounces = 3;
    const auto deeper = bake_probes(grid, scene, settings);
    CHECK(deeper.irradiance == direct.irradiance);  // no emission double count from random hits
    winding_invariant(grid, scene, settings);
    scene.textures.push_back({1, 1, {128, 128, 128, 255}});
    scene.triangles[0].emission_texture = 0;
    const auto textured = bake_probes(grid, scene, settings);
    CHECK(std::abs(textured.irradiance[at][1] / direct.irradiance[at][1] - .21586f) < .0001f);
    scene.triangles[0].emission_texture = -1;
    {
        // The runtime's masked window fixture uses exactly this colour and mask.
        auto window = scene.triangles[0];
        window.emission = {.25f, .125f, .0625f};
        window.emission_texture = 0;
        window.emission_mask = EmissionMask::Green;
        scene.textures[0].rgba = {0, 255, 0, 255};
        CHECK((bake_emission(scene, window, 0, 0) == Float3{0, .125f, 0}));
        scene.textures[0].rgba = {128, 128, 128, 255};
        CHECK((bake_emission(scene, window, 0, 0) == Float3{}));
        CHECK(emission_mask(EmissionMask::Green, {.2f, .4f, .2f}) == 1);
        CHECK(emission_mask(EmissionMask::Green, {.3f, .4f, .3f}) < .00001f);
        window.emission = {1, 1, 1};
        window.emission_mask = EmissionMask::None;
        window.emission_vertex_colour = true;
        window.emission_colours.fill({.25f, 1, .5f});
        scene.textures[0].rgba = {255, 128, 0, 255};
        const auto ooze = bake_emission(scene, window, 0, 0);
        CHECK(std::abs(ooze[0] - .050876f) < .00001f);
        CHECK(std::abs(ooze[1] - .21586f) < .00001f);
        CHECK(ooze[2] == 0 && ooze[1] > ooze[0]);
        scene.textures[0].rgba = {128, 128, 128, 255};
    }
    BakeTriangle blocker{};
    blocker.vertices = {Float3{-4, -4, 1}, Float3{4, -4, 1}, Float3{0, 4, 1}};
    blocker.diffuse = {};
    scene.triangles.push_back(blocker);
    const auto blocked = bake_probes(grid, scene, settings);
    CHECK(blocked.irradiance[at][1] == 0);
    scene.textures[0].rgba[3] = 0;
    scene.triangles[1].texture = 0;
    const auto cutout = bake_probes(grid, scene, settings);
    CHECK(cutout.irradiance[at] == direct.irradiance[at]);
    scene.triangles.pop_back();
    scene.triangles[0].texture = 0;
    const auto invisible_emitter = bake_probes(grid, scene, settings);
    CHECK(invisible_emitter.irradiance[at][1] == 0);
    scene.triangles[0].texture = -1;
    blocker.vertices = {Float3{-4, -4, -1}, Float3{4, -4, -1}, Float3{0, 4, -1}};
    blocker.diffuse = {1, 1, 1};
    scene.triangles.push_back(blocker);
    settings.bounces = 1;
    const auto bounce = bake_probes(grid, scene, settings);
    // The lower hemisphere cannot see the lamp, but sees its diffuse reflection.
    const auto down = 1 * side + 1;
    CHECK(oct_decode({.125f, .125f})[2] < 0);
    CHECK(bounce.irradiance[down][1] > 0);
    CHECK(bounce.irradiance[down][0] == 0);
    scene.triangles[0].emission[1] = -1;
    bool rejected = false;

    try {
        bake_probes(grid, scene, settings);
    } catch (const std::runtime_error&) {
        rejected = true;
    }

    CHECK(rejected);
}

}  // namespace

// Run the independent lighting fixtures.
int main() {
    authored_terrain_albedo();
    sunlight_bounce_correction();
    directional_sky();
    point_emission();
    emissive_transport();
    return openrac::test::result();
}

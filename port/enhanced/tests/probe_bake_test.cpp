// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>
#include <numbers>
#include <stdexcept>

#include "enhanced/lighting/bake.h"
#include "tests/check.h"

using namespace openrac::lighting;

namespace {
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
        CHECK(after->irradiance[0] < before->irradiance[0] * 0.5f);
    }
}

std::vector<BakeTriangle> box(Float3 low, Float3 high) {
    const Float3 vertices[] =
        {{low[0], low[1], low[2]},
         {high[0], low[1], low[2]},
         {high[0], high[1], low[2]},
         {low[0], high[1], low[2]},
         {low[0], low[1], high[2]},
         {high[0], low[1], high[2]},
         {high[0], high[1], high[2]},
         {low[0], high[1], high[2]}};
    const unsigned faces[][3] =
        {{0, 2, 1},
         {0, 3, 2},
         {4, 5, 6},
         {4, 6, 7},
         {0, 1, 5},
         {0, 5, 4},
         {1, 2, 6},
         {1, 6, 5},
         {2, 3, 7},
         {2, 7, 6},
         {3, 0, 4},
         {3, 4, 7}};
    std::vector<BakeTriangle> triangles;
    for (const auto& face : faces) {
        triangles.push_back({{vertices[face[0]], vertices[face[1]], vertices[face[2]]}, {0, 0, 0}});
    }
    return triangles;
}

void equivalent(const ProbeVolume& a, const ProbeVolume& b) {
    CHECK(a.probes.size() == b.probes.size());
    for (std::size_t i = 0; i < a.probes.size(); ++i) {
        CHECK(a.probes[i].valid == b.probes[i].valid);
        for (unsigned c = 0; c < 3; ++c) {
            CHECK(std::abs(a.probes[i].offset[c] - b.probes[i].offset[c]) < 0.0001f);
        }
    }
    for (std::size_t i = 0; i < a.irradiance.size(); ++i) {
        for (unsigned c = 0; c < 3; ++c) {
            CHECK(std::abs(a.irradiance[i][c] - b.irradiance[i][c]) < 0.0001f);
        }
        for (unsigned c = 0; c < 2; ++c) {
            CHECK(std::abs(a.distance_moments[i][c] - b.distance_moments[i][c]) < 0.001f);
        }
    }
}

void winding_invariant(
    const ProbeGrid& grid, const BakeScene& scene, const BakeSettings& settings
) {
    const auto reference = bake_probes(grid, scene, settings);
    for (unsigned pattern : {0u, 1u, 2u}) {
        auto changed = scene;
        for (std::size_t i = 0; i < changed.triangles.size(); ++i) {
            if (pattern == 2 || i % 2 == pattern) {
                std::swap(changed.triangles[i].vertices[1], changed.triangles[i].vertices[2]);
                std::swap(changed.triangles[i].uv[1], changed.triangles[i].uv[2]);
            }
        }
        equivalent(reference, bake_probes(grid, changed, settings));
    }
}
}  // namespace

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
    const auto normal = oct_decode({1.5f / 4, 1.5f / 4});
    CHECK(std::abs(lit.irradiance[at][0] - 2 * normal[2] / 4) < .00001f);
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
    const auto normal = oct_decode({(x + .5f) / 4, (y + .5f) / 4});
    // Small far triangle: E approaches Le * area * cos(receiver) / distance².
    const float expected = 2 * .005f * normal[2] / 4;
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

int main() {
    relocation_limits();
    sunlight_bounce_correction();
    authored_terrain_albedo();
    doorway_visibility_range();
    emissive_transport();
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
    directional_sky();
    point_emission();
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

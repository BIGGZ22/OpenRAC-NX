// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

#include "enhanced/lighting/bake.h"
#include "tests/check.h"

using namespace openrac::lighting;

namespace {
// Analytic receiving cosine for an overhead point light across a fixed 2x2 texel footprint.
float texel_up_cosine(unsigned x, unsigned y, unsigned resolution) {
    float cosine = 0;
    for (const float v : {.25f, .75f}) {
        for (const float u : {.25f, .75f}) {
            const auto direction = oct_decode(
                {(static_cast<float>(x) + u) / static_cast<float>(resolution),
                 (static_cast<float>(y) + v) / static_cast<float>(resolution)}
            );
            cosine += std::max(0.0f, direction[2]) * .25f;
        }
    }
    return cosine;
}

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
        // The 2x2 angular footprint softens this wall/sky boundary; retain substantial cap-based suppression.
        CHECK(after->irradiance[0] < before->irradiance[0] * 0.6f);
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
    CHECK(std::any_of(lit.irradiance.begin(), lit.irradiance.end(), [](auto c) { return c[0] > 0; })
    );
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
    CHECK(std::any_of(lit.irradiance.begin(), lit.irradiance.end(), [](auto c) { return c[0] > 0; })
    );
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
    // Small far triangle: E approaches Le * area * cos(receiver) / distance².
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

int main() {
    test_capped_first_hit_transport();
    test_alpha_visibility();
    test_fully_transparent_geometry();
    test_layered_alpha_visibility();
    test_sun_visibility();
    test_emitter_visibility(false);
    test_emitter_visibility(true);
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

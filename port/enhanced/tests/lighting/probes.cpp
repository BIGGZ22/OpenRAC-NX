// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/lighting/probes.h"

#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

#include "tests/check.h"

using namespace openrac::lighting;

namespace {
bool near(float a, float b) {
    return std::abs(a - b) < 0.0001f;
}

template <class F>
void rejects(F operation) {
    bool rejected = false;
    try {
        operation();
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    CHECK(rejected);
}

ProbeVolume constant() {
    ProbeGrid grid;
    CHECK(grid.map_resolution == 6);
    CHECK(grid.texels_per_probe() == 64);
    grid.scene_hash = 123;
    grid.settings_hash = 456;
    grid.level = 1;
    auto v = ProbeVolume::allocate(grid);
    for (auto& p : v.probes) {
        p.valid = true;
    }
    // Unit radiance -> pi irradiance -> unit Lambertian illumination.
    for (auto& e : v.irradiance) {
        e = {std::numbers::pi_v<float>, 2, 0.5f};
    }
    return v;
}

void coordinates() {
    const Float3 directions[] =
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, -1}, {-1, -2, 3}, {-3, 2, -1}};
    for (auto d : directions) {
        auto actual = oct_decode(oct_encode(d));
        float length = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
        for (unsigned i = 0; i < 3; ++i) {
            CHECK(near(actual[i], d[i] / length));
        }
    }
    auto v = constant();
    v.grid.origin = {-16, -32, 8};
    v.grid.spacing = {8, 16, 32};
    CHECK((v.position(0) == Float3{-16, -32, 8}));
    CHECK((v.position(7) == Float3{-8, -16, 40}));
    v.probes[7].offset = {1, -2, 3};
    CHECK((v.position(7) == Float3{-7, -18, 43}));
    v.validate();
}

void sampling() {
    auto v = constant();
    for (Float3 p : {Float3{8, 8, 8}, Float3{0, 0, 0}, Float3{16, 16, 16}}) {
        auto sample = sample_probes(v, p, {0, 0, 1});
        CHECK(sample.has_value());
        if (sample) {
            CHECK(near(sample->irradiance[0] / std::numbers::pi_v<float>, 1));
            CHECK(near(sample->irradiance[1], 2));
            CHECK(near(sample->irradiance[2], 0.5f));
        }
    }
    CHECK(!sample_probes(v, {-1, 8, 8}, {0, 0, 1}));
    CHECK(!sample_probes(v, {8, 8, 8}, {0, 0, 0}));
    CHECK(!sample_probes(v, {std::numeric_limits<float>::quiet_NaN(), 8, 8}, {0, 0, 1}));
    // Tiny visibility must not normalize bright outdoor sparse GI back to
    // full intensity inside a wall. Exercise nonzero moment variance too.
    for (auto& m : v.distance_moments) {
        m = {1, 5};
    }
    const auto leaking = sample_probes(v, {8, 8, 8}, {0, 0, 1});
    CHECK(leaking && leaking->confidence > 0 && leaking->confidence < 0.01f);
    if (leaking) {
        CHECK(leaking->irradiance[0] < 0.001f);
    }
    // A zero-variance first-hit shell fully hides receivers beyond the shell.
    for (auto& m : v.distance_moments) {
        m = {1, 1};
    }
    auto occluded = sample_probes(v, {8, 8, 8}, {0, 0, 1});
    CHECK(occluded && (occluded->irradiance == Float3{}));
    // Restore unoccluded distances, but only one relocated neighbour is usable.
    for (auto& m : v.distance_moments) {
        m = {1024, 1024 * 1024};
    }
    for (auto& p : v.probes) {
        p.valid = false;
    }
    CHECK(!sample_probes(v, {8, 8, 8}, {0, 0, 1}));
    v.probes[7].valid = true;
    v.probes[7].offset = {1, 1, 1};
    auto sample = sample_probes(v, {8, 8, 8}, {0, 0, 1});
    CHECK(sample.has_value());
    if (sample) {
        CHECK(near(sample->irradiance[1], 2));
    }
    // Numerical errors in second moments must not produce negative variance.
    for (auto& m : v.distance_moments) {
        m = {1, 0.999999f};
    }
    occluded = sample_probes(v, {8, 8, 8}, {0, 0, 1});
    CHECK(occluded && (occluded->irradiance == Float3{}));
}

void encoded_interpolation() {
    auto v = constant();
    const auto texels = v.grid.texels_per_probe();
    // At the centre, symmetric black/bright x neighbours have equal weights.
    for (std::size_t i = 0; i < v.probes.size(); ++i) {
        for (std::size_t t = 0; t < texels; ++t) {
            v.irradiance[i * texels + t] = i % 2 ? Float3{4, 1, 0} : Float3{};
        }
    }
    const auto mixed = sample_probes(v, {8, 8, 8}, {0, 0, 1});
    CHECK(mixed && near(mixed->irradiance[0], 1) && near(mixed->irradiance[1], .25f));
    // Directional filtering happens on fifth-root encoded texels first.
    // +Z samples four central texels: half encode to 2, half to 0.
    const auto side = v.grid.map_resolution + 2;
    for (std::size_t i = 0; i < v.irradiance.size(); ++i) {
        v.irradiance[i] = i % side < side / 2 ? Float3{32, 32, 32} : Float3{};
    }
    const auto filtered = sample_probes(v, {8, 8, 8}, {0, 0, 1});
    CHECK(filtered && near(filtered->irradiance[0], 1));
    // Visibility attenuation follows the square, so 50% support attenuation
    // still means half the linear light, rather than a quarter.
    v = constant();
    const float chebyshev = std::cbrt(.05f);
    const float variance = chebyshev * 192.0f / (1.0f - chebyshev);
    for (auto& moments : v.distance_moments) {
        moments = {0, variance};
    }
    const auto attenuated = sample_probes(v, {8, 8, 8}, {0, 0, 1}, 0);
    CHECK(attenuated && near(attenuated->irradiance[1], 1));
    // Offline expansion and repacking preserve the quantized baked asset.
    CHECK(encode_probes(decode_probes(encode_probes(v))) == encode_probes(v));
}

void floor_side_visibility() {
    auto volume = constant();
    // Reproduce the Novalis floor: bright probes underneath, dark probes
    // above, and moment depths that incorrectly consider all eight visible.
    for (std::size_t i = 0; i < volume.probes.size(); ++i) {
        for (std::size_t t = 0; t < volume.grid.texels_per_probe(); ++t) {
            volume.irradiance[i * volume.grid.texels_per_probe() + t] =
                i < 4 ? Float3{1, 2, 3} : Float3{};
        }
    }
    const auto floor = sample_probes(volume, {8, 8, 8}, {0, 0, 1});
    CHECK(floor && (floor->irradiance == Float3{}));
    const auto underside = sample_probes(volume, {8, 8, 8}, {0, 0, -1});
    CHECK(underside && near(underside->irradiance[2], 3));
    // All valid probes behind the plane remain valid darkness, never fallback.
    for (std::size_t i = 4; i < volume.probes.size(); ++i) {
        volume.probes[i].valid = false;
    }
    const auto blocked = sample_probes(volume, {8, 8, 8}, {0, 0, 1});
    CHECK(blocked && (blocked->irradiance == Float3{}));
    // Orientation weighting must not cause dark bands when crossing a probe
    // layer in uniformly lit air. Test both sides of the smooth plane guard.
    volume = constant();
    volume.grid.dimensions = {2, 2, 3};
    volume = ProbeVolume::allocate(volume.grid);
    for (auto& probe : volume.probes) {
        probe.valid = true;
    }
    for (auto& e : volume.irradiance) {
        e = {1, 1, 1};
    }
    for (float z : {15.99f, 16.0f, 16.01f, 16.05f, 16.1f, 16.11f, 16.5f}) {
        const auto sample = sample_probes(volume, {8, 8, z}, {0, 0, 1});
        CHECK(sample && near(sample->irradiance[0], 1));
    }
}

void camera_surface_bias() {
    auto volume = constant();
    for (auto& probe : volume.probes) {
        probe.valid = false;
    }
    volume.probes[7].valid = true;
    for (auto& moment : volume.distance_moments) {
        moment = {0.25f, 0.0625f};
    }
    const Float3 p{16, 16, 15}, n{0, 0, 1};
    const auto normal_only = sample_probes(volume, p, n, 0.5f);
    const auto view_only = sample_probes(volume, p, n, 0, {0, 0, 0.5f});
    const auto combined = sample_probes(volume, p, n, 0.5f, {0, 0, 0.5f});
    CHECK(normal_only && (normal_only->irradiance == Float3{}));
    CHECK(view_only && (view_only->irradiance == Float3{}));
    CHECK(combined && near(combined->irradiance[1], 2));
    CHECK(!sample_probes(volume, p, n, 0.5f, {0, 0, std::numeric_limits<float>::infinity()}));
}

void biased_cell_selection() {
    ProbeGrid grid;
    grid.spacing = {4, 4, 4};
    grid.dimensions = {3, 2, 2};
    auto volume = ProbeVolume::allocate(grid);
    for (auto& probe : volume.probes) {
        probe.valid = true;
    }
    for (auto& moment : volume.distance_moments) {
        moment = {100, 10000};
    }
    for (std::size_t i = 0; i < volume.probes.size(); ++i) {
        for (std::size_t t = 0; t < grid.texels_per_probe(); ++t) {
            volume.irradiance[i * grid.texels_per_probe() + t] =
                i % 3 == 0 ? Float3{1, 2, 3} : Float3{};
        }
    }
    // Just inside the rock's grid cell, its corners are dark/behind the face.
    // The biased receiver crosses to the adjacent cell with visible air probes.
    const Float3 n{-1, 0, 0};
    const auto unbiased = sample_probes(volume, {4.1f, 2, 2}, n, 0);
    const auto biased = sample_probes(volume, {4.01f, 2, 2}, n, .5f, {-.5f, 0, 0});
    const auto adjacent = sample_probes(volume, {3.99f, 2, 2}, n, .5f, {-.5f, 0, 0});
    CHECK(unbiased && near(unbiased->irradiance[0], 0));
    CHECK(biased && biased->irradiance[0] > .36f);
    CHECK(adjacent && biased && std::abs(adjacent->irradiance[0] - biased->irradiance[0]) < .05f);
    // A large bias cannot move an outside surface into valid volume coverage.
    CHECK(!sample_probes(volume, {-0.1f, 2, 2}, {1, 0, 0}, .5f));
}

void two_volumes() {
    auto coarse = constant();
    for (auto& e : coarse.irradiance) {
        e = {1, 0, 0};
    }
    auto grid = coarse.grid;
    grid.origin = {4, 4, 4};
    grid.spacing = {2, 2, 2};
    grid.dimensions = {5, 5, 5};
    auto dense = ProbeVolume::allocate(grid);
    for (auto& p : dense.probes) {
        p.valid = true;
    }
    for (auto& e : dense.irradiance) {
        e = {0, 0, 1};
    }
    validate_probe_pair(coarse, dense);
    const auto centre = sample_probe_volumes(coarse, &dense, {8, 8, 8}, {0, 0, 1});
    CHECK(centre && near(centre->irradiance[2], 1) && near(centre->irradiance[0], 0));
    const auto fade = sample_probe_volumes(coarse, &dense, {5, 8, 8}, {0, 0, 1});
    CHECK(fade && near(fade->irradiance[0], 0.5f) && near(fade->irradiance[2], 0.5f));
    for (float x : {3.0f, 4.0f, 12.0f, 13.0f}) {
        const auto sample = sample_probe_volumes(coarse, &dense, {x, 8, 8}, {0, 0, 1});
        CHECK(sample && near(sample->irradiance[0], 1));
    }
    for (auto& p : dense.probes) {
        p.valid = false;
    }
    auto fallback = sample_probe_volumes(coarse, &dense, {8, 8, 8}, {0, 0, 1});
    CHECK(fallback && near(fallback->irradiance[0], 1));
    for (auto& p : dense.probes) {
        p.valid = true;
    }
    for (auto& m : dense.distance_moments) {
        m = {0.01f, 0.0001f};
    }
    fallback = sample_probe_volumes(coarse, &dense, {7, 7, 7}, {0, 0, 1});
    CHECK(fallback && near(fallback->irradiance[0], 0) && near(fallback->irradiance[2], 0));
    for (auto& m : dense.distance_moments) {
        m = {1024, 1024 * 1024};
    }
    for (auto& p : coarse.probes) {
        p.valid = false;
    }
    fallback = sample_probe_volumes(coarse, &dense, {5, 8, 8}, {0, 0, 1});
    CHECK(fallback && near(fallback->irradiance[2], 1));
    CHECK(!sample_probe_volumes(coarse, &dense, {3, 8, 8}, {0, 0, 1}));
    dense.grid.scene_hash++;
    rejects([&] { validate_probe_pair(coarse, dense); });
    dense.grid.scene_hash--;
    dense.grid.origin[0] = -1;
    rejects([&] { validate_probe_pair(coarse, dense); });
    dense.grid.origin[0] = 4;
    dense.grid.spacing[0] = 16;
    rejects([&] { validate_probe_pair(coarse, dense); });
}

void internal_coverage_fade() {
    auto coarse = constant();
    for (auto& e : coarse.irradiance) {
        e = {1, 0, 0};
    }
    auto grid = coarse.grid;
    grid.origin = {4, 4, 4};
    grid.spacing = {2, 2, 2};
    grid.dimensions = {5, 5, 5};
    auto dense = ProbeVolume::allocate(grid);
    for (auto& e : dense.irradiance) {
        e = {0, 0, 1};
    }
    // A usable x=6 plane borders an internal hole starting at x=8.
    // Exercise both invalid probes and valid probes hidden by zero-variance walls.
    for (bool occluded : {false, true}) {
        const auto texels = grid.texels_per_probe();
        for (std::size_t i = 0; i < dense.probes.size(); ++i) {
            const bool usable = i % 5 == 1;
            dense.probes[i].valid = usable || occluded;
            for (std::size_t t = 0; t < texels; ++t) {
                dense.distance_moments[i * texels + t] =
                    usable ? Float2{1024, 1048576} : Float2{0.01f, 0.0001f};
            }
        }
        float previous = 1;
        for (int step = 0; step <= 1001; ++step) {
            const auto sample = sample_probe_volumes(
                coarse, &dense, {6 + static_cast<float>(step) * 0.002f, 7, 7}, {0, 0, 1}
            );
            CHECK(sample.has_value());
            if (!sample) {
                continue;
            }
            const float blue = sample->irradiance[2];
            CHECK(blue <= previous + 0.0001f);
            CHECK(previous - blue < 0.03f);
            if (occluded) {
                CHECK(near(sample->irradiance[0], 0));
                // Partial occlusion must not introduce bright coarse light.
                if (step == 500) {
                    CHECK(near(blue, 1));
                }
            } else {
                CHECK(near(sample->irradiance[0] + blue, 1));
            }
            if (step == 0) {
                CHECK(near(blue, 1));
            }
            if (step >= 999) {
                CHECK(blue < 0.001f);
            }
            previous = blue;
        }
    }
}

void storage() {
    auto v = constant();
    v.probes[3].offset = {1, 2, 3};
    v.probes[4].valid = false;
    auto bytes = encode_probes(v);
    auto loaded = decode_probes(bytes);
    CHECK(loaded.probes[3].offset == v.probes[3].offset);
    CHECK(!loaded.probes[4].valid);
    for (unsigned c = 0; c < 3; ++c) {
        CHECK(std::abs(loaded.irradiance[0][c] - v.irradiance[0][c]) < 0.15f);
    }
    CHECK(loaded.distance_moments == v.distance_moments);
    CHECK(probe_identity_matches(loaded, "rac1-pal", 1, 123, 456));
    CHECK(!probe_identity_matches(loaded, "rac1-ntsc", 1, 123, 456));
    CHECK(!probe_identity_matches(loaded, "rac1-pal", 2, 123, 456));
    CHECK(!probe_identity_matches(loaded, "rac1-pal", 1, 124, 456));
    CHECK(!probe_identity_matches(loaded, "rac1-pal", 1, 123, 457));
    for (auto size : {std::size_t{0}, std::size_t{8}, std::size_t{30}, bytes.size() - 1}) {
        rejects([&] { decode_probes(std::span(bytes).first(size)); });
    }
    bytes.push_back(0);
    rejects([&] { decode_probes(bytes); });
    bytes.pop_back();
    CHECK(loaded.grid.bake_version == kProbeBakeVersion);
    auto legacy = bytes;
    legacy.erase(legacy.begin() + 8, legacy.begin() + 12);
    legacy[4] = 1;
    rejects([&] { decode_probes(legacy); });
    legacy = bytes;
    legacy[4] = 2;
    rejects([&] { decode_probes(legacy); });
    auto future_bake = bytes;
    future_bake[8] = 255;
    rejects([&] { decode_probes(future_bake); });
    bytes[4] = 255;
    rejects([&] { decode_probes(bytes); });
    v.irradiance[0][0] = std::numeric_limits<float>::infinity();
    rejects([&] { encode_probes(v); });
    ProbeGrid bad;
    bad.dimensions[0] = 0;
    rejects([&] { ProbeVolume::allocate(bad); });
    bad.dimensions = {0xffffffffu, 0xffffffffu, 0xffffffffu};
    rejects([&] { ProbeVolume::allocate(bad); });
}

void gutters() {
    auto v = constant();
    const auto side = v.grid.map_resolution + 2;
    for (std::size_t y = 1; y < side - 1; ++y) {
        for (std::size_t x = 1; x < side - 1; ++x) {
            v.irradiance[y * side + x] = {static_cast<float>(x), static_cast<float>(y), 0};
        }
    }
    update_probe_gutters(v);
    CHECK(v.irradiance[0] == v.irradiance[(side - 2) * side + side - 2]);
    CHECK(v.irradiance[2 * side] == v.irradiance[(side - 3) * side + 1]);
    CHECK(v.irradiance[3] == v.irradiance[side + side - 1 - 3]);
}
}  // namespace

int main() {
    coordinates();
    sampling();
    encoded_interpolation();
    floor_side_visibility();
    camera_surface_bias();
    biased_cell_selection();
    two_volumes();
    internal_coverage_fade();
    storage();
    gutters();
    return openrac::test::result();
}

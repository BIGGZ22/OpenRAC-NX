// SPDX-License-Identifier: GPL-3.0-or-later
#include <cfenv>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <thread>

#include "enhanced/lighting/bake.h"
#include "tests/check.h"

using namespace openrac::lighting;

namespace {
BakeScene fixture() {
    BakeScene scene;
    scene.environment = {.3f, .2f, .1f};
    scene.suns = {{{.2f, .1f, -1}, {1, 2, 3}}};
    scene.point_emitters = {{{0, 0, 3}, {2, 1, .5f}}};
    scene.textures.push_back({2, 1, {64, 128, 192, 255, 192, 128, 64, 0}});
    // Floor, cutout wall, and a thin closed slab that exercises relocation.
    scene.triangles =
        {{{{{-30, -30, 0}, {30, -30, 0}, {30, 30, 0}}}},
         {{{{-30, -30, 0}, {30, 30, 0}, {-30, 30, 0}}}},
         {{{{0, -30, 0}, {0, 30, 0}, {0, 30, 30}}}},
         {{{{0, -30, 0}, {0, 30, 30}, {0, -30, 30}}}}};
    for (unsigned i = 2; i < 4; ++i) {
        scene.triangles[i].texture = 0;
        scene.triangles[i].uv = {{{0, 0}, {1, 0}, {1, 1}}};
        scene.triangles[i].emission = {.2f, .1f, .05f};
    }
    const std::array<Float3, 8> corners{
        {{-30, -30, -.2f},
         {30, -30, -.2f},
         {30, 30, -.2f},
         {-30, 30, -.2f},
         {-30, -30, .2f},
         {30, -30, .2f},
         {30, 30, .2f},
         {-30, 30, .2f}}
    };
    for (auto face : std::array<std::array<unsigned, 4>, 6>{
             {{0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}}
         }) {
        scene.triangles.push_back({{{corners[face[0]], corners[face[1]], corners[face[2]]}}});
        scene.triangles.push_back({{{corners[face[0]], corners[face[2]], corners[face[3]]}}});
    }
    return scene;
}
}  // namespace

int main(int argc, char** argv) {
    const bool benchmark = argc == 2 && std::string_view(argv[1]) == "--benchmark";
    ProbeGrid grid;
    grid.origin = {-3, -3, 0};
    grid.spacing = {2, 2, 2};
    grid.dimensions =
        benchmark ? std::array<std::uint32_t, 3>{8, 8, 8} : std::array<std::uint32_t, 3>{4, 4, 4};
    grid.max_distance = 12;
    BakeSettings settings;
    settings.rays_per_probe = benchmark ? 1024 : 128;
    const auto scene = fixture();
    const auto start = std::chrono::steady_clock::now();
    const auto serial = bake_probes(grid, scene, settings, {}, 1);
    const auto serial_seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    const auto encoded = encode_probes(serial);
    const auto caller = std::this_thread::get_id();
    for (unsigned threads : {2u, 4u, 0u}) {
        const auto parallel_start = std::chrono::steady_clock::now();
        std::size_t last = 0, calls = 0;
        const auto parallel = bake_probes(
            grid,
            scene,
            settings,
            [&](std::size_t done, std::size_t total) {
                CHECK(std::this_thread::get_id() == caller);
                CHECK(total == grid.probe_count());
                CHECK(calls == 0 ? done == 0 : done > last);
                CHECK(done <= total);
                last = done;
                ++calls;
            },
            threads
        );
        const auto seconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - parallel_start)
                .count();
        CHECK(last == grid.probe_count());
        CHECK(calls >= 2);
        CHECK(parallel.irradiance == serial.irradiance);
        CHECK(parallel.distance_moments == serial.distance_moments);
        for (std::size_t i = 0; i < serial.probes.size(); ++i) {
            CHECK(parallel.probes[i].valid == serial.probes[i].valid);
            CHECK(parallel.probes[i].offset == serial.probes[i].offset);
        }
        CHECK(encode_probes(parallel) == encoded);
        if (benchmark) {
            std::cout << "threads " << threads << " seconds " << seconds << " speedup "
                      << serial_seconds / seconds << '\n';
        }
    }
    if (benchmark) {
        std::uint64_t hash = 14695981039346656037ull;
        for (auto byte : encoded) {
            hash = (hash ^ byte) * 1099511628211ull;
        }
        std::cout << "serial_seconds " << serial_seconds << " encoded_hash " << hash << '\n';
    } else {
        const int rounding = std::fegetround();
        CHECK(std::fesetround(FE_TOWARDZERO) == 0);
        const auto rounded = bake_probes(grid, scene, settings, {}, 1);
        const auto parallel = bake_probes(grid, scene, settings, {}, 4);
        CHECK(parallel.irradiance == rounded.irradiance);
        CHECK(parallel.distance_moments == rounded.distance_moments);
        CHECK(encode_probes(parallel) == encode_probes(rounded));
        CHECK(std::fegetround() == FE_TOWARDZERO);
        CHECK(std::fesetround(rounding) == 0);
        for (unsigned threads : {1u, 4u}) {
            bool caught = false;
            try {
                bake_probes(
                    grid,
                    scene,
                    settings,
                    [](std::size_t done, std::size_t) {
                        if (done > 0) {
                            throw std::runtime_error("callback cancelled");
                        }
                    },
                    threads
                );
            } catch (const std::runtime_error& error) {
                caught = std::string_view(error.what()) == "callback cancelled";
            }
            CHECK(caught);
        }
        // A small grid clamps even an explicitly excessive thread request.
        grid.dimensions = {2, 2, 2};
        CHECK(
            encode_probes(bake_probes(grid, scene, settings, {}, 1000))
            == encode_probes(bake_probes(grid, scene, settings, {}, 1))
        );
        // A worker's tracing exception must reach the caller after every join.
        BakeScene layered;
        for (unsigned i = 0; i < 4098; ++i) {
            const float z = static_cast<float>(i) + 1;
            layered.triangles.push_back(
                {{{{-1000000, -1000000, z}, {1000000, -1000000, z}, {0, 1000000, z}}}}
            );
        }
        grid.origin = {0, 0, 0};
        grid.dimensions = {2, 2, 2};
        bool caught = false;
        try {
            bake_probes(grid, layered, settings, {}, 4);
        } catch (const std::runtime_error& error) {
            caught = std::string_view(error.what()) == "probe occupancy ray exceeds crossing limit";
        }
        CHECK(caught);
    }
    return openrac::test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include <cfenv>
#include <chrono>
#include <cmath>
#include <fstream>
#include <limits>
#include <numbers>

#include "enhanced/tools/probes/activation.h"
#include "enhanced/tools/probes/level_scene.h"
#include "tests/check.h"

using namespace openrac;
using namespace openrac::lighting;

int main() {
    scene::LevelData level;
    level.level = 3;
    level.light_sets.resize(256);
    level.light_sets[0] = 1;
    level.light_sets[1] = 0.5f;
    level.light_sets[2] = 0.25f;
    level.light_sets[6] = -1;
    level.materials.push_back({});
    scene::Vertex a{}, b{}, c{};
    b.position[0] = 1;
    c.position[1] = 1;
    a.colour[0] = 100;
    b.colour[1] = 99;
    c.colour[2] = 98;  // prelit data must not become albedo
    level.vertices = {a, b, c};
    level.indices = {0, 1, 2};
    scene::Model model;
    model.primitives.push_back({0, 3, 0, -1});
    level.models.push_back(model);
    scene::Instance instance;
    instance.matrix = renderer::translation({10, 20, 30});
    level.instances[static_cast<std::size_t>(scene::Layer::Terrain)].push_back(instance);
    instance.matrix = renderer::translation({40, 50, 60});
    level.instances[static_cast<std::size_t>(scene::Layer::Ties)].push_back(instance);
    instance.matrix = renderer::translation({100, 200, 300});
    level.instances[static_cast<std::size_t>(scene::Layer::Mobys)].push_back(instance);
    auto exported = export_probe_scene(level, 0, {0.1f, 0.1f, 0.1f});
    CHECK(exported.scene.triangles.size() == 2);
    CHECK((exported.scene.triangles[0].vertices[0] == Float3{10, 20, 30}));
    CHECK((exported.scene.triangles[1].vertices[0] == Float3{40, 50, 60}));
    CHECK((exported.scene.triangles[0].diffuse == Float3{1, 1, 1}));
    CHECK(exported.scene.suns.size() == 1);
    {
        auto liquid = level;
        liquid.bake_surfaces.push_back(
            liquid.instances[static_cast<std::size_t>(scene::Layer::Terrain)][0]
        );
        const auto lava = export_probe_scene(liquid, 0, {.1f, .1f, .1f});
        CHECK(lava.surface_instances == 1);
        CHECK(lava.scene.triangles.size() == exported.scene.triangles.size() + 1);
        CHECK(lava.scene_hash != exported.scene_hash);
        for (unsigned layer = 0; layer < scene::kLayerCount; ++layer) {
            CHECK(liquid.instances[layer].size() == level.instances[layer].size());
        }
    }
    {
        auto emitting = level;
        emitting.materials[0].emission = {1, 2, 3};
        emitting.materials[0].emission_mask = scene::EmissionMask::Green;
        auto shared = export_probe_scene(emitting, 0, {.1f, .1f, .1f});
        CHECK(shared.scene.triangles[0].emission == emitting.materials[0].emission);
        CHECK(shared.scene.triangles[0].emission_mask == scene::EmissionMask::Green);
        emitting.materials[0].emission_mask = scene::EmissionMask::None;
        CHECK(shared.scene_hash != export_probe_scene(emitting, 0, {.1f, .1f, .1f}).scene_hash);
        emitting.materials[0].emission_vertex_colour = true;
        emitting.materials[0].diffuse_scale = 0;
        emitting.vertices[0].colour[0] = .25f;
        emitting.vertices[0].colour[1] = 1;
        const auto tinted = export_probe_scene(emitting, 0, {.1f, .1f, .1f});
        CHECK(tinted.scene.triangles[0].emission_vertex_colour);
        CHECK(tinted.scene.triangles[0].emission_colours[0][0] == .25f);
        CHECK(tinted.scene.triangles[0].diffuse_scale == 0);
    }
    {
        auto holders = level;
        holders.moby_classes[1038] = {0, false};
        const auto e = export_probe_scene(holders, 0, {.1f, .1f, .1f});
        CHECK(e.scene.point_emitters.size() == 1);
        CHECK((e.scene.point_emitters[0].position == Float3{100, 200, 300.85f}));
        CHECK(e.scene.triangles.size() == exported.scene.triangles.size());
        CHECK(e.scene_hash != exported.scene_hash);
    }
    CHECK(exported.scene.primary_directional_colour[0] == 1.0f);
    {
        auto without_primary = level;
        without_primary.light_sets[4] = without_primary.light_sets[5] =
            without_primary.light_sets[6] = 0;
        const auto e = export_probe_scene(without_primary, 0, {.1f, .1f, .1f});
        CHECK(e.scene.primary_directional_colour[0] == 1.0f);
    }
    CHECK(exported.scene.suns[0].irradiance[0] == std::numbers::pi_v<float>);
    CHECK((exported.bounds_max == Float3{100, 200, 300}));
    {
        auto props = level;
        auto& prop = props.instances[static_cast<std::size_t>(scene::Layer::Mobys)][0];
        prop.bake_static = true;
        prop.tint = {.5f, 1, 1, 1};
        for (auto& v : props.vertices) {
            std::fill(std::begin(v.colour), std::end(v.colour), 1.0f);
        }
        props.vertices[0].colour[0] = .25f;
        auto selected = export_probe_scene(props, 0, {.1f, .1f, .1f});
        CHECK(selected.scene.triangles.size() == 3);
        CHECK(selected.instances[3] == 1 && selected.skipped_mobys == 0);
        CHECK((selected.scene.triangles.back().vertices[0] == Float3{100, 200, 300}));
        CHECK(selected.scene.triangles.back().multipliers[0][0] == .125f);
        CHECK(selected.scene_hash != exported.scene_hash);
        props.vertices[0].colour[0] = .5f;
        CHECK(export_probe_scene(props, 0, {.1f, .1f, .1f}).scene_hash != selected.scene_hash);
        auto excluded = [&] {
            const auto e = export_probe_scene(props, 0, {.1f, .1f, .1f});
            CHECK(e.scene.triangles.size() == 2);
            CHECK(e.instances[3] == 0 && e.skipped_mobys == 1);
        };
        prop.stand_in = true;
        excluded();
        prop.stand_in = false;
        props.models[0].joints = 1;
        excluded();
        props.models[0].joints = 0;
        prop.palette = 0;
        excluded();
        prop.palette = -1;
        prop.blend = 1;
        excluded();
        prop.blend = 0;
        prop.tint[3] = .5f;
        excluded();
        prop.tint[3] = 1;
        props.vertices[0].light_slot = -2;
        const auto glow = export_probe_scene(props, 0, {.1f, .1f, .1f});
        CHECK(glow.scene.triangles.size() == 2 && glow.skipped_glow == 1);
        CHECK(exported.skipped_mobys == 1);  // no heuristic opt-in for unskinned objects
        props.materials[0].emission = {0, 3, 0};
        const auto emitting = export_probe_scene(props, 0, {.1f, .1f, .1f});
        CHECK(emitting.emissive_triangles == 3);
        CHECK(emitting.skipped_glow == 0);
        CHECK((emitting.scene.triangles.back().diffuse == Float3{}));
        CHECK((emitting.scene.triangles.back().emission == Float3{0, 3, 0}));
        CHECK(emitting.scene_hash != selected.scene_hash);
    }
    {
        auto sky_level = level;
        sky_level.sky_panorama = renderer::Rgba8Image(2, 1);
        sky_level.sky_panorama.pixels = {128, 128, 128, 255, 0, 0, 255, 255};
        auto sky_export = export_probe_scene(sky_level, 0, {.1f, .1f, .1f});
        CHECK(sky_export.scene.sky_width == 2 && sky_export.scene.sky_height == 1);
        CHECK(std::abs(sky_export.scene.sky_radiance[0][0] - .21586f) < .0001f);
        CHECK(sky_export.scene.sky_radiance[1][2] == 1);
        CHECK(sky_export.scene_hash != exported.scene_hash);
        sky_level.sky_panorama.pixels[6] = 0;
        CHECK(
            export_probe_scene(sky_level, 0, {.1f, .1f, .1f}).scene_hash != sky_export.scene_hash
        );
        sky_level.sky_panorama = renderer::Rgba8Image(512, 256);
        for (int y = 0; y < 256; ++y) {
            for (int x = 0; x < 512; ++x) {
                sky_level.sky_panorama
                    .set_texel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xffffffffu : 0xff000000u);
            }
        }
        sky_export = export_probe_scene(sky_level, 0, {.1f, .1f, .1f});
        CHECK(sky_export.scene.sky_width == 256 && sky_export.scene.sky_height == 128);
        for (const auto& pixel : sky_export.scene.sky_radiance) {
            CHECK(std::abs(pixel[0] - .25f) < .00001f);
        }
    }
    level.vertices[0].colour[0] = 0;
    CHECK(export_probe_scene(level, 0, {0.1f, 0.1f, 0.1f}).scene_hash != exported.scene_hash);
    level.vertices[0].colour[0] = 100;
    // Source shading normals are not bake geometry. Flipped, zero, non-unit
    // and nonfinite normals must not enter transport or placement inputs.
    for (Float3 normal :
         {Float3{0, 0, 1},
          Float3{0, 0, -1},
          Float3{},
          Float3{100, -50, 4},
          Float3{std::numeric_limits<float>::quiet_NaN(), 0, 0},
          Float3{0, std::numeric_limits<float>::infinity(), 0}}) {
        for (auto& vertex : level.vertices) {
            std::copy(normal.begin(), normal.end(), vertex.normal);
        }
        const auto changed = export_probe_scene(level, 0, {0.1f, 0.1f, 0.1f});
        CHECK(changed.scene_hash == exported.scene_hash);
        CHECK(changed.scene.triangles.size() == exported.scene.triangles.size());
        for (std::size_t i = 0; i < changed.scene.triangles.size(); ++i) {
            const auto& a = changed.scene.triangles[i];
            const auto& b = exported.scene.triangles[i];
            CHECK(a.vertices == b.vertices);
            CHECK(a.diffuse == b.diffuse);
            CHECK(a.uv == b.uv);
            CHECK(a.texture == b.texture);
            CHECK(a.alpha_cutoff == b.alpha_cutoff);
        }
    }
    level.materials[0].colour[0] = 0.5f;
    CHECK(export_probe_scene(level, 0, {0.1f, 0.1f, 0.1f}).scene_hash != exported.scene_hash);
    const auto grid = sparse_probe_grid(exported, 128, 8);
    grid.validate();
    CHECK(grid.probe_count() > 0);
    const auto local = bounded_probe_grid(exported, {10, 20, 30}, {30, 40, 50}, 4, 8);
    CHECK((local.origin == Float3{8, 20, 28}));
    CHECK((local.dimensions == std::array<std::uint32_t, 3>{7, 6, 7}));
    CHECK(local.scene_hash == grid.scene_hash);
    CHECK(std::abs(local.max_distance - 1.5f * std::sqrt(48.0f)) < 0.0001f);
    const auto vertical =
        bounded_probe_grid(exported, {10, 20, 30}, {30, 40, 50}, Float3{4, 4, 2}, 8);
    CHECK((vertical.spacing == Float3{4, 4, 2}));
    CHECK((vertical.origin == Float3{8, 20, 30}));
    CHECK((vertical.dimensions == std::array<std::uint32_t, 3>{7, 6, 11}));
    CHECK(vertical.max_distance == 9);  // 1.5 times the 4x4x2 cell diagonal
    vertical.validate();
    auto large_scene = exported;
    large_scene.bounds_min = {-1000, -1000, -1000};
    large_scene.bounds_max = {1000, 1000, 1000};
    const auto fine_local = bounded_probe_grid(large_scene, {0, 0, 0}, {8, 8, 8}, 2, 8);
    CHECK(fine_local.probe_count() == 125);
    CHECK(std::abs(fine_local.max_distance - 1.5f * std::sqrt(12.0f)) < 0.0001f);
    CHECK(
        bounded_probe_grid(exported, {0, 0, 0}, {8, 8, 8}, 2, 8).max_distance
        == fine_local.max_distance
    );  // far-away scene bounds cannot inflate wall moments
    for (unsigned axis = 0; axis < 3; ++axis) {
        CHECK(grid.origin[axis] < exported.bounds_min[axis]);
        CHECK(
            grid.origin[axis] + grid.spacing[axis] * static_cast<float>(grid.dimensions[axis] - 1)
            > exported.bounds_max[axis]
        );
    }
    BakeSettings settings;
    const auto hash = probe_bake_settings_hash(grid, settings);
    auto sky_settings = settings;
    sky_settings.use_level_sky = false;
    CHECK(probe_bake_settings_hash(grid, sky_settings) != hash);
    sky_settings = settings;
    sky_settings.sky_intensity = 2;
    CHECK(probe_bake_settings_hash(grid, sky_settings) != hash);
    sky_settings = settings;
    sky_settings.sky_weight = 0.3f;
    CHECK(probe_bake_settings_hash(grid, sky_settings) != hash);
    sky_settings = settings;
    sky_settings.sunlight_scale = {.4f, .7f, 1.2f};
    CHECK(probe_bake_settings_hash(grid, sky_settings) != hash);
    auto placement_settings = settings;
    placement_settings.min_inside_fraction = 0.875f;
    CHECK(probe_bake_settings_hash(grid, placement_settings) != hash);
    settings.rays_per_probe *= 2;
    CHECK(probe_bake_settings_hash(grid, settings) != hash);
    level.materials[0].blend = true;
    bool rejected = false;
    try {
        export_probe_scene(level, 0, {0.1f, 0.1f, 0.1f});
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    CHECK(rejected);
    // Runtime activation without a graphics context: correct inputs accepted;
    // stale geometry, different settings and the wrong region are rejected.
    level.materials[0].colour[0] = 1;
    level.materials[0].blend = false;
    auto active_grid = grid;
    active_grid.game_version = "rac1-ntsc";
    active_grid.level = 3;
    ProbeActivation expected;
    active_grid.settings_hash = probe_bake_settings_hash(active_grid, expected.bake);
    auto volume = ProbeVolume::allocate(active_grid);
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path()
                      / ("openrac_probe_activation_" + std::to_string(unique));
    if (!std::filesystem::create_directory(root)) {
        throw std::runtime_error("test directory already exists");
    }
    // Shared defaults, partial level/mod overrides and invalid profiles.
    CHECK((LevelBakeProfile{}.sparse_spacing == Float3{256, 256, 256}));
    CHECK((load_level_bake_profile(root).sparse_spacing == Float3{256, 256, 256}));
    CHECK(BakeSettings{}.sky_intensity == 1.0f);
    CHECK(BakeSettings{}.sky_weight == 0.7f);
    CHECK(load_level_bake_profile(root).activation.bake.sky_weight == 0.7f);
    CHECK(BakeSettings{}.terrain_vertex_albedo);
    CHECK(load_level_bake_profile(root).activation.bake.terrain_vertex_albedo);
    CHECK(load_level_bake_profile(root).activation.bake.sky_intensity == 1.0f);
    const auto profile_path = root / "probe-bake.json";
    auto profile_source = [&](const char* text) {
        std::ofstream output(profile_path);
        output << text;
    };
    profile_source(R"({"version":1})");
    CHECK((load_level_bake_profile(root).sparse_spacing == Float3{256, 256, 256}));
    CHECK(load_level_bake_profile(root).activation.bake.sky_weight == 0.7f);
    CHECK(load_level_bake_profile(root).activation.bake.terrain_vertex_albedo);
    profile_source(R"({"version":1,"relocation_max_fraction":0.45})");
    CHECK(load_level_bake_profile(root).activation.bake.relocation_max_fraction == 0.45f);
    profile_source(R"({"version":1,"terrain_vertex_albedo":false})");
    CHECK(!load_level_bake_profile(root).activation.bake.terrain_vertex_albedo);
    profile_source(R"({"version":1,"sky_intensity":0.75,"sky_weight":0.3,"rays":32,"bounces":2,
        "environment":[0.1,0.2,0.3],"sparse_spacing":[64,64,64],
        "dense_spacing":[4,4,2],"resolution":6,"light_set":2,"sunlight_scale":[0.4,0.7,1.2]})"
    );
    auto profile = load_level_bake_profile(root);
    CHECK(profile.activation.bake.sky_intensity == 0.75f);
    CHECK(profile.activation.bake.sky_weight == 0.3f);
    CHECK(profile.activation.bake.terrain_vertex_albedo);
    CHECK((profile.activation.bake.sunlight_scale == Float3{.4f, .7f, 1.2f}));
    const int profile_rounding = std::fegetround();
    std::fesetround(FE_TOWARDZERO);
    const auto game_profile = load_level_bake_profile(root);
    CHECK(std::fegetround() == FE_TOWARDZERO);
    std::fesetround(profile_rounding);
    CHECK(game_profile.activation.bake.sunlight_scale == profile.activation.bake.sunlight_scale);
    CHECK(profile.activation.bake.rays_per_probe == 32);
    CHECK(profile.activation.bake.bounces == 2);
    CHECK(profile.activation.light_set == 2);
    CHECK(profile.activation.environment[1] == 0.2f);
    CHECK(profile.sparse_spacing[0] == 64);
    CHECK(profile.dense_spacing[2] == 2);
    const auto mod = root / "mod";
    std::filesystem::create_directory(mod);
    {
        std::ofstream output(mod / "probe-bake.json");
        output << R"({"version":1,"sky_intensity":2})";
    }
    profile = load_level_bake_profile(root, mod);
    CHECK(profile.activation.bake.sky_intensity == 2);
    CHECK(profile.activation.bake.rays_per_probe == 32);
    std::filesystem::remove(mod / "probe-bake.json");
    std::filesystem::remove(mod);
    for (const auto* invalid :
         {R"({"version":2})",
          R"({"version":1,"sky_intensitty":1})",
          R"({"version":1,"sky_intensity":-1})",
          R"({"version":1,"sky_weight":1.1})",
          R"({"version":1,"sky_weight":-0.1})",
          R"({"version":1,"sunlight_scale":[-0.1,1,1]})",
          R"({"version":1,"sunlight_scale":[1,5,1]})",
          R"({"version":1,"sunlight_scale":0.5})",
          R"({"version":1,"terrain_vertex_albedo":1})",
          R"({"version":1,"rays":16.5})",
          R"({"version":1,"rays":8})",
          R"({"version":1,"sparse_spacing":[1,0,1]})",
          R"({"version":1,"relocation_max_fraction":0.46})",
          R"({"version":1,"relocation_max_fraction":0.5})"}) {
        profile_source(invalid);
        bool denied = false;
        try {
            load_level_bake_profile(root);
        } catch (const std::runtime_error&) {
            denied = true;
        }
        CHECK(denied);
    }
    std::filesystem::remove(profile_path);
    level.dir = root;
    const auto asset = root / "test.orpg", manifest = root / "manifest.json";
    auto identity = [&](const char* version) {
        std::ofstream output(manifest);
        output << "{\"game\":\"rac1\",\"version\":\"" << version << "\"}";
    };
    identity("ntsc");
    const auto geometry = root / "fixture.glb";
    auto geometry_source = [&](const char* text) {
        std::ofstream output(geometry);
        output << text;
    };
    geometry_source("first geometry fixture");
    volume.grid.scene_hash = export_probe_scene(level, 0, expected.environment).scene_hash;
    {
        std::ofstream(root / "pvs.json") << "camera-only visibility metadata";
        std::ofstream(root / "pvs.bin", std::ios::binary) << "camera grid fixture";
        CHECK(
            export_probe_scene(level, 0, expected.environment).scene_hash == volume.grid.scene_hash
        );
        std::filesystem::remove(root / "pvs.json");
        std::filesystem::remove(root / "pvs.bin");
    }
    const auto bytes = encode_probes(volume);
    {
        std::ofstream output(asset, std::ios::binary);
        output.write(
            reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())
        );
    }
    auto refuses = [&](const ProbeActivation& wanted) {
        bool denied = false;
        try {
            load_level_probes(asset, level, wanted);
        } catch (const std::runtime_error&) {
            denied = true;
        }
        CHECK(denied);
    };
    CHECK(load_level_probes(asset, level, expected).grid.scene_hash == volume.grid.scene_hash);
    level.emission_profile_revision = 1;
    refuses(expected);
    level.emission_profile_revision = 0;
    const auto previous_rounding = std::fegetround();
    std::fesetround(FE_TOWARDZERO);
    CHECK(load_level_probes(asset, level, expected).grid.scene_hash == volume.grid.scene_hash);
    std::fesetround(previous_rounding);
    geometry_source("changed geometry fixture");
    refuses(expected);
    geometry_source("first geometry fixture");
    ProbeActivation different = expected;
    different.bake.rays_per_probe *= 2;
    refuses(different);
    identity("pal");
    refuses(expected);
    std::filesystem::remove(asset);
    std::filesystem::remove(manifest);
    std::filesystem::remove(geometry);
    std::filesystem::remove(root);
    return test::result();
}

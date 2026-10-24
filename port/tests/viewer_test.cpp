// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The level viewer on a synthetic level written by the test itself, in the
// shape the editor's port export writes (manifest.json, placements.json,
// glTF meshes, PNG textures): the JSON reader, the loader, and, where an
// OpenGL context can be made, one frame drawn and its pixels checked.
//
//   viewer_test DIR    writes the level into DIR (replacing it)

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stb_image_write.h>
#include <string>
#include <vector>

#include "check.h"
#include "common/json.h"
#include "game/rac1/game_state.h"
#include "gpu_draw_reference.h"
#include "renderer/framebuffer.h"
#include "renderer/renderer.h"
#include "renderer_gl_context.h"
#include "scene/level.h"
#include "scene/level_renderer.h"
#include "viewer/camera.h"

namespace {

namespace fs = std::filesystem;
using namespace openrac;

viewer::Pvs pvs_fixture(unsigned height = 0) {
    viewer::Pvs pvs;
    pvs.grid.resize(32 + 256);
    auto put = [&](unsigned at, unsigned value) {
        pvs.grid[at] = static_cast<std::uint8_t>(value);
        pvs.grid[at + 1] = static_cast<std::uint8_t>(value >> 8);
    };
    put(0, 32);  // mask array
    put(4, height);
    put(6, 1);
    put(8, 3);  // z -> y node at 12
    put(12, 0);
    put(14, 1);
    put(16, 5);  // y -> x node at 20
    put(20, 0);
    put(22, 2);
    put(24, 0);
    put(26, 1);
    pvs.grid[32] = 1;
    pvs.grid[160] = 2;
    return pvs;
}

void pvs_lookup() {
    auto pvs = pvs_fixture();
    auto a = pvs.select({0, 0, 0});
    auto b = pvs.select({4, 0, 0});
    CHECK(viewer::Pvs::visible(a, 1));
    CHECK(!viewer::Pvs::visible(a, 2));
    CHECK(!viewer::Pvs::visible(b, 1));
    CHECK(viewer::Pvs::visible(b, 2));
    CHECK(viewer::Pvs::visible(a, viewer::Pvs::always));
    CHECK(viewer::Pvs::visible(a, 0));
    CHECK(viewer::Pvs::visible(a, 3));
    CHECK(viewer::Pvs::visible(a, 0xff01));
    CHECK(pvs.select({-3.9f, 0, 0}) == a);  // truncate, never floor
    CHECK(pvs.select({8, 0, 0}) == b);      // neighbouring-cell union
    CHECK(pvs.select({400, 0, 0}) == viewer::Pvs::all_visible());
    CHECK(pvs.select({NAN, 0, 0}) == viewer::Pvs::all_visible());
    CHECK(pvs.select({INFINITY, 0, 0}) == viewer::Pvs::all_visible());
    for (const unsigned size : {0u, 7u, 31u, 33u}) {
        auto broken = pvs;
        broken.grid.resize(size);
        CHECK(broken.select({0, 0, 0}) == viewer::Pvs::all_visible());
    }
}

void pvs_loader(const fs::path& dir) {
    // Identity is checked before any mapping is activated; absent metadata is
    // already covered by the normal loader fixture below.
    const auto pvs = pvs_fixture();
    std::ofstream grid(dir / "pvs.bin", std::ios::binary);
    grid.write(
        reinterpret_cast<const char*>(pvs.grid.data()),
        static_cast<std::streamsize>(pvs.grid.size())
    );
    grid.close();
    const auto metadata = [&](const char* version) {
        std::ofstream(dir / "pvs.json")
            << "{\"format\":1,\"game\":\"rac1\",\"version\":\"" << version
            << "\",\"level\":3,\"grid\":\"pvs.bin\",\"terrain\":[1]}";
    };
    viewer::LevelData loaded;
    std::string error;
    metadata("ntsc");
    CHECK(viewer::load_level(dir, loaded, error));
    CHECK(loaded.pvs.grid.empty());
    metadata("pal");
    CHECK(viewer::load_level(dir, loaded, error));
    CHECK(loaded.pvs.grid == pvs.grid);
    fs::remove(dir / "pvs.json");
    fs::remove(dir / "pvs.bin");
}

std::string base64(const std::vector<std::uint8_t>& data) {
    static const char* const kAlphabet =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    for (std::size_t i = 0; i < data.size(); i += 3) {
        const std::uint32_t a = data[i];
        const std::uint32_t b = i + 1 < data.size() ? data[i + 1] : 0;
        const std::uint32_t c = i + 2 < data.size() ? data[i + 2] : 0;
        const std::uint32_t v = (a << 16) | (b << 8) | c;
        out += kAlphabet[(v >> 18) & 63];
        out += kAlphabet[(v >> 12) & 63];
        out += i + 1 < data.size() ? kAlphabet[(v >> 6) & 63] : '=';
        out += i + 2 < data.size() ? kAlphabet[v & 63] : '=';
    }
    return out;
}

void write_text(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << text;
}

// A square in the XY plane (Z up), side `size`, centred on the origin at
// height z, textured with `image` (a URI relative to the file).
void write_square(const fs::path& path, float size, float z, const std::string& image) {
    const float h = size / 2;
    const float positions[] = {-h, -h, z, h, -h, z, -h, h, z, h, h, z};
    const float normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
    const float uvs[] = {0, 1, 1, 1, 0, 0, 1, 0};
    const std::uint32_t indices[] = {0, 1, 2, 1, 3, 2};
    std::vector<std::uint8_t> buffer;
    auto append = [&](const void* data, std::size_t bytes) {
        const auto* p = static_cast<const std::uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + bytes);
    };
    append(positions, sizeof(positions));
    append(normals, sizeof(normals));
    append(uvs, sizeof(uvs));
    append(indices, sizeof(indices));
    const std::string gltf =
        std::string(
            R"({"asset":{"version":"2.0"},"extensionsUsed":["KHR_materials_emissive_strength"],"scene":0,"scenes":[{"nodes":[0]}],)"
        )
        + R"("nodes":[{"name":"square","mesh":0}],)"
        + R"("meshes":[{"primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TEXCOORD_0":2},"indices":3,"material":0}]}],)"
        + R"("materials":[{"pbrMetallicRoughness":{"baseColorTexture":{"index":0}},"doubleSided":true,)"
        + R"("emissiveFactor":[0,0.25,0],"emissiveTexture":{"index":0},"extensions":{"KHR_materials_emissive_strength":{"emissiveStrength":2}}}],)"
        + R"("textures":[{"source":0}],"images":[{"uri":")" + image + R"("}],)" + R"("accessors":[)"
        + R"({"bufferView":0,"componentType":5126,"count":4,"type":"VEC3","min":[)"
        + std::to_string(-h) + "," + std::to_string(-h) + "," + std::to_string(z) + R"(],"max":[)"
        + std::to_string(h) + "," + std::to_string(h) + "," + std::to_string(z) + "]},"
        + R"({"bufferView":1,"componentType":5126,"count":4,"type":"VEC3"},)"
        + R"({"bufferView":2,"componentType":5126,"count":4,"type":"VEC2"},)"
        + R"({"bufferView":3,"componentType":5125,"count":6,"type":"SCALAR"}],)"
        + R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":48},{"buffer":0,"byteOffset":48,"byteLength":48},)"
        + R"({"buffer":0,"byteOffset":96,"byteLength":32},{"buffer":0,"byteOffset":128,"byteLength":24}],)"
        + R"("buffers":[{"byteLength":152,"uri":"data:application/octet-stream;base64,)"
        + base64(buffer) + R"("}]})";
    write_text(path, gltf);
}

void write_png(const fs::path& path, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    fs::create_directories(path.parent_path());
    const std::uint8_t pixels[] = {r, g, b, 255, r, g, b, 255, r, g, b, 255, r, g, b, 255};
    stbi_write_png(path.string().c_str(), 2, 2, 4, pixels, 8);
}

void write_level(const fs::path& dir) {
    fs::remove_all(dir);
    write_png(dir / "textures" / "terrain_0000.png", 220, 30, 30);
    write_png(dir / "textures" / "tie_0001.png", 30, 30, 220);
    write_png(dir / "sky.png", 80, 120, 200);
    write_square(dir / "terrain.glb.gltf", 20.0f, 0.0f, "textures/terrain_0000.png");
    write_square(dir / "ties" / "tie_5.gltf", 2.0f, 0.0f, "../textures/tie_0001.png");
    write_text(dir / "manifest.json", R"({
  "format": 3, "game": "rac1", "version": "pal", "level": 3,
  "terrain": "terrain.glb.gltf",
  "sky": {"background": [10, 20, 30], "mesh": null, "panorama": "sky.png"},
  "classes": {
    "tie": {"5": {"mesh": "ties/tie_5.gltf"}},
    "shrub": {},
    "moby": {"7": {"mesh": null, "scale": 0.25, "name": "crate"}}
  },
  "placements": "placements.json"
})");
    write_text(dir / "placements.json", R"({
  "format": 3,
  "ties": [{"index": 0, "class": 5, "matrix": [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,1,1], "draw_distance": 100}],
  "shrubs": [],
  "mobys": [{"index": 0, "class": 7, "matrix": [2,0,0,0, 0,2,0,0, 0,0,2,0, 6,6,0,1], "colour": [128, 128, 128]},
            {"index": 1, "class": 99, "matrix": [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]}]
})");
}

void json_reader() {
    using json::Value;
    Value v;
    std::string error;
    CHECK(json::parse(
        R"({"a": [1, 2.5e1, -3], "b": {"c": "xé\n"}, "t": true, "n": null})", v, error
    ));
    CHECK(v["a"].items().size() == 3);
    CHECK(v["a"][1].number() == 25.0);
    CHECK(v["a"][2].number() == -3.0);
    CHECK(v["b"]["c"].string() == "x\xc3\xa9\n");
    CHECK(v["t"].boolean());
    CHECK(v["n"].is_null());
    CHECK(v["missing"]["deeper"].is_null());
    CHECK(!json::parse("{\"a\": }", v, error));
    CHECK(!error.empty());
    CHECK(!json::parse("[1, 2", v, error));
    CHECK(!json::parse("[1] x", v, error));
}

void loader(const fs::path& dir, viewer::LevelData& level) {
    std::string error;
    CHECK(viewer::load_level(dir, level, error));
    if (!error.empty()) {
        std::fprintf(stderr, "%s\n", error.c_str());
    }
    CHECK(level.level == 3);
    CHECK(level.models.size() == 3);  // terrain, the tie class, the moby box
    CHECK(level.instances[static_cast<std::size_t>(viewer::Layer::Terrain)].size() == 1);
    CHECK(level.instances[static_cast<std::size_t>(viewer::Layer::Ties)].size() == 1);
    // The placement of an unknown class is left out.
    CHECK(level.instances[static_cast<std::size_t>(viewer::Layer::Mobys)].size() == 1);
    CHECK(level.images.size() == 2);
    CHECK((level.materials[0].emission == viewer::Vec3{0, .5f, 0}));
    CHECK(level.materials[0].emission_image == level.materials[0].image);
    CHECK(level.sky_panorama.width == 2 && level.sky_panorama.height == 2);
    CHECK(level.sky_panorama.pixels[0] == 80 && level.sky_panorama.pixels[2] == 200);
    CHECK(level.missing_images == 0);
    CHECK(level.vertices.size() == 4 + 4 + 24);
    CHECK(std::fabs(level.background[2] - 30.0f / 255.0f) < 1e-6f);
    const auto& moby = level.instances[static_cast<std::size_t>(viewer::Layer::Mobys)][0];
    // A class without a mesh is a box, tinted by class; its scale is the
    // placement's alone (the class scale applies to meshes).
    CHECK(moby.tint[0] != 1.0f || moby.tint[1] != 1.0f || moby.tint[2] != 1.0f);
    CHECK(moby.matrix[0] == 2.0f && moby.matrix[12] == 6.0f);
    const auto& tie = level.instances[static_cast<std::size_t>(viewer::Layer::Ties)][0];
    CHECK(tie.matrix[14] == 1.0f);
    CHECK(level.bounds_min[0] == -10.0f && level.bounds_max[0] >= 6.5f);
    {
        const auto ooze_dir = dir / "orxon-ooze";
        write_level(ooze_dir);
        write_png(ooze_dir / "textures/terrain_0012.png", 80, 220, 30);
        write_square(ooze_dir / "terrain.glb.gltf", 20, 0, "textures/terrain_0012.png");
        write_text(ooze_dir / "manifest.json", R"({
          "format":3,"game":"rac1","version":"ntsc","level":10,
          "terrain":"terrain.glb.gltf","placements":"placements.json",
          "classes":{"tie":{"5":{"mesh":"ties/tie_5.gltf"}}}
        })");
        viewer::LevelData ooze;
        CHECK(viewer::load_level(ooze_dir, ooze, error));
        CHECK((ooze.materials[0].emission == viewer::Vec3{1, 1, 1}));
        CHECK(ooze.emission_profile_revision == 2);
        CHECK(ooze.materials[0].emission_vertex_colour);
        CHECK(ooze.materials[0].diffuse_scale == 0);
        CHECK((ooze.materials[1].emission == viewer::Vec3{0, .5f, 0}));
        const auto source_emission = ooze.materials[0].emission;
        write_text(ooze_dir / "probe-emission.json", R"({
          "textures/terrain_0012.png":{"preserve_authored_lighting":false}
        })");
        CHECK(viewer::load_level(ooze_dir, ooze, error));
        CHECK(ooze.materials[0].emission_mask == lighting::EmissionMask::None);
        CHECK(ooze.materials[0].emission == source_emission);
        for (const char* window : {"textures/tie_0048.png", "textures/terrain_0059.png"}) {
            write_png(ooze_dir / window, 80, 220, 30);
            write_square(ooze_dir / "terrain.glb.gltf", 20, 0, window);
            CHECK(viewer::load_level(ooze_dir, ooze, error));
            CHECK(ooze.materials[0].emission_mask == lighting::EmissionMask::Green);
            CHECK(!ooze.materials[0].emission_vertex_colour);
            CHECK(ooze.materials[0].diffuse_scale == 1);
            CHECK(ooze.materials[1].emission_mask == lighting::EmissionMask::None);
            CHECK(ooze.materials[0].emission == source_emission);
        }
        write_square(ooze_dir / "lava.gltf", 20, 0, "textures/probe_gaspar_lava.png");
        write_png(ooze_dir / "textures/probe_gaspar_lava.png", 255, 255, 255);
        write_text(ooze_dir / "manifest.json", R"({
          "format":3,"game":"rac1","version":"ntsc","level":9,
          "terrain":"terrain.glb.gltf","placements":"placements.json"
        })");
        write_text(ooze_dir / "probe-emission.json", R"({"surfaces":[{"mesh":"lava.gltf"}]})");
        CHECK(viewer::load_level(ooze_dir, ooze, error));
        CHECK(ooze.bake_surfaces.size() == 1);
        CHECK(ooze.instances[static_cast<std::size_t>(viewer::Layer::Terrain)].size() == 1);
        CHECK(ooze.emission_profile_revision == 1);
        const auto lava_model = ooze.bake_surfaces[0].model;
        const auto lava_material = ooze.models[lava_model].primitives[0].material;
        CHECK((ooze.materials[lava_material].emission == viewer::Vec3{1, 1, 1}));
        CHECK(ooze.materials[lava_material].emission_vertex_colour);
        CHECK(ooze.materials[lava_material].diffuse_scale == 0);
    }
    // The images: red terrain, blue tie.
    CHECK((level.images[0].texel(0, 0) & 0xFF) == 220);
    CHECK(((level.images[1].texel(1, 1) >> 16) & 0xFF) == 220);
    {
        const auto selection_dir = dir / "bake-selection";
        write_level(selection_dir);
        write_text(selection_dir / "manifest.json", R"({
          "format":3,"game":"rac1","version":"ntsc","level":3,
          "terrain":"terrain.glb.gltf","placements":"placements.json",
          "classes":{"moby":{
            "817":{"mesh":"ties/tie_5.gltf"},
            "1827":{"mesh":"ties/tie_5.gltf","bake_static":false},
            "500":{"mesh":null,"bake_static":true}}}
        })");
        write_text(selection_dir / "placements.json", R"({"mobys":[
          {"class":817,"matrix":[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]},
          {"class":817,"bake_static":false,"matrix":[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]},
          {"class":1827,"matrix":[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]},
          {"class":1827,"bake_static":true,"matrix":[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]},
          {"class":500,"matrix":[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]}]})");
        viewer::LevelData selected;
        CHECK(viewer::load_level(selection_dir, selected, error));
        const auto& props = selected.instances[static_cast<std::size_t>(viewer::Layer::Mobys)];
        CHECK(props.size() == 5);
        CHECK(props[0].bake_static && !props[0].stand_in);
        CHECK(!props[1].bake_static && !props[2].bake_static);
        CHECK(props[3].bake_static);
        CHECK(props[4].bake_static && props[4].stand_in);
        const auto source_emission = selected.materials[0].emission;
        write_text(selection_dir / "probe-emission.json", R"({
            "textures/terrain_0000.png":{"colour":[0.25,0.5,1],"intensity":4,"use_base_texture":false,"preserve_authored_lighting":true}
        })");
        CHECK(viewer::load_level(selection_dir, selected, error));
        CHECK((selected.materials[0].emission == viewer::Vec3{1, 2, 4}));
        CHECK(selected.materials[0].emission_image == -1);
        CHECK(selected.materials[0].emission_mask == lighting::EmissionMask::None);
        write_text(selection_dir / "probe-emission.json", R"({
            "textures/terrain_0000.png":{"preserve_authored_lighting":true}
        })");
        CHECK(viewer::load_level(selection_dir, selected, error));
        CHECK(selected.materials[0].emission_mask == lighting::EmissionMask::None);
        CHECK(selected.materials[0].emission == source_emission);
        write_text(selection_dir / "probe-emission.json", R"({
            "textures/terrain_0000.png":{"colour":[1,2,3],"texture":"textures/tie_0001.png","mask":"green"}
        })");
        CHECK(viewer::load_level(selection_dir, selected, error));
        CHECK((selected.materials[0].emission == viewer::Vec3{1, 2, 3}));
        CHECK(selected.materials[0].emission_mask == lighting::EmissionMask::Green);
        CHECK(selected.materials[0].emission_image != selected.materials[0].image);
        write_text(selection_dir / "probe-emission.json", R"({
            "textures/terrain_0000.png":{"mask":"bad"}
        })");
        CHECK(!viewer::load_level(selection_dir, selected, error));
        write_text(selection_dir / "probe-emission.json", R"({
            "textures/terrain_0000.png":{"intensity":-1}
        })");
        CHECK(!viewer::load_level(selection_dir, selected, error));
        write_square(selection_dir / "unsupported.gltf", 2, 0, "textures/tie_0001.png");
        const auto unsupported = selection_dir / "unsupported.gltf";
        std::ifstream input(unsupported);
        std::string source((std::istreambuf_iterator<char>(input)), {});
        input.close();
        const auto texture = source.find("\"emissiveTexture\":{\"index\":0}");
        CHECK(texture != std::string::npos);
        source.replace(
            texture,
            std::strlen("\"emissiveTexture\":{\"index\":0}"),
            "\"emissiveTexture\":{\"index\":0,\"texCoord\":1}"
        );
        write_text(unsupported, source);
        viewer::LevelData unsupported_level;
        CHECK(!viewer::load_model(unsupported, unsupported_level, error).has_value());
        CHECK(error.find("TEXCOORD_0") != std::string::npos);
    }
}

int draw(viewer::LevelData& level) {
    test::GlContext context;
    std::string why;
    if (!context.open(why)) {
        std::printf("SKIP: no OpenGL 4.1 core context: %s\n", why.c_str());
        return test::kSkip;
    }
    renderer::Renderer renderer;
    viewer::LevelScene scene;
    std::string error;
    CHECK(scene.upload(level, renderer.textures(), error));
    for (std::size_t layer = 0; layer < viewer::kLayerCount; ++layer) {
        renderer.add(
            std::make_unique<viewer::LayerRenderer>(scene, static_cast<viewer::Layer>(layer)), error
        );
    }
    CHECK(renderer.init(error));
    renderer::FrameBuffer frame;
    CHECK(frame.create(128, 128, error));

    // Looking straight down from 30 units: the terrain square fills the
    // middle, the tie sits on it at the centre, the sky colour round it.
    viewer::FlyCamera camera;
    camera.position = {0.0f, 0.0f, 30.0f};
    camera.pitch = -1.55f;
    renderer::FrameInput input;
    input.camera.view = camera.view();
    input.camera.projection = camera.projection(1.0f);
    input.clear_colour = {level.background[0], level.background[1], level.background[2], 1.0f};
    renderer.render(input, {frame.id(), 128, 128});
    const std::vector<std::uint8_t> px = frame.read_rgba();
    auto at = [&](int x, int y) {
        return &px[static_cast<std::size_t>((y * 128 + x) * 4)];
    };
    const std::uint8_t* corner = at(2, 2);
    CHECK(corner[0] == 10 && corner[1] == 20 && corner[2] == 30);
    const std::uint8_t* terrain = at(64, 40);
    CHECK(terrain[0] > 150 && terrain[1] < 60 && terrain[2] < 60);
    const std::uint8_t* tie = at(64, 64);
    CHECK(tie[2] > 150 && tie[0] < 60);
    // Terrain and tie each draw once. Mobys also submit a glow pass; this
    // fixture has no glow faces, so that pass discards all its fragments.
    CHECK(renderer.last_stats().draw_calls == 4);

    // Removing the moby layer removes both its solid and glow submissions.
    renderer.renderers()[static_cast<std::size_t>(viewer::Layer::Mobys)]->enabled = false;
    renderer.render(input, {frame.id(), 128, 128});
    CHECK(renderer.last_stats().draw_calls == 2);

    // What the camera does not see is not drawn: looking down far beside the level, the terrain,
    // the tie and the moby are all outside the picture.
    renderer.renderers()[static_cast<std::size_t>(viewer::Layer::Mobys)]->enabled = true;
    camera.position = {500.0f, 0.0f, 30.0f};
    input.camera.view = camera.view();
    renderer.render(input, {frame.id(), 128, 128});
    CHECK(renderer.last_stats().draw_calls == 0);

    // PVS rejects camera geometry, independently of frustum culling. Re-upload
    // does not keep the previous level's mask or compacted instance buffers.
    auto baked = level;
    baked.pvs = pvs_fixture(7);
    baked.models[baked.instances[1][0].model].primitives[0].occlusion = 1;
    auto& fragments = baked.models[baked.instances[1][0].model].primitives;
    auto hidden_fragment = fragments[0];
    hidden_fragment.occlusion = 2;
    fragments.push_back(hidden_fragment);
    fragments.push_back(fragments[0]);
    baked.instances[2][0].occlusion = 2;
    baked.instances[4][0].occlusion = 2;
    auto second_tie = baked.instances[2][0];
    second_tie.matrix[12] = 6;
    second_tie.occlusion = 1;
    baked.instances[2].push_back(second_tie);
    scene.release();
    CHECK(scene.upload(baked, renderer.textures(), error));
    scene.lighting = false;
    camera.position = {0, 0, 30};
    input.camera.view = camera.view();
    input.camera.position = camera.position;
    renderer.render(input, {frame.id(), 128, 128});
    CHECK(renderer.last_stats().draw_calls == (gl::is_vulkan() ? 2 : 4));
    auto pvs_pixels = frame.read_rgba();
    if (gl::is_vulkan()) {
        CHECK(pvs_pixels[(64 * 128 + 64) * 4] > 150);  // hidden blue tie exposes red terrain
    } else {
        CHECK(pvs_pixels[(64 * 128 + 64) * 4 + 2] > 150);  // GL/ES keep drawing the blue tie
    }
    camera.position[0] = 4;
    input.camera.position = camera.position;
    input.camera.view = camera.view();
    renderer.render(input, {frame.id(), 128, 128});
    CHECK(renderer.last_stats().draw_calls == 4);  // alternate fragment, tie and moby survive
    camera.position[0] = 0;
    input.camera.position = camera.position;
    input.camera.view = camera.view();
    renderer.render(input, {frame.id(), 128, 128});
    CHECK(renderer.last_stats().draw_calls == (gl::is_vulkan() ? 2 : 4));  // camera cache refresh

    if (gl::is_vulkan()) {
        stats::set_enabled(stats::Client::Overlay, true);
        // Exercise multiple compaction chunks and order-sensitive blended objects.
        for (unsigned n = 0; n < 300; ++n) {
            auto tie_copy = second_tie;
            tie_copy.matrix =
                renderer::translation({n % 3 == 0 ? 5000.f : float(n % 11) - 5, 0, 1});
            tie_copy.occlusion = n % 2 ? 1 : 2;
            if (n % 7 == 0) {
                tie_copy.palette = 0;
            }
            baked.instances[2].push_back(tie_copy);
        }
        for (unsigned n = 0; n < 130; ++n) {
            auto blended = baked.instances[4][0];
            blended.blend = 1;
            blended.occlusion = n % 2 ? 1 : 2;
            blended.matrix = renderer::translation({0, 0, 1});
            blended.tint = {n % 3 ? 1.f : .2f, n % 3 ? .2f : 1.f, .5f, .1f};
            baked.instances[4].push_back(blended);
        }
        scene.release();
        CHECK(scene.upload(baked, renderer.textures(), error));
        scene.lighting = false;
        for (float x : {0.f, 4.f, 0.f}) {
            camera.position[0] = x;
            input.camera.position = camera.position;
            input.camera.view = camera.view();
            test::gpu_draw_reference(true);
            renderer.render(input, {frame.id(), 128, 128});
            const auto cpu_reference = frame.read_rgba();
            const auto cpu_draws = renderer.last_stats().draw_calls;
            const auto cpu_triangles = renderer.last_stats().triangles;
            test::gpu_draw_reference(false);
            CHECK(test::gpu_draw_active());
            renderer.render(input, {frame.id(), 128, 128});
            CHECK(frame.read_rgba() == cpu_reference);
#if defined(OPENRAC_VULKAN)
            const auto completed = vkgl::gpu_completed_draws();
            CHECK(completed.draws == static_cast<std::uint64_t>(cpu_draws));
            CHECK(completed.triangles == cpu_triangles);
#endif
        }
#if defined(OPENRAC_VULKAN)
        // Counters are asynchronous: recording a frame must not read or wait for
        // its result. Slot retirement returns the prior frame, including zeros.
        std::array<std::uint64_t, 2> expected_draws{}, expected_triangles{};
        test::gpu_draw_reference(true);
        for (unsigned n = 0; n < 2; ++n) {
            camera.position[0] = n ? 50000.f : 0.f;
            input.camera.position = camera.position;
            input.camera.view = camera.view();
            renderer.render(input, {frame.id(), 128, 128});
            expected_draws[n] = renderer.last_stats().draw_calls;
            expected_triangles[n] = renderer.last_stats().triangles;
            frame.read_rgba();
        }
        test::gpu_draw_reference(false);
        for (unsigned n = 0; n < 8; ++n) {
            const auto before = vkgl::gpu_completed_draws();
            camera.position[0] = n % 2 ? 50000.f : 0.f;
            input.camera.position = camera.position;
            input.camera.view = camera.view();
            renderer.render(input, {frame.id(), 128, 128});
            CHECK(vkgl::gpu_completed_draws().recording == before.recording);
            vkgl::present();  // only the existing frame-slot fence may wait
            const auto completed = vkgl::gpu_completed_draws();
            CHECK(completed.recording >= before.recording);
            if (n > 0) {
                CHECK(completed.draws == expected_draws[(n - 1) % 2]);
                CHECK(completed.triangles == expected_triangles[(n - 1) % 2]);
            }
        }
        // Synchronous pixel readback completes a newer, empty recording. An
        // older in-flight slot retiring afterward must not rewind that result.
        frame.read_rgba();
        const auto empty = vkgl::gpu_completed_draws();
        CHECK(empty.draws == 0 && empty.triangles == 0 && empty.visible_instances == 0);
        vkgl::present();
        CHECK(vkgl::gpu_completed_draws().recording >= empty.recording);
        CHECK(vkgl::gpu_completed_draws().draws == 0);
        // With no statistics client, rendering still matches but no summary
        // dispatch, copy or readback is performed. Re-enabling cannot expose
        // an old staging-buffer payload from an uncollected recording.
        stats::set_enabled(stats::Client::Overlay, false);
        CHECK(vkgl::gpu_completed_draws().recording == 0);
        const auto no_stats = vkgl::submission_counters();
        camera.position[0] = 0;
        input.camera.position = camera.position;
        input.camera.view = camera.view();
        renderer.render(input, {frame.id(), 128, 128});
        const auto without_stats = frame.read_rgba();
        const auto disabled_work = vkgl::submission_counters();
        CHECK(disabled_work.gpu_counter_dispatches == no_stats.gpu_counter_dispatches);
        CHECK(disabled_work.gpu_counter_copies == no_stats.gpu_counter_copies);
        stats::set_enabled(stats::Client::Overlay, true);
        CHECK(vkgl::gpu_completed_draws().recording == 0);
        // Retiring a previously collected slot cannot revive pre-disable data.
        vkgl::present();
        CHECK(vkgl::gpu_completed_draws().recording == 0);
        renderer.render(input, {frame.id(), 128, 128});
        CHECK(frame.read_rgba() == without_stats);
        const auto enabled_work = vkgl::submission_counters();
        CHECK(enabled_work.gpu_counter_dispatches > disabled_work.gpu_counter_dispatches);
        CHECK(enabled_work.gpu_counter_copies > disabled_work.gpu_counter_copies);
        CHECK(vkgl::gpu_completed_draws().draws == expected_draws[0]);
        CHECK(vkgl::gpu_completed_draws().readback_bytes == 16);
        // Exceed the 4096-group page limit. Each distinct model has one group;
        // both page summaries must be included, and must survive slot reuse.
        auto paged = baked;
        paged.instances[2].clear();
        paged.instances[4].clear();
        const auto model = paged.models[second_tie.model];
        for (unsigned n = 0; n < 4200; ++n) {
            auto instance = second_tie;
            instance.model = static_cast<unsigned>(paged.models.size());
            instance.occlusion = viewer::Pvs::always;
            instance.matrix = renderer::Mat4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            paged.models.push_back(model);
            paged.instances[2].push_back(instance);
        }
        scene.release();
        CHECK(scene.upload(paged, renderer.textures(), error));
        scene.lighting = false;
        test::gpu_draw_reference(true);
        renderer.render(input, {frame.id(), 128, 128});
        const auto paged_reference = frame.read_rgba();
        const auto paged_draws = renderer.last_stats().draw_calls;
        const auto paged_triangles = renderer.last_stats().triangles;
        test::gpu_draw_reference(false);
        for (unsigned n = 0; n < 3; ++n) {
            renderer.render(input, {frame.id(), 128, 128});
            CHECK(frame.read_rgba() == paged_reference);
            const auto completed = vkgl::gpu_completed_draws();
            CHECK(completed.draws == std::uint64_t(paged_draws));
            CHECK(completed.triangles == paged_triangles);
            CHECK(completed.readback_bytes == 32);
            vkgl::present();
        }
        stats::set_enabled(stats::Client::Overlay, false);
#endif
        test::gpu_draw_reference(true);
    }

#if defined(OPENRAC_VULKAN)
    if (gl::is_vulkan() && vkgl::gpu_material_counters().supported) {
        // More unique textures than one descriptor page, interleaved repeated
        // materials and different meshes. Compare actual legacy/bindless shaders,
        // not just two instance-culling paths using the same material shader.
        viewer::LevelData many;
        for (const auto& position : std::array<renderer::Vec3, 4>{
                 renderer::Vec3{-.7f, -.7f, 0}, {.7f, -.7f, 0}, {.7f, .7f, 0}, {-.7f, .7f, 0}
             }) {
            viewer::Vertex vertex{};
            std::copy(position.begin(), position.end(), vertex.position);
            vertex.normal[2] = 1;
            vertex.light_slot = -1;
            std::fill_n(vertex.colour, 4, 1.f);
            many.vertices.push_back(vertex);
        }
        many.indices = {0, 1, 2, 0, 2, 3};
        for (unsigned i = 0; i < 1050; ++i) {
            renderer::Rgba8Image image(1, 1);
            image.set_texel(
                0,
                0,
                0xff000000u | ((i * 37u + 23u) & 255u) | (((i * 71u + 51u) & 255u) << 8)
                    | (((i * 101u + 19u) & 255u) << 16)
            );
            many.images.push_back(std::move(image));
            many.image_paths.push_back("material_fixture_" + std::to_string(i));
            viewer::Material material;
            material.image = static_cast<int>(i);
            material.colour = {.5f + float(i % 5) * .1f, .7f, .9f, 1.f};
            many.materials.push_back(material);
            for (unsigned copy = 0; copy < (i % 32 == 0 ? 2u : 1u); ++copy) {
                viewer::Model model;
                model.min = {-.7f, -.7f, 0};
                model.max = {.7f, .7f, 0};
                model.primitives.push_back({0, 6, static_cast<int>(i)});
                viewer::Instance instance;
                instance.model = static_cast<std::uint32_t>(many.models.size());
                const auto n = many.instances[2].size();
                instance.matrix = renderer::translation(
                    {float(n % 34) * 2.f - 33.f, float(n / 34) * 2.f - 31.f, 0}
                );
                many.models.push_back(std::move(model));
                many.instances[2].push_back(instance);
            }
        }
        test::gpu_draw_reference(false);
        camera.position = {0, 0, 100};
        camera.pitch = -1.57f;
        input.camera.position = camera.position;
        input.camera.view = camera.view();
        std::vector<std::uint8_t> legacy;
        std::uint64_t legacy_calls = 0;
        for (bool reference : {true, false}) {
            test::gpu_material_reference(reference);
            scene.release();
            CHECK(scene.upload(many, renderer.textures(), error));
            scene.lighting = false;
            const auto before = vkgl::gpu_material_counters();
            renderer.render(input, {frame.id(), 128, 128});
            const auto pixels = frame.read_rgba();
            const auto after = vkgl::gpu_material_counters();
            if (reference) {
                legacy = pixels;
                std::vector<std::uint32_t> colours;
                for (std::size_t i = 0; i < pixels.size(); i += 4) {
                    colours.push_back(
                        std::uint32_t(pixels[i]) | (std::uint32_t(pixels[i + 1]) << 8)
                        | (std::uint32_t(pixels[i + 2]) << 16)
                    );
                }
                std::sort(colours.begin(), colours.end());
                CHECK(std::unique(colours.begin(), colours.end()) - colours.begin() > 64);
                legacy_calls = after.native_draws - before.native_draws;
                CHECK(after.draws == before.draws);
            } else {
                CHECK(pixels == legacy);
                CHECK(after.draws - before.draws > 1000);
                CHECK(after.records - before.records < after.draws - before.draws);
                CHECK(after.textures - before.textures > 1024);  // descriptor page rollover
                CHECK(after.native_draws - before.native_draws < legacy_calls / 4);
                std::printf(
                    "bindless materials: %llu -> %llu native calls; %llu draws, "
                    "%llu unique records, %llu texture/sampler slots\n",
                    static_cast<unsigned long long>(legacy_calls),
                    static_cast<unsigned long long>(after.native_draws - before.native_draws),
                    static_cast<unsigned long long>(after.draws - before.draws),
                    static_cast<unsigned long long>(after.records - before.records),
                    static_cast<unsigned long long>(after.textures - before.textures)
                );
                // Readback starts a fresh recording; retired tables must be reusable.
                renderer.render(input, {frame.id(), 128, 128});
                CHECK(frame.read_rgba() == legacy);
            }
        }
        test::gpu_draw_reference(true);
    }
#endif

    frame.release();
    scene.release();
    renderer.release();
    return openrac::test::result();
}

// A mod's folder for the level (the --mods folder's level_03, editor/overrides.py): an upscaled
// terrain texture and a bigger tie mesh are read in place of the level's own, and the mod's mesh
// names a texture of the level it does not carry (the tie's), which comes from the level.
void overrides(const fs::path& dir) {
    const fs::path mods = dir.parent_path() / (dir.filename().string() + "_mods") / "level_03";
    fs::remove_all(mods.parent_path());
    fs::create_directories(mods / "textures");
    std::vector<std::uint8_t> green(8 * 8 * 4);
    for (std::size_t i = 0; i < green.size(); i += 4) {
        green[i + 1] = 200;
        green[i + 3] = 255;
    }
    stbi_write_png((mods / "textures" / "terrain_0000.png").string().c_str(), 8, 8, 4, green.data(), 8 * 4);
    write_square(mods / "ties" / "tie_5.gltf", 6.0f, 0.0f, "../textures/tie_0001.png");

    viewer::LevelData level;
    std::string error;
    CHECK(viewer::load_level(dir, level, error, mods / "placements.json"));
    CHECK(level.overridden_models == 1);
    CHECK(level.overridden_images == 1);
    CHECK(level.missing_images == 0);
    CHECK(level.images.size() == 2);
    // The tie's mesh is the mod's: 6 units across.
    const auto& tie = level.instances[static_cast<std::size_t>(viewer::Layer::Ties)][0];
    CHECK(std::fabs(level.models[tie.model].max[0] - 3.0f) < 1e-6f);
    // The terrain's texture is the mod's 8 x 8 green one; the tie's the level's blue one.
    CHECK(level.images[0].width == 8 && ((level.images[0].texel(0, 0) >> 8) & 0xFF) == 200);
    CHECK(level.images[1].width == 2 && ((level.images[1].texel(1, 1) >> 16) & 0xFF) == 220);

    // Without the mod's folder, the level's own.
    viewer::LevelData plain;
    CHECK(viewer::load_level(dir, plain, error));
    CHECK(plain.overridden_models == 0 && plain.overridden_images == 0 && plain.images[0].width == 2);
    fs::remove_all(mods.parent_path());
}

// A terrain as editor/assets.py imports a denser one: an N x N grid of quads, side `size`,
// centred on the origin at height z, with vertex colours (the game's lighting, 0x80 = 1.0, so up
// to 2) and 32-bit indices.
void write_grid(const fs::path& path, int n, float size, float z, float colour, const std::string& image) {
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<float> colours;
    std::vector<std::uint32_t> indices;
    const auto side = static_cast<std::uint32_t>(n + 1);
    for (std::uint32_t j = 0; j < side; ++j) {
        for (std::uint32_t i = 0; i < side; ++i) {
            const float u = static_cast<float>(i) / static_cast<float>(n);
            const float v = static_cast<float>(j) / static_cast<float>(n);
            positions.insert(positions.end(), {(u - 0.5f) * size, (v - 0.5f) * size, z});
            normals.insert(normals.end(), {0.0f, 0.0f, 1.0f});
            uvs.insert(uvs.end(), {u, 1.0f - v});
            colours.insert(colours.end(), {colour, colour, colour, 1.0f});
        }
    }
    for (std::uint32_t j = 0; j + 1 < side; ++j) {
        for (std::uint32_t i = 0; i + 1 < side; ++i) {
            const std::uint32_t a = j * side + i;
            indices.insert(indices.end(), {a, a + 1, a + side, a + 1, a + side + 1, a + side});
        }
    }
    std::vector<std::uint8_t> buffer;
    std::vector<std::size_t> offsets;
    auto append = [&](const void* data, std::size_t bytes) {
        offsets.push_back(buffer.size());
        const auto* p = static_cast<const std::uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + bytes);
    };
    append(positions.data(), positions.size() * sizeof(float));
    append(normals.data(), normals.size() * sizeof(float));
    append(uvs.data(), uvs.size() * sizeof(float));
    append(colours.data(), colours.size() * sizeof(float));
    append(indices.data(), indices.size() * sizeof(std::uint32_t));
    offsets.push_back(buffer.size());
    const std::string count = std::to_string(side * side);
    const float h = size / 2;
    std::string views;
    for (std::size_t k = 0; k + 1 < offsets.size(); ++k) {
        views += (k != 0 ? "," : "") + std::string(R"({"buffer":0,"byteOffset":)") + std::to_string(offsets[k])
                 + R"(,"byteLength":)" + std::to_string(offsets[k + 1] - offsets[k]) + "}";
    }
    const std::string gltf =
        std::string(R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],)")
        + R"("nodes":[{"name":"Terrain_000","mesh":0}],)"
        + R"("meshes":[{"primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TEXCOORD_0":2,"COLOR_0":3},"indices":4,"material":0}]}],)"
        + R"("materials":[{"name":"terrain_0000","pbrMetallicRoughness":{"baseColorTexture":{"index":0}},"doubleSided":true}],)"
        + R"("textures":[{"source":0}],"images":[{"uri":")" + image + R"("}],)" + R"("accessors":[)"
        + R"({"bufferView":0,"componentType":5126,"count":)" + count + R"(,"type":"VEC3","min":[)"
        + std::to_string(-h) + "," + std::to_string(-h) + "," + std::to_string(z) + R"(],"max":[)"
        + std::to_string(h) + "," + std::to_string(h) + "," + std::to_string(z) + "]},"
        + R"({"bufferView":1,"componentType":5126,"count":)" + count + R"(,"type":"VEC3"},)"
        + R"({"bufferView":2,"componentType":5126,"count":)" + count + R"(,"type":"VEC2"},)"
        + R"({"bufferView":3,"componentType":5126,"count":)" + count + R"(,"type":"VEC4"},)"
        + R"({"bufferView":4,"componentType":5125,"count":)" + std::to_string(indices.size()) + R"(,"type":"SCALAR"}],)"
        + R"("bufferViews":[)" + views + "],"
        + R"("buffers":[{"byteLength":)" + std::to_string(buffer.size())
        + R"(,"uri":"data:application/octet-stream;base64,)" + base64(buffer) + R"("}]})";
    write_text(path, gltf);
}

// A mod's terrain (editor/overrides.py: level_NN/terrain.glb, as the manifest names it), denser than
// the level's: drawn in its place whatever its vertex count, lit by its vertex colours, naming the
// level's terrain texture it does not carry.
void denser_terrain(const fs::path& dir) {
    const fs::path mods = dir.parent_path() / (dir.filename().string() + "_terrain_mods") / "level_03";
    fs::remove_all(mods.parent_path());
    constexpr int kQuads = 300;  // 90601 vertices, more than 16-bit indices hold
    write_grid(mods / "terrain.glb.gltf", kQuads, 20.0f, 0.5f, 1.5f, "textures/terrain_0000.png");

    viewer::LevelData level;
    std::string error;
    CHECK(viewer::load_level(dir, level, error, mods / "placements.json"));
    CHECK(level.overridden_models == 1);
    CHECK(level.overridden_images == 0);  // the texture is the level's
    CHECK(level.missing_images == 0);
    const auto& terrain = level.instances[static_cast<std::size_t>(viewer::Layer::Terrain)];
    CHECK(terrain.size() == 1);
    const viewer::Model& model = level.models[terrain[0].model];
    CHECK(model.primitives.size() == 1);
    CHECK(model.primitives[0].index_count == 6u * kQuads * kQuads);
    CHECK(std::fabs(model.min[2] - 0.5f) < 1e-6f && std::fabs(model.max[0] - 10.0f) < 1e-4f);
    // The mod's grid in place of the level's square, then the tie (4 vertices) and the box (24).
    CHECK(level.vertices.size() == static_cast<std::size_t>((kQuads + 1) * (kQuads + 1)) + 4 + 24);
    std::uint32_t highest = 0;
    for (std::uint32_t i = 0; i < model.primitives[0].index_count; ++i) {
        highest = std::max(highest, level.indices[model.primitives[0].first_index + i]);
    }
    CHECK(highest == static_cast<std::uint32_t>((kQuads + 1) * (kQuads + 1)) - 1);
    CHECK(std::fabs(level.vertices[highest].colour[0] - 1.5f) < 1e-6f && level.vertices[highest].light_slot == -1.0f);
    CHECK(level.images.size() == 2 && level.images[0].width == 2);
    fs::remove_all(mods.parent_path());
}

// A tie texture part-clear all over with no fully clear texel (Novalis's windows: glass at alpha
// 0x1F in an opaque frame), which the export leaves OPAQUE, takes the cutout passes as the GS draws
// it (blended); the opaque terrain's material does not.
void translucent_texels(const fs::path& dir) {
    const fs::path glass = dir.parent_path() / (dir.filename().string() + "_glass");
    write_level(glass);
    const std::uint8_t pane[] = {30, 30, 220, 255, 30, 30, 220, 62, 30, 30, 220, 62, 30, 30, 220, 62};
    stbi_write_png((glass / "textures" / "tie_0001.png").string().c_str(), 2, 2, 4, pane, 8);

    viewer::LevelData level;
    std::string error;
    CHECK(viewer::load_level(glass, level, error));
    CHECK(level.translucent_images.size() == level.images.size());
    const auto material_of = [&](viewer::Layer layer) {
        const auto& instance = level.instances[static_cast<std::size_t>(layer)][0];
        const int m = level.models[instance.model].primitives[0].material;
        return level.materials[static_cast<std::size_t>(m)];
    };
    CHECK(!material_of(viewer::Layer::Terrain).cutout);
    CHECK(material_of(viewer::Layer::Ties).cutout);
    fs::remove_all(glass);
}


// MobyProc's distance cull as the port replays it: a moby whose sphere is past its draw distance
// along the view, or wholly behind the eye, is not drawn; one with no sphere yet is.
void moby_draw_distance() {
    std::uint8_t moby[0x100] = {};
    const auto place = [&](float x, float y, float z, float r, std::int16_t distance) {
        const float sphere[4] = {x * 1024.0f, y * 1024.0f, z * 1024.0f, r * 1024.0f};
        std::memcpy(moby, sphere, sizeof(sphere));
        std::memcpy(moby + 0x32, &distance, sizeof(distance));
    };
    const renderer::Vec3 eye{100.0f, 0.0f, 0.0f};
    const renderer::Vec3 forward{1.0f, 0.0f, 0.0f};
    place(150.0f, 30.0f, 0.0f, 1.0f, 64);  // 50 deep, off to the side: drawn
    CHECK(!game::rac1::moby_beyond_draw_distance(moby, eye, forward));
    place(170.0f, 0.0f, 0.0f, 1.0f, 64);  // 70 deep, past 64
    CHECK(game::rac1::moby_beyond_draw_distance(moby, eye, forward));
    place(90.0f, 0.0f, 0.0f, 1.0f, 64);  // behind the eye
    CHECK(game::rac1::moby_beyond_draw_distance(moby, eye, forward));
    place(99.5f, 0.0f, 0.0f, 1.0f, 64);  // the eye inside its sphere
    CHECK(!game::rac1::moby_beyond_draw_distance(moby, eye, forward));
    // Pokitaru's class 1161 placed with a turn of -5 pi: its sphere thousands of units away.
    place(9286.5f, 5217.6f, 235.1f, 1.08f, 64);
    CHECK(
        game::rac1::moby_beyond_draw_distance(moby, {550.0f, 530.0f, 230.0f}, {0.7f, 0.7f, 0.0f})
    );
    place(0.0f, 0.0f, 0.0f, 0.0f, 64);  // no sphere yet
    CHECK(!game::rac1::moby_beyond_draw_distance(moby, eye, forward));
}

// The authored sphere is world-space and packed independently of the model's scale.
void moby_world_sphere() {
    std::vector<std::uint8_t> ram(0x200);
    constexpr unsigned address = 0x100;
    const float scale = 2;
    std::memcpy(ram.data() + address + 0x2c, &scale, sizeof(scale));
    const auto set = [&](std::array<float, 4> sphere) {
        std::memcpy(ram.data() + address, sphere.data(), sizeof(sphere));
        return game::rac1::read_live_moby(ram, address);
    };
    auto live = set({10240, -20480, 30720, 4096});
    CHECK(live.has_value());
    CHECK((live->world_sphere == std::array<float, 4>{10, -20, 30, 4}));
    CHECK(live->matrix[0] == 2);
    for (auto sphere :
         {std::array<float, 4>{0, 0, 0, 0},
          std::array<float, 4>{1024, 2048, 3072, -4096},
          std::array<float, 4>{NAN, 0, 0, 1024},
          std::array<float, 4>{0, 0, 0, INFINITY}}) {
        live = set(sphere);
        CHECK(live.has_value());
        CHECK((live->world_sphere == std::array<float, 4>{}));
    }
}
}  // namespace

int main(int argc, char** argv) {
    test::gpu_draw_reference(true);
    const fs::path dir =
        argc > 1 ? fs::path(argv[1]) : fs::temp_directory_path() / "openrac_viewer_test";
    json_reader();
    pvs_lookup();
    moby_draw_distance();
    moby_world_sphere();
    write_level(dir);
    viewer::LevelData level;
    loader(dir, level);
    overrides(dir);
    denser_terrain(dir);
    translucent_texels(dir);
    pvs_loader(dir);
    if (openrac::test::failures() != 0) {
        return openrac::test::result();
    }
    return draw(level);
}

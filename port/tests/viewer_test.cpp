// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The level viewer on a synthetic level written by the test itself, in the
// shape the editor's port export writes (manifest.json, placements.json,
// glTF meshes, PNG textures): the JSON reader, the loader, and, where an
// OpenGL context can be made, one frame drawn and its pixels checked.
//
//   viewer_test DIR    writes the level into DIR (replacing it)

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
#include "renderer/framebuffer.h"
#include "renderer/renderer.h"
#include "renderer_gl_context.h"
#include "viewer/camera.h"
#include "viewer/game_state.h"
#include "viewer/json.h"
#include "viewer/level.h"
#include "viewer/level_renderer.h"

namespace {

namespace fs = std::filesystem;
using namespace openrac;

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
        std::string(R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],)")
        + R"("nodes":[{"name":"square","mesh":0}],)"
        + R"("meshes":[{"primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TEXCOORD_0":2},"indices":3,"material":0}]}],)"
        + R"("materials":[{"pbrMetallicRoughness":{"baseColorTexture":{"index":0}},"doubleSided":true}],)"
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
    write_square(dir / "terrain.glb.gltf", 20.0f, 0.0f, "textures/terrain_0000.png");
    write_square(dir / "ties" / "tie_5.gltf", 2.0f, 0.0f, "../textures/tie_0001.png");
    write_text(dir / "manifest.json", R"({
  "format": 3, "game": "rac1", "version": "pal", "level": 3,
  "terrain": "terrain.glb.gltf",
  "sky": {"background": [10, 20, 30], "mesh": null},
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
    using viewer::json::Value;
    Value v;
    std::string error;
    CHECK(viewer::json::parse(
        R"({"a": [1, 2.5e1, -3], "b": {"c": "xé\n"}, "t": true, "n": null})", v, error
    ));
    CHECK(v["a"].items().size() == 3);
    CHECK(v["a"][1].number() == 25.0);
    CHECK(v["a"][2].number() == -3.0);
    CHECK(v["b"]["c"].string() == "x\xc3\xa9\n");
    CHECK(v["t"].boolean());
    CHECK(v["n"].is_null());
    CHECK(v["missing"]["deeper"].is_null());
    CHECK(!viewer::json::parse("{\"a\": }", v, error));
    CHECK(!error.empty());
    CHECK(!viewer::json::parse("[1, 2", v, error));
    CHECK(!viewer::json::parse("[1] x", v, error));
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
    // The images: red terrain, blue tie.
    CHECK((level.images[0].texel(0, 0) & 0xFF) == 220);
    CHECK(((level.images[1].texel(1, 1) >> 16) & 0xFF) == 220);
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
    CHECK(!viewer::moby_beyond_draw_distance(moby, eye, forward));
    place(170.0f, 0.0f, 0.0f, 1.0f, 64);  // 70 deep, past 64
    CHECK(viewer::moby_beyond_draw_distance(moby, eye, forward));
    place(90.0f, 0.0f, 0.0f, 1.0f, 64);  // behind the eye
    CHECK(viewer::moby_beyond_draw_distance(moby, eye, forward));
    place(99.5f, 0.0f, 0.0f, 1.0f, 64);  // the eye inside its sphere
    CHECK(!viewer::moby_beyond_draw_distance(moby, eye, forward));
    // Pokitaru's class 1161 placed with a turn of -5 pi: its sphere thousands of units away.
    place(9286.5f, 5217.6f, 235.1f, 1.08f, 64);
    CHECK(viewer::moby_beyond_draw_distance(moby, {550.0f, 530.0f, 230.0f}, {0.7f, 0.7f, 0.0f}));
    place(0.0f, 0.0f, 0.0f, 0.0f, 64);  // no sphere yet
    CHECK(!viewer::moby_beyond_draw_distance(moby, eye, forward));
}
}  // namespace

int main(int argc, char** argv) {
    const fs::path dir =
        argc > 1 ? fs::path(argv[1]) : fs::temp_directory_path() / "openrac_viewer_test";
    json_reader();
    moby_draw_distance();
    write_level(dir);
    viewer::LevelData level;
    loader(dir, level);
    overrides(dir);
    translucent_texels(dir);
    if (openrac::test::failures() != 0) {
        return openrac::test::result();
    }
    return draw(level);
}

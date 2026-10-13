// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// One level as the editor's port export writes it (editor/port.py):
//
//   manifest.json      format 1: the level, its files, the classes
//   placements.json    ties, shrubs and mobys: class, 4 x 4 matrix (game
//                      axes, column-major) and the fields the game stores
//   terrain.glb        the terrain fragments
//   ties/, shrubs/, mobys/   one GLB per class (mobys in their bind pose)
//   sky.glb            the sky shells, in drawing order (vertex colours)
//   textures/*.png     shared by every mesh
//
// Loading is all CPU work (no GL), so it is tested without a context: every
// mesh goes into one vertex and index array, every PNG into an RGBA8 image.

#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "renderer/math.h"
#include "renderer/texture.h"

namespace openrac::viewer {

using renderer::Mat4;
using renderer::Vec3;

inline constexpr int kManifestFormat = 1;

struct Vertex {
    float position[3];
    float normal[3];
    float uv[2];
    float colour[4];
    // A moby vertex's skin: up to four joints of its class and their weights (summing to 1; all
    // 0 for a vertex that is not skinned). The joints are the game's joint numbers.
    std::uint8_t joints[4];
    float weights[4];
    // A tie vertex's light slot (0..63): which of its instance's 64 lit colours it takes; -1
    // for a vertex that keeps its own colour.
    float light_slot;
};

struct Material {
    int image = -1;       // index into LevelData::images; -1 draws white
    bool cutout = false;  // glTF alphaMode MASK: alpha test at 0.5
    bool blend = false;   // glTF alphaMode BLEND
    std::array<float, 4> colour{1.0f, 1.0f, 1.0f, 1.0f};
};

struct Primitive {
    std::uint32_t first_index = 0;
    std::uint32_t index_count = 0;
    int material = 0;
};

struct Model {
    std::string name;
    std::vector<Primitive> primitives;
    Vec3 min{};
    Vec3 max{};
    int joints = 0;  // a skinned mesh: one past the highest joint its vertices use
};

struct Instance {
    std::uint32_t model = 0;
    Mat4 matrix = renderer::identity();
    std::array<float, 4> tint{1.0f, 1.0f, 1.0f, 1.0f};
    // A live moby's joint palette: the first of its matrices in the scene's palette
    // (LevelScene::set_palette); -1 draws the mesh as stored (its bind pose).
    int palette = -1;
    // A lit tie or shrub: the first of its 64 (24) colours in LevelData::light_colours; -1 none.
    int lights = -1;
    // A live moby lit as the game lights it: its light word (set 0, set 1, the cross-fade
    // 0..1; w = 1 to light it) and its ambient colour (RGB bytes / 128).
    std::array<float, 4> moby_light{0.0f, 0.0f, 0.0f, 0.0f};
    std::array<float, 4> moby_ambient{0.5f, 0.5f, 0.5f, 1.0f};
};

// What the viewer draws, in the game's order.
enum class Layer : std::uint8_t {
    Sky,
    Terrain,
    Ties,
    Shrubs,
    Mobys,
    Count
};

inline constexpr std::size_t kLayerCount = static_cast<std::size_t>(Layer::Count);

const char* layer_name(Layer layer);

struct LevelData {
    std::filesystem::path dir;
    int level = -1;
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<Model> models;
    std::vector<Material> materials;
    std::vector<renderer::Rgba8Image> images;
    std::vector<std::string> image_paths;
    std::array<std::vector<Instance>, kLayerCount> instances;
    // Moby classes: model and whether it is the stand-in box, for placing live mobys.
    std::map<int, std::pair<std::uint32_t, bool>> moby_classes;
    // The ties' and shrubs' lit colours, 64 or 24 per instance, RGBA bytes (0x80 = 1.0), from
    // placements.json.
    std::vector<std::uint32_t> light_colours;
    // The level's 16 directional light sets (manifest "lights"): per set colour A (w = back
    // factor), direction A, colour B, direction B; empty when the export has none.
    std::vector<float> light_sets;
    std::array<float, 3> background{0.0f, 0.0f, 0.0f};  // where no sky shell covers
    Vec3 bounds_min{};
    Vec3 bounds_max{};
    int missing_images = 0;
};

// Load a level directory. Returns false with a reason if the manifest or a
// mesh cannot be read; a missing texture only counts in missing_images.
// `edited`, when it names a file, is a placements file from a mod (the editor's
// pack): the lists it has ("ties", "shrubs") stand for the level's own.
bool load_level(
    const std::filesystem::path& dir, LevelData& out, std::string& error,
    const std::filesystem::path& edited = {}
);

// The level's sky textures (textures/sky_NNNN.png, by the sky's own texture numbers), including the
// ones only its sprites use; an empty image where one is missing.
std::vector<renderer::Rgba8Image> load_sky_textures(const std::filesystem::path& dir);

// Load one glTF or GLB file as one model (every mesh node, in its node's
// transform, except skinned meshes, which are stored in their bind pose with
// their skin: the joints by the number in their node's name, joint_NNN, the
// game's joint numbers). Returns the model's index.
std::optional<std::uint32_t> load_model(
    const std::filesystem::path& path, LevelData& level, std::string& error
);

// A unit box, 0.5 on a side, standing on its base: what the viewer draws for
// a moby class without a mesh.
std::uint32_t add_box_model(LevelData& level);

}  // namespace openrac::viewer

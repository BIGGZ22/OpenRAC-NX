// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "viewer/level.h"

#include <algorithm>
#include <cgltf.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <format>
#include <map>
#include <stb_image.h>

#include "common/log.h"
#include "viewer/json.h"

namespace openrac::viewer {

namespace fs = std::filesystem;

const char* layer_name(Layer layer) {
    switch (layer) {
        case Layer::Sky:
            return "sky";
        case Layer::Terrain:
            return "terrain";
        case Layer::Ties:
            return "ties";
        case Layer::Shrubs:
            return "shrubs";
        case Layer::Mobys:
            return "mobys";
        case Layer::Count:
            break;
    }
    return "?";
}

namespace {

int image_index(const fs::path& path, LevelData& level) {
    const std::string key = path.lexically_normal().generic_string();
    auto it = std::find(level.image_paths.begin(), level.image_paths.end(), key);
    if (it != level.image_paths.end()) {
        const auto index = static_cast<int>(it - level.image_paths.begin());
        return level.images[static_cast<std::size_t>(index)].width > 0 ? index : -1;
    }
    level.image_paths.push_back(key);
    int w = 0;
    int h = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(key.c_str(), &w, &h, &channels, 4);
    renderer::Rgba8Image image;
    if (pixels != nullptr) {
        image = renderer::Rgba8Image(w, h);
        std::copy(pixels, pixels + static_cast<std::ptrdiff_t>(w) * h * 4, image.pixels.begin());
        stbi_image_free(pixels);
    } else {
        log::warn("texture {}: {}", key, stbi_failure_reason());
        ++level.missing_images;
    }
    level.images.push_back(std::move(image));
    return pixels != nullptr ? static_cast<int>(level.images.size() - 1) : -1;
}

int material_index(const cgltf_material* source, const fs::path& base, LevelData& level) {
    Material m;
    if (source != nullptr) {
        m.cutout = source->alpha_mode == cgltf_alpha_mode_mask;
        m.blend = source->alpha_mode == cgltf_alpha_mode_blend;
        if (source->has_pbr_metallic_roughness) {
            const auto& pbr = source->pbr_metallic_roughness;
            std::copy(pbr.base_color_factor, pbr.base_color_factor + 4, m.colour.begin());
            const cgltf_texture* texture = pbr.base_color_texture.texture;
            if (texture != nullptr && texture->image != nullptr && texture->image->uri != nullptr) {
                m.image = image_index(base / texture->image->uri, level);
            }
        }
    }
    for (std::size_t i = 0; i < level.materials.size(); ++i) {
        const Material& o = level.materials[i];
        if (o.image == m.image && o.cutout == m.cutout && o.blend == m.blend
            && o.colour == m.colour) {
            return static_cast<int>(i);
        }
    }
    level.materials.push_back(m);
    return static_cast<int>(level.materials.size() - 1);
}

const cgltf_accessor* attribute(const cgltf_primitive& p, cgltf_attribute_type type) {
    for (cgltf_size i = 0; i < p.attributes_count; ++i) {
        if (p.attributes[i].type == type && p.attributes[i].index == 0) {
            return p.attributes[i].data;
        }
    }
    return nullptr;
}

bool read_matrix(const json::Value& value, Mat4& out) {
    if (value.items().size() != 16) {
        return false;
    }
    for (std::size_t i = 0; i < 16; ++i) {
        out[i] = static_cast<float>(value[i].number());
    }
    return true;
}

// A colour per class number, for boxes: the golden-ratio hue walk the
// editor's Godot export uses for its markers.
std::array<float, 4> class_colour(int class_id) {
    const double hue = std::fmod(class_id * 0.618034, 1.0) * 6.0;
    const double s = 0.65;
    const double v = 0.95;
    const int sector = static_cast<int>(hue) % 6;
    const double f = hue - std::floor(hue);
    const auto p = static_cast<float>(v * (1 - s));
    const auto q = static_cast<float>(v * (1 - s * f));
    const auto t = static_cast<float>(v * (1 - s * (1 - f)));
    const auto vv = static_cast<float>(v);
    switch (sector) {
        case 0:
            return {vv, t, p, 1.0f};
        case 1:
            return {q, vv, p, 1.0f};
        case 2:
            return {p, vv, t, 1.0f};
        case 3:
            return {p, q, vv, 1.0f};
        case 4:
            return {t, p, vv, 1.0f};
        default:
            return {vv, p, q, 1.0f};
    }
}

void grow(Vec3& low, Vec3& high, const Vec3& p) {
    for (int i = 0; i < 3; ++i) {
        low[i] = std::min(low[i], p[i]);
        high[i] = std::max(high[i], p[i]);
    }
}

}  // namespace

std::optional<std::uint32_t> load_model(
    const fs::path& path, LevelData& level, std::string& error
) {
    cgltf_options options{};
    cgltf_data* data = nullptr;
    const std::string name = path.string();
    if (cgltf_parse_file(&options, name.c_str(), &data) != cgltf_result_success) {
        error = std::format("cannot read glTF {}", name);
        return std::nullopt;
    }
    if (cgltf_load_buffers(&options, data, name.c_str()) != cgltf_result_success) {
        cgltf_free(data);
        error = std::format("cannot read the buffers of {}", name);
        return std::nullopt;
    }
    const fs::path base = path.parent_path();
    Model model;
    model.name = path.filename().string();
    constexpr float kHuge = 3.0e38f;
    model.min = {kHuge, kHuge, kHuge};
    model.max = {-kHuge, -kHuge, -kHuge};

    for (cgltf_size n = 0; n < data->nodes_count; ++n) {
        const cgltf_node& node = data->nodes[n];
        if (node.mesh == nullptr) {
            continue;
        }
        // A skinned mesh is stored in its bind pose: its vertices as stored, with their skin.
        Mat4 world = renderer::identity();
        if (node.skin == nullptr) {
            cgltf_node_transform_world(&node, world.data());
        }
        // The skin's joints as the game numbers them (the exporter names them joint_NNN).
        std::vector<std::uint8_t> skin_joints;
        if (node.skin != nullptr) {
            for (cgltf_size j = 0; j < node.skin->joints_count; ++j) {
                const cgltf_node* joint = node.skin->joints[j];
                int number = static_cast<int>(j);
                if (joint != nullptr && joint->name != nullptr
                    && std::strncmp(joint->name, "joint_", 6) == 0) {
                    number = std::atoi(joint->name + 6);
                }
                skin_joints.push_back(static_cast<std::uint8_t>(std::clamp(number, 0, 255)));
            }
        }
        for (cgltf_size p = 0; p < node.mesh->primitives_count; ++p) {
            const cgltf_primitive& prim = node.mesh->primitives[p];
            if (prim.type != cgltf_primitive_type_triangles) {
                continue;
            }
            const cgltf_accessor* positions = attribute(prim, cgltf_attribute_type_position);
            if (positions == nullptr) {
                continue;
            }
            const cgltf_accessor* normals = attribute(prim, cgltf_attribute_type_normal);
            const cgltf_accessor* uvs = attribute(prim, cgltf_attribute_type_texcoord);
            const cgltf_accessor* colours = attribute(prim, cgltf_attribute_type_color);
            const cgltf_accessor* light_slots = nullptr;
            for (cgltf_size a = 0; a < prim.attributes_count; ++a) {
                if (prim.attributes[a].name != nullptr
                    && std::strcmp(prim.attributes[a].name, "_LIGHT_SLOT") == 0) {
                    light_slots = prim.attributes[a].data;
                }
            }
            const cgltf_accessor* joints =
                skin_joints.empty() ? nullptr : attribute(prim, cgltf_attribute_type_joints);
            const cgltf_accessor* weights =
                skin_joints.empty() ? nullptr : attribute(prim, cgltf_attribute_type_weights);
            const auto base_vertex = static_cast<std::uint32_t>(level.vertices.size());
            for (cgltf_size i = 0; i < positions->count; ++i) {
                Vertex v{};
                Vec3 position{};
                cgltf_accessor_read_float(positions, i, position.data(), 3);
                position = renderer::transform_point(world, position);
                std::copy(position.begin(), position.end(), v.position);
                grow(model.min, model.max, position);
                Vec3 normal{0.0f, 0.0f, 1.0f};
                if (normals != nullptr) {
                    cgltf_accessor_read_float(normals, i, normal.data(), 3);
                    Mat4 rotation = world;
                    rotation[12] = rotation[13] = rotation[14] = 0.0f;
                    normal = renderer::normalize(renderer::transform_point(rotation, normal));
                }
                std::copy(normal.begin(), normal.end(), v.normal);
                if (uvs != nullptr) {
                    cgltf_accessor_read_float(uvs, i, v.uv, 2);
                }
                v.colour[0] = v.colour[1] = v.colour[2] = v.colour[3] = 1.0f;
                if (colours != nullptr) {
                    cgltf_accessor_read_float(
                        colours, i, v.colour, colours->type == cgltf_type_vec3 ? 3 : 4
                    );
                }
                v.light_slot = -1.0f;
                if (light_slots != nullptr) {
                    cgltf_accessor_read_float(light_slots, i, &v.light_slot, 1);
                }
                if (joints != nullptr && weights != nullptr) {
                    cgltf_uint js[4] = {0, 0, 0, 0};
                    cgltf_accessor_read_uint(joints, i, js, 4);
                    cgltf_accessor_read_float(weights, i, v.weights, 4);
                    for (int k = 0; k < 4; ++k) {
                        v.joints[k] = js[k] < skin_joints.size() ? skin_joints[js[k]] : 0;
                        if (js[k] >= skin_joints.size()) {
                            v.weights[k] = 0.0f;
                        } else if (v.weights[k] > 0.0f) {
                            model.joints = std::max(model.joints, v.joints[k] + 1);
                        }
                    }
                }
                level.vertices.push_back(v);
            }
            Primitive out;
            out.first_index = static_cast<std::uint32_t>(level.indices.size());
            if (prim.indices != nullptr) {
                for (cgltf_size i = 0; i < prim.indices->count; ++i) {
                    level.indices.push_back(
                        base_vertex
                        + static_cast<std::uint32_t>(cgltf_accessor_read_index(prim.indices, i))
                    );
                }
            } else {
                for (cgltf_size i = 0; i < positions->count; ++i) {
                    level.indices.push_back(base_vertex + static_cast<std::uint32_t>(i));
                }
            }
            out.index_count = static_cast<std::uint32_t>(level.indices.size()) - out.first_index;
            out.material = material_index(prim.material, base, level);
            if (node.name != nullptr && std::strncmp(node.name, "Sky_", 4) == 0) {
                out.shell = std::atoi(node.name + 4);
            }
            model.primitives.push_back(out);
        }
    }
    cgltf_free(data);
    if (model.primitives.empty()) {
        model.min = model.max = Vec3{};
    }
    level.models.push_back(std::move(model));
    return static_cast<std::uint32_t>(level.models.size() - 1);
}

std::uint32_t add_box_model(LevelData& level) {
    Model model;
    model.name = "box";
    model.min = {-0.25f, -0.25f, 0.0f};
    model.max = {0.25f, 0.25f, 0.5f};
    Material plain;
    level.materials.push_back(plain);
    const int material = static_cast<int>(level.materials.size() - 1);
    // Six faces, each its own four corners so every face has its normal.
    const float faces[6][3] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    Primitive prim;
    prim.first_index = static_cast<std::uint32_t>(level.indices.size());
    prim.material = material;
    for (const auto& n : faces) {
        // Two axes across the face.
        const Vec3 normal{n[0], n[1], n[2]};
        const Vec3 a = std::abs(n[2]) > 0.5f ? Vec3{1, 0, 0} : Vec3{0, 0, 1};
        const Vec3 b = renderer::cross(normal, a);
        const auto first = static_cast<std::uint32_t>(level.vertices.size());
        for (int corner = 0; corner < 4; ++corner) {
            const float u = (corner & 1) != 0 ? 1.0f : -1.0f;
            const float w = (corner & 2) != 0 ? 1.0f : -1.0f;
            Vertex v{};
            v.light_slot = -1.0f;
            for (int i = 0; i < 3; ++i) {
                v.position[i] = 0.25f * (normal[i] + u * a[i] + w * b[i]);
                v.normal[i] = normal[i];
            }
            v.position[2] += 0.25f;
            v.uv[0] = (u + 1.0f) / 2.0f;
            v.uv[1] = (w + 1.0f) / 2.0f;
            v.colour[0] = v.colour[1] = v.colour[2] = v.colour[3] = 1.0f;
            level.vertices.push_back(v);
        }
        for (std::uint32_t i : {0u, 1u, 2u, 1u, 3u, 2u}) {
            level.indices.push_back(first + i);
        }
    }
    prim.index_count = static_cast<std::uint32_t>(level.indices.size()) - prim.first_index;
    model.primitives.push_back(prim);
    level.models.push_back(std::move(model));
    return static_cast<std::uint32_t>(level.models.size() - 1);
}

std::vector<renderer::Rgba8Image> load_sky_textures(const fs::path& dir) {
    std::vector<renderer::Rgba8Image> out;
    for (int i = 0; i < 64; ++i) {
        const fs::path path = dir / "textures" / std::format("sky_{:04d}.png", i);
        if (!fs::exists(path)) {
            break;
        }
        int w = 0;
        int h = 0;
        int channels = 0;
        renderer::Rgba8Image image;
        if (unsigned char* pixels = stbi_load(path.string().c_str(), &w, &h, &channels, 4)) {
            image = renderer::Rgba8Image(w, h);
            std::copy(pixels, pixels + static_cast<std::ptrdiff_t>(w) * h * 4, image.pixels.begin());
            stbi_image_free(pixels);
        }
        out.push_back(std::move(image));
    }
    return out;
}

bool load_level(const fs::path& dir, LevelData& out, std::string& error, const fs::path& edited) {
    out = LevelData{};
    out.dir = dir;
    json::Value manifest;
    if (!json::parse_file((dir / "manifest.json").string(), manifest, error)) {
        return false;
    }
    const int format = static_cast<int>(manifest["format"].number(-1));
    if (format != kManifestFormat) {
        error = std::format(
            "{}: manifest format {} (this viewer reads {})", dir.string(), format, kManifestFormat
        );
        return false;
    }
    out.level = static_cast<int>(manifest["level"].number(-1));

    auto model_at = [&](const json::Value& file) -> std::optional<std::uint32_t> {
        if (!file.is_string()) {
            return std::nullopt;
        }
        return load_model(dir / file.string(), out, error);
    };

    for (const json::Value& set : manifest["lights"].items()) {
        for (const json::Value& v : set.items()) {
            for (const json::Value& x : v.items()) {
                out.light_sets.push_back(static_cast<float>(x.number(0)));
            }
        }
    }
    if (out.light_sets.size() != 16 * 16) {
        out.light_sets.clear();
    }
    const json::Value& sky = manifest["sky"];
    if (sky.is_object()) {
        for (std::size_t i = 0; i < 3; ++i) {
            out.background[i] = static_cast<float>(sky["background"][i].number()) / 255.0f;
        }
        if (sky["mesh"].is_string()) {
            auto model = model_at(sky["mesh"]);
            if (!model) {
                return false;
            }
            out.instances[static_cast<std::size_t>(Layer::Sky)].push_back(
                {*model, renderer::identity(), {1, 1, 1, 1}}
            );
        }
    }
    if (manifest["terrain"].is_string()) {
        auto model = model_at(manifest["terrain"]);
        if (!model) {
            return false;
        }
        out.instances[static_cast<std::size_t>(Layer::Terrain)].push_back(
            {*model, renderer::identity(), {1, 1, 1, 1}}
        );
    }

    // Classes: one model each.
    struct ClassModel {
        std::uint32_t model;
        float scale;
        bool box;
    };

    std::map<int, ClassModel> ties;
    std::map<int, ClassModel> shrubs;
    std::map<int, ClassModel> mobys;
    std::optional<std::uint32_t> box;
    const json::Value& classes = manifest["classes"];
    const std::pair<const char*, std::map<int, ClassModel>*> families[] =
        {{"tie", &ties}, {"shrub", &shrubs}, {"moby", &mobys}};
    for (const auto& [family, table] : families) {
        for (const auto& [key, cls] : classes[family].members()) {
            const int id = std::atoi(key.c_str());
            const auto scale = static_cast<float>(cls["scale"].number(1.0));
            if (cls["mesh"].is_string()) {
                auto model = model_at(cls["mesh"]);
                if (!model) {
                    return false;
                }
                (*table)[id] = {*model, scale, false};
            } else {
                if (!box) {
                    box = add_box_model(out);
                }
                (*table)[id] = {*box, 1.0f, true};
            }
        }
    }

    for (const auto& [id, cls] : mobys) {
        out.moby_classes[id] = {cls.model, cls.box};
    }

    const std::string placements_file =
        manifest["placements"].is_string() ? manifest["placements"].string() : "placements.json";
    json::Value placements;
    if (!json::parse_file((dir / placements_file).string(), placements, error)) {
        return false;
    }
    // A mod's placements: the lists it has stand for the level's own.
    json::Value edits;
    if (!edited.empty() && fs::exists(edited) && !json::parse_file(edited.string(), edits, error)) {
        return false;
    }
    const std::tuple<const char*, Layer, std::map<int, ClassModel>*> groups[] =
        {{"ties", Layer::Ties, &ties},
         {"shrubs", Layer::Shrubs, &shrubs},
         {"mobys", Layer::Mobys, &mobys}};
    for (const auto& [key, layer, table] : groups) {
        const json::Value& list = edits[key].is_array() ? edits[key] : placements[key];
        if (edits[key].is_array()) {
            log::info("{} {} from {}", list.items().size(), key, edited.string());
        }
        for (const json::Value& p : list.items()) {
            const int id = static_cast<int>(p["class"].number(-1));
            auto cls = table->find(id);
            if (cls == table->end()) {
                // A placement of a class the level does not have; the editor
                // refuses those, so a hand-edited file.
                log::warn("{} placement {} has unknown class {}", key, p["index"].number(-1), id);
                continue;
            }
            Instance instance;
            instance.model = cls->second.model;
            if (!read_matrix(p["matrix"], instance.matrix)) {
                error = std::format(
                    "{}: {} placement {} has no 4 x 4 matrix",
                    placements_file,
                    key,
                    p["index"].number(-1)
                );
                return false;
            }
            if (cls->second.scale != 1.0f) {
                instance.matrix =
                    renderer::multiply(instance.matrix, renderer::scale(cls->second.scale));
            }
            if (cls->second.box) {
                instance.tint = class_colour(id);
            }
            const std::size_t lit = p["colours"].items().size();
            if ((layer == Layer::Ties && lit == 64) || (layer == Layer::Shrubs && lit == 24)) {
                instance.lights = static_cast<int>(out.light_colours.size());
                for (const json::Value& c : p["colours"].items()) {
                    out.light_colours.push_back(static_cast<std::uint32_t>(c.number(0)));
                }
            }
            out.instances[static_cast<std::size_t>(layer)].push_back(instance);
        }
    }

    // Bounds of everything but the sky, in game axes.
    constexpr float kHuge = 3.0e38f;
    out.bounds_min = {kHuge, kHuge, kHuge};
    out.bounds_max = {-kHuge, -kHuge, -kHuge};
    for (std::size_t layer = static_cast<std::size_t>(Layer::Terrain); layer < kLayerCount;
         ++layer) {
        for (const Instance& instance : out.instances[layer]) {
            const Model& m = out.models[instance.model];
            if (m.primitives.empty()) {
                continue;
            }
            for (int corner = 0; corner < 8; ++corner) {
                const Vec3 p{
                    (corner & 1) != 0 ? m.max[0] : m.min[0],
                    (corner & 2) != 0 ? m.max[1] : m.min[1],
                    (corner & 4) != 0 ? m.max[2] : m.min[2]
                };
                grow(out.bounds_min, out.bounds_max, renderer::transform_point(instance.matrix, p));
            }
        }
    }
    if (out.bounds_min[0] > out.bounds_max[0]) {
        out.bounds_min = out.bounds_max = Vec3{};
    }
    return true;
}

}  // namespace openrac::viewer

// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/tools/probes/level_scene.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <numbers>
#include <stdexcept>

#include "assets/disc/wad.h"
#include "enhanced/tools/probes/liquids.h"
#include "viewer/json.h"

namespace openrac::lighting {
namespace {
// Stable FNV-1a over canonical scalar bytes; a cache identity, not a security
// digest. Bump the domain when scene or solver interpretation changes.
struct Hash {
    std::uint64_t value = 14695981039346656037ull;

    void byte(std::uint8_t b) { value = (value ^ b) * 1099511628211ull; }

    void u32(std::uint32_t n) {
        for (unsigned i = 0; i < 4; ++i) {
            byte(static_cast<std::uint8_t>(n >> (8 * i)));
        }
    }

    void f32(float n) { u32(std::bit_cast<std::uint32_t>(n)); }

    template <std::size_t N>
    void floats(const std::array<float, N>& v) {
        for (float n : v) {
            f32(n);
        }
    }
};

Float3 as_float3(const float* v) {
    return {v[0], v[1], v[2]};
}

void source_files(Hash& hash, const std::filesystem::path& root, const char* label) {
    namespace fs = std::filesystem;
    for (const char* p = label; *p; ++p) {
        hash.byte(static_cast<std::uint8_t>(*p));
    }
    hash.byte(0);
    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        // PVS and offline reference selection do not change light transport.
        // Their sidecars must not invalidate otherwise identical probe bakes.
        const auto name = entry.path().lexically_relative(root).generic_string();
        if (name == "pvs.json" || name == "sunlight-calibration.json") {
            continue;
        }
        const auto extension = entry.path().extension().string();
        if (extension == ".glb" || extension == ".png" || extension == ".json"
            || (extension == ".bin"
                && entry.path().lexically_relative(root).generic_string().starts_with("bake-inputs/"
                ))) {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end(), [&](const auto& a, const auto& b) {
        return a.lexically_relative(root).generic_string()
               < b.lexically_relative(root).generic_string();
    });
    hash.u32(static_cast<std::uint32_t>(files.size()));
    std::array<char, 65536> buffer{};
    for (const auto& file : files) {
        const auto relative = file.lexically_relative(root).generic_string();
        for (char c : relative) {
            hash.byte(static_cast<std::uint8_t>(c));
        }
        hash.byte(0);
        const auto size = fs::file_size(file);
        hash.u32(static_cast<std::uint32_t>(size));
        hash.u32(static_cast<std::uint32_t>(size >> 32));
        std::ifstream input(file, std::ios::binary);
        if (!input) {
            throw std::runtime_error("cannot fingerprint probe scene input");
        }
        while (input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()))
               || input.gcount() > 0) {
            for (std::streamsize i = 0; i < input.gcount(); ++i) {
                hash.byte(static_cast<std::uint8_t>(buffer[static_cast<std::size_t>(i)]));
            }
        }
        if (!input.eof()) {
            throw std::runtime_error("cannot read probe scene fingerprint");
        }
    }
}

// The normal extractor retains raw source sections. Keep legacy explicitly
// supplied surfaces authoritative so old exports never emit the lava twice.
bool automatic_liquids(const viewer::LevelData& level) {
    if (level.dir.empty() || level.level != 9) {
        return false;
    }
    viewer::json::Value manifest;
    std::string error;
    if (!viewer::json::parse_file((level.dir / "manifest.json").string(), manifest, error)) {
        throw std::runtime_error(error);
    }
    const auto& source = manifest["bake_inputs"]["liquid"].string();
    if (source.empty()) {
        return false;
    }
    if (source != "rac1-ntsc-gaspar-v1" || manifest["game"].string() != "rac1"
        || manifest["version"].string() != "ntsc") {
        throw std::runtime_error("Unsupported authored liquid bake input");
    }
    for (const auto& instance : level.bake_surfaces) {
        for (const auto& primitive : level.models.at(instance.model).primitives) {
            if (primitive.material < 0) {
                continue;
            }
            const auto image = level.materials.at(primitive.material).image;
            if (image >= 0
                && std::filesystem::path(level.image_paths.at(static_cast<std::size_t>(image)))
                           .filename()
                       == "probe_gaspar_lava.png") {
                return false;
            }
        }
    }
    return true;
}

std::vector<std::uint8_t> liquid_input(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        throw std::runtime_error("Missing authored liquid bake source: " + path.string());
    }
    const auto size = input.tellg();
    if (size <= 0 || size > 256 * 1024 * 1024) {
        throw std::runtime_error("Unsupported authored liquid source size");
    }
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    input.seekg(0);
    if (!input.read(reinterpret_cast<char*>(bytes.data()), size)) {
        throw std::runtime_error("Cannot read authored liquid source");
    }
    return assets::disc::is_wad(bytes) ? assets::disc::wad_decompress(bytes) : bytes;
}
}  // namespace

BakeSceneExport export_probe_scene(
    const viewer::LevelData& level, unsigned light_set, Float3 environment, bool analyze_liquids
) {
    if (light_set >= 16 || level.light_sets.size() != 256) {
        throw std::runtime_error("level has no requested directional light set");
    }
    if (level.missing_images) {
        throw std::runtime_error("bake requires all referenced textures; level has missing images");
    }
    BakeSceneExport out;
    out.bounds_min.fill(std::numeric_limits<float>::max());
    out.bounds_max.fill(std::numeric_limits<float>::lowest());
    out.scene.environment = environment;
    // Reuse the editor's composited static sky shells. Its documented axes
    // are +Z at the top and +Y at longitude zero, toward +X. Do not use this
    // distant environment as blocking geometry or include runtime sun sprites.
    const auto& sky = level.sky_panorama;
    if (sky.width > 0 && sky.height > 0) {
        if (sky.pixels.size()
            != std::size_t{static_cast<unsigned>(sky.width)} * static_cast<unsigned>(sky.height)
                   * 4) {
            throw std::runtime_error("invalid level sky panorama");
        }
        out.scene.sky_width = static_cast<unsigned>(std::min(sky.width, 256));
        out.scene.sky_height = static_cast<unsigned>(std::min(sky.height, 128));
        out.scene.sky_radiance.resize(std::size_t{out.scene.sky_width} * out.scene.sky_height);
        std::array<float, 256> linear{};
        for (unsigned i = 0; i < 256; ++i) {
            const float v = static_cast<float>(i) / 255;
            linear[i] = v <= 0.04045f ? v / 12.92f : std::pow((v + 0.055f) / 1.055f, 2.4f);
        }
        for (unsigned y = 0; y < out.scene.sky_height; ++y) {
            const unsigned y0 = y * static_cast<unsigned>(sky.height) / out.scene.sky_height;
            const unsigned y1 = (y + 1) * static_cast<unsigned>(sky.height) / out.scene.sky_height;
            for (unsigned x = 0; x < out.scene.sky_width; ++x) {
                const unsigned x0 = x * static_cast<unsigned>(sky.width) / out.scene.sky_width;
                const unsigned x1 =
                    (x + 1) * static_cast<unsigned>(sky.width) / out.scene.sky_width;
                Float3 sum{};
                for (unsigned sy = y0; sy < y1; ++sy) {
                    for (unsigned sx = x0; sx < x1; ++sx) {
                        const auto at =
                            (std::size_t{sy} * static_cast<unsigned>(sky.width) + sx) * 4;
                        for (unsigned c = 0; c < 3; ++c) {
                            sum[c] += linear[sky.pixels[at + c]];
                        }
                    }
                }
                for (float& v : sum) {
                    v /= static_cast<float>((x1 - x0) * (y1 - y0));
                }
                out.scene.sky_radiance[std::size_t{y} * out.scene.sky_width + x] = sum;
            }
        }
    }
    for (unsigned c = 0; c < 3; ++c) {
        out.scene.primary_directional_colour[c] = level.light_sets[light_set * 16 + c];
    }
    for (unsigned which : {0u, 2u}) {
        const auto at = light_set * 16 + which * 4;
        BakeSun sun;
        sun.primary = which == 0;
        for (unsigned c = 0; c < 3; ++c) {
            sun.direction[c] = level.light_sets[at + 4 + c];
            sun.irradiance[c] = level.light_sets[at + c] * std::numbers::pi_v<float>;
        }
        if (renderer::dot(sun.direction, sun.direction) > 0
            && *std::max_element(sun.irradiance.begin(), sun.irradiance.end()) > 0) {
            out.scene.suns.push_back(sun);
        }
    }
    std::vector<int> texture_indices(level.images.size(), -1);
    auto texture_index = [&](int image_index) {
        if (image_index < 0) {
            return -1;
        }
        const auto image = static_cast<std::size_t>(image_index);
        auto& texture = texture_indices.at(image);
        if (texture < 0) {
            const auto& source = level.images.at(image);
            texture = static_cast<int>(out.scene.textures.size());
            out.scene.textures.push_back(
                {static_cast<std::uint32_t>(source.width),
                 static_cast<std::uint32_t>(source.height),
                 source.pixels}
            );
        }
        return texture;
    };
    for (unsigned layer = 0; layer < 5; ++layer) {
        const auto id = static_cast<std::size_t>(viewer::Layer::Terrain) + layer;
        const bool bake_surface = layer == 4;
        for (const auto& instance : bake_surface ? level.bake_surfaces : level.instances[id]) {
            const auto& model = level.models.at(instance.model);
            const bool moby = layer == 3;
            if (moby
                && (!instance.bake_static || instance.stand_in || model.joints > 0
                    || instance.palette >= 0 || instance.blend != 0 || instance.tint[3] < .99f)) {
                ++out.skipped_mobys;
                continue;
            }
            if (bake_surface) {
                ++out.surface_instances;
            } else {
                ++out.instances[layer];
            }
            for (const auto& primitive : model.primitives) {
                const auto& material =
                    level.materials.at(static_cast<std::size_t>(primitive.material));
                if (material.metal || material.blend || material.colour[3] < 0.99f) {
                    ++out.skipped_blended;
                    continue;
                }
                const int texture = texture_index(material.image);
                const bool emits =
                    *std::max_element(material.emission.begin(), material.emission.end()) > 0;
                if (primitive.index_count % 3) {
                    throw std::runtime_error("non-triangle bake primitive");
                }
                for (std::size_t j = 0; j < primitive.index_count; j += 3) {
                    BakeTriangle triangle;
                    triangle.terrain_prelighting = layer == 0;
                    triangle.texture = texture;
                    triangle.emission = material.emission;
                    triangle.emission_texture = texture_index(material.emission_image);
                    triangle.emission_mask = material.emission_mask;
                    triangle.emission_vertex_colour = material.emission_vertex_colour;
                    triangle.diffuse = as_float3(material.colour.data());
                    triangle.diffuse_scale = material.diffuse_scale;
                    triangle.alpha_cutoff = material.cutout ? 0.5f : 0.02f;
                    bool glow = false;
                    for (unsigned k = 0; k < 3; ++k) {
                        const auto vertex = level.indices.at(primitive.first_index + j + k);
                        const auto& input = level.vertices.at(vertex);
                        if (triangle.terrain_prelighting) {
                            for (unsigned c = 0; c < 3; ++c) {
                                triangle.authored_colours[k][c] =
                                    input.colour[c] * instance.tint[c];
                            }
                        }
                        // Ignore input.normal, even if inverted or nonfinite.
                        // Embree derives transport normals from these positions.
                        triangle.vertices[k] =
                            renderer::transform_point(instance.matrix, as_float3(input.position));
                        triangle.uv[k] = {input.uv[0], input.uv[1]};
                        if (material.emission_vertex_colour) {
                            for (unsigned c = 0; c < 3; ++c) {
                                triangle.emission_colours[k][c] =
                                    input.colour[c] * instance.tint[c];
                            }
                        }
                        if (moby) {
                            glow |= input.light_slot < -1.5f;
                            for (unsigned c = 0; c < 4; ++c) {
                                triangle.multipliers[k][c] = input.colour[c] * instance.tint[c];
                            }
                        }
                    }
                    if (glow && !emits) {
                        ++out.skipped_glow;
                        continue;
                    }
                    if (glow) {
                        triangle.diffuse = {};
                    }
                    const auto& a = triangle.vertices[0];
                    const auto& b = triangle.vertices[1];
                    const auto& c = triangle.vertices[2];
                    const auto n = renderer::cross(
                        Float3{b[0] - a[0], b[1] - a[1], b[2] - a[2]},
                        Float3{c[0] - a[0], c[1] - a[1], c[2] - a[2]}
                    );
                    if (renderer::dot(n, n) < 1e-16f) {
                        ++out.skipped_degenerate;
                        continue;
                    }
                    for (const auto& p : triangle.vertices) {
                        for (unsigned axis = 0; axis < 3; ++axis) {
                            if (!std::isfinite(p[axis])) {
                                throw std::runtime_error("nonfinite static scene position");
                            }
                            out.bounds_min[axis] = std::min(out.bounds_min[axis], p[axis]);
                            out.bounds_max[axis] = std::max(out.bounds_max[axis], p[axis]);
                        }
                    }
                    out.scene.triangles.push_back(triangle);
                    out.emissive_triangles += emits ? 1 : 0;
                }
            }
        }
    }
    const bool liquid_source = automatic_liquids(level);
    if (liquid_source && analyze_liquids) {
        const auto directory = level.dir / "bake-inputs";
        const auto overlay = liquid_input(directory / "overlay.bin");
        const auto index = liquid_input(directory / "core_index.bin");
        const auto core = liquid_input(directory / "core_data.bin");
        const auto triangles = gaspar_liquid_triangles(overlay, index, core);
        out.automatic_liquid_triangles = triangles.size();
        out.surface_instances += triangles.empty() ? 0 : 1;
        for (const auto& triangle : triangles) {
            for (const auto& point : triangle.vertices) {
                for (unsigned axis = 0; axis < 3; ++axis) {
                    out.bounds_min[axis] = std::min(out.bounds_min[axis], point[axis]);
                    out.bounds_max[axis] = std::max(out.bounds_max[axis], point[axis]);
                }
            }
            out.scene.triangles.push_back(triangle);
            ++out.emissive_triangles;
        }
    }
    if (out.scene.triangles.empty()) {
        throw std::runtime_error("no static render geometry to bake");
    }
    // All mobys remain receivers, including those excluded from transport.
    for (const auto& instance : level.instances[static_cast<std::size_t>(viewer::Layer::Mobys)]) {
        for (unsigned axis = 0; axis < 3; ++axis) {
            out.bounds_min[axis] = std::min(out.bounds_min[axis], instance.matrix[12 + axis]);
            out.bounds_max[axis] = std::max(out.bounds_max[axis], instance.matrix[12 + axis]);
        }
    }
    // The holder's update spawns class 1040 at its world position +0.85 Z.
    // Use a warm emission-only proxy for that initial state; no rotating mesh
    // enters visibility or distance moments. Intensity is an artistic bake setting.
    const auto holder = level.moby_classes.find(1038);
    if (holder != level.moby_classes.end() && !holder->second.second) {
        for (const auto& instance :
             level.instances[static_cast<std::size_t>(viewer::Layer::Mobys)]) {
            if (instance.model == holder->second.first && !instance.stand_in) {
                out.scene.point_emitters.push_back(
                    {{instance.matrix[12], instance.matrix[13], instance.matrix[14] + 0.85f},
                     {2.0f, 1.0f, 0.2f}}
                );
            }
        }
    }
    Hash hash;
    hash.u32(0x53434e07);
    if (liquid_source) {
        hash.u32(0x4c415601);  // authored liquid interpretation revision
    }
    hash.u32(light_set);
    hash.floats(environment);
    if (level.emission_profile_revision) {
        hash.u32(0x454d4901);
        hash.u32(level.emission_profile_revision);
    }
    // Only levels with sphere proxies change identity; unaffected bakes stay valid.
    if (!out.scene.point_emitters.empty()) {
        hash.u32(0x53504801);
        hash.u32(static_cast<std::uint32_t>(out.scene.point_emitters.size()));
        hash.f32(0.85f);
        hash.floats(Float3{2.0f, 1.0f, 0.2f});
        // Real levels fingerprint source placements below. Do not hash derived
        // additions here: the game runs FE_TOWARDZERO, the offline bake does not.
        if (level.dir.empty()) {
            for (const auto& light : out.scene.point_emitters) {
                hash.floats(light.position);
                hash.floats(light.intensity);
            }
        }
    }
    // The running game uses FE_TOWARDZERO, including while loading assets.
    // Fingerprint canonical source bytes rather than rounded transform
    // products. File identity is stable across baker and game FP modes.
    if (!level.dir.empty()) {
        source_files(hash, level.dir, "base");
        if (!level.overrides.empty() && level.overrides != level.dir) {
            source_files(hash, level.overrides, "override");
        }
        out.scene_hash = hash.value;
        return out;
    }
    // In-memory fixtures have no source directory. Keep a structural hash
    // for those providers, which must share their arithmetic convention.
    hash.u32(out.scene.sky_width);
    hash.u32(out.scene.sky_height);
    for (const auto& pixel : out.scene.sky_radiance) {
        hash.floats(pixel);
    }
    hash.floats(out.scene.primary_directional_colour);
    for (const auto& sun : out.scene.suns) {
        hash.floats(sun.direction);
        hash.floats(sun.irradiance);
    }
    hash.u32(static_cast<std::uint32_t>(out.scene.textures.size()));
    for (const auto& texture : out.scene.textures) {
        hash.u32(texture.width);
        hash.u32(texture.height);
        for (auto b : texture.rgba) {
            hash.byte(b);
        }
    }
    for (const auto& triangle : out.scene.triangles) {
        for (auto p : triangle.vertices) {
            hash.floats(p);
        }
        for (auto uv : triangle.uv) {
            hash.floats(uv);
        }
        hash.floats(triangle.diffuse);
        hash.f32(triangle.diffuse_scale);
        if (triangle.terrain_prelighting) {
            hash.u32(0x54494e54);
            for (auto tint : triangle.authored_colours) {
                hash.floats(tint);
            }
        }
        hash.u32(static_cast<std::uint32_t>(triangle.texture));
        hash.f32(triangle.alpha_cutoff);
        hash.floats(triangle.emission);
        hash.u32(static_cast<std::uint32_t>(triangle.emission_texture));
        if (triangle.emission_mask != EmissionMask::None) {
            hash.u32(0x4d41534b);
            hash.u32(static_cast<unsigned>(triangle.emission_mask));
        }
        if (triangle.emission_vertex_colour) {
            hash.u32(0x54494e54);
            for (const auto& c : triangle.emission_colours) {
                hash.floats(c);
            }
        }
        for (const auto& multiplier : triangle.multipliers) {
            hash.floats(multiplier);
        }
    }
    out.scene_hash = hash.value;
    return out;
}

static ProbeGrid scene_probe_grid(
    const BakeSceneExport& scene, Float3 spacing, unsigned resolution, bool validate
) {
    for (float s : spacing) {
        if (!std::isfinite(s) || s <= 0) {
            throw std::runtime_error("probe spacing must be positive");
        }
    }
    ProbeGrid grid;
    grid.spacing = spacing;
    grid.map_resolution = resolution;
    grid.scene_hash = scene.scene_hash;
    double cell_diagonal_squared = 0;
    for (unsigned axis = 0; axis < 3; ++axis) {
        // One full cell of padding and cell-aligned extents.
        const float step = spacing[axis];
        grid.origin[axis] = std::floor(scene.bounds_min[axis] / step) * step - step;
        const double upper = std::ceil(scene.bounds_max[axis] / step) * step + step;
        const double count = std::ceil((upper - grid.origin[axis]) / step) + 1;
        if (!std::isfinite(count) || count < 2 || count > 65536) {
            throw std::runtime_error("unsupported sparse probe extent");
        }
        grid.dimensions[axis] = static_cast<std::uint32_t>(count);
        cell_diagonal_squared += static_cast<double>(step) * step;
    }
    // Moments describe visibility to this cell's eight probes, not the whole
    // level. Distant sky misses otherwise swamp nearby wall depths at openings.
    // Radiance and occupancy still trace the complete scene independently.
    grid.max_distance = static_cast<float>(1.5 * std::sqrt(cell_diagonal_squared));
    if (validate) {
        grid.validate();
    }
    return grid;
}

ProbeGrid sparse_probe_grid(const BakeSceneExport& scene, float spacing, unsigned resolution) {
    return sparse_probe_grid(scene, Float3{spacing, spacing, spacing}, resolution);
}

ProbeGrid sparse_probe_grid(const BakeSceneExport& scene, Float3 spacing, unsigned resolution) {
    return scene_probe_grid(scene, spacing, resolution, true);
}

ProbeGrid bounded_probe_grid(
    const BakeSceneExport& scene, Float3 minimum, Float3 maximum, float spacing, unsigned resolution
) {
    return bounded_probe_grid(
        scene, minimum, maximum, Float3{spacing, spacing, spacing}, resolution
    );
}

ProbeGrid bounded_probe_grid(
    const BakeSceneExport& scene,
    Float3 minimum,
    Float3 maximum,
    Float3 spacing,
    unsigned resolution
) {
    // Only the bounded allocation needs to fit atlas indexing. A fine local
    // grid must not be rejected because a hypothetical whole-level grid at
    // that spacing would exceed the budget.
    auto grid = scene_probe_grid(scene, spacing, resolution, false);
    // Retain the cell-local moment range; allocation bounds do not clip transport.
    for (unsigned axis = 0; axis < 3; ++axis) {
        if (!std::isfinite(minimum[axis]) || !std::isfinite(maximum[axis])
            || minimum[axis] >= maximum[axis]) {
            throw std::runtime_error("invalid local probe bounds");
        }
        const float step = spacing[axis];
        grid.origin[axis] = std::floor(minimum[axis] / step) * step;
        const double end = std::ceil(maximum[axis] / step) * step;
        const double count = std::ceil((end - grid.origin[axis]) / step) + 1;
        if (!std::isfinite(count) || count < 2 || count > 65536) {
            throw std::runtime_error("unsupported local probe extent");
        }
        grid.dimensions[axis] = static_cast<std::uint32_t>(count);
    }
    grid.validate();
    return grid;
}

std::uint64_t probe_bake_settings_hash(const ProbeGrid& grid, const BakeSettings& settings) {
    Hash hash;
    hash.u32(0x42414b09);
    hash.u32(settings.use_level_sky ? 1u : 0u);
    hash.f32(settings.sky_intensity);
    hash.f32(settings.sky_weight);
    hash.floats(settings.sunlight_scale);
    hash.u32(settings.terrain_vertex_albedo ? 1u : 0u);
    hash.floats(grid.origin);
    hash.floats(grid.spacing);
    for (auto n : grid.dimensions) {
        hash.u32(n);
    }
    hash.u32(grid.map_resolution);
    hash.f32(grid.max_distance);
    hash.u32(settings.rays_per_probe);
    hash.u32(settings.bounces);
    hash.u32(settings.seed);
    hash.f32(settings.ray_bias);
    hash.f32(settings.distance_exponent);
    hash.f32(settings.min_inside_fraction);
    hash.u32(settings.relocation_iterations);
    hash.f32(settings.relocation_max_fraction);
    return hash.value;
}
}  // namespace openrac::lighting

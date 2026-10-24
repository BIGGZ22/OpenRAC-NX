// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/viewer/probe_lighting.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <format>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>

#include "common/env.h"
#include "common/json.h"
#include "common/log.h"
#include "common/stats.h"
#include "enhanced/tools/probes/activation.h"
#include "enhanced/viewer/early_lighting.h"
#include "renderer/gl.h"
#include "viewer_shaders.h"

namespace openrac::lighting {
using namespace openrac::gl;

namespace {
struct ProbeTextureFormat {
    GLenum internal, source, type;
};

constexpr ProbeTextureFormat kProbeFormats[] = {
    {GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV},
    {GL_RG32F, GL_RG, GL_FLOAT},
    {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},
};
constexpr std::size_t kDenseResidencyBudget = 128ull * 1024 * 1024;
constexpr unsigned kFirstUnit = 3;

// Returns the value `var` was set to, or the empty string when it is unset: the probe GI's reads
// take the two alike.
std::string environment(env::Var var) {
    return std::string(env::value(var).value_or(std::string_view{}));
}

unsigned rays_from_environment() {
    const auto value = environment(env::Var::ProbeRays);
    if (value.empty()) {
        return 1024;
    }
    std::string_view text(value);
    unsigned result = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || result < 16) {
        throw std::runtime_error("invalid OPENRAC_PROBE_RAYS");
    }
    return result;
}
}  // namespace

std::string probe_shader_source(std::string source) {
    if (!enhanced_rendering_available()) {
        return source;
    }
    source.insert(source.find('\n') + 1, scene::shaders::probes_glsl);
    static const bool& gather_off = stats::probe(
        "OPENRAC_SHADOW_GATHER_OFF", "Reference: 16 individual shadow texel fetches (reload level)"
    );
    if (gl::is_vulkan() && !gather_off) {
        source.insert(source.find('\n') + 1, "#define OPENRAC_SHADOW_GATHER 1\n");
    }
    static const bool& reference = stats::probe(
        "OPENRAC_SHADOW_FILTER_REFERENCE",
        "Profiling reference: nine bilinear shadow taps instead of unique texels"
    );
    if (reference) {
        source.insert(source.find('\n') + 1, "#define OPENRAC_SHADOW_FILTER_REFERENCE 1\n");
    }
    static const bool& zero_weight_reference = stats::probe(
        "OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE",
        "Profiling reference: fetch probe atlases for zero-weight valid corners"
    );
    if (zero_weight_reference) {
        source.insert(source.find('\n') + 1, "#define OPENRAC_PROBE_ZERO_WEIGHT_REFERENCE 1\n");
    }
    return source;
}

std::string probe_mesh_fragment(std::string source) {
    return probe_shader_source(std::move(source));
}

std::string probe_early_fragment(std::string source) {
    if (!enhanced_rendering_available()) {
        return probe_mesh_fragment(std::move(source));
    }
    source += scene::shaders::early_lighting_glsl;
    source.insert(source.find('\n') + 1, "#define OPENRAC_EARLY_LIGHTING 1\n");
    return probe_shader_source(std::move(source));
}

bool ProbeLighting::upload(
    const lighting::ProbeVolume& volume, std::string& error, const lighting::ProbeVolume* dense
) {
    std::vector<lighting::ProbeVolume> regions;
    if (dense) {
        regions.push_back(*dense);
    }
    if (!upload_regions(volume, std::move(regions), error)) {
        return false;
    }
    // An explicit dense pair starts fully active until the first camera update.
    if (dense) {
        m_selector.slots[0] = {0, 1};
        upload_region(0, 0);
    }
    return true;
}

bool ProbeLighting::upload_regions(
    const lighting::ProbeVolume& volume,
    std::vector<lighting::ProbeVolume> regions,
    std::string& error
) {
    release();
    if (!enhanced_rendering_available()) {
        error = "Probe lighting requires an enhanced rendering build and Vulkan";
        return false;
    }
    try {
        volume.validate();
        GLint limit = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
        if (limit <= 0) {
            throw std::runtime_error("no probe texture capacity");
        }
        for (const auto& region : regions) {
            lighting::validate_probe_pair(volume, region);
            m_region_capacity = std::max(m_region_capacity, region.atlas_capacity());
        }
        m_regions = std::move(regions);
        std::size_t total_regions = 0;
        for (const auto& region : m_regions) {
            m_region_grids.push_back(region.grid);
            m_region_offsets.push_back(
                static_cast<unsigned>(volume.atlas_capacity() + total_regions)
            );
            total_regions += region.atlas_capacity();
        }
        std::string compute_error;
        const auto side = volume.grid.map_resolution + 2;
        const auto resident_bytes = total_regions * (12ull * side * side + 8);
        if (resident_bytes <= kDenseResidencyBudget) {
            m_full_residency = m_selection.prepare(m_region_grids, m_region_offsets, compute_error);
        } else {
            m_selection.prepare({}, {}, compute_error);
            log::warn("probe GPU selection fallback: dense residency exceeds 128 MiB budget");
        }
        if (!compute_error.empty()) {
            log::warn("probe GPU selection fallback: {}", compute_error);
        }
        if (volume.gpu_bytes.empty()) {
            throw std::runtime_error("probe upload requires GPU-ready baked data");
        }
        for (const auto& region : m_regions) {
            if (region.gpu_bytes.empty()) {
                throw std::runtime_error("dense probe upload requires GPU-ready baked data");
            }
        }
        const unsigned width = lighting::kProbeAtlasTilesX * side;
        const auto rows_for = [&](bool resident) {
            return (volume.atlas_capacity()
                    + (resident ? total_regions : m_region_capacity * lighting::kDenseProbeSlots))
                   / lighting::kProbeAtlasTilesX;
        };
        if (width > static_cast<unsigned>(limit)) {
            throw std::runtime_error("probe atlas width exceeds GPU capacity");
        }
        if (rows_for(m_full_residency) * side > static_cast<unsigned>(limit) && m_full_residency) {
            m_full_residency = false;
            m_selection.release();
            log::warn("probe GPU selection fallback: full resident atlas exceeds capacity");
        }
        const auto rows = rows_for(m_full_residency);
        if (rows * side > static_cast<unsigned>(limit)) {
            throw std::runtime_error("probe atlas exceeds GPU capacity");
        }
        glGenTextures(3, m_textures.data());
        for (unsigned i = 0; i < 3; ++i) {
            glActiveTexture(GL_TEXTURE0 + kFirstUnit + i);
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, i == 2 ? GL_NEAREST : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, i == 2 ? GL_NEAREST : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                kProbeFormats[i].internal,
                static_cast<GLsizei>(i == 2 ? lighting::kProbeAtlasTilesX : width),
                static_cast<GLsizei>(i == 2 ? rows : rows * side),
                0,
                kProbeFormats[i].source,
                kProbeFormats[i].type,
                nullptr
            );
        }
        glActiveTexture(GL_TEXTURE0);
        if (glGetError() != GL_NO_ERROR) {
            throw std::runtime_error("probe texture upload failed");
        }
        m_grid = volume.grid;
        m_sparse_capacity = volume.atlas_capacity();
        upload_volume(volume, 0);
        log::info(
            "probe textures: R11G11B10F irradiance, RG32F moments, RGBA16F metadata; {} bytes",
            std::size_t{width} * rows * side * 12 + lighting::kProbeAtlasTilesX * rows * 8
        );
        if (m_full_residency) {
            for (unsigned i = 0; i < m_regions.size(); ++i) {
                upload_region(0, static_cast<int>(i));
            }
            log::info(
                "tiled probe selection: {} resident regions, 16x16 tiles, two volumes per fragment",
                m_regions.size()
            );
        }
        const auto append_spheres = [&](const lighting::ProbeVolume& source, int region) {
            const float radius =
                0.025f * *std::min_element(source.grid.spacing.begin(), source.grid.spacing.end());
            for (std::size_t i = 0; i < source.probes.size(); ++i) {
                const auto position = source.position(i);
                const auto& probe = source.probes[i];
                const bool relocated = probe.offset != lighting::Float3{};
                m_spheres.push_back(
                    {{position[0], position[1], position[2], radius},
                     !probe.valid  ? lighting::Float3{1, 0.05f, 0.7f}
                     : relocated   ? lighting::Float3{1, 0.6f, 0.05f}
                     : region >= 0 ? lighting::Float3{0.15f, 1.0f, 0.3f}
                                   : lighting::Float3{0.05f, 0.85f, 1},
                     region}
                );
            }
        };
        append_spheres(volume, -1);
        for (std::size_t i = 0; i < m_regions.size(); ++i) {
            append_spheres(m_regions[i], static_cast<int>(i));
        }
        return true;
    } catch (const std::exception& why) {
        error = why.what();
        release();
        return false;
    }
}

void ProbeLighting::upload_volume(const lighting::ProbeVolume& volume, std::size_t offset) {
    const auto side = volume.grid.map_resolution + 2;
    // Each file already contains complete atlas rows in the exact GPU formats.
    // No tile rearrangement, irradiance encoding or float conversion here.
    for (unsigned map = 0; map < 3; ++map) {
        glActiveTexture(GL_TEXTURE0 + kFirstUnit + map);
        glBindTexture(GL_TEXTURE_2D, m_textures[map]);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            static_cast<GLint>(offset / lighting::kProbeAtlasTilesX * (map == 2 ? 1 : side)),
            static_cast<GLsizei>(lighting::kProbeAtlasTilesX * (map == 2 ? 1 : side)),
            static_cast<GLsizei>(volume.atlas_rows() * (map == 2 ? 1 : side)),
            kProbeFormats[map].source,
            kProbeFormats[map].type,
            volume.gpu_plane(map).data()
        );
    }
    glActiveTexture(GL_TEXTURE0);
    if (glGetError() != GL_NO_ERROR) {
        throw std::runtime_error("packed probe texture upload failed");
    }
}

std::size_t ProbeLighting::region_offset(unsigned slot, int region) const {
    return m_full_residency ? m_region_offsets.at(region)
                            : m_sparse_capacity + slot * m_region_capacity;
}

void ProbeLighting::upload_region(unsigned slot, int region) {
    upload_volume(m_regions.at(region), region_offset(slot, region));
    if (!m_full_residency) {
        m_gpu_regions[slot] = region;
    }
}

void ProbeLighting::update_regions(
    lighting::Float3 camera, float seconds, const renderer::Mat4* world_to_clip
) {
    const auto now = std::chrono::steady_clock::now();
    if (seconds < 0) {
        seconds = m_camera_started
                      ? std::min(0.1f, std::chrono::duration<float>(now - m_region_time).count())
                      : m_selector.fade_seconds;
    }
    if (!m_camera_started) {
        m_selector.slots = {};
    }
    m_camera_started = true;
    m_region_time = now;
    if (!active()) {
        return;
    }
    m_selector.update(m_region_grids, camera, seconds, world_to_clip);
    if (m_full_residency) {
        return;
    }
    for (unsigned slot = 0; slot < lighting::kDenseProbeSlots; ++slot) {
        const int region = m_selector.slots[slot].region;
        if (region >= 0 && region != m_gpu_regions[slot]) {
            upload_region(slot, region);
        }
    }
}

namespace {
std::filesystem::path& probe_root_storage() {
    static std::filesystem::path root;
    return root;
}

// The level's bake under the probe root: <root>/level_NN/sparse.orpg, NN its folder's name.
std::string level_bake(const scene::LevelData& level) {
    const auto& root = probe_root();
    if (root.empty()) {
        return {};
    }
    auto name = level.dir.filename();
    if (name.empty()) {
        name = level.dir.parent_path().filename();
    }
    if (name.empty() && level.level >= 0) {
        name = std::format("level_{:02}", level.level);
    }
    const auto sparse = root / name / "sparse.orpg";
    std::error_code error;
    if (name.empty() || !std::filesystem::is_regular_file(sparse, error)) {
        log::info("probe GI: no bake for {} yet; original lighting", name.string());
        return {};
    }
    return sparse.string();
}
}  // namespace

void set_probe_root(std::filesystem::path root) {
    probe_root_storage() = std::move(root);
}

const std::filesystem::path& probe_root() {
    return probe_root_storage();
}

void ProbeLighting::load(const scene::LevelData& level) {
    if (!enhanced_rendering_available()) {
        return;
    }
    auto path = environment(env::Var::ProbeGi);
    if (path == "0") {
        return;
    }
    if (path.empty()) {
        path = level_bake(level);
    }
    if (path.empty()) {
        return;
    }
    try {
        auto expected = lighting::load_level_bake_profile(level.dir, level.overrides).activation;
        if (!environment(env::Var::ProbeRays).empty()) {
            expected.bake.rays_per_probe = rays_from_environment();
        }
        const auto sky = environment(env::Var::ProbeSky);
        if (!sky.empty()) {
            if (sky != "level" && sky != "uniform") {
                throw std::runtime_error("invalid OPENRAC_PROBE_SKY");
            }
            expected.bake.use_level_sky = sky == "level";
        }
        const auto intensity = environment(env::Var::ProbeSkyIntensity);
        if (!intensity.empty()) {
            std::size_t consumed = 0;
            expected.bake.sky_intensity = std::stof(intensity, &consumed);
            if (consumed != intensity.size() || !std::isfinite(expected.bake.sky_intensity)
                || expected.bake.sky_intensity < 0) {
                throw std::runtime_error("invalid OPENRAC_PROBE_SKY_INTENSITY");
            }
        }
        const auto volume = lighting::load_level_probes(path, level, expected);
        std::vector<lighting::ProbeVolume> regions;
        const auto add_region = [&](const std::filesystem::path& file) {
            try {
                auto region = lighting::load_level_probes(file, level, expected);
                lighting::validate_probe_pair(volume, region);
                regions.push_back(std::move(region));
            } catch (const std::exception& why) {
                log::warn("local probe region {} inactive: {}", file.string(), why.what());
            }
        };
        const auto dense_path = environment(env::Var::ProbeDense);
        if (!dense_path.empty() && dense_path != "0") {
            add_region(dense_path);
        }
        auto manifest_path = environment(env::Var::ProbeRegions);
        if (manifest_path.empty()) {
            const auto sidecar = path + ".regions.json";
            if (std::filesystem::is_regular_file(sidecar)) {
                manifest_path = sidecar;
            }
        }
        if (!manifest_path.empty() && manifest_path != "0") {
            json::Value manifest;
            std::string why;
            if (!json::parse_file(manifest_path, manifest, why)
                || !manifest["regions"].is_array()) {
                log::warn(
                    "probe region manifest inactive: {}",
                    why.empty() ? "regions must be an array" : why
                );
            } else {
                for (const auto& entry : manifest["regions"].items()) {
                    if (!entry.is_string() || entry.string().empty()) {
                        log::warn("probe region manifest entry must be an asset path");
                        continue;
                    }
                    add_region(std::filesystem::path(manifest_path).parent_path() / entry.string());
                }
            }
        }
        std::string error;
        if (!upload_regions(volume, std::move(regions), error)) {
            log::warn("probe region allocation failed: {}; using level grid", error);
            if (!upload(volume, error)) {
                throw std::runtime_error(error);
            }
        }
        log::info(
            "probe GI: {} dense regions registered; at most two dense samples per fragment",
            m_regions.size()
        );
        m_sunlight_scale = expected.bake.sunlight_scale;
        m_terrain_vertex_albedo = expected.bake.terrain_vertex_albedo;
        log::info(
            "enhanced sunlight RGB scale: {} {} {}",
            m_sunlight_scale[0],
            m_sunlight_scale[1],
            m_sunlight_scale[2]
        );
        const auto view = environment(env::Var::ProbeView);
        if (!view.empty()) {
            const std::string_view name(view);
            if (name == "original") {
                mode = 0;
            } else if (name == "combined") {
                mode = 1;
            } else if (name == "indirect") {
                mode = 2;
            } else if (name == "direct") {
                mode = 3;
            } else if (name == "coverage") {
                mode = 4;
            } else {
                throw std::runtime_error("unknown OPENRAC_PROBE_VIEW");
            }
        }
        backlight = env::flag(env::Var::ProbeBacklight);
        const auto sphere_view = environment(env::Var::ProbeSpheres);
        sphere_xray = sphere_view == "xray";
        show_spheres = sphere_view == "1" || sphere_xray;
        environment_spheres = show_spheres;
        if (show_spheres) {
            log::info(
                "probe spheres: actual positions within {} grid spacings of camera; cyan "
                "sparse, green dense, gold relocated, magenta invalid",
                sphere_xray ? 10 : 5
            );
        }
        log::info(
            "probe GI: {} probes for {} level {}, scene/settings verified; view {}",
            volume.probes.size(),
            m_grid.game_version,
            m_grid.level,
            mode
        );
        log::info("probe GI: original ambient replaced on terrain and lit mobys; GTAO follows the "
                  "user setting");
    } catch (const std::exception& why) {
        release();
        log::warn("probe GI inactive for level {}: {}", level.level, why.what());
    }
}

void ProbeLighting::bind(
    const renderer::Shader& shader,
    scene::Layer layer,
    bool all_static_receivers,
    const renderer::Camera* camera,
    bool depth_matched
) const {
    if (!enhanced_rendering_available()) {
        return;
    }
    m_selection.bind(shader, camera, depth_matched);
    const bool static_receiver =
        layer == scene::Layer::Terrain
        || ((mode == 2 || all_static_receivers)
            && (layer == scene::Layer::Ties || layer == scene::Layer::Shrubs));
    const int receiver = static_receiver ? 1 : (layer == scene::Layer::Mobys ? 2 : 0);
    glUniform1i(shader.uniform("probe_receiver"), receiver);
    glUniform1i(shader.uniform("probe_mode"), active() && receiver ? mode : 0);
    glUniform1f(shader.uniform("probe_gi_multiplier"), gi_multiplier);
    glUniform1i(
        shader.uniform("probe_terrain_albedo"),
        m_terrain_vertex_albedo && layer == scene::Layer::Terrain ? 1 : 0
    );
    glUniform3f(
        shader.uniform("probe_sunlight_scale"),
        m_sunlight_scale[0],
        m_sunlight_scale[1],
        m_sunlight_scale[2]
    );
    glUniform1f(shader.uniform("probe_normal_bias"), normal_bias);
    glUniform1f(shader.uniform("probe_view_bias"), view_bias);
    glUniform1i(shader.uniform("probe_effect"), 0);
    glUniform1i(shader.uniform("probe_backlight"), backlight ? 1 : 0);
    glUniform1i(shader.uniform("probe_irradiance"), kFirstUnit);
    glUniform1i(shader.uniform("probe_moments"), kFirstUnit + 1);
    glUniform1i(shader.uniform("probe_metadata"), kFirstUnit + 2);
    if (!active() || !receiver) {
        return;
    }
    glUniform3f(
        shader.uniform("probe_origin"), m_grid.origin[0], m_grid.origin[1], m_grid.origin[2]
    );
    glUniform3f(
        shader.uniform("probe_spacing"), m_grid.spacing[0], m_grid.spacing[1], m_grid.spacing[2]
    );
    glUniform3f(
        shader.uniform("probe_dimensions"),
        static_cast<float>(m_grid.dimensions[0]),
        static_cast<float>(m_grid.dimensions[1]),
        static_cast<float>(m_grid.dimensions[2])
    );
    glUniform1f(shader.uniform("probe_resolution"), static_cast<float>(m_grid.map_resolution));
    glUniform1i(shader.uniform("probe_tiles_x"), static_cast<GLint>(lighting::kProbeAtlasTilesX));
    for (unsigned slot = 0; slot < lighting::kDenseProbeSlots; ++slot) {
        const auto& selected = m_selector.slots[slot];
        const auto suffix = "[" + std::to_string(slot) + "]";
        glUniform1f(
            shader.uniform(("probe_dense_weight" + suffix).c_str()),
            selected.region >= 0 ? selected.weight : 0.0f
        );
        if (selected.region < 0) {
            continue;
        }
        const auto& g = m_region_grids[selected.region];
        glUniform3f(
            shader.uniform(("probe_dense_origin" + suffix).c_str()),
            g.origin[0],
            g.origin[1],
            g.origin[2]
        );
        glUniform3f(
            shader.uniform(("probe_dense_spacing" + suffix).c_str()),
            g.spacing[0],
            g.spacing[1],
            g.spacing[2]
        );
        glUniform3f(
            shader.uniform(("probe_dense_dimensions" + suffix).c_str()),
            float(g.dimensions[0]),
            float(g.dimensions[1]),
            float(g.dimensions[2])
        );
        glUniform1i(
            shader.uniform(("probe_dense_offset" + suffix).c_str()),
            static_cast<int>(region_offset(slot, selected.region))
        );
    }
    for (unsigned i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + kFirstUnit + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
    }
    glActiveTexture(GL_TEXTURE0);
}

void ProbeLighting::draw_spheres(const renderer::FrameInput& input, renderer::RenderState& state) {
    if (!active() || !show_spheres) {
        return;
    }
    if (!m_sphere_shader.id()) {
        std::string error;
        if (!m_sphere_shader.build(
                "probe placement spheres",
                scene::shaders::probe_spheres_vert,
                scene::shaders::probe_spheres_frag,
                error
            )) {
            log::warn("probe spheres disabled: {}", error);
            show_spheres = false;
            return;
        }
        glGenVertexArrays(1, &m_sphere_vao);
        glGenBuffers(1, &m_sphere_buffer);
        glGenBuffers(1, &m_sphere_indices);
        glBindVertexArray(m_sphere_vao);
        std::array<unsigned, 12 * 8 * 6> indices{};
        std::iota(indices.begin(), indices.end(), 0u);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphere_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, m_sphere_buffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Sphere), nullptr);
        glVertexAttribDivisor(0, 1);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Sphere),
            reinterpret_cast<const void*>(offsetof(Sphere, colour))
        );
        glVertexAttribDivisor(1, 1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            1,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Sphere),
            reinterpret_cast<const void*>(offsetof(Sphere, opacity))
        );
        glVertexAttribDivisor(2, 1);
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(
            3, 1, GL_INT, sizeof(Sphere), reinterpret_cast<const void*>(offsetof(Sphere, region))
        );
        glVertexAttribDivisor(3, 1);
    }
    m_visible_spheres.clear();
    // A local placement view keeps the level-wide lattice from obscuring the
    // scene and bounds diagnostic geometry cost. This never changes GI sampling.
    const float range = (sphere_xray ? 10.0f : 5.0f)
                        * *std::max_element(m_grid.spacing.begin(), m_grid.spacing.end());
    for (auto sphere : m_spheres) {
        if (sphere.region >= 0 && !m_selection.active()) {
            const auto selected = std::find_if(
                m_selector.slots.begin(),
                m_selector.slots.end(),
                [&](const auto& slot) { return slot.region == sphere.region && slot.weight > 0; }
            );
            if (selected == m_selector.slots.end()) {
                continue;
            }
            sphere.opacity = selected->weight;
        }
        float squared = 0;
        for (unsigned axis = 0; axis < 3; ++axis) {
            const float delta = sphere.position_radius[axis] - input.camera.position[axis];
            squared += delta * delta;
        }
        if (squared <= range * range) {
            m_visible_spheres.push_back(sphere);
        }
    }
    if (m_visible_spheres.empty()) {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return;
    }
    glBindVertexArray(m_sphere_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphere_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_visible_spheres.size() * sizeof(Sphere)),
        m_visible_spheres.data(),
        GL_STREAM_DRAW
    );
    m_sphere_shader.use();
    m_selection.bind(m_sphere_shader, &input.camera);
    const auto matrix = renderer::multiply(input.camera.projection, input.camera.view);
    glUniformMatrix4fv(m_sphere_shader.uniform("view_projection"), 1, GL_FALSE, matrix.data());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_FALSE);  // diagnostic spheres never hide the scene or later markers
    // The latitude/longitude triangles have inward winding. Draw only the
    // outward-visible hemisphere to avoid alpha self-overdraw during fades.
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    constexpr int vertices = 12 * 8 * 6;
    const auto draw = [&]() {
        glDrawElementsInstanced(
            GL_TRIANGLES,
            vertices,
            GL_UNSIGNED_INT,
            nullptr,
            static_cast<GLsizei>(m_visible_spheres.size())
        );
        ++state.stats.draw_calls;
        state.stats.triangles += m_visible_spheres.size() * (vertices / 3);
    };
    if (sphere_xray) {
        // Hidden portions are translucent and never modify scene depth.
        // Both visible and hidden portions follow the dense region's fade.
        glDepthFunc(GL_LESS);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
        glUniform1f(m_sphere_shader.uniform("debug_alpha"), 0.18f);
        draw();
        glDepthFunc(GL_GEQUAL);
    }
    glUniform1f(m_sphere_shader.uniform("debug_alpha"), 1.0f);
    draw();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ProbeLighting::release() {
    m_sunlight_scale = {1, 1, 1};
    m_terrain_vertex_albedo = false;
    m_selection.release();
    m_full_residency = false;
    m_region_offsets.clear();
    m_sphere_shader.release();
    if (m_sphere_vao) {
        glDeleteVertexArrays(1, &m_sphere_vao);
    }
    if (m_sphere_buffer) {
        glDeleteBuffers(1, &m_sphere_buffer);
    }
    if (m_sphere_indices) {
        glDeleteBuffers(1, &m_sphere_indices);
    }
    m_sphere_vao = m_sphere_buffer = m_sphere_indices = 0;
    m_spheres.clear();
    m_visible_spheres.clear();
    show_spheres = false;
    environment_spheres = false;
    sphere_xray = false;
    if (m_textures[0]) {
        glDeleteTextures(3, m_textures.data());
    }
    m_textures = {};
    m_sparse_capacity = 0;
    m_regions.clear();
    m_region_grids.clear();
    m_selector = {};
    m_gpu_regions = {-1, -1};
    m_region_capacity = 0;
    m_camera_started = false;
    m_region_time = {};
}
}  // namespace openrac::lighting

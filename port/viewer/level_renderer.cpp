// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "viewer/level_renderer.h"

#include <algorithm>

#include "renderer/gl.h"
#include "viewer_shaders.h"

namespace openrac::viewer {

using namespace openrac::gl;
using renderer::Bucket;

namespace {

// The joint palette texture: RGBA32F, four texels (columns) per matrix, this many texels a row.
constexpr int kPaletteWidth = 1024;
constexpr GLenum kRgba32f = 0x8814;
constexpr GLenum kPaletteUnit = 1;
// The ties' lit colours: RGBA8, one texel per colour, this many a row.
constexpr int kLightWidth = 1024;
constexpr GLenum kLightUnit = 2;

Bucket bucket_of(Layer layer) {
    switch (layer) {
        case Layer::Sky:
            return Bucket::Sky;
        case Layer::Terrain:
            return Bucket::Terrain;
        case Layer::Ties:
            return Bucket::Ties;
        case Layer::Shrubs:
            return Bucket::Shrubs;
        default:
            return Bucket::Mobys;
    }
}

const void* offset(std::size_t bytes) {
    return reinterpret_cast<const void*>(bytes);
}

// The sides of what a camera sees (its view and projection, column-major), as planes a point is
// inside of when a x + b y + c z + d >= 0: left, right, bottom, top. Near and far are left out, so
// that nothing depends on the projection's depth range; they would leave out little.
std::array<std::array<float, 4>, 4> frustum_sides(const renderer::Mat4& m) {
    const auto row = [&](int i) { return std::array<float, 4>{m[i], m[4 + i], m[8 + i], m[12 + i]}; };
    const auto x = row(0), y = row(1), w = row(3);
    std::array<std::array<float, 4>, 4> planes{};
    for (int k = 0; k < 4; ++k) {
        planes[0][k] = w[k] + x[k];
        planes[1][k] = w[k] - x[k];
        planes[2][k] = w[k] + y[k];
        planes[3][k] = w[k] - y[k];
    }
    return planes;
}

// Whether a box is wholly outside one of the sides: its corner furthest along the side's inward
// direction is outside it.
bool outside(const std::array<std::array<float, 4>, 4>& planes, const renderer::Vec3& low, const renderer::Vec3& high) {
    for (const auto& p : planes) {
        const float x = p[0] >= 0.0f ? high[0] : low[0];
        const float y = p[1] >= 0.0f ? high[1] : low[1];
        const float z = p[2] >= 0.0f ? high[2] : low[2];
        if (p[0] * x + p[1] * y + p[2] * z + p[3] < 0.0f) {
            return true;
        }
    }
    return false;
}

}  // namespace

bool LevelScene::upload(
    const LevelData& level, renderer::TexturePool& textures, std::string& error
) {
    if (!m_mesh.build("mesh", shaders::mesh_vert, shaders::mesh_frag, error)
        || !m_sky.build("sky", shaders::sky_vert, shaders::sky_frag, error)) {
        return false;
    }
    m_models = level.models;
    m_materials = level.materials;
    m_light_sets = level.light_sets;
    renderer::Rgba8Image white(1, 1);
    white.set_texel(0, 0, 0xFFFFFFFFu);
    m_white = textures.add(std::move(white), renderer::AlphaScale::Full, "white");
    // The cutout materials' moby textures (the hologram's lettering, fences, foliage) with one
    // level, bilinear: the GS samples moby textures without mip levels (TEX1 MXL 0), and a mipmap
    // chain averages a cutout's alpha away at a distance (thin, broken letters).
    std::vector<bool> cutout_moby(level.images.size(), false);
    for (const Material& m : level.materials) {
        if (m.cutout && m.image >= 0 && static_cast<std::size_t>(m.image) < level.images.size()
            && level.image_paths[static_cast<std::size_t>(m.image)].find("/textures/moby_") != std::string::npos) {
            cutout_moby[static_cast<std::size_t>(m.image)] = true;
        }
    }
    for (std::size_t i = 0; i < level.images.size(); ++i) {
        m_textures.push_back(
            level.images[i].width > 0
                ? textures.add(level.images[i], renderer::AlphaScale::Full, level.image_paths[i], !cutout_moby[i],
                               cutout_moby[i])
                : m_white
        );
    }

    // The static layers' models again, their primitives in material order, for the opaque pass:
    // the level's terrain is one model of a primitive per fragment and material, two thousand
    // draws in a frame, and in material order a material's fragments are one run of indices,
    // one draw (draw_layer joins them). Depth decides what is in front in that pass, so the order
    // does not change the picture; the blended passes keep the stored order.
    std::vector<std::uint32_t> indices = level.indices;
    m_opaque.assign(m_models.size(), 0);
    std::vector<bool> fixed(m_models.size(), false);
    for (const Layer layer : {Layer::Terrain, Layer::Ties, Layer::Shrubs}) {
        for (const Instance& instance : level.instances[static_cast<std::size_t>(layer)]) {
            if (instance.model < fixed.size()) {
                fixed[instance.model] = true;
            }
        }
    }
    for (std::uint32_t m = 0; m < m_opaque.size(); ++m) {
        m_opaque[m] = m;
        const Model& model = m_models[m];
        if (!fixed[m] || model.primitives.size() < 2) {
            continue;
        }
        Model sorted = model;
        std::stable_sort(sorted.primitives.begin(), sorted.primitives.end(),
                         [](const Primitive& a, const Primitive& b) { return a.material < b.material; });
        for (Primitive& p : sorted.primitives) {
            const std::uint32_t first = static_cast<std::uint32_t>(indices.size());
            if (static_cast<std::size_t>(p.first_index) + p.index_count > level.indices.size()) {
                continue;
            }
            indices.insert(indices.end(), level.indices.begin() + p.first_index,
                           level.indices.begin() + p.first_index + p.index_count);
            p.first_index = first;
        }
        m_opaque[m] = static_cast<std::uint32_t>(m_models.size());
        m_models.push_back(std::move(sorted));
    }

    glGenBuffers(1, &m_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertices);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(level.vertices.size() * sizeof(Vertex)),
        level.vertices.data(),
        GL_STATIC_DRAW
    );
    // Buffers have no type of their own: the indices are filled through the
    // array target, before any vertex array exists to hold the element binding.
    glGenBuffers(1, &m_indices);
    glBindBuffer(GL_ARRAY_BUFFER, m_indices);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
        indices.data(),
        GL_STATIC_DRAW
    );

    if (!level.light_colours.empty()) {
        const std::size_t rows = (level.light_colours.size() + kLightWidth - 1) / kLightWidth;
        std::vector<std::uint32_t> texels(rows * kLightWidth, 0u);
        std::copy(level.light_colours.begin(), level.light_colours.end(), texels.begin());
        glGenTextures(1, &m_lights);
        glBindTexture(GL_TEXTURE_2D, m_lights);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kLightWidth, static_cast<GLsizei>(rows), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, texels.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    for (std::size_t layer = 0; layer < kLayerCount; ++layer) {
        build_groups(static_cast<Layer>(layer), level.instances[layer], false);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

// A class model's vertex array: the level's vertices and indices, and an instance buffer of its own
// (empty until fill_group).
LevelScene::Group LevelScene::make_group(std::uint32_t model) {
    Group g;
    g.model = model;
    glGenVertexArrays(1, &g.vao);
    glBindVertexArray(g.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices);
    constexpr auto stride = static_cast<GLsizei>(sizeof(Vertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, stride, offset(offsetof(Vertex, position))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, stride, offset(offsetof(Vertex, normal))
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, offset(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 4, GL_FLOAT, GL_FALSE, stride, offset(offsetof(Vertex, colour))
    );
    glEnableVertexAttribArray(10);
    glVertexAttribIPointer(10, 4, GL_UNSIGNED_BYTE, stride, offset(offsetof(Vertex, joints)));
    glEnableVertexAttribArray(12);
    glVertexAttribPointer(
        12, 1, GL_FLOAT, GL_FALSE, stride, offset(offsetof(Vertex, light_slot))
    );
    glEnableVertexAttribArray(11);
    glVertexAttribPointer(
        11, 4, GL_FLOAT, GL_FALSE, stride, offset(offsetof(Vertex, weights))
    );
    glGenBuffers(1, &g.instances);
    glBindBuffer(GL_ARRAY_BUFFER, g.instances);
    constexpr auto instance_stride = static_cast<GLsizei>(sizeof(InstanceData));
    for (GLuint column = 0; column < 4; ++column) {
        glEnableVertexAttribArray(4 + column);
        glVertexAttribPointer(
            4 + column,
            4,
            GL_FLOAT,
            GL_FALSE,
            instance_stride,
            offset(column * 4 * sizeof(float))
        );
        glVertexAttribDivisor(4 + column, 1);
    }
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(
        8, 4, GL_FLOAT, GL_FALSE, instance_stride, offset(offsetof(InstanceData, tint))
    );
    glVertexAttribDivisor(8, 1);
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(
        9, 1, GL_FLOAT, GL_FALSE, instance_stride, offset(offsetof(InstanceData, palette))
    );
    glVertexAttribDivisor(9, 1);
    glEnableVertexAttribArray(13);
    glVertexAttribPointer(
        13, 1, GL_FLOAT, GL_FALSE, instance_stride, offset(offsetof(InstanceData, lights))
    );
    glVertexAttribDivisor(13, 1);
    glEnableVertexAttribArray(14);
    glVertexAttribPointer(
        14, 4, GL_FLOAT, GL_FALSE, instance_stride, offset(offsetof(InstanceData, moby_light))
    );
    glVertexAttribDivisor(14, 1);
    glEnableVertexAttribArray(15);
    glVertexAttribPointer(
        15, 4, GL_FLOAT, GL_FALSE, instance_stride, offset(offsetof(InstanceData, moby_ambient))
    );
    glVertexAttribDivisor(15, 1);
    return g;
}

// A group's instances, sent to its buffer. `usage`: GL_STATIC_DRAW for the level's own, sent once;
// GL_STREAM_DRAW for the live ones, sent again every frame (the driver then gives the buffer new
// storage rather than wait for the frame still drawing from the old one).
void LevelScene::fill_group(Group& g, const std::vector<InstanceData>& list, unsigned usage) {
    g.count = static_cast<int>(list.size());
    glBindBuffer(GL_ARRAY_BUFFER, g.instances);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(list.size() * sizeof(InstanceData)),
        list.data(),
        usage
    );
}

void LevelScene::build_groups(Layer layer, const std::vector<Instance>& instances, bool live) {
    // Instances grouped by class model and blend (Instance::blend), in first-seen order. The lists
    // are kept between calls (a live layer is grouped again every frame).
    for (auto& [key, list] : m_grouping) {
        list.clear();
    }
    std::size_t used = 0;
    for (const Instance& instance : instances) {
        const std::uint32_t key = instance.model << 2 | static_cast<std::uint32_t>(instance.blend & 3);
        std::size_t at = 0;
        while (at < used && m_grouping[at].first != key) {
            ++at;
        }
        if (at == used) {
            if (used == m_grouping.size()) {
                m_grouping.emplace_back();
            }
            m_grouping[used++].first = key;
        }
        InstanceData data{};
        std::copy(instance.matrix.begin(), instance.matrix.end(), data.matrix);
        std::copy(instance.tint.begin(), instance.tint.end(), data.tint);
        data.palette = static_cast<float>(instance.palette);
        data.lights = static_cast<float>(instance.lights);
        std::copy(instance.moby_light.begin(), instance.moby_light.end(), data.moby_light);
        std::copy(instance.moby_ambient.begin(), instance.moby_ambient.end(), data.moby_ambient);
        m_grouping[at].second.push_back(data);
    }
    auto& groups = m_groups[static_cast<std::size_t>(layer)];
    auto& idle = m_idle[static_cast<std::size_t>(layer)];
    for (std::size_t i = 0; i < used; ++i) {
        const std::uint32_t model = m_grouping[i].first >> 2;
        // A live layer draws the same classes frame after frame: each keeps its vertex array and
        // buffer, and only the instances are sent again.
        auto kept = std::find_if(idle.begin(), idle.end(), [&](const Group& g) { return g.model == model; });
        Group g;
        if (kept != idle.end()) {
            g = *kept;
            *kept = idle.back();
            idle.pop_back();
        } else {
            g = make_group(model);
        }
        g.blend = static_cast<int>(m_grouping[i].first & 3);
        fill_group(g, m_grouping[i].second, live ? GL_STREAM_DRAW : GL_STATIC_DRAW);
        // A static group's box: its model's box at each of its instances.
        g.bounded = false;
        const Model& shape = m_models[model];
        if (!live && shape.min[0] <= shape.max[0]) {
            constexpr float kHuge = 3.0e38f;
            g.low = {kHuge, kHuge, kHuge};
            g.high = {-kHuge, -kHuge, -kHuge};
            for (const InstanceData& d : m_grouping[i].second) {
                renderer::Mat4 matrix;
                std::copy(std::begin(d.matrix), std::end(d.matrix), matrix.begin());
                for (int corner = 0; corner < 8; ++corner) {
                    const renderer::Vec3 at = renderer::transform_point(
                        matrix, {(corner & 1) != 0 ? shape.max[0] : shape.min[0],
                                 (corner & 2) != 0 ? shape.max[1] : shape.min[1],
                                 (corner & 4) != 0 ? shape.max[2] : shape.min[2]});
                    for (int k = 0; k < 3; ++k) {
                        g.low[k] = std::min(g.low[k], at[k]);
                        g.high[k] = std::max(g.high[k], at[k]);
                    }
                }
            }
            g.bounded = g.count > 0;
        }
        groups.push_back(g);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void LevelScene::set_instances(Layer layer, const std::vector<Instance>& instances) {
    auto& groups = m_groups[static_cast<std::size_t>(layer)];
    auto& idle = m_idle[static_cast<std::size_t>(layer)];
    idle.insert(idle.end(), groups.begin(), groups.end());
    groups.clear();
    build_groups(layer, instances, true);
}

void LevelScene::draw_mobys(const std::vector<Instance>& instances, const renderer::FrameInput& input,
                            renderer::RenderState& state) {
    auto& groups = m_groups[static_cast<std::size_t>(Layer::Mobys)];
    auto& idle = m_idle[static_cast<std::size_t>(Layer::Mobys)];
    std::vector<Group> own;
    own.swap(groups);
    build_groups(Layer::Mobys, instances, true);
    draw_layer(Layer::Mobys, input, state);
    idle.insert(idle.end(), groups.begin(), groups.end());
    groups.swap(own);
}

void LevelScene::set_palette(const std::vector<JointColumns>& matrices) {
    if (matrices.empty()) {
        return;
    }
    if (m_palette == 0) {
        glGenTextures(1, &m_palette);
        glBindTexture(GL_TEXTURE_2D, m_palette);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }
    // Whole rows: the matrices, then zeros to the end of the last row.
    constexpr std::size_t per_row = kPaletteWidth / 4;
    const std::size_t rows = (matrices.size() + per_row - 1) / per_row;
    m_palette_data.assign(rows * per_row, JointColumns{});
    std::copy(matrices.begin(), matrices.end(), m_palette_data.begin());
    glBindTexture(GL_TEXTURE_2D, m_palette);
    // The texture keeps its storage while the palettes fit in it: only its texels are sent.
    if (rows > m_palette_rows) {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            static_cast<GLint>(kRgba32f),
            kPaletteWidth,
            static_cast<GLsizei>(rows),
            0,
            GL_RGBA,
            GL_FLOAT,
            m_palette_data.data()
        );
        m_palette_rows = rows;
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kPaletteWidth, static_cast<GLsizei>(rows), GL_RGBA, GL_FLOAT,
                        m_palette_data.data());
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void LevelScene::release() {
    if (m_palette != 0) {
        glDeleteTextures(1, &m_palette);
        m_palette = 0;
    }
    m_palette_rows = 0;
    for (auto* all : {&m_groups, &m_idle}) {
        for (auto& groups : *all) {
            for (Group& g : groups) {
                glDeleteVertexArrays(1, &g.vao);
                glDeleteBuffers(1, &g.instances);
            }
            groups.clear();
        }
    }
    if (m_vertices != 0) {
        glDeleteBuffers(1, &m_vertices);
        m_vertices = 0;
    }
    if (m_indices != 0) {
        glDeleteBuffers(1, &m_indices);
        m_indices = 0;
    }
    if (m_lights != 0) {
        glDeleteTextures(1, &m_lights);
        m_lights = 0;
    }
    m_mesh.release();
    m_sky.release();
}

void LevelScene::set_fog(const std::array<float, 3>& colour, float near_depth, float far_depth,
                         float near_f, float far_f) {
    // UpdateViewContext's terms (renderer/world/fog.h): F = depth x slope + offset, clamped to
    // [far F, near F]. Equal depths or no fog asked for (both F 255) leave it off.
    const float range = far_depth - near_depth;
    if (!(range > 0.0f) || (near_f >= 255.0f && far_f >= 255.0f)) {
        m_fog_colour = {colour[0], colour[1], colour[2], 0.0f};
        return;
    }
    const float slope = (far_f - near_f) / range;
    const float offset = (near_f * far_depth - far_f * near_depth) / range;
    m_fog_colour = {colour[0], colour[1], colour[2], 1.0f};
    m_fog_params = {slope, offset, std::min(near_f, far_f), std::max(near_f, far_f)};
}

void LevelScene::draw_sky(const renderer::FrameInput& input, renderer::RenderState& state) {
    // The shells sit around the camera and never move with it: the view's
    // rotation only. Each is blended over the last, without depth.
    renderer::Mat4 rotation = input.camera.view;
    rotation[12] = rotation[13] = rotation[14] = 0.0f;
    const renderer::Mat4 view_projection = renderer::multiply(input.camera.projection, rotation);
    m_sky.use();
    glUniformMatrix4fv(m_sky.uniform("view_projection"), 1, GL_FALSE, view_projection.data());
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glActiveTexture(GL_TEXTURE0);
    for (const Group& g : m_groups[static_cast<std::size_t>(Layer::Sky)]) {
        glBindVertexArray(g.vao);
        for (const Primitive& p : m_models[g.model].primitives) {
            const Material& m = m_materials[static_cast<std::size_t>(p.material)];
            const renderer::TextureHandle texture =
                m.image >= 0 ? m_textures[static_cast<std::size_t>(m.image)] : m_white;
            glBindTexture(GL_TEXTURE_2D, state.textures.gl_texture(texture));
            // The shell's matrix as the game drew it this tick; its translation is in the game's
            // sky units, the vertices were exported in 1/1024ths of them (editor/sky.py).
            renderer::Mat4 shell = renderer::identity();
            if (p.shell >= 0 && p.shell < 8 && (input.sky_shell_mask & (1u << p.shell)) != 0) {
                shell = input.sky_shells[static_cast<std::size_t>(p.shell)];
                shell[3] = shell[7] = shell[11] = 0.0f;
                shell[12] /= 1024.0f;
                shell[13] /= 1024.0f;
                shell[14] /= 1024.0f;
                shell[15] = 1.0f;
            }
            glUniformMatrix4fv(m_sky.uniform("shell"), 1, GL_FALSE, shell.data());
            glDrawElementsInstanced(
                GL_TRIANGLES,
                static_cast<GLsizei>(p.index_count),
                GL_UNSIGNED_INT,
                offset(p.first_index * sizeof(std::uint32_t)),
                g.count
            );
            ++state.stats.draw_calls;
            state.stats.triangles += p.index_count / 3;
        }
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

void LevelScene::draw_layer(
    Layer layer, const renderer::FrameInput& input, renderer::RenderState& state
) {
    if (layer == Layer::Sky) {
        draw_sky(input, state);
        return;
    }
    const auto& groups = m_groups[static_cast<std::size_t>(layer)];
    if (groups.empty()) {
        return;
    }
    const renderer::Mat4 view_projection =
        renderer::multiply(input.camera.projection, input.camera.view);
    // The static groups the camera does not see are left out: a level's ties and shrubs are
    // hundreds of draws, most of them behind or beside the camera.
    const auto sides = frustum_sides(view_projection);
    m_visible.assign(groups.size(), true);
    for (std::size_t i = 0; i < groups.size(); ++i) {
        m_visible[i] = !groups[i].bounded || !outside(sides, groups[i].low, groups[i].high);
    }
    m_mesh.use();
    glUniformMatrix4fv(m_mesh.uniform("view_projection"), 1, GL_FALSE, view_projection.data());
    // On a target wider than the game's picture, the 2D layer's mobys are cut at the picture's sides
    // (mesh.vert).
    // An ES driver without GL_EXT_clip_cull_distance has no clip planes (renderer/shader.h, es_source).
    const bool cut = layer == Layer::Mobys && state.target.picture_scale_x < 1.0f
        && (!gl::is_es() || gl::has_extension("GL_EXT_clip_cull_distance"));
    glUniform1f(m_mesh.uniform("picture_scale"), state.target.picture_scale_x);
    if (cut) {
        glEnable(GL_CLIP_DISTANCE0);
        glEnable(GL_CLIP_DISTANCE1);
    }
    // The terrain's vertex colours are the game's own lighting (the extractor lights it as
    // LightTfrags does): it gets no viewer sun.
    glUniform1i(m_mesh.uniform("lighting"), lighting && layer != Layer::Terrain ? 1 : 0);
    // The joint palettes (live mobys): unit 1; the textures go on unit 0.
    const bool palettes = layer == Layer::Mobys && m_palette != 0;
    glUniform1i(m_mesh.uniform("skinning"), palettes ? 1 : 0);
    glUniform1i(m_mesh.uniform("palette_texture"), static_cast<GLint>(kPaletteUnit));
    glUniform1i(m_mesh.uniform("light_texture"), static_cast<GLint>(kLightUnit));
    if (m_light_sets.size() == 16 * 16) {
        glUniform4fv(m_mesh.uniform("light_sets"), 64, m_light_sets.data());
    }
    glUniform4f(m_mesh.uniform("fog_colour"), m_fog_colour[0], m_fog_colour[1], m_fog_colour[2],
                m_fog_colour[3]);
    glUniform4f(m_mesh.uniform("fog_params"), m_fog_params[0], m_fog_params[1], m_fog_params[2],
                m_fog_params[3]);
    glUniformMatrix4fv(m_mesh.uniform("view"), 1, GL_FALSE, input.camera.view.data());
    glActiveTexture(GL_TEXTURE0 + kLightUnit);
    glBindTexture(GL_TEXTURE_2D, m_lights);
    if (palettes) {
        glActiveTexture(GL_TEXTURE0 + kPaletteUnit);
        glBindTexture(GL_TEXTURE_2D, m_palette);
    }
    const int cutout = m_mesh.uniform("cutout");
    const int colour = m_mesh.uniform("material_colour");
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);  // reversed depth
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);  // the editor's materials are double-sided
    glActiveTexture(GL_TEXTURE0);
    // Mobys in two passes: their solid faces, then their glow faces blended over them as the GS
    // blends them (ALPHA 0x44: (Cs - Cd) As + Cd), without writing depth.
    // Then the translucent texels of cutout materials, blended the same way, without depth.
    const int glow_pass = m_mesh.uniform("glow_pass");
    const int translucent_pass = m_mesh.uniform("translucent_pass");
    const int effect_pass = m_mesh.uniform("effect_pass");
    const int metal_pass = m_mesh.uniform("metal_pass");
    glUniform1i(effect_pass, 0);
    glUniform1i(metal_pass, 0);
    // Mobys: a fourth pass for those MobyProc blends (Instance::blend), then a fifth for the metal
    // packets (the shine pass), after everything else.
    const int passes = layer == Layer::Mobys ? 5 : 2;
    // The material state the program holds now (flush): none yet.
    int set_material = -1;
    unsigned set_texture = 0;
    int set_cutout = -1;
    decltype(Material::colour) set_colour{};
    bool colour_set = false;
    for (int pass = 0; pass < passes; ++pass) {
        // The blended mobys' pass: their whole models, all texels, at their alpha (blend 1:
        // ALPHA 0x44, (Cs - Cd) As + Cd) or added (blend 2: 0x48, Cs As + Cd), depth tested and
        // not written, as ReRAC draws MobyProc's translucent and additive groups.
        const bool effects = layer == Layer::Mobys && pass == 3;
        // The metal packets: blended over the moby with ALPHA 0x44, depth tested and not written,
        // pulled a little toward the camera (MobyProc adds 1000 to their GS Z; mesh.vert).
        const bool metal = layer == Layer::Mobys && pass == 4;
        const bool translucent = !effects && !metal && pass == (layer == Layer::Mobys ? 2 : 1);
        glUniform1i(metal_pass, metal ? 1 : 0);
        glUniform1i(glow_pass, layer == Layer::Mobys && pass == 1 ? 1 : 0);
        glUniform1i(translucent_pass, translucent ? 1 : 0);
        glUniform1i(effect_pass, effects ? 1 : 0);
        if (pass >= 1) {
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
            glDepthMask(GL_FALSE);
        }
        for (std::size_t gi = 0; gi < groups.size(); ++gi) {
            const Group& g = groups[gi];
            if (!m_visible[gi] || (!metal && (g.blend != 0) != effects)) {
                continue;
            }
            if (effects) {
                glBlendFuncSeparate(GL_SRC_ALPHA, g.blend == 2 ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
            }
            glBindVertexArray(g.vao);
            // Primitives that follow one another in the index buffer with one material are one
            // draw: the same triangles in the same order, in one call instead of several.
            std::uint32_t run_first = 0;
            std::uint32_t run_count = 0;
            int run_material = -1;
            const auto flush = [&] {
                if (run_count == 0) {
                    return;
                }
                // The material's state is set only when it is not the last draw's: a model's parts
                // mostly share a few, and each call into the driver costs more than the test.
                if (run_material != set_material) {
                    const Material& m = m_materials[static_cast<std::size_t>(run_material)];
                    const renderer::TextureHandle texture =
                        m.image >= 0 ? m_textures[static_cast<std::size_t>(m.image)] : m_white;
                    const unsigned gl_texture = state.textures.gl_texture(texture);
                    if (gl_texture != set_texture) {
                        glBindTexture(GL_TEXTURE_2D, gl_texture);
                        set_texture = gl_texture;
                    }
                    if (static_cast<int>(m.cutout) != set_cutout) {
                        glUniform1i(cutout, m.cutout ? 1 : 0);
                        set_cutout = static_cast<int>(m.cutout);
                    }
                    if (!colour_set || m.colour != set_colour) {
                        glUniform4f(colour, m.colour[0], m.colour[1], m.colour[2], m.colour[3]);
                        set_colour = m.colour;
                        colour_set = true;
                    }
                    set_material = run_material;
                }
                glDrawElementsInstanced(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(run_count),
                    GL_UNSIGNED_INT,
                    offset(run_first * sizeof(std::uint32_t)),
                    g.count
                );
                ++state.stats.draw_calls;
                state.stats.triangles +=
                    static_cast<std::uint64_t>(run_count / 3) * static_cast<std::uint64_t>(g.count);
                run_count = 0;
            };
            // The opaque pass of a static layer in material order (m_opaque).
            const std::uint32_t model = pass == 0 && layer != Layer::Mobys && g.model < m_opaque.size()
                                            ? m_opaque[g.model] : g.model;
            for (const Primitive& p : m_models[model].primitives) {
                const bool is_metal = m_materials[static_cast<std::size_t>(p.material)].metal;
                if (is_metal != metal) {
                    continue;
                }
                if (translucent && !m_materials[static_cast<std::size_t>(p.material)].cutout) {
                    continue;
                }
                if (run_count != 0 && p.material == run_material && p.first_index == run_first + run_count) {
                    run_count += p.index_count;
                    continue;
                }
                flush();
                run_first = p.first_index;
                run_count = p.index_count;
                run_material = p.material;
            }
            flush();
        }
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
    if (cut) {
        glDisable(GL_CLIP_DISTANCE0);
        glDisable(GL_CLIP_DISTANCE1);
    }
    if (palettes) {
        glActiveTexture(GL_TEXTURE0 + kPaletteUnit);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
    }
}

LayerRenderer::LayerRenderer(LevelScene& scene, Layer layer)
    : BucketRenderer(layer_name(layer), bucket_of(layer)),
      m_scene(scene),
      m_layer(layer) {}

void LayerRenderer::render(const renderer::FrameInput& input, renderer::RenderState& state) {
    m_scene.draw_layer(m_layer, input, state);
}

}  // namespace openrac::viewer

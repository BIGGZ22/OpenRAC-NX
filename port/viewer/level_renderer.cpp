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
    for (std::size_t i = 0; i < level.images.size(); ++i) {
        m_textures.push_back(
            level.images[i].width > 0
                ? textures
                      .add(level.images[i], renderer::AlphaScale::Full, level.image_paths[i], true)
                : m_white
        );
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
        static_cast<GLsizeiptr>(level.indices.size() * sizeof(std::uint32_t)),
        level.indices.data(),
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
    // Instances grouped by class model, in first-seen order. The lists are kept between calls (a
    // live layer is grouped again every frame).
    for (auto& [model, list] : m_grouping) {
        list.clear();
    }
    std::size_t used = 0;
    for (const Instance& instance : instances) {
        std::size_t at = 0;
        while (at < used && m_grouping[at].first != instance.model) {
            ++at;
        }
        if (at == used) {
            if (used == m_grouping.size()) {
                m_grouping.emplace_back();
            }
            m_grouping[used++].first = instance.model;
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
        const std::uint32_t model = m_grouping[i].first;
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
        fill_group(g, m_grouping[i].second, live ? GL_STREAM_DRAW : GL_STATIC_DRAW);
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
    m_mesh.use();
    glUniformMatrix4fv(m_mesh.uniform("view_projection"), 1, GL_FALSE, view_projection.data());
    // On a target wider than the game's picture, the 2D layer's mobys are cut at the picture's sides
    // (mesh.vert).
    const bool cut = layer == Layer::Mobys && state.target.picture_scale_x < 1.0f;
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
    const int passes = layer == Layer::Mobys ? 3 : 2;
    // The material state the program holds now (flush): none yet.
    int set_material = -1;
    unsigned set_texture = 0;
    int set_cutout = -1;
    decltype(Material::colour) set_colour{};
    bool colour_set = false;
    for (int pass = 0; pass < passes; ++pass) {
        const bool translucent = pass == passes - 1;
        glUniform1i(glow_pass, layer == Layer::Mobys && pass == 1 ? 1 : 0);
        glUniform1i(translucent_pass, translucent ? 1 : 0);
        if (pass >= 1) {
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
            glDepthMask(GL_FALSE);
        }
        for (const Group& g : groups) {
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
            for (const Primitive& p : m_models[g.model].primitives) {
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

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "scene/level_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>
#include <stdexcept>

#include "common/log.h"
#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/render_opt.h"
#include "renderer/world/gl_world.h"
#include "renderer_shaders.h"
#include "scene/hero_shadow.h"
#include "scene/moby_passes.h"
#include "scene/shadow_atlas.h"
#include "scene_shaders.h"
#if defined(OPENRAC_VULKAN)
#include "renderer/vulkan/vkgl.h"
#endif

namespace openrac::scene {

using namespace openrac::gl;
using renderer::Bucket;

namespace {

bool gpu_driven() {
#if defined(OPENRAC_VULKAN)
    return vkgl::gpu_driven_active();
#else
    return false;
#endif
}

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

// Lets `target` and its GL objects go: a FrameBuffer frees them only in release() (its destructor
// runs where the context may be gone), so a reset alone would keep them on the GPU.
void release_target(std::unique_ptr<renderer::FrameBuffer>& target) {
    // Nothing made yet.
    if (!target) {
        return;
    }

    target->release();
    target.reset();
}

// Returns the textures `materials` draw with, in their order: each one's image and emission image,
// `white` where it has none, as the layer and sky passes pick them (`textures` is by image).
std::vector<renderer::TextureHandle> material_textures(
    const std::vector<Material>& materials,
    const std::vector<renderer::TextureHandle>& textures,
    renderer::TextureHandle white
) {
    const auto of = [&](int image) {
        // A material without an image, or with one the level does not hold, draws white.
        return image >= 0 && static_cast<std::size_t>(image) < textures.size()
                   ? textures[static_cast<std::size_t>(image)]
                   : white;
    };
    std::vector<renderer::TextureHandle> used;
    used.reserve(materials.size() * 2 + 1);  // two a material, and white
    used.push_back(white);

    // Two a material: the colour image, then the emission image (white without one).
    for (const Material& material : materials) {
        used.push_back(of(material.image));
        used.push_back(of(material.emission_image));
    }

    return used;
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
template <std::size_t N>
bool outside(
    const std::array<std::array<float, 4>, N>& planes,
    const renderer::Vec3& low,
    const renderer::Vec3& high,
    std::size_t plane_count = N
) {
    for (std::size_t i = 0; i < plane_count; ++i) {
        const auto& p = planes[i];
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

// ---- Scene upload ----

bool LevelScene::upload(
    const LevelData& level, renderer::TexturePool& textures, std::string& error
) {
    // mesh.frag again with OPENRAC_SOLID, after its version line (which has to stay the first).
    // The registered lighting puts its GLSL into the mesh shader; without it the scene's own.
    const std::string fragment = m_lighting ? m_lighting->mesh_fragment(shaders::mesh_frag)
                                            : std::string(shaders::mesh_frag);
    m_early_capable = m_lighting && m_lighting->early_supported();
    std::string solid(fragment);
    solid.insert(solid.find('\n') + 1, "#define OPENRAC_SOLID 1\n");
    if (!m_mesh.build("mesh", shaders::mesh_vert, fragment, error)
        || !m_solid.build("mesh (solid)", shaders::mesh_vert, solid, error)
        || !m_sky.build("sky", shaders::sky_vert, shaders::sky_frag, error)) {
        return false;
    }
    if (m_early_capable) {
        std::string early = m_lighting->early_fragment(shaders::mesh_frag), early_solid(early);
        early_solid.insert(early_solid.find('\n') + 1, "#define OPENRAC_SOLID 1\n");
        if (!m_early_mesh.build("early lit mesh", shaders::mesh_vert, early, error)
            || !m_early_solid.build("early lit solid", shaders::mesh_vert, early_solid, error)) {
            return false;
        }
    }
#if defined(OPENRAC_VULKAN)
    if (vkgl::gpu_materials_active()) {
        const auto opaque_source = [](std::string source) {
            source.insert(source.find('\n') + 1, "#define OPENRAC_OPAQUE_COLOUR 1\n");
            const auto effect = source.find("uniform int probe_effect;");
            if (effect != std::string::npos) {
                source.replace(
                    effect, std::strlen("uniform int probe_effect;"), "const int probe_effect = 0;"
                );
            }
            source.insert(
                source.find("void main() {"),
                "uniform int opaque_solid;\n"
            );
            return source;
        };
        if (!m_colour_opaque
                 .build("opaque colour batch", shaders::mesh_vert, opaque_source(fragment), error)
            || (m_early_capable
                && !m_early_colour_opaque.build(
                    "early opaque colour batch",
                    shaders::mesh_vert,
                    opaque_source(m_lighting->early_fragment(shaders::mesh_frag)),
                    error
                ))) {
            return false;
        }
        // Keep opaque features constant per native program. In particular,
        // static geometry never needs the joint-palette path, and solid
        // geometry must not inherit the alpha shader's discard instructions.
        const auto fixed_int = [](std::string source, const char* name, int value) {
            const auto declaration = std::string("uniform int ") + name + ";";
            const auto at = source.find(declaration);
            if (at != std::string::npos) {
                source.replace(at, declaration.size(),
                    std::string("const int ") + name + " = " + std::to_string(value) + ";");
            }
            return source;
        };
        for (unsigned variant = 0; variant < m_colour_specialised.size(); ++variant) {
            std::string vertex(shaders::mesh_vert);
            vertex.insert(vertex.find('\n') + 1, "#define OPENRAC_COLOUR_SPECIALISED 1\n");
            vertex = fixed_int(vertex, "metal_pass", 0);
            if (variant < 2) {
                vertex = fixed_int(vertex, "skinning", 0);
            }
            const auto specialise = [&](std::string source) {
                source.insert(source.find('\n') + 1, "#define OPENRAC_COLOUR_SPECIALISED 1\n");
                if (variant == 0) {
                    source.insert(source.find('\n') + 1, "#define OPENRAC_SOLID 1\n");
                }
                for (const auto* name : {"metal_pass", "glow_pass",
                                         "translucent_pass", "effect_pass", "probe_effect"}) {
                    source = fixed_int(std::move(source), name, 0);
                }
                return source;
            };
            const char* names[] = {"solid static colour", "alpha static colour", "opaque moby colour"};
            if (!m_colour_specialised[variant]
                     .build(names[variant], vertex, specialise(fragment), error)
                || (m_early_capable
                    && !m_early_colour_specialised[variant].build(
                        std::string("early ") + names[variant],
                        vertex,
                        specialise(m_lighting->early_fragment(shaders::mesh_frag)),
                        error
                    ))) {
                return false;
            }
        }
        std::string static_vertex(shaders::mesh_vert), static_translucent(fragment);
        static_vertex.insert(static_vertex.find('\n') + 1, "#define OPENRAC_COLOUR_SPECIALISED 1\n");
        static_translucent.insert(static_translucent.find('\n') + 1, "#define OPENRAC_COLOUR_SPECIALISED 1\n");
        static_vertex = fixed_int(std::move(static_vertex), "skinning", 0);
        static_vertex = fixed_int(std::move(static_vertex), "metal_pass", 0);
        for (const auto* name : {"metal_pass", "glow_pass", "effect_pass", "probe_effect"}) {
            static_translucent = fixed_int(std::move(static_translucent), name, 0);
        }
        static_translucent = fixed_int(std::move(static_translucent), "translucent_pass", 1);
        if (!m_static_translucent.build(
                "static translucent colour", static_vertex, static_translucent, error)) {
            return false;
        }
    }
#endif
    std::string depth_vertex(shaders::mesh_depth_vert), depth_fragment(shaders::mesh_depth_frag);
    depth_vertex.insert(depth_vertex.find('\n') + 1, "#define OPENRAC_DEPTH_SOLID 1\n");
    depth_fragment.insert(depth_fragment.find('\n') + 1, "#define OPENRAC_DEPTH_SOLID 1\n");
    if (!m_depth.build("mesh depth", shaders::mesh_depth_vert, shaders::mesh_depth_frag, error)
        || !m_depth_solid.build("mesh depth solid", depth_vertex, depth_fragment, error)) {
        return false;
    }
    // Compatibility rendering retains the minimal depth-only shaders.
    if (gl::enhanced_rendering_available()) {
        const std::string define = "#define OPENRAC_DEPTH_NORMALS 1\n";
        std::string vertex(shaders::mesh_depth_vert);
        std::string fragment(shaders::mesh_depth_frag);
        fragment.insert(fragment.find('\n') + 1, renderer::shaders::shading_normal_glsl);
        depth_fragment
            .insert(depth_fragment.find('\n') + 1, renderer::shaders::shading_normal_glsl);
        vertex.insert(vertex.find('\n') + 1, define);
        fragment.insert(fragment.find('\n') + 1, define);
        depth_vertex.insert(depth_vertex.find('\n') + 1, define);
        depth_fragment.insert(depth_fragment.find('\n') + 1, define);
        if (!m_surface_depth.build("mesh surface depth", vertex, fragment, error)
            || !m_surface_depth_solid
                    .build("mesh solid surface depth", depth_vertex, depth_fragment, error)) {
            return false;
        }
    }
    m_models = level.models;
    for (std::size_t k = 0; k < m_scenery.size(); ++k) {
        Scenery& scenery = m_scenery[k];
        const auto layer = static_cast<std::size_t>(kSceneryLayers[k]);
        const auto& distances = level.draw_distances[layer];
        const bool sized = distances.size() == level.instances[layer].size();
        const bool chosen = sized && !distances.empty();
        scenery.source = chosen ? level.instances[layer] : std::vector<Instance>{};
        scenery.distances = chosen ? distances : std::vector<float>{};
        scenery.centres.clear();
        for (const Instance& instance : scenery.source) {
            scenery.centres.push_back(
                instance.model < level.models.size()
                    ? bounds_centre(level.models[instance.model], instance.matrix)
                    : Vec3{instance.matrix[12], instance.matrix[13], instance.matrix[14]}
            );
        }
        scenery.drawn.clear();
    }
    // A glow packet's vertex (light slot -2, mesh.vert); an index past the vertices counts as one,
    // so that a broken primitive is still drawn.
    const auto glow_vertex = [&level](std::uint32_t i) {
        return i >= level.vertices.size() || level.vertices[i].light_slot < -1.5f;
    };
    // The primitives without a glow packet's vertex, which the mobys' glow pass would draw only to
    // discard every fragment (mesh.frag: v_glow != glow_pass): left out of it (draw_layer).
    for (Model& model : m_models) {
        for (Primitive& p : model.primitives) {
            if (static_cast<std::size_t>(p.first_index) + p.index_count > level.indices.size()) {
                continue;  // kept as drawn
            }
            p.glow_vertices = std::any_of(
                level.indices.begin() + p.first_index,
                level.indices.begin() + p.first_index + p.index_count,
                glow_vertex
            );
        }
    }
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

    m_pvs = gl::is_vulkan() ? level.pvs : Pvs{};
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

    // The glow primitives' triangles with a glow vertex, copied after the rest: a triangle without
    // one has v_glow 0 at its provoking vertex, whichever that is, so the glow pass discards all of
    // it (mesh.frag). The kept triangles keep their order, so what the pass draws is unchanged.
    for (Model& model : m_models) {
        for (Primitive& p : model.primitives) {
            const std::size_t end = static_cast<std::size_t>(p.first_index) + p.index_count;
            if (!p.glow_vertices || p.index_count % 3 != 0 || end > level.indices.size()) {
                continue;
            }
            const auto first = static_cast<std::uint32_t>(indices.size());
            for (std::size_t t = p.first_index; t < end; t += 3) {
                const auto triangle = level.indices.begin() + static_cast<std::ptrdiff_t>(t);
                if (std::any_of(triangle, triangle + 3, glow_vertex)) {
                    indices.insert(indices.end(), triangle, triangle + 3);
                }
            }
            const auto count = static_cast<std::uint32_t>(indices.size()) - first;
            // Every triangle glows: the primitive's own indices serve.
            if (count == p.index_count) {
                indices.resize(first);
                continue;
            }
            p.glow_first = first;
            p.glow_count = count;
        }
    }

    if (gl::is_vulkan()) {
        m_cpu_indices = indices;
        m_pvs_models.resize(m_models.size());
    }
    // A material's opaque flag alone does not guarantee coverage: vertex alpha
    // may still cut holes. Sorted copies use the same vertices and their new indices.
    m_shadow_solid_primitives.clear();
    for (const auto& model : m_models) {
        auto& eligible = m_shadow_solid_primitives.emplace_back();
        for (const auto& primitive : model.primitives) {
            bool solid =
                std::size_t(primitive.first_index) + primitive.index_count <= indices.size();
            for (std::size_t i = primitive.first_index;
                 solid && i < std::size_t(primitive.first_index) + primitive.index_count;
                 ++i) {
                const auto vertex = indices[i];
                solid = vertex < level.vertices.size()
                        && std::isfinite(level.vertices[vertex].colour[3])
                        && level.vertices[vertex].colour[3] >= 0.5f;
            }
            eligible.push_back(solid);
        }
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
    m_moby_layout.valid = false;
    ++m_static_generation;
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // The level's probe bake, found and uploaded by the registered lighting.
    if (m_lighting) {
        m_lighting->load(level);
    }

    // The materials' textures made now, while the level loads, so that its first frames make
    // none (texture-prepare, docs/port/RENDER_PERF.md); off, each is made at its first draw.
    if (renderer::render_opt::enabled(renderer::render_opt::Opt::TexturePrepare)) {
        constexpr double kMiB = 1024.0 * 1024.0;
        const renderer::PreparedTextures made =
            textures.prepare(material_textures(m_materials, m_textures, m_white));
        log::verbose(
            "level textures: {} made at the load, {:.1f} MiB uploaded, {:.1f} MiB with mip chains, "
            "{} batches",
            made.textures,
            static_cast<double>(made.staged_bytes) / kMiB,
            static_cast<double>(made.resident_bytes) / kMiB,
            made.batches
        );
    }

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

// ---- Instance groups ----

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

// Returns whether every instance of `instances` is opaque and lit as scenery, so that the opaque
// shadow shader (no texture, no discard) may draw the group's opaque faces.
bool LevelScene::solid_shadow_instances(const std::vector<InstanceData>& instances) {
    return std::all_of(instances.begin(), instances.end(), [](const InstanceData& data) {
        return std::isfinite(data.tint[3]) && data.tint[3] >= 1.0f && data.moby_light[3] <= 0.5f;
    });
}

// Returns OPENRAC_MOBY_SPHERE_CULL_OFF's switch: animated mobys left unbounded.
bool LevelScene::moby_sphere_culling_off() {
    static const bool& off = stats::probe(
        "OPENRAC_MOBY_SPHERE_CULL_OFF",
        "Profiling reference: leave animated mobys unbounded (next upload)"
    );
    return off;
}

// Returns whether the palette texture's texels `sent` (`texels` of them) are `matrices` followed
// by zeros, bit for bit: what set_palette would send again.
bool LevelScene::same_palette(
    const std::vector<JointColumns>& matrices,
    const std::vector<JointColumns>& sent,
    std::size_t texels
) {
    // Another size: another texture's worth.
    if (sent.size() != texels || matrices.size() > texels) {
        return false;
    }

    const std::size_t bytes = matrices.size() * sizeof(JointColumns);

    // The joints themselves.
    if (std::memcmp(matrices.data(), sent.data(), bytes) != 0) {
        return false;
    }

    // The rest of the last row, zeros when it was sent.
    const JointColumns zero{};
    return std::all_of(
        sent.begin() + static_cast<std::ptrdiff_t>(matrices.size()),
        sent.end(),
        [&](const JointColumns& texel) { return std::memcmp(&texel, &zero, sizeof(zero)) == 0; }
    );
}

// Returns the key build_groups groups `instance` by: its class model, its blend and whether it
// casts the hero's shadow, in that order of significance.
std::uint32_t LevelScene::group_key(const Instance& instance) {
    return instance.model << 3 | (static_cast<std::uint32_t>(instance.blend & 3) << 1)
           | (instance.hero_shadow_caster ? 1u : 0u);
}

// Returns `instance` as the vertex shader's per-instance attributes take it.
LevelScene::InstanceData LevelScene::instance_data(const Instance& instance) {
    InstanceData data{};
    std::copy(instance.matrix.begin(), instance.matrix.end(), data.matrix);
    std::copy(instance.tint.begin(), instance.tint.end(), data.tint);
    data.palette = static_cast<float>(instance.palette);
    data.lights = static_cast<float>(instance.lights);
    std::copy(instance.moby_light.begin(), instance.moby_light.end(), data.moby_light);
    std::copy(instance.moby_ambient.begin(), instance.moby_ambient.end(), data.moby_ambient);
    return data;
}

// Returns the world bounds of one instance of `shape` with `data`: the shape's box through the
// instance's matrix for a rigid mesh, its world `sphere` for an animated one (unless `sphere_off`),
// padded; invalid (unbounded) when either cannot be trusted.
LevelScene::CasterBounds LevelScene::instance_bounds(
    const Model& shape,
    const InstanceData& data,
    const std::array<float, 4>& sphere,
    bool sphere_off
) {
    CasterBounds bounds;
    bounds.low.fill(3e38f);
    bounds.high.fill(-3e38f);
    bounds.valid = data.palette < 0 && shape.min[0] <= shape.max[0];
    renderer::Mat4 matrix;
    std::copy_n(data.matrix, 16, matrix.begin());
    bounds.valid &= matrix[3] == 0 && matrix[7] == 0 && matrix[11] == 0 && matrix[15] == 1;
    for (int corner = 0; corner < 8 && bounds.valid; ++corner) {
        const auto p = renderer::transform_point(
            matrix,
            {(corner & 1) ? shape.max[0] : shape.min[0],
             (corner & 2) ? shape.max[1] : shape.min[1],
             (corner & 4) ? shape.max[2] : shape.min[2]}
        );
        for (int k = 0; k < 3; ++k) {
            bounds.valid &= std::isfinite(p[k]) && shape.min[k] <= shape.max[k];
            bounds.low[k] = std::min(bounds.low[k], p[k]);
            bounds.high[k] = std::max(bounds.high[k], p[k]);
        }
    }
    if (data.palette >= 0 && !sphere_off && data.moby_light[3] <= 1.5f) {
        bounds.valid = sphere[3] > 0 && std::all_of(sphere.begin(), sphere.end(), [](float x) {
                           return std::isfinite(x);
                       });
        for (int k = 0; k < 3 && bounds.valid; ++k) {
            bounds.low[k] = sphere[k] - sphere[3];
            bounds.high[k] = sphere[k] + sphere[3];
            bounds.valid &= std::isfinite(bounds.low[k]) && std::isfinite(bounds.high[k]);
        }
    }
    if (bounds.valid) {
        for (int k = 0; k < 3; ++k) {
            const float pad =
                .001f + 1e-6f * std::max(std::abs(bounds.low[k]), std::abs(bounds.high[k]));
            bounds.low[k] -= pad;
            bounds.high[k] += pad;
        }
    }
    return bounds;
}

// Returns `bounds` as the GPU culling takes them, with the instance's PVS word and its flags
// (valid box, UI layer).
renderer::GpuCullBounds LevelScene::gpu_cull_bounds(
    const CasterBounds& bounds, const InstanceData& data, std::uint16_t occlusion
) {
    return {
        bounds.low,
        bounds.high,
        occlusion,
        (bounds.valid ? 1u : 0u) | (data.moby_light[3] > 1.5f ? 2u : 0u)
    };
}

void LevelScene::build_groups(Layer layer, const std::vector<Instance>& instances, bool live) {
    // Instances grouped by class model and blend (Instance::blend), in first-seen order. The lists
    // are kept between calls (a live layer is grouped again every frame).
    for (auto& group : m_grouping) {
        group.instances.clear();
        group.spheres.clear();
        group.occlusion.clear();
    }
    const bool sphere_off = moby_sphere_culling_off();
    std::size_t used = 0;
    m_group_of_key.clear();

    // Each instance into the list of its key, a new list for a key not seen yet (first-seen order).
    for (const Instance& instance : instances) {
        const std::uint32_t key = group_key(instance);
        const auto [found, fresh] = m_group_of_key.try_emplace(key, used);
        const std::size_t at = found->second;

        // A key not seen yet this call: the next list, made the first time it is needed.
        if (fresh) {
            if (used == m_grouping.size()) {
                m_grouping.emplace_back();
            }
            m_grouping[used++].key = key;
        }

        m_grouping[at].instances.push_back(instance_data(instance));
        m_grouping[at].spheres.push_back(instance.world_sphere);
        m_grouping[at].occlusion.push_back(instance.occlusion);
    }
    auto& groups = m_groups[static_cast<std::size_t>(layer)];
    auto& idle = m_idle[static_cast<std::size_t>(layer)];
    for (std::size_t i = 0; i < used; ++i) {
        const std::uint32_t model = m_grouping[i].key >> 3;
        // A live layer draws the same classes frame after frame: each keeps its vertex array and
        // buffer, and only the instances are sent again.
        auto kept = std::find_if(idle.begin(), idle.end(), [&](const Group& g) {
            return g.model == model;
        });
        Group g;
        if (kept != idle.end()) {
            g = std::move(*kept);
            if (kept != idle.end() - 1) {
                *kept = std::move(idle.back());
            }
            idle.pop_back();
        } else {
            g = make_group(model);
        }
        g.blend = static_cast<int>((m_grouping[i].key >> 1) & 3);
        g.hero_shadow_caster = (m_grouping[i].key & 1) != 0;
        fill_group(g, m_grouping[i].instances, live ? GL_STREAM_DRAW : GL_STATIC_DRAW);
        g.cpu_instances = m_grouping[i].instances;
        g.shadow_solid_instances = solid_shadow_instances(g.cpu_instances);
        g.camera_valid = false;
        g.pvs_valid = false;
        g.occlusion = m_grouping[i].occlusion;
        g.caster_bounds.clear();
        g.gpu_bounds.clear();
        g.caster_bounds.reserve(g.cpu_instances.size());
        const bool native_gpu_bounds = gl::is_vulkan();
        if (native_gpu_bounds) {
            g.gpu_bounds.reserve(g.cpu_instances.size());
        }
        // Rigid meshes keep their tighter native boxes. Animated meshes use the
        // game's current world sphere, never their bind-pose box. Missing or
        // malformed spheres remain unbounded. Refresh on every instance upload.
        for (std::size_t n = 0; n < g.cpu_instances.size(); ++n) {
            const CasterBounds bounds = instance_bounds(
                m_models[model], g.cpu_instances[n], m_grouping[i].spheres[n], sphere_off
            );
            g.caster_bounds.push_back(bounds);
            if (native_gpu_bounds) {
                g.gpu_bounds.push_back(gpu_cull_bounds(bounds, g.cpu_instances[n], g.occlusion[n]));
            }
        }
        // Aggregate the same conservative bounds used by per-instance tests.
        g.bounded = false;
        if (!live
            && std::all_of(
                g.caster_bounds.begin(),
                g.caster_bounds.end(),
                [](const CasterBounds& b) { return b.valid; }
            )) {
            constexpr float kHuge = 3.0e38f;
            g.low = {kHuge, kHuge, kHuge};
            g.high = {-kHuge, -kHuge, -kHuge};
            for (const auto& bound : g.caster_bounds) {
                for (int k = 0; k < 3; ++k) {
                    g.low[k] = std::min(g.low[k], bound.low[k]);
                    g.high[k] = std::max(g.high[k], bound.high[k]);
                }
            }
            g.bounded = g.count > 0;
        }

#if defined(OPENRAC_VULKAN)
        // A static group's instances change only when the layer is set again: registered then, a
        // view's GPU batch of them sends only the view (gpu-resident-instances, RENDER_PERF.md).
        if (!live && native_gpu_bounds
            && renderer::render_opt::enabled(renderer::render_opt::Opt::GpuResidentInstances)) {
            vkgl::gpu_register_instances(g.instances, g.gpu_bounds);
        }
#endif

        groups.push_back(std::move(g));
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

renderer::GpuInstanceBatch LevelScene::gpu_batch(
    const Group& group, const renderer::GpuCullView& view
) {
    static_assert(sizeof(InstanceData) == 120);
#if defined(OPENRAC_VULKAN)
    return vkgl::gpu_instances(group.instances, group.gpu_bounds, view);
#else
    (void)group;
    (void)view;
    return {};
#endif
}

void LevelScene::submit_instances(
    renderer::GpuInstanceBatch batch, unsigned first, unsigned count, int instances
) {
#if defined(OPENRAC_VULKAN)
    if (batch) {
        if (!vkgl::draw_gpu_instances(batch, first, count)) {
            throw std::runtime_error("GPU instance batch expired before draw submission");
        }
        return;
    }
#else
    (void)batch;
#endif
    glDrawElementsInstanced(
        GL_TRIANGLES,
        static_cast<GLsizei>(count),
        GL_UNSIGNED_INT,
        offset(first * sizeof(std::uint32_t)),
        instances
    );
}

void LevelScene::set_instances(Layer layer, const std::vector<Instance>& instances) {
    // The CPU time of handing the scene a layer's instances (the moby layer's, every frame).
    const stats::Scope pass(layer == Layer::Mobys ? "moby instances" : "layer instances");
    if (layer == Layer::Mobys) {
        m_hero_centre.reset();
        for (const auto& instance : instances) {
            if (instance.hero_shadow_caster && instance.moby_light[3] < 1.5f
                && instance.tint[3] > 0) {
                m_hero_centre =
                    renderer::Vec3{instance.matrix[12], instance.matrix[13], instance.matrix[14]};
                break;
            }
        }
        // The body remains the projection anchor even when equipment is submitted first.
        for (const auto& instance : instances) {
            if (instance.hero_shadow_anchor && instance.hero_shadow_caster) {
                m_hero_centre =
                    renderer::Vec3{instance.matrix[12], instance.matrix[13], instance.matrix[14]};
                break;
            }
        }
    }
    // Static instances set again: the static shadow cache's cascades no longer hold them.
    if (layer != Layer::Mobys) {
        ++m_static_generation;
    }

    // The same mobys in the same order as the last time (between two ticks, or a tick that changed
    // no membership): their poses are written into the groups they are in (moby-pose-in-place).
    if (layer == Layer::Mobys
        && renderer::render_opt::enabled(renderer::render_opt::Opt::MobyPoseInPlace)
        && rewrite_mobys(instances)) {
        return;
    }

    auto& groups = m_groups[static_cast<std::size_t>(layer)];
    auto& idle = m_idle[static_cast<std::size_t>(layer)];
    idle.insert(
        idle.end(), std::make_move_iterator(groups.begin()), std::make_move_iterator(groups.end())
    );
    groups.clear();
    // A static layer set again (choose_scenery) keeps its bounds and resident instances.
    build_groups(layer, instances, layer == Layer::Mobys);

    // The moby groups as build_groups made them: where each instance went, for the next call.
    if (layer == Layer::Mobys) {
        remember_mobys(instances);
    }
}

void LevelScene::choose_scenery(
    bool original, const renderer::Vec3& eye, const renderer::Mat4& projection
) {
    // How much farther than its depth a point in the view can be: 1 / the cosine of the view's
    // corner, from the projection's tangents. Within it, a straight-line distance below this times
    // the draw distance holds every instance the game draws.
    const float tx = projection[0] != 0.0f ? 1.0f / projection[0] : 0.0f;
    const float ty = projection[5] != 0.0f ? 1.0f / projection[5] : 0.0f;
    const float reach = std::sqrt(1.0f + tx * tx + ty * ty);
    constexpr float kLeaveMargin = 8.0f;  // game units past its distance before one is left out
    bool rebuilt = false;
    for (std::size_t k = 0; k < m_scenery.size(); ++k) {
        Scenery& scenery = m_scenery[k];
        const Layer layer = kSceneryLayers[k];
        if (scenery.source.empty()) {
            continue;
        }
        // Off again after being on: the instances as loaded, once.
        if (!original) {
            if (!scenery.drawn.empty()) {
                scenery.drawn.clear();
                set_instances(layer, scenery.source);
            }
            continue;
        }
        const bool first = scenery.drawn.empty();
        scenery.drawn.resize(scenery.source.size(), 1);
        bool changed = first;
        for (std::size_t i = 0; i < scenery.source.size(); ++i) {
            const Vec3& centre = scenery.centres[i];
            const Vec3 to{centre[0] - eye[0], centre[1] - eye[1], centre[2] - eye[2]};
            const bool was = !first && scenery.drawn[i] != 0;
            // Squared, so that the test needs no root: limit and limit + margin, both positive.
            const float limit = scenery.distances[i] * reach;
            const float reach_to = was ? limit + kLeaveMargin : limit;
            const float away = to[0] * to[0] + to[1] * to[1] + to[2] * to[2];
            const bool shown =
                limit > 0.0f && (was ? away <= reach_to * reach_to : away < reach_to * reach_to);
            const std::uint8_t drawn = shown ? 1 : 0;
            changed = changed || scenery.drawn[i] != drawn;
            scenery.drawn[i] = drawn;
        }
        if (!changed) {
            continue;
        }
        std::vector<Instance> chosen;
        chosen.reserve(scenery.source.size());
        for (std::size_t i = 0; i < scenery.source.size(); ++i) {
            if (scenery.drawn[i] == 0) {
                continue;
            }
            chosen.push_back(scenery.source[i]);
        }
        set_instances(layer, chosen);
        rebuilt = true;
    }
    // How often the layers are grouped again (a frame's average: the share of frames).
    if (stats::enabled()) {
        stats::gauge("scenery regroups", rebuilt ? 1 : 0);
    }
}

void LevelScene::remember_mobys(const std::vector<Instance>& instances) {
    auto& layout = m_moby_layout;
    const auto& groups = m_groups[static_cast<std::size_t>(Layer::Mobys)];
    layout.keys.resize(instances.size());
    layout.group.resize(instances.size());
    layout.slot.resize(instances.size());
    layout.spheres.resize(instances.size());
    layout.filled.assign(groups.size(), 0);
    layout.sphere_off = moby_sphere_culling_off();

    // Each instance's group, by its key (build_groups' lists are the groups, in order), and its
    // place in it, in the order build_groups appended it.
    for (std::size_t n = 0; n < instances.size(); ++n) {
        const std::uint32_t key = group_key(instances[n]);
        const std::size_t group = m_group_of_key.at(key);
        layout.keys[n] = key;
        layout.group[n] = static_cast<std::uint32_t>(group);
        layout.slot[n] = layout.filled[group]++;
        layout.spheres[n] = instances[n].world_sphere;
    }

    layout.valid = true;
}

bool LevelScene::rewrite_mobys(const std::vector<Instance>& instances) {
    auto& layout = m_moby_layout;
    auto& groups = m_groups[static_cast<std::size_t>(Layer::Mobys)];

    // No layout, one of another count of mobys, or bounds made with the sphere switch otherwise
    // (OPENRAC_MOBY_SPHERE_CULL_OFF changed since): they are grouped again.
    if (!layout.valid || instances.size() != layout.keys.size()
        || layout.filled.size() != groups.size()
        || layout.sphere_off != moby_sphere_culling_off()) {
        return false;
    }

    // A moby of another class, blend or shadow role at a place: the membership changed.
    for (std::size_t n = 0; n < instances.size(); ++n) {
        // Another key at this place: grouped again.
        if (group_key(instances[n]) != layout.keys[n]) {
            return false;
        }
    }

    const bool native_gpu_bounds = gl::is_vulkan();

    // Each moby's attributes into its place; its bounds again only when its pose or sphere moved.
    for (std::size_t n = 0; n < instances.size(); ++n) {
        Group& g = groups[layout.group[n]];
        const std::uint32_t slot = layout.slot[n];
        const InstanceData data = instance_data(instances[n]);
        const auto& sphere = instances[n].world_sphere;
        const bool moved =
            std::memcmp(&g.cpu_instances[slot], &data, sizeof(data)) != 0
            || std::memcmp(layout.spheres[n].data(), sphere.data(), sizeof(sphere)) != 0;
        g.cpu_instances[slot] = data;
        g.occlusion[slot] = instances[n].occlusion;

        // The same pose and sphere give the same bounds as before.
        if (moved) {
            g.caster_bounds[slot] =
                instance_bounds(m_models[g.model], data, sphere, layout.sphere_off);
            layout.spheres[n] = sphere;
        }

        // The GPU's form also carries the PVS word, which may change with the same pose.
        if (native_gpu_bounds) {
            g.gpu_bounds[slot] = gpu_cull_bounds(g.caster_bounds[slot], data, g.occlusion[slot]);
        }
    }

    // Each group's instances sent, and what build_groups derives from them made again.
    for (Group& g : groups) {
        fill_group(g, g.cpu_instances, GL_STREAM_DRAW);
        g.shadow_solid_instances = solid_shadow_instances(g.cpu_instances);
        g.camera_valid = false;
        g.pvs_valid = false;
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

// ---- World drawing ----

void LevelScene::draw_mobys(
    const std::vector<Instance>& instances,
    const renderer::FrameInput& input,
    renderer::RenderState& state
) {
    auto& groups = m_groups[static_cast<std::size_t>(Layer::Mobys)];
    auto& idle = m_idle[static_cast<std::size_t>(Layer::Mobys)];
    std::vector<Group> own;
    own.swap(groups);
    build_groups(Layer::Mobys, instances, true);
    draw_layer(Layer::Mobys, input, state);
    idle.insert(
        idle.end(), std::make_move_iterator(groups.begin()), std::make_move_iterator(groups.end())
    );
    groups.swap(own);
}

void LevelScene::set_palette(const std::vector<JointColumns>& matrices) {
    // The CPU time of the joint palette's upload, every frame the mobys move.
    const stats::Scope pass("moby palette");
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

    // The texture holds these joints already, bit for bit (a still palette): nothing is sent.
    if (renderer::render_opt::enabled(renderer::render_opt::Opt::MobyPoseInPlace)
        && rows <= m_palette_rows && same_palette(matrices, m_palette_data, rows * per_row)) {
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

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

bool LevelScene::enable_hero_shadow(bool enabled, std::string& error) {
    if (enabled && (!enhanced_rendering_available() || !probe_data_available())) {
        error = "Modern shadows require Vulkan, an enhanced build and validated probe data";
        return false;
    }
    m_hero_ready = false;
    m_hero_enabled = false;
    if (!enabled) {
        return true;
    }
    if (!m_hero_depth.id()
        && !m_hero_depth.build(
            "Ratchet shadow depth", shaders::shadow_depth_vert, shaders::hero_shadow_frag, error
        )) {
        return false;
    }
    if (!m_hero_map) {
        m_hero_map = std::make_unique<renderer::FrameBuffer>();
    }
    if (!m_hero_map->id() && !m_hero_map->create(512, 512, error, 1, true)) {
        return false;
    }
    m_hero_enabled = true;
    return true;
}

bool LevelScene::make_shadow_atlas(std::string& error) {
    if (!m_atlas_depth.id()) {
        std::string vertex(shaders::shadow_depth_vert);
        vertex.insert(
            vertex.find('\n') + 1,
            "#extension GL_ARB_shader_viewport_layer_array : require\n"
            "#define OPENRAC_SHADOW_ATLAS 1\n"
        );
        if (!m_atlas_depth.build("atlas shadow depth", vertex, shaders::hero_shadow_frag, error)) {
            return false;
        }
        std::string fragment(shaders::hero_shadow_frag);
        vertex.insert(vertex.find('\n') + 1, "#define OPENRAC_SHADOW_SOLID 1\n");
        fragment.insert(fragment.find('\n') + 1, "#define OPENRAC_SHADOW_SOLID 1\n");
        if (!m_atlas_solid.build("atlas opaque shadow depth", vertex, fragment, error)) {
            m_atlas_depth.release();
            return false;
        }
    }
    if (!m_shadow_atlas) {
        m_shadow_atlas = std::make_unique<renderer::FrameBuffer>();
    }
    if (!m_shadow_atlas->id()
        && !m_shadow_atlas->create(kShadowAtlasSize, kShadowAtlasSize, error, 1, true)) {
        return false;
    }
    // The reference targets are recreated on demand if the user switches
    // paths; retaining both sets would waste shadow-map storage every frame.
    release_target(m_hero_map);
    for (auto& map : m_sun_maps) {
        release_target(map);
    }
    return true;
}

void LevelScene::prepare_shadows(const renderer::FrameInput& input, renderer::RenderState& state) {
    m_shadow_atlas_active = false;
#if defined(OPENRAC_VULKAN)
    static const bool& atlas_off = stats::probe(
        "OPENRAC_SHADOW_ATLAS_OFF", "Reference: separate shadow targets and per-map submission"
    );
    static const bool& layer_reference = stats::probe(
        "OPENRAC_SHADOW_LAYER_REFERENCE", "Reference: flush shadow buckets after each layer"
    );
    static const bool& detail = stats::probe(
        "OPENRAC_SUN_TIMING_DETAIL", "Detailed cascade/layer GPU timers (higher profiling overhead)"
    );
    if (!atlas_off && !layer_reference && !detail && (m_sun_enabled || m_hero_enabled)
        && probe_data_available() && gpu_driven() && vkgl::indexed_viewports_supported(5)) {
        std::string error;
        m_shadow_atlas_active = make_shadow_atlas(error);
        if (!m_shadow_atlas_active) {
            log::warn("shadow atlas unavailable: {}", error);
        }
    }
#endif
    if (!m_shadow_atlas_active) {
        release_target(m_shadow_atlas);
        release_target(m_static_shadow);
        m_static_keys = {};
    }
    prepare_hero_shadow(input, state);
    prepare_sun_shadow(input, state);
    if (m_shadow_atlas_active && (m_hero_ready || m_sun_ready)) {
        draw_shadow_atlas(state);
        report_sun_culling();
    }
    stats::gauge("shadow atlas", m_shadow_atlas_active ? 1 : 0);
}

void LevelScene::draw_shadow_atlas(renderer::RenderState& state) {
#if defined(OPENRAC_VULKAN)
    const stats::Scope pass("shadow atlas", stats::kGpu);
    const auto draws_before = state.stats.draw_calls;
    const auto triangles_before = state.stats.triangles;
    for (unsigned i = 0; i < 5; ++i) {
        glActiveTexture(GL_TEXTURE0 + 6 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    for (unsigned i = 0; i < 4; ++i) {
        m_shadow_view_projection[i] = m_sun_cascades.matrices[i];
    }
    m_shadow_view_projection[4] = m_hero_matrix;
    for (auto& bucket : m_shadow_draws) {
        bucket.clear();
    }
    const bool cache =
        m_sun_ready && renderer::render_opt::enabled(renderer::render_opt::Opt::ShadowStaticCache);
    unsigned rerendered = m_sun_ready ? kSunCascades : 0;

    // Nothing samples the atlas's colour or reads its stencil: neither is loaded or written back
    // (shadow-atlas-discard; the depth is cleared with the stencil so it is not loaded either).
    const bool discard =
        glInvalidateFramebuffer != nullptr
        && renderer::render_opt::enabled(renderer::render_opt::Opt::ShadowAtlasDiscard);
    const GLenum unread[] = {GL_COLOR_ATTACHMENT0, GL_STENCIL_ATTACHMENT};

    // The static casters come from the cache, the atlas holding its copy; otherwise it is cleared.
    if (cache) {
        rerendered = draw_cached_cascades(state);
    } else {
        release_target(m_static_shadow);
        m_static_keys = {};
        begin_shadow_target(m_shadow_atlas->id());
        glClearDepth(0);

        // The colour let go before the pass, the stencil cleared with the depth: nothing loaded.
        if (discard) {
            glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, unread);
            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        } else {
            glClear(GL_DEPTH_BUFFER_BIT);
        }
    }

    vkgl::indexed_viewports(kShadowAtlasRects);
    if (m_sun_ready) {
        for (unsigned i = 0; i < 4; ++i) {
            draw_shadow_map(
                m_shadow_atlas->id(),
                kSunResolutions[i],
                m_sun_cascades.matrices[i],
                false,
                m_hero_ready,
                state,
                i,
                static_cast<int>(i),
                i == 3 && !m_hero_ready,
                cache ? ShadowCasters::Dynamic : ShadowCasters::All
            );
        }
    }
    if (m_hero_ready) {
        draw_shadow_map(m_shadow_atlas->id(), 512, m_hero_matrix, true, false, state, 4, 4, true);
    }
    vkgl::indexed_viewports({});
    glBindVertexArray(0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Only the depth is sampled: the colour and the stencil are not written back.
    if (discard) {
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, unread);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, state.target.framebuffer);
    glViewport(0, 0, state.target.width, state.target.height);
    // What the atlas cost this frame, cache or not, for RENDER_PERF.md's R-18 comparison.
    stats::gauge("vk_shadow_draws", static_cast<double>(state.stats.draw_calls - draws_before));
    stats::gauge(
        "vk_shadow_triangles", static_cast<double>(state.stats.triangles - triangles_before)
    );
    stats::gauge("shadow_static_rerenders", static_cast<double>(rerendered));
#else
    (void)state;
#endif
}

unsigned LevelScene::draw_cached_cascades(renderer::RenderState& state) {
#if defined(OPENRAC_VULKAN)
    std::array<StaticShadowKey, kSunCascades> keys{};
    std::array<bool, kSunCascades> missed{};
    unsigned misses = 0;
    unsigned last = 0;

    // A fresh cache holds nothing: every cascade is drawn into it.
    if (!m_static_shadow || !m_static_shadow->id()) {
        m_static_keys = {};
    }

    // Each cascade's key: what its static casters depend on (sun_shadow.h).
    for (unsigned i = 0; i < kSunCascades; ++i) {
        const auto& volume = m_sun_cascades.casters[i];
        keys[i].valid = true;
        keys[i].matrix = m_sun_cascades.matrices[i];
        keys[i].planes = volume.planes;
        keys[i].plane_count = volume.plane_count;
        keys[i].separate_hero = m_hero_ready;
        keys[i].generation = m_static_generation;
        missed[i] = !same_static_key(keys[i], m_static_keys[i]);

        // A changed key draws again; the last one drawn flushes the buckets.
        if (missed[i]) {
            ++misses;
            last = i;
        }
    }

    // A cascade changed: its rectangle of the cache is cleared and its static casters drawn.
    if (misses > 0) {
        const bool fresh = !m_static_shadow || !m_static_shadow->id();

        // The cache is made as the atlas is, so that its depth format and size are the same.
        if (fresh) {
            m_static_shadow = std::make_unique<renderer::FrameBuffer>();
            std::string error;

            // No memory for it: the atlas draws every caster, as with the entry off.
            if (!m_static_shadow->create(kShadowAtlasSize, kShadowAtlasSize, error, 1, true)) {
                log::warn("static shadow cache unavailable: {}", error);
                release_target(m_static_shadow);
                begin_shadow_target(m_shadow_atlas->id());
                glClearDepth(0);
                glClear(GL_DEPTH_BUFFER_BIT);
                return kSunCascades;
            }
        }

        begin_shadow_target(m_static_shadow->id());
        glClearDepth(0);

        // A new cache is cleared whole, as the atlas is: its hero and unused texels stay 0.
        if (fresh) {
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        // An old one only where a cascade is drawn again.
        if (!fresh) {
            glEnable(GL_SCISSOR_TEST);

            // Each changed cascade's rectangle.
            for (unsigned i = 0; i < kSunCascades; ++i) {
                // Unchanged: its rectangle is kept.
                if (!missed[i]) {
                    continue;
                }

                const auto& rect = kShadowAtlasRects[i];
                glScissor(rect[0], rect[1], rect[2], rect[3]);
                glClear(GL_DEPTH_BUFFER_BIT);
            }

            glDisable(GL_SCISSOR_TEST);
        }

        vkgl::indexed_viewports(kShadowAtlasRects);

        // The static casters of each changed cascade, with the atlas's own shaders and views.
        for (unsigned i = 0; i < kSunCascades; ++i) {
            // Unchanged: copied from the cache as it is.
            if (!missed[i]) {
                continue;
            }

            draw_shadow_map(
                m_static_shadow->id(),
                kSunResolutions[i],
                m_sun_cascades.matrices[i],
                false,
                m_hero_ready,
                state,
                i,
                static_cast<int>(i),
                i == last,
                ShadowCasters::Static
            );
            m_static_keys[i] = keys[i];
        }

        vkgl::indexed_viewports({});
    }

    // The atlas starts as the cache: the static depth, 0 where nothing static is (unchanged
    // copy: vkCmdCopyImage of the depth aspect).
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_static_shadow->id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadow_atlas->id());
    glBlitFramebuffer(
        0,
        0,
        kShadowAtlasSize,
        kShadowAtlasSize,
        0,
        0,
        kShadowAtlasSize,
        kShadowAtlasSize,
        GL_DEPTH_BUFFER_BIT,
        GL_NEAREST
    );
    begin_shadow_target(m_shadow_atlas->id());
    return misses;
#else
    (void)state;
    return 0;
#endif
}

void LevelScene::begin_shadow_target(unsigned framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, kShadowAtlasSize, kShadowAtlasSize);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_TRUE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void LevelScene::prepare_hero_shadow(
    const renderer::FrameInput& input, renderer::RenderState& state
) {
    m_hero_ready = false;
    if (!probe_data_available() || !m_hero_enabled || !input.hero_pixel_shadow || !m_hero_centre
        || m_lighting->mode() == 2 || m_lighting->mode() == 4) {
        m_hero_transition = {};
        return;
    }
    renderer::Vec3 direction{0, 0, -1};
    if (m_light_sets.size() == 256) {
        std::copy_n(m_light_sets.data() + 4, 3, direction.data());
    }
    direction = m_lighting->sunlight_direction(direction);
    const float blend = hero_shadow_blend(input.hero_airborne, input.seconds, m_hero_transition);
    const auto projection = hero_shadow_projection(*m_hero_centre, direction, blend);
    m_hero_matrix = projection.matrix;
    m_hero_airborne = input.hero_airborne;
    if (!m_hero_logged_airborne || *m_hero_logged_airborne != m_hero_airborne) {
        log::info(
            "Ratchet shadow map: {}, travel direction {:.3f} {:.3f} {:.3f}",
            m_hero_airborne ? "airborne/downward" : "grounded/main light",
            projection.direction[0],
            projection.direction[1],
            projection.direction[2]
        );
        m_hero_logged_airborne = m_hero_airborne;
    }
    glActiveTexture(GL_TEXTURE0 + 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!m_shadow_atlas_active) {
        if (!m_hero_map) {
            m_hero_map = std::make_unique<renderer::FrameBuffer>();
        }
        std::string error;
        if (!m_hero_map->id() && !m_hero_map->create(512, 512, error, 1, true)) {
            log::warn("hero shadow target unavailable: {}", error);
            return;
        }
        draw_shadow_map(m_hero_map->id(), 512, m_hero_matrix, true, false, state);
    }
    m_hero_ready = true;
}

bool LevelScene::enable_sun_shadow(bool enabled, std::string& error) {
    if (enabled && (!enhanced_rendering_available() || !probe_data_available())) {
        error = "Modern shadows require Vulkan, an enhanced build and validated probe data";
        return false;
    }
    m_sun_enabled = m_sun_ready = false;
    if (!enabled) {
        return true;
    }
    if (!m_hero_depth.id()
        && !m_hero_depth.build(
            "modern shadow depth", shaders::shadow_depth_vert, shaders::hero_shadow_frag, error
        )) {
        return false;
    }
    if (!m_shadow_solid.id()) {
        std::string vertex(shaders::shadow_depth_vert), fragment(shaders::hero_shadow_frag);
        vertex.insert(vertex.find('\n') + 1, "#define OPENRAC_SHADOW_SOLID 1\n");
        fragment.insert(fragment.find('\n') + 1, "#define OPENRAC_SHADOW_SOLID 1\n");
        if (!m_shadow_solid.build("opaque static shadow depth", vertex, fragment, error)) {
            return false;
        }
    }
    for (unsigned i = 0; i < kSunCascades; ++i) {
        auto& map = m_sun_maps[i];
        if (!map) {
            map = std::make_unique<renderer::FrameBuffer>();
        }
        if (!map->id() && !map->create(kSunResolutions[i], kSunResolutions[i], error, 1, true)) {
            return false;
        }
    }
    m_sun_enabled = true;
    return true;
}

void LevelScene::prepare_sun_shadow(
    const renderer::FrameInput& input, renderer::RenderState& state
) {
    m_sun_ready = false;
    m_sun_culling = {};
    if (!probe_data_available() || !m_sun_enabled || !input.sun_pixel_shadow
        || m_lighting->mode() == 2 || m_lighting->mode() == 4) {
        return;
    }
    renderer::Vec3 direction{0, 0, -1}, low{3e38f, 3e38f, 3e38f}, high{-3e38f, -3e38f, -3e38f};
    if (m_light_sets.size() == 256) {
        std::copy_n(m_light_sets.data() + 4, 3, direction.data());
    }
    for (std::size_t layer = 1; layer < kLayerCount; ++layer) {
        for (const auto& group : m_groups[layer]) {
            if (!group.bounded) {
                continue;
            }
            for (int k = 0; k < 3; ++k) {
                low[k] = std::min(low[k], group.low[k]);
                high[k] = std::max(high[k], group.high[k]);
            }
        }
    }
    try {
        direction = m_lighting->sunlight_direction(direction);
        m_sun_cascades = sun_cascades(input.camera, direction, low, high);
    } catch (const std::exception&) {
        return;
    }  // invalid/transitional camera: leave surfaces lit
    for (unsigned i = 0; i < kSunCascades; ++i) {
        glActiveTexture(GL_TEXTURE0 + 7 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    if (!m_shadow_atlas_active) {
        for (unsigned i = 0; i < kSunCascades; ++i) {
            if (!m_sun_maps[i]) {
                m_sun_maps[i] = std::make_unique<renderer::FrameBuffer>();
            }
            std::string error;
            if (!m_sun_maps[i]->id()
                && !m_sun_maps[i]->create(kSunResolutions[i], kSunResolutions[i], error, 1, true)) {
                log::warn("sun shadow target unavailable: {}", error);
                return;
            }
            draw_shadow_map(
                m_sun_maps[i]->id(),
                kSunResolutions[i],
                m_sun_cascades.matrices[i],
                false,
                m_hero_ready,
                state,
                i
            );
        }
    }
    m_sun_ready = true;
    if (!m_shadow_atlas_active) {
        report_sun_culling();
    }
}

void LevelScene::report_sun_culling() const {
    static constexpr const char* names[] =
        {"sun cascade 0 culled",
         "sun cascade 1 culled",
         "sun cascade 2 culled",
         "sun cascade 3 culled"};
    std::uint64_t candidates = 0, rejected = 0, submitted = 0, compacted = 0, unbounded = 0;
    std::uint64_t solid_draws = 0, alpha_draws = 0;
    for (unsigned i = 0; i < kSunCascades; ++i) {
        const auto& count = m_sun_culling[i];
        stats::gauge(names[i], static_cast<double>(count.rejected));
        candidates += count.candidates;
        rejected += count.rejected;
        submitted += count.submitted;
        compacted += count.compacted_groups;
        unbounded += count.unbounded;
        solid_draws += count.solid_draws;
        alpha_draws += count.alpha_draws;
    }
    stats::gauge("sun instance candidates", static_cast<double>(candidates));
    stats::gauge("sun instances culled", static_cast<double>(rejected));
    stats::gauge("sun instances submitted", static_cast<double>(submitted));
    stats::gauge("sun solid draws", static_cast<double>(solid_draws));
    stats::gauge("sun alpha draws", static_cast<double>(alpha_draws));
    stats::gauge("sun compacted groups", static_cast<double>(compacted));
    stats::gauge("sun unbounded instances", static_cast<double>(unbounded));
}

void LevelScene::draw_shadow_map(
    unsigned target,
    int resolution,
    const renderer::Mat4& matrix,
    bool hero,
    bool separate_hero,
    renderer::RenderState& state,
    unsigned cascade,
    int atlas_view,
    bool flush_atlas,
    ShadowCasters casters
) {
    static const bool& detailed_timing = stats::probe(
        "OPENRAC_SUN_TIMING_DETAIL", "Detailed cascade/layer GPU timers (higher profiling overhead)"
    );
    static constexpr const char* map_names[] = {"sun0 map", "sun1 map", "sun2 map", "sun3 map"};
    const stats::Scope pass(
        hero ? "Ratchet shadow" : (cascade < kSunCascades ? map_names[cascade] : "sun shadow map"),
        atlas_view < 0 && (hero || !detailed_timing) ? stats::kGpu : 0
    );
    if (atlas_view < 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, target);
        glViewport(0, 0, resolution, resolution);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GEQUAL);
        glDepthMask(GL_TRUE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        {
            const stats::Scope clear("shadow clear", !hero && detailed_timing ? stats::kGpu : 0);
            glClearDepth(0);
            glClear(GL_DEPTH_BUFFER_BIT);
        }
    }
    const renderer::Shader* program = nullptr;
    int set_skinning = -1;
    static const bool& solid_off = stats::probe(
        "OPENRAC_SHADOW_SOLID_OFF",
        "Profiling reference: alpha-capable shader for opaque static shadow casters"
    );
    const auto use = [&](bool solid, bool skinning) {
        const auto& shader = solid ? m_shadow_solid : m_hero_depth;
        if (program != &shader) {
            shader.use();
            glUniformMatrix4fv(shader.uniform("view_projection"), 1, GL_FALSE, matrix.data());
            if (!solid) {
                glUniform1i(shader.uniform("palette_texture"), 1);
                glUniform1i(shader.uniform("tex_T0"), 0);
            }
            program = &shader;
            set_skinning = -1;
        }
        if (!solid && set_skinning != int(skinning)) {
            glUniform1i(shader.uniform("skinning"), skinning ? 1 : 0);
            set_skinning = int(skinning);
        }
    };
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_palette);
    glActiveTexture(GL_TEXTURE0);
    const auto sides = frustum_sides(matrix);
    static const bool& receiver_culling_off =
        stats::probe("OPENRAC_SUN_RECEIVER_CULL_OFF", "Reference: square shadow-map caster bounds");
    const auto* volume = gl::is_vulkan() && !hero && !receiver_culling_off && cascade < kSunCascades
                                 && m_sun_cascades.casters[cascade].plane_count
                             ? &m_sun_cascades.casters[cascade]
                             : nullptr;
    const auto outside_casters = [&](const renderer::Vec3& low, const renderer::Vec3& high) {
        return volume ? outside(volume->planes, low, high, volume->plane_count)
                      : outside(sides, low, high);
    };
    if (!hero) {
        stats::gauge("sun receiver culling", volume ? 1 : 0);
    }
    const bool gpu = gpu_driven();
    renderer::GpuCullView gpu_view;
    gpu_view.bypass_ui = false;
    std::copy(sides.begin(), sides.end(), gpu_view.planes.begin());
    gpu_view.plane_count = 4;
    if (volume) {
        gpu_view.planes = volume->planes;
        gpu_view.plane_count = volume->plane_count;
    }
    static const bool& culling_off = stats::probe(
        "OPENRAC_SUN_INSTANCE_CULL_OFF",
        "Profiling reference: retain every instance in a surviving cascade group"
    );
    auto* counts = !hero && cascade < kSunCascades ? &m_sun_culling[cascade] : nullptr;
    static const bool& batch_off =
        stats::probe("OPENRAC_SHADOW_BATCH_OFF", "Reference: submit shadow casters in model order");
    auto& shadow_draws = m_shadow_draws;
    const auto submit = [&](const ShadowDraw& draw) {
        if (atlas_view >= 0) {
            const auto& shader = draw.solid ? m_atlas_solid : m_atlas_depth;
            if (program != &shader) {
                shader.use();
                glUniformMatrix4fv(
                    shader.uniform("shadow_view_projection"),
                    5,
                    GL_FALSE,
                    m_shadow_view_projection[0].data()
                );
                glUniform1i(shader.uniform("palette_texture"), 1);
                glUniform1i(shader.uniform("tex_T0"), 0);
                program = &shader;
                set_skinning = -1;
            }
            glUniform1i(shader.uniform("shadow_view"), draw.view);
            if (!draw.solid && set_skinning != int(draw.skinning)) {
                glUniform1i(shader.uniform("skinning"), draw.skinning ? 1 : 0);
                set_skinning = int(draw.skinning);
            }
        } else {
            use(draw.solid, draw.skinning);
        }
        glBindVertexArray(draw.vao);
        if (!draw.solid) {
            const auto& shader = atlas_view >= 0 ? m_atlas_depth : m_hero_depth;
            const Material* material = draw.material >= 0 ? &m_materials[draw.material] : nullptr;
            const auto texture = material && material->image >= 0
                                     ? m_textures[static_cast<std::size_t>(material->image)]
                                     : m_white;
            glBindTexture(GL_TEXTURE_2D, state.textures.gl_texture(texture));
            glUniform1i(shader.uniform("cutout"), material && material->cutout ? 1 : 0);
            if (material) {
                glUniform4fv(shader.uniform("material_colour"), 1, material->colour.data());
            } else {
                glUniform4f(shader.uniform("material_colour"), 1, 1, 1, 1);
            }
        }
        submit_instances(draw.batch, draw.first, draw.count, draw.instances);
        ++state.stats.draw_calls;
        state.stats.triangles += std::uint64_t{draw.count / 3} * draw.instances;
    };
    const auto flush_buckets = [&] {
        for (auto& bucket : shadow_draws) {
            for (const auto& command : bucket) {
                submit(command);
            }
            bucket.clear();
        }
    };
    static const bool& layer_reference = stats::probe(
        "OPENRAC_SHADOW_LAYER_REFERENCE", "Reference: flush shadow buckets after each layer"
    );
    for (std::size_t layer = 1; layer < kLayerCount; ++layer) {
        if (hero && layer != static_cast<std::size_t>(Layer::Mobys)) {
            continue;
        }

        const bool moby_layer = layer == static_cast<std::size_t>(Layer::Mobys);

        // The static shadow cache draws the static layers alone, the atlas over it the mobys.
        if ((casters == ShadowCasters::Static && moby_layer)
            || (casters == ShadowCasters::Dynamic && !moby_layer)) {
            continue;
        }

        static constexpr const char* names[kSunCascades][4] =
            {{"sun0 terrain", "sun0 ties", "sun0 shrubs", "sun0 mobys"},
             {"sun1 terrain", "sun1 ties", "sun1 shrubs", "sun1 mobys"},
             {"sun2 terrain", "sun2 ties", "sun2 shrubs", "sun2 mobys"},
             {"sun3 terrain", "sun3 ties", "sun3 shrubs", "sun3 mobys"}};
        std::optional<stats::Scope> layer_pass;
        if (!hero && cascade < kSunCascades) {
            layer_pass.emplace(names[cascade][layer - 1], detailed_timing ? stats::kGpu : 0);
        }
        const bool skinning = layer == static_cast<std::size_t>(Layer::Mobys) && m_palette;
        for (const auto& group : m_groups[layer]) {
            if ((hero && !group.hero_shadow_caster)
                || (!hero && separate_hero && group.hero_shadow_caster) || group.blend != 0) {
                continue;
            }
            if (counts) {
                counts->candidates += group.count;
            }
            if (group.bounded
                && (outside(sides, group.low, group.high)
                    || (volume && outside_casters(group.low, group.high)))) {
                if (counts) {
                    counts->rejected += group.count;
                }
                continue;
            }
            unsigned vao = group.vao;
            int instances = group.count;
            renderer::GpuInstanceBatch batch;
            if (gpu) {
                gpu_view.plane_count =
                    !hero && !culling_off ? (volume ? volume->plane_count : 4u) : 0u;
                batch = gpu_batch(group, gpu_view);
            }
            if (!batch && !hero && !culling_off) {
                m_shadow_survivors.clear();
                for (std::size_t i = 0; i < group.cpu_instances.size(); ++i) {
                    const auto& bounds = group.caster_bounds[i];
                    if (bounds.valid && outside_casters(bounds.low, bounds.high)) {
                        if (counts) {
                            ++counts->rejected;
                        }
                        continue;
                    }
                    if (counts && !bounds.valid) {
                        ++counts->unbounded;
                    }
                    m_shadow_survivors.push_back(group.cpu_instances[i]);
                }
                instances = static_cast<int>(m_shadow_survivors.size());
                if (!instances) {
                    continue;
                }
                if (instances != group.count) {
                    if (!m_shadow_compact.vao) {
                        m_shadow_compact = make_group(group.model);
                    }
                    fill_group(m_shadow_compact, m_shadow_survivors, GL_STREAM_DRAW);
                    vao = m_shadow_compact.vao;
                    if (counts) {
                        ++counts->compacted_groups;
                    }
                }
            }
            if (counts) {
                counts->submitted += instances;
            }
            std::uint32_t first = 0, count = 0;
            bool run_solid = false;
            const auto draw = [&](std::uint32_t start, std::uint32_t size, bool solid, int material
                              ) {
                const ShadowDraw command{
                    vao, instances, batch, start, size, solid, skinning, material, atlas_view
                };
                if (counts) {
                    if (solid) {
                        ++counts->solid_draws;
                    } else {
                        ++counts->alpha_draws;
                    }
                }
                if (batch && !batch_off) {
                    shadow_draws[solid ? 0 : skinning ? 2 : 1].push_back(command);
                } else {
                    // The CPU culling fallback reuses one compact instance buffer;
                    // consume it before the next group can overwrite its contents.
                    submit(command);
                }
            };
            const auto flush = [&] {
                if (!count) {
                    return;
                }
                draw(first, count, run_solid, -1);
                count = 0;
            };
            // Opaque depth needs no material texture. Static copies have
            // contiguous indices, allowing entire terrain runs in one draw.
            const auto model = layer == static_cast<std::size_t>(Layer::Mobys)
                                   ? group.model
                                   : m_opaque[group.model];
            for (std::size_t pi = 0; pi < m_models[model].primitives.size(); ++pi) {
                const auto& primitive = m_models[model].primitives[pi];
                const auto& material = m_materials[static_cast<std::size_t>(primitive.material)];
                if (material.metal || material.blend || material.colour[3] < .5f) {
                    flush();
                    continue;
                }
                if (!material.cutout && material.colour[3] >= 1) {
                    const bool solid =
                        !solid_off && !hero && layer != static_cast<std::size_t>(Layer::Mobys)
                        && group.shadow_solid_instances && m_shadow_solid_primitives[model][pi];
                    if (count && (primitive.first_index != first + count || solid != run_solid)) {
                        flush();
                    }
                    if (!count) {
                        first = primitive.first_index;
                        run_solid = solid;
                    }
                    count += primitive.index_count;
                    continue;
                }
                flush();
                draw(primitive.first_index, primitive.index_count, false, primitive.material);
            }
            flush();
        }
        if (layer_reference && atlas_view < 0) {
            flush_buckets();
        }
    }
    // Depth-only buckets can span layers and, with indexed viewports, maps.
    if (atlas_view < 0 || flush_atlas) {
        flush_buckets();
    }
    if (atlas_view < 0) {
        glBindVertexArray(0);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBindFramebuffer(GL_FRAMEBUFFER, state.target.framebuffer);
        glViewport(0, 0, state.target.width, state.target.height);
    }
}

void LevelScene::release() {
#if defined(OPENRAC_VULKAN)
    // The registered static instances go with the level (once the frames reading them retire).
    vkgl::gpu_release_instances();
#endif
    m_motion_records.release();
    m_motion_records_ready = false;
    m_motion_words.data().clear();
    m_motion_batches.clear();
    m_motion_groups.clear();
    for (auto& map : m_sun_maps) {
        release_target(map);
    }
    m_sun_enabled = m_sun_ready = false;
    glDeleteVertexArrays(1, &m_shadow_compact.vao);
    glDeleteBuffers(1, &m_shadow_compact.instances);
    m_shadow_compact = {};
    m_shadow_survivors.clear();
    m_sun_culling = {};
    release_target(m_hero_map);
    m_hero_depth.release();
    m_shadow_solid.release();
    m_atlas_depth.release();
    m_atlas_solid.release();
    release_target(m_shadow_atlas);
    m_shadow_atlas_active = false;
    release_target(m_static_shadow);
    m_static_keys = {};
    m_moby_layout.valid = false;
    for (auto& bucket : m_shadow_draws) {
        bucket.clear();
    }
    m_shadow_solid_primitives.clear();
    m_hero_enabled = m_hero_ready = false;
    m_hero_logged_airborne.reset();
    m_hero_transition = {};
    m_hero_centre.reset();
    // Only registered lighting has GL objects to free.
    if (m_lighting) {
        m_lighting->release();
    }
    m_surface_depth.release();
    m_surface_depth_solid.release();
    m_early_capable = false;
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
                glDeleteVertexArrays(1, &g.camera_vao);
                glDeleteBuffers(1, &g.camera_instances);
                glDeleteVertexArrays(1, &g.pvs_vao);
                glDeleteBuffers(1, &g.pvs_instances);
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
    m_static_translucent.release();
    m_colour_opaque.release();
    m_early_colour_opaque.release();
    for (auto& shader : m_colour_specialised) shader.release();
    for (auto& shader : m_early_colour_specialised) shader.release();
    m_early_mesh.release();
    m_early_solid.release();
    m_solid.release();
    m_depth.release();
    m_depth_solid.release();
    m_camera_survivors.clear();
    m_camera_culling = {};
    m_pvs = {};
    for (auto& model : m_pvs_models) {
        glDeleteBuffers(1, &model.indices);
    }
    m_pvs_models.clear();
    m_cpu_indices.clear();
    // Nothing to choose from until the next level (choose_scenery).
    m_scenery = {};
    m_sky.release();
}

void LevelScene::set_fog(
    const std::array<float, 3>& colour, float near_depth, float far_depth, float near_f, float far_f
) {
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
            state.textures.bind_upscaled_texture_colours(
                texture, m_sky, input.texture_filter, !m.clamp_s, !m.clamp_t
            );
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

bool LevelScene::needs_early_lighting(const renderer::FrameInput& input) const {
    return probe_data_available() && m_early_capable && m_lighting->early_supported()
           && m_lighting->mode() != 4
           && (m_lighting->active() || (m_sun_enabled && input.sun_pixel_shadow)
               || (m_hero_enabled && input.hero_pixel_shadow));
}

void LevelScene::bind_shadow_atlas(const renderer::Shader& shader) const {
    glUniform1i(shader.uniform("shadow_atlas_enabled"), m_shadow_atlas_active ? 1 : 0);
    if (m_shadow_atlas_active) {
        static const auto packed = [] {
            std::array<float, 20> result{};
            for (unsigned i = 0; i < 5; ++i) {
                for (unsigned j = 0; j < 4; ++j) {
                    result[4 * i + j] = static_cast<float>(kShadowAtlasRects[i][j]);
                }
            }
            return result;
        }();
        glUniform4fv(shader.uniform("shadow_atlas_rects"), 5, packed.data());
    }
}

void LevelScene::bind_hero_shadow(const renderer::Shader& shader) const {
    if (!enhanced_rendering_available()) {
        return;
    }
    glUniform1i(shader.uniform("hero_shadow_enabled"), m_hero_ready ? 1 : 0);
    bind_shadow_atlas(shader);
    glUniform1i(shader.uniform("hero_shadow_airborne"), m_hero_airborne ? 1 : 0);
    glUniform1f(shader.uniform("hero_shadow_air_blend"), m_hero_transition.value);
    glUniform1i(shader.uniform("hero_shadow_map"), 6);
    glUniformMatrix4fv(shader.uniform("hero_shadow_matrix"), 1, GL_FALSE, m_hero_matrix.data());
    if (m_hero_ready) {
        glActiveTexture(GL_TEXTURE0 + 6);
        glBindTexture(
            GL_TEXTURE_2D,
            m_shadow_atlas_active ? m_shadow_atlas->depth_texture() : m_hero_map->depth_texture()
        );
        glActiveTexture(GL_TEXTURE0);
    }
}

void LevelScene::bind_sun_shadow(const renderer::Shader& shader, const renderer::Camera& camera)
    const {
    if (!enhanced_rendering_available()) {
        return;
    }
    glUniform1i(shader.uniform("sun_shadow_enabled"), m_sun_ready ? 1 : 0);
    bind_shadow_atlas(shader);
    glUniform4fv(shader.uniform("sun_shadow_ends"), 1, m_sun_cascades.ends.data());
    glUniform1f(shader.uniform("sun_shadow_near"), m_sun_cascades.near_distance);
    glUniformMatrix4fv(shader.uniform("sun_shadow_view"), 1, GL_FALSE, camera.view.data());
    const char* map_names[] =
        {"sun_shadow_map0", "sun_shadow_map1", "sun_shadow_map2", "sun_shadow_map3"};
    const char* matrix_names[] =
        {"sun_shadow_matrix[0]",
         "sun_shadow_matrix[1]",
         "sun_shadow_matrix[2]",
         "sun_shadow_matrix[3]"};
    for (unsigned i = 0; i < kSunCascades; ++i) {
        glUniform1i(shader.uniform(map_names[i]), static_cast<GLint>(7 + i));
        glUniformMatrix4fv(
            shader.uniform(matrix_names[i]), 1, GL_FALSE, m_sun_cascades.matrices[i].data()
        );
        if (m_sun_ready) {
            glActiveTexture(GL_TEXTURE0 + 7 + i);
            glBindTexture(
                GL_TEXTURE_2D,
                m_shadow_atlas_active ? m_shadow_atlas->depth_texture()
                                      : m_sun_maps[i]->depth_texture()
            );
        }
    }
    glActiveTexture(GL_TEXTURE0);
}

void LevelScene::render_early_lighting(
    unsigned depth, const renderer::FrameInput& input, renderer::RenderState& state
) {
    prepare_shadows(input, state);
    state.world_shadows_prepared = true;
    std::string error;
    // Drawn only where it is needed (needs_early_lighting), which takes registered lighting.
    state.early_lighting =
        m_lighting
        && m_lighting->render_early(
            depth,
            input,
            state.target,
            [&](const renderer::Shader& shader) {
                m_lighting->bind(shader, Layer::Terrain, true, &input.camera, true);
                bind_sun_shadow(shader, input.camera);
                bind_hero_shadow(shader);
                glUniform1i(shader.uniform("early_ao_enabled"), state.gtao_raw != 0 ? 1 : 0);
                glUniform1i(shader.uniform("early_ao"), 18);
                glUniform1i(shader.uniform("early_ao_scale"), state.gtao_scale);
                glUniform1f(
                    shader.uniform("early_ao_strength"), std::clamp(state.gtao_strength, 0.0f, 3.0f)
                );
                glActiveTexture(GL_TEXTURE0 + 18);
                glBindTexture(renderer::world::gl::GL_TEXTURE_BUFFER, state.gtao_raw);
                glActiveTexture(GL_TEXTURE0);
            },
            error
        );
    stats::gauge("early lighting active", state.early_lighting ? 1 : 0);
    if (!error.empty()) {
        log::warn("early lighting: {}", error);
    }
}

bool LevelScene::batches_opaque_colour() const {
#if defined(OPENRAC_VULKAN)
    static const bool& off = stats::probe(
        "OPENRAC_OPAQUE_COLOUR_BATCH_OFF", "Reference: opaque colour in per-layer order"
    );
    return !off && m_colour_opaque.id() && gpu_driven() && vkgl::gpu_materials_active();
#else
    return false;
#endif
}

bool LevelScene::colour_blend_barrier(Layer layer) const {
    // Mobys may have glow, effects or metal passes. Static cutout materials
    // also draw their translucent texels after their opaque texels.
    if (layer == Layer::Mobys) {
        return true;
    }
    for (const auto& group : m_groups[static_cast<std::size_t>(layer)]) {
        if (!group.count) {
            continue;
        }
        for (const auto& primitive : m_models[group.model].primitives) {
            const auto& material = m_materials[static_cast<std::size_t>(primitive.material)];
            if (material.cutout || material.metal || group.blend) {
                return true;
            }
        }
    }
    return false;
}

void LevelScene::draw_layer(
    Layer layer, const renderer::FrameInput& input, renderer::RenderState& state
) {
    if (layer == Layer::Sky) {
        const stats::Scope pass("sky colour", stats::kGpu);
        draw_sky(input, state);
        return;
    }
    if (layer == Layer::Terrain && !state.depth_prepass && !state.world_shadows_prepared) {
        prepare_shadows(input, state);
        state.world_shadows_prepared = true;
    }
    const bool colour_batch = !state.depth_prepass && batches_opaque_colour()
                              && (state.opaque_colour_phase >= 0 || state.opaque_colour_batched);
    const bool reuse_camera = colour_batch && state.opaque_colour_phase != 0;
    const stats::Scope colour(
        layer_name(layer),
        state.depth_prepass || colour_batch || layer == Layer::Mobys || layer == Layer::Ties
            ? 0
            : stats::kGpu
    );
    auto& groups = m_groups[static_cast<std::size_t>(layer)];
    auto& camera_counts = m_camera_culling[static_cast<std::size_t>(layer)];
    if (!reuse_camera) {
        camera_counts = {};
    }
    if (groups.empty()) {
        return;
    }
    const renderer::Mat4 view_projection =
        renderer::multiply(input.camera.projection, input.camera.view);
    // The static groups the camera does not see are left out: a level's ties and shrubs are
    // hundreds of draws, most of them behind or beside the camera.
    const auto sides = frustum_sides(view_projection);
    std::array<std::array<float, 4>, 6> camera_planes;
    std::copy(sides.begin(), sides.end(), camera_planes.begin());
    for (unsigned k = 0; k < 4; ++k) {
        camera_planes[4][k] = view_projection[k * 4 + 3] + view_projection[k * 4 + 2];
        camera_planes[5][k] = view_projection[k * 4 + 3] - view_projection[k * 4 + 2];
    }
    static const bool& camera_culling_off = stats::probe(
        "OPENRAC_CAMERA_INSTANCE_CULL_OFF", "Profiling reference: aggregate-group camera culling"
    );
    auto& visible = m_colour_visible[static_cast<std::size_t>(layer)];
    if (!reuse_camera) {
        visible.assign(groups.size(), true);
    }
    static const bool& pvs_off =
        stats::probe("OPENRAC_PVS_OFF", "Profiling reference: disable baked camera visibility");
    const auto pvs_mask = pvs_off ? Pvs::all_visible() : m_pvs.select(input.camera.position);
    const bool use_pvs = !pvs_off && !m_pvs.grid.empty();
    const bool gpu = gpu_driven();
    renderer::GpuCullView gpu_view;
    gpu_view.planes = camera_planes;
    stats::gauge("gpu driven mesh", gpu ? 1 : 0);
    gpu_view.plane_count = camera_culling_off ? 0u : 6u;
    gpu_view.pvs = pvs_mask;
    gpu_view.use_pvs = use_pvs;
    std::uint64_t pvs_rejected = 0, pvs_primitives_rejected = 0;
    if (!reuse_camera) {
        for (std::size_t i = 0; i < groups.size(); ++i) {
            visible[i] = !groups[i].bounded || !outside(sides, groups[i].low, groups[i].high);
            auto& group = groups[i];
            group.gpu_camera = group.gpu_pvs = {};
            if (gpu && !visible[i]) {
                camera_counts.candidates += group.count;
                camera_counts.rejected += group.count;
                group.pvs_count = group.count;
                group.camera_count = 0;
                continue;
            }
            if (gpu && visible[i]) {
                group.gpu_camera = gpu_batch(group, gpu_view);
                group.pvs_count = group.camera_count = group.count;
                camera_counts.candidates += group.count;
                camera_counts.submitted +=
                    group.count;  // candidate upper bound; GPU counts are asynchronous
                group.camera_valid = group.pvs_valid = false;
                continue;
            }
            if (!use_pvs) {
                if (!group.pvs_valid || group.pvs_mask != pvs_mask) {
                    group.camera_valid = false;
                }
                group.pvs_count = group.count;
                group.pvs_mask = pvs_mask;
                group.pvs_valid = true;
            } else if (!group.pvs_valid || group.pvs_mask != pvs_mask) {
                m_pvs_survivors.clear();
                for (std::size_t n = 0; n < group.cpu_instances.size(); ++n) {
                    if (group.cpu_instances[n].moby_light[3] > 1.5f
                        || Pvs::visible(pvs_mask, group.occlusion[n])) {
                        m_pvs_survivors.push_back(group.cpu_instances[n]);
                    }
                }
                group.pvs_count = static_cast<int>(m_pvs_survivors.size());
                if (group.pvs_count && group.pvs_count != group.count) {
                    Group compact;
                    if (!group.pvs_vao) {
                        compact = make_group(group.model);
                        group.pvs_vao = compact.vao;
                        group.pvs_instances = compact.instances;
                    } else {
                        compact.vao = group.pvs_vao;
                        compact.instances = group.pvs_instances;
                    }
                    fill_group(compact, m_pvs_survivors, GL_STREAM_DRAW);
                }
                group.pvs_mask = pvs_mask;
                group.pvs_valid = true;
                group.camera_valid = false;
            }
            pvs_rejected += group.count - group.pvs_count;
            camera_counts.candidates += group.count;
            if (!visible[i]) {
                camera_counts.rejected += group.count;
                continue;
            }
            if (camera_culling_off) {
                camera_counts.submitted += group.pvs_count;
                camera_counts.rejected += group.count - group.pvs_count;
                continue;
            }
            if (!group.camera_valid || group.camera_clip != view_projection) {
                m_camera_survivors.clear();
                for (std::size_t n = 0; n < group.cpu_instances.size(); ++n) {
                    const auto& bounds = group.caster_bounds[n];
                    const bool ui = group.cpu_instances[n].moby_light[3] > 1.5f;
                    if (use_pvs && !ui && !Pvs::visible(pvs_mask, group.occlusion[n])) {
                        continue;
                    }
                    if (bounds.valid && !ui && outside(camera_planes, bounds.low, bounds.high)) {
                        continue;
                    }
                    m_camera_survivors.push_back(group.cpu_instances[n]);
                }
                group.camera_count = static_cast<int>(m_camera_survivors.size());
                if (group.camera_count && group.camera_count != group.count) {
                    Group compact;
                    if (!group.camera_vao) {
                        compact = make_group(group.model);
                        group.camera_vao = compact.vao;
                        group.camera_instances = compact.instances;
                    } else {
                        compact.vao = group.camera_vao;
                        compact.instances = group.camera_instances;
                    }
                    fill_group(compact, m_camera_survivors, GL_STREAM_DRAW);
                }
                group.camera_clip = view_projection;
                group.camera_valid = true;
            }
            camera_counts.submitted += group.camera_count;
            camera_counts.rejected += group.count - group.camera_count;
            for (const auto& bounds : group.caster_bounds) {
                camera_counts.unbounded += !bounds.valid;
            }
        }
    }
    if (stats::enabled()) {
        const auto name = std::string("camera ") + layer_name(layer);
        stats::gauge(name + " submitted", static_cast<double>(camera_counts.submitted));
        stats::gauge(name + " culled", static_cast<double>(camera_counts.rejected));
    }
    // On a target wider than the game's picture, the 2D layer's mobys are cut at the picture's sides
    // (mesh.vert).
    // An ES driver without GL_EXT_clip_cull_distance has no clip planes (renderer/shader.h, es_source).
    const bool cut = layer == Layer::Mobys && state.target.picture_scale_x < 1.0f
                     && (!gl::is_es() || gl::has_extension("GL_EXT_clip_cull_distance"));
    if (cut) {
        glEnable(GL_CLIP_DISTANCE0);
        glEnable(GL_CLIP_DISTANCE1);
    }
    // The joint palettes (live mobys): unit 1; the textures go on unit 0.
    const bool palettes = layer == Layer::Mobys && m_palette != 0;
    // The frame's uniforms, on the program the next passes draw with: the solid one for the static
    // layers' opaque materials without a cutout (mesh.frag, OPENRAC_SOLID), the general one else.
    const auto use = [&](const renderer::Shader& shader, bool depth_matched) {
        shader.use();
        glUniformMatrix4fv(shader.uniform("view_projection"), 1, GL_FALSE, view_projection.data());
        glUniform1f(shader.uniform("picture_scale"), state.target.picture_scale_x);
        if (state.depth_prepass) {
            glUniformMatrix4fv(shader.uniform("view"), 1, GL_FALSE, input.camera.view.data());
            glUniform1i(shader.uniform("skinning"), palettes ? 1 : 0);
            glUniform1i(shader.uniform("palette_texture"), static_cast<GLint>(kPaletteUnit));
            glUniform1i(shader.uniform("light_texture"), static_cast<GLint>(kLightUnit));
            glUniform1i(shader.uniform("tex_T0"), 0);
            return;
        }
        glUniform1i(shader.uniform("gtao_enabled"), state.gtao_visibility != 0 ? 1 : 0);
        glUniform1i(shader.uniform("gtao_visibility"), 11);
        if (state.gtao_visibility != 0) {
            glActiveTexture(GL_TEXTURE0 + 11);
            glBindTexture(GL_TEXTURE_2D, state.gtao_visibility);
            glActiveTexture(GL_TEXTURE0);
        }
        // The terrain's vertex colours are the game's own lighting (the extractor lights it as
        // LightTfrags does): it gets no viewer sun.
        glUniform1i(shader.uniform("lighting"), lighting && layer != Layer::Terrain ? 1 : 0);
        glUniform1i(shader.uniform("skinning"), palettes ? 1 : 0);
        glUniform1i(shader.uniform("palette_texture"), static_cast<GLint>(kPaletteUnit));
        glUniform1i(shader.uniform("light_texture"), static_cast<GLint>(kLightUnit));
        if (m_light_sets.size() == 16 * 16) {
            glUniform4fv(shader.uniform("light_sets"), 64, m_light_sets.data());
        }
        std::array<float, 32> point_positions{}, point_colours{};
        for (std::size_t i = 0; i < 8; ++i) {
            std::copy_n(m_point_lights.data() + i * 8, 4, point_colours.data() + i * 4);
            std::copy_n(m_point_lights.data() + i * 8 + 4, 4, point_positions.data() + i * 4);
        }
        glUniform4fv(shader.uniform("probe_point_position"), 8, point_positions.data());
        glUniform4fv(shader.uniform("probe_point_colour"), 8, point_colours.data());
        // The registered lighting's uniforms; without it the shader keeps the game's lighting.
        if (m_lighting) {
            m_lighting->bind(
                shader, layer, m_hero_enabled || m_sun_enabled, &input.camera, depth_matched
            );
        }
        bind_sun_shadow(shader, input.camera);
        // The early lighting's result, where registered lighting drew one.
        if (m_lighting) {
            m_lighting->bind_early(shader, input, state);
        }
        bind_hero_shadow(shader);
        glUniform4f(shader.uniform("fog_colour"), m_fog_colour[0], m_fog_colour[1], m_fog_colour[2],
                    m_fog_colour[3]);
        glUniform4f(shader.uniform("fog_params"), m_fog_params[0], m_fog_params[1], m_fog_params[2],
                    m_fog_params[3]);
        glUniformMatrix4fv(shader.uniform("view"), 1, GL_FALSE, input.camera.view.data());
        glUniform1i(shader.uniform("effect_pass"), 0);
        glUniform1i(shader.uniform("metal_pass"), 0);
    };
    glActiveTexture(GL_TEXTURE0 + kLightUnit);
    glBindTexture(GL_TEXTURE_2D, m_lights);
    if (palettes || colour_batch) {
        glActiveTexture(GL_TEXTURE0 + kPaletteUnit);
        glBindTexture(GL_TEXTURE_2D, m_palette);
    }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);  // reversed depth
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);  // the editor's materials are double-sided
    glActiveTexture(GL_TEXTURE0);
    // Mobys in two passes: their solid faces, then their glow faces blended over them as the GS
    // blends them (ALPHA 0x44: (Cs - Cd) As + Cd), without writing depth.
    // Then the translucent texels of cutout materials, blended the same way, without depth.
    // Mobys: a fourth pass for those MobyProc blends (Instance::blend), then a fifth for the metal
    // packets (the shine pass), after everything else.
    // The static layers: their opaque materials without a cutout (solid), then the opaque texels
    // of the cutout ones, then those cutouts' translucent texels.
    const bool mobys = layer == Layer::Mobys;
    const int passes = mobys ? 5 : 3;
    static constexpr const char* moby_pass_names[] =
        {"mobys opaque", "mobys glow", "mobys translucent", "mobys effects", "mobys metal"};
    // The material state the program holds now (flush): none yet.
    int set_material = -1;
    renderer::TextureHandle set_texture = 0;  // bound on unit 0 (and its colours) for `program`
    int set_wrap = -1;                        // and its clamp: bit 0 across, bit 1 down
    unsigned set_emission_texture = 0;
    int set_cutout = -1;
    decltype(Material::colour) set_colour{};
    bool colour_set = false;
    const renderer::Shader* program = nullptr;
    bool program_depth_matched = false;
    // The mobys' glow pass runs twice (scene/moby_passes.h): its texels at or above the GS's alpha
    // reference first, writing depth, then the rest without.
    const int steps = mobys ? static_cast<int>(kMobyPassSteps.size()) : passes;
    for (int step = 0; step < steps; ++step) {
        const MobyPassStep moby_step =
            mobys ? kMobyPassSteps[static_cast<std::size_t>(step)] : MobyPassStep{};
        const int pass = mobys ? moby_step.pass : step;
        const int glow_split = moby_step.glow_split;
        if (state.depth_prepass && (mobys ? pass > 0 : pass > 1)) {
            break;
        }
        if (colour_batch) {
            const bool opaque = mobys ? pass == 0 : pass <= 1;
            if (state.opaque_colour_phase >= 0) {
                if (!opaque || (state.opaque_colour_phase == 0) != (!mobys && pass == 0)) {
                    continue;
                }
            } else if (opaque) {
                continue;
            }
        }
        const bool solid = !mobys && pass == 0;
        const bool reused =
            state.reuse_prepass_depth && !state.depth_prepass && (mobys ? pass == 0 : pass <= 1);
        std::optional<stats::Scope> layer_pass;
        if (layer == Layer::Ties && !state.depth_prepass && state.opaque_colour_phase < 0) {
            static constexpr const char* names[] =
                {"ties solid", "ties cutout", "ties translucent"};
            layer_pass.emplace(names[pass], stats::kGpu);
        }
        if (mobys && !state.depth_prepass && state.opaque_colour_phase < 0) {
            layer_pass.emplace(moby_pass_names[pass], stats::kGpu);
        }
        const bool early = !state.depth_prepass && (mobys ? pass == 0 : pass <= 1) && m_lighting
                           && m_lighting->early_matches(input, state);
        const bool batch_shader = colour_batch && state.opaque_colour_phase >= 0;
        static const bool& specialisation_off = stats::probe(
            "OPENRAC_OPAQUE_SPECIALISE_OFF", "Reference: unspecialised mesh colour shaders"
        );
        const unsigned variant = solid ? 0 : mobys ? 2 : 1;
        const auto& specialised = early ? m_early_colour_specialised : m_colour_specialised;
        const bool static_translucent = !state.depth_prepass && !mobys && pass == 2
                                        && m_static_translucent.id() && !specialisation_off;
        const renderer::Shader* selected = &m_mesh;
        // Both depth variants retain identical positions and alpha coverage.
        if (state.depth_prepass) {
            // Early lighting and AO request smooth normals alongside depth.
            if (state.target.normal_texture != 0) {
                selected = solid ? &m_surface_depth_solid : &m_surface_depth;
            } else {
                selected = solid ? &m_depth_solid : &m_depth;
            }
        } else if (static_translucent) {
            // Static blended packets omit moby-only vertex work.
            selected = &m_static_translucent;
        } else if (batch_shader && !specialisation_off) {
            // Opaque variants specialise alpha handling for each geometry kind.
            selected = &specialised[variant];
        } else if (batch_shader) {
            // The reference batch keeps dynamic alpha handling.
            selected = early ? &m_early_colour_opaque : &m_colour_opaque;
        } else if (early) {
            // Opaque colour consumes the cached compute lighting.
            selected = solid ? &m_early_solid : &m_early_mesh;
        } else {
            // Blended surfaces and unsupported early passes retain fragment lighting.
            selected = solid ? &m_solid : &m_mesh;
        }

        const renderer::Shader& wanted = *selected;
        const bool depth_matched = mobys ? pass == 0 : pass <= 1;
        if (program != &wanted || program_depth_matched != depth_matched) {
            use(wanted, depth_matched);
            program = &wanted;
            program_depth_matched = depth_matched;
            set_material = -1;
            set_texture = 0;
            set_wrap = -1;
            set_cutout = -1;
            colour_set = false;
        }
        const int cutout = program->uniform("cutout");
        const int colour = program->uniform("material_colour");
        glUniform1i(program->uniform("opaque_solid"), solid ? 1 : 0);
        // The cutout materials' opaque texels (static layers).
        const bool cutouts = !mobys && pass == 1;
        // The blended mobys' pass: their whole models, all texels, at their alpha (blend 1:
        // ALPHA 0x44, (Cs - Cd) As + Cd) or added (blend 2: 0x48, Cs As + Cd), depth tested and
        // not written, as ReRAC draws MobyProc's translucent and additive groups.
        const bool effects = mobys && pass == 3;
        // The metal packets: blended over the moby with ALPHA 0x44, depth tested and not written,
        // pulled a little toward the camera (MobyProc adds 1000 to their GS Z; mesh.vert).
        const bool metal = mobys && pass == 4;
        const bool translucent = pass == 2;
        glDepthFunc(reused ? GL_EQUAL : GL_GEQUAL);
        glDepthMask(reused ? GL_FALSE : GL_TRUE);
        glUniform1i(program->uniform("metal_pass"), metal ? 1 : 0);
        glUniform1i(program->uniform("glow_pass"), mobys && pass == 1 ? 1 : 0);
        glUniform1i(program->uniform("translucent_pass"), translucent ? 1 : 0);
        glUniform1i(program->uniform("effect_pass"), effects ? 1 : 0);
        if (mobys ? pass >= 1 : pass >= 2) {
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
            glDepthMask(mobys && moby_step.depth_write ? GL_TRUE : GL_FALSE);
        }
        glUniform1i(program->uniform("glow_split"), glow_split);
        for (std::size_t gi = 0; gi < groups.size(); ++gi) {
            Group& g = groups[gi];
            if (!visible[gi] || (!metal && (g.blend != 0) != effects)) {
                continue;
            }
            const bool culled = !camera_culling_off && (mobys ? !metal : pass <= 1);
            const int instances = culled ? g.camera_count : g.pvs_count;
            if (!instances) {
                continue;
            }
            if (effects) {
                glBlendFuncSeparate(
                    GL_SRC_ALPHA, g.blend == 2 ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE
                );
            }
            glUniform1i(program->uniform("probe_effect"), g.blend == 2 ? 1 : 0);
            glUniform1i(program->uniform("hero_shadow_receiver"), g.hero_shadow_caster ? 1 : 0);
            glBindVertexArray(instances == g.count ? g.vao : culled ? g.camera_vao : g.pvs_vao);
            auto batch = g.gpu_camera;
            if (batch && !culled) {
                if (!g.gpu_pvs) {
                    auto broad = gpu_view;
                    broad.plane_count = 0;
                    g.gpu_pvs = gpu_batch(g, broad);
                }
                batch = g.gpu_pvs;
            }
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
                if (!(state.depth_prepass && solid) && run_material != set_material) {
                    const Material& m = m_materials[static_cast<std::size_t>(run_material)];
                    if (enhanced_rendering_available()) {
                        glUniform3f(
                            program->uniform("material_emission"),
                            m.emission[0],
                            m.emission[1],
                            m.emission[2]
                        );
                        glUniform1i(
                            program->uniform("emission_mask"), static_cast<int>(m.emission_mask)
                        );
                        glUniform1i(
                            program->uniform("emission_vertex_colour"),
                            m.emission_vertex_colour ? 1 : 0
                        );
                        glUniform1f(program->uniform("material_diffuse_scale"), m.diffuse_scale);
                        glUniform1i(program->uniform("emission_texture"), 12);
                        const auto emission_texture =
                            m.emission_image >= 0
                                ? m_textures[static_cast<std::size_t>(m.emission_image)]
                                : m_white;
                        const unsigned gl_emission = state.textures.gl_texture(emission_texture);
                        if (gl_emission != set_emission_texture) {
                            glActiveTexture(GL_TEXTURE0 + 12);
                            glBindTexture(GL_TEXTURE_2D, gl_emission);
                            glActiveTexture(GL_TEXTURE0);
                            set_emission_texture = gl_emission;
                        }
                    }
                    const renderer::TextureHandle texture =
                        m.image >= 0 ? m_textures[static_cast<std::size_t>(m.image)] : m_white;
                    // Materials share textures: rebinding one the program already samples costs
                    // driver calls for nothing. A texture's clamp is its material's.
                    const int wrap = (m.clamp_s ? 1 : 0) | (m.clamp_t ? 2 : 0);
                    if (texture != set_texture || wrap != set_wrap) {
                        state.textures.bind_upscaled_texture_colours(
                            texture, *program, input.texture_filter, !m.clamp_s, !m.clamp_t
                        );
                        set_texture = texture;
                        set_wrap = wrap;
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
                submit_instances(batch, run_first, run_count, instances);
                ++state.stats.draw_calls;
                state.stats.triangles += static_cast<std::uint64_t>(run_count / 3)
                                         * static_cast<std::uint64_t>(instances);
                run_count = 0;
            };
            // The opaque pass of a static layer in material order (m_opaque).
            const std::uint32_t model =
                pass <= 1 && !mobys && g.model < m_opaque.size() ? m_opaque[g.model] : g.model;
            const auto* primitives = &m_models[model].primitives;
            bool compact_indices = false;
            if (use_pvs && layer == Layer::Terrain) {
                auto& cached = m_pvs_models[model];
                if (!cached.valid || cached.mask != pvs_mask) {
                    cached.primitives.clear();
                    cached.rejected = 0;
                    std::vector<std::uint32_t> visible_indices;
                    for (const auto& primitive : *primitives) {
                        if (!Pvs::visible(pvs_mask, primitive.occlusion)) {
                            ++cached.rejected;
                            continue;
                        }
                        auto kept = primitive;
                        kept.first_index = static_cast<std::uint32_t>(visible_indices.size());
                        visible_indices.insert(
                            visible_indices.end(),
                            m_cpu_indices.begin() + primitive.first_index,
                            m_cpu_indices.begin() + primitive.first_index + primitive.index_count
                        );
                        cached.primitives.push_back(kept);
                    }
                    if (cached.rejected && !visible_indices.empty()) {
                        if (!cached.indices) {
                            glGenBuffers(1, &cached.indices);
                        }
                        glBindBuffer(GL_ARRAY_BUFFER, cached.indices);
                        glBufferData(
                            GL_ARRAY_BUFFER,
                            static_cast<GLsizeiptr>(visible_indices.size() * sizeof(std::uint32_t)),
                            visible_indices.data(),
                            GL_DYNAMIC_DRAW
                        );
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    }
                    cached.mask = pvs_mask;
                    cached.valid = true;
                }
                pvs_primitives_rejected += cached.rejected;
                if (cached.rejected) {
                    primitives = &cached.primitives;
                    compact_indices = !primitives->empty();
                    if (compact_indices) {
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cached.indices);
                    }
                }
            }
            for (const Primitive& p : *primitives) {
                const bool is_metal = m_materials[static_cast<std::size_t>(p.material)].metal;
                if (is_metal != metal) {
                    continue;
                }
                // The glow pass draws a glow packet's faces only: the rest would be discarded.
                if (mobys && pass == kMobyGlow && !p.glow_vertices
                    && renderer::render_opt::enabled(renderer::render_opt::Opt::MobyGlowSkip)) {
                    continue;
                }
                const bool is_cutout = m_materials[static_cast<std::size_t>(p.material)].cutout;
                if ((translucent || cutouts) && !is_cutout) {
                    continue;
                }
                if (solid && is_cutout) {
                    continue;
                }
                // Only its glowing triangles there, from the level's own index buffer (a
                // PVS-compacted one holds the whole primitives alone).
                std::uint32_t first = p.first_index;
                std::uint32_t count = p.index_count;
                if (mobys && pass == kMobyGlow && p.glow_count != 0 && !compact_indices
                    && renderer::render_opt::enabled(renderer::render_opt::Opt::MobyGlowSkip)) {
                    first = p.glow_first;
                    count = p.glow_count;
                }
                if (run_count != 0 && (p.material == run_material || (state.depth_prepass && solid))
                    && first == run_first + run_count) {
                    run_count += count;
                    continue;
                }
                flush();
                run_first = first;
                run_count = count;
                run_material = p.material;
            }
            flush();
            if (compact_indices) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices);
            }
        }
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_GEQUAL);
    glBindVertexArray(0);
    if (stats::enabled()) {
        const auto name = std::string("pvs ") + layer_name(layer);
        stats::gauge(name + " instances culled", static_cast<double>(pvs_rejected));
        stats::gauge(
            name + " primitive passes culled", static_cast<double>(pvs_primitives_rejected)
        );
        stats::gauge("pvs enabled", !pvs_off && !m_pvs.grid.empty() ? 1 : 0);
    }
    if (cut) {
        glDisable(GL_CLIP_DISTANCE0);
        glDisable(GL_CLIP_DISTANCE1);
    }
    if (palettes && !colour_batch) {
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
    if (m_layer == Layer::Mobys) {
        m_scene.draw_probe_spheres(input, state);
    }
}

void LayerRenderer::render_depth(const renderer::FrameInput& input, renderer::RenderState& state) {
    if (m_layer != Layer::Sky) {
        m_scene.draw_layer(m_layer, input, state);
    }
}

}  // namespace openrac::scene

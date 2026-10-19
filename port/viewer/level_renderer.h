// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The viewer's renderers: a loaded level on the GPU, drawn through the port's
// renderer (renderer/renderer.h) as one BucketRenderer per layer, in the
// game's order: the sky shells, then terrain, ties, shrubs and mobys.
//
// Every mesh shares one vertex and one index buffer; the instances of a class
// are drawn with one instanced call per material, their matrices in an
// instance buffer. Live mobys are skinned in the vertex shader with the joint
// palettes the game's animation fields give (moby_pose.h), from a float
// texture each instance indexes. Textures go through the renderer's texture pool. The
// conventions are the game's: Z up, reversed depth (cleared to 0, GEQUAL).
// Lighting is a fixed sun and ambient, only so shapes read: the game's own
// lights (tie and shrub light indices, the level's directional lights) are
// not applied yet.

#pragma once

#include <array>
#include <string>
#include <vector>

#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "viewer/level.h"

namespace openrac::viewer {

class LevelScene {
public:
    // Upload a level; the GL context must be current.
    bool upload(const LevelData& level, renderer::TexturePool& textures, std::string& error);
    void release();

    void draw_layer(Layer layer, const renderer::FrameInput& input, renderer::RenderState& state);

    // Replaces a layer's instances (live mobys, each frame).
    void set_instances(Layer layer, const std::vector<Instance>& instances);

    // Draws these mobys now, as the moby layer draws its own, into what is bound (the game's
    // render to a texture: the vendor's screens); the layer's own instances are kept.
    void draw_mobys(const std::vector<Instance>& instances, const renderer::FrameInput& input,
                    renderer::RenderState& state);

    // One joint matrix of a palette, as the vertex shader takes it: four columns (the images of
    // the axes, then the translation, in the mesh's units), column-major.
    using JointColumns = std::array<float, 16>;

    // Replaces the joint palettes the mobys' instances index (Instance::palette), each frame.
    void set_palette(const std::vector<JointColumns>& matrices);

    bool lighting = true;

    // The game's fog for this frame (GameState): FOGCOL 0..1, the depths in raw units (game
    // units x 1024) and the GS F at them (255 = none). The world layers are fogged as the GS
    // fogs them: C = FOGCOL + (C - FOGCOL) x F / 255, F linear in camera depth, clamped.
    void set_fog(const std::array<float, 3>& colour, float near_depth, float far_depth,
                 float near_f, float far_f);

    // The light bank the mobys are lit from this frame (16 sets of 16 floats, as the game holds
    // it); until set, the level's exported sets.
    void set_light_sets(const std::vector<float>& sets) {
        if (sets.size() == 16 * 16) {
            m_light_sets = sets;
        }
    }

private:
    struct Group {
        std::uint32_t model = 0;
        int blend = 0;  // Instance::blend of all its instances
        unsigned vao = 0;
        unsigned instances = 0;  // instance buffer
        int count = 0;
        // A static group's instances' box in the world, for leaving it out when the camera does
        // not see it (draw_layer); bounded false: always drawn (live mobys).
        bool bounded = false;
        renderer::Vec3 low{}, high{};
    };

    // One instance, as the vertex shader's per-instance attributes take it.
    struct InstanceData {
        float matrix[16];
        float tint[4];
        float palette;  // the first matrix of its joint palette; -1 none
        float lights;   // a lit tie: the first of its 64 colours in the light texture; -1 none
        float moby_light[4];    // a lit moby: sets, cross-fade, 1 (Instance::moby_light)
        float moby_ambient[4];  // its ambient colour / 128
    };

    void draw_sky(const renderer::FrameInput& input, renderer::RenderState& state);
    Group make_group(std::uint32_t model);
    void fill_group(Group& g, const std::vector<InstanceData>& list, unsigned usage);
    void build_groups(Layer layer, const std::vector<Instance>& instances, bool live);

    std::vector<Model> m_models;
    // Each model's copy in material order, for the opaque pass of the static layers (upload); a
    // model of one primitive, or one only live mobys use, is its own.
    std::vector<std::uint32_t> m_opaque;
    std::vector<bool> m_visible;  // draw_layer's: which of the layer's groups the camera sees
    std::vector<Material> m_materials;
    std::vector<renderer::TextureHandle> m_textures;  // by image
    renderer::TextureHandle m_white = 0;
    std::array<std::vector<Group>, kLayerCount> m_groups;
    // The groups a live layer does not draw this frame, kept for the frames it does (set_instances).
    std::array<std::vector<Group>, kLayerCount> m_idle;
    // build_groups' lists, by model, kept for their storage.
    std::vector<std::pair<std::uint32_t, std::vector<InstanceData>>> m_grouping;
    std::size_t m_palette_rows = 0;  // the palette texture's height now
    unsigned m_vertices = 0;
    unsigned m_indices = 0;
    unsigned m_palette = 0;  // RGBA32F texture, four texels per matrix
    unsigned m_lights = 0;   // RGBA8 texture, the ties' lit colours
    std::vector<float> m_light_sets;  // the level's 16 directional light sets (LevelData)
    std::array<float, 4> m_fog_colour{0, 0, 0, 0};  // w = 1: fog on
    std::array<float, 4> m_fog_params{0, 255, 255, 255};  // slope, offset, lower, upper clamp
    std::vector<JointColumns> m_palette_data;
    renderer::Shader m_mesh;
    // mesh.frag's opaque path alone, without discard (the static layers' opaque pass; draw_layer).
    renderer::Shader m_solid;
    renderer::Shader m_sky;
};

// One layer of a LevelScene, as a bucket of the renderer.
class LayerRenderer : public renderer::BucketRenderer {
public:
    LayerRenderer(LevelScene& scene, Layer layer);

    void render(const renderer::FrameInput& input, renderer::RenderState& state) override;

private:
    LevelScene& m_scene;
    Layer m_layer;
};

}  // namespace openrac::viewer

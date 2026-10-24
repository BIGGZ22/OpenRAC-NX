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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "enhanced/viewer/early_lighting.h"
#include "enhanced/viewer/probe_lighting.h"
#include "enhanced/viewer/sun_shadow.h"
#include "renderer/framebuffer.h"
#include "renderer/gpu_draw.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "scene/level.h"

namespace openrac::scene {

class LevelScene {
public:
    // Upload a level; the GL context must be current.
    bool upload(const LevelData& level, renderer::TexturePool& textures, std::string& error);
    void release();

    void draw_layer(Layer layer, const renderer::FrameInput& input, renderer::RenderState& state);
    bool needs_early_lighting(const renderer::FrameInput& input) const;

    bool probe_data_available() const { return m_probes.has_data(); }

    void render_early_lighting(
        unsigned depth, const renderer::FrameInput& input, renderer::RenderState& state
    );
    bool enable_hero_shadow(bool enabled, std::string& error);

    bool hero_shadow_enabled() const { return m_hero_enabled; }

    bool enable_sun_shadow(bool enabled, std::string& error);

    bool shadow_atlas_active() const { return m_shadow_atlas_active; }

    bool batches_opaque_colour() const;
    bool colour_blend_barrier(Layer layer) const;

    bool sun_shadow_enabled() const { return m_sun_enabled; }

    struct CascadeCulling {
        std::uint64_t candidates = 0, rejected = 0, submitted = 0, unbounded = 0,
                      compacted_groups = 0;
        std::uint64_t solid_draws = 0, alpha_draws = 0;
    };

    const std::array<CascadeCulling, kSunCascades>& sun_culling() const { return m_sun_culling; }

    struct CameraCulling {
        std::uint64_t candidates = 0, rejected = 0, submitted = 0, unbounded = 0;
    };

    const std::array<CameraCulling, kLayerCount>& camera_culling() const {
        return m_camera_culling;
    }

    // Replaces a layer's instances (live mobys, each frame).
    void set_instances(Layer layer, const std::vector<Instance>& instances);

    // Draws these mobys now, as the moby layer draws its own, into what is bound (the game's
    // render to a texture: the vendor's screens); the layer's own instances are kept.
    void draw_mobys(
        const std::vector<Instance>& instances,
        const renderer::FrameInput& input,
        renderer::RenderState& state
    );

    // One joint matrix of a palette, as the vertex shader takes it: four columns (the images of
    // the axes, then the translation, in the mesh's units), column-major.
    using JointColumns = std::array<float, 16>;

    // Replaces the joint palettes the mobys' instances index (Instance::palette), each frame.
    void set_palette(const std::vector<JointColumns>& matrices);

    bool lighting = true;

    bool probes_active() const { return m_probes.active(); }

    bool compute_probe_selection() const { return m_probes.compute_selection(); }

    void select_visible_probe_regions(
        unsigned depth, const renderer::Camera& camera, int width, int height
    ) {
        m_probes.select_visible_regions(depth, camera, width, height);
    }

    void set_probe_lighting(bool enabled, float multiplier) {
        m_probes.enabled = enabled;
        m_probes.gi_multiplier = multiplier;
    }

    void set_probe_bias(float normal, float view) {
        m_probes.normal_bias = normal;
        m_probes.view_bias = view;
    }

    void set_probe_spheres(bool shown) {
        m_probes.show_spheres = shown || m_probes.environment_spheres;
    }

    void update_probe_regions(const renderer::Camera& camera, float seconds = -1) {
        const auto clip = renderer::multiply(camera.projection, camera.view);
        m_probes.update_regions(camera.position, seconds, &clip);
    }

    bool set_probe_regions(
        const lighting::ProbeVolume& volume,
        std::vector<lighting::ProbeVolume> regions,
        std::string& error
    ) {
        return m_probes.upload_regions(volume, std::move(regions), error);
    }

    renderer::AoLighting ao_lighting() const {
        if (!m_probes.active() || m_probes.mode == 0) {
            return renderer::AoLighting::Legacy;
        }
        return m_probes.mode == 1 || m_probes.mode == 2 ? renderer::AoLighting::Indirect
                                                        : renderer::AoLighting::Unoccluded;
    }

    void set_probe_view(int view) {
        if (view >= 0 && view <= 4) {
            m_probes.mode = view;
        }
    }

    void draw_probe_spheres(const renderer::FrameInput& input, renderer::RenderState& state) {
        m_probes.draw_spheres(input, state);
    }

    // In-memory providers must verify scene/version identity before calling.
    // File-based activation performs those checks through ProbeLighting::load.
    bool set_probe_volume(
        const lighting::ProbeVolume& volume,
        std::string& error,
        const lighting::ProbeVolume* dense = nullptr
    ) {
        return m_probes.upload(volume, error, dense);
    }

    // Eight point lights after the 16 directional sets: colour, position/radius.
    void set_point_lights(const std::array<float, 64>& lights) { m_point_lights = lights; }

    // The game's fog for this frame (GameState): FOGCOL 0..1, the depths in raw units (game
    // units x 1024) and the GS F at them (255 = none). The world layers are fogged as the GS
    // fogs them: C = FOGCOL + (C - FOGCOL) x F / 255, F linear in camera depth, clamped.
    void set_fog(
        const std::array<float, 3>& colour,
        float near_depth,
        float far_depth,
        float near_f,
        float far_f
    );

    // The light bank the mobys are lit from this frame (16 sets of 16 floats, as the game holds
    // it); until set, the level's exported sets.
    void set_light_sets(const std::vector<float>& sets) {
        if (sets.size() == 16 * 16) {
            m_light_sets = sets;
        }
    }

private:
    struct CasterBounds {
        renderer::Vec3 low{}, high{};
        bool valid = false;
    };

    // One instance, as the vertex shader's per-instance attributes take it.
    struct InstanceData {
        float matrix[16];
        float tint[4];
        float palette;
        float lights;
        float moby_light[4];
        float moby_ambient[4];
    };

    struct Group {
        bool hero_shadow_caster = false;
        bool shadow_solid_instances = false;  // tint/glow cannot discard opaque static faces
        std::uint32_t model = 0;
        int blend = 0;  // Instance::blend of all its instances
        unsigned vao = 0;
        unsigned instances = 0;  // instance buffer
        int count = 0;
        // A static group's instances' box in the world, for leaving it out when the camera does
        // not see it (draw_layer); bounded false: always drawn (live mobys).
        bool bounded = false;
        renderer::Vec3 low{}, high{};
        std::vector<InstanceData> cpu_instances;
        std::vector<CasterBounds> caster_bounds;
        std::vector<renderer::GpuCullBounds> gpu_bounds;
        renderer::GpuInstanceBatch gpu_camera, gpu_pvs;
        std::vector<std::uint16_t> occlusion;
        unsigned pvs_vao = 0, pvs_instances = 0;
        int pvs_count = 0;
        bool pvs_valid = false;
        Pvs::Mask pvs_mask{};
        unsigned camera_vao = 0, camera_instances = 0;
        int camera_count = 0;
        bool camera_valid = false;
        renderer::Mat4 camera_clip{};
    };

    void draw_sky(const renderer::FrameInput& input, renderer::RenderState& state);
    void prepare_hero_shadow(const renderer::FrameInput& input, renderer::RenderState& state);
    void prepare_sun_shadow(const renderer::FrameInput& input, renderer::RenderState& state);
    void prepare_shadows(const renderer::FrameInput& input, renderer::RenderState& state);
    bool make_shadow_atlas(std::string& error);
    void draw_shadow_atlas(renderer::RenderState& state);
    // Which casters a shadow map draws: all of them, or, for the static shadow cache (R-18), the
    // static layers (terrain, ties, shrubs) into the cache or the mobys over a copy of it.
    enum class ShadowCasters : std::uint8_t {
        All,
        Static,
        Dynamic,
    };
    // Draws the sun cascades whose static casters changed into the static shadow cache, copies
    // the cache into the atlas and draws the dynamic casters over it; returns the cascades it drew
    // into the cache (shadow-static-cache, docs/port/RENDER_PERF.md, R-18).
    unsigned draw_cached_cascades(renderer::RenderState& state);
    // Binds `framebuffer`, the atlas or the cache, for depth-only shadow drawing over all of it.
    void begin_shadow_target(unsigned framebuffer);
    void report_sun_culling() const;
    void draw_shadow_map(
        unsigned target,
        int resolution,
        const renderer::Mat4& matrix,
        bool hero,
        bool separate_hero,
        renderer::RenderState& state,
        unsigned cascade = kSunCascades,
        int atlas_view = -1,
        bool flush_atlas = false,
        ShadowCasters casters = ShadowCasters::All
    );
    Group make_group(std::uint32_t model);
    void fill_group(Group& g, const std::vector<InstanceData>& list, unsigned usage);
    void build_groups(Layer layer, const std::vector<Instance>& instances, bool live);
    // build_groups' parts, also for set_instances' in-place path (level_renderer.cpp says more).
    static std::uint32_t group_key(const Instance& instance);
    static InstanceData instance_data(const Instance& instance);
    static CasterBounds instance_bounds(
        const Model& shape,
        const InstanceData& data,
        const std::array<float, 4>& sphere,
        bool sphere_off
    );
    static renderer::GpuCullBounds gpu_cull_bounds(
        const CasterBounds& bounds, const InstanceData& data, std::uint16_t occlusion
    );
    static bool solid_shadow_instances(const std::vector<InstanceData>& instances);
    static bool moby_sphere_culling_off();
    static bool same_palette(
        const std::vector<JointColumns>& matrices,
        const std::vector<JointColumns>& sent,
        std::size_t texels
    );
    // set_instances' in-place path for the mobys (moby-pose-in-place, RENDER_PERF.md R-16):
    // remember_mobys notes where build_groups put each moby, rewrite_mobys writes the next call's
    // poses there when the mobys and their order are the same, and returns false when they are not.
    void remember_mobys(const std::vector<Instance>& instances);
    bool rewrite_mobys(const std::vector<Instance>& instances);
    renderer::GpuInstanceBatch gpu_batch(const Group& group, const renderer::GpuCullView& view);
    void submit_instances(
        renderer::GpuInstanceBatch batch, unsigned first, unsigned count, int instances
    );

    std::vector<Model> m_models;
    std::vector<std::vector<bool>> m_shadow_solid_primitives;

    Group m_shadow_compact;
    std::vector<InstanceData> m_shadow_survivors;
    std::array<CascadeCulling, kSunCascades> m_sun_culling{};
    std::vector<InstanceData> m_camera_survivors;
    Pvs m_pvs;
    std::vector<InstanceData> m_pvs_survivors;

    struct PvsModel {
        Pvs::Mask mask{};
        bool valid = false;
        unsigned indices = 0;
        std::size_t rejected = 0;
        std::vector<Primitive> primitives;
    };

    // Terrain filtering must preserve material runs instead of leaving holes
    // in the sorted index buffer and turning each fragment into another draw.
    std::vector<std::uint32_t> m_cpu_indices;
    std::vector<PvsModel> m_pvs_models;
    std::array<CameraCulling, kLayerCount> m_camera_culling{};
    // Each model's copy in material order, for the opaque pass of the static layers (upload); a
    // model of one primitive, or one only live mobys use, is its own.
    std::vector<std::uint32_t> m_opaque;
    // Retained between the opaque rounds and the original blended passes.
    std::array<std::vector<bool>, kLayerCount> m_colour_visible;
    std::vector<Material> m_materials;
    std::vector<renderer::TextureHandle> m_textures;  // by image
    renderer::TextureHandle m_white = 0;
    std::array<std::vector<Group>, kLayerCount> m_groups;
    // The groups a live layer does not draw this frame, kept for the frames it does (set_instances).
    std::array<std::vector<Group>, kLayerCount> m_idle;

    // build_groups' lists, by model, kept for their storage.
    struct InstanceGroup {
        std::uint32_t key = 0;
        std::vector<InstanceData> instances;
        std::vector<std::array<float, 4>> spheres;  // CPU only: no extra GPU instance attributes
        std::vector<std::uint16_t> occlusion;
    };

    std::vector<InstanceGroup> m_grouping;
    // Each key's list in m_grouping during a build_groups call, for a lookup by hash.
    std::unordered_map<std::uint32_t, std::size_t> m_group_of_key;

    // Where set_instances' last regrouping put each moby (rewrite_mobys): its key, its group of
    // m_groups[Mobys] and its place there, the sphere its bounds came from, and each group's count.
    struct MobyLayout {
        std::vector<std::uint32_t> keys;
        std::vector<std::uint32_t> group;
        std::vector<std::uint32_t> slot;
        std::vector<std::array<float, 4>> spheres;
        std::vector<std::uint32_t> filled;
        bool sphere_off = false;  // OPENRAC_MOBY_SPHERE_CULL_OFF when the groups were made.
        bool valid = false;       // False until a regrouping, and after an upload or release.
    };

    MobyLayout m_moby_layout;
    std::size_t m_palette_rows = 0;  // the palette texture's height now
    unsigned m_vertices = 0;
    unsigned m_indices = 0;
    unsigned m_palette = 0;           // RGBA32F texture, four texels per matrix
    unsigned m_lights = 0;            // RGBA8 texture, the ties' lit colours
    std::vector<float> m_light_sets;  // the level's 16 directional light sets (LevelData)
    std::array<float, 64> m_point_lights{};
    ProbeLighting m_probes;
    EarlyLighting m_early_lighting;
    bool m_early_capable = false;
    void bind_hero_shadow(const renderer::Shader& shader) const;
    void bind_shadow_atlas(const renderer::Shader& shader) const;
    void bind_sun_shadow(const renderer::Shader& shader, const renderer::Camera& camera) const;
    std::array<float, 4> m_fog_colour{0, 0, 0, 0};        // w = 1: fog on
    std::array<float, 4> m_fog_params{0, 255, 255, 255};  // slope, offset, lower, upper clamp
    std::vector<JointColumns> m_palette_data;
    renderer::Shader m_mesh;
    renderer::Shader m_early_mesh, m_early_solid;
    renderer::Shader m_colour_opaque, m_early_colour_opaque, m_static_translucent;
    // Static solid, static alpha-tested, then opaque mobys (optional skinning).
    std::array<renderer::Shader, 3> m_colour_specialised, m_early_colour_specialised;
    // mesh.frag's opaque path alone, without discard (the static layers' opaque pass; draw_layer).
    renderer::Shader m_solid;
    renderer::Shader m_depth, m_depth_solid;
    renderer::Shader m_sky;
    std::unique_ptr<renderer::FrameBuffer> m_hero_map;
    renderer::Shader m_hero_depth, m_shadow_solid;
    renderer::Shader m_atlas_depth, m_atlas_solid;
    std::unique_ptr<renderer::FrameBuffer> m_shadow_atlas;
    bool m_shadow_atlas_active = false;
    std::array<renderer::Mat4, 5> m_shadow_view_projection;
    // The sun cascades' static casters, in the atlas's layout, kept between frames, and what each
    // cascade's rectangle was drawn for (shadow-static-cache, R-18).
    std::unique_ptr<renderer::FrameBuffer> m_static_shadow;
    std::array<StaticShadowKey, kSunCascades> m_static_keys{};
    // Bumped whenever the static layers' instances change (an upload, set_instances of one).
    std::uint64_t m_static_generation = 1;

    struct ShadowDraw {
        unsigned vao;
        int instances;
        renderer::GpuInstanceBatch batch;
        std::uint32_t first, count;
        bool solid, skinning;
        int material, view;
    };

    std::array<std::vector<ShadowDraw>, 3> m_shadow_draws;
    renderer::Mat4 m_hero_matrix = renderer::identity();
    std::optional<renderer::Vec3> m_hero_centre;
    bool m_hero_enabled = false, m_hero_ready = false, m_hero_airborne = false;
    std::optional<bool> m_hero_logged_airborne;
    std::array<std::unique_ptr<renderer::FrameBuffer>, kSunCascades> m_sun_maps;
    SunCascades m_sun_cascades;
    bool m_sun_enabled = false, m_sun_ready = false;
};

// One layer of a LevelScene, as a bucket of the renderer.
class LayerRenderer : public renderer::BucketRenderer {
public:
    LayerRenderer(LevelScene& scene, Layer layer);

    void render(const renderer::FrameInput& input, renderer::RenderState& state) override;

    renderer::AoLighting ao_lighting() const override { return m_scene.ao_lighting(); }

    void render_depth(const renderer::FrameInput& input, renderer::RenderState& state) override;

    bool batches_opaque_colour() const override {
        return m_layer != Layer::Sky && m_scene.batches_opaque_colour();
    }

    bool opaque_colour_barrier() const override { return m_scene.colour_blend_barrier(m_layer); }

    void render_opaque_colour(
        unsigned phase, const renderer::FrameInput& input, renderer::RenderState& state
    ) override {
        state.opaque_colour_phase = static_cast<int>(phase);
        m_scene.draw_layer(m_layer, input, state);
        state.opaque_colour_phase = -1;
    }

    bool enhanced_lighting_available() const override { return m_scene.probe_data_available(); }

    bool needs_early_lighting(const renderer::FrameInput& input) const override {
        return m_layer == Layer::Terrain && m_scene.needs_early_lighting(input);
    }

    void render_early_lighting(
        unsigned depth, const renderer::FrameInput& input, renderer::RenderState& state
    ) override {
        m_scene.render_early_lighting(depth, input, state);
    }

    bool owns_gpu_timing() const override { return true; }

private:
    LevelScene& m_scene;
    Layer m_layer;
};

}  // namespace openrac::scene

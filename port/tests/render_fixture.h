// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The synthetic scene the render benchmark (tests/render_bench.cpp) and the equivalence test
// (tests/render_equivalence_test.cpp) draw: a level built in memory from a seed, with no clock and
// no retail data, drawn through the scene's renderer (scene/level_renderer.h) as the frontend
// draws a level, and the camera paths and moby movement each frame takes.
//
// The default scene is about a large level's size: 4,000 static instances whose meshes add up to
// about 1,000,000 triangles, 140 materials, 600 RGBA8 textures with mip chains of mixed sizes, 300
// live mobys of skinned classes with joint palettes (one in ten blended), a sun, and a share of
// small far instances (a fifth of them) under one pixel at the far end of the camera's walk. The
// same seed gives the same scene on every machine, so the counters it yields are comparable before
// and after a change on one machine.
//
// Between two ticks the frontend's draw_kept hands the scene the mobys and their palettes part way
// from one tick to the next (set_palette, set_instances) and renders; draw() does the same, and
// frame_state() makes the part-way state by blending two ticks' states.
//
// Its own, written for these two programs: viewer_test.cpp builds its levels as files, which a
// scene of this size should not be; one builder for both is a follow-up in PROGRESS.md.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numbers>
#include <string>
#include <string_view>
#include <vector>

#include "enhanced/tests/probe_fixture.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/math.h"
#include "renderer/render_opt.h"
#include "renderer/renderer.h"
#include "scene/level.h"
#include "scene/level_renderer.h"

namespace openrac::test::render {

// ---- The scene's size ----

// What the synthetic scene holds. The defaults are the benchmark's; the equivalence test draws a
// smaller one, which is the same code at other numbers.
struct SceneSize {
    std::uint64_t seed = 1;               // Every random choice follows from it.
    int static_instances = 4000;          // Ties and shrubs placed over the level.
    std::uint64_t triangles = 1'000'000;  // About what their meshes add up to.
    int materials = 140;                  // The static meshes' materials, two to a mesh.
    int textures = 600;                   // RGBA8 images, mipped; the rest go to the mobys.
    int mobys = 300;                      // Live mobys, set each frame as the frontend sets them.
    int moby_parts = 10;                  // Primitives (materials) of each moby class.
    int joints = 8;                       // Joints of each moby class, each with a palette matrix.
    double far_share = 0.2;               // The static instances placed small and far ahead.
};

// The side of the square the static instances stand on, in game units, centred on the origin.
inline constexpr float kLevelSide = 2000.0f;

// The size of the small far instances, in game units: under a pixel at 1080p from 1,200 units on.
inline constexpr float kFarScale = 0.3f;

// The ticks a second the moby movement and the camera walk are timed in (the game's 60 Hz).
inline constexpr double kTicksPerSecond = 60.0;

// ---- Deterministic numbers ----

// A small generator of the scene's choices: SplitMix64, the same sequence for a seed on every
// machine and compiler (std::uniform_*_distribution is not).
class Random {
public:
    explicit Random(std::uint64_t seed) : m_state(seed) {}

    // Returns the next 64 random bits.
    std::uint64_t next() {
        // SplitMix64's constants (Steele, Lea and Flood, "Fast splittable pseudorandom number
        // generators", 2014).
        m_state += 0x9E3779B97F4A7C15ull;
        std::uint64_t z = m_state;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }

    // Returns a float in [low, high), from the top 24 bits (a float's mantissa).
    float uniform(float low, float high) {
        constexpr float kUnit = 1.0f / 16'777'216.0f;  // 2^-24
        return low + (high - low) * static_cast<float>(next() >> 40) * kUnit;
    }

    // Returns a whole number in [0, count); count is at least 1.
    int below(int count) { return static_cast<int>(next() % static_cast<std::uint64_t>(count)); }

private:
    std::uint64_t m_state;  // The generator's whole state.
};

// ---- Building the level ----

// Returns a rotation about the Z axis (the game's up) by `angle` radians.
inline renderer::Mat4 rotation_z(float angle) {
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    return {c, s, 0, 0, -s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

// Returns the rings of a sphere mesh of about `triangles` triangles: 4 x rings squared of them,
// with twice as many segments as rings.
inline int rings_for(std::uint64_t triangles) {
    const double rings =
        std::sqrt(static_cast<double>(std::max<std::uint64_t>(triangles, 16)) / 4.0);
    return std::max(2, static_cast<int>(std::lround(rings)));
}

// Appends a unit sphere of `rings` rings to `level` as a model whose primitives each take an equal
// band of rings, with the materials from `first_material` on. A skinned model (`joints` above 0)
// weights each band to the joint of its height, so its palette bends it. Returns the model's index.
inline std::uint32_t add_sphere(
    viewer::LevelData& level, int rings, int parts, int first_material, int joints
) {
    const int segments = rings * 2;
    const auto first_vertex = static_cast<std::uint32_t>(level.vertices.size());

    // The vertices, ring by ring from the south pole up, the seam's column twice for its UVs.
    for (int ring = 0; ring <= rings; ++ring) {
        const float v = static_cast<float>(ring) / static_cast<float>(rings);
        const float polar = std::numbers::pi_v<float> * v;
        const int joint = joints > 0 ? std::min(joints - 1, ring * joints / (rings + 1)) : 0;

        // Around the ring.
        for (int segment = 0; segment <= segments; ++segment) {
            const float u = static_cast<float>(segment) / static_cast<float>(segments);
            const float azimuth = 2.0f * std::numbers::pi_v<float> * u;
            viewer::Vertex vertex{};
            vertex.normal[0] = std::sin(polar) * std::cos(azimuth);
            vertex.normal[1] = std::sin(polar) * std::sin(azimuth);
            vertex.normal[2] = -std::cos(polar);
            std::copy(std::begin(vertex.normal), std::end(vertex.normal), vertex.position);
            vertex.position[2] += 1.0f;  // standing on its base, as the game's meshes do
            vertex.uv[0] = u * 4.0f;     // four repeats around, two up the side
            vertex.uv[1] = v * 2.0f;
            std::fill(std::begin(vertex.colour), std::end(vertex.colour), 1.0f);
            vertex.light_slot = -1.0f;
            vertex.joints[0] = static_cast<std::uint8_t>(joint);
            vertex.weights[0] = joints > 0 ? 1.0f : 0.0f;
            level.vertices.push_back(vertex);
        }
    }

    viewer::Model model;
    model.name = "synthetic";
    model.min = {-1.0f, -1.0f, 0.0f};
    model.max = {1.0f, 1.0f, 2.0f};
    model.joints = joints;

    // The bands, each a primitive of its own material: two triangles a quad of the grid.
    for (int part = 0; part < parts; ++part) {
        viewer::Primitive primitive;
        primitive.first_index = static_cast<std::uint32_t>(level.indices.size());
        primitive.material = first_material + part;

        // The rings of this band.
        for (int ring = part * rings / parts; ring < (part + 1) * rings / parts; ++ring) {
            // The quads around the ring.
            for (int segment = 0; segment < segments; ++segment) {
                const auto row = static_cast<std::uint32_t>(segments + 1);
                const std::uint32_t a = first_vertex + static_cast<std::uint32_t>(ring) * row
                                        + static_cast<std::uint32_t>(segment);
                const std::uint32_t b = a + row;
                level.indices.insert(level.indices.end(), {a, a + 1, b, a + 1, b + 1, b});
            }
        }

        primitive.index_count =
            static_cast<std::uint32_t>(level.indices.size()) - primitive.first_index;
        model.primitives.push_back(primitive);
    }

    level.models.push_back(model);
    return static_cast<std::uint32_t>(level.models.size() - 1);
}

// Appends one RGBA8 image of `side` texels a side, a pattern of `random`'s colours, opaque unless
// `holes` (a cutout material's: every eighth texel clear).
inline void add_image(viewer::LevelData& level, int side, Random& random, bool holes) {
    renderer::Rgba8Image image(side, side);
    const std::array<std::uint8_t, 3> base = {
        static_cast<std::uint8_t>(random.below(256)),
        static_cast<std::uint8_t>(random.below(256)),
        static_cast<std::uint8_t>(random.below(256)),
    };

    // A checker of the base colour and its complement, eight texels a square.
    for (int y = 0; y < side; ++y) {
        // Along the row.
        for (int x = 0; x < side; ++x) {
            const bool odd = ((x / 8) + (y / 8)) % 2 != 0;
            const std::size_t at = (static_cast<std::size_t>(y) * static_cast<std::size_t>(side)
                                    + static_cast<std::size_t>(x))
                                   * 4;

            // One channel at a time: the base colour, or its complement on the odd squares.
            for (std::size_t c = 0; c < 3; ++c) {
                image.pixels[at + c] = odd ? static_cast<std::uint8_t>(255 - base[c]) : base[c];
            }

            image.pixels[at + 3] = holes && (x + y) % 8 == 0 ? 0 : 255;
        }
    }

    level.images.push_back(std::move(image));
    level.image_paths.push_back("synthetic/" + std::to_string(level.images.size()) + ".png");
    level.translucent_images.push_back(holes);
}

// Returns the level the size `size` asks for: terrain, the static instances, the moby classes
// (their instances are live, set each frame: moby_tick), the textures and a sun.
inline viewer::LevelData make_level(const SceneSize& size) {
    viewer::LevelData level;
    Random random(size.seed);
    level.level = 1;
    level.dir = "synthetic";

    // The textures, side 32 to 256 by powers of two, so their mip chains are of mixed lengths.
    for (int n = 0; n < size.textures; ++n) {
        const int side = 32 << random.below(4);
        add_image(level, side, random, n % 10 == 3);
    }

    // The materials: the static meshes' first, then the moby classes', each on a texture of its own
    // while there are textures, every tenth static one a cutout and every seventeenth blended.
    const int moby_classes = std::max(1, (size.textures - size.materials) / size.moby_parts);
    const int all_materials = size.materials + moby_classes * size.moby_parts;

    // Each material on its texture.
    for (int n = 0; n < all_materials; ++n) {
        viewer::Material material;
        material.image = size.textures > 0 ? n % size.textures : -1;
        material.cutout = n < size.materials && n % 10 == 3;
        material.blend = n < size.materials && n % 17 == 5;
        level.materials.push_back(material);
    }

    // The terrain: one sphere flattened under the level, about a twentieth of the triangles.
    const std::uint32_t terrain = add_sphere(level, rings_for(size.triangles / 20), 2, 0, 0);
    viewer::Instance ground;
    ground.model = terrain;
    ground.matrix = renderer::multiply(
        renderer::translation({0.0f, 0.0f, -2.0f}),
        renderer::multiply(
            renderer::scale(kLevelSide * 0.5f),
            renderer::Mat4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.001f, 0, 0, 0, 0, 1}
        )
    );
    level.instances[static_cast<std::size_t>(viewer::Layer::Terrain)].push_back(ground);

    // The static meshes, two materials each, from half to one and a half times the mean size.
    const int meshes = std::max(1, size.materials / 2);
    const std::uint64_t mean =
        size.triangles / static_cast<std::uint64_t>(std::max(1, size.static_instances));
    std::vector<std::uint32_t> static_models;

    // Each mesh, a little larger than the one before.
    for (int n = 0; n < meshes; ++n) {
        const double share =
            0.5 + static_cast<double>(n) / static_cast<double>(std::max(1, meshes - 1));
        const auto triangles = static_cast<std::uint64_t>(static_cast<double>(mean) * share);
        static_models.push_back(
            add_sphere(level, rings_for(triangles), 2, (n * 2) % std::max(1, size.materials), 0)
        );
    }

    // The static instances: ties and shrubs over the level, and the far share small and ahead of
    // the camera's walk (camera_at), past where it ends.
    for (int n = 0; n < size.static_instances; ++n) {
        viewer::Instance instance;
        instance.model = static_models[static_cast<std::size_t>(random.below(meshes))];
        const bool far = static_cast<double>(n) < size.far_share * size.static_instances;
        const float along = far ? random.uniform(1200.0f, 2400.0f) : 0.0f;
        const renderer::Vec3 position =
            far ? renderer::
                      Vec3{900.0f + along * 0.7071f, 900.0f + along * 0.7071f, random.uniform(0.0f, 40.0f)}
                : renderer::Vec3{
                      random.uniform(-kLevelSide, kLevelSide) * 0.5f,
                      random.uniform(-kLevelSide, kLevelSide) * 0.5f,
                      0.0f
                  };
        const float scale = far ? kFarScale : random.uniform(1.0f, 4.0f);
        instance.matrix = renderer::multiply(
            renderer::translation(position),
            renderer::multiply(rotation_z(random.uniform(0.0f, 6.2832f)), renderer::scale(scale))
        );
        const auto layer = n % 5 < 3 ? viewer::Layer::Ties : viewer::Layer::Shrubs;
        level.instances[static_cast<std::size_t>(layer)].push_back(instance);
    }

    // The moby classes: skinned spheres of size.moby_parts bands, one class per run of materials.
    for (int n = 0; n < moby_classes; ++n) {
        const std::uint32_t model = add_sphere(
            level,
            std::max(size.moby_parts, 10),
            size.moby_parts,
            size.materials + n * size.moby_parts,
            size.joints
        );
        level.moby_classes[n] = {model, false};
    }

    // The sun: light set 0's colour A and its direction, down and to the side (the layout of
    // LevelData::light_sets: colour A, direction A, colour B, direction B per set).
    level.light_sets.assign(16 * 16, 0.0f);
    level.light_sets[0] = level.light_sets[1] = level.light_sets[2] = 0.8f;
    level.light_sets[4] = 0.4f;
    level.light_sets[5] = 0.3f;
    level.light_sets[6] = -0.87f;
    level.background = {0.3f, 0.4f, 0.6f};
    // The bounds of the walk (walk_over): the square the instances stand on, the far ones past it.
    level.bounds_min = {-kLevelSide * 0.5f, -kLevelSide * 0.5f, -30.0f};
    level.bounds_max = {kLevelSide * 0.5f, kLevelSide * 0.5f, 30.0f};
    return level;
}

// ---- The live mobys ----

// What the scene is handed for its live mobys in one frame: their instances and the joint palette
// they index (LevelScene::set_instances, set_palette).
struct MobyFrame {
    std::vector<viewer::Instance> instances;                // The moby layer's instances.
    std::vector<viewer::LevelScene::JointColumns> palette;  // Every moby's joints, in order.
};

// Returns the mobys of `level` at `tick` (a fraction of a tick between two): each walks a circle of
// its own and bends its joints, one in ten blended at half alpha. The same tick gives the same
// state, so a frame's mobys depend on its tick alone.
inline MobyFrame moby_tick(const viewer::LevelData& level, const SceneSize& size, double tick) {
    MobyFrame frame;
    const auto classes = static_cast<int>(level.moby_classes.size());

    // Nothing to place without a moby class.
    if (classes == 0) {
        return frame;
    }

    Random random(size.seed ^ 0x6D6F6279ull);  // "moby": the mobys' own stream of the seed
    const double seconds = tick / kTicksPerSecond;

    // Each moby, its circle and swing drawn from the stream in the same order every tick.
    for (int n = 0; n < size.mobys; ++n) {
        const int cls = n % classes;
        const float centre_x = random.uniform(-600.0f, 600.0f);
        const float centre_y = random.uniform(-600.0f, 600.0f);
        const float radius = random.uniform(2.0f, 12.0f);
        const float phase = random.uniform(0.0f, 6.2832f);
        const auto angle = static_cast<float>(seconds * 0.8) + phase;
        viewer::Instance instance;
        instance.model = level.moby_classes.at(cls).first;
        instance.matrix = renderer::multiply(
            renderer::translation(
                {centre_x + radius * std::cos(angle), centre_y + radius * std::sin(angle), 0.0f}
            ),
            renderer::multiply(rotation_z(angle + 1.5708f), renderer::scale(1.5f))
        );
        instance.palette = static_cast<int>(frame.palette.size());
        instance.moby_light = {0.0f, 0.0f, 0.0f, 1.0f};
        instance.moby_ambient = {0.5f, 0.5f, 0.5f, 1.0f};

        // One in ten blended at half its alpha, as MobyProc blends a fading moby.
        if (n % 10 == 7) {
            instance.blend = 1;
            instance.tint[3] = 0.5f;
        }

        frame.instances.push_back(instance);
        const int joints = level.models[instance.model].joints;

        // Its joints, each turned about the up axis by its own swing (the class's own count, so a
        // real level's classes index palettes of their size too).
        for (int joint = 0; joint < joints; ++joint) {
            const auto swing = static_cast<float>(0.3 * std::sin(seconds * 2.0 + joint + phase));
            frame.palette.push_back(rotation_z(swing));
        }
    }

    return frame;
}

// Returns the state part way from `from` to `to` by `alpha` (0 to 1): matrices and joint columns
// blended element by element, as the frontend blends between two ticks' mobys.
inline MobyFrame blend_mobys(const MobyFrame& from, const MobyFrame& to, float alpha) {
    MobyFrame frame = to;

    // Each instance's matrix.
    for (std::size_t n = 0; n < frame.instances.size() && n < from.instances.size(); ++n) {
        // Each element of it.
        for (std::size_t e = 0; e < frame.instances[n].matrix.size(); ++e) {
            const float a = from.instances[n].matrix[e];
            frame.instances[n].matrix[e] = a + (to.instances[n].matrix[e] - a) * alpha;
        }
    }

    // Each palette matrix.
    for (std::size_t n = 0; n < frame.palette.size() && n < from.palette.size(); ++n) {
        // Each element of it.
        for (std::size_t e = 0; e < frame.palette[n].size(); ++e) {
            const float a = from.palette[n][e];
            frame.palette[n][e] = a + (to.palette[n][e] - a) * alpha;
        }
    }

    return frame;
}

// ---- The camera ----

// How the camera moves over the frames of a run.
enum class CameraPath : std::uint8_t {
    Static,  // still, at the walk's start, looking along it
    Walk,    // from one corner of the level towards the far corner, where the small instances are
    Turn,    // turning on the spot at the level's middle
    Creep,   // from the walk's start, a hundredth of a unit a tick: under or across a shadow texel
};

// Returns the camera path named `name` (static, walk, turn, creep), or Walk for another name.
inline CameraPath camera_path(std::string_view name) {
    // The ones that are not the default.
    if (name == "static") {
        return CameraPath::Static;
    }

    // The shadow cache's case: cascades that stay or move by a texel from one frame to the next.
    if (name == "creep") {
        return CameraPath::Creep;
    }

    return name == "turn" ? CameraPath::Turn : CameraPath::Walk;
}

// Where the camera walks: from a twentieth of the way into `level`'s bounds to a twentieth from
// their far corner, at a height above the ground (the synthetic level: -900 to 900 in X and Y).
struct Walk {
    renderer::Vec3 start{};  // Where a static camera stands and a walk begins.
    renderer::Vec3 end{};    // Where a walk ends; a turn stands halfway between.
};

// Returns the walk over `level`'s bounds.
inline Walk walk_over(const viewer::LevelData& level) {
    constexpr float kInset = 0.05f;      // of the bounds, at each end
    constexpr float kEyeHeight = 30.0f;  // game units above the bounds' middle height
    const renderer::Vec3& low = level.bounds_min;
    const renderer::Vec3& high = level.bounds_max;
    const float z = (low[2] + high[2]) * 0.5f + kEyeHeight;
    const auto at = [&](float share) {
        return renderer::Vec3{
            low[0] + (high[0] - low[0]) * share, low[1] + (high[1] - low[1]) * share, z
        };
    };
    return {at(kInset), at(1.0f - kInset)};
}

// Returns the camera on `path` over `walk` at `tick` of a run of `ticks`, for a picture `aspect`
// wide.
inline renderer::Camera camera_at(
    const Walk& walk, CameraPath path, double tick, double ticks, float aspect
) {
    constexpr float kFovY = 1.0472f;  // 60 degrees
    constexpr float kNear = 0.05f;
    constexpr float kFar = 4000.0f;  // past the far instances at the walk's end
    const double t = ticks > 0.0 ? std::clamp(tick / ticks, 0.0, 1.0) : 0.0;
    renderer::Camera camera;
    const renderer::Vec3 span{walk.end[0] - walk.start[0], walk.end[1] - walk.start[1], 0.0f};
    renderer::Vec3 forward =
        renderer::normalize({span[0], span[1], -0.05f * std::hypot(span[0], span[1])});
    camera.position = walk.start;

    // The walk goes from one corner to the other; the turn stays in the middle.
    if (path == CameraPath::Walk) {
        const auto along = static_cast<float>(t);
        camera.position =
            {walk.start[0] + span[0] * along, walk.start[1] + span[1] * along, walk.start[2]};
    } else if (path == CameraPath::Creep) {
        // A tick's step, about the near cascade's texel at the walk's start (assumed).
        constexpr float kCreep = 0.01f;
        const float along = kCreep * static_cast<float>(tick) / std::hypot(span[0], span[1]);
        camera.position =
            {walk.start[0] + span[0] * along, walk.start[1] + span[1] * along, walk.start[2]};
    } else if (path == CameraPath::Turn) {
        const auto yaw = static_cast<float>(2.0 * std::numbers::pi * t);
        camera.position =
            {walk.start[0] + span[0] * 0.5f, walk.start[1] + span[1] * 0.5f, walk.start[2]};
        forward = {std::cos(yaw), std::sin(yaw), -0.05f};
    }

    camera.view =
        renderer::look_along(camera.position, renderer::normalize(forward), {0.0f, 0.0f, 1.0f});
    camera.projection = renderer::perspective_reversed(kFovY, aspect, kNear, kFar);
    return camera;
}

// ---- Scenarios ----

// A way to run the scene: how the camera moves, how many pictures between two ticks, and whether
// the level is uploaded again halfway (a level change). Later packages add their own.
struct Scenario {
    std::string_view name;  // As --scenario names it.
    CameraPath camera;      // The camera's path over the run.
    int between;    // Pictures drawn between two ticks (alpha in steps of 1 / (between + 1)).
    bool reupload;  // The level uploaded again halfway through the run.
};

// The scenarios, the benchmark's default first.
// clang-format off: one scenario per row.
inline constexpr std::array<Scenario, 6> kScenarios = {{
    {"walk", CameraPath::Walk, 0, false},           // the camera walks, the mobys move
    {"static-mobys", CameraPath::Static, 0, false}, // the camera still, the mobys move
    {"turn", CameraPath::Turn, 0, false},           // the camera turns on the spot
    {"between", CameraPath::Walk, 3, false},        // three pictures between ticks, a quarter apart
    {"reupload", CameraPath::Walk, 0, true},        // the level uploaded again halfway
    {"creep", CameraPath::Creep, 0, false},         // the camera creeping by shadow texels
}};
// clang-format on

// Returns the scenario named `name`, or null when there is none.
inline const Scenario* find_scenario(std::string_view name) {
    const auto found =
        std::ranges::find_if(kScenarios, [name](const Scenario& s) { return s.name == name; });
    return found != kScenarios.end() ? &*found : nullptr;
}

// One picture of a run: the tick it belongs to, how far from the tick before (1 for the tick's
// own picture), and the state it shows.
struct Picture {
    std::uint64_t tick = 0;  // The tick it shows, or is part way to.
    float alpha = 1.0f;      // How far from the tick before: 1 is the tick's own picture.
    bool between = false;    // True for a picture drawn between two ticks.
};

// Returns picture `index` of a run with `between` pictures between two ticks.
inline Picture picture(std::uint64_t index, int between) {
    const auto per_tick = static_cast<std::uint64_t>(between + 1);
    const std::uint64_t step = index % per_tick;
    Picture out;
    out.tick = index / per_tick + 1;
    out.alpha = static_cast<float>(step + 1) / static_cast<float>(per_tick);
    out.between = step + 1 != per_tick;
    return out;
}

// Returns the mobys of `level` at `shown`: its tick's state, or for a picture between two ticks
// the two ticks' states blended (blend_mobys), as the frontend blends them.
inline MobyFrame mobys_at(
    const viewer::LevelData& level, const SceneSize& size, const Picture& shown
) {
    MobyFrame now = moby_tick(level, size, static_cast<double>(shown.tick));

    // The tick's own picture shows the tick as it is.
    if (!shown.between) {
        return now;
    }

    return blend_mobys(
        moby_tick(level, size, static_cast<double>(shown.tick - 1)), now, shown.alpha
    );
}

// ---- Drawing it ----

// How a SceneRenderer draws.
struct DrawOptions {
    int width = 640;  // The picture, in pixels.
    int height = 360;
    int samples = 1;        // MSAA samples of the picture's framebuffer.
    bool enhanced = false;  // Sun shadows and GTAO (port/enhanced), where the build has them.
};

// The level on the GPU, drawn as the frontend draws one: the scene's five layers as bucket
// renderers of a Renderer, into a framebuffer of its own. The GL (or Vulkan) context must be
// current while it is used and when it goes.
//
// Neither copied nor moved: the renderers it adds hold references to its scene.
class SceneRenderer {
public:
    SceneRenderer() = default;
    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;

    // Releases the scene before the renderer whose texture pool it used.
    ~SceneRenderer() { release(); }

    // Uploads `level` and makes the framebuffer `options` asks for. False with `error` when the
    // scene, a renderer or the framebuffer cannot be made, or enhanced was asked and is not here.
    bool open(const viewer::LevelData& level, const DrawOptions& options, std::string& error) {
        m_options = options;

        // The level's meshes, textures and shaders.
        if (!m_scene.upload(level, m_renderer.textures(), error)) {
            return false;
        }

        // Enhanced: the sun's cascades need a probe volume to read (enhanced/tests/probe_fixture.h).
        if (options.enhanced
            && (!shadow_probe_fixture(m_scene, error) || !m_scene.enable_sun_shadow(true, error))) {
            return false;
        }

        // The five layers, in the game's order, as the frontend adds them.
        for (std::size_t layer = 0; layer < viewer::kLayerCount; ++layer) {
            auto bucket =
                std::make_unique<viewer::LayerRenderer>(m_scene, static_cast<viewer::Layer>(layer));

            // A renderer that cannot be added leaves the scene incomplete.
            if (m_renderer.add(std::move(bucket), error) == nullptr) {
                return false;
            }
        }

        m_open = m_renderer.init(error)
                 && m_frame.create(options.width, options.height, error, options.samples);
        return m_open;
    }

    // Uploads `level` again, as a level change does, keeping the renderers and the framebuffer.
    // The scene is a new one, as the frontend's use_level makes it: its textures are new handles
    // of the pool, made again (a released scene that uploads again keeps its first handles).
    bool reupload(const viewer::LevelData& level, std::string& error) {
        m_scene.release();
        m_scene = viewer::LevelScene();
        return m_scene.upload(level, m_renderer.textures(), error)
               && (!m_options.enhanced
                   || (shadow_probe_fixture(m_scene, error)
                       && m_scene.enable_sun_shadow(true, error)));
    }

    // Draws one frame seen by `camera` with `mobys`, as draw_kept draws one: the mobys' palette
    // and instances handed to the scene, then the renderer's passes into the framebuffer.
    void draw(const renderer::Camera& camera, const MobyFrame& mobys, std::uint64_t frame) {
        m_scene.set_palette(mobys.palette);
        m_scene.set_instances(viewer::Layer::Mobys, mobys.instances);
        renderer::FrameInput input;
        input.camera = camera;
        input.clear_colour = {0.3f, 0.4f, 0.6f, 1.0f};
        input.gtao = m_options.enhanced;
        // The sun's cascades are drawn when the frame asks for them, as the frontend asks.
        input.sun_pixel_shadow = m_options.enhanced && m_scene.sun_shadow_enabled();
        input.frame = frame;
        renderer::RenderTarget target{m_frame.id(), m_options.width, m_options.height};
        target.resolve_framebuffer = m_frame.resolve_target();
        target.resolves_itself = m_frame.resolves_itself();
        m_renderer.render(input, target);
        selftest_reference_path();
    }

    // Returns the last frame's pixels, RGBA, bottom row first (a read-back: it waits for the GPU).
    [[nodiscard]] std::vector<std::uint8_t> read_rgba() const { return m_frame.read_rgba(); }

    // The renderer, for its stats and its textures.
    [[nodiscard]] renderer::Renderer& renderer() { return m_renderer; }

    // Lets the scene, the renderers and the framebuffer go; open() may be called again.
    void release() {
        // Nothing was made: a renderer never opened has nothing to release.
        if (!m_open) {
            return;
        }

        m_scene.release();
        m_renderer.release();
        m_frame.release();
        m_open = false;
    }

private:
    // The reference path of the registry's self-test entry: nothing, so its frames with the entry
    // on and off are the same. The equivalence test compares them as its check of itself; a
    // one-pixel change here is what makes that test fail (docs/port/RENDER_PERF.md, the equivalence test).
    void selftest_reference_path() {
        // The entry is on: the optimised path, which is also nothing.
        if (renderer::render_opt::enabled(renderer::render_opt::Opt::RegistrySelftest)) {
            return;
        }
    }

    renderer::Renderer m_renderer;  // Its texture pool holds the scene's textures.
    viewer::LevelScene m_scene;     // Released before m_renderer, which it draws through.
    renderer::FrameBuffer m_frame;  // The picture.
    DrawOptions m_options;          // As open() was given them.
    bool m_open = false;            // True between open() and release().
};

}  // namespace openrac::test::render

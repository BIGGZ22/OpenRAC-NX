// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include "renderer/math.h"
#include "scene/level.h"

namespace openrac::deko {
class Presenter;
using MeshId = std::uint32_t;
using TextureId = std::uint32_t;
// Zero texture means white. Zero mesh is invalid. IDs are never recycled.
enum class Blend { Opaque, Alpha, Additive };
struct SceneDraw {
    MeshId mesh = 0;
    // mesh==0: per-frame vertices/indices, copied into the acquired slot's ring.
    // Used for changing particles, effects and native HUD geometry.
    std::span<const scene::Vertex> dynamic_vertices;
    std::span<const std::uint32_t> dynamic_indices;
    TextureId texture = 0;
    std::uint32_t first_index = 0, index_count = 0;
    renderer::Mat4 model = renderer::identity();
    std::array<float,4> tint{1,1,1,1};
    std::array<float,4> material{1,1,1,1};
    std::array<float,4> moby_light{}, ambient{.5f,.5f,.5f,0};
    std::span<const renderer::Mat4> joints;
    std::span<const std::uint32_t> light_colours;
    Blend blend = Blend::Opaque;
    bool depth_test = true, depth_write = true, cutout = false, metal = false;
    unsigned alpha_test = 0; // 0 off, 1 half, 2 solid >=.99, 3 translucent [.02,.99)
    // UI draws pass an identity camera and clip-space positions explicitly.
    bool screen_space = false;
    std::array<unsigned,4> scissor{}; // width/height zero: entire display
};
struct SceneView {
    renderer::Mat4 projection = renderer::identity(), view = renderer::identity();
    std::array<float,4> background{0,0,0,1};
    std::array<float,4> fog_params{}, fog_colour{};
    std::array<std::array<float,4>,64> light_sets{};
};
struct SceneCounters { std::uint64_t draws = 0, triangles = 0, uniform_bytes = 0, dynamic_bytes = 0; };
// One native frame, many ordered objects/materials; no GL or Vulkan calls.
// Presenter owns the device/window and MUST outlive this object.
class SceneRenderer {
public:
    explicit SceneRenderer(Presenter& presenter);
    ~SceneRenderer();
    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;
    bool load_shaders(std::span<const std::uint8_t> vertex, std::span<const std::uint8_t> fragment, std::string& error);
    MeshId create_mesh(std::span<const scene::Vertex> vertices, std::span<const std::uint32_t> indices, std::string& error);
    TextureId create_texture(const renderer::Rgba8Image& image, bool linear, bool repeat_s, bool repeat_t, std::string& error);
    bool render(const SceneView& view, std::span<const SceneDraw> draws, std::string& error, bool verify_red_center = false);
    void release_resources();
    SceneCounters counters() const;
private:
    struct Impl;
    std::unique_ptr<Impl> p;
};
}

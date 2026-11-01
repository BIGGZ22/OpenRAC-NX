// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <array>
#include <memory>
#include <span>
#include <string>

namespace openrac::renderer { struct Rgba8Image; }

namespace openrac::deko {
struct MeshVertex { float position[3], color[4], uv[2]{}; };
// Native Switch display/image-transfer foundation. This is not a GL shim and
// does not yet draw OpenRAC's world. No SDL/EGL/Vulkan object owns its NWindow.
class Presenter {
public:
    Presenter();
    ~Presenter();
    Presenter(const Presenter&) = delete;
    Presenter& operator=(const Presenter&) = delete;
    bool open(int width, int height, std::string& error);
    bool clear(std::uint32_t rgba, std::string& error);
    bool load_triangle(std::span<const std::uint8_t> vertex_dksh,
                       std::span<const std::uint8_t> fragment_dksh, std::string& error);
    bool triangle(std::string& error, bool verify = false);
    bool load_mesh_shaders(std::span<const std::uint8_t> vertex_dksh,
                           std::span<const std::uint8_t> fragment_dksh, std::string& error);
    // Retained geometry: upload on scene/object change, not on every frame.
    bool upload_mesh(std::span<const MeshVertex> vertices, std::span<const std::uint32_t> indices, std::string& error);
    // Bridge from the game's existing PS2 -> RGBA8 conversion. Loading only:
    // replacement drains GPU consumers before retiring the old native image.
    bool upload_texture(const renderer::Rgba8Image& image, std::string& error,
                        bool linear_filter = false, bool repeat = false, int repeat_t = -1);
    bool mesh(const std::array<float,16>& column_major_transform, std::string& error, bool verify_red_center = false, bool textured = false);
    // RGBA8, top row first. Optional GPU readback verifies a 2x2 corner block.
    bool present(std::span<const std::uint32_t> pixels, std::string& error, bool verify = false);
    std::uint64_t frames() const;
private:
    friend class SceneRenderer;
    struct Impl;
    std::unique_ptr<Impl> p;
};
}

// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "enhanced/lighting/probes.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"

namespace openrac::lighting {
// Compute writes complete per-tile candidate masks. Fragments consume the
// same GPU buffer through a texture-buffer view, without selection readback.
class ProbeSelection {
public:
    static constexpr unsigned capacity = 128, tile_size = 16;
    static bool supported();
    bool prepare(
        std::span<const lighting::ProbeGrid> grids,
        std::span<const unsigned> offsets,
        std::string& error
    );
    void dispatch(unsigned depth, const renderer::Camera& camera, int width, int height);
    // Only receivers represented by the culling depth may use its tile masks.
    void bind(
        const renderer::Shader& shader,
        const renderer::Camera* camera = nullptr,
        bool depth_matched = true
    ) const;

    bool available() const { return m_cull.id() != 0; }

    bool active() const { return available() && m_started; }

    void release();

private:
    renderer::Shader m_cull;
    mutable std::array<unsigned, 2> m_buffers{};
    mutable unsigned m_mask_texture = 0;
    int m_width = 0, m_height = 0;
    renderer::Mat4 m_clip = renderer::identity();
    void ensure_buffers() const;
    unsigned m_count = 0;
    bool m_started = false;
};
}  // namespace openrac::lighting

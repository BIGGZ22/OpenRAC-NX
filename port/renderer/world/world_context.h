// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the world renderers share: the frame's WorldFrame, the camera as the
// game sees it, the WorldView and WorldLights uniform blocks every world
// shader reads, the sampler objects, the game's own mip levels uploaded into
// the pool's textures, a scratch copy of the frame for the full-screen
// passes, and a full-screen triangle.
//
// One WorldContext is made per Renderer and handed to each world renderer
// (std::shared_ptr). Before Renderer::render, the game sets the frame:
//
//     context->set_frame(&world_frame);
//     renderer.render(input, target);

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "renderer/renderer.h"
#include "renderer/world/gpu.h"
#include "renderer/world/profile.h"
#include "renderer/world/world_frame.h"

namespace openrac::renderer::world {

class WorldContext {
public:
    WorldContext();
    ~WorldContext();
    WorldContext(const WorldContext&) = delete;
    WorldContext& operator=(const WorldContext&) = delete;

    // Create the shared GL objects (once; later calls do nothing). Fails when
    // world::gl::load() has not run.
    bool init(std::string& error);
    void release();

    void set_frame(const WorldFrame* frame) { m_frame = frame; }

    // The frame's state; an empty frame when none was set.
    const WorldFrame& frame() const { return m_frame != nullptr ? *m_frame : m_empty; }

    // At the start of each world renderer's render(): the camera from
    // `input`, the uniform blocks written and bound.
    void prepare(const FrameInput& input, const RenderTarget& target);

    const WorldCamera& camera() const { return m_camera; }

    const Camera& gl_camera() const { return m_gl_camera; }

    // Bind a world texture with a sampler state to `unit`: its GL texture
    // from the pool, with the game's mip levels uploaded on first use. A
    // texture of 0 binds the pool's placeholder.
    void bind_texture(
        TexturePool& pool, const WorldTexture& texture, const SamplerState& sampler, int unit
    );

    // Bind `gl_texture` with a sampler state to `unit`.
    void bind_gl_texture(unsigned gl_texture, const SamplerState& sampler, int unit);

    // Copy the target's colour into the scratch texture (resized as needed)
    // and return it, for a pass that reads the frame and writes it back.
    unsigned copy_target(const RenderTarget& target);

    // Draw one triangle covering the viewport (shaders/world/fullscreen.vert).
    void draw_fullscreen();

    SamplerCache& samplers() { return m_samplers; }

    // The game version's frame and constants (profile.h); RAC1 PAL's until set.
    void set_profile(const WorldProfile& profile) { m_profile = profile; }

    const WorldProfile& profile() const { return m_profile; }

private:
    struct Uploaded {
        unsigned gl = 0;
        TextureHandle handle = 0;
    };

    WorldProfile m_profile = *world_profile(assets::Game::Rac1, assets::Region::Pal);
    const WorldFrame* m_frame = nullptr;
    WorldFrame m_empty;
    WorldCamera m_camera;
    Camera m_gl_camera;
    GpuBuffer m_view_block;
    GpuBuffer m_lights_block;
    SamplerCache m_samplers;
    std::vector<Uploaded> m_uploaded;
    unsigned m_scratch = 0;
    int m_scratch_width = 0;
    int m_scratch_height = 0;
    unsigned m_scratch_framebuffer = 0;
    unsigned m_empty_vao = 0;
    bool m_initialised = false;
};

}  // namespace openrac::renderer::world

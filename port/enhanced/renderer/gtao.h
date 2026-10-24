// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once

#include "renderer/ambient_occlusion.h"
#include "renderer/framebuffer.h"
#include "renderer/shader.h"

namespace openrac::renderer {
struct FrameInput;
struct RenderTarget;

// Optional, spatial-only ground-truth ambient occlusion. The target supplies smooth normals
// from the world prepass; depth is used for positions and horizons, never to derive normals.
// The horizon estimate is filtered at full size. Modern
// lighting consumes visibility_texture() after render(..., false); the default
// colour composite is retained only for legacy prelit paths. evaluate_only
// exposes the raw horizon/guide texture for fused early-lighting denoising.
//
// The renderer's ambient occlusion hook (renderer/ambient_occlusion.h): a program registers one
// with Renderer::set_ambient_occlusion, and the tests also run it alone.
class Gtao final : public AmbientOcclusion {
public:
    bool render(
        const FrameInput& input,
        const RenderTarget& target,
        bool apply_to_colour = true,
        bool evaluate_only = false
    ) override;

    // Packed RGBA8 texture-buffer view, one AO/guide record per evaluation cell.
    unsigned raw_texture() const override { return m_ao_texture; }

    unsigned visibility_texture() const override { return m_visibility.colour_texture(); }

    // Depth sampled by the last successful render; may be borrowed from its target.
    unsigned depth_texture() const override { return m_sampled_depth; }

    void release() override;

private:
    bool prepare(int width, int height, int scale, bool visibility, bool copy_depth);
    FrameBuffer m_depth;
    unsigned m_sampled_depth = 0;  // Borrowed target depth or our owned copy for this render.
    unsigned m_ao_buffer = 0, m_ao_texture = 0;
    int m_ao_width = 0, m_ao_height = 0;
    FrameBuffer m_visibility;
    Shader m_evaluate;
    Shader m_evaluate_performance;
    Shader m_composite;
    unsigned m_vao = 0;
    bool m_failed = false;
};
}  // namespace openrac::renderer

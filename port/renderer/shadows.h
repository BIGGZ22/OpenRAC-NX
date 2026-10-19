// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once
#include "renderer/renderer.h"
#include "renderer/shader.h"

namespace openrac::renderer {
class ShadowRenderer : public BucketRenderer {
public:
    ShadowRenderer() : BucketRenderer("ratchet shadows", Bucket::Mobys) {}

    bool init(RenderState&, std::string& error) override;
    void render(const FrameInput& input, RenderState& state) override;
    void release() override;

private:
    Shader m_volume, m_composite;
    unsigned m_vao = 0, m_buffer = 0;
};
}  // namespace openrac::renderer

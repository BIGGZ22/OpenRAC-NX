// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// FrameTarget allocation uses the renderer's ordinary colour attachments for intermediate passes.
#include "enhanced/fsr3/frame_target.h"

#include "renderer/gl.h"

namespace openrac::enhanced::fsr3_detail {
using namespace openrac::gl;

FrameTarget::~FrameTarget() {
    release();
}

bool FrameTarget::create(
    int width, int height, unsigned format, unsigned depth, std::string& error
) {
    release();
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    // Floating-point intermediates are allocated without a CPU upload.
    glTexImage2D(
        GL_TEXTURE_2D, 0, static_cast<int>(format), width, height, 0, GL_RGBA, GL_FLOAT, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    // Velocity geometry tests against the current world depth without modifying it.
    if (depth) {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0
        );
    }

    // An incomplete target must never reach a fullscreen pass or SDK dispatch.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        error = "cannot create temporal frame target";
        release();
        return false;
    }

    return true;
}

void FrameTarget::release() {
    // Zero names are valid no-op deletions, including partially constructed targets.
    if (m_framebuffer) {
        glDeleteFramebuffers(1, &m_framebuffer);
    }

    // A failed allocation may leave only the framebuffer name populated.
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
    }

    m_framebuffer = 0;
    m_texture = 0;
}
}  // namespace openrac::enhanced::fsr3_detail

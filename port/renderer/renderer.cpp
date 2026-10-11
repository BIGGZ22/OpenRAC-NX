// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/renderer.h"

#include "renderer/direct.h"
#include "renderer/gl.h"
#include "renderer/subsystems.h"

namespace openrac::renderer {

using namespace openrac::gl;

const char* bucket_name(Bucket bucket) {
    switch (bucket) {
        case Bucket::Sky:
            return "sky";
        case Bucket::Terrain:
            return "terrain";
        case Bucket::Ties:
            return "ties";
        case Bucket::Shrubs:
            return "shrubs";
        case Bucket::Mobys:
            return "mobys";
        case Bucket::Particles:
            return "particles";
        case Bucket::Hud:
            return "hud";
        case Bucket::Count:
            break;
    }
    return "?";
}

Renderer::~Renderer() = default;

bool Renderer::init(std::string& error) {
    RenderState state{m_textures, {}, {}};
    for (auto& r : m_renderers) {
        if (!r->init(state, error)) {
            return false;
        }
    }
    m_initialised = true;
    return true;
}

BucketRenderer* Renderer::add(std::unique_ptr<BucketRenderer> renderer, std::string& error) {
    if (m_initialised) {
        RenderState state{m_textures, {}, {}};
        if (!renderer->init(state, error)) {
            return nullptr;
        }
    }
    m_renderers.push_back(std::move(renderer));
    return m_renderers.back().get();
}

bool Renderer::add_game_renderers(std::string& error) {
    return add(std::make_unique<SkyRenderer>(), error) != nullptr
           && add(std::make_unique<TfragRenderer>(), error) != nullptr
           && add(std::make_unique<TieRenderer>(), error) != nullptr
           && add(std::make_unique<ShrubRenderer>(), error) != nullptr
           && add(std::make_unique<MobyRenderer>(), error) != nullptr
           && add(std::make_unique<ParticleRenderer>(), error) != nullptr
           && add(std::make_unique<DirectRenderer>("hud", Bucket::Hud), error) != nullptr;
}

void Renderer::render(const FrameInput& input, const RenderTarget& target) {
    glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
    glViewport(0, 0, target.width, target.height);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glClearColor(
        input.clear_colour[0], input.clear_colour[1], input.clear_colour[2], input.clear_colour[3]
    );
    // Reversed depth: nothing is in front of 0, and the tests are GEQUAL.
    glClearDepth(0.0);
    glClear((input.keep_colour ? 0 : GL_COLOR_BUFFER_BIT) | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);

    RenderState state{m_textures, target, {}};
    // The HUD's renderer is the last of its bucket (the part of the 2D path before the world is
    // drawn by one ahead of the world's).
    std::size_t hud = m_renderers.size();
    for (std::size_t i = 0; i < m_renderers.size(); ++i) {
        if (m_renderers[i]->bucket() == Bucket::Hud) {
            hud = i;
        }
    }
    for (std::size_t i = 0; i < m_renderers.size(); ++i) {
        if (i == hud && input.before_hud) {
            input.before_hud();
            glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
            glViewport(0, 0, target.width, target.height);
        }
        auto& r = m_renderers[i];
        if (r->enabled) {
            r->render(input, state);
        }
    }
    m_stats = state.stats;
    m_textures.end_frame();
}

void Renderer::clear_renderers() {
    for (auto& r : m_renderers) {
        r->release();
    }
    m_renderers.clear();
}

void Renderer::release() {
    for (auto& r : m_renderers) {
        r->release();
    }
    m_textures.release_gl();
}

}  // namespace openrac::renderer

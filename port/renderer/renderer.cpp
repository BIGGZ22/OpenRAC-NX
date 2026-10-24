// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/renderer.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "common/stats.h"
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

unsigned readable(const RenderTarget& target) {
    if (target.resolve_framebuffer == 0) {
        return target.framebuffer;
    }
    if (target.resolves_itself) {
        return target.resolve_framebuffer;
    }
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, target.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.resolve_framebuffer);
    glBlitFramebuffer(
        0,
        0,
        target.width,
        target.height,
        0,
        0,
        target.width,
        target.height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );
    return target.resolve_framebuffer;
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

void Renderer::render(const FrameInput& input, const RenderTarget& source) {
    RenderTarget target = source;
    // [debug tools] The frame's target set and cleared, timed on the CPU only: on Vulkan the clear
    // is recorded with the first pass that draws.
    {
        const stats::Scope clear("frame clear");
        glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
        glViewport(0, 0, target.width, target.height);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glClearColor(
            input.clear_colour[0],
            input.clear_colour[1],
            input.clear_colour[2],
            input.clear_colour[3]
        );
        // Reversed depth: nothing is in front of 0, and the tests are GEQUAL.
        glClearDepth(0.0);
        glClear(
            (input.keep_colour ? 0 : GL_COLOR_BUFFER_BIT) | GL_DEPTH_BUFFER_BIT
            | GL_STENCIL_BUFFER_BIT
        );
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GEQUAL);
    }

    RenderState state{m_textures, target, {}};
    const bool enhanced_available =
        std::any_of(m_renderers.begin(), m_renderers.end(), [](const auto& r) {
            return r->enabled && r->enhanced_lighting_available();
        });
    // Drawn when the frame asks, a renderer lights with it and a program registered one.
    const bool gtao = input.gtao && enhanced_available && m_ambient_occlusion != nullptr;
    bool scoped_ao = false, needs_indirect_ao = false;
    for (const auto& r : m_renderers) {
        if (!r->enabled || r->bucket() < Bucket::Terrain || r->bucket() > Bucket::Mobys) {
            continue;
        }
        const auto mode = r->ao_lighting();
        scoped_ao |= mode != AoLighting::Legacy;
        needs_indirect_ao |= mode == AoLighting::Indirect;
    }
    const bool early_lighting =
        std::any_of(m_renderers.begin(), m_renderers.end(), [&](const auto& r) {
            return r->enabled && r->needs_early_lighting(input);
        });
    const bool prepass_needed = early_lighting || input.after_world_depth
                                || (gtao && needs_indirect_ao && input.gtao_settings.strength > 0);
    static const bool& discard_prepass = stats::probe(
        "OPENRAC_DEPTH_REUSE_OFF",
        "Profiling reference: discard GTAO prepass depth before world colour"
    );
    const auto world_prepass = [&] {
        // Early background/UI and sky have already drawn. Depth now belongs
        // exclusively to the world and remains intact through its colour passes.
        glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
        glViewport(0, 0, target.width, target.height);
        glDepthMask(GL_TRUE);
        glClearDepth(0);
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        state.depth_prepass = true;
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        {
            const stats::Scope depth("world depth prepass", stats::kGpu);
            for (const auto& r : m_renderers) {
                if (r->enabled) {
                    r->render_depth(input, state);
                }
            }
        }
        state.depth_prepass = false;
        state.reuse_prepass_depth = true;
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        unsigned resolved_depth = 0;
        if (gtao && needs_indirect_ao
            && m_ambient_occlusion->render(input, target, false, early_lighting)) {
            if (early_lighting) {
                state.gtao_raw = m_ambient_occlusion->raw_texture();
                state.gtao_scale = input.gtao_settings.half_resolution ? 2 : 1;
                state.gtao_strength = input.gtao_settings.strength;
            } else {
                state.gtao_visibility = m_ambient_occlusion->visibility_texture();
            }
            state.stats.draw_calls += early_lighting ? 0 : 1;
            state.stats.triangles += early_lighting ? 0 : 1;
            resolved_depth = m_ambient_occlusion->depth_texture();
        }
        if (input.after_world_depth || early_lighting) {
            const stats::Scope selection("probe coverage selection", stats::kGpu);
            std::string error;
            if (!resolved_depth
                && ((m_selection_depth.width() == target.width
                     && m_selection_depth.height() == target.height)
                    || m_selection_depth.create(target.width, target.height, error, 1, true))) {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, target.framebuffer);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_selection_depth.id());
                glBlitFramebuffer(
                    0,
                    0,
                    target.width,
                    target.height,
                    0,
                    0,
                    target.width,
                    target.height,
                    GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                    GL_NEAREST
                );
                resolved_depth = m_selection_depth.depth_texture();
            }
            if (resolved_depth && input.after_world_depth) {
                input.after_world_depth(resolved_depth, target.width, target.height);
            }
        }
        if (resolved_depth && early_lighting) {
            for (const auto& r : m_renderers) {
                if (r->enabled && r->needs_early_lighting(input)) {
                    r->render_early_lighting(resolved_depth, input, state);
                }
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
        glViewport(0, 0, target.width, target.height);
        glDepthMask(GL_TRUE);
        if (discard_prepass) {
            glClearDepth(0);
            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            state.reuse_prepass_depth = false;
        }
        stats::gauge("world depth reused", state.reuse_prepass_depth ? 1 : 0);
    };
    // The HUD's renderer is the last of its bucket (the part of the 2D path before the world is
    // drawn by one ahead of the world's).
    std::size_t hud = m_renderers.size();
    std::size_t ao_before = 0;
    for (std::size_t i = 0; i < m_renderers.size(); ++i) {
        const Bucket bucket = m_renderers[i]->bucket();
        // DirectBackground has a HUD bucket but runs before the world. AO must
        // follow the last world pass, not the first pass labelled as UI.
        if (bucket >= Bucket::Terrain && bucket <= Bucket::Mobys) {
            ao_before = i + 1;
        }
        if (m_renderers[i]->bucket() == Bucket::Hud) {
            hud = i;
        }
    }
    bool ao_applied = false;
    bool prepass_done = false;
    bool opaque_done = false;
    bool particles_started = false;
    std::vector<bool> opaque_prepared(m_renderers.size());
    const bool batch_opaque =
        std::any_of(m_renderers.begin(), m_renderers.end(), [](const auto& r) {
            return r->enabled && r->batches_opaque_colour();
        });
    for (std::size_t i = 0; i < m_renderers.size(); ++i) {
        const auto bucket = m_renderers[i]->bucket();
        if (prepass_needed && !prepass_done && m_renderers[i]->enabled && bucket >= Bucket::Terrain
            && bucket <= Bucket::Mobys) {
            world_prepass();
            prepass_done = true;
        }
        if (batch_opaque && !opaque_prepared[i] && state.reuse_prepass_depth
            && m_renderers[i]->enabled && m_renderers[i]->batches_opaque_colour()) {
            // Extend only across layers with no blended work between their
            // opaque draws. Include a final layer's opaque draws, then stop
            // before its translucent/glow/effect passes.
            std::size_t end = i;
            for (; end < m_renderers.size(); ++end) {
                const auto& next = m_renderers[end];
                if (!next->enabled) {
                    continue;
                }
                if (!next->batches_opaque_colour()) {
                    break;
                }
                if (next->opaque_colour_barrier()) {
                    ++end;
                    break;
                }
            }
            const stats::Scope opaque("world opaque colour", stats::kGpu);
            for (unsigned phase = 0; phase < 2; ++phase) {
                for (std::size_t n = i; n < end; ++n) {
                    const auto& next = m_renderers[n];
                    if (next->enabled && next->batches_opaque_colour()) {
                        next->render_opaque_colour(phase, input, state);
                        opaque_prepared[n] = true;
                    }
                }
            }
            opaque_done = true;
        }
        state.opaque_colour_batched = opaque_prepared[i];
        if (ao_before != 0 && i == ao_before) {
            if (!scoped_ao && gtao && m_ambient_occlusion->render(input, target, true, false)) {
                state.stats.draw_calls += 1;
                state.stats.triangles += 1;
            }
            ao_applied = true;
        }
        // Opaque colour is retained once, even when the particle bucket is empty or disabled.
        if (!particles_started && bucket == Bucket::Particles && input.on_before_particles) {
            input.on_before_particles(target);
            particles_started = true;
            glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
            glViewport(0, 0, target.width, target.height);
        }

        // Temporal reconstruction finishes before any HUD geometry uses display-resolution coordinates.
        if (i == hud && input.on_resolve_world) {
            input.on_resolve_world(target);
            state.target = target;
            glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
            glViewport(0, 0, target.width, target.height);
            glDisable(GL_SCISSOR_TEST);
            glDepthMask(GL_TRUE);
            glClearDepth(0);
            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        if (i == hud && input.before_hud) {
            input.before_hud();
            glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
            glViewport(0, 0, target.width, target.height);
        }
        auto& r = m_renderers[i];
        if (r->enabled) {
            // [debug tools] The pass, timed and counted for the overlay and the dump (common/stats.h).
            const stats::Scope pass(r->name(), r->owns_gpu_timing() ? 0 : stats::kGpu);
            r->render(input, state);
        }
    }
    if (!scoped_ao && !ao_applied && ao_before != 0 && gtao
        && m_ambient_occlusion->render(input, target, true, false)) {
        state.stats.draw_calls += 1;
        state.stats.triangles += 1;
    }
    // Nothing reads the frame's depth and stencil after it (the next frame clears them): a tiled
    // GPU (a phone's) then does not write them back to memory at the end of the pass, which it does
    // for every attachment it is not told to drop (gl.h, OPENRAC_GL_ES_OPTIONAL_FUNCTIONS).
    if (glInvalidateFramebuffer != nullptr && target.framebuffer != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
        const GLenum dropped[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, dropped);
    }
    stats::gauge("opaque colour batching", opaque_done ? 1 : 0);
    m_stats = state.stats;
    // [debug tools] The textures no pass drew with for a while let go, which walks every entry.
    const stats::Scope retire("texture retire");
    m_textures.end_frame();
}

void Renderer::clear_renderers() {
    for (auto& r : m_renderers) {
        r->release();
    }
    m_renderers.clear();
}

void Renderer::release() {
    m_selection_depth.release();
    // Only a registered ambient occlusion has GL objects to free.
    if (m_ambient_occlusion != nullptr) {
        m_ambient_occlusion->release();
    }
    for (auto& r : m_renderers) {
        r->release();
    }
    m_textures.release_gl();
}

void Renderer::set_ambient_occlusion(std::unique_ptr<AmbientOcclusion> ambient_occlusion) {
    m_ambient_occlusion = std::move(ambient_occlusion);
}

}  // namespace openrac::renderer

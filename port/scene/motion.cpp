// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// LevelScene's velocity draw reuses uploaded geometry and alpha masks, including live joint palettes.
#include <algorithm>
#include <limits>

#include "common/stats.h"
#include "renderer/gl.h"
#include "scene/level_renderer.h"

namespace openrac::scene {
using namespace openrac::gl;

void LevelScene::draw_motion(
    const renderer::Shader& shader,
    std::span<const MotionInstance> instances,
    const MotionView& view,
    renderer::TexturePool& textures,
    MotionSubmission submission
) {
    shader.use();
    glUniformMatrix4fv(shader.uniform("raster"), 1, GL_FALSE, view.raster.data());
    glUniformMatrix4fv(shader.uniform("current_view"), 1, GL_FALSE, view.current.data());
    glUniformMatrix4fv(shader.uniform("previous_view"), 1, GL_FALSE, view.previous.data());
    glUniform2f(
        shader.uniform("render_size"),
        static_cast<float>(view.size[0]),
        static_cast<float>(view.size[1])
    );
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_palette);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, view.previous_palette);
    glActiveTexture(GL_TEXTURE0);

    // Even the individual shader branch needs a valid descriptor for its unused record sampler.
    if (!m_motion_records_ready) {
        m_motion_records.upload({}, renderer::world::RecordBuffer::Format::Rgba32ui, true);
        m_motion_records_ready = true;
    }

    GLint texels = 0;
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &texels);
    // Nine RGBA texels hold a pose; an oversized input retains the individual path.
    const bool batched = submission == MotionSubmission::Batched
                         && instances.size() <= static_cast<std::size_t>(std::max(texels, 0) / 9);
    glUniform1i(shader.uniform("motion_instances"), batched ? 1 : 0);

    // Pack once rather than updating pose uniforms before every draw.
    if (batched) {
        prepare_motion_batches(instances);
        m_motion_records
            .upload(m_motion_words.data(), renderer::world::RecordBuffer::Format::Rgba32ui, true);
    }

    m_motion_records.bind(3);
    glActiveTexture(GL_TEXTURE0);

    // Both paths share the same textures, pose history and coverage shader.
    if (batched) {
        draw_motion_batches(shader, textures);
    } else {
        // Only opaque coverage overwrites camera motion; translucent effects use the reactive mask.
        for (const auto& instance : instances) {
            draw_motion_instance(shader, instance, textures);
        }
    }

    glBindVertexArray(0);
}

void LevelScene::prepare_motion_batches(std::span<const MotionInstance> instances) {
    const auto& groups = m_groups[static_cast<std::size_t>(Layer::Mobys)];
    constexpr unsigned kMissing = std::numeric_limits<unsigned>::max();
    m_motion_groups.assign(m_models.size(), kMissing);
    m_motion_batches.clear();
    m_motion_words.data().clear();

    // Keep the first geometry VAO for each model, matching individual-draw lookup.
    for (std::size_t n = 0; n < groups.size(); ++n) {
        const std::uint32_t model = groups[n].model;
        if (model < m_motion_groups.size() && m_motion_groups[model] == kMissing) {
            m_motion_groups[model] = static_cast<unsigned>(n);
        }
    }

    for (const auto& instance : instances) {
        // Missing and invisible objects never own opaque depth.
        if (instance.model >= m_motion_groups.size() || instance.alpha <= 0
            || m_motion_groups[instance.model] == kMissing) {
            continue;
        }
        // Multiple materials retain instance-major order, including equal-depth alpha coverage.
        if (m_motion_batches.empty() || m_motion_batches.back().model != instance.model
            || m_models[instance.model].primitives.size() != 1) {
            m_motion_batches.push_back(
                {instance.model,
                 m_motion_groups[instance.model],
                 static_cast<int>(m_motion_words.size() / 36),
                 0}
            );
        }
        ++m_motion_batches.back().count;
        // Two matrices plus palettes, opacity and the reactive/glow bits occupy 36 words.
        m_motion_words.mat(instance.current)
            .mat(instance.previous)
            .f({static_cast<float>(instance.palettes[0]),
                static_cast<float>(instance.palettes[1]),
                instance.alpha})
            .u((instance.reactive ? 1u : 0u) | (instance.exclude_glow ? 2u : 0u));
    }

    const std::size_t instance_count = m_motion_words.size() / 36;
    stats::gauge("FSR motion instances", static_cast<double>(instance_count));
    stats::gauge("FSR motion batches", static_cast<double>(m_motion_batches.size()));
}

void LevelScene::draw_motion_batches(
    const renderer::Shader& shader, renderer::TexturePool& textures
) {
    const auto& groups = m_groups[static_cast<std::size_t>(Layer::Mobys)];

    // Stable consecutive runs leave cross-model and multi-material depth ties in their original order.
    for (const auto& batch : m_motion_batches) {
        glBindVertexArray(groups[batch.group].vao);
        glUniform1i(shader.uniform("motion_first"), batch.first);
        draw_motion_materials(shader, batch.model, textures, batch.count);
    }
}

void LevelScene::draw_motion_instance(
    const renderer::Shader& shader, const MotionInstance& instance, renderer::TexturePool& textures
) {
    const auto& groups = m_groups[static_cast<std::size_t>(Layer::Mobys)];
    const auto group = std::find_if(groups.begin(), groups.end(), [&](const Group& candidate) {
        return candidate.model == instance.model;
    });

    // Missing models and invisible instances cannot contribute a velocity surface.
    if (group == groups.end() || instance.model >= m_models.size() || instance.alpha <= 0) {
        return;
    }

    glBindVertexArray(group->vao);
    glUniformMatrix4fv(shader.uniform("current_model"), 1, GL_FALSE, instance.current.data());
    glUniformMatrix4fv(shader.uniform("previous_model"), 1, GL_FALSE, instance.previous.data());
    glUniform2f(
        shader.uniform("palettes"),
        static_cast<float>(instance.palettes[0]),
        static_cast<float>(instance.palettes[1])
    );
    glUniform1f(shader.uniform("instance_alpha"), instance.alpha);
    glUniform1f(shader.uniform("reactive"), instance.reactive ? 1.0f : 0.0f);
    glUniform1i(shader.uniform("exclude_glow"), instance.exclude_glow ? 1 : 0);
    draw_motion_materials(shader, instance.model, textures);
}

void LevelScene::draw_motion_materials(
    const renderer::Shader& shader,
    std::uint32_t model,
    renderer::TexturePool& textures,
    int instances
) {
    // Material runs use their original alpha texture, matching world depth coverage.
    for (const auto& primitive : m_models[model].primitives) {
        const auto& material = m_materials[static_cast<std::size_t>(primitive.material)];

        // Additive and metal passes never own opaque depth.
        if (material.blend || material.metal) {
            continue;
        }

        const auto texture =
            material.image < 0 ? m_white : m_textures[static_cast<std::size_t>(material.image)];
        glBindTexture(GL_TEXTURE_2D, textures.gl_texture(texture));
        glUniform1f(shader.uniform("material_alpha"), material.colour[3]);
        // Indices are byte offsets into the VAO's element buffer, as required by GL.
        // Single-instance submissions keep the original reference command.
        if (instances == 1) {
            glDrawElements(
                GL_TRIANGLES,
                static_cast<int>(primitive.index_count),
                GL_UNSIGNED_INT,
                // NOLINTNEXTLINE(performance-no-int-to-ptr): GL requires pointer-valued buffer offsets.
                reinterpret_cast<const void*>(primitive.first_index * sizeof(std::uint32_t))
            );
        } else {
            glDrawElementsInstanced(
                GL_TRIANGLES,
                static_cast<int>(primitive.index_count),
                GL_UNSIGNED_INT,
                // NOLINTNEXTLINE(performance-no-int-to-ptr): GL requires pointer-valued buffer offsets.
                reinterpret_cast<const void*>(primitive.first_index * sizeof(std::uint32_t)),
                instances
            );
        }
    }
}
}  // namespace openrac::scene

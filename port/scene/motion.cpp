// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// LevelScene's velocity draw reuses uploaded geometry and alpha masks, including live joint palettes.
#include <algorithm>

#include "renderer/gl.h"
#include "scene/level_renderer.h"

namespace openrac::scene {
using namespace openrac::gl;

void LevelScene::draw_motion(
    const renderer::Shader& shader,
    std::span<const MotionInstance> instances,
    const MotionView& view,
    renderer::TexturePool& textures
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

    // Only opaque coverage overwrites camera motion; translucent effects use the reactive mask.
    for (const auto& instance : instances) {
        draw_motion_instance(shader, instance, textures);
    }

    glBindVertexArray(0);
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
    const renderer::Shader& shader, std::uint32_t model, renderer::TexturePool& textures
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
        glDrawElements(
            GL_TRIANGLES,
            static_cast<int>(primitive.index_count),
            GL_UNSIGNED_INT,
            // NOLINTNEXTLINE(performance-no-int-to-ptr): GL requires pointer-valued buffer offsets.
            reinterpret_cast<const void*>(primitive.first_index * sizeof(std::uint32_t))
        );
    }
}
}  // namespace openrac::scene

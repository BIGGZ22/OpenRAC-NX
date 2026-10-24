// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Fsr3 frame resolve: velocities, particle reactivity, SDK dispatch and displayed-pose history.
#include <algorithm>
#include <cmath>

#include "common/log.h"
#include "enhanced/fsr3/fsr3_internal.h"
#include "renderer/gl.h"
#include "scene/level_renderer.h"

namespace openrac::enhanced {
using namespace openrac::gl;

void Fsr3::State::build_velocities() {
    velocities.clear();

    // Map by stable identity, never by draw order; disappearing objects cannot shift another object's history.
    for (const auto& instance : instances) {
        // Blended mobys have no opaque depth and belong in the reactive colour difference.
        if (instance.blend) {
            continue;
        }

        const auto old = previous.find(instance.temporal_id);
        bool matched = !dispatch.reset && instance.temporal_id != 0 && old != previous.end();
        matched = matched && old->second.model == instance.model;

        // A replaced skeleton or a large translation must not reproject an unrelated pose.
        if (matched) {
            float movement = 0;

            // The fourth matrix column contains the world translation.
            for (std::size_t n = 12; n < 15; ++n) {
                const float delta = instance.matrix[n] - old->second.matrix[n];
                movement += delta * delta;
            }

            matched = movement < 64 && (instance.palette < 0) == (old->second.palette < 0);
        }

        const auto& from = matched ? old->second : instance;
        scene::MotionInstance velocity;
        velocity.model = instance.model;
        velocity.current = instance.matrix;
        velocity.previous = from.matrix;
        velocity.palettes = {instance.palette, from.palette};
        velocity.alpha = instance.tint[3];
        velocity.reactive = !matched;
        velocity.exclude_glow =
            std::abs(instance.moby_light[3]) > .5f && instance.moby_ambient[3] > .5f;
        velocities.push_back(velocity);
    }
}

void Fsr3::State::save_palette() {
    // The vertex shader uses four texels per matrix, with 256 matrices per row.
    constexpr std::size_t kMatricesPerRow = 256;
    const std::size_t rows =
        std::max(std::size_t{1}, (palette.size() + kMatricesPerRow - 1) / kMatricesPerRow);
    std::vector<renderer::Mat4> padded(rows * kMatricesPerRow);
    std::copy(palette.begin(), palette.end(), padded.begin());

    // The first displayed frame has no previous joint image yet.
    if (!previous_palette) {
        glGenTextures(1, &previous_palette);
    }

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, previous_palette);
    // RGBA32F retains the exact joint columns consumed by the current-frame skinning shader.
    glTexImage2D(
        GL_TEXTURE_2D, 0, 0x8814, 1024, static_cast<int>(rows), 0, GL_RGBA, GL_FLOAT, padded.data()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glActiveTexture(GL_TEXTURE0);
}

void Fsr3::State::capture_opaque(const renderer::RenderTarget& target) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, target.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, opaque.framebuffer());
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
    has_opaque = true;
}

void Fsr3::State::draw_camera_motion() {
    // Fullscreen passes use no mesh input, depth test, blending or stencil mask.
    if (!vao) {
        glGenVertexArrays(1, &vao);
    }

    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDepthMask(GL_FALSE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glViewport(0, 0, world.width(), world.height());
    camera_motion.use();
    glUniformMatrix4fv(camera_motion.uniform("inverse_raster"), 1, GL_FALSE, inverse_raster.data());
    glUniformMatrix4fv(camera_motion.uniform("previous_view"), 1, GL_FALSE, view.previous.data());
    glUniform3f(
        camera_motion.uniform("camera_position"),
        camera.position[0],
        camera.position[1],
        camera.position[2]
    );
    glUniform2f(camera_motion.uniform("jitter"), dispatch.jitter[0], dispatch.jitter[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, world.depth_texture());
    glBindFramebuffer(GL_FRAMEBUFFER, depth.framebuffer());
    glUniform1i(camera_motion.uniform("depth_only"), 1);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindFramebuffer(GL_FRAMEBUFFER, motion.framebuffer());
    glUniform1i(camera_motion.uniform("depth_only"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Fsr3::State::draw_motion() {
    draw_camera_motion();

    // Fresh history uses current joints at both endpoints; the reactive flag rejects any old colour.
    if (dispatch.reset || !previous_palette) {
        save_palette();
    }

    build_velocities();
    view.previous_palette = previous_palette;
    // Attach world depth only after the fullscreen sampling passes have finished.
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, world.depth_texture(), 0
    );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    scene->draw_motion(object_motion, velocities, view, *textures);
    glDisable(GL_DEPTH_TEST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
}

void Fsr3::State::draw_reactive() {
    // A renderer without a particle bucket has no intervening colour changes.
    if (!has_opaque) {
        capture_opaque({world.id(), world.width(), world.height()});
    }

    glBindFramebuffer(GL_FRAMEBUFFER, reactive.framebuffer());
    glBindVertexArray(vao);
    reactive_mask.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, world.colour_texture());
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, opaque.texture());
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, motion.texture());
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(0);
}

void Fsr3::State::resolve(renderer::RenderTarget& target) {
    draw_motion();
    draw_reactive();
    std::string error;
    const std::array<unsigned, 5> inputs{
        world.colour_texture(),
        depth.texture(),
        motion.texture(),
        reactive.texture(),
        output_texture
    };
    const bool resolved = sdk.dispatch(inputs, dispatch, error);

    // A rejected dispatch still presents a complete world and full-resolution HUD, then stops retrying.
    if (!resolved) {
        log::error("{}", error);
        world.blit_to(display.framebuffer, display.width, display.height);
        failed = true;
    }

    previous.clear();

    // The next frame sees exactly the displayed transforms, including interpolation between simulation ticks.
    for (const auto& instance : instances) {
        previous.insert_or_assign(instance.temporal_id, instance);
    }

    save_palette();
    old_camera = camera;
    valid = resolved;
    target = display;
}
}  // namespace openrac::enhanced

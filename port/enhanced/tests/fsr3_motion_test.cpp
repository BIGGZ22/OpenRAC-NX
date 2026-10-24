// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Synthetic rigid and skinned velocity checks use a known quarter-unit pose change in identity clip space.
#include <algorithm>
#include <iostream>
#include <vector>

#include "enhanced/fsr3/frame_target.h"
#include "fsr3_shaders.h"
#include "renderer/framebuffer.h"
#include "enhanced/viewer/scene_lighting.h"
#include "renderer/gl.h"
#include "scene/level_renderer.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

namespace {
using namespace openrac;
using namespace openrac::gl;

// Float velocity is converted to UNORM only for readback; 16 pixels maps to one output colour unit.
constexpr const char* kReadMotion = R"glsl(#version 410 core
uniform sampler2D tex_T0;
out vec4 colour;
void main() { colour = vec4(texelFetch(tex_T0, ivec2(gl_FragCoord.xy), 0).xy / 16.0 + .5, 0, 1); }
)glsl";

// Geometry, shaders and targets owned by one render-thread fixture.
struct MotionFixture {
    MotionFixture() = default;
    MotionFixture(const MotionFixture&) = delete;
    MotionFixture& operator=(const MotionFixture&) = delete;
    MotionFixture(MotionFixture&&) = delete;
    MotionFixture& operator=(MotionFixture&&) = delete;
    renderer::Renderer renderer;                // Owns original alpha textures.
    scene::LevelScene scene;                   // Owns uploaded mesh and joint data.
    enhanced::fsr3_detail::FrameTarget motion;  // Signed RGBA16F velocity output.
    renderer::FrameBuffer output;               // UNORM readback target.
    renderer::Shader shader;                    // Production velocity shader.
    renderer::Shader read;                      // Test-only float-to-UNORM conversion.
    unsigned vao = 0;                           // Empty VAO for readback triangle.
    unsigned previous_palette = 0;              // Independent previous-frame joint image.

    // Creates a triangle whose first joint has unit weight at every vertex.
    bool create(std::string& error) {
        lighting::use_scene_lighting(scene);
        scene::LevelData level;
        level.vertices.resize(3);
        // An oversized triangle covers the centre without edge interpolation ambiguity.
        constexpr std::array<std::array<float, 3>, 3> kPositions{
            {{-1, -1, 0}, {3, -1, 0}, {-1, 3, 0}}
        };

        // Unit alpha and weight isolate pose motion from shading and material coverage.
        for (std::size_t n = 0; n < level.vertices.size(); ++n) {
            auto& vertex = level.vertices[n];
            std::copy(kPositions[n].begin(), kPositions[n].end(), vertex.position);
            std::fill_n(vertex.colour, 4, 1.0f);
            vertex.weights[0] = 1;
            vertex.light_slot = -1;
        }

        level.indices = {0, 1, 2};
        level.materials.emplace_back();
        level.models.emplace_back();
        level.models[0].primitives.push_back({0, 3, 0});
        level.models[0].joints = 1;
        glGenVertexArrays(1, &vao);
        const std::vector<scene::Instance> instances(1);

        // A fixture that cannot allocate its production pipeline cannot provide a meaningful sample.
        if (!renderer.init(error) || !scene.upload(level, renderer.textures(), error)
            || !motion.create(64, 64, 0x881A, 0, error) || !output.create(64, 64, error)
            || !shader.build(
                "motion fixture",
                enhanced::shaders::motion_vert,
                enhanced::shaders::motion_frag,
                error
            )
            || !read.build(
                "read motion fixture", enhanced::shaders::fullscreen_vert, kReadMotion, error
            )) {
            return false;
        }

        scene.set_instances(scene::Layer::Mobys, instances);
        // A palette row contains 256 matrices; the first is an identity joint.
        std::vector<renderer::Mat4> joints(256, renderer::identity());
        glGenTextures(1, &previous_palette);
        glBindTexture(GL_TEXTURE_2D, previous_palette);
        glTexImage2D(GL_TEXTURE_2D, 0, 0x8814, 1024, 1, 0, GL_RGBA, GL_FLOAT, joints.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        scene.set_palette({renderer::identity()});
        return true;
    }

    // Draws one production velocity instance and reads its signed centre velocity through a test shader.
    std::array<int, 2> sample(scene::MotionInstance instance) {
        glBindFramebuffer(GL_FRAMEBUFFER, motion.framebuffer());
        glViewport(0, 0, 64, 64);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        scene::MotionView view;
        view.raster = renderer::identity();
        view.current = renderer::identity();
        view.previous = renderer::identity();
        view.size = {64, 64};
        // Both poses index separate matrices in the same synthetic palette image.
        view.previous_palette = previous_palette;
        scene.draw_motion(shader, std::span(&instance, 1), view, renderer.textures());
        glBindFramebuffer(GL_FRAMEBUFFER, output.id());
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, motion.texture());
        read.use();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        const auto pixels = output.read_rgba();
        const std::size_t at = (std::size_t{32} * 64 + 32) * 4;
        return {pixels[at], pixels[at + 1]};
    }

    // Releases scene images before their pool and graphics context are destroyed.
    ~MotionFixture() {
        scene.release();
        motion.release();
        output.release();
        shader.release();
        read.release();
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures(1, &previous_palette);
        renderer.release();
    }
};

// A previous quarter-unit translation yields eight render pixels of current-to-previous motion.
void test_rigid_motion_and_new_object_rejection() {
    MotionFixture fixture;
    std::string error;
    const bool ready = fixture.create(error);
    CHECK(ready);

    // Setup diagnostics identify shader or allocation failures before any readback is attempted.
    if (!ready) {
        std::cerr << error << '\n';
        return;
    }

    scene::MotionInstance instance;
    instance.current = renderer::identity();
    instance.previous = renderer::translation({.25f, 0, 0});
    auto velocity = fixture.sample(instance);
    // Eight pixels / sixteen plus 0.5 is 1.0; stationary Y remains 0.5.
    CHECK(velocity[0] >= 253 && std::abs(velocity[1] - 128) <= 1);
    instance.reactive = true;
    instance.palettes = {-1, 100000};
    velocity = fixture.sample(instance);
    CHECK(std::abs(velocity[0] - 128) <= 1);
    CHECK(std::abs(velocity[1] - 128) <= 1);
}

// Skinning motion follows displayed joint deformation even when the object's world matrix stays still.
void test_skinned_motion() {
    MotionFixture fixture;
    std::string error;
    const bool ready = fixture.create(error);
    CHECK(ready);

    // Setup diagnostics identify shader or allocation failures before any readback is attempted.
    if (!ready) {
        std::cerr << error << '\n';
        return;
    }

    fixture.scene.set_palette({renderer::translation({0, .25f, 0})});
    scene::MotionInstance instance;
    instance.current = renderer::identity();
    instance.previous = renderer::identity();
    instance.palettes = {0, 0};
    const auto velocity = fixture.sample(instance);
    CHECK(std::abs(velocity[0] - 128) <= 1);
    CHECK(velocity[1] <= 2);
}
}  // namespace

int main() {
    try {
        test::GlContext context;
        std::string error;

        // Only an unavailable graphics context may skip the GPU fixture.
        if (!context.open(error)) {
            std::cerr << error << '\n';
            return test::kSkip;
        }

        test_rigid_motion_and_new_object_rejection();
        test_skinned_motion();
        return test::result();
    } catch (...) {
        return 1;
    }
}

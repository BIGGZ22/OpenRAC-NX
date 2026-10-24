// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Synthetic rigid and skinned velocity checks use a known quarter-unit pose change in identity clip space.
#include <algorithm>
#include <iostream>
#include <vector>

#include "common/stats.h"
#include "enhanced/fsr3/frame_target.h"
#include "enhanced/viewer/scene_lighting.h"
#include "fsr3_shaders.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/gl_stats.h"
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
void main() {
    vec4 motion = texelFetch(tex_T0, ivec2(gl_FragCoord.xy), 0);
    colour = vec4(motion.xy / 16.0 + .5, motion.z, 1);
}
)glsl";

// Builds three test models with optional cutouts and an overlapping two-material run.
scene::LevelData motion_level(bool cutout) {
    scene::LevelData level;
    level.vertices.resize(3);
    // An oversized triangle covers the centre without edge interpolation ambiguity.
    constexpr std::array<std::array<float, 3>, 3> kPositions{{{-1, -1, 0}, {3, -1, 0}, {-1, 3, 0}}};

    // Unit alpha and weight isolate pose motion from shading and material coverage.
    for (std::size_t n = 0; n < level.vertices.size(); ++n) {
        auto& vertex = level.vertices[n];
        std::copy(kPositions[n].begin(), kPositions[n].end(), vertex.position);
        std::fill_n(vertex.colour, 4, 1.0f);
        vertex.weights[0] = 1;
        vertex.light_slot = cutout ? -2.0f : -1.0f;
        vertex.uv[0] = (vertex.position[0] + 1.0f) * 0.5f;
    }

    level.indices = {0, 1, 2};
    level.materials.emplace_back();
    level.models.emplace_back();
    level.models[0].primitives.push_back({0, 3, 0});
    level.models[0].joints = 1;
    level.models.push_back(level.models[0]);
    level.models.push_back(level.models[0]);
    level.models[2].primitives.push_back({0, 3, 0});
    // Half of the alpha texture is transparent; the shader must retain its original coverage.
    if (cutout) {
        renderer::Rgba8Image image(2, 1);
        image.set_texel(0, 0, 0x00FFFFFFu);
        image.set_texel(1, 0, 0xFFFFFFFFu);
        level.images.push_back(image);
        level.image_paths.push_back("motion-cutout.png");
        level.translucent_images.push_back(true);
        level.materials[0].image = 0;
        level.materials[0].cutout = true;
    }
    return level;
}

// Geometry, shaders and targets owned by one render-thread fixture.
struct MotionFixture {
    MotionFixture() = default;
    MotionFixture(const MotionFixture&) = delete;
    MotionFixture& operator=(const MotionFixture&) = delete;
    MotionFixture(MotionFixture&&) = delete;
    MotionFixture& operator=(MotionFixture&&) = delete;
    renderer::Renderer renderer;                // Owns original alpha textures.
    scene::LevelScene scene;                    // Owns uploaded mesh and joint data.
    enhanced::fsr3_detail::FrameTarget motion;  // Signed RGBA16F velocity output.
    renderer::FrameBuffer output;               // UNORM readback target.
    renderer::Shader shader;                    // Production velocity shader.
    renderer::Shader read;                      // Test-only float-to-UNORM conversion.
    unsigned vao = 0;                           // Empty VAO for readback triangle.
    unsigned previous_palette = 0;              // Independent previous-frame joint image.

    // Creates a triangle whose first joint has unit weight at every vertex.
    bool create(std::string& error, bool cutout = false) {
        lighting::use_scene_lighting(scene);
        const auto level = motion_level(cutout);
        glGenVertexArrays(1, &vao);
        std::vector<scene::Instance> instances(3);
        instances[1].model = 1;
        instances[2].model = 2;

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

    // Reads all velocity and reactive channels for either submission path, including uncovered pixels.
    std::vector<std::uint8_t> sample_many(
        std::span<const scene::MotionInstance> instances, scene::MotionSubmission submission
    ) {
        glBindFramebuffer(GL_FRAMEBUFFER, motion.framebuffer());
        glViewport(0, 0, 64, 64);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glClearColor(-8, -8, 0.25f, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        scene::MotionView view;
        view.raster = renderer::identity();
        view.current = renderer::identity();
        view.previous = renderer::identity();
        view.size = {64, 64};
        // Both poses index separate matrices in the same synthetic palette image.
        view.previous_palette = previous_palette;
        scene.draw_motion(shader, instances, view, renderer.textures(), submission);
        glBindFramebuffer(GL_FRAMEBUFFER, output.id());
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, motion.texture());
        read.use();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        return output.read_rgba();
    }

    // Reads the signed centre velocity through the production batched path.
    std::array<int, 2> sample(scene::MotionInstance instance) {
        const auto pixels = sample_many(std::span(&instance, 1), scene::MotionSubmission::Batched);
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

// Creates overlapping rigid/skinned objects, model boundaries and distinct coverage/history flags.
std::vector<scene::MotionInstance> mixed_motion_instances() {
    std::vector<scene::MotionInstance> instances(12);

    // Consecutive runs mix rigid and independently skinned poses with distinct history flags.
    for (std::size_t n = 0; n < instances.size(); ++n) {
        auto& instance = instances[n];
        const std::size_t row = n / 4;
        instance.current = renderer::multiply(
            renderer::translation(
                {static_cast<float>(n % 4) * .4f - .6f, static_cast<float>(row) * .4f - .6f, 0}
            ),
            renderer::scale(.3f)
        );
        instance.previous =
            renderer::multiply(renderer::translation({.0625f, -.03125f, 0}), instance.current);
        instance.palettes = {static_cast<int>(n % 2), 0};
        instance.reactive = n % 3 == 0;
        instance.exclude_glow = n % 5 == 0;
    }
    instances[4].model = 1;
    instances[7].alpha = .5f;
    instances[8].model = 99;
    instances[9].model = 2;
    instances[10].model = 2;
    instances[0].palettes[1] = 100000;
    return instances;
}

// Checks exact velocities, opacity, reactive/glow flags and ordering against individual draws.
void test_batched_motion_matches_reference(bool cutout) {
    MotionFixture fixture;
    std::string error;
    const bool ready = fixture.create(error, cutout);
    CHECK(ready);

    // Failed allocation or shader setup cannot provide a meaningful comparison.
    if (!ready) {
        std::cerr << error << '\n';
        return;
    }

    fixture.scene.set_palette(
        {renderer::translation({0, .125f, 0}), renderer::translation({.125f, 0, 0})}
    );
    auto instances = mixed_motion_instances();
    const std::size_t counter = static_cast<std::size_t>(stats::Counter::DrawCalls);
    const std::uint64_t before = stats::g_counters[counter];
    const auto reference = fixture.sample_many(instances, scene::MotionSubmission::Individual);
    const std::uint64_t reference_draws = stats::g_counters[counter] - before;
    const std::uint64_t middle = stats::g_counters[counter];
    CHECK(fixture.sample_many(instances, scene::MotionSubmission::Batched) == reference);
    CHECK(stats::g_counters[counter] - middle < reference_draws);
    std::reverse(instances.begin(), instances.end());
    const auto reversed = fixture.sample_many(instances, scene::MotionSubmission::Individual);
    CHECK(fixture.sample_many(instances, scene::MotionSubmission::Batched) == reversed);
    instances.resize(2);
    const auto smaller = fixture.sample_many(instances, scene::MotionSubmission::Individual);
    CHECK(fixture.sample_many(instances, scene::MotionSubmission::Batched) == smaller);
    CHECK(glGetError() == GL_NO_ERROR);
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

        // Count native draw submissions as well as comparing the complete output images.
        renderer::gl_stats::install(platform::Window::gl_loader());
        test_rigid_motion_and_new_object_rejection();
        test_skinned_motion();
        test_batched_motion_matches_reference(false);
        test_batched_motion_matches_reference(true);
        return test::result();
    } catch (...) {
        return 1;
    }
}

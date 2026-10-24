// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Exercises AMD temporal dispatch on synthetic uniform colour, reversed depth and zero motion.
#include <array>
#include <iostream>
#include <vector>

#include "enhanced/fsr3/sdk_context.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/vulkan/vkgl.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

namespace {
using namespace openrac;
using namespace openrac::gl;

// A small synthetic scene with explicitly initialised temporal inputs and a storage-capable output.
struct Frame {
    Frame() = default;
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
    Frame(Frame&&) = delete;
    Frame& operator=(Frame&&) = delete;
    renderer::FrameBuffer world;         // Render-resolution colour and sampleable depth.
    renderer::FrameBuffer output;        // Display-resolution colour read back after dispatch.
    std::array<unsigned, 3> textures{};  // Motion, reactive mask and float depth input.

    // Allocates independent input images; no game data or external files participate.
    bool create(std::string& error) {
        // A 1.5x display-to-render ratio is the FSR Quality preset.
        if (!world.create(64, 48, error, 1, true) || !output.create(96, 72, error)) {
            return false;
        }

        glGenTextures(3, textures.data());
        std::vector<float> zero(std::size_t{64} * 48 * 4);

        // Motion is float RGBA; the SDK consumes its first two channels in render-pixel units.
        for (const auto texture : textures) {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, 0x8814, 64, 48, 0, GL_RGBA, GL_FLOAT, zero.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        std::vector<float> depth(std::size_t{64} * 48 * 4, .5f);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 48, GL_RGBA, GL_FLOAT, depth.data());
        return vkgl::allocate_storage_texture(output.colour_texture(), GL_RGBA8, 96, 72);
    }

    // Clears a finite reversed-depth surface with a known, uniform LDR colour.
    void clear(std::array<float, 3> colour) {
        glBindFramebuffer(GL_FRAMEBUFFER, world.id());
        glViewport(0, 0, 64, 48);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glClearColor(colour[0], colour[1], colour[2], 1);
        glClearDepth(0.5);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    // Releases owned images while the test's context is still current.
    ~Frame() {
        glDeleteTextures(3, textures.data());
        world.release();
        output.release();
    }
};

// Compares the centre RGB, outside the reconstruction filter's boundary footprint, in UNORM bytes.
void check_centre_colour(const renderer::FrameBuffer& output, std::array<float, 3> colour) {
    const auto pixels = output.read_rgba();

    // Three UNORM steps allow filter and quantisation rounding without accepting stale colour.
    for (unsigned c = 0; c < 3; ++c) {
        const auto at = (36 * 96 + 48) * 4 + c;
        CHECK(std::abs(static_cast<float>(pixels[at]) - colour[c] * 255) < 3);
    }
}

// Constant inputs must retain their RGB after upscaling, including a reset after a colour change.
void test_temporal_dispatch_preserves_constant_colour() {
    Frame frame;
    std::string error;
    CHECK(frame.create(error));
    enhanced::fsr3_detail::SdkContext upscaler;
    const bool configured = upscaler.configure({64, 48}, {96, 72}, error);
    CHECK(configured);

    // SDK failures include their numeric error or the originating shader diagnostic.
    if (!configured) {
        std::cerr << error << '\n';
        return;
    }

    const std::array<unsigned, 5> textures{
        frame.world.colour_texture(),
        frame.textures[2],
        frame.textures[0],
        frame.textures[1],
        frame.output.colour_texture()
    };

    // The final iteration changes colour and invalidates history rather than accepting stale radiance.
    for (unsigned iteration = 0; iteration < 5; ++iteration) {
        const std::array<float, 3> colour = iteration == 4 ? std::array<float, 3>{.8f, .1f, .4f}
                                                           : std::array<float, 3>{.25f, .5f, .75f};
        frame.clear(colour);
        enhanced::fsr3_detail::Dispatch dispatch;
        dispatch.reset = iteration == 0 || iteration == 4;
        dispatch.sharpness = 0;
        const bool rendered = upscaler.dispatch(textures, dispatch, error);
        CHECK(rendered);

        // Do not inspect an output image the SDK failed to produce.
        if (!rendered) {
            std::cerr << error << '\n';
            break;
        }

        check_centre_colour(frame.output, colour);
    }

    upscaler.release();
    CHECK(vkgl::submission_counters().validation_errors == 0);
}
}  // namespace

int main() {
    try {
        openrac::test::GlContext context;
        std::string error;

        // Native GPU checks may skip only when their graphics context cannot be opened.
        if (!context.open(error)) {
            std::cerr << error << '\n';
            return openrac::test::kSkip;
        }

        CHECK(openrac::gl::enhanced_rendering_available());
        test_temporal_dispatch_preserves_constant_colour();
        return openrac::test::result();
    } catch (...) {
        return 1;
    }
}

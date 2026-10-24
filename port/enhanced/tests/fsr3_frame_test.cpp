// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Synthetic temporal frame handoff: Quality renders two-thirds size, while a one-pixel HUD stays native.
#include <iostream>
#include <memory>

#include "enhanced/fsr3/fsr3.h"
#include "enhanced/fsr3/temporal_math.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/vkgl.h"
#include "scene/level_renderer.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

namespace {
using namespace openrac;
using namespace openrac::gl;

// Procedural world/HUD buckets keep expected colours independent of game assets.
class TestBucket final : public renderer::BucketRenderer {
public:
    // Chooses an opaque world or final HUD draw for this synthetic frame.
    explicit TestBucket(bool hud, const std::array<int, 2>& world_size)
        : BucketRenderer(
              "temporal fixture", hud ? renderer::Bucket::Hud : renderer::Bucket::Terrain
          ),
          m_hud(hud),
          m_world_size(world_size) {}

    void render(const renderer::FrameInput&, renderer::RenderState& state) override {
        // A single display pixel reveals accidental low-resolution HUD rendering or later upscaling.
        if (m_hud) {
            CHECK(state.target.width == 96 && state.target.height == 72);
            glEnable(GL_SCISSOR_TEST);
            glScissor(48, 36, 1, 1);
            glClearColor(1, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);
            return;
        }

        CHECK(state.target.width == m_world_size[0] && state.target.height == m_world_size[1]);
        glClearColor(.25f, .5f, .75f, 1);
        glClearDepth(.5);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

private:
    bool m_hud;                              // Only the final bucket marks a display pixel.
    const std::array<int, 2>& m_world_size;  // Expected extent owned by the enclosing test.
};

// Preset extents and matrix inversion are checked against arithmetic and an identity product.
void test_temporal_math() {
    CHECK(
        (enhanced::fsr3_detail::render_extent({1920, 1080}, renderer::TemporalQuality::Quality)
         == std::array<int, 2>{1280, 720})
    );
    CHECK(
        (enhanced::fsr3_detail::render_extent({1920, 1080}, renderer::TemporalQuality::Performance)
         == std::array<int, 2>{960, 540})
    );
    renderer::Mat4 matrix = renderer::multiply(
        renderer::perspective_reversed(1, 1.5f, .05f, 2000), renderer::translation({-3, 4, 2})
    );
    renderer::Mat4 inverse;
    CHECK(enhanced::fsr3_detail::invert(matrix, inverse));
    const auto product = renderer::multiply(matrix, inverse);

    // A matrix times its computed inverse must reproduce the identity in every component.
    for (std::size_t n = 0; n < product.size(); ++n) {
        CHECK(std::abs(product[n] - renderer::identity()[n]) < .001f);
    }

    CHECK(!enhanced::fsr3_detail::invert({}, inverse));
}

// Checks one resolved frame against constant world RGB and an exact one-pixel HUD marker.
void check_temporal_frame(
    renderer::Renderer& renderer,
    scene::LevelScene& scene,
    enhanced::Fsr3& upscaler,
    renderer::FrameBuffer& display,
    renderer::TemporalQuality quality
) {
    std::string error;
    renderer::FrameInput input;
    input.camera.projection = renderer::perspective_reversed(1, 1.5f, .05f, 2000);
    renderer::RenderTarget target{display.id(), 96, 72};
    const enhanced::TemporalFrame frame{input, target, display, scene, renderer.textures(), 0};
    const bool prepared = upscaler.begin(frame, quality, 0, error);
    CHECK(prepared);

    // An incomplete setup must not be rendered as if reconstruction succeeded.
    if (!prepared) {
        std::cerr << error << '\n';
        return;
    }

    renderer.render(input, target);
    const auto pixels = display.read_rgba();
    // read_rgba returns top row first; GL pixel (48,36) is row 35.
    const std::size_t at = (std::size_t{35} * 96 + 48) * 4;
    CHECK(pixels[at] == 255 && pixels[at + 1] == 0 && pixels[at + 2] == 0);
    CHECK(std::abs(static_cast<int>(pixels[at + 4]) - 64) <= 2);
    CHECK(std::abs(static_cast<int>(pixels[at + 5]) - 128) <= 2);
    CHECK(std::abs(static_cast<int>(pixels[at + 6]) - 191) <= 2);
}

// Resolving before the final HUD bucket retains world RGB across quality and output-image changes.
void test_world_resolve_and_native_hud() {
    renderer::Renderer renderer;
    scene::LevelScene scene;
    enhanced::Fsr3 upscaler;
    renderer::FrameBuffer display;
    std::string error;
    std::array<int, 2> world_size{64, 48};
    CHECK(renderer.init(error));
    CHECK(renderer.add(std::make_unique<TestBucket>(false, world_size), error));
    CHECK(renderer.add(std::make_unique<TestBucket>(true, world_size), error));
    CHECK(display.create(96, 72, error));

    // Preset switches resize history; replacing the output at the same extent tests recycled texture names.
    for (int n = 0; n < 6; ++n) {
        auto quality = renderer::TemporalQuality::Quality;
        world_size = {64, 48};

        // Performance halves both display dimensions.
        if (n == 2) {
            quality = renderer::TemporalQuality::Performance;
            world_size = {48, 36};
        }

        // Native AA reconstructs at display resolution.
        if (n == 3) {
            quality = renderer::TemporalQuality::NativeAA;
            world_size = {96, 72};
        }

        bool recreated = true;

        // A recreated framebuffer can reuse its previous texture name without retaining storage usage.
        if (n == 5) {
            recreated = display.create(96, 72, error);
        }

        CHECK(recreated);
        check_temporal_frame(renderer, scene, upscaler, display, quality);
    }

    upscaler.release();
    display.release();
    renderer.release();
    CHECK(vkgl::submission_counters().validation_errors == 0);
}
}  // namespace

int main() {
    try {
        test_temporal_math();
        test::GlContext context;
        std::string error;

        // Only unavailable graphics hardware may skip the synthetic GPU part.
        if (!context.open(error)) {
            std::cerr << error << '\n';
            return test::kSkip;
        }

        test_world_resolve_and_native_hud();
        return test::result();
    } catch (...) {
        return 1;
    }
}

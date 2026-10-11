// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The world renderers on a real OpenGL 4.1 context: their shaders compile and
// link, an empty level loads, a frame renders, and GL reports no error.
// Skipped (77) when no context can be made.

#include <cstdio>
#include <memory>
#include <string>

#include "platform/window.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/texture_pool.h"
#include "renderer/world/gl_world.h"
#include "renderer/world/tfrag_renderer.h"
#include "renderer/world/tie_renderer.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

using namespace openrac::renderer;

int main() {
    openrac::test::GlContext context;
    std::string why;
    if (!context.open(why)) {
        std::printf("skipped: %s\n", why.c_str());
        return openrac::test::kSkip;
    }
    std::string missing;
    if (!world::gl::load(openrac::platform::Window::gl_loader(), missing)) {
        std::printf("skipped: world GL functions missing: %s\n", missing.c_str());
        return openrac::test::kSkip;
    }

    FrameBuffer target;
    std::string error;
    CHECK(target.create(64, 64, error));
    TexturePool pool;
    RenderState state{pool, {target.id(), 64, 64}, {}};

    auto shared = std::make_shared<world::WorldContext>();
    CHECK(shared->init(error));
    world::TfragRenderer tfrag(shared);
    world::TieRenderer tie(shared);
    if (!tfrag.init(state, error) || !tie.init(state, error)) {
        std::fprintf(stderr, "init: %s\n", error.c_str());
        CHECK(false);
        return openrac::test::result();
    }
    CHECK(tfrag.load(world::TfragDrawData{}, error));
    CHECK(tie.load(world::TieDrawData{}, error));

    const FrameInput input;
    tfrag.render(input, state);
    tie.render(input, state);
    CHECK(tfrag.modes().empty());
    CHECK(openrac::gl::glGetError() == openrac::gl::GL_NO_ERROR);

    tfrag.unload();
    tie.unload();
    tfrag.release();
    tie.release();
    shared->release();
    return openrac::test::result();
}

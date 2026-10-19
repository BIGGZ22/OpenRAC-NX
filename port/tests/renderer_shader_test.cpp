// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The shaders as OpenGL ES 3.2 reads them (renderer/shader.h, es_source): the translation itself,
// without a context, and then every program whose sources stand alone (the direct renderer's, the
// effect renderer's, the viewer's mesh and sky) built on whatever context the test gets: OpenGL
// 4.1, or ES 3.2 under OPENRAC_GL_API=es (renderer_shader_es). The world's programs are built by
// world_gl_smoke. Then a multisampled frame resolved and read back, as the frontend draws one (with
// GL_EXT_multisampled_render_to_texture on an ES driver that has it, renderer/framebuffer.h).
// Skipped (exit 77) where no context can be made.

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "check.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/shader.h"
#include "renderer_gl_context.h"
#include "renderer_shaders.h"
#include "viewer_shaders.h"

namespace {

using openrac::renderer::es_source;
using openrac::renderer::Shader;

bool contains(const std::string& text, const char* part) {
    return text.find(part) != std::string::npos;
}

void translation() {
    const std::string source =
        "#version 410 core\nnoperspective out vec4 v;\nvoid main() { gl_ClipDistance[0] = 1.0; }\n";

    const std::string with = es_source(source, true, true);
    CHECK(with.rfind("#version 320 es\n", 0) == 0);
    CHECK(contains(with, "#extension GL_NV_shader_noperspective_interpolation : require"));
    CHECK(contains(with, "#extension GL_EXT_clip_cull_distance : require"));
    CHECK(contains(with, "precision highp float;"));
    CHECK(contains(with, "precision highp usamplerBuffer;"));
    CHECK(contains(with, "noperspective out vec4 v;"));
    CHECK(contains(with, "gl_ClipDistance[0]"));
    CHECK(!contains(with, "410"));

    const std::string without = es_source(source, false, false);
    CHECK(!contains(without, "#extension"));
    CHECK(!contains(without, "noperspective"));
    CHECK(contains(without, "out vec4 v;"));
    CHECK(!contains(without, "gl_ClipDistance"));
    CHECK(contains(without, "float openrac_clip_distance[8];"));
    CHECK(contains(without, "openrac_clip_distance[0] = 1.0;"));

    // A source that is not GLSL 4.10 is left alone.
    const std::string other = "#version 300 es\nvoid main() {}\n";
    CHECK(es_source(other, false, false) == other);
}

void build(const char* name, const char* vertex, const char* fragment) {
    Shader shader;
    std::string error;
    const bool ok = shader.build(name, vertex, fragment, error);
    if (!ok) {
        std::fprintf(stderr, "%s\n", error.c_str());
    }
    CHECK(ok);
    shader.release();
}

// A 4x frame cleared to one colour reads back as that colour, through resolved() and read_rgba().
void multisampled() {
    using namespace openrac::gl;
    openrac::renderer::FrameBuffer frame;
    std::string error;
    const bool made = frame.create(16, 8, error, 4);
    if (!made) {
        std::fprintf(stderr, "%s\n", error.c_str());
    }
    CHECK(made);
    if (!made) {
        return;
    }
    std::printf("multisampled frame: %d samples, %s\n", frame.samples(),
                frame.resolves_itself() ? "resolved by the GPU" : "resolved by a copy");
    glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
    glViewport(0, 0, 16, 8);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const std::vector<std::uint8_t> pixels = frame.read_rgba();
    CHECK(pixels.size() == 16u * 8u * 4u);
    CHECK(pixels.size() >= 4 && pixels[0] == 255 && pixels[1] == 0 && pixels[2] == 0 && pixels[3] == 255);
    CHECK(pixels.size() >= 4 && pixels[pixels.size() - 4] == 255 && pixels[pixels.size() - 3] == 0);
    CHECK(glGetError() == GL_NO_ERROR);
    frame.release();
}

}  // namespace

int main() {
    translation();

    openrac::test::GlContext context;
    std::string why;
    if (!context.open(why)) {
        std::printf("SKIP: no OpenGL context: %s\n", why.c_str());
        return openrac::test::failures() == 0 ? openrac::test::kSkip : openrac::test::result();
    }
    namespace renderer_src = openrac::renderer::shaders;
    namespace viewer_src = openrac::viewer::shaders;
    build("direct", renderer_src::direct_vert, renderer_src::direct_frag);
    build("effect", renderer_src::effect_vert, renderer_src::direct_frag);
    build("mesh", viewer_src::mesh_vert, viewer_src::mesh_frag);
    // The static layers' opaque pass (viewer/level_renderer.cpp, m_solid).
    std::string solid(viewer_src::mesh_frag);
    solid.insert(solid.find('\n') + 1, "#define OPENRAC_SOLID 1\n");
    build("mesh (solid)", viewer_src::mesh_vert, solid.c_str());
    build("sky", viewer_src::sky_vert, viewer_src::sky_frag);
    multisampled();
    return openrac::test::result();
}

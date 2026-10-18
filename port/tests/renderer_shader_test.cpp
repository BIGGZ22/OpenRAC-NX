// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The shaders as OpenGL ES 3.2 reads them (renderer/shader.h, es_source): the translation itself,
// without a context, and then every program whose sources stand alone (the direct renderer's, the
// effect renderer's, the viewer's mesh and sky) built on whatever context the test gets: OpenGL
// 4.1, or ES 3.2 under OPENRAC_GL_API=es (renderer_shader_es). The world's programs are built by
// world_gl_smoke. Skipped (exit 77) where no context can be made.

#include <cstdio>
#include <string>

#include "check.h"
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
    build("sky", viewer_src::sky_vert, viewer_src::sky_frag);
    return openrac::test::result();
}

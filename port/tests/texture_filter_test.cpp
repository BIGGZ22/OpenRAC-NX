// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Independent 4x4 CPU reconstruction against the production fragment helper.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "check.h"
#include "enhanced/renderer/texture_upscale.h"
#include "enhanced/viewer/probe_lighting.h"
#include "renderer/framebuffer.h"
#include "renderer/shader.h"
#include "renderer/texture_pool.h"
#include "renderer_gl_context.h"
#include "renderer_shaders.h"
#include "scene/level_renderer.h"
#include "viewer_shaders.h"
using namespace openrac;
using namespace openrac::gl;

namespace {
constexpr const char* names[] = {"normal", "edge-aware"};
constexpr const char* vertex = R"(#version 410 core
out vec2 uv;
void main() {
 vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);
 uv=p; gl_Position=vec4(p*2.0-1.0,0,1);
})";

std::string fragment() {
    return "#version 410 core\n"
           "uniform sampler2D tex_T0; in vec2 uv; out vec4 colour;\n"
           "// OPENRAC_UPSCALED_TEXTURE_COLOURS_SOURCE\n"
           "void main(){colour=sample_upscaled_texture_colours(uv);}";
}

// Scalar CPU reference, including quantisation of the cached RGBA8 texels.
std::vector<unsigned char> edge_reference(
    const std::vector<unsigned char>& pixels, int n, bool repeat_s = true, bool repeat_t = true
) {
    const auto texel = [&](int x, int y, int c) {
        x = repeat_s ? (x % n + n) % n : std::clamp(x, 0, n - 1);
        y = repeat_t ? (y % n + n) % n : std::clamp(y, 0, n - 1);
        return pixels[std::size_t((y * n + x) * 4 + c)] / 255.0;
    };
    const auto lum = [&](int x, int y) {
        return .299 * texel(x, y, 0) + .587 * texel(x, y, 1) + .114 * texel(x, y, 2);
    };
    std::vector<unsigned char> out(std::size_t(n * n * 16));

    for (int y = 0; y < n * 2; ++y) {
        for (int x = 0; x < n * 2; ++x) {
            double px = (x + .5) * .5 - .5, py = (y + .5) * .5 - .5;
            int bx = int(std::floor(px)), by = int(std::floor(py));
            double fx = px - bx, fy = py - by;
            double l00 = lum(bx, by), l10 = lum(bx + 1, by), l01 = lum(bx, by + 1),
                   l11 = lum(bx + 1, by + 1);
            double gx = l10 - l00 + l11 - l01, gy = l01 - l00 + l11 - l10;
            double magnitude = std::hypot(gx, gy);
            double energy = std::abs(l10 - l00) + std::abs(l11 - l01) + std::abs(l01 - l00)
                            + std::abs(l11 - l10);
            double coherence = std::clamp(magnitude / std::max(energy, .0001), 0.0, 1.0);

            for (int c = 0; c < 3; ++c) {
                double c00 = texel(bx, by, c), c10 = texel(bx + 1, by, c),
                       c01 = texel(bx, by + 1, c), c11 = texel(bx + 1, by + 1, c);
                double value = std::lerp(std::lerp(c00, c10, fx), std::lerp(c01, c11, fx), fy);

                if (magnitude >= .0001) {
                    double nx = gx / magnitude, ny = gy / magnitude, sum = 0, total = 0;

                    for (int j = -1; j <= 2; ++j) {
                        for (int i = -1; i <= 2; ++i) {
                            double dx = i - fx, dy = j - fy;
                            double radius = std::hypot(
                                (-dx * ny + dy * nx) / std::lerp(1.0, 1.5, coherence),
                                (dx * nx + dy * ny) / std::lerp(1.0, .75, coherence)
                            );
                            constexpr double pi = 3.141592653589793;
                            double weight = radius >= 2 ? 0
                                            : radius < .0001
                                                ? 1
                                                : std::sin(pi * radius) * std::sin(.5 * pi * radius)
                                                      / (.5 * pi * pi * radius * radius);
                            sum += texel(bx + i, by + j, c) * weight;
                            total += weight;
                        }
                    }

                    if (total > .0001) {
                        value = sum / total;
                    }

                    value = std::clamp(
                        value, std::min({c00, c10, c01, c11}), std::max({c00, c10, c01, c11})
                    );
                }

                out[std::size_t((y * n * 2 + x) * 4 + c)] =
                    static_cast<unsigned char>(std::lround(value * 255));
            }

            out[std::size_t((y * n * 2 + x) * 4 + 3)] = 255;
        }
    }

    return out;
}

double reference(
    const std::vector<unsigned char>& pixels,
    int n,
    double u,
    double v,
    int channel,
    bool repeat_s = true,
    bool repeat_t = true
) {
    const double x = u * n - .5, y = v * n - .5;
    const int ix = int(std::floor(x)), iy = int(std::floor(y));
    const double fx = x - ix, fy = y - iy;
    const auto texel = [&](int a, int b) {
        a = repeat_s ? (a % n + n) % n : std::clamp(a, 0, n - 1);
        b = repeat_t ? (b % n + n) % n : std::clamp(b, 0, n - 1);
        return pixels[std::size_t((b * n + a) * 4 + channel)] / 255.0;
    };
    return (1 - fy) * ((1 - fx) * texel(ix, iy) + fx * texel(ix + 1, iy))
           + fy * ((1 - fx) * texel(ix, iy + 1) + fx * texel(ix + 1, iy + 1));
}
}  // namespace

int main() {
    test::GlContext context;
    std::string error;

    if (!context.open(error, 1280, 720)) {
        std::puts(error.c_str());
        return test::kSkip;
    }

    renderer::Shader shader;

    if (!shader.build("live texture filtering", vertex, fragment(), error)) {
        std::puts(error.c_str());
        return 1;
    }

    CHECK(
        (shader.uniform("upscaled_texture_colours_ready") >= 0) == enhanced_rendering_available()
    );
    renderer::TexturePool pool;
    enhanced::TextureUpscaler* upscaler = nullptr;

    if (enhanced_rendering_available()) {
        auto source = std::make_unique<enhanced::TextureUpscaler>();
        upscaler = source.get();
        pool.set_upscaled_texture_colours(std::move(source));
    }

    CHECK(pool.has_upscaled_texture_colours() == enhanced_rendering_available());
    renderer::TextureHandle source = 0;
    unsigned vao = 0, texture = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    std::vector<unsigned char> pixels(8 * 8 * 4);

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            for (int c = 0; c < 4; ++c) {
                pixels[std::size_t((y * 8 + x) * 4 + c)] = static_cast<unsigned char>(
                    c == 3 ? (((x / 2 + y / 2) % 2) ? 255 : 0) : (x * 37 + y * 53 + c * 71) % 256
                );
            }
        }
    }

    renderer::Rgba8Image source_image(8, 8);
    source_image.pixels = pixels;
    source = pool.add(std::move(source_image), renderer::AlphaScale::Full, "fixture", false, true);
    texture = pool.gl_texture(source);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    const auto edge_pixels = edge_reference(pixels, 8);
    renderer::FrameBuffer frame;
    CHECK(frame.create(64, 64, error));
    bool repeat_s = true, repeat_t = true;
    const auto draw = [&](int m, int w, int h) {
        glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
        glViewport(0, 0, w, h);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader.use();
        pool.bind_upscaled_texture_colours(
            source, shader, static_cast<renderer::TextureFilter>(m), repeat_s, repeat_t
        );
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };

    for (int m = 0; m < 2; ++m) {
        draw(m, 64, 64);
        auto out = frame.read_rgba();
        int maximum = 0;

        for (int y = 0; y < 64; ++y) {
            for (int x = 0; x < 64; ++x) {
                for (int c = 0; c < 4; ++c) {
                    int expected = int(std::lround(
                        255
                        * (m == 1 && enhanced_rendering_available() && c != 3
                               ? reference(edge_pixels, 16, (x + .5) / 64, 1 - (y + .5) / 64, c)
                               : reference(pixels, 8, (x + .5) / 64, 1 - (y + .5) / 64, c))
                    ));
                    maximum = std::max(
                        maximum, std::abs(expected - int(out[std::size_t((y * 64 + x) * 4 + c)]))
                    );
                }
            }
        }

        std::printf("reference %s maximum_error %d/255\n", names[m], maximum);
        CHECK(maximum <= 2);
    }

    // A missing/full cache must fall back to original sampling, with no cubic path.
    draw(0, 64, 64);
    const auto fallback_normal = frame.read_rgba();
    draw(1, 64, 64);
    glUniform1i(shader.uniform("upscaled_texture_colours_ready"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    CHECK(frame.read_rgba() == fallback_normal);
    CHECK((upscaler ? upscaler->cache_builds() : 0) == (enhanced_rendering_available() ? 1u : 0u));
    CHECK(
        (upscaler ? upscaler->cache_bytes() : 0)
        == (enhanced_rendering_available() ? 8u * 8u * 16u : 0u)
    );
    frame.release();
    CHECK(frame.create(4, 4, error));
    std::vector<unsigned char> base;

    for (int m = 0; m < 2; ++m) {
        draw(m, 4, 4);
        auto out = frame.read_rgba();

        if (m == 0) {
            base = out;
        } else {
            CHECK(out == base);
        }
    }

    // Exercise the production material shaders, including their real alpha tests.
    frame.release();
    CHECK(frame.create(64, 64, error));
    const std::string material_vertex = R"(#version 410 core
out vec2 v_uv;
out vec3 v_stq;
out vec4 v_colour;
out vec3 v_normal;
out vec3 v_world_position;
out vec4 v_probe_multiplier;
flat out vec4 v_probe_selector;
flat out int v_lit;
flat out int v_glow;
out vec3 v_script_emission;
out vec3 v_emission_colour;
noperspective out float v_fog;
void main(){
 vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);
 gl_Position=vec4(p*2-1,0,1);v_uv=p;v_stq=vec3(p,1);
 v_colour=vec4(1);v_normal=vec3(0,0,1);v_world_position=vec3(0);
 v_probe_multiplier=vec4(1);v_probe_selector=vec4(0);v_lit=1;v_glow=0;
 v_script_emission=vec3(0);v_emission_colour=vec3(0);v_fog=0;
})";

    struct PathCase {
        const char* name;
        int cutout, translucent, metal, effect;
        int kind = 0;
    };

    const PathCase cases[] =
        {{"character", 0, 0, 0, 0},
         {"ship reflection", 0, 0, 1, 0},
         {"cutout", 1, 0, 0, 0},
         {"cutout translucent", 1, 1, 0, 0},
         {"mesh effect", 0, 0, 0, 1},
         {"sky", 0, 0, 0, 0, 1},
         {"world liquid/particle", 0, 0, 0, 0, 2}};

    for (const auto& path : cases) {
        renderer::Shader material;
        std::string vs = material_vertex;
        std::string fs = path.kind == 1 ? scene::shaders::sky_frag
                                        : lighting::probe_mesh_fragment(scene::shaders::mesh_frag);

        if (path.kind == 2) {
            fs = renderer::shaders::direct_frag;
            fs.insert(fs.find('\n') + 1, "#define OPENRAC_UPSCALED_TEXTURE_COLOURS 1\n");
            vs.replace(vs.find("noperspective "), std::string("noperspective ").size(), "");
            vs.replace(
                vs.find("v_colour=vec4(1)"),
                std::string("v_colour=vec4(1)").size(),
                "v_colour=vec4(128)"
            );
        }

        if (!material.build(path.name, vs, fs, error)) {
            std::puts(error.c_str());
            return 1;
        }

        std::vector<unsigned char> normal;

        for (int mode = 0; mode < 2; ++mode) {
            glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
            glViewport(0, 0, 64, 64);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            material.use();
            pool.bind_upscaled_texture_colours(
                source, material, static_cast<renderer::TextureFilter>(mode)
            );
            glUniform4f(material.uniform("material_colour"), 1, 1, 1, 1);
            glUniform1f(material.uniform("material_diffuse_scale"), 1);
            glUniform1i(material.uniform("cutout"), path.cutout);
            glUniform1i(material.uniform("translucent_pass"), path.translucent);
            glUniform1i(material.uniform("metal_pass"), path.metal);
            glUniform1i(material.uniform("effect_pass"), path.effect);
            glUniform1i(material.uniform("textured"), 1);
            glUniform1i(material.uniform("tcc"), 1);
            glUniform1f(material.uniform("tex_alpha_scale"), 0.5f);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            auto image = frame.read_rgba();

            if (mode == 0) {
                normal = image;
                continue;
            }

            int changed = 0;

            for (std::size_t i = 0; i < image.size(); i += 4) {
                CHECK(image[i + 3] == normal[i + 3]);
                changed += image[i] != normal[i] || image[i + 1] != normal[i + 1]
                           || image[i + 2] != normal[i + 2];
            }

            CHECK(enhanced_rendering_available() ? changed > 0 : changed == 0);
            std::printf(
                "material %s %s RGB_changed %d alpha_exact\n", path.name, names[mode], changed
            );
        }
    }

    // Repeat/clamp variants must reconstruct their own border neighbourhoods.
    for (unsigned wrap = 0; wrap < 3; ++wrap) {
        repeat_s = (wrap & 1) != 0;
        repeat_t = (wrap & 2) != 0;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat_s ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat_t ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        const auto expected_edge = edge_reference(pixels, 8, repeat_s, repeat_t);
        draw(1, 64, 64);
        const auto out = frame.read_rgba();
        int maximum = 0;

        for (int y = 0; y < 64; ++y) {
            for (int x = 0; x < 64; ++x) {
                for (int c = 0; c < 4; ++c) {
                    const double u = (x + .5) / 64, v = 1 - (y + .5) / 64;
                    const double expected =
                        enhanced_rendering_available() && c != 3
                            ? reference(expected_edge, 16, u, v, c, repeat_s, repeat_t)
                            : reference(pixels, 8, u, v, c, repeat_s, repeat_t);
                    maximum = std::max(
                        maximum,
                        std::abs(
                            int(out[std::size_t((y * 64 + x) * 4 + c)])
                            - int(std::lround(expected * 255))
                        )
                    );
                }
            }
        }

        std::printf("border wrap %u maximum_error %d/255\n", wrap, maximum);
        CHECK(maximum <= 2);
    }

    repeat_s = repeat_t = true;
    CHECK((upscaler ? upscaler->cache_builds() : 0) == (enhanced_rendering_available() ? 4u : 0u));

    if (upscaler) {
        // Retiring a source must also drop its cached colours before GL can reuse its name.
        upscaler->forget(texture);
        CHECK(upscaler->cache_bytes() == 0);
        draw(1, 64, 64);
        CHECK(upscaler->cache_builds() == 5);
        const auto persistent_bytes = upscaler->cache_bytes();
        renderer::ImageUpload upload;
        upload.dbp = 64;
        upload.width = upload.height = 8;
        upload.data = pixels;
        pool.upload(std::move(upload));
        renderer::gs::Tex0 tex0{};
        tex0.tbp0 = 64;
        tex0.tbw = 1;
        tex0.tw = tex0.th = 3;
        const auto streamed = pool.resolve(tex0, {});
        shader.use();
        pool.bind_upscaled_texture_colours(streamed, shader, renderer::TextureFilter::EdgeAware);
        CHECK(upscaler->cache_bytes() == persistent_bytes * 2);

        for (unsigned i = 0; i < renderer::TexturePool::kKeepFrames; ++i) {
            pool.end_frame();
        }

        CHECK(upscaler->cache_bytes() == persistent_bytes);
    }

    CHECK(glGetError() == GL_NO_ERROR);
    pool.release_gl();
    CHECK((upscaler ? upscaler->cache_bytes() : 0) == 0);
    glDeleteVertexArrays(1, &vao);
    return test::result();
}

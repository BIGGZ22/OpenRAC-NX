// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Independent 4x4 CPU reconstruction against the production fragment helper.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <exception>
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
#include "scene/screenshot.h"
#include "scene_shaders.h"
#include "tests/texture_filter_reference.h"
using namespace openrac;
using namespace openrac::gl;

namespace {
using test::texture_reference::edge_reference;
using test::texture_reference::reference;
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

const std::string kMaterialVertex = R"(#version 410 core
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

const PathCase kCases[] =
    {{"character", 0, 0, 0, 0},
     {"ship reflection", 0, 0, 1, 0},
     {"cutout", 1, 0, 0, 0},
     {"cutout translucent", 1, 1, 0, 0},
     {"mesh effect", 0, 0, 0, 1},
     {"sky", 0, 0, 0, 0, 1},
     {"world liquid/particle", 0, 0, 0, 0, 2}};

// Produces coloured texels with an independent checkerboard alpha mask.
std::vector<unsigned char> fixture_pixels() {
    std::vector<unsigned char> pixels(std::size_t{8} * 8 * 4);
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const auto offset = (static_cast<std::size_t>(y) * 8 + x) * 4;
            for (int channel = 0; channel < 3; ++channel) {
                pixels[offset + channel] =
                    static_cast<unsigned char>((x * 37 + y * 53 + channel * 71) % 256);
            }
            pixels[offset + 3] = ((x / 2 + y / 2) % 2) ? 255 : 0;
        }
    }
    return pixels;
}

// Produces 4/255 bands above a larger colour discontinuity, with alternating alpha.
std::vector<unsigned char> ramp_pixels(int n) {
    std::vector<unsigned char> ramp(static_cast<std::size_t>(n) * n * 4);
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < n; ++x) {
            const auto i = (static_cast<std::size_t>(y) * n + x) * 4;
            if (y < 16) {
                ramp[i] = static_cast<unsigned char>(96 + (x / 8) * 4);
            } else {
                ramp[i] = x < 16 ? 32 : 224;
            }
            ramp[i + 1] = static_cast<unsigned char>(y < 16 ? ramp[i] + 12 : 64);
            ramp[i + 2] = static_cast<unsigned char>(y < 16 ? ramp[i] + 24 : 128);
            ramp[i + 3] = (x + y) % 2 ? 255 : 0;
        }
    }
    return ramp;
}

// Compares a 64x64 readback with independent RGB reconstruction and original alpha.
int sampled_error(
    const std::vector<unsigned char>& output,
    const test::texture_reference::Image& source,
    const test::texture_reference::Image& edge,
    bool reconstructed
) {
    int maximum = 0;
    // Output rows are read top-down while the source sampling coordinates increase upward.
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            for (int channel = 0; channel < 4; ++channel) {
                // Reconstruction changes RGB only, preserving the source's alpha coverage.
                const auto& image = reconstructed && channel != 3 ? edge : source;
                const int expected = static_cast<int>(
                    std::lround(255 * reference(image, (x + .5) / 64, 1 - (y + .5) / 64, channel))
                );
                const auto offset = (static_cast<std::size_t>(y) * 64 + x) * 4 + channel;
                maximum = std::max(maximum, std::abs(expected - int(output[offset])));
            }
        }
    }
    return maximum;
}

// Owns production filtering resources while the caller's graphics context remains open.
class FilterFixture {
public:
    FilterFixture() = default;

    // Releases the vertex array before the caller closes its context.
    ~FilterFixture() { glDeleteVertexArrays(1, &m_vao); }

    FilterFixture(const FilterFixture&) = delete;
    FilterFixture& operator=(const FilterFixture&) = delete;
    FilterFixture(FilterFixture&&) = delete;
    FilterFixture& operator=(FilterFixture&&) = delete;

    // Returns the setup diagnostic when create() cannot build its resources.
    const std::string& error_message() const { return m_error; }

    // Creates the checkerboard source, production shader and output target.
    bool create() {
        if (!m_shader.build("live texture filtering", vertex, fragment(), m_error)) {
            return false;
        }
        CHECK(
            (m_shader.uniform("upscaled_texture_colours_ready") >= 0)
            == enhanced_rendering_available()
        );
        if (enhanced_rendering_available()) {
            auto provider = std::make_unique<enhanced::TextureUpscaler>();
            m_upscaler = provider.get();
            m_pool.set_upscaled_texture_colours(std::move(provider));
        }
        CHECK(m_pool.has_upscaled_texture_colours() == enhanced_rendering_available());
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        m_pixels = fixture_pixels();
        renderer::Rgba8Image image(8, 8);
        image.pixels = m_pixels;
        m_source = m_pool.add(std::move(image), renderer::AlphaScale::Full, "fixture", false, true);
        m_texture = m_pool.gl_texture(m_source);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        m_edge_pixels = edge_reference({m_pixels, 8});
        return m_frame.create(64, 64, m_error);
    }

    // Checks both modes against independent scalar samples, including original alpha.
    void test_reference_sampling() {
        for (int m = 0; m < 2; ++m) {
            draw(m, 64, 64);
            auto out = m_frame.read_rgba();
            const int maximum = sampled_error(
                out, {m_pixels, 8}, {m_edge_pixels, 16}, m == 1 && enhanced_rendering_available()
            );

            std::printf("reference %s maximum_error %d/255\n", names[m], maximum);
            CHECK(maximum <= 2);
        }
    }

    // Checks the unavailable-cache fallback and that minification keeps original sampling.
    void test_fallback_and_minification() {
        // A missing/full cache must fall back to original sampling, with no cubic path.
        draw(0, 64, 64);
        const auto fallback_normal = m_frame.read_rgba();
        draw(1, 64, 64);
        glUniform1i(m_shader.uniform("upscaled_texture_colours_ready"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        CHECK(m_frame.read_rgba() == fallback_normal);
        CHECK(
            (m_upscaler ? m_upscaler->cache_builds() : 0)
            == (enhanced_rendering_available() ? 1u : 0u)
        );
        CHECK(
            (m_upscaler ? m_upscaler->cache_bytes() : 0)
            == (enhanced_rendering_available() ? 8u * 8u * 16u : 0u)
        );
        m_frame.release();
        CHECK(m_frame.create(4, 4, m_error));
        draw(0, 4, 4);
        const auto base = m_frame.read_rgba();
        draw(1, 4, 4);
        CHECK(m_frame.read_rgba() == base);
    }

    // Checks each production material's colour path while preserving its exact alpha tests.
    void test_materials() {
        m_frame.release();
        CHECK(m_frame.create(64, 64, m_error));
        for (const auto& path : kCases) {
            test_material_path(path);
        }
    }

    // Checks independently reconstructed repeat/clamp border neighbourhoods.
    void test_wrapping() {
        // Repeat/clamp variants must reconstruct their own border neighbourhoods.
        for (unsigned wrap = 0; wrap < 3; ++wrap) {
            m_repeat_s = (wrap & 1) != 0;
            m_repeat_t = (wrap & 2) != 0;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_repeat_s ? GL_REPEAT : GL_CLAMP_TO_EDGE
            );
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_repeat_t ? GL_REPEAT : GL_CLAMP_TO_EDGE
            );
            const auto expected_edge = edge_reference({m_pixels, 8, m_repeat_s, m_repeat_t});
            draw(1, 64, 64);
            const auto out = m_frame.read_rgba();
            const int maximum = sampled_error(
                out,
                {m_pixels, 8, m_repeat_s, m_repeat_t},
                {expected_edge, 16, m_repeat_s, m_repeat_t},
                enhanced_rendering_available()
            );

            std::printf("border wrap %u maximum_error %d/255\n", wrap, maximum);
            CHECK(maximum <= 2);
        }

        m_repeat_s = m_repeat_t = true;
        CHECK(
            (m_upscaler ? m_upscaler->cache_builds() : 0)
            == (enhanced_rendering_available() ? 4u : 0u)
        );
    }

    // Checks source retirement, streamed texture eviction and final cache release.
    void test_cache_retirement() {
        if (m_upscaler) {
            // Retiring a source must also drop its cached colours before GL can reuse its name.
            m_upscaler->forget(m_texture);
            CHECK(m_upscaler->cache_bytes() == 0);
            draw(1, 64, 64);
            CHECK(m_upscaler->cache_builds() == 5);
            const auto persistent_bytes = m_upscaler->cache_bytes();
            renderer::ImageUpload upload;
            upload.dbp = 64;
            upload.width = upload.height = 8;
            upload.data = m_pixels;
            m_pool.upload(std::move(upload));
            renderer::gs::Tex0 tex0{};
            tex0.tbp0 = 64;
            tex0.tbw = 1;
            tex0.tw = tex0.th = 3;
            const auto streamed = m_pool.resolve(tex0, {});
            m_shader.use();
            m_pool.bind_upscaled_texture_colours(
                streamed, m_shader, renderer::TextureFilter::EdgeAware
            );
            CHECK(m_upscaler->cache_bytes() == persistent_bytes * 2);

            for (unsigned i = 0; i < renderer::TexturePool::kKeepFrames; ++i) {
                m_pool.end_frame();
            }

            CHECK(m_upscaler->cache_bytes() == persistent_bytes);
        }

        CHECK(glGetError() == GL_NO_ERROR);
        m_pool.release_gl();
        CHECK((m_upscaler ? m_upscaler->cache_bytes() : 0) == 0);
    }

    // Checks a quantised ramp, sharp colour edge and transparent coverage together.
    void test_debanding(const std::string& output_directory) {
        constexpr int kRampSize = 32;
        const auto ramp = ramp_pixels(kRampSize);
        const auto output = draw_debanding(ramp, kRampSize);
        if (!output_directory.empty()) {
            CHECK(scene::write_png(output_directory + "/gradient.png", 64, 64, output));
        }
        check_debanding(output, ramp, kRampSize);
    }

private:
    // Draws one mode using this fixture's current source, output and wrap rules.
    void draw(int m, int w, int h) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_frame.id());
        glViewport(0, 0, w, h);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        m_shader.use();
        m_pool.bind_upscaled_texture_colours(
            m_source, m_shader, static_cast<renderer::TextureFilter>(m), m_repeat_s, m_repeat_t
        );
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    // Draws one production material variant with its authored coverage controls.
    std::vector<unsigned char> material_sample(
        renderer::Shader& material, const PathCase& path, int mode
    ) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_frame.id());
        glViewport(0, 0, 64, 64);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        material.use();
        m_pool.bind_upscaled_texture_colours(
            m_source, material, static_cast<renderer::TextureFilter>(mode)
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
        return m_frame.read_rgba();
    }

    // Checks one material against normal filtering before advancing to the next variant.
    void test_material_path(const PathCase& path) {
        renderer::Shader material;
        std::string vs = kMaterialVertex;
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

        if (!material.build(path.name, vs, fs, m_error)) {
            std::puts(m_error.c_str());
            CHECK(false);
            return;
        }

        const auto normal = material_sample(material, path, 0);
        const auto image = material_sample(material, path, 1);
        int changed = 0;
        // Alpha remains exact even where the enhanced colour reconstruction changes RGB.
        for (std::size_t i = 0; i < image.size(); i += 4) {
            CHECK(image[i + 3] == normal[i + 3]);
            changed += image[i] != normal[i] || image[i + 1] != normal[i + 1]
                       || image[i + 2] != normal[i + 2];
        }
        CHECK(enhanced_rendering_available() ? changed > 0 : changed == 0);
        std::printf("material %s edge-aware RGB_changed %d alpha_exact\n", path.name, changed);
    }

    // Produces the debanded image while the temporary source and cache remain alive.
    std::vector<unsigned char> draw_debanding(const std::vector<unsigned char>& ramp, int n) {
        renderer::TexturePool candidate;
        candidate.set_upscaled_texture_colours(std::make_unique<enhanced::TextureUpscaler>());
        renderer::Rgba8Image image(n, n);
        image.pixels = ramp;
        const auto handle = candidate.add(
            std::move(image), renderer::AlphaScale::Full, "deband fixture", false, true
        );
        glBindFramebuffer(GL_FRAMEBUFFER, m_frame.id());
        glViewport(0, 0, 64, 64);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, candidate.gl_texture(handle));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        m_shader.use();
        candidate.bind_upscaled_texture_colours(
            handle, m_shader, renderer::TextureFilter::EdgeAware, false, false
        );
        glDrawArrays(GL_TRIANGLES, 0, 3);
        const auto output = m_frame.read_rgba();
        candidate.release_gl();
        return output;
    }

    // Compares RGB, alpha, colour edges and residual band steps against the scalar oracle.
    void check_debanding(
        const std::vector<unsigned char>& output, const std::vector<unsigned char>& ramp, int n
    ) {
        const auto expected = edge_reference({ramp, n, false, false});
        int maximum = 0, alpha_error = 0;
        for (int y = 0; y < 64; ++y) {
            for (int x = 0; x < 64; ++x) {
                for (int c = 0; c < 3; ++c) {
                    maximum = std::max(
                        maximum,
                        std::abs(
                            int(output[(static_cast<std::size_t>(y) * 64 + x) * 4 + c])
                            - int(expected[(static_cast<std::size_t>(63 - y) * 64 + x) * 4 + c])
                        )
                    );
                }
                const int alpha = static_cast<int>(std::lround(
                    255 * reference({ramp, n, false, false}, (x + .5) / 64, 1 - (y + .5) / 64, 3)
                ));
                alpha_error = std::max(
                    alpha_error,
                    std::abs(alpha - int(output[(static_cast<std::size_t>(y) * 64 + x) * 4 + 3]))
                );
            }
        }
        std::printf("deband maximum_error %d/255 alpha_error %d/255\n", maximum, alpha_error);
        CHECK(maximum <= 3);
        CHECK(alpha_error <= 1);
        // Far inside each solid side of the sharp edge, cleanup must preserve colour exactly.
        CHECK(output[(std::size_t{8} * 64 + 8) * 4] == 32);
        CHECK(output[(std::size_t{8} * 64 + 56) * 4] == 224);
        // The selected ramp row crossed 4/255 bands before debanding.
        int maximum_step = 0;
        for (int x = 5; x < 60; ++x) {
            maximum_step = std::max(
                maximum_step,
                std::abs(
                    int(output[(std::size_t{48} * 64 + x) * 4])
                    - int(output[(std::size_t{48} * 64 + x - 1) * 4])
                )
            );
        }
        CHECK(maximum_step <= 2);
    }

    renderer::Shader m_shader;     // Production sampling shader.
    renderer::TexturePool m_pool;  // Owns the source and optional cached-colour provider.
    enhanced::TextureUpscaler* m_upscaler = nullptr;  // Borrowed from pool until its destruction.
    renderer::FrameBuffer m_frame;         // Output for both modes and material variants.
    renderer::TextureHandle m_source = 0;  // Source handle owned by pool.
    unsigned m_vao = 0, m_texture = 0;     // Owns the vertex array; texture is borrowed from pool.
    bool m_repeat_s = true, m_repeat_t = true;           // Current source wrapping rules.
    std::vector<unsigned char> m_pixels, m_edge_pixels;  // Original and scalar cached texels.
    std::string m_error;                                 // Setup and shader diagnostics.
};

}  // namespace

int main(int argc, char** argv) try {
    test::GlContext context;
    std::string error;
    if (!context.open(error, 1280, 720)) {
        std::puts(error.c_str());
        return test::kSkip;
    }
    FilterFixture fixture;
    if (!fixture.create()) {
        std::puts(fixture.error_message().c_str());
        return 1;
    }
    fixture.test_reference_sampling();
    fixture.test_fallback_and_minification();
    fixture.test_materials();
    fixture.test_wrapping();
    fixture.test_cache_retirement();
    if (enhanced_rendering_available()) {
        fixture.test_debanding(argc == 2 ? argv[1] : "");
    }
    return test::result();
} catch (const std::exception& failure) {
    std::fprintf(stderr, "%s\n", failure.what());
    return 1;
} catch (...) {
    return 1;
}

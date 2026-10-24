// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

#include "enhanced/tests/gpu_probe_fixture.h"
#include "enhanced/viewer/early_lighting.h"
#include "enhanced/viewer/probe_lighting.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/world/gl_world.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"
#include "viewer_shaders.h"
using namespace openrac;
using namespace openrac::gl;

namespace {
constexpr auto vertex = R"glsl(#version 410 core
void main() {
    vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)glsl";
constexpr auto depth_fragment = R"glsl(#version 410 core
uniform mat4 projection;
uniform int fixture;
out vec4 colour;
void main() {
    float x = gl_FragCoord.x;
    float distance = 8.0;
    // Parallel surfaces separated by a thin wall, a large step, and a
    // one-pixel foreground strip. These must never share their lighting.
    if (fixture == 1 && x < 18.0) {
        distance = 7.95;
    }
    if (fixture == 2 && x < 18.0) {
        distance = 4.0;
    }
    if (fixture == 3 && int(x) == 18) {
        distance = 4.0;
    }
    if (fixture == 4) {
        discard;
    }
    vec4 clip = projection * vec4(0, 0, -distance, 1);
    gl_FragDepth = clip.z / clip.w * 0.5 + 0.5;
    colour = vec4(0);
}
)glsl";
constexpr auto fragment = R"glsl(#version 410 core
uniform int translucent_pass;
uniform int effect_pass;
uniform int use_cache;
uniform int show_hits;
uniform int show_hero;
uniform sampler2D source_depth;
uniform mat4 view;
bool early_surface(
    vec3 eye_position,
    vec3 normal_view,
    out vec4 result,
    out bool covered,
    out float hero_visibility
);
uniform mat4 early_projection;
uniform vec2 early_extent;
out vec4 colour;
void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(source_depth, pixel, 0).r;
    if (depth <= 0.0) {
        colour = vec4(0);
        return;
    }
    float distance = early_projection[3][2] / (depth * 2.0 + (early_projection[2][2] - 1.0));
    vec2 ndc = gl_FragCoord.xy / early_extent * 2.0 - 1.0;
    vec3 eye =
        vec3(
            (ndc + early_projection[2].xy) / vec2(early_projection[0][0], early_projection[1][1]),
            -1
        )
        * distance;
    mat3 inverse_view = transpose(mat3(view));
    vec3 world = inverse_view * (eye - view[3].xyz), normal = inverse_view * vec3(0, 0, 1);
    vec4 light = vec4(0);
    bool covered = false;
    float hero_visibility = 1.0;
    bool hit = use_cache != 0 && early_surface(eye, vec3(0, 0, 1), light, covered, hero_visibility);
    if (show_hits != 0) {
        colour = vec4(hit ? 1 : 0, covered ? 1 : 0, 0, 1);
        return;
    }
    if (!hit) {
        covered = sample_probe_irradiance(
            world, normal, probe_surface_bias(normal, inverse_view * (-eye)), light.rgb
        );
        light.a = sun_shadow_visibility(world, normal, normal);
        hero_visibility = hero_shadow_visibility(world, normal);
    }
    if (hit && !hero_shadow_applies(normal)) {
        hero_visibility = 1.0;
    }
    colour =
        show_hero != 0 ? vec4(hero_visibility) : vec4(covered ? light.rgb / 4.0 : vec3(0), light.a);
}
)glsl";

void check() {
    std::string error;
    renderer::Shader fill, sample;
    CHECK(fill.build("early lighting fixture depth", vertex, depth_fragment, error));
    std::string source = scene::probe_shader_source(fragment);
    // Common uniforms are already declared above for the reference path.
    std::string reconstruction = scene::shaders::early_lighting_glsl;
    for (const char* line : {"uniform mat4 early_projection;\n", "uniform vec2 early_extent;\n"}) {
        reconstruction.erase(reconstruction.find(line), std::char_traits<char>::length(line));
    }
    source += reconstruction;
    CHECK(sample.build("early lighting fixture sample", vertex, source, error));
    if (!error.empty()) {
        std::fprintf(stderr, "%s\n", error.c_str());
    }
    scene::ProbeLighting probes;
    lighting::ProbeGrid grid;
    grid.origin = {-16, -16, -.025f};
    grid.spacing = {32, 32, .05f};
    grid.dimensions = {3, 3, 4};
    auto volume = lighting::ProbeVolume::allocate(grid);
    for (auto& p : volume.probes) {
        p.valid = true;
    }
    for (std::size_t i = 0; i < volume.irradiance.size(); ++i) {
        const auto z = i / grid.texels_per_probe() / 9;
        volume.irradiance[i] = z < 2 ? lighting::Float3{.25f, 1, 3} : lighting::Float3{3, 1, .25f};
    }
    for (auto& m : volume.distance_moments) {
        m = {1024, 1024 * 1024};
    }
    CHECK(probes.upload(openrac::test::gpu_probes(volume), error));
    probes.normal_bias = probes.view_bias = 0;
    renderer::FrameBuffer depth, output, shadow;
    CHECK(shadow.create(4, 4, error, 1, true));
    unsigned ao_texture = 0, ao_buffer = 0;
    glGenTextures(1, &ao_texture);
    glActiveTexture(GL_TEXTURE0 + 18);
    glBindTexture(renderer::world::gl::GL_TEXTURE_BUFFER, ao_texture);
    glGenBuffers(1, &ao_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ao_buffer);
    const std::vector<unsigned> ao_samples(19 * 12, 0x80800080u);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        ao_samples.size() * sizeof(unsigned),
        ao_samples.data(),
        GL_DYNAMIC_DRAW
    );
    glTexBuffer(renderer::world::gl::GL_TEXTURE_BUFFER, GL_RGBA8, ao_buffer);
    glActiveTexture(GL_TEXTURE0);
    float ao_strength = 0;
    int hero_mode = 0;
    scene::EarlyLighting early;
    renderer::TexturePool textures;
    unsigned vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    renderer::FrameInput input;
    input.camera.position = {0, 0, 8};
    input.camera.view = renderer::look_along(input.camera.position, {0, 0, -1}, {0, 1, 0});
    const auto bind = [&](const renderer::Shader& shader, bool shadows) {
        probes.bind(shader, scene::Layer::Terrain, true, &input.camera);
        glUniform1i(shader.uniform("early_ao_enabled"), ao_strength > 0 ? 1 : 0);
        glUniform1i(shader.uniform("early_ao"), 18);
        glUniform1i(shader.uniform("early_ao_scale"), 2);
        glUniform1f(shader.uniform("early_ao_strength"), ao_strength);
        glActiveTexture(GL_TEXTURE0 + 18);
        glBindTexture(renderer::world::gl::GL_TEXTURE_BUFFER, ao_texture);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(shader.uniform("hero_shadow_enabled"), hero_mode != 0 ? 1 : 0);
        glUniform1i(shader.uniform("hero_shadow_airborne"), hero_mode == 2 ? 1 : 0);
        glUniform1i(shader.uniform("hero_shadow_receiver"), 0);
        glUniform1i(shader.uniform("hero_shadow_map"), 6);
        const auto hero_matrix = renderer::scale(.01f);
        glUniformMatrix4fv(shader.uniform("hero_shadow_matrix"), 1, GL_FALSE, hero_matrix.data());
        glActiveTexture(GL_TEXTURE0 + 6);
        glBindTexture(GL_TEXTURE_2D, shadow.depth_texture());
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(shader.uniform("sun_shadow_enabled"), shadows ? 1 : 0);
        glUniformMatrix4fv(
            shader.uniform("sun_shadow_view"), 1, GL_FALSE, input.camera.view.data()
        );
        glUniform4f(shader.uniform("sun_shadow_ends"), 16, 32, 64, 128);
        glUniform1f(shader.uniform("sun_shadow_near"), .1f);
        auto matrix = renderer::scale(.01f);
        glUniformMatrix4fv(shader.uniform("sun_shadow_matrix[0]"), 1, GL_FALSE, matrix.data());
        glUniform1i(shader.uniform("sun_shadow_map0"), 7);
        glActiveTexture(GL_TEXTURE0 + 7);
        glBindTexture(GL_TEXTURE_2D, shadow.depth_texture());
        glActiveTexture(GL_TEXTURE0);
    };
    for (const auto size : {std::array{37, 23}, std::array{19, 17}}) {
        const auto [width, height] = size;
        input.camera.projection = renderer::perspective_reversed(
            std::numbers::pi_v<float> / 2, float(width) / height, .1f, 128
        );
        CHECK(depth.create(width, height, error, 1, true));
        CHECK(output.create(width, height, error));
        renderer::RenderState state{textures, {output.id(), width, height}, {}};
        for (int fixture = 0; fixture < 5; ++fixture) {
            for (bool shadows : {false, true}) {
                ++input.frame;
                glBindFramebuffer(GL_FRAMEBUFFER, shadow.id());
                glDepthMask(GL_TRUE);
                glClearDepth(.75);
                glClear(GL_DEPTH_BUFFER_BIT);
                glBindFramebuffer(GL_FRAMEBUFFER, depth.id());
                glViewport(0, 0, width, height);
                glDepthMask(GL_TRUE);
                glClearDepth(0);
                glClear(GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_ALWAYS);
                glDisable(GL_BLEND);
                glDisable(GL_CULL_FACE);
                fill.use();
                glUniformMatrix4fv(
                    fill.uniform("projection"), 1, GL_FALSE, input.camera.projection.data()
                );
                glUniform1i(fill.uniform("fixture"), fixture);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                CHECK(early.render(
                    depth.depth_texture(),
                    input,
                    state.target,
                    [&](const auto& shader) { bind(shader, shadows); },
                    error
                ));
                state.early_lighting = true;
                const auto draw = [&](bool cache,
                                      bool hits,
                                      int translucent = 0,
                                      bool hero_view = false,
                                      bool self = false) {
                    glBindFramebuffer(GL_FRAMEBUFFER, output.id());
                    glViewport(0, 0, width, height);
                    glDisable(GL_DEPTH_TEST);
                    glDepthMask(GL_FALSE);
                    sample.use();
                    bind(sample, shadows);
                    early.bind(sample, input, state);
                    glUniform1i(sample.uniform("use_cache"), cache ? 1 : 0);
                    glUniform1i(sample.uniform("show_hits"), hits ? 1 : 0);
                    glUniform1i(sample.uniform("show_hero"), hero_view ? 1 : 0);
                    glUniform1i(sample.uniform("hero_shadow_receiver"), self ? 1 : 0);
                    glUniform1i(sample.uniform("translucent_pass"), translucent);
                    glUniform1i(sample.uniform("effect_pass"), 0);
                    glUniformMatrix4fv(
                        sample.uniform("view"), 1, GL_FALSE, input.camera.view.data()
                    );
                    glUniformMatrix4fv(
                        sample.uniform("early_projection"),
                        1,
                        GL_FALSE,
                        input.camera.projection.data()
                    );
                    glUniform2f(sample.uniform("early_extent"), float(width), float(height));
                    glUniform1i(sample.uniform("source_depth"), 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, depth.depth_texture());
                    glBindVertexArray(vao);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                    return output.read_rgba();
                };
                const auto reference = draw(false, false), actual = draw(true, false);
                unsigned differences = 0;
                for (std::size_t i = 0; i < actual.size(); ++i) {
                    if (std::abs(int(actual[i]) - int(reference[i])) > (i % 4 == 3 ? 1 : 6)) {
                        if (differences < 4) {
                            std::fprintf(
                                stderr,
                                "pixel %zu,%zu ch%zu: %u vs %u\n",
                                i / 4 % width,
                                i / 4 / width,
                                i % 4,
                                unsigned(actual[i]),
                                unsigned(reference[i])
                            );
                        }
                        ++differences;
                    }
                }
                if (differences) {
                    std::fprintf(
                        stderr,
                        "fixture %d, shadows %d, %dx%d: %u mismatched channels\n",
                        fixture,
                        int(shadows),
                        width,
                        height,
                        differences
                    );
                }
                CHECK(differences == 0);
                if (fixture == 0) {
                    // AO attenuates only RGB, before encoding. Alpha keeps the
                    // same cascade visibility, including fully shadowed samples.
                    for (float strength : {1.0f, 2.0f}) {
                        ao_strength = strength;
                        CHECK(early.render(
                            depth.depth_texture(),
                            input,
                            state.target,
                            [&](const auto& shader) { bind(shader, shadows); },
                            error
                        ));
                        const auto occluded = draw(true, false);
                        for (std::size_t i = 0; i < actual.size(); ++i) {
                            const float expected =
                                i % 4 == 3 ? float(actual[i]) : actual[i] * (1 - .5f * strength);
                            CHECK(std::abs(float(occluded[i]) - expected) <= 4);
                        }
                    }
                    ao_strength = 0;
                    CHECK(early.render(
                        depth.depth_texture(),
                        input,
                        state.target,
                        [&](const auto& shader) { bind(shader, shadows); },
                        error
                    ));
                }
                if (fixture == 0) {
                    // Hero visibility stays separate from cascade visibility. Receiver
                    // exclusion is applied after reconstruction, not baked into the cache.
                    for (int hero = 1; hero <= 2; ++hero) {
                        hero_mode = hero;
                        CHECK(early.render(
                            depth.depth_texture(),
                            input,
                            state.target,
                            [&](const auto& shader) { bind(shader, shadows); },
                            error
                        ));
                        for (bool self : {false, true}) {
                            const auto expected = draw(false, false, 0, true, self);
                            const auto cached = draw(true, false, 0, true, self);
                            for (std::size_t i = 0; i < cached.size(); ++i) {
                                CHECK(std::abs(int(cached[i]) - int(expected[i])) <= 1);
                            }
                            const auto centre = (height / 2 * width + width / 2) * 4;
                            CHECK(cached[centre] == (hero == 2 && self ? 255 : 0));
                        }
                    }
                    hero_mode = 0;
                    CHECK(early.render(
                        depth.depth_texture(),
                        input,
                        state.target,
                        [&](const auto& shader) { bind(shader, shadows); },
                        error
                    ));
                }
                auto hits = draw(true, true);
                unsigned accepted = 0;
                for (std::size_t i = 0; i < hits.size(); i += 4) {
                    accepted += hits[i] > 0;
                }
                if (fixture == 0) {
                    CHECK(accepted > unsigned(width * height) * 8 / 10);
                }
                if (fixture == 4) {
                    CHECK(accepted == 0);
                }
                hits = draw(true, true, 1);
                for (std::size_t i = 0; i < hits.size(); i += 4) {
                    CHECK(hits[i] == 0);
                }
                // Same-size secondary cameras and stale frames cannot reuse it.
                input.camera.view[12] += .25f;
                hits = draw(true, true);
                for (std::size_t i = 0; i < hits.size(); i += 4) {
                    CHECK(hits[i] == 0);
                }
                input.camera.view[12] -= .25f;
                state.target.framebuffer = depth.id();
                hits = draw(true, true);
                for (std::size_t i = 0; i < hits.size(); i += 4) {
                    CHECK(hits[i] == 0);
                }
                state.target.framebuffer = output.id();
                ++input.frame;
                hits = draw(true, true);
                for (std::size_t i = 0; i < hits.size(); i += 4) {
                    CHECK(hits[i] == 0);
                }
            }
        }
    }
    early.release();
    probes.release();
    glDeleteTextures(1, &ao_texture);
    glDeleteBuffers(1, &ao_buffer);
    glDeleteVertexArrays(1, &vao);
}
}  // namespace

int main() {
    test::GlContext context;
    std::string why;
    if (!context.open(why)) {
        std::puts(why.c_str());
        return test::kSkip;
    }
    if (!scene::EarlyLighting::supported()) {
        return test::kSkip;
    }
    check();
    return test::result();
}

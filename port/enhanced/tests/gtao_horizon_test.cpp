// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

#include "gtao_horizon_reference.h"
#include "gtao_reference_shaders.h"
#include "renderer/framebuffer.h"
#include "renderer/shader.h"
#include "renderer/world/gl_world.h"
#include "renderer_shaders.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

namespace {
using namespace openrac;
using namespace gl;
constexpr const char* names[] = {"compute", "rejection", "open", "both", "production"};

struct Difference {
    std::size_t channels = 0, changed = 0, guide_changes = 0;
    unsigned maximum = 0, guide_maximum = 0;
    double absolute = 0;

    void compare(
        const std::vector<std::uint8_t>& actual,
        const std::vector<std::uint8_t>& expected,
        bool compare_guides = true
    ) {
        CHECK(actual.size() == expected.size());
        for (std::size_t i = 0; i < actual.size(); ++i) {
            unsigned d = unsigned(std::abs(int(actual[i]) - int(expected[i])));
            if (i % 4 == 0) {
                ++channels;
                changed += d != 0;
                maximum = std::max(maximum, d);
                absolute += d;
            } else {
                if (compare_guides) {
                    guide_changes += d != 0;
                    guide_maximum = std::max(guide_maximum, d);
                }
            }
        }
    }

    void print(const char* kind, const char* variant) const {
        std::printf(
            "%s %s: changed %zu/%zu AO pixels, max %u/255, mean %.9f/255, guide changes %zu, guide "
            "max %u/255\n",
            kind,
            variant,
            changed,
            channels,
            maximum,
            absolute / double(std::max(std::size_t(1), channels)),
            guide_changes,
            guide_maximum
        );
    }
};

std::string expanded(const char* text) {
    std::string s(text);
    s.insert(s.find('\n') + 1, renderer::shaders::gtao_common_glsl);
    return s;
}

std::string compute_source(int variant) {
    auto s = expanded(test::gtao_reference::shaders::gtao_horizons_candidates_comp);
    auto disable = [&](const char* key) {
        auto i = s.find(key);
        CHECK(i != std::string::npos);
        if (i != std::string::npos) {
            s.replace(
                i,
                std::char_traits<char>::length(key),
                std::string(key).replace(std::string(key).find("true"), 4, "false")
            );
        }
    };
    if (!(variant & 1)) {
        disable("const bool gtao_early_rejection = true;");
    }
    if (!(variant & 2)) {
        disable("const bool gtao_open_horizons = true;");
    }
    return s;
}

void cpu_predicates() {
    using namespace test::gtao_cpu;
    std::size_t tested = 0, mismatches = 0;
    std::vector<float> pixels(97 * 73 * 4, 0);
    std::uint32_t seed = 0x817331;
    auto random = [&] {
        seed = seed * 1664525u + 1013904223u;
        return float(seed >> 8) / 16777216.f;
    };
    for (float far : {100.f, 10000.f}) {
        for (float radius : {.05f, 1.25f, 8.f}) {
            auto p = renderer::perspective_reversed(1.f, 97.f / 73.f, .1f, far);
            p[8] = .13f;
            p[9] = -.07f;
            for (unsigned i = 0; i < 80000; ++i) {
                int x = int(random() * 97), y = int(random() * 73), tx = int(random() * 97),
                    ty = int(random() * 73);
                float z = .11f + random() * far * .8f, d = (p[14] / z - p[10] + 1.f) * .5f;
                pixels[(std::size_t(y) * 97 + x) * 4] = d;
                Scene scene{97, 73, p, pixels};
                auto c = scene.position(x, y, d);
                V n = unit(V{random() * 2 - 1, random() * 2 - 1, random() * 2 - 1});
                float td = std::max(1e-8f, random());
                // Half the probes target the original plane threshold, including
                // its rounding boundary, rather than only easy distant samples.
                if ((i & 1u) == 0) {
                    auto ray = scene.position(tx, ty, 1.f);
                    float rayDistance = -ray[2];
                    float directionPlane = dot(n, ray) / rayDistance;
                    float sampleDistance =
                        (dot(n, c) + radius * .002f + (random() - .5f) * .00001f) / directionPlane;
                    if (sampleDistance > .1f && std::isfinite(sampleDistance)) {
                        float candidateDepth = (p[14] / sampleDistance - p[10] + 1.f) * .5f;
                        if (candidateDepth > 0 && candidateDepth <= 1) {
                            td = candidateDepth;
                        }
                    }
                }
                bool original = dot(n, sub(scene.position(tx, ty, td), c)) > radius * .002f;
                bool optimized = scene.occludes(tx, ty, td, x, y, c, n, radius);
                mismatches += original != optimized;
                ++tested;
            }
        }
    }
    std::printf(
        "CPU receiver-plane predicates: %zu comparisons, %zu mismatches\n", tested, mismatches
    );
    CHECK(mismatches == 0);
    double maximum = 0;
    for (int i = -10000; i <= 10000; ++i) {
        const double pi = std::acos(-1.0), a = double(i) * pi / 20000.0, nx = std::cos(a),
                     ny = std::sin(a);
        auto primitive = [&](double t) {
            return .5 * (nx * std::sin(t) * std::sin(t) + ny * (t - std::sin(t) * std::cos(t)));
        };
        double integral = primitive(a - pi * .5) + primitive(a + pi * .5) - 2 * primitive(0);
        maximum = std::max(maximum, std::abs(integral - (nx + a * ny)));
    }
    std::printf("CPU full-hemisphere identity: 20001 angles, maximum error %.12g\n", maximum);
    CHECK(maximum < 1e-12);
}
}  // namespace

int main(int argc, char** argv) {
    cpu_predicates();
    if (test::failures() || (argc > 1 && std::string_view(argv[1]) == "--cpu")) {
        return test::result();
    }
    test::GlContext context;
    std::string error;
    if (!context.open(error)) {
        std::printf("SKIP: %s\n", error.c_str());
        return test::kSkip;
    }
    if (!glDispatchCompute || !glMemoryBarrier) {
        return test::kSkip;
    }
    renderer::Shader original, copy, performance, performance_reference;
    std::array<renderer::Shader, 5> variants;
    CHECK(original.build(
        "original fragment GTAO",
        renderer::shaders::gtao_vert,
        expanded(test::gtao_reference::shaders::gtao_horizons_reference_frag),
        error
    ));
    constexpr const char* copy_source = R"glsl(#version 410 core
uniform samplerBuffer tex_T1;
uniform int ao_width;
layout(location = 0) out vec4 colour;
void main() {
    ivec2 p = ivec2(gl_FragCoord.xy);
    colour = texelFetch(tex_T1, p.y * ao_width + p.x);
}
)glsl";
    CHECK(
        copy.build("GTAO packed buffer readback", renderer::shaders::gtao_vert, copy_source, error)
    );
    for (int v = 0; v < 5; ++v) {
        CHECK(variants[v].build_compute(
            names[v],
            v == 4 ? expanded(renderer::shaders::gtao_evaluate_comp) : compute_source(v),
            error
        ));
    }
    auto performance_source = expanded(renderer::shaders::gtao_evaluate_comp);
    performance_source.insert(performance_source.find('\n') + 1,
                              "#define GTAO_REFINEMENT_STEPS 3\n");
    auto reference_source = compute_source(2);
    auto refinement_loop = reference_source.find("refine < 7");
    CHECK(refinement_loop != std::string::npos);
    if (refinement_loop != std::string::npos) {
        reference_source.replace(refinement_loop, std::string("refine < 7").size(), "refine < 3");
    }
    CHECK(performance.build_compute("performance GTAO", performance_source, error));
    CHECK(performance_reference.build_compute("three-step reference", reference_source, error));
    if (!error.empty()) {
        std::fprintf(stderr, "%s\n", error.c_str());
    }
    if (test::failures()) {
        return test::result();
    }
    unsigned vao = 0, depth = 0, buffer = 0, texture = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenTextures(1, &depth);
    glGenBuffers(1, &buffer);
    glGenTextures(1, &texture);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    renderer::FrameBuffer target;
    std::array<Difference, 5> fragment_differences, compute_differences, cpu_differences,
        cpu_variant_differences;
    Difference cpu_fragment, performance_reference_difference, performance_quality_difference;
    unsigned cases = 0;
    for (const auto [width, height] : {std::pair{65, 49}, std::pair{131, 99}, std::pair{192, 96}}) {
        for (float far : {100.f, 10000.f}) {
            auto projection =
                renderer::perspective_reversed(1.f, float(width) / float(height), .1f, far);
            projection[8] = .13f;
            projection[9] = -.07f;
            for (int pattern = 0; pattern < 6; ++pattern) {
                std::vector<float> pixels(std::size_t(width) * height * 4, 0);
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        float d = .03f;
                        if (pattern == 1 && x > width / 3 && x < width * 2 / 3 && y > height / 3
                            && y < height * 2 / 3) {
                            d = .045f;
                        }
                        if (pattern == 2) {
                            d += .015f * float(x) / float(width) + .01f * float(y) / float(height);
                        }
                        if (pattern == 3) {
                            d = x > width / 2 ? .12f : .012f;
                            if (y < height / 4) {
                                d = 0;
                            }
                        }
                        if (pattern == 4 && (x % 13 == 0 || y % 17 == 0)) {
                            d = .08f;
                        }
                        if (pattern == 5) {
                            d = .015f + float((x * 1973u + y * 9277u) % 997u) * .00015f;
                            if ((x + y * 3) % 29 == 0) {
                                d = 0;
                            }
                        }
                        pixels[(std::size_t(y) * width + x) * 4] = d;
                    }
                }
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, depth);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    renderer::world::gl::GL_RGBA32F,
                    width,
                    height,
                    0,
                    GL_RGBA,
                    GL_FLOAT,
                    pixels.data()
                );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                for (int scale : {1, 2}) {
                    int w = (width + scale - 1) / scale, h = (height + scale - 1) / scale;
                    CHECK(target.create(w, h, error));
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
                    glBufferData(
                        GL_SHADER_STORAGE_BUFFER, std::size_t(w) * h * 4, nullptr, GL_DYNAMIC_DRAW
                    );
                    glActiveTexture(GL_TEXTURE0 + 1);
                    glBindTexture(renderer::world::gl::GL_TEXTURE_BUFFER, texture);
                    glTexBuffer(renderer::world::gl::GL_TEXTURE_BUFFER, GL_RGBA8, buffer);
                    for (int quality = 0; quality < 3; ++quality) {
                        for (float radius : {.05f, 1.25f, 8.f}) {
                            auto uniforms = [&](renderer::Shader& shader) {
                                shader.use();
                                glUniformMatrix4fv(
                                    shader.uniform("projection"), 1, GL_FALSE, projection.data()
                                );
                                glUniform1f(shader.uniform("radius"), radius);
                                glUniform1i(shader.uniform("evaluation_scale"), scale);
                                glUniform1i(
                                    shader.uniform("slice_count"),
                                    quality == 0   ? 1
                                    : quality == 1 ? 4
                                                   : 12
                                );
                                glUniform1i(
                                    shader.uniform("step_count"),
                                    quality == 0   ? 4
                                    : quality == 1 ? 8
                                                   : 24
                                );
                                glActiveTexture(GL_TEXTURE0);
                                glBindTexture(GL_TEXTURE_2D, depth);
                            };
                            glBindFramebuffer(GL_FRAMEBUFFER, target.id());
                            glViewport(0, 0, w, h);
                            uniforms(original);
                            glDrawArrays(GL_TRIANGLES, 0, 3);
                            const auto fragment = target.read_rgba();
                            std::vector<std::uint8_t> compute, profiled_open;
                            std::array<std::vector<std::uint8_t>, 5> cpu;
                            bool cpu_case =
                                width == 65 && far == 100.f && quality == 1 && radius == 1.25f;
                            auto render_compute = [&](renderer::Shader& shader) {
                                uniforms(shader);
                                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, buffer);
                                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
                                glDispatchCompute(unsigned((w + 7) / 8), unsigned((h + 7) / 8), 1);
                                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
                                glBindFramebuffer(GL_FRAMEBUFFER, target.id());
                                glViewport(0, 0, w, h);
                                copy.use();
                                glUniform1i(copy.uniform("ao_width"), w);
                                glActiveTexture(GL_TEXTURE0 + 1);
                                glBindTexture(renderer::world::gl::GL_TEXTURE_BUFFER, texture);
                                glDrawArrays(GL_TRIANGLES, 0, 3);
                                return target.read_rgba();
                            };
                            if (cpu_case) {
                                test::gtao_cpu::Scene scene{width, height, projection, pixels};
                                for (int v = 0; v < 5; ++v) {
                                    cpu[v] = fragment;
                                    for (int y = 0; y < h; ++y) {
                                        for (int x = 0; x < w; ++x) {
                                            float value = scene.evaluate(
                                                x,
                                                y,
                                                scale,
                                                radius,
                                                (v < 4 && (v & 1) != 0),
                                                (v == 4 || (v & 2) != 0)
                                            );
                                            cpu[v][(std::size_t(h - 1 - y) * w + x) * 4] =
                                                std::uint8_t(std::round(value * 255));
                                        }
                                    }
                                }
                                cpu_fragment.compare(cpu[0], fragment, false);
                            }
                            for (int v = 0; v < 5; ++v) {
                                auto actual = render_compute(variants[v]);
                                if (v == 0) {
                                    compute = actual;
                                }
                                if (v == 2) {
                                    profiled_open = actual;
                                }
                                if (v == 4) {
                                    CHECK(actual == profiled_open);
                                }
                                fragment_differences[v].compare(actual, fragment);
                                compute_differences[v].compare(actual, compute);
                                if (cpu_case) {
                                    cpu_differences[v].compare(cpu[v], actual, false);
                                    cpu_variant_differences[v].compare(cpu[v], cpu[0], false);
                                }
                            }
                            const auto performance_expected = render_compute(performance_reference);
                            const auto performance_actual = render_compute(performance);
                            CHECK(performance_actual == performance_expected);
                            performance_reference_difference.compare(performance_actual, performance_expected);
                            performance_quality_difference.compare(performance_actual, profiled_open);
                            ++cases;
                        }
                    }
                }
            }
        }
    }
    std::printf(
        "GPU horizon fixture cases: %u (open ground, contacts, slopes, silhouettes, ridges, "
        "stress)\n",
        cases
    );
    performance_reference_difference.print("GPU/three-step reference", "performance");
    performance_quality_difference.print("GPU/seven-step quality", "performance");
    CHECK(performance_reference_difference.changed == 0);
    CHECK(performance_reference_difference.guide_changes == 0);
    CHECK(performance_quality_difference.changed > 0);
    CHECK(performance_quality_difference.guide_changes == 0);
    cpu_fragment.print("CPU-AO/fragment", "reference");
    for (int v = 0; v < 5; ++v) {
        fragment_differences[v].print("GPU/fragment", names[v]);
        compute_differences[v].print("GPU/compute", names[v]);
        cpu_differences[v].print("CPU-AO/GPU", names[v]);
        cpu_variant_differences[v].print("CPU-AO/CPU", names[v]);
        // Analytic integration and UNORM quantization can round by one code.
        // Contacts and silhouettes cannot move, and metadata must stay exact.
        CHECK(fragment_differences[v].maximum <= 1);
        CHECK(fragment_differences[v].guide_maximum <= 1);
        CHECK(compute_differences[v].maximum <= 1);
        CHECK(compute_differences[v].guide_changes == 0);
        CHECK(cpu_variant_differences[v].maximum <= 1);
        CHECK(cpu_differences[v].maximum <= 1);
    }
    target.release();
    performance.release();
    performance_reference.release();
    original.release();
    copy.release();
    for (auto& shader : variants) {
        shader.release();
    }
    glDeleteTextures(1, &depth);
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &buffer);
    glDeleteVertexArrays(1, &vao);
    return test::result();
}

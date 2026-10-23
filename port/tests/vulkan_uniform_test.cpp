// SPDX-License-Identifier: GPL-3.0-or-later
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "renderer/framebuffer.h"
#include "renderer/shader.h"
#include "renderer/vulkan/vkgl.h"
#include "renderer/world/gl_world.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

using namespace openrac;
using namespace openrac::gl;
namespace wgl = openrac::renderer::world::gl;

void texel_buffer_fallbacks() {
    renderer::Shader shader;
    std::string error;
    CHECK(shader.build(
        "typed incomplete texel buffers",
        R"(#version 410 core
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)",
        R"(#version 410 core
uniform samplerBuffer floats;
uniform isamplerBuffer integers;
uniform usamplerBuffer unsigned_integers;
layout(location = 0) out vec4 result;
void main() {
    result = vec4(texelFetch(floats, 0).xyz + vec3(texelFetch(integers, 0).xyz)
                  + vec3(texelFetch(unsigned_integers, 0).xyz), 1);
}
)",
        error
    ));
    renderer::FrameBuffer target;
    CHECK(target.create(1, 1, error));
    unsigned vao = 0, buffer = 0;
    std::array<unsigned, 3> textures{};
    glGenVertexArrays(1, &vao);
    glGenTextures(3, textures.data());
    glGenBuffers(1, &buffer);
    const char* names[] = {"floats", "integers", "unsigned_integers"};
    const GLenum formats[] = {wgl::GL_RGBA32F, 0x8D82 /* GL_RGBA32I */, wgl::GL_RGBA32UI};
    glBindFramebuffer(GL_FRAMEBUFFER, target.id());
    glBindVertexArray(vao);
    glViewport(0, 0, 1, 1);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    shader.use();
    for (unsigned mode = 0; mode < 4; ++mode) {
        for (unsigned unit = 0; unit < textures.size(); ++unit) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glUniform1i(shader.uniform(names[unit]), static_cast<int>(unit));
            // Unbound, texture with no storage, empty buffer, undersized buffer.
            glBindTexture(wgl::GL_TEXTURE_BUFFER, mode == 0 ? 0 : textures[unit]);
            if (mode >= 2) {
                glBindBuffer(wgl::GL_TEXTURE_BUFFER, buffer);
                const unsigned zero = 0;
                glBufferData(
                    wgl::GL_TEXTURE_BUFFER, mode == 2 ? 0 : 1, &zero, GL_STATIC_DRAW
                );
                glTexBuffer(wgl::GL_TEXTURE_BUFFER, formats[unit], buffer);
            }
        }
        glDrawArrays(GL_TRIANGLES, 0, 3);
        const auto pixels = target.read_rgba();
        CHECK((pixels == std::vector<std::uint8_t>{0, 0, 0, 255}));
    }
    for (unsigned unit = 0; unit < textures.size(); ++unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(wgl::GL_TEXTURE_BUFFER, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindBuffer(wgl::GL_TEXTURE_BUFFER, 0);
    glDeleteBuffers(1, &buffer);
    glDeleteTextures(3, textures.data());
    glDeleteVertexArrays(1, &vao);
    CHECK(glGetError() == GL_NO_ERROR);
}

int main() {
    test::GlContext context;
    std::string error;
    if (!context.open(error) || !vkgl::active()) {
        std::printf("SKIP: Vulkan unavailable: %s\n", error.c_str());
        return test::kSkip;
    }
    const auto uniform_ui =
        reinterpret_cast<void(OPENRAC_GLAPI*)(GLint, GLuint)>(vkgl::get_proc("glUniform1ui"));
    CHECK(uniform_ui != nullptr);
    renderer::Shader shader;
    if (!shader.build(
            "uniform snapshot fixture",
            R"(#version 410 core

void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)",
            R"(#version 410 core
uniform sampler2D tex_T0;
uniform int control;
uniform uint unsigned_control;
uniform float scalar;
uniform vec2 pair_value;
uniform vec3 triple;
uniform vec4 colour;
uniform vec4 values[2];
uniform mat4 matrices[2];
layout(location = 0) out vec4 result;

void main() {
    float checksum = float(control) + float(unsigned_control) + scalar + pair_value.x + pair_value.y
                     + triple.x + triple.y + triple.z + values[0].x + values[1].x
                     + matrices[0][1][0] + matrices[1][0][1];
    result = vec4(texture(tex_T0, vec2(0.5)).rgb * colour.rgb + checksum * 0.001, 1);
}
)",
            error
        )) {
        std::printf("%s\n", error.c_str());
        return 1;
    }
    renderer::FrameBuffer frame;
    if (!frame.create(5, 1, error)) {
        std::printf("%s\n", error.c_str());
        return 1;
    }
    unsigned vao = 0;
    glGenVertexArrays(1, &vao);
    std::array<unsigned, 2> textures{};
    glGenTextures(2, textures.data());
    for (unsigned i = 0; i < 2; ++i) {
        const std::array<unsigned char, 4> texel{
            static_cast<unsigned char>(i ? 0 : 255),
            255,
            static_cast<unsigned char>(i ? 0 : 255),
            255
        };
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    const auto* off = std::getenv("OPENRAC_VK_UNIFORM_REUSE_OFF");
    const bool reference = off != nullptr;
    std::array<float, 32> matrices{};
    for (unsigned i = 0; i < 2; ++i) {
        for (unsigned j = 0; j < 4; ++j) {
            matrices[i * 16 + j * 5] = 1;
        }
    }
    matrices[4] = 11;
    matrices[17] = 12;
    std::array<float, 32> transposed{};
    for (unsigned i = 0; i < 2; ++i) {
        for (unsigned r = 0; r < 4; ++r) {
            for (unsigned c = 0; c < 4; ++c) {
                transposed[i * 16 + c * 4 + r] = matrices[i * 16 + r * 4 + c];
            }
        }
    }
    const float values[]{9, 0, 0, 0, 10, 0, 0, 0};
    auto constants = [&](float red, bool transpose) {
        shader.use();
        glUniform1i(shader.uniform("tex_T0"), 0);
        glUniform1i(shader.uniform("control"), 1);
        uniform_ui(shader.uniform("unsigned_control"), 2);
        glUniform1f(shader.uniform("scalar"), 3);
        glUniform2f(shader.uniform("pair_value"), 4, 5);
        glUniform3f(shader.uniform("triple"), 6, 7, 8);
        glUniform4f(shader.uniform("colour"), red, 0.25f, 0.125f, 1);
        glUniform4fv(shader.uniform("values"), 2, values);
        glUniformMatrix4fv(
            shader.uniform("matrices"),
            2,
            transpose ? GL_TRUE : GL_FALSE,
            transpose ? transposed.data() : matrices.data()
        );
        // Ignored locations must remain ignored across the entire array.
        glUniform4fv(-1, 2, values);
        glUniformMatrix4fv(-1, 2, GL_FALSE, matrices.data());
    };
    auto setup = [&] {
        glBindFramebuffer(GL_FRAMEBUFFER, frame.id());
        glBindVertexArray(vao);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glActiveTexture(GL_TEXTURE0);
    };
    // Read back only after recording every value/texture change: earlier draws
    // must retain immutable constants and bindings. Repeat through frame-slot reuse.
    for (unsigned round = 0; round < 4; ++round) {
        setup();
        const auto before = vkgl::submission_counters();
        for (unsigned i = 0; i < 5; ++i) {
            constants(i == 2 || i == 3 ? 0.25f : 0.5f, i % 2 != 0);
            glBindTexture(GL_TEXTURE_2D, textures[i == 3 ? 1 : 0]);
            glViewport(static_cast<int>(i), 0, 1, 1);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        const auto after = vkgl::submission_counters();
        CHECK(after.uniform_uploads - before.uniform_uploads == (reference ? 5u : 3u));
        CHECK(after.descriptor_sets - before.descriptor_sets == (reference ? 5u : 2u));
        const auto pixels = frame.read_rgba();
        CHECK(pixels.size() == 20);
        for (unsigned i = 0; i < 5 && pixels.size() == 20; ++i) {
            const float red = i == 3 ? 0.078f : (i == 2 ? 0.328f : 0.578f);
            const float blue = i == 3 ? 0.078f : 0.203f;
            CHECK(std::abs(float(pixels[i * 4]) / 255 - red) < 0.006f);
            CHECK(std::abs(float(pixels[i * 4 + 1]) / 255 - 0.328f) < 0.006f);
            CHECK(std::abs(float(pixels[i * 4 + 2]) / 255 - blue) < 0.006f);
        }
        // Readback flushes and restarts command recording in the same frame.
        // Rebinding a cached descriptor must still work afterwards.
        setup();
        constants(0.5f, false);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glViewport(0, 0, 1, 1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        CHECK(frame.read_rgba() == pixels);
        context.window().swap();
    }
    // Warmed CPU recording fixture. This measures submission, not game FPS.
    std::uint64_t uploads = 0, sets = 0, bytes = 0, skipped = 0;
    double milliseconds = 0;
    constexpr unsigned rounds = 12, draws = 1024;
    for (unsigned round = 0; round < rounds; ++round) {
        setup();
        glViewport(0, 0, 5, 1);
        const auto before = vkgl::submission_counters();
        const auto start = std::chrono::steady_clock::now();
        for (unsigned i = 0; i < draws; ++i) {
            constants((i / 8) % 2 ? 0.25f : 0.5f, false);
            glBindTexture(GL_TEXTURE_2D, textures[(i / 16) % 2]);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        const auto elapsed =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                .count();
        const auto after = vkgl::submission_counters();
        if (round >= 2) {
            milliseconds += elapsed;
            uploads += after.uniform_uploads - before.uniform_uploads;
            sets += after.descriptor_sets - before.descriptor_sets;
            bytes += after.uniform_upload_bytes - before.uniform_upload_bytes;
            skipped += after.uniform_writes_skipped - before.uniform_writes_skipped;
        }
        context.window().swap();
    }
    std::printf(
        "submission reference=%d draws=%u cpu_ms=%.3f uploads=%llu upload_bytes=%llu "
        "descriptor_sets=%llu skipped_writes=%llu\n",
        reference ? 1 : 0,
        draws * (rounds - 2),
        milliseconds,
        static_cast<unsigned long long>(uploads),
        static_cast<unsigned long long>(bytes),
        static_cast<unsigned long long>(sets),
        static_cast<unsigned long long>(skipped)
    );
    // Force the default-uniform ring across its 8 MiB chunk boundary. A new
    // backing buffer needs a new descriptor; old draws must keep the old one.
    renderer::Shader rollover;
    CHECK(rollover.build(
        "uniform ring rollover",
        R"(#version 410 core

void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)",
        R"(#version 410 core
uniform int selected;
uniform vec4 payload[512];
layout(location = 0) out vec4 result;

void main() {
    result = payload[selected];
}
)",
        error
    ));
    std::array<float, 512 * 4> payload{};
    for (unsigned i = 0; i < 512; ++i) {
        payload[i * 4] = float(i % 7) / 8;
        payload[i * 4 + 1] = float(i % 5) / 8;
        payload[i * 4 + 2] = float(i % 3) / 4;
        payload[i * 4 + 3] = 1;
    }
    setup();
    rollover.use();
    glUniform4fv(rollover.uniform("payload"), 512, payload.data());
    const auto before_rollover = vkgl::submission_counters();
    for (unsigned i = 0; i < 1200; ++i) {
        glUniform1i(rollover.uniform("selected"), static_cast<int>(i % 512));
        glViewport(static_cast<int>(i % 5), 0, 1, 1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    const auto after_rollover = vkgl::submission_counters();
    if (!reference) {
        CHECK(after_rollover.descriptor_sets - before_rollover.descriptor_sets >= 2);
        CHECK(after_rollover.descriptor_sets - before_rollover.descriptor_sets <= 4);
    }
    const auto rolled_pixels = frame.read_rgba();
    CHECK(rolled_pixels.size() == 20);
    for (unsigned x = 0; x < 5 && rolled_pixels.size() == 20; ++x) {
        const auto at = (1195 + x) % 512;
        for (unsigned c = 0; c < 4; ++c) {
            CHECK(std::abs(float(rolled_pixels[x * 4 + c]) / 255 - payload[at * 4 + c]) < 0.006f);
        }
    }
    CHECK(glGetError() == GL_NO_ERROR);
    glDeleteTextures(2, textures.data());
    glDeleteVertexArrays(1, &vao);
    texel_buffer_fallbacks();
    return test::result();
}

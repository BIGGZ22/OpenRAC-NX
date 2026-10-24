// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Pins what the Vulkan backend does with a GL buffer that a renderer fills once and draws for
// many frames (renderer/vulkan/vkgl_resources.cpp, resolve; vkgl_gl.cpp, the buffer functions),
// for the change that moves such buffers into device memory and frees their CPU copy
// (docs/port/RENDER_PERF.md, "R-05"). It passes before that change and after it.
//
// One buffer of kBufferBytes, filled with glBufferData(GL_STATIC_DRAW), holds a tint at offset 0,
// a full-screen triangle's positions and its vertices' colour; another holds 32-bit and 8-bit
// indices. Each check draws, reads the picture's middle pixel back with glReadPixels (the one
// read-back the backend has) and compares it with the colour it wrote: after the buffer has been
// drawn for a few frames, after glBufferSubData rewrites a part of it, after glBufferData fills it
// again, through 8-bit indices, and as a uniform block. The expected pixels are the colours'
// channels times 255, which kColour and its kin choose to be whole numbers.
//
// Skipped (77) where Vulkan cannot open.

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "renderer/gl.h"
#include "renderer/shader.h"
#include "renderer/vulkan/vkgl.h"
#include "renderer/world/gl_world.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

namespace {

using namespace openrac::gl;
using openrac::renderer::world::gl::GL_UNIFORM_BUFFER;

// The size of both buffers: past the 64 KiB from which a static buffer is kept in device memory.
constexpr std::size_t kBufferBytes = 128 * 1024;

// Where the vertex buffer holds its parts: the tint (a uniform block's one vec4), the triangle's
// three vec2 positions and its three vec4 colours.
constexpr std::size_t kTintOffset = 0;
constexpr std::size_t kPositionOffset = 64;
constexpr std::size_t kColourOffset = 256;

// Where the index buffer holds its three 8-bit indices; its 32-bit ones are at 0.
constexpr std::size_t kByteIndexOffset = 4096;

// The picture's side, the context's own (renderer_gl_context.h), and its middle.
constexpr int kSide = 64;
constexpr int kMiddle = kSide / 2;

// Frames drawn before a buffer is changed: past the three after which the backend has always
// given an unchanged buffer a copy of its own, so that the change meets that copy.
constexpr int kSettleFrames = 5;

// The colours written, each channel a whole number over 255 so that the pixel read is exact.
constexpr std::array<float, 4> kColour = {51.0f / 255, 102.0f / 255, 153.0f / 255, 1.0f};
constexpr std::array<float, 4> kRewritten = {204.0f / 255, 51.0f / 255, 0.0f, 1.0f};
constexpr std::array<float, 4> kRefilled = {0.0f, 255.0f / 255, 102.0f / 255, 1.0f};
constexpr std::array<float, 4> kTint = {153.0f / 255, 153.0f / 255, 51.0f / 255, 1.0f};

// The triangle that covers the whole picture, in clip space.
constexpr std::array<float, 6> kTriangle = {-1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f};

// The shaders: colour from the vertices, or from the uniform block Tint.
constexpr const char* kVertexShader = "#version 410 core\n"
                                      "layout(location = 0) in vec2 position;\n"
                                      "layout(location = 1) in vec4 colour;\n"
                                      "out vec4 shade;\n"
                                      "void main() {\n"
                                      "    shade = colour;\n"
                                      "    gl_Position = vec4(position, 0.0, 1.0);\n"
                                      "}\n";
constexpr const char* kVertexColour = "#version 410 core\n"
                                      "in vec4 shade;\n"
                                      "layout(location = 0) out vec4 result;\n"
                                      "void main() { result = shade; }\n";
constexpr const char* kTintColour = "#version 410 core\n"
                                    "in vec4 shade;\n"
                                    "layout(std140) uniform Tint { vec4 tint; };\n"
                                    "layout(location = 0) out vec4 result;\n"
                                    "void main() { result = tint + shade * 0.0; }\n";

// The uniform block binding the tint shader reads.
constexpr GLuint kTintBinding = 3;

// The vertex buffer's bytes with `colour` on each vertex.
std::vector<std::uint8_t> vertex_bytes(const std::array<float, 4>& colour) {
    std::vector<std::uint8_t> bytes(kBufferBytes, 0);
    std::memcpy(bytes.data() + kTintOffset, kTint.data(), sizeof(kTint));
    std::memcpy(bytes.data() + kPositionOffset, kTriangle.data(), sizeof(kTriangle));

    // The same colour on each of the three vertices.
    for (std::size_t vertex = 0; vertex < 3; ++vertex) {
        std::memcpy(
            bytes.data() + kColourOffset + vertex * sizeof(colour), colour.data(), sizeof(colour)
        );
    }

    return bytes;
}

// The index buffer's bytes: 0, 1, 2 as 32-bit indices at 0 and as 8-bit ones at kByteIndexOffset.
std::vector<std::uint8_t> index_bytes() {
    std::vector<std::uint8_t> bytes(kBufferBytes, 0);
    constexpr std::array<std::uint32_t, 3> kWide = {0, 1, 2};
    constexpr std::array<std::uint8_t, 3> kNarrow = {0, 1, 2};
    std::memcpy(bytes.data(), kWide.data(), sizeof(kWide));
    std::memcpy(bytes.data() + kByteIndexOffset, kNarrow.data(), sizeof(kNarrow));
    return bytes;
}

// The pixel `colour` is drawn as: each channel times 255.
std::array<std::uint8_t, 4> pixel_of(const std::array<float, 4>& colour) {
    std::array<std::uint8_t, 4> pixel{};

    // Each channel on its own.
    for (std::size_t channel = 0; channel < pixel.size(); ++channel) {
        pixel[channel] = static_cast<std::uint8_t>(std::lround(colour[channel] * 255.0f));
    }

    return pixel;
}

// The GL objects every check draws with, made on an open context.
struct Scene {
    GLuint vao = 0;
    GLuint vertices = 0;
    GLuint indices = 0;
    openrac::renderer::Shader coloured;  // Colour from the vertices.
    openrac::renderer::Shader tinted;    // Colour from the uniform block.
};

// Makes `scene`'s buffers, filled once with GL_STATIC_DRAW, and the vertex array that reads them.
void make_buffers(Scene& scene) {
    const std::vector<std::uint8_t> vertices = vertex_bytes(kColour);
    const std::vector<std::uint8_t> indices = index_bytes();
    glGenVertexArrays(1, &scene.vao);
    glBindVertexArray(scene.vao);
    glGenBuffers(1, &scene.vertices);
    glBindBuffer(GL_ARRAY_BUFFER, scene.vertices);
    glBufferData(GL_ARRAY_BUFFER, kBufferBytes, vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(kPositionOffset)
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 4, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(kColourOffset)
    );
    glGenBuffers(1, &scene.indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene.indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, kBufferBytes, indices.data(), GL_STATIC_DRAW);
}

// Deletes `scene`'s buffers and vertex array.
void delete_buffers(Scene& scene) {
    glDeleteBuffers(1, &scene.vertices);
    glDeleteBuffers(1, &scene.indices);
    glDeleteVertexArrays(1, &scene.vao);
}

// Clears the picture, draws the triangle with `shader` through `index_type`'s indices, reads the
// middle pixel back and ends the frame.
std::array<std::uint8_t, 4> draw_frame(
    openrac::test::GlContext& context, const openrac::renderer::Shader& shader, GLenum index_type
) {
    glClear(GL_COLOR_BUFFER_BIT);
    shader.use();

    // The 8-bit indices sit at their own offset; the 32-bit ones at the buffer's start.
    if (index_type == GL_UNSIGNED_BYTE) {
        glDrawElements(
            GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, reinterpret_cast<const void*>(kByteIndexOffset)
        );
    } else {
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
    }

    std::array<std::uint8_t, 4> pixel{};
    glReadPixels(kMiddle, kMiddle, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
    context.window().swap();
    return pixel;
}

// Draws kSettleFrames frames and returns whether each showed `colour`.
bool settle(
    openrac::test::GlContext& context,
    const openrac::renderer::Shader& shader,
    const std::array<float, 4>& colour
) {
    bool all = true;

    // Every frame read, so that each of the backend's first frames is checked.
    for (int frame = 0; frame < kSettleFrames; ++frame) {
        all = all && draw_frame(context, shader, GL_UNSIGNED_INT) == pixel_of(colour);
    }

    return all;
}

// Tests that a static buffer draws its data in each of its first frames and after them.
void test_a_static_buffer_draws_its_data(openrac::test::GlContext& context, Scene& scene) {
    make_buffers(scene);
    CHECK(settle(context, scene.coloured, kColour));
    delete_buffers(scene);
}

// Tests that glBufferSubData on a static buffer that has been drawn for a while is drawn at once.
void test_a_part_written_later_is_drawn(openrac::test::GlContext& context, Scene& scene) {
    make_buffers(scene);
    CHECK(settle(context, scene.coloured, kColour));
    const std::vector<std::uint8_t> rewritten = vertex_bytes(kRewritten);
    glBindBuffer(GL_ARRAY_BUFFER, scene.vertices);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        static_cast<GLintptr>(kColourOffset),
        3 * sizeof(kRewritten),
        rewritten.data() + kColourOffset
    );
    CHECK(settle(context, scene.coloured, kRewritten));
    delete_buffers(scene);
}

// Tests that glBufferData on a static buffer that has been drawn for a while replaces it.
void test_buffer_data_again_replaces_it(openrac::test::GlContext& context, Scene& scene) {
    make_buffers(scene);
    CHECK(settle(context, scene.coloured, kColour));
    const std::vector<std::uint8_t> refilled = vertex_bytes(kRefilled);
    glBindBuffer(GL_ARRAY_BUFFER, scene.vertices);
    glBufferData(GL_ARRAY_BUFFER, kBufferBytes, refilled.data(), GL_STATIC_DRAW);
    CHECK(settle(context, scene.coloured, kRefilled));
    delete_buffers(scene);
}

// Tests that 8-bit indices, which the backend widens on the CPU, are read from a static buffer
// that has been drawn for a while.
void test_byte_indices_are_read_later(openrac::test::GlContext& context, Scene& scene) {
    make_buffers(scene);
    CHECK(settle(context, scene.coloured, kColour));
    CHECK(draw_frame(context, scene.coloured, GL_UNSIGNED_BYTE) == pixel_of(kColour));
    CHECK(draw_frame(context, scene.coloured, GL_UNSIGNED_BYTE) == pixel_of(kColour));
    delete_buffers(scene);
}

// Tests that a static buffer drawn for a while as vertices is read as a uniform block afterwards.
void test_a_static_buffer_is_a_uniform_block_later(
    openrac::test::GlContext& context, Scene& scene
) {
    make_buffers(scene);
    CHECK(settle(context, scene.coloured, kColour));
    glBindBufferBase(GL_UNIFORM_BUFFER, kTintBinding, scene.vertices);
    CHECK(draw_frame(context, scene.tinted, GL_UNSIGNED_INT) == pixel_of(kTint));
    CHECK(draw_frame(context, scene.tinted, GL_UNSIGNED_INT) == pixel_of(kTint));
    glBindBufferBase(GL_UNIFORM_BUFFER, kTintBinding, 0);
    delete_buffers(scene);
}

// Builds `scene`'s two programs, the tinted one's block at kTintBinding. False with `error` set.
bool build_shaders(Scene& scene, std::string& error) {
    // Either program failing to build leaves nothing to draw with.
    if (!scene.coloured.build("buffer test, vertex colour", kVertexShader, kVertexColour, error)
        || !scene.tinted.build("buffer test, tint", kVertexShader, kTintColour, error)) {
        return false;
    }

    const GLuint block = glGetUniformBlockIndex(scene.tinted.id(), "Tint");
    glUniformBlockBinding(scene.tinted.id(), block, kTintBinding);
    return true;
}

}  // namespace

int main() {
    namespace vkgl = openrac::vkgl;
    openrac::test::GlContext context;
    std::string error;

    // No Vulkan here (or it fell back to OpenGL): nothing of the backend to test.
    if (!context.open(error, kSide, kSide) || !vkgl::active()) {
        std::printf("SKIP: Vulkan unavailable: %s\n", error.c_str());
        return openrac::test::kSkip;
    }

    Scene scene;
    CHECK(build_shaders(scene, error));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, kSide, kSide);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    test_a_static_buffer_draws_its_data(context, scene);
    test_a_part_written_later_is_drawn(context, scene);
    test_buffer_data_again_replaces_it(context, scene);
    test_byte_indices_are_read_later(context, scene);
    test_a_static_buffer_is_a_uniform_block_later(context, scene);
    CHECK(vkgl::submission_counters().validation_errors == 0);
    return openrac::test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "renderer/framebuffer.h"
#include "renderer/render_opt.h"
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

// ---- Pipelines and descriptor sets by state (R-04, vk-draw-cache) ----

// The states the state-bounce test draws with: each changes one input of a draw's pipeline or of
// its descriptor set from the base state.
enum class Bounce {
    BlendFactor,      // the blend's destination factor
    DepthFunc,        // the depth test's function
    AttributeFormat,  // the format of the tint attribute
    BoundTexture,     // the texture on unit 0
};

// What the state-bounce test draws with: a program that shows its texture times its tint, two
// textures and two tint buffers of different formats, and a 3 by 1 target with depth.
struct BounceFixture {
    renderer::Shader shader;
    renderer::FrameBuffer target;
    unsigned vao = 0;
    std::array<unsigned, 2> textures{};  // White, then blue.
    unsigned byte_tints = 0;             // White as normalised bytes, three vertices.
    unsigned float_tints = 0;            // Green as floats, three vertices.
};

// Sets the base state, then `which`'s state A (`second` false) or B (`second` true).
void set_bounce_state(const BounceFixture& fixture, Bounce which, bool second) {
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
    glDepthFunc(GL_ALWAYS);
    glBindBuffer(GL_ARRAY_BUFFER, fixture.byte_tints);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fixture.textures[0]);

    // State B of each kind; A is the base.
    if (!second) {
        return;
    }

    switch (which) {
        // Keeps the clear colour instead of writing the draw's.
        case Bounce::BlendFactor:
            glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ONE);
            break;
        // Fails every fragment, so the clear colour stays.
        case Bounce::DepthFunc:
            glDepthFunc(GL_NEVER);
            break;
        // Green floats instead of white bytes: another vertex format, so another pipeline.
        case Bounce::AttributeFormat:
            glBindBuffer(GL_ARRAY_BUFFER, fixture.float_tints);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
            break;
        // The blue texture: another descriptor set.
        case Bounce::BoundTexture:
            glBindTexture(GL_TEXTURE_2D, fixture.textures[1]);
            break;
    }
}

// Makes the state-bounce fixture's program, textures, buffers and target. False when one fails.
bool make_bounce_fixture(BounceFixture& fixture, std::string& error) {
    const bool built = fixture.shader.build(
        "pipeline and descriptor cache fixture",
        R"(#version 410 core
layout(location = 1) in vec4 tint;
out vec4 v_tint;
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    v_tint = tint;
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)",
        R"(#version 410 core
uniform sampler2D picture;
in vec4 v_tint;
layout(location = 0) out vec4 result;
void main() {
    result = texture(picture, vec2(0.5)) * v_tint;
}
)",
        error
    );

    // A program or target that cannot be made leaves nothing to draw.
    if (!built || !fixture.target.create(3, 1, error)) {
        return false;
    }

    // One texel each: white, then blue.
    constexpr std::array<std::array<unsigned char, 4>, 2> kTexels = {{
        {255, 255, 255, 255},
        {0, 0, 255, 255},
    }};
    glGenTextures(2, fixture.textures.data());

    // Each texture, sampled without filtering.
    for (std::size_t n = 0; n < kTexels.size(); ++n) {
        glBindTexture(GL_TEXTURE_2D, fixture.textures[n]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, kTexels[n].data()
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    // The tint of the triangle's three vertices, in two formats.
    constexpr std::array<unsigned char, 12> kByteTints =
        {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
    constexpr std::array<float, 12> kFloatTints = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
    glGenBuffers(1, &fixture.byte_tints);
    glBindBuffer(GL_ARRAY_BUFFER, fixture.byte_tints);
    glBufferData(GL_ARRAY_BUFFER, sizeof kByteTints, kByteTints.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &fixture.float_tints);
    glBindBuffer(GL_ARRAY_BUFFER, fixture.float_tints);
    glBufferData(GL_ARRAY_BUFFER, sizeof kFloatTints, kFloatTints.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &fixture.vao);
    glBindVertexArray(fixture.vao);
    glEnableVertexAttribArray(1);
    return true;
}

// Draws the fixture's triangle into pixel `x` of its target.
void draw_bounce_pixel(int x) {
    glViewport(x, 0, 1, 1);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// Tests that two states differing in one input give different pipelines or descriptor sets, and
// that after a bounce A, B, A with no draw between, the draw is A's again: pixel 0 is drawn with
// A, pixel 1 with B, pixel 2 after the bounce, and pixel 2 must be pixel 0, not pixel 1. The
// third draw also makes no pipeline: A's is found, not a new one or B's.
void test_a_state_bounce_draws_the_first_state_again(BounceFixture& fixture, Bounce which) {
    glBindFramebuffer(GL_FRAMEBUFFER, fixture.target.id());
    glBindVertexArray(fixture.vao);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDepthMask(GL_TRUE);
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClearDepth(0.5);
    glViewport(0, 0, 3, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    fixture.shader.use();
    glUniform1i(fixture.shader.uniform("picture"), 0);
    set_bounce_state(fixture, which, false);
    draw_bounce_pixel(0);
    set_bounce_state(fixture, which, true);
    draw_bounce_pixel(1);
    set_bounce_state(fixture, which, false);
    set_bounce_state(fixture, which, true);
    set_bounce_state(fixture, which, false);
    const auto before = vkgl::submission_counters();
    draw_bounce_pixel(2);
    const auto after = vkgl::submission_counters();
    const std::vector<std::uint8_t> pixels = fixture.target.read_rgba();
    constexpr std::size_t kChannels = 4;

    CHECK(after.pipelines_created == before.pipelines_created);
    CHECK(pixels.size() == 3 * kChannels);

    // A target that did not read back has no pixels to compare.
    if (pixels.size() != 3 * kChannels) {
        return;
    }

    const auto pixel = [&pixels](std::size_t x) {
        return std::vector<std::uint8_t>(
            pixels.begin() + static_cast<std::ptrdiff_t>(x * kChannels),
            pixels.begin() + static_cast<std::ptrdiff_t>((x + 1) * kChannels)
        );
    };
    CHECK(pixel(0) != pixel(1));
    CHECK(pixel(2) == pixel(0));
}

// Draws one triangle with a program made for it into the fixture's target, so that its pipeline
// is made before the fixture's, and returns the program.
renderer::Shader draw_with_an_earlier_program(const BounceFixture& fixture, std::string& error) {
    renderer::Shader earlier;
    CHECK(earlier.build(
        "a program deleted while others' pipelines stay",
        R"(#version 410 core
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)",
        R"(#version 410 core
layout(location = 0) out vec4 result;
void main() {
    result = vec4(1.0, 0.0, 1.0, 1.0);
}
)",
        error
    ));
    glBindFramebuffer(GL_FRAMEBUFFER, fixture.target.id());
    glBindVertexArray(fixture.vao);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    earlier.use();
    draw_bounce_pixel(0);
    return earlier;
}

// Runs the state-bounce test for each kind of state, on a fixture of its own. A program whose
// pipeline was made before the fixture's is deleted halfway: its pipelines go, and the fixture's,
// made after it, must stay usable (the validation layer reports a destroyed one).
void test_state_bounces() {
    BounceFixture fixture;
    std::string error;
    const bool made = make_bounce_fixture(fixture, error);
    CHECK(made);

    // Without the fixture there is nothing to draw.
    if (!made) {
        std::printf("%s\n", error.c_str());
        return;
    }

    renderer::Shader earlier = draw_with_an_earlier_program(fixture, error);
    constexpr std::array<Bounce, 4> kBounces =
        {Bounce::BlendFactor, Bounce::DepthFunc, Bounce::AttributeFormat, Bounce::BoundTexture};

    // Each kind of state.
    for (const Bounce which : kBounces) {
        test_a_state_bounce_draws_the_first_state_again(fixture, which);
    }

    earlier.release();

    // The frames that end before the deleted program's pipelines are destroyed: a frame slot's
    // deferred work runs when the slot comes round again, and the backend has two.
    constexpr int kFramesUntilDestroyed = 3;

    // Each of those frames ended.
    for (int frame = 0; frame < kFramesUntilDestroyed; ++frame) {
        vkgl::present();
    }

    // Each kind of state again, with the fixture's pipelines as they were made.
    for (const Bounce which : kBounces) {
        test_a_state_bounce_draws_the_first_state_again(fixture, which);
    }

    glDeleteTextures(2, fixture.textures.data());
    glDeleteBuffers(1, &fixture.byte_tints);
    glDeleteBuffers(1, &fixture.float_tints);
    glDeleteVertexArrays(1, &fixture.vao);
    CHECK(glGetError() == GL_NO_ERROR);
}

// ---- The fixed state as dynamic state (R-11, vk-extended-dynamic-state) ----

// The cull and front-face states the fixed-state test draws, one a pixel, in one pass: each
// pixel's triangle winds counter-clockwise, as GL counts it.
struct CullCase {
    bool cull = false;           // GL_CULL_FACE on.
    GLenum cull_face = GL_BACK;  // Which faces it culls.
    GLenum front = GL_CCW;       // Which winding is the front.
};

// Draws the fixed-state test's cases into `target`, one a pixel, cull and front face toggled
// between the draws with no pass between them, and returns the pixels.
std::vector<std::uint8_t> draw_cull_cases(
    const renderer::FrameBuffer& target, const renderer::Shader& shader, unsigned vao
) {
    // clang-format off: one case per row: whether it culls, what, and the front winding.
    constexpr std::array<CullCase, 5> kCases = {{
        {false, GL_BACK, GL_CCW},  // drawn
        {true, GL_BACK, GL_CCW},   // drawn: its front faces the viewer
        {true, GL_BACK, GL_CW},    // culled: its back faces the viewer
        {true, GL_FRONT, GL_CCW},  // culled: its front is culled
        {true, GL_FRONT, GL_CW},   // drawn
    }};
    // clang-format on
    glBindFramebuffer(GL_FRAMEBUFFER, target.id());
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glViewport(0, 0, static_cast<int>(kCases.size()), 1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    shader.use();

    // Each case into its pixel.
    for (std::size_t n = 0; n < kCases.size(); ++n) {
        const CullCase& c = kCases[n];

        // Culling on or off for this case.
        if (c.cull) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }

        glCullFace(c.cull_face);
        glFrontFace(c.front);
        glViewport(static_cast<int>(n), 0, 1, 1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    return target.read_rgba();
}

// Tests that cull and front face toggled between draws in one pass draw the same pixels with the
// fixed state as dynamic state as with it baked into each pipeline, and the pixels each case's
// winding and culling give.
void test_cull_and_front_face_toggled_in_one_pass() {
    renderer::Shader shader;
    std::string error;
    CHECK(shader.build(
        "fixed-state fixture",
        R"(#version 410 core
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)",
        R"(#version 410 core
layout(location = 0) out vec4 result;
void main() {
    result = vec4(1.0, 1.0, 1.0, 1.0);
}
)",
        error
    ));
    renderer::FrameBuffer target;
    CHECK(target.create(5, 1, error));
    unsigned vao = 0;
    glGenVertexArrays(1, &vao);
    namespace render_opt = renderer::render_opt;
    const bool began = render_opt::enabled(render_opt::Opt::VkExtendedDynamicState);
    render_opt::set(render_opt::Opt::VkExtendedDynamicState, true);
    const std::vector<std::uint8_t> dynamic = draw_cull_cases(target, shader, vao);
    render_opt::set(render_opt::Opt::VkExtendedDynamicState, false);
    const std::vector<std::uint8_t> baked = draw_cull_cases(target, shader, vao);
    render_opt::set(render_opt::Opt::VkExtendedDynamicState, began);
    constexpr std::size_t kChannels = 4;
    constexpr std::array<bool, 5> kDrawn = {true, true, false, false, true};
    CHECK(dynamic == baked);
    CHECK(dynamic.size() == kDrawn.size() * kChannels);

    // Each pixel white where its case draws and black where it culls.
    for (std::size_t n = 0; n < kDrawn.size() && dynamic.size() == kDrawn.size() * kChannels; ++n) {
        CHECK((dynamic[n * kChannels] == 255) == kDrawn[n]);
    }

    glDeleteVertexArrays(1, &vao);
    CHECK(glGetError() == GL_NO_ERROR);
}

// ---- Descriptor sets that outlive the frame (R-14, vk-persistent-descriptors) ----

// Makes a 1 by 1 texture of `texel` and returns its name.
unsigned one_texel_texture(const std::array<unsigned char, 4>& texel) {
    unsigned texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texture;
}

// Draws `texture` into the 1 by 1 `target` with `shader` and returns the pixel.
std::vector<std::uint8_t> draw_texture(
    const renderer::FrameBuffer& target,
    const renderer::Shader& shader,
    unsigned vao,
    unsigned texture
) {
    glBindFramebuffer(GL_FRAMEBUFFER, target.id());
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, 1, 1);
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(shader.uniform("picture"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    return target.read_rgba();
}

// Tests that a texture deleted and replaced by a new one, which may get the same Vulkan handles,
// never draws through the old texture's descriptor set: each round draws texture A, lets frames
// pass so that A's image and views are destroyed, makes texture B of another colour and draws it,
// and requires B's colour.
void test_a_deleted_texture_s_set_is_never_drawn_again() {
    renderer::Shader shader;
    std::string error;
    CHECK(shader.build(
        "deleted texture fixture",
        R"(#version 410 core
void main() {
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);
}
)",
        R"(#version 410 core
uniform sampler2D picture;
layout(location = 0) out vec4 result;
void main() {
    result = texture(picture, vec2(0.5));
}
)",
        error
    ));
    renderer::FrameBuffer target;
    CHECK(target.create(1, 1, error));
    unsigned vao = 0;
    glGenVertexArrays(1, &vao);
    constexpr std::array<unsigned char, 4> kRed = {255, 0, 0, 255};
    constexpr std::array<unsigned char, 4> kBlue = {0, 0, 255, 255};

    // The rounds give the driver several chances to hand out a handle again.
    constexpr int kRounds = 8;

    // The frames that end before a deleted texture's image is destroyed: a frame slot's deferred
    // work runs when the slot comes round again, and the backend has two.
    constexpr int kFramesUntilDestroyed = 3;

    // Each round: A drawn, A deleted and destroyed, B made and drawn.
    for (int round = 0; round < kRounds; ++round) {
        const unsigned first = one_texel_texture(kRed);
        const std::vector<std::uint8_t> red = draw_texture(target, shader, vao, first);
        CHECK(red.size() == 4 && red[0] == 255 && red[2] == 0);
        vkgl::present();
        glDeleteTextures(1, &first);

        // The frames after which A's image and views are gone.
        for (int frame = 0; frame < kFramesUntilDestroyed; ++frame) {
            vkgl::present();
        }

        const unsigned second = one_texel_texture(kBlue);
        const std::vector<std::uint8_t> blue = draw_texture(target, shader, vao, second);
        CHECK(blue.size() == 4 && blue[0] == 0 && blue[2] == 255);
        vkgl::present();
        glDeleteTextures(1, &second);
    }

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
    test_state_bounces();
    test_cull_and_front_face_toggled_in_one_pass();
    test_a_deleted_texture_s_set_is_never_drawn_again();
    return test::result();
}

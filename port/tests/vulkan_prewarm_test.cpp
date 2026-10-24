// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests the Vulkan backend's pipeline pre-warm (R-10, renderer/vulkan/vkgl_internal.h's
// PipelinePrewarm and PipelineManifest): the pipelines one run makes are made again in the next
// run on the worker, once their program links, so its drawing thread makes none of them; a damaged
// manifest, and one whose program is no longer built, are ignored and the draws make their own;
// with the registry entry pipeline-prewarm off nothing is made ahead.
//
// Each run is a context of its own, opened and closed in turn, on one private pipeline cache file
// and its manifest (OPENRAC_VULKAN_PIPELINE_CACHE, set before anything reads the environment). The
// expected counts follow from what each run draws: kStates pipelines of one program.

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/render_opt.h"
#include "renderer/shader.h"
#include "renderer/vulkan/vkgl.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

namespace {

namespace fs = std::filesystem;
namespace render_opt = openrac::renderer::render_opt;
using namespace openrac::gl;
namespace vkgl = openrac::vkgl;

// The states each run draws with, each its own pipeline: blend off, blend on, the depth test.
constexpr int kStates = 3;

// The size of the picture drawn into.
constexpr int kSide = 4;

// The fragment colours of the two programs: one the first runs build, one a run builds instead.
constexpr std::string_view kFirstColour = "vec4(0.25, 0.5, 0.75, 1.0)";
constexpr std::string_view kOtherColour = "vec4(0.75, 0.5, 0.25, 1.0)";

// What one run did: the pipelines the worker had made by the first draw, and those the drawing
// thread made itself while drawing.
struct RunCounts {
    std::uint64_t made_ahead = 0;     // wait_for_pipeline_prewarm's count once its jobs are done.
    std::uint64_t made_by_draws = 0;  // pipelines_created over the draws.
    bool ran = false;                 // The context opened on Vulkan.
};

// Sets the environment variable `name` to `value` for this process, before anything reads it:
// common/env.h reads the environment once, at the first question.
void set_variable(const char* name, const std::string& value) {
#if defined(_WIN32)
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}

// Draws one triangle into `target` in each of the kStates states.
void draw_states(const openrac::renderer::FrameBuffer& target) {
    glBindFramebuffer(GL_FRAMEBUFFER, target.id());
    glViewport(0, 0, kSide, kSide);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Each state, from the base: blend and depth test off.
    for (int state = 0; state < kStates; ++state) {
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        // State 1 blends, state 2 tests depth; state 0 is the base.
        if (state == 1) {
            glEnable(GL_BLEND);
        } else if (state == 2) {
            glEnable(GL_DEPTH_TEST);
        }

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

// Runs one context: builds the program of colour `colour`, waits for the pre-warm, draws the
// kStates states and closes. Not `ran` where Vulkan is not here.
RunCounts run(std::string_view colour) {
    RunCounts counts;
    openrac::test::GlContext context;
    std::string error;

    // No Vulkan here: nothing to pre-warm.
    if (!context.open(error) || !vkgl::active()) {
        std::printf("SKIP: Vulkan unavailable: %s\n", error.c_str());
        return counts;
    }

    openrac::renderer::Shader shader;
    const std::string fragment = std::string(
                                     "#version 410 core\n"
                                     "layout(location = 0) out vec4 result;\n"
                                     "void main() { result = "
                                 )
                                 + std::string(colour) + "; }\n";
    CHECK(shader.build(
        "pre-warm fixture",
        "#version 410 core\n"
        "void main() {\n"
        "    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);\n"
        "    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);\n"
        "}\n",
        fragment,
        error
    ));
    openrac::renderer::FrameBuffer target;
    CHECK(target.create(kSide, kSide, error));
    unsigned vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    shader.use();
    counts.made_ahead = vkgl::wait_for_pipeline_prewarm();
    const std::uint64_t before = vkgl::submission_counters().pipelines_created;
    draw_states(target);
    counts.made_by_draws = vkgl::submission_counters().pipelines_created - before;
    CHECK(target.read_rgba().size() == static_cast<std::size_t>(kSide * kSide * 4));
    CHECK(vkgl::submission_counters().validation_errors == 0);
    glDeleteVertexArrays(1, &vao);
    counts.ran = true;
    return counts;
}

// Tests that a second run of the same program makes its pipelines ahead and its draws make none.
// False when Vulkan is not here.
bool test_a_run_s_pipelines_are_made_ahead_in_the_next() {
    const RunCounts first = run(kFirstColour);

    // No Vulkan: the whole test is skipped.
    if (!first.ran) {
        return false;
    }

    const RunCounts second = run(kFirstColour);
    CHECK(first.made_ahead == 0);
    CHECK(first.made_by_draws >= static_cast<std::uint64_t>(kStates));
    CHECK(second.made_ahead >= static_cast<std::uint64_t>(kStates));
    CHECK(second.made_by_draws == 0);
    return true;
}

// Tests that a run that builds another program finds nothing of it in the manifest.
void test_a_manifest_of_a_program_no_longer_built_is_ignored() {
    const RunCounts other = run(kOtherColour);
    CHECK(other.made_ahead == 0);
    CHECK(other.made_by_draws >= static_cast<std::uint64_t>(kStates));
}

// Tests that a damaged manifest is dropped: nothing is made ahead and the draws make their own.
void test_a_damaged_manifest_is_ignored(const fs::path& manifest) {
    CHECK(fs::exists(manifest));

    // The manifest's first bytes overwritten: its envelope's hash no longer matches.
    {
        std::fstream file(manifest, std::ios::binary | std::ios::in | std::ios::out);
        constexpr std::array<char, 4> kGarbage = {'X', 'X', 'X', 'X'};
        file.seekp(0);
        file.write(kGarbage.data(), kGarbage.size());
    }

    const RunCounts damaged = run(kFirstColour);
    CHECK(damaged.made_ahead == 0);
    CHECK(damaged.made_by_draws >= static_cast<std::uint64_t>(kStates));
}

// Tests that with pipeline-prewarm off nothing is made ahead, though the manifest has the keys.
void test_nothing_is_made_ahead_with_the_entry_off() {
    render_opt::set(render_opt::Opt::PipelinePrewarm, false);
    const RunCounts off = run(kFirstColour);
    render_opt::set(render_opt::Opt::PipelinePrewarm, true);
    CHECK(off.made_ahead == 0);
    CHECK(off.made_by_draws >= static_cast<std::uint64_t>(kStates));
}

}  // namespace

int main() {
    const fs::path folder =
        fs::temp_directory_path() / ("openrac_vulkan_prewarm_test_" + std::to_string(std::rand()));
    const fs::path cache = folder / "pipelines.bin";
    std::error_code ec;
    fs::remove_all(folder, ec);
    fs::create_directories(folder, ec);
    set_variable("OPENRAC_VULKAN_PIPELINE_CACHE", cache.string());

    // The entry under test is on, whatever OPENRAC_RENDER_OPT_OFF says (ctest runs the Vulkan set
    // with all entries off too); one test switches it off itself.
    render_opt::set(render_opt::Opt::PipelinePrewarm, true);

    // Without Vulkan nothing below can run.
    if (!test_a_run_s_pipelines_are_made_ahead_in_the_next()) {
        fs::remove_all(folder, ec);
        return openrac::test::kSkip;
    }

    test_nothing_is_made_ahead_with_the_entry_off();
    test_a_manifest_of_a_program_no_longer_built_is_ignored();
    test_a_damaged_manifest_is_ignored(fs::path(cache.string() + ".manifest"));
    fs::remove_all(folder, ec);
    return openrac::test::result();
}

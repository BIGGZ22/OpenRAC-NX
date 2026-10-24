// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests the optimisation registry (renderer/render_opt.h) and the Vulkan backend's render counters
// (renderer/vulkan/vkgl.h, SubmissionCounters). One program, three ctest tests:
//
//   render_opt              the table and the parsing of OPENRAC_RENDER_OPT_OFF, no GPU; it runs in
//                           the headless preset too
//   render_opt_environment  --environment: the variable as ctest sets it, read and applied
//   render_opt_vulkan       --vulkan: draws on the Vulkan renderer and checks that the counters
//                           follow; skipped (77) where Vulkan cannot open
//
// The expected values come from the table itself and from the arithmetic of what each check
// records: a draw is at least one recorded command, and a frame without a read-back is one submit.

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

#include "common/log.h"
#include "renderer/render_opt.h"
#include "tests/check.h"

#if defined(OPENRAC_RENDER_OPT_TEST_VULKAN)
#include "renderer/gl.h"
#include "renderer/shader.h"
#include "renderer/vulkan/vkgl.h"
#include "tests/renderer_gl_context.h"
#endif

namespace {

namespace render_opt = openrac::renderer::render_opt;
using render_opt::Opt;

// The text ctest gives OPENRAC_RENDER_OPT_OFF for render_opt_environment (cmake/Render.cmake).
constexpr std::string_view kUnknownInEnvironment = "no-such-optimisation";

// ---- The table ----

// Tests that every entry has a name of its own, at its Opt's place, and that find returns it.
void test_each_entry_has_a_name_of_its_own() {
    const auto entries = render_opt::entries();
    CHECK(entries.size() == render_opt::kOptCount);

    // Each entry at its place, findable, and named unlike every entry after it.
    for (std::size_t n = 0; n < entries.size(); ++n) {
        CHECK(static_cast<std::size_t>(entries[n].opt) == n);
        CHECK(!entries[n].name.empty());
        CHECK(entries[n].name != "all");
        CHECK(render_opt::find(entries[n].name) == &entries[n]);

        // The names after this one, so each pair is compared once.
        for (std::size_t other = n + 1; other < entries.size(); ++other) {
            CHECK(entries[n].name != entries[other].name);
        }
    }

    CHECK(render_opt::find("") == nullptr);
    CHECK(render_opt::find("Registry-Selftest") == nullptr);
}

// Tests that the self-test entry reads back as the table holds it: on by default, exact frames.
void test_the_selftest_entry_reads_back() {
    const render_opt::Entry& selftest = render_opt::entry(Opt::RegistrySelftest);
    CHECK(selftest.name == "registry-selftest");
    CHECK(selftest.default_on);
    CHECK(selftest.tolerance.exact());
    CHECK(selftest.tolerance == render_opt::Tolerance{});
    CHECK(!selftest.description.empty());
}

// Tests that a tolerance other than zero is not exact.
void test_a_tolerance_other_than_zero_is_not_exact() {
    CHECK(!(render_opt::Tolerance{0.0002, 0}.exact()));
    CHECK(!(render_opt::Tolerance{0.0, 2}.exact()));
}

// ---- Parsing OPENRAC_RENDER_OPT_OFF ----

// Tests that one name switches its entry off and nothing else.
void test_a_name_is_parsed() {
    const render_opt::OffList list = render_opt::parse_off_list("registry-selftest");
    CHECK(list.off.size() == 1);
    CHECK(!list.off.empty() && list.off[0] == Opt::RegistrySelftest);
    CHECK(!list.all);
    CHECK(list.unknown.empty());
}

// Tests that a list with spaces, empty items and a repeated name lists each entry once.
void test_a_list_is_parsed() {
    const render_opt::OffList list =
        render_opt::parse_off_list(" registry-selftest ,, registry-selftest,\t");
    CHECK(list.off.size() == 1);
    CHECK(!list.all);
    CHECK(list.unknown.empty());
}

// Tests that the word all says every entry, alone or in a list.
void test_all_is_parsed() {
    CHECK(render_opt::parse_off_list("all").all);
    CHECK(render_opt::parse_off_list("registry-selftest, all").all);
    CHECK(!render_opt::parse_off_list("ALL").all);
}

// Tests that an unknown name is kept apart, in order, and switches nothing.
void test_an_unknown_name_is_kept_apart() {
    const render_opt::OffList list =
        render_opt::parse_off_list("first-unknown,registry-selftest,second-unknown");
    CHECK(list.unknown.size() == 2);
    CHECK(list.unknown.size() == 2 && list.unknown[0] == "first-unknown");
    CHECK(list.unknown.size() == 2 && list.unknown[1] == "second-unknown");
    CHECK(list.off.size() == 1);
    CHECK(render_opt::parse_off_list("").off.empty());
    CHECK(render_opt::parse_off_list(" , ").unknown.empty());
}

// ---- The switches ----

// Tests that set switches an entry over what the run began with, and reset puts it back.
void test_set_and_reset() {
    const bool began = render_opt::enabled(Opt::RegistrySelftest);
    render_opt::set(Opt::RegistrySelftest, !began);
    CHECK(render_opt::enabled(Opt::RegistrySelftest) == !began);
    render_opt::reset();
    CHECK(render_opt::enabled(Opt::RegistrySelftest) == began);
}

// Marks the text sought as found when the recent log line `text` holds it; `context` is the
// text and the mark (log::recent_lines' callback).
void find_in_line(const char* text, std::size_t length, void* context) {
    auto* wanted = static_cast<std::pair<std::string_view, bool>*>(context);
    wanted->second =
        wanted->second
        || std::string_view(text, length).find(wanted->first) != std::string_view::npos;
}

// Tests that the variable ctest sets is applied: the self-test entry off, the unknown name warned
// of, set and reset over it as over a default.
int test_the_environment_is_applied() {
    CHECK(!render_opt::enabled(Opt::RegistrySelftest));
    std::pair<std::string_view, bool> wanted{kUnknownInEnvironment, false};
    openrac::log::recent_lines(find_in_line, &wanted);
    CHECK(wanted.second);
    render_opt::set(Opt::RegistrySelftest, true);
    CHECK(render_opt::enabled(Opt::RegistrySelftest));
    render_opt::reset();
    CHECK(!render_opt::enabled(Opt::RegistrySelftest));
    return openrac::test::result();
}

#if defined(OPENRAC_RENDER_OPT_TEST_VULKAN)

// ---- The Vulkan render counters ----

// The draws each frame of the counter check records, and its frames.
constexpr int kDraws = 6;
constexpr int kFrames = 3;

// Draws kDraws full-screen triangles a frame for kFrames frames on the default framebuffer.
void draw_frames(openrac::test::GlContext& context, const openrac::renderer::Shader& shader) {
    using namespace openrac::gl;
    unsigned vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 64, 64);  // the context's size (renderer_gl_context.h)
    glDisable(GL_DEPTH_TEST);
    shader.use();

    // Each frame cleared, drawn and shown, which ends it and submits it.
    for (int frame = 0; frame < kFrames; ++frame) {
        glClear(GL_COLOR_BUFFER_BIT);

        // The same triangle each time: what is counted is the draws, not what they show.
        for (int draw = 0; draw < kDraws; ++draw) {
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        context.window().swap();
    }

    glDeleteVertexArrays(1, &vao);
}

// Prints what the fixture's frames added to the render counters, for a package's report.
void print_counters(
    const openrac::vkgl::SubmissionCounters& before, const openrac::vkgl::SubmissionCounters& after
) {
    // The kinds of command, in the order of vkgl::Command.
    constexpr std::array<const char*, openrac::vkgl::kCommandKinds> kKinds = {
        "bind_pipeline",
        "bind_descriptors",
        "bind_buffers",
        "dynamic_state",
        "draw",
        "barrier",
        "copy",
        "dispatch",
        "render_pass",
        "query",
        "other",
    };
    const auto added = [](std::uint64_t now, std::uint64_t then) {
        return static_cast<unsigned long long>(now - then);
    };
    std::printf(
        "vk_cmds_recorded %llu\n",
        added(openrac::vkgl::commands_recorded(after), openrac::vkgl::commands_recorded(before))
    );

    // Each kind on a line of its own.
    for (std::size_t kind = 0; kind < kKinds.size(); ++kind) {
        std::printf(
            "vk_cmds_%s %llu\n", kKinds[kind], added(after.commands[kind], before.commands[kind])
        );
    }

    std::printf("vk_render_passes %llu\n", added(after.render_passes, before.render_passes));
    std::printf(
        "vk_attachment_load_bytes %llu\n",
        added(after.attachment_load_bytes, before.attachment_load_bytes)
    );
    std::printf(
        "vk_attachment_store_bytes %llu\n",
        added(after.attachment_store_bytes, before.attachment_store_bytes)
    );
    std::printf("vk_barriers %llu\n", added(after.barriers, before.barriers));
    std::printf(
        "vk_barriers_all_commands %llu\n",
        added(after.barriers_all_commands, before.barriers_all_commands)
    );
    std::printf("vk_submits %llu\n", added(after.submits, before.submits));
    std::printf(
        "vk_fullscreen_copies %llu\n", added(after.fullscreen_copies, before.fullscreen_copies)
    );
    std::printf("vk_ring_bytes %llu\n", added(after.ring_bytes, before.ring_bytes));
    std::printf(
        "vk_descriptor_sets_allocated %llu\n",
        added(after.descriptor_sets_allocated, before.descriptor_sets_allocated)
    );
    std::printf(
        "vk_descriptor_writes %llu\n", added(after.descriptor_writes, before.descriptor_writes)
    );
    std::printf("vk_pipelines_created (total) %llu\n", added(after.pipelines_created, 0));
    std::printf("vk_texture_creations (total) %llu\n", added(after.texture_creations, 0));
    std::printf("vk_shader_compiles (total) %llu\n", added(after.shader_compiles, 0));
}

// Tests that after N draws the commands recorded are at least N, and that each frame presented
// without a read-back is one submit; skipped (77) without Vulkan.
int test_the_vulkan_counters_follow_the_draws() {
    namespace vkgl = openrac::vkgl;
    // The shader below is compiled by glslang and its pipeline made by the draw, not taken from
    // the SPIR-V cache or the pre-warm an earlier run left (R-12, R-10): the counters then have a
    // compile and a pipeline to count.
    render_opt::set(Opt::SpirvDiskCache, false);
    render_opt::set(Opt::PipelinePrewarm, false);
    openrac::test::GlContext context;
    std::string error;

    // No Vulkan here (or it fell back to OpenGL): nothing of the backend to count.
    if (!context.open(error) || !vkgl::active()) {
        std::printf("SKIP: Vulkan unavailable: %s\n", error.c_str());
        return openrac::test::kSkip;
    }

    openrac::renderer::Shader shader;
    CHECK(shader.build(
        "render counter fixture",
        "#version 410 core\n"
        "void main() {\n"
        "    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);\n"
        "    gl_Position = vec4(p * 2.0 - 1.0, 0, 1);\n"
        "}\n",
        "#version 410 core\n"
        "layout(location = 0) out vec4 colour;\n"
        "void main() { colour = vec4(0.25, 0.5, 0.75, 1.0); }\n",
        error
    ));
    const vkgl::SubmissionCounters before = vkgl::submission_counters();
    draw_frames(context, shader);
    const vkgl::SubmissionCounters after = vkgl::submission_counters();

    const auto draws = static_cast<std::uint64_t>(kDraws * kFrames);
    const auto frames = static_cast<std::uint64_t>(kFrames);
    const auto draw_kind = static_cast<std::size_t>(vkgl::Command::Draw);
    CHECK(vkgl::commands_recorded(after) - vkgl::commands_recorded(before) >= draws);
    CHECK(after.commands[draw_kind] - before.commands[draw_kind] == draws);
    CHECK(after.frames_presented - before.frames_presented == frames);
    CHECK(after.submits - before.submits == frames);
    CHECK(after.render_passes - before.render_passes >= frames);
    CHECK(after.attachment_store_bytes > before.attachment_store_bytes);
    CHECK(after.pipelines_created >= 1);
    CHECK(after.shader_compiles >= 1);
    CHECK(after.validation_errors == 0);
    print_counters(before, after);
    return openrac::test::result();
}

#endif

}  // namespace

int main(int argc, char** argv) {
    const std::string_view mode = argc > 1 ? argv[1] : "";

    // The variable as ctest sets it for render_opt_environment.
    if (mode == "--environment") {
        return test_the_environment_is_applied();
    }

#if defined(OPENRAC_RENDER_OPT_TEST_VULKAN)
    // The counters, on a Vulkan context of its own.
    if (mode == "--vulkan") {
        return test_the_vulkan_counters_follow_the_draws();
    }
#endif

    test_each_entry_has_a_name_of_its_own();
    test_the_selftest_entry_reads_back();
    test_a_tolerance_other_than_zero_is_not_exact();
    test_a_name_is_parsed();
    test_a_list_is_parsed();
    test_all_is_parsed();
    test_an_unknown_name_is_kept_apart();
    test_set_and_reset();
    return openrac::test::result();
}

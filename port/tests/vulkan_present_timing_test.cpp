// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests that the Vulkan backend opens and presents with each presentation extension of R-09
// switched off (vkgl::Config's present_id, present_wait and fifo_latest_ready; docs/port/
// RENDER_PERF.md "R-09"), and that what it says of the presents' timing follows: no present is
// known without a swapchain, and the calls that ask never block. With present-timing on, as the
// registry's tests switch it.
//
// It opens the backend without a window (the tests' hidden windows have no surface), so the
// device is made without the extensions in every case; the timing a real window gives is the
// maintainers' to read (OPENRAC_PACING=1). Skipped (77) where Vulkan cannot open.

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>

#include "common/log.h"
#include "renderer/render_opt.h"
#include "renderer/vulkan/vkgl.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

namespace {

namespace vkgl = openrac::vkgl;
namespace render_opt = openrac::renderer::render_opt;

// The default framebuffer's size without a window, and the frames each opening presents.
constexpr int kSide = 32;
constexpr int kFrames = 3;

// One way to open the device: which of the three extensions Config allows.
struct Allowed {
    const char* name;  // For the messages.
    bool present_id;
    bool present_wait;
    bool fifo_latest_ready;
};

// Every extension allowed, then each one switched off, then all of them.
constexpr std::array<Allowed, 5> kOpenings = {{
    {"all allowed", true, true, true},
    {"no present ids", false, true, true},
    {"no present waits", true, false, true},
    {"no latest-ready mode", true, true, false},
    {"none allowed", false, false, false},
}};

// Tests that the backend opens as `allowed` says, presents kFrames frames, and reports no present
// time it cannot know; returns false when Vulkan does not open at all.
bool test_opens_and_presents(const Allowed& allowed) {
    vkgl::Config config;
    config.width = kSide;
    config.height = kSide;
    config.present_id = allowed.present_id;
    config.present_wait = allowed.present_wait;
    config.fifo_latest_ready = allowed.fifo_latest_ready;
    std::string error;

    // No Vulkan here: nothing to open, for this way or any other.
    if (!vkgl::open(config, error)) {
        std::printf("%s: Vulkan unavailable: %s\n", allowed.name, error.c_str());
        return false;
    }

    const std::uint64_t before = vkgl::submission_counters().frames_presented;

    // Each frame shown, and the timing asked after it, as the frontend's pacer would.
    for (int frame = 0; frame < kFrames; ++frame) {
        vkgl::present();
        const vkgl::PresentTiming timing = vkgl::present_timing();
        CHECK(!timing.supported);
        CHECK(!timing.known);
    }

    CHECK(vkgl::submission_counters().frames_presented - before == kFrames);
    CHECK(!vkgl::wait_for_present(std::chrono::milliseconds(1)));
    CHECK(vkgl::submission_counters().validation_errors == 0);
    vkgl::close();
    std::printf("%s: opened and presented %d frames\n", allowed.name, kFrames);
    return true;
}

}  // namespace

int main() {
    render_opt::set(render_opt::Opt::PresentTiming, true);

    // Each way of opening the device, one after another.
    for (const Allowed& allowed : kOpenings) {
        // Vulkan absent: the test has nothing to say.
        if (!test_opens_and_presents(allowed)) {
            return openrac::test::kSkip;
        }
    }

    return openrac::test::result();
}

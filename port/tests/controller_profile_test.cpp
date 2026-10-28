// SPDX-License-Identifier: GPL-3.0-or-later
#include <cmath>

#include "platform/pad.h"
#include "platform/switch/controller_profile.h"
#include "tests/check.h"
using namespace openrac;

int main() {
    using namespace platform;
    using C = controller::CaptureState;
    controller::BindingCapture capture;
    CHECK(capture.update(1) == C::ReleaseButtons);  // Initial confirm press is never captured.
    CHECK(capture.update(0) == C::PressButton);
    CHECK(capture.update(2) == C::ReleaseToConfirm);
    CHECK(capture.update(2) == C::ReleaseToConfirm);
    CHECK(capture.update(0) == C::Bound && capture.physical == 1);
    capture = {};
    capture.update(0);
    capture.update(1u << 8);
    CHECK(capture.update((1u << 8) | (1u << 9)) == C::Cancelled);
    capture = {};
    capture.update(0);
    CHECK(capture.update(3) == C::MultipleButtons);
    CHECK(capture.update(0) == C::PressButton);
    CHECK(capture.update(4) == C::ReleaseToConfirm);
    CHECK(capture.update(0) == C::Bound && capture.physical == 2);
    controller::Profile p;
    const std::array<int, 16> original = {9, 10, 11, 8, 12, 13, 14, 15, 6, 7, 4, 5, 2, 0, 1, 3};
    for (unsigned action = 0; action < 16; ++action) {
        CHECK(
            controller::buttons(p, std::uint16_t(1u << original[action]))
            == std::uint16_t(~(1u << action))
        );
        CHECK(
            controller::physical_from_key(controller::binding_key(original[action]))
            == original[action]
        );
    }
    controller::bind(p, 14, 0);  // Jump to A swaps Shoot onto B.
    CHECK(p.buttons[14] == 0 && p.buttons[13] == 1);
    CHECK(controller::buttons(p, 1) == std::uint16_t(~(1u << 14)));
    controller::bind(p, 14, -1);
    CHECK(controller::buttons(p, 1) == 0xffff);
    CHECK(controller::buttons(p, 2) == std::uint16_t(~(1u << 13)));
    p = {};
    controller::bind(p, 8, 4);  // Lock-strafe follows remapped L2, not physical ZL.
    CHECK(p.buttons[8] == 4 && p.buttons[10] == 6);
    CHECK(controller::buttons(p, 1u << 4) == std::uint16_t(~(1u << 8)));
    p = {};
    p.buttons[14] = p.buttons[0];
    p.camera_x = 900;
    p.camera_deadzone = 90;
    controller::sanitize(p);
    CHECK(p.buttons[14] == -1 && p.camera_x == 200 && p.camera_deadzone == 30);
    p = {};
    p.camera_x = 150;
    p.camera_y = 50;
    p.invert_x = p.invert_y = true;
    auto axes = controller::sticks(p, {.25f, -.25f, .5f, -.5f});
    CHECK(axes[0] == .25f && axes[1] == -.25f && axes[2] == -.75f && axes[3] == .25f);
    p = {};
    p.swap_sticks = true;
    axes = controller::sticks(p, {.2f, .3f, .4f, .5f});
    CHECK((axes == std::array<float, 4>({.4f, .5f, .2f, .3f})));
    p = {};
    p.movement_deadzone = 15;
    p.camera_deadzone = 20;
    axes = controller::sticks(p, {.04f, .05f, .08f, .09f});
    CHECK((axes == std::array<float, 4>({0, 0, 0, 0})));
    // Every full-rim direction must retain running speed after the new deadzone
    // adjustment and the established circular-to-square gate correction.
    for (int zone : {0, 10, 30}) {
        p = {};
        p.movement_deadzone = zone;
        StickGate gate;
        for (int angle = 0; angle < 360; ++angle) {
            double radians = angle * 3.141592653589793 / 180;
            axes =
                controller::sticks(p, {float(std::cos(radians)), float(std::sin(radians)), 0, 0});
            const auto axis = [](float v) {
                return std::int16_t(std::clamp(std::lround(v * 32768), -32768L, 32767L));
            };
            std::uint8_t x, y;
            stick_bytes(axis(axes[0]), axis(axes[1]), gate, x, y);
            CHECK(std::max(std::abs(int(x) - 128), std::abs(int(y) - 128)) >= 126);
        }
    }
    return test::result();
}

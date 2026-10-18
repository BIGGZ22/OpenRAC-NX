// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The on-screen pad (platform/touch_pad.h): fingers on its controls, as SDL reports them, become
// the pad's buttons and sticks.

#include <cmath>
#include <cstdio>

#include "check.h"
#include "platform/touch_pad.h"

namespace {

using namespace openrac::platform;

const TouchControl& control(const TouchPad& touch, TouchControl::Kind kind, Button button = Button::Cross) {
    for (const TouchControl& c : touch.controls()) {
        if (c.kind == kind && (kind != TouchControl::Kind::Button || c.button == button)) {
            return c;
        }
    }
    std::fprintf(stderr, "no such control\n");
    return touch.controls()[0];
}

Pad read(const TouchPad& touch) {
    Pad pad;
    touch.apply(pad);
    return pad;
}

}  // namespace

int main() {
    TouchPad touch;
    touch.set_aspect(16.0f / 9.0f);
    CHECK(!touch.visible());
    CHECK(read(touch).buttons == Pad{}.buttons);

    // Cross, held and let go.
    const TouchControl& cross = control(touch, TouchControl::Kind::Button, Button::Cross);
    touch.finger_down(1, cross.x, cross.y);
    CHECK(touch.visible());
    CHECK(read(touch).held(Button::Cross));
    CHECK(!read(touch).held(Button::Triangle));
    touch.finger_up(1);
    CHECK(!read(touch).held(Button::Cross));

    // The left stick pushed right to its edge and beyond: full right, centre vertically.
    const TouchControl& left = control(touch, TouchControl::Kind::LeftStick);
    touch.finger_down(2, left.x, left.y);
    touch.finger_move(2, left.x + 0.5f, left.y);
    Pad pad = read(touch);
    CHECK(pad.left_x == 0xFF);
    CHECK(pad.left_y >= 0x7F && pad.left_y <= 0x81);
    CHECK(pad.right_x == kStickCentre);
    // A second finger on Square while the stick is held: both count.
    const TouchControl& square = control(touch, TouchControl::Kind::Button, Button::Square);
    touch.finger_down(3, square.x, square.y);
    pad = read(touch);
    CHECK(pad.held(Button::Square));
    CHECK(pad.left_x == 0xFF);
    // The stick's finger dragged up over the d-pad keeps the stick (full up), presses no button.
    touch.finger_move(2, left.x, 0.0f);
    pad = read(touch);
    CHECK(pad.left_y == 0x00);
    CHECK(!pad.held(Button::Up));

    // Everything lifted (the app went to the background).
    touch.release_all();
    pad = read(touch);
    CHECK(pad.left_x == kStickCentre && pad.left_y == kStickCentre);
    CHECK(!pad.held(Button::Square));

    // Away from every control: nothing.
    touch.finger_down(4, 0.5f, 0.5f);
    CHECK(read(touch).buttons == Pad{}.buttons);
    touch.hide();
    CHECK(!touch.visible());

    // A thumb in the lower left, away from the stick, brings the stick to it; lifted, it goes home.
    TouchPad floating;
    floating.set_aspect(2.0f);
    const TouchControl& stick = control(floating, TouchControl::Kind::LeftStick);
    const float home_x = stick.x;
    const float home_y = stick.y;
    floating.finger_down(7, 0.32f, 0.55f);
    CHECK(stick.x == 0.32f && stick.y == 0.55f);
    floating.finger_move(7, 0.32f, 0.40f);
    CHECK(read(floating).left_y < 0x40);
    floating.finger_up(7);
    CHECK(stick.x == home_x && stick.y == home_y);
    // In the middle of the screen it does not.
    floating.finger_down(8, 0.6f, 0.6f);
    CHECK(stick.x == home_x && read(floating).left_x == kStickCentre);
    floating.finger_up(8);

    // The controls keep their distance from the edges, inside the safe area.
    const TouchControl& circle = control(floating, TouchControl::Kind::Button, Button::Circle);
    const float circle_x = circle.x;
    floating.set_safe_area(0.0f, 0.1f);
    CHECK(std::abs((circle_x - circle.x) * 2.0f - 0.1f) < 1e-5f);
    floating.set_aspect(2.4f);
    CHECK(std::abs((1.0f - circle.x) * 2.4f - (1.0f - circle_x) * 2.0f - 0.1f) < 1e-5f);

    // Left alone, the pad goes away; a tap that lifts within the frame still counts as a touch.
    TouchPad idle;
    idle.idle(1000);
    CHECK(!idle.visible());
    idle.finger_down(5, 0.5f, 0.5f);
    idle.finger_up(5);
    idle.idle(50000);
    CHECK(idle.visible());
    idle.idle(50000 + TouchPad::kIdleHideMs - 1);
    CHECK(idle.visible());
    idle.idle(50000 + TouchPad::kIdleHideMs);
    CHECK(!idle.visible());
    // A finger held still keeps it.
    idle.finger_down(6, 0.5f, 0.5f);
    idle.idle(90000);
    idle.idle(200000);
    CHECK(idle.visible());
    return openrac::test::result();
}

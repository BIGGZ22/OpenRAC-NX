// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The on-screen pad (platform/touch_pad.h): fingers on its controls, as SDL reports them, become
// the pad's buttons and sticks.

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
    return openrac::test::result();
}

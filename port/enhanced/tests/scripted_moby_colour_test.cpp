// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/game/scripted_moby_colour.h"
#include "tests/check.h"

int main() {
    openrac::ScriptedMobyColours colours;
    CHECK(!colours.matches(10, 0x1000, 0x808080));
    colours.set(10, 0x1000, 0x808080);  // energy sphere
    CHECK(colours.matches(10, 0x1000, 0x808080));
    CHECK(!colours.matches(10, 0x2000, 0x808080));  // other objects stay on GI
    colours.set(10, 0x2000, 0x4040c8);              // crate red flash
    CHECK(colours.matches(10, 0x2000, 0x4040c8));
    CHECK(!colours.matches(10, 0x2000, 0x292424));  // saved ambient restored
    colours.set(10, 0x3000, 0xc0c0c0);              // swingshot target
    CHECK(colours.matches(10, 0x3000, 0xc0c0c0));
    CHECK(!colours.matches(3, 0x3000, 0xc0c0c0));
    colours.set(3, 0x4000, 0x808080);  // level change clears old addresses
    CHECK(!colours.matches(10, 0x1000, 0x808080));
    CHECK(colours.matches(3, 0x4000, 0xff808080));  // alpha is not RGB
    return openrac::test::result();
}

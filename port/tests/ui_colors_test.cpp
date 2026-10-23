// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The launcher's colours for the drawer (game/common/ui_colors.h): what OPENRAC_UI_COLORS sets, as
// the launcher writes it (launcher/core/src/theme.rs), and what it leaves at the launcher's own.

#include "check.h"
#include "game/common/ui_colors.h"

namespace {

using namespace openrac::ui_colors;

void reads_the_launchers_colours() {
    const Colors own;
    CHECK(own.panel == rgba(0x1d, 0x1d, 0x28));
    const Colors nord = parse("panel=#3b4252,line=#d8dee938,amber=#FFD23F");
    CHECK(nord.panel == rgba(0x3b, 0x42, 0x52));
    CHECK(nord.line == rgba(0xd8, 0xde, 0xe9, 0x38));
    CHECK(nord.amber == rgba(0xff, 0xd2, 0x3f));
    CHECK(nord.text == own.text);  // left as it was
    // What it cannot read keeps the launcher's own: a name it does not know, a value that is not
    // a colour, a pair without a value.
    const Colors odd = parse("glow=#ffffff,panel=red,text=#12345,soft,dim=#12345678,,");
    CHECK(odd.panel == own.panel && odd.text == own.text && odd.soft == own.soft);
    CHECK(odd.dim == rgba(0x12, 0x34, 0x56, 0x78));
    CHECK(parse("").panel == own.panel);
}

void mixes_and_fades() {
    const Color black = rgba(0, 0, 0);
    const Color white = rgba(255, 255, 255);
    CHECK(mix(black, white, 0.0f) == black);
    CHECK(mix(black, white, 1.0f) == white);
    CHECK(mix(black, white, 0.5f) == rgba(128, 128, 128));
    CHECK(mix(black, white, 9.0f) == white);
    CHECK(fade(white, 0.5f) == rgba(255, 255, 255, 128));
    CHECK(fade(white, 0.0f) == rgba(255, 255, 255, 0));
}

}  // namespace

int main() {
    reads_the_launchers_colours();
    mixes_and_fades();
    return openrac::test::result();
}

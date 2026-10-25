// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The launcher's colours in the game: the options drawer (debug/drawer.cpp) wears the theme the
// player picked in the launcher, and on Android the picked game's accent, as the launcher does. The
// launcher hands them over in OPENRAC_UI_COLORS (launcher/core/src/theme.rs), by app.css's names;
// a game started without it, or a name it does not hold, keeps the launcher's own (app.css's
// :root, the defaults below).

#pragma once

#include <cstdint>
#include <string_view>

namespace openrac::ui_colors {

// A colour as Dear ImGui's IM_COL32 packs it: red in the low byte, alpha in the high one.
using Color = std::uint32_t;

constexpr Color rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
    return static_cast<Color>(r) | static_cast<Color>(g) << 8 | static_cast<Color>(b) << 16
           | static_cast<Color>(a) << 24;
}

// app.css's tokens the drawer draws with, under their names there.
struct Colors {
    Color ink_deep = rgba(0x12, 0x12, 0x1a);      // --ink-deep: under everything; a switch's track
    Color panel = rgba(0x1d, 0x1d, 0x28);         // --panel: the drawer
    Color panel_hi = rgba(0x22, 0x22, 0x2f);      // --panel-hi: its cards
    Color brand = rgba(0xd5, 0x8a, 0x00);         // --brand: a switch that is on
    Color bar_top = rgba(0xe5, 0xa0, 0x13);       // --bar-top, --bar-bottom: a main button's
    Color bar_bottom = rgba(0xcd, 0x83, 0x00);    //   gradient
    Color bar_ink = rgba(0x1b, 0x12, 0x06);       // --bar-ink-deep: the text on it
    Color amber = rgba(0xff, 0xb4, 0x43);         // --amber: the focus, the arrows, the logo's mark
    Color gold = rgba(0xff, 0xd0, 0x8a);          // --gold: values, headings
    Color text = rgba(0xe8, 0xf0, 0xff);          // --text
    Color soft = rgba(0xaa, 0xb6, 0xdc);          // --soft: what is said under or beside
    Color dim = rgba(0x8f, 0x9a, 0xc4);           // --dim: a switch's knob when off, small print
    Color line = rgba(156, 161, 212, 56);         // --line: hairlines and borders
    Color line_strong = rgba(156, 161, 212, 89);  // --line-strong
    Color err = rgba(0xff, 0x8a, 0x7a);           // --err: Quit
};

// `text` read over the defaults: NAME=#RRGGBB or NAME=#RRGGBBAA pairs, comma-separated, each NAME
// app.css's without its leading dashes ("panel-hi=#22222f,line=#9ca1d438").
Colors parse(std::string_view text);

// The colours this run was started with (OPENRAC_UI_COLORS), read once.
const Colors& launcher();

// `a` and `b` mixed, `t` (0..1) of the way to `b`, alpha too.
Color mix(Color a, Color b, float t);

// `c` with its alpha multiplied by `t` (0..1): a colour fading in or out.
Color fade(Color c, float t);

}  // namespace openrac::ui_colors

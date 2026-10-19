// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Which pause menu page is its Options page (game/common/pause_options.h): the page whose state is
// the Options page's, and no other, with the words seen on Novalis (PAL): an item grid page, whose
// focus keeps 0.08 at +0x34, the Options page, and the port's Video page (a copy of Camera's).

#include <cstdint>
#include <map>

#include "check.h"
#include "game/common/pause_options.h"

namespace {

using openrac::frontend::pause_options::options_list;

std::map<std::uint32_t, std::uint32_t> memory;

std::uint32_t read(std::uint32_t at) {
    const auto it = memory.find(at);
    return it != memory.end() ? it->second : 0;
}

void page(std::uint32_t at, std::uint32_t back, std::int32_t state, std::uint32_t focus) {
    memory[at + 0x38] = back;
    memory[at + 0x3C] = static_cast<std::uint32_t>(state);
    memory[at + 0x40] = focus;
}

}  // namespace

int main() {
    constexpr std::uint32_t kRoot = 0x1B2D08, kGrid = 0x1B3538, kOptions = 0x1B50D0, kVideo = 0xE0200210;
    page(kRoot, 0, 2, 0x1B2E38);
    page(kGrid, kRoot, 3, 0x1B36D8);
    memory[0x1B36D8 + 0x34] = 0x3DA3D70Au;  // the grid's margin, 0.08
    page(kOptions, kRoot, 8, 0x1B5278);
    memory[0x1B5278 + 0x34] = 0x1B5218u;
    page(kVideo, kOptions, 13, 0xE0200300);

    CHECK(options_list(read, kOptions) == 0x1B5278u);
    CHECK(options_list(read, kGrid) == 0);   // pause pages 0 to 3: the crash was reading this focus's list
    CHECK(options_list(read, kRoot) == 0);   // the pause menu's own page
    CHECK(options_list(read, kVideo) == 0);  // the port's Video page is not the Options page
    CHECK(options_list(read, 0) == 0);       // no page
    return openrac::test::result();
}

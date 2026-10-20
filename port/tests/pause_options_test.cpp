// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Which pause menu page is its Options page (game/common/pause_options.h): the page whose state is
// the Options page's, and no other, with the words seen on Novalis (PAL): an item grid page, whose
// focus keeps 0.08 at +0x34, the Options page, and the port's Video page (a copy of Camera's). Then
// the Options page's picture stream, found among its screens, and its entries with Video's.

#include <cstdint>
#include <map>

#include "check.h"
#include "game/common/pause_options.h"

namespace {

using openrac::frontend::pause_options::entries_with_video;
using openrac::frontend::pause_options::options_list;
using openrac::frontend::pause_options::options_stream;

std::map<std::uint32_t, std::uint32_t> memory;

std::uint32_t read_word(std::uint32_t at) {
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

    CHECK(options_list(read_word, kOptions) == 0x1B5278u);
    CHECK(options_list(read_word, kGrid) == 0);   // pause pages 0 to 3: the crash was reading this focus's list
    CHECK(options_list(read_word, kRoot) == 0);   // the pause menu's own page
    CHECK(options_list(read_word, kVideo) == 0);  // the port's Video page is not the Options page
    CHECK(options_list(read_word, 0) == 0);       // no page

    // The Options page's screens as on Rilgar (PAL): the hint box (+0x30 its flags 0x90), the picture
    // stream (+0x30 its entries, flags 0x20, two buffers) and the list (+0x30 its flags 0x1000).
    constexpr std::uint32_t kHint = 0x1B5368, kStream = 0x1B5230, kList = 0x1B52F8, kEntries = 0x138140;
    memory[kOptions + 0x44] = kHint;
    memory[kOptions + 0x48] = kStream;
    memory[kOptions + 0x4C] = kList;
    memory[kHint + 0x30] = 0x90;
    memory[kHint + 0x34] = 0x1B5348;
    memory[kList + 0x30] = 0x1000;
    memory[kList + 0x34] = 0x1B5298;
    memory[kStream + 0x30] = kEntries;
    memory[kStream + 0x34] = 0x20;
    memory[kStream + 0x48] = 0x3D0000;
    memory[kStream + 0x4C] = 0x3E1800;
    // HelpDesk, Save, Load, Sound, Camera, Subtitles, Quit Game; then the next table's words, which
    // entries[7] read: 446 sectors into a picture buffer.
    const std::uint32_t game[] = {0x1359, 7, 0x1360, 7, 0x1367, 7, 0x136E, 6, 0x1374, 7, 0x137B, 8, 0x1383, 6, 0x1389, 0x1BE};
    for (std::uint32_t i = 0; i < 16; ++i) {
        memory[kEntries + 4 * i] = game[i];
    }
    CHECK(options_stream(read_word, kOptions, kList, 7) == kStream);
    CHECK(options_stream(read_word, kOptions, kList, 0) == 0);
    CHECK(options_stream(read_word, kGrid, 0x1B36D8, 7) == 0);
    memory[kStream + 0x34] = 0x24;  // a grid cell's picture: not the list's index
    CHECK(options_stream(read_word, kOptions, kList, 7) == 0);
    memory[kStream + 0x34] = 0x20;

    // Video after Sound (the list's fifth item): an empty entry there, Camera's picture at Camera's
    // place, and Quit Game's the last of eight, none read past the table.
    std::uint32_t with[16] = {};
    entries_with_video(game, 7, 4, with);
    const std::uint32_t want[] = {0x1359, 7, 0x1360, 7, 0x1367, 7, 0x136E, 6, 0, 0, 0x1374, 7, 0x137B, 8, 0x1383, 6};
    for (int i = 0; i < 16; ++i) {
        CHECK(with[i] == want[i]);
    }
    // The table made from is found no more: its fifth entry has no sectors.
    for (std::uint32_t i = 0; i < 16; ++i) {
        memory[kEntries + 4 * i] = with[i];
    }
    CHECK(options_stream(read_word, kOptions, kList, 7) == 0);
    // At the end: after every item.
    std::uint32_t last[16] = {};
    entries_with_video(game, 7, 7, last);
    CHECK(last[12] == 0x1383 && last[13] == 6 && last[14] == 0 && last[15] == 0);
    return openrac::test::result();
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The OpenRAC logo as a coverage mask made on the CPU, for the overlays that show it (the
// on-screen pad's options button, the drawer's header). Its paths are the launcher's
// (launcher/src/components/Logo.svelte): the mark, the lombax's head and the "O" it makes, and the
// name, "pen" and "RAC". No GL here: each caller uploads the mask its own way.
#pragma once

#include <cstdint>
#include <vector>

namespace openrac::renderer::ui::logo {

// A logo drawn `height` pixels high, its first row the top: each byte is how much of that pixel
// the logo covers, 0 to 255. Its box is what the logo draws, the lombax's ears included, with a
// pixel of margin around it.
struct Mask {
    int width = 0;
    int height = 0;
    int name_x = 0;  // where the name starts, in pixels from the left; `width` for the mark alone
    std::vector<std::uint8_t> alpha;
};

// The mark alone, or the whole logo with its name.
Mask draw(int height, bool with_name);

}  // namespace openrac::renderer::ui::logo

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The on-screen pad (platform/touch_pad.h) drawn over the window, while it is visible: a
// glass for each control: the face buttons with their symbols in their colours, the shoulders
// with their names, the d-pad on its plate, the sticks with their knobs where the thumbs hold them.
#pragma once

#include "platform/touch_pad.h"

namespace openrac::renderer::ui::touch_overlay {

// Draws into the window's framebuffer, width x height pixels (the context is current), and tells
// the pad the window's shape.
void draw(platform::TouchPad& touch, int width, int height);

// Frees its GL objects (the context is still current).
void release();

}  // namespace openrac::renderer::ui::touch_overlay

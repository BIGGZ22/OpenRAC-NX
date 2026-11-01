// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace openrac::platform::overlay_stats {
// Before SDL/audio worker threads start: handles loaders with only one port session.
void initialize();
// Called on the presentation thread after an actual buffer swap. Render dimensions and
// drawable dimensions are kept distinct (Status Monitor's render/viewport lists).
void presented(int render_width, int render_height, int output_width, int output_height);
// ReverseNX-RT profile selection, falling back to the physical operating mode.
bool docked();
void shutdown();
}

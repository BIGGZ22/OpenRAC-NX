// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/window.h"
#include "platform/pad.h"
#include "platform/audio.h"

namespace openrac::platform {

Platform::Platform(unsigned subsystems) {
  // Initialize for Switch platform - headless operation
}

Platform::~Platform() {
  // Cleanup for Switch platform
}

void Platform::use_offscreen_video() {
  // No-op for Switch - headless build
}

}  // namespace openrac::platform
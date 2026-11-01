// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#pragma once

#include "platform/window.h"

namespace openrac::platform {

// Switch-specific implementation of window/platform functionality for headless builds
class Platform {
public:
    explicit Platform(unsigned subsystems);
    ~Platform();
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;

    bool ok() const { return m_ok; }

    const std::string& error() const { return m_error; }

    // Before constructing a Platform: draw without a display, through SDL's
    // offscreen driver (EGL; Mesa's llvmpipe in a container). Used by tests
    // and by the viewer's screenshot mode when no display server is running.
    static void use_offscreen_video();

    // True when a display server is reachable (on Linux, $DISPLAY or
    // Wayland) or on macOS it is not necessary. For the Switch, this always 
    // returns false since we don't have a windowing system in headless mode.
    static bool display_available() { return false; }

    // The displays connected now, in SDL's order (the first is the primary one). Needs a Platform.
    // Returns empty list for console platform.
    static std::vector<Display> displays() { return {}; }
    
private:
    bool m_ok = true;
    std::string m_error;
};

}  // namespace openrac::platform
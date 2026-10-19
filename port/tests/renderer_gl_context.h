// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// An OpenGL 4.1 core context for the tests that draw: a hidden window through
// SDL's offscreen driver (EGL; Mesa's llvmpipe on a machine without a GPU).
// When none can be made, a test prints why and exits with kSkip, which CTest
// reports as skipped (SKIP_RETURN_CODE in port/cmake/Window.cmake).

#pragma once

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#include "platform/window.h"
#include "renderer/gl.h"

namespace openrac::test {

inline constexpr int kSkip = 77;

class GlContext {
public:
    bool open(std::string& why, int width = 64, int height = 64) {
        // The offscreen driver unless the caller chose one.
        const char* driver = std::getenv("SDL_VIDEO_DRIVER");
        if (driver == nullptr || driver[0] == '\0') {
            platform::Platform::use_offscreen_video();
        }
        m_sdl = std::make_unique<platform::Platform>(platform::kVideo);
        if (!m_sdl->ok()) {
            why = m_sdl->error();
            return false;
        }
        platform::WindowConfig config;
        config.title = "OpenRAC test";
        config.width = width;
        config.height = height;
        config.hidden = true;
        config.vsync = false;
        config.resizable = false;
        m_window = platform::Window::open(config, why);
        if (!m_window) {
            return false;
        }
        std::string missing;
        if (!gl::load(platform::Window::gl_loader(), missing)) {
            why = "OpenGL functions missing: " + missing;
            return false;
        }
        std::printf(
            "OpenGL %s on %s\n", m_window->gl_version().c_str(), m_window->gl_renderer().c_str()
        );
        return true;
    }

    platform::Window& window() { return *m_window; }

private:
    std::unique_ptr<platform::Platform> m_sdl;
    std::unique_ptr<platform::Window> m_window;  // destroyed before m_sdl
};

}  // namespace openrac::test

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/window.h"

#include <cstdlib>
#include <format>

#include <SDL3/SDL.h>

#include "common/log.h"

namespace openrac::platform {
namespace {

constexpr int kGlMajor = 4;
constexpr int kGlMinor = 1;

// Read a GL string without the renderer's loader: the platform layer only
// needs it for the log.
std::string gl_string(unsigned name) {
    using GetString = const unsigned char* (*)(unsigned);
    auto get = reinterpret_cast<GetString>(SDL_GL_GetProcAddress("glGetString"));
    if (get == nullptr) {
        return {};
    }
    const unsigned char* text = get(name);
    return text != nullptr ? std::string(reinterpret_cast<const char*>(text)) : std::string();
}

constexpr unsigned kGlVersion = 0x1F02;
constexpr unsigned kGlRenderer = 0x1F01;

}  // namespace

void set_app_metadata(const char* name, const char* id) {
    SDL_SetAppMetadata(name, nullptr, id);
}

Platform::Platform(unsigned subsystems) {
    SDL_InitFlags flags = 0;
    if ((subsystems & kVideo) != 0) {
        flags |= SDL_INIT_VIDEO;
    }
    if ((subsystems & kGamepad) != 0) {
        flags |= SDL_INIT_GAMEPAD;
    }
    if ((subsystems & kAudio) != 0) {
        flags |= SDL_INIT_AUDIO;
    }
    m_ok = SDL_Init(flags);
    if (!m_ok) {
        m_error = std::format("SDL could not start: {}", SDL_GetError());
    }
}

Platform::~Platform() {
    SDL_Quit();
}

void Platform::use_offscreen_video() {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
}

bool Platform::display_available() {
#if defined(__linux__) || defined(__FreeBSD__)
    const char* x11 = std::getenv("DISPLAY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    return (x11 != nullptr && x11[0] != '\0') || (wayland != nullptr && wayland[0] != '\0');
#else
    return true;
#endif
}

void Window::set_icon(const unsigned char* rgba, int width, int height) {
    SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32,
                                                 const_cast<unsigned char*>(rgba), width * 4);
    if (surface == nullptr) {
        return;
    }
    SDL_SetWindowIcon(m_window, surface);
    SDL_DestroySurface(surface);
}

std::unique_ptr<Window> Window::open(const WindowConfig& config, std::string& error) {
    // Context attributes must be set before the window exists.
    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, kGlMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, kGlMinor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // macOS only gives a core context to a forward-compatible request.
    int context_flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
    if (config.debug_context) {
        context_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    if (config.msaa_samples > 1) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, config.msaa_samples);
    }

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (config.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }
    if (config.hidden) {
        flags |= SDL_WINDOW_HIDDEN;
    }

    // Without OpenGL 4.1 there is no game to show: say so in a message box
    // too, since a player starting from the launcher sees no console.
    auto no_opengl = [&](SDL_Window* parent) {
        error = std::format(
            "OpenGL {}.{} (core profile) is not available: {}. OpenRAC draws with OpenGL {}.{}; "
            "update the graphics driver, or check that the graphics card supports it.",
            kGlMajor,
            kGlMinor,
            SDL_GetError(),
            kGlMajor,
            kGlMinor
        );
        if (!config.hidden) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenRAC", error.c_str(), parent);
        }
    };

    std::unique_ptr<Window> window(new Window());
    window->m_window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
    if (window->m_window == nullptr) {
        // SDL fails here when the video driver has no OpenGL at all.
        no_opengl(nullptr);
        return nullptr;
    }
    SDL_GLContext context = SDL_GL_CreateContext(window->m_window);
    if (context == nullptr) {
        no_opengl(window->m_window);
        return nullptr;
    }
    window->m_context = context;
    if (!SDL_GL_MakeCurrent(window->m_window, context)) {
        error = std::format("could not use the OpenGL context: {}", SDL_GetError());
        return nullptr;
    }
    window->m_gl_version = gl_string(kGlVersion);
    window->m_gl_renderer = gl_string(kGlRenderer);
    log::info("OpenGL {} on {}", window->m_gl_version, window->m_gl_renderer);

    window->set_vsync(config.vsync);
    if (config.mode != DisplayMode::Windowed) {
        window->set_display_mode(config.mode, config.fullscreen_width, config.fullscreen_height);
    }
    return window;
}

Window::~Window() {
    if (m_context != nullptr) {
        SDL_GL_DestroyContext(m_context);
    }
    if (m_window != nullptr) {
        SDL_DestroyWindow(m_window);
    }
}

bool Window::set_display_mode(DisplayMode mode, int width, int height) {
    bool ok = true;
    switch (mode) {
        case DisplayMode::Windowed:
            ok = SDL_SetWindowFullscreen(m_window, false);
            break;
        case DisplayMode::Borderless:
            // A null mode is SDL's "fullscreen desktop": no mode change.
            ok = SDL_SetWindowFullscreenMode(m_window, nullptr)
                 && SDL_SetWindowFullscreen(m_window, true);
            break;
        case DisplayMode::Fullscreen: {
            SDL_DisplayID display = SDL_GetDisplayForWindow(m_window);
            SDL_DisplayMode chosen{};
            const SDL_DisplayMode* desktop = SDL_GetDesktopDisplayMode(display);
            if (width <= 0 || height <= 0) {
                if (desktop != nullptr) {
                    chosen = *desktop;
                }
            } else if (!SDL_GetClosestFullscreenDisplayMode(
                           display, width, height, 0.0f, true, &chosen
                       )) {
                log::warn(
                    "no {}x{} display mode ({}); using the desktop's", width, height, SDL_GetError()
                );
                if (desktop != nullptr) {
                    chosen = *desktop;
                }
            }
            ok = SDL_SetWindowFullscreenMode(m_window, chosen.w > 0 ? &chosen : nullptr)
                 && SDL_SetWindowFullscreen(m_window, true);
            break;
        }
    }
    SDL_SyncWindow(m_window);
    if (!ok) {
        log::warn("could not change the display mode: {}", SDL_GetError());
        return false;
    }
    m_mode = mode;
    return true;
}

bool Window::set_vsync(bool on) {
    if (!on) {
        return SDL_GL_SetSwapInterval(0);
    }
    if (SDL_GL_SetSwapInterval(-1)) {
        return true;
    }
    return SDL_GL_SetSwapInterval(1);
}

void Window::swap() {
    SDL_GL_SwapWindow(m_window);
}

void Window::drawable_size(int& width, int& height) const {
    if (!SDL_GetWindowSizeInPixels(m_window, &width, &height)) {
        width = 0;
        height = 0;
    }
}

GlLoader Window::gl_loader() {
    return SDL_GL_GetProcAddress;
}

}  // namespace openrac::platform

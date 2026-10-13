// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// SDL3 and the game's window: an OpenGL 4.1 core context (the version macOS
// stops at, and all OpenGOAL's renderer needs), windowed, fullscreen at a
// chosen resolution, or borderless at the desktop's, with vsync on or off.
//
// The renderer never sees SDL: it gets the context's function loader
// (gl_loader) and the drawable size, and draws into whatever is current.

#pragma once

#include <memory>
#include <string>

struct SDL_Window;
struct SDL_GLContextState;

namespace openrac::platform {

// Which of SDL's subsystems a program uses.
enum Subsystem : unsigned {
    kVideo = 1u << 0,
    kGamepad = 1u << 1,
    kAudio = 1u << 2,
};

// SDL for the life of the program: SDL_Init in the constructor, SDL_Quit in
// the destructor. Only one may exist at a time.
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
    // $WAYLAND_DISPLAY is set; elsewhere always).
    static bool display_available();

private:
    bool m_ok = false;
    std::string m_error;
};

enum class DisplayMode {
    Windowed,
    Fullscreen,  // exclusive, at WindowConfig's fullscreen resolution
    Borderless,  // a window covering the desktop, at its resolution
};

// The program's name and id for the desktop, before the Platform starts SDL: on Wayland the id is
// the window's app id, by which the desktop finds the program's icon and groups its windows.
void set_app_metadata(const char* name, const char* id);

struct WindowConfig {
    std::string title = "OpenRAC";
    // The client area when windowed. The PAL picture is 512 x 448 with
    // non-square pixels shown at 4:3; 1280 x 960 is a 4:3 default.
    int width = 1280;
    int height = 960;
    DisplayMode mode = DisplayMode::Windowed;
    // For DisplayMode::Fullscreen; 0 x 0 uses the desktop's mode.
    int fullscreen_width = 0;
    int fullscreen_height = 0;
    bool vsync = true;
    bool resizable = true;
    bool hidden = false;  // never shown: headless checks
    bool debug_context = false;
    int msaa_samples = 0;
};

// The context's function loader, in SDL_GL_GetProcAddress's shape: what
// renderer/gl.h's load() takes.
using GlProc = void (*)();
using GlLoader = GlProc (*)(const char* name);

class Window {
public:
    // Opens the window and its context, and makes the context current. On
    // failure returns null and says why in `error`; when the reason is that
    // OpenGL 4.1 is not available, also shows a message box (unless hidden),
    // since a player starting the game from a launcher sees no console.
    static std::unique_ptr<Window> open(const WindowConfig& config, std::string& error);

    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool set_display_mode(DisplayMode mode, int width = 0, int height = 0);

    DisplayMode display_mode() const { return m_mode; }

    // Vsync: adaptive (late frames tear rather than wait) when the driver has
    // it, otherwise plain. Returns false if the driver refused both.
    bool set_vsync(bool on);

    // The window's icon: `rgba` is width x height pixels, 4 bytes each, top row first.
    void set_icon(const unsigned char* rgba, int width, int height);
    void swap();

    // The size to render at, in pixels (larger than the window's size on a
    // high-density display).
    void drawable_size(int& width, int& height) const;

    static GlLoader gl_loader();

    // GL_VERSION and GL_RENDERER of the context, for the log.
    const std::string& gl_version() const { return m_gl_version; }

    const std::string& gl_renderer() const { return m_gl_renderer; }

    SDL_Window* sdl() const { return m_window; }

private:
    Window() = default;

    SDL_Window* m_window = nullptr;
    SDL_GLContextState* m_context = nullptr;
    DisplayMode m_mode = DisplayMode::Windowed;
    std::string m_gl_version;
    std::string m_gl_renderer;
};

}  // namespace openrac::platform

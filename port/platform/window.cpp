// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/window.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <string_view>

#include <SDL3/SDL.h>

#include "common/log.h"

namespace openrac::platform {
namespace {

constexpr int kGlMajor = 4;
constexpr int kGlMinor = 1;
constexpr int kGlesMajor = 3;
constexpr int kGlesMinor = 2;

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
constexpr unsigned kGlVendor = 0x1F00;
constexpr unsigned kGlShadingLanguageVersion = 0x8B8C;
constexpr unsigned kGlFramebufferSrgb = 0x8DB9;

// SDL's id of a display counted from 1 as displays() lists them; 0 when there is no such display.
SDL_DisplayID display_id(int display) {
    int count = 0;
    SDL_DisplayID* ids = SDL_GetDisplays(&count);
    SDL_DisplayID id = 0;
    if (ids != nullptr && display >= 1 && display <= count) {
        id = ids[display - 1];
    }
    SDL_free(ids);
    return id;
}

}  // namespace

#if defined(_WIN32)
// A laptop with two graphics cards runs a program on the slow one unless the program exports these:
// NVIDIA's Optimus and AMD's switchable graphics drivers look for them in the executable (their
// public developer notes; OpenGOAL exports the same two, game/main.cpp).
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

void prefer_discrete_gpu() {
#if defined(__linux__) || defined(__FreeBSD__)
    if (const char* gpu = std::getenv("OPENRAC_GPU"); gpu != nullptr && std::string(gpu) == "integrated") {
        log::info("graphics card: the system's default (OPENRAC_GPU=integrated)");
        return;
    }
    // Mesa: the second card of a PRIME pair (with one card it changes nothing). A value the
    // player set stays.
    setenv("DRI_PRIME", "1", 0);
    // NVIDIA's driver beside an integrated card (PRIME render offload): only when its kernel
    // driver is loaded, since naming a GLX vendor that is not installed leaves no OpenGL at all.
    std::error_code none;
    if (std::filesystem::exists("/proc/driver/nvidia/version", none)) {
        setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 0);
        setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 0);
        setenv("__VK_LAYER_NV_optimus", "NVIDIA_only", 0);
    }
#endif
}

bool is_software_renderer(const std::string& renderer, const std::string& vendor) {
    std::string text = renderer + " " + vendor;
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    // Mesa's (llvmpipe, softpipe, swrast), Google's SwiftShader, Apple's software renderer,
    // Windows' OpenGL 1.1 without a driver and its WARP adapter (through Mesa's D3D12 driver).
    for (const char* name : {"llvmpipe", "softpipe", "swrast", "swiftshader", "software rasterizer",
                             "software renderer", "gdi generic", "microsoft basic render"}) {
        if (text.find(name) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::vector<DisplayInfo> displays() {
    std::vector<DisplayInfo> list;
    int count = 0;
    SDL_DisplayID* ids = SDL_GetDisplays(&count);
    for (int i = 0; ids != nullptr && i < count; ++i) {
        DisplayInfo info;
        if (const char* name = SDL_GetDisplayName(ids[i])) {
            info.name = name;
        }
        if (const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(ids[i])) {
            info.width = mode->w;
            info.height = mode->h;
            info.refresh_rate = mode->refresh_rate;
        }
        list.push_back(std::move(info));
    }
    SDL_free(ids);
    return list;
}

std::vector<DisplaySize> fullscreen_sizes(int display) {
    std::vector<DisplaySize> sizes;
    SDL_DisplayID id = display_id(display);
    if (id == 0) {
        id = SDL_GetPrimaryDisplay();
    }
    int count = 0;
    SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(id, &count);
    for (int i = 0; modes != nullptr && i < count; ++i) {
        const DisplaySize size{modes[i]->w, modes[i]->h};
        if (std::find(sizes.begin(), sizes.end(), size) == sizes.end()) {
            sizes.push_back(size);
        }
    }
    SDL_free(modes);
    std::sort(sizes.begin(), sizes.end(), [](const DisplaySize& a, const DisplaySize& b) {
        return a.width != b.width ? a.width > b.width : a.height > b.height;
    });
    return sizes;
}

#if defined(__APPLE__)
void macos_set_app_name(const char* name);  // macos_app.mm
#else
void install_app_menus(bool) {}
#endif

void set_app_metadata(const char* name, const char* id) {
    SDL_SetAppMetadata(name, nullptr, id);
#if defined(__APPLE__)
    // Outside its bundle the menu bar would show the program file's name (openrac-rac1-pal).
    macos_set_app_name("OpenRAC");
#endif
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

GlApi default_gl_api() {
    if (const char* env = std::getenv("OPENRAC_GL_API"); env != nullptr && env[0] != '\0') {
        const std::string_view value(env);
        if (value == "es" || value == "gles") {
            return GlApi::ES;
        }
        if (value == "desktop" || value == "gl") {
            return GlApi::Desktop;
        }
        log::warn("OPENRAC_GL_API={}: not es or desktop, ignored", value);
    }
#if defined(__ANDROID__)
    return GlApi::ES;
#else
    return GlApi::Desktop;
#endif
}

std::unique_ptr<Window> Window::open(const WindowConfig& config, std::string& error) {
    const bool es = config.gl_api == GlApi::ES;
    const int major = es ? kGlesMajor : kGlMajor;
    const int minor = es ? kGlesMinor : kGlMinor;
    const char* api_name = es ? "OpenGL ES" : "OpenGL";
    // Context attributes must be set before the window exists. The same request everywhere: 4.1
    // core is the most macOS gives, and Mesa and Windows' drivers give a core context of that
    // version or a later one that runs it.
    const auto set_attributes = [&] {
        SDL_GL_ResetAttributes();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, es ? SDL_GL_CONTEXT_PROFILE_ES : SDL_GL_CONTEXT_PROFILE_CORE);
        // macOS only gives a core context to a forward-compatible request; ES has no such flag.
        int context_flags = es ? 0 : SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
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
        // No sRGB conversion on the way to the window (see below, where it is turned off as well).
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
        if (config.msaa_samples > 1) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, config.msaa_samples);
        }
    };
    set_attributes();

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (config.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }
    if (config.hidden) {
        flags |= SDL_WINDOW_HIDDEN;
    }

    // Without OpenGL 4.1 (ES 3.2) there is no game to show: say so in a message box
    // too, since a player starting from the launcher sees no console.
    auto no_opengl = [&](SDL_Window* parent) {
        error = std::format(
            "{} {}.{}{} is not available: {}. OpenRAC draws with {} {}.{}; "
            "update the graphics driver, or check that the graphics card supports it.",
            api_name,
            major,
            minor,
            es ? "" : " (core profile)",
            SDL_GetError(),
            api_name,
            major,
            minor
        );
        if (!config.hidden) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenRAC", error.c_str(), parent);
        }
    };

    std::unique_ptr<Window> window(new Window());
    // The first request is for the graphics card only (WGL's full acceleration, macOS's
    // accelerated pixel formats); where the system cannot promise that, the context is made
    // without the condition and its strings say below what it is.
    SDL_GLContext context = nullptr;
    for (const int accelerated : {1, -1}) {
        if (accelerated > 0) {
            SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        } else {
            // Back to "either": SDL has no call that clears one attribute.
            set_attributes();
        }
        window->m_window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
        if (window->m_window != nullptr) {
            context = SDL_GL_CreateContext(window->m_window);
            if (context != nullptr) {
                break;
            }
            if (accelerated > 0) {
                SDL_DestroyWindow(window->m_window);
                window->m_window = nullptr;
            }
        }
    }
    if (window->m_window == nullptr) {
        // SDL fails here when the video driver has no OpenGL at all.
        no_opengl(nullptr);
        return nullptr;
    }
    if (context == nullptr) {
        no_opengl(window->m_window);
        return nullptr;
    }
    window->m_context = context;
    window->m_gl_api = config.gl_api;
    if (!SDL_GL_MakeCurrent(window->m_window, context)) {
        error = std::format("could not use the OpenGL context: {}", SDL_GetError());
        return nullptr;
    }
    window->m_gl_version = gl_string(kGlVersion);
    window->m_gl_renderer = gl_string(kGlRenderer);
    window->m_gl_vendor = gl_string(kGlVendor);
    window->m_software = is_software_renderer(window->m_gl_renderer, window->m_gl_vendor);
    // What draws the game, said once and plainly: the first thing to read in a report of a slow or
    // wrong picture.
    {
        int red = 0, depth = 0, stencil = 0, samples = 0, doublebuffer = 0;
        SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &red);
        SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
        SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil);
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &samples);
        SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer);
        const char* driver = SDL_GetCurrentVideoDriver();
        // GL_VERSION of an ES context starts "OpenGL ES" already.
        const char* prefix = es ? "" : "OpenGL ";
        log::info("graphics: {} ({}), {}{}, GLSL {}", window->m_gl_renderer, window->m_gl_vendor, prefix,
                  window->m_gl_version, gl_string(kGlShadingLanguageVersion));
        // The card and its driver in the session's header too, for a crash report (common/crash.h).
        log::header(std::format("graphics  {} ({}), {}{}, {} through SDL's {} driver", window->m_gl_renderer,
                                window->m_gl_vendor, prefix, window->m_gl_version,
                                window->m_software ? "SOFTWARE" : "hardware", driver != nullptr ? driver : "?"),
                    false);
        log::info("graphics: {} context through SDL's {} driver; the window's buffer {} bits a channel, depth {}, "
                  "stencil {}, {} samples, {}",
                  window->m_software ? "SOFTWARE (drawn on the CPU)" : "hardware", driver != nullptr ? driver : "?",
                  red, depth, stencil, samples, doublebuffer != 0 ? "double buffered" : "single buffered");
    }
    if (window->m_software) {
        const std::string what = std::format(
            "OpenGL is being drawn on the CPU by a software rasteriser ({}), not by a graphics card: the game "
            "would run at a few frames a second. Install or update the graphics driver; on a machine with two "
            "graphics cards, check that the program is allowed the fast one. (OPENRAC_SOFTWARE_GL=1 in the "
            "environment runs it anyway.)",
            window->m_gl_renderer
        );
        if (config.software == SoftwareRenderer::Refuse) {
            error = what;
            log::error("{}", what);
            if (!config.hidden) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenRAC", what.c_str(), window->m_window);
            }
            return nullptr;
        }
        log::warn("{}", what);
    }
    // The renderer blends and writes colours as the console's chip did, in the display's own
    // (gamma) space: the window's buffer must never convert them as if they were linear. Off is
    // OpenGL's default; some drivers start with it on for a buffer that can do sRGB.
    // ES has no such switch: an ES window's buffer is linear unless asked for in sRGB.
    if (!es) {
        using Disable = void (*)(unsigned);
        if (auto disable = reinterpret_cast<Disable>(SDL_GL_GetProcAddress("glDisable"))) {
            disable(kGlFramebufferSrgb);
        }
    }

    window->set_vsync(config.vsync);
    if (config.display > 0 && config.mode == DisplayMode::Windowed) {
        if (const SDL_DisplayID id = display_id(config.display)) {
            SDL_SetWindowPosition(window->m_window, static_cast<int>(SDL_WINDOWPOS_CENTERED_DISPLAY(id)),
                                  static_cast<int>(SDL_WINDOWPOS_CENTERED_DISPLAY(id)));
        }
    }
    if (config.mode != DisplayMode::Windowed) {
        window->set_display_mode(config.mode, config.fullscreen_width, config.fullscreen_height, config.display);
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

bool Window::set_display_mode(DisplayMode mode, int width, int height, int display) {
    bool ok = true;
    // Onto the display asked for first: fullscreen covers the display the window is on.
    if (const SDL_DisplayID wanted = display_id(display);
        wanted != 0 && mode != DisplayMode::Windowed && wanted != SDL_GetDisplayForWindow(m_window)) {
        if (m_mode != DisplayMode::Windowed) {
            SDL_SetWindowFullscreen(m_window, false);
            SDL_SyncWindow(m_window);
        }
        SDL_SetWindowPosition(m_window, static_cast<int>(SDL_WINDOWPOS_CENTERED_DISPLAY(wanted)),
                              static_cast<int>(SDL_WINDOWPOS_CENTERED_DISPLAY(wanted)));
        SDL_SyncWindow(m_window);
    }
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

SwapInterval Window::set_swap_interval(SwapInterval interval) {
    if (interval == SwapInterval::Adaptive && SDL_GL_SetSwapInterval(-1)) {
        return SwapInterval::Adaptive;
    }
    if (interval != SwapInterval::Immediate && SDL_GL_SetSwapInterval(1)) {
        return SwapInterval::VSync;
    }
    if (!SDL_GL_SetSwapInterval(0)) {
        log::warn("the swap interval could not be set: {}", SDL_GetError());
    }
    return SwapInterval::Immediate;
}

float Window::refresh_rate() const {
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(m_window));
    return mode != nullptr ? mode->refresh_rate : 0.0f;
}

bool Window::focused() const {
    return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_INPUT_FOCUS) != 0;
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

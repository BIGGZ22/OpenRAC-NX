// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// SDL3 and the game's window: an OpenGL 4.1 core context (the version macOS
// stops at, and all OpenGOAL's renderer needs; OpenGL ES 3.2 on Android, GlApi), windowed, fullscreen at a
// chosen resolution, or borderless at the desktop's, on a chosen display, with
// vsync on, off or adaptive. The context is asked for on the graphics card:
// a software rasteriser is named in the log, and refused when the program
// says so (SoftwareRenderer).
//
// The renderer never sees SDL: it gets the context's function loader
// (gl_loader) and the drawable size, and draws into whatever is current.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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

// Whether a swap waits for the display's refresh.
enum class SwapInterval {
    Immediate,  // no wait: the frame is shown at once, and may tear
    VSync,      // each swap waits for a refresh
    Adaptive,   // waits, but a frame that comes late is shown at once (falls back to VSync)
};

// What to do when the context OpenGL gives is a software rasteriser (Mesa's llvmpipe, SwiftShader,
// Apple's software renderer, Windows' GDI Generic): it draws a level at a few frames a second.
enum class SoftwareRenderer {
    Allow,   // say so in the log (headless checks, which draw through llvmpipe on purpose)
    Refuse,  // no window: an error that says what to do about it, and a message box
};

// On a machine with two graphics cards, ask for the fast one, before the Platform starts SDL:
// Linux's PRIME offloading through the environment (DRI_PRIME for Mesa's drivers; NVIDIA's
// variables when its kernel driver is loaded), unless the player set those themselves. Windows
// reads the request from two symbols the program exports (window.cpp) and macOS gives an OpenGL
// program the discrete card already, so there this does nothing. OPENRAC_GPU=integrated in the
// environment turns it off.
void prefer_discrete_gpu();

// A display, as SDL lists it: its name, the size of its desktop and its refresh rate.
struct DisplayInfo {
    std::string name;
    int width = 0;
    int height = 0;
    float refresh_rate = 0.0f;
};

// The displays connected now, in SDL's order (the first is the primary one). Needs a Platform.
std::vector<DisplayInfo> displays();

// A size a display can be switched to for DisplayMode::Fullscreen.
struct DisplaySize {
    int width = 0;
    int height = 0;
    bool operator==(const DisplaySize&) const = default;
};

// The sizes of a display's fullscreen modes, largest first, each once (`display` counts from 1,
// as displays() lists them; 0 or one that is gone: the primary display).
std::vector<DisplaySize> fullscreen_sizes(int display);

// The program's name and id for the desktop, before the Platform starts SDL: on Wayland the id is
// the window's app id, by which the desktop finds the program's icon and groups its windows.
void set_app_metadata(const char* name, const char* id);

// macOS: the menus of the menu bar beside the ones the system gives (View, Sound, Developer),
// whose items press the keys the port already answers (F2, F3, F5...), so they need no code of
// their own in the game. Call once the window exists. Nothing on other systems.
void install_app_menus(bool developer);

// Which OpenGL the window's context is: the desktop's 4.1 core profile, OpenGL ES 3.2 (Android,
// whose drivers have only ES; renderer/gl.h and renderer/shader.h adapt to it), or the renderer's
// GL carried out with Vulkan (renderer/vulkan/vkgl.h), where the build has it; a window that
// cannot have Vulkan opens with the system's OpenGL instead.
enum class GlApi {
    Desktop,
    ES,
    Vulkan,
};

// ES on Android, the desktop's elsewhere; OPENRAC_GL_API=es, =desktop or =vulkan in the
// environment chooses (es on a desktop draws through its driver's ES context, which is how the ES
// path is checked without a phone).
GlApi default_gl_api();

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
    // The display the window opens on, counted from 1 as displays() lists them; 0: SDL's choice.
    int display = 0;
    SoftwareRenderer software = SoftwareRenderer::Allow;
    bool resizable = true;
    bool hidden = false;  // never shown: headless checks
    bool debug_context = false;
    int msaa_samples = 0;
    GlApi gl_api = default_gl_api();
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

    // `display`: the display to cover (counted from 1, as displays() lists them), the window
    // moving there first; 0 is the one the window is on.
    bool set_display_mode(DisplayMode mode, int width = 0, int height = 0, int display = 0);

    DisplayMode display_mode() const { return m_mode; }

    // Vsync: adaptive (late frames tear rather than wait) when the driver has
    // it, otherwise plain. Returns false if the driver refused both.
    bool set_vsync(bool on);

    // The swap's wait, as asked. Returns what the driver took: Adaptive may come back as VSync, and
    // anything as Immediate when the driver refuses to wait.
    SwapInterval set_swap_interval(SwapInterval interval);

    // The refresh rate of the display the window is on now, in hertz; 0 when it does not say.
    float refresh_rate() const;

    // True while the window has the keyboard.
    bool focused() const;

    // The window's icon: `rgba` is width x height pixels, 4 bytes each, top row first.
    void set_icon(const unsigned char* rgba, int width, int height);
    void swap();

    // The size to render at, in pixels (larger than the window's size on a
    // high-density display).
    void drawable_size(int& width, int& height) const;

    static GlLoader gl_loader();

    // GL_VERSION, GL_RENDERER and GL_VENDOR of the context, for the log.
    const std::string& gl_version() const { return m_gl_version; }

    const std::string& gl_renderer() const { return m_gl_renderer; }

    const std::string& gl_vendor() const { return m_gl_vendor; }

    // The context draws on the CPU (is_software_renderer of its strings).
    bool software() const { return m_software; }

    // The kind of context open() made.
    GlApi gl_api() const { return m_gl_api; }

    SDL_Window* sdl() const { return m_window; }

private:
    Window() = default;

    // Opens the window for GlApi::Vulkan; false (and nothing left open) when Vulkan cannot be had.
    bool open_vulkan(const WindowConfig& config, std::string& error);

    SDL_Window* m_window = nullptr;
    SDL_GLContextState* m_context = nullptr;
    bool m_vulkan = false;
    bool m_vulkan_library = false;  // macOS: the Vulkan library this window had SDL load
    DisplayMode m_mode = DisplayMode::Windowed;
    std::string m_gl_version;
    std::string m_gl_renderer;
    std::string m_gl_vendor;
    bool m_software = false;
    GlApi m_gl_api = GlApi::Desktop;
};

// True when GL_RENDERER or GL_VENDOR names a rasteriser that runs on the CPU.
bool is_software_renderer(const std::string& renderer, const std::string& vendor);

// An outside maker of the Vulkan instance and device, which then shares them (a mod's OpenXR
// session: openrac_mod.h's openrac_vulkan_setup), for the windows opened with Vulkan from now on.
// A null function leaves that step the port's own. Nothing in a build without Vulkan.
struct VulkanSetup {
    void* user = nullptr;
    int (*create_instance)(
        void* user, void* get_instance_proc_addr, const void* create_info, void** instance
    ) = nullptr;
    void* (*physical_device)(void* user, void* instance) = nullptr;
    int (*create_device)(
        void* user,
        void* get_instance_proc_addr,
        void* physical_device,
        const void* create_info,
        void** device
    ) = nullptr;
};

void set_vulkan_setup(const VulkanSetup& setup);

// The open Vulkan context's instance, physical device, device and queue (vkgl::Native): all null
// without one.
struct VulkanNative {
    void* instance = nullptr;
    void* physical_device = nullptr;
    void* device = nullptr;
    void* queue = nullptr;
    std::uint32_t queue_family = 0;
    std::uint32_t queue_index = 0;
    void* get_instance_proc_addr = nullptr;
};

VulkanNative vulkan_native();

// Copies GL texture `texture` of the Vulkan context into an outside VkImage of its device
// (vkgl::copy_to_image). False without Vulkan.
bool copy_to_vulkan_image(
    unsigned texture, void* image, std::uint32_t layout, int width, int height
);

}  // namespace openrac::platform

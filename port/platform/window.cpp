// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/window.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <SDL3/SDL.h>

#include "common/env.h"
#include "common/log.h"
#include "platform/gpu_driver.h"
#if defined(OPENRAC_VULKAN)
#include <SDL3/SDL_vulkan.h>

#include "renderer/vulkan/vkgl.h"
#endif
#if defined(OPENRAC_VULKAN) && defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

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
    if (env::value(env::Var::Gpu) == "integrated") {
        log::info("graphics card: the system's default (OPENRAC_GPU=integrated)");
        return;
    }

    // NVIDIA's driver beside an integrated card (PRIME render offload): only when its kernel
    // driver is loaded, since naming a GLX vendor that is not installed leaves no OpenGL at all.
    std::error_code none;
    if (std::filesystem::exists("/proc/driver/nvidia/version", none)) {
        setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 0);
        setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 0);
        setenv("__VK_LAYER_NV_optimus", "NVIDIA_only", 0);
        return;
    }

    // If DRI_PRIME is already configured by the user, leave it untouched.
    if (env::system_variable("DRI_PRIME").has_value()) {
        return;
    }

    // Inspect available DRM cards to choose the best GPU intelligently.
    // On multi-GPU systems (e.g. desktop with iGPU + dedicated PCIe GPU), setting DRI_PRIME=1
    // tells Mesa to offload to the SECONDARY card. If the desktop booted with the discrete GPU
    // as primary (boot_vga == 1), DRI_PRIME=1 would erroneously demote the game to the slow iGPU.
    struct GpuCard {
        int index = 0;
        bool boot_vga = false;
        std::uint64_t vram_bytes = 0;
        std::string pci_id;
    };

    std::vector<GpuCard> cards;
    for (int i = 0; i < 8; ++i) {
        const std::string dev_path = std::format("/sys/class/drm/card{}/device", i);
        if (!std::filesystem::exists(dev_path, none)) {
            continue;
        }
        GpuCard card;
        card.index = i;

        const std::string boot_path = dev_path + "/boot_vga";
        if (std::FILE* f = std::fopen(boot_path.c_str(), "r")) {
            int val = 0;
            if (std::fscanf(f, "%d", &val) == 1) {
                card.boot_vga = (val == 1);
            }
            std::fclose(f);
        }

        const std::string vram_path = dev_path + "/mem_info_vram_total";
        if (std::FILE* f = std::fopen(vram_path.c_str(), "r")) {
            unsigned long long vram = 0;
            if (std::fscanf(f, "%llu", &vram) == 1) {
                card.vram_bytes = vram;
            }
            std::fclose(f);
        }

        const std::string uevent_path = dev_path + "/uevent";
        if (std::FILE* f = std::fopen(uevent_path.c_str(), "r")) {
            char line[256];
            while (std::fgets(line, sizeof(line), f) != nullptr) {
                if (std::strncmp(line, "PCI_ID=", 7) == 0) {
                    std::string id = line + 7;
                    while (!id.empty() && (id.back() == '\n' || id.back() == '\r')) {
                        id.pop_back();
                    }
                    card.pci_id = id;
                    break;
                }
            }
            std::fclose(f);
        }

        cards.push_back(std::move(card));
    }

    if (cards.size() > 1) {
        const auto* boot_card = &cards[0];
        const auto* best_card = &cards[0];
        for (const auto& c : cards) {
            if (c.boot_vga) {
                boot_card = &c;
            }
            if (c.vram_bytes > best_card->vram_bytes) {
                best_card = &c;
            }
        }

        // If primary boot GPU is already discrete (>= 1 GB VRAM and greatest VRAM),
        // keep default: setting DRI_PRIME=1 would force the secondary (integrated) GPU.
        if (boot_card == best_card && boot_card->vram_bytes >= (1024ull * 1024 * 1024)) {
            log::info("graphics card: primary GPU is discrete (card{}, {:.1f} GB VRAM); keeping default",
                      boot_card->index, static_cast<double>(boot_card->vram_bytes) / (1024.0 * 1024.0 * 1024.0));
            return;
        }

        // If another card has significantly more VRAM than boot GPU, select it via DRI_PRIME.
        if (best_card != boot_card && best_card->vram_bytes > boot_card->vram_bytes) {
            if (!best_card->pci_id.empty()) {
                setenv("DRI_PRIME", best_card->pci_id.c_str(), 0);
            } else {
                setenv("DRI_PRIME", "1", 0);
            }
            log::info("graphics card: preferring secondary discrete GPU (card{}, {:.1f} GB VRAM)",
                      best_card->index, static_cast<double>(best_card->vram_bytes) / (1024.0 * 1024.0 * 1024.0));
            return;
        }
    }

    // Default fallback: Mesa PRIME offload
    setenv("DRI_PRIME", "1", 0);
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
    if (id != nullptr) {
        SDL_SetHint(SDL_HINT_APP_ID, id);
    }
    if (name != nullptr) {
        SDL_SetHint(SDL_HINT_APP_NAME, name);
    }
#if defined(__APPLE__)
    // Outside its bundle the menu bar would show the program file's name (openrac-rac1-pal).
    macos_set_app_name("OpenRAC");
#endif
}

Platform::Platform(unsigned subsystems) {
    SDL_InitFlags flags = 0;
    if ((subsystems & kVideo) != 0) {
        flags |= SDL_INIT_VIDEO;
#if defined(__linux__) || defined(__FreeBSD__)
        // On Wayland, allow tearing / immediate presentation when vsync is disabled.
        SDL_SetHint("SDL_VIDEO_WAYLAND_ALLOW_TEARING", "1");
#endif
        // The game goes on while another window has the focus: a full-screen window stays up
        // instead of minimising (SDL's default on Windows), so it can stay on one display while
        // the player uses another.
        SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    }
    if ((subsystems & kGamepad) != 0) {
        flags |= SDL_INIT_GAMEPAD;
        // ...and the controller still plays it. The keyboard counts only while the window has the
        // focus (Input::update), so typing elsewhere never moves Ratchet.
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
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
    const std::optional<std::string> x11 = env::system_variable("DISPLAY");
    const std::optional<std::string> wayland = env::system_variable("WAYLAND_DISPLAY");
    return (x11 && !x11->empty()) || (wayland && !wayland->empty());
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
    if (const std::string_view value = env::value(env::Var::GlApi).value_or(""); !value.empty()) {
        if (value == "es" || value == "gles") {
            return GlApi::ES;
        }
        if (value == "desktop" || value == "gl") {
            return GlApi::Desktop;
        }
        if (value == "vulkan" || value == "vk") {
            return GlApi::Vulkan;
        }
        log::warn("OPENRAC_GL_API={}: not es, desktop or vulkan, ignored", value);
    }
#if defined(__ANDROID__)
    return GlApi::ES;
#else
    return GlApi::Desktop;
#endif
}

#if defined(OPENRAC_VULKAN) && defined(__APPLE__)
namespace {

// The folder of the running program, links resolved (<build>/openrac-rac1-pal is a link into its
// application bundle).
std::filesystem::path program_folder() {
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string path(size, '\0');
    if (size == 0 || _NSGetExecutablePath(path.data(), &size) != 0) {
        return {};
    }
    path.resize(std::strlen(path.c_str()));
    std::error_code ec;
    const std::filesystem::path real = std::filesystem::canonical(path, ec);
    return ec ? std::filesystem::path(path).parent_path() : real.parent_path();
}

// macOS has no Vulkan of its own: it is MoltenVK, Vulkan on Metal. SDL is given the library by
// its full path (a bare name is not looked for in Homebrew's or the bundle's folders), and volk
// takes its vkGetInstanceProcAddr from SDL, so that the window's surface and the renderer use the
// same one. In order: OPENRAC_VULKAN_LIBRARY; MoltenVK in the application bundle
// (Contents/Frameworks, cmake/MacApp.cmake), what a player has; a Vulkan loader installed with
// Homebrew or the Vulkan SDK (it has the validation layer, so it comes first when that is asked
// for); MoltenVK installed there; SDL's own search.
bool load_vulkan_library(bool validation, std::string& error) {
    std::vector<std::string> paths;
    const std::string_view library = env::value(env::Var::VulkanLibrary).value_or("");
    if (!library.empty()) {
        paths.emplace_back(library);
    }
    const std::string bundled = (program_folder().parent_path() / "Frameworks" / "libMoltenVK.dylib").string();
    std::vector<std::string> folders;
    const std::string sdk = env::system_variable("VULKAN_SDK").value_or("");
    if (!sdk.empty()) {
        folders.push_back(sdk + "/lib");
    }
    folders.insert(folders.end(), {"/opt/homebrew/lib", "/usr/local/lib"});
    if (!validation) {
        paths.push_back(bundled);
    }
    for (const std::string& folder : folders) {
        paths.push_back(folder + "/libvulkan.1.dylib");
    }
    if (validation) {
        paths.push_back(bundled);
    }
    for (const std::string& folder : folders) {
        paths.push_back(folder + "/libMoltenVK.dylib");
    }
    std::string tried;
    for (const std::string& path : paths) {
        std::error_code ec;
        // OPENRAC_VULKAN_LIBRARY may be a bare name, for the system's own search.
        if (path.find('/') != std::string::npos && !std::filesystem::exists(path, ec)) {
            continue;
        }
        if (SDL_Vulkan_LoadLibrary(path.c_str())) {
            log::info("vulkan: {}", path);
            return true;
        }
        tried += std::format("{}{}: {}", tried.empty() ? "" : "; ", path, SDL_GetError());
    }
    if (SDL_Vulkan_LoadLibrary(nullptr)) {
        return true;
    }
    error = tried.empty() ? "no MoltenVK or Vulkan loader found (the game's bundle carries none; brew install molten-vk)"
                          : std::format("no Vulkan library could be opened ({})", tried);
    return false;
}

}  // namespace
#endif

bool Window::open_vulkan(const WindowConfig& config, std::string& error) {
#if defined(OPENRAC_VULKAN)
#if defined(__APPLE__)
    if (custom_vulkan_get_instance_proc_addr() == nullptr) {
        const bool validation = config.debug_context || env::flag(env::Var::VulkanValidation);
        if (!load_vulkan_library(validation, error)) {
            return false;
        }
        m_vulkan_library = true;
    }
#endif
    // What this window loaded is let go again when Vulkan cannot be had.
    const auto fail = [this] {
        if (m_vulkan_library) {
            SDL_Vulkan_UnloadLibrary();
            m_vulkan_library = false;
        }
    };
    // A hidden window (the headless checks) draws without a screen to show it on.
    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY | (config.hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_VULKAN);
#if defined(__APPLE__)
    // On macOS a window without a graphics flag is an OpenGL one, and SDL opens OpenGL for it
    // (which macOS 27 no longer has where SDL looks): a hidden window is a Vulkan one as well.
    flags |= SDL_WINDOW_VULKAN;
#endif
    if (config.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }
#if defined(__ANDROID__)
    flags |= SDL_WINDOW_FULLSCREEN;
#endif
    m_window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
    if (m_window == nullptr) {
        error = std::format("no window for Vulkan: {}", SDL_GetError());
        fail();
        return false;
    }
    vkgl::Config vk;
    vk.window = config.hidden ? nullptr : m_window;
    SDL_GetWindowSizeInPixels(m_window, &vk.width, &vk.height);
    vk.vsync = config.vsync;
    vk.get_instance_proc_addr = custom_vulkan_get_instance_proc_addr();
    if (vk.get_instance_proc_addr == nullptr && m_vulkan_library) {
        vk.get_instance_proc_addr = reinterpret_cast<void*>(SDL_Vulkan_GetVkGetInstanceProcAddr());
    }
    vk.validation = config.debug_context;
    if (!vkgl::open(vk, error)) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        fail();
        return false;
    }
    m_vulkan = true;
    m_gl_api = GlApi::Vulkan;
    m_gl_version = std::format("Vulkan {}", vkgl::api_version());
    m_gl_renderer = vkgl::device_name();
    m_gl_vendor = vkgl::driver_version();
    m_software = vkgl::software();
    log::info("graphics: {} ({}), Vulkan {}, the renderer's GL carried out with Vulkan", m_gl_renderer, m_gl_vendor,
              vkgl::api_version());
    log::header(std::format("graphics  {} ({}), Vulkan {}, {}", m_gl_renderer, m_gl_vendor, vkgl::api_version(),
                            m_software ? "SOFTWARE" : "hardware"),
                false);
    if (config.display > 0 && config.mode == DisplayMode::Windowed) {
        if (const SDL_DisplayID id = display_id(config.display)) {
            SDL_SetWindowPosition(m_window, static_cast<int>(SDL_WINDOWPOS_CENTERED_DISPLAY(id)),
                                  static_cast<int>(SDL_WINDOWPOS_CENTERED_DISPLAY(id)));
        }
    }
    if (config.mode != DisplayMode::Windowed) {
        set_display_mode(config.mode, config.fullscreen_width, config.fullscreen_height, config.display);
    }
    return true;
#else
    (void)config;
    error = "this build has no Vulkan renderer";
    return false;
#endif
}

std::unique_ptr<Window> Window::open(const WindowConfig& asked, std::string& error) {
    WindowConfig config = asked;
    if (config.gl_api == GlApi::Vulkan) {
        std::unique_ptr<Window> window(new Window());
        std::string why;
        if (window->open_vulkan(config, why)) {
            return window;
        }
        // Drawn with OpenGL instead: the game still runs.
        log::warn("graphics: Vulkan could not be used ({}); drawing with OpenGL", why);
#if defined(__ANDROID__)
        config.gl_api = GlApi::ES;
#else
        config.gl_api = GlApi::Desktop;
#endif
    }
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
#if defined(__ANDROID__)
    // A phone's game covers the screen: SDL's activity hides the system's bars for a fullscreen
    // window only (a swipe from the edge shows them for a moment).
    flags |= SDL_WINDOW_FULLSCREEN;
#endif

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
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) {
        vkgl::close();
    }
#endif
    if (m_context != nullptr) {
        SDL_GL_DestroyContext(m_context);
    }
    if (m_window != nullptr) {
        SDL_DestroyWindow(m_window);
    }
#if defined(OPENRAC_VULKAN)
    if (m_vulkan_library) {
        SDL_Vulkan_UnloadLibrary();
    }
#endif
}

bool Window::set_display_mode(DisplayMode mode, int width, int height, int display) {
    bool ok = true;
#if defined(__ANDROID__)
    // No windows on Android: the game always covers the screen, without the system's bars.
    mode = DisplayMode::Borderless;
#endif
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
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) {
        vkgl::set_swap_interval(on ? -1 : 0);
        return true;
    }
#endif
    if (!on) {
        return SDL_GL_SetSwapInterval(0);
    }
    if (SDL_GL_SetSwapInterval(-1)) {
        return true;
    }
    return SDL_GL_SetSwapInterval(1);
}

SwapInterval Window::set_swap_interval(SwapInterval interval) {
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) {
        const int got = vkgl::set_swap_interval(interval == SwapInterval::Adaptive ? -1
                                                : interval == SwapInterval::VSync  ? 1
                                                                                   : 0);
        return got < 0 ? SwapInterval::Adaptive : got > 0 ? SwapInterval::VSync : SwapInterval::Immediate;
    }
#endif
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
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) {
        vkgl::present();
        return;
    }
#endif
    SDL_GL_SwapWindow(m_window);
}

void Window::drawable_size(int& width, int& height) const {
    if (!SDL_GetWindowSizeInPixels(m_window, &width, &height)) {
        width = 0;
        height = 0;
    }
}

GlLoader Window::gl_loader() {
#if defined(OPENRAC_VULKAN)
    if (vkgl::active()) {
        return vkgl::get_proc;
    }
#endif
    return SDL_GL_GetProcAddress;
}

}  // namespace openrac::platform

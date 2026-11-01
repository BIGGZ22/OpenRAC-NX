// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/sdl.h"
#include <switch.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include "common/log.h"
#include "common/stats.h"
#include "platform/window.h"
#include "platform/input.h"
#include "platform/pump.h"
#include "platform/switch/clocks.h"
#include "platform/switch/input.h"
#include "platform/switch/overlay_stats.h"
#if defined(OPENRAC_VULKAN)
#include <volk.h>
#include "renderer/vulkan/vkgl.h"
// Native driver function, distinct from volk's renamed dispatch pointer.
extern "C" PFN_vkVoidFunction switch_nvk_proc(VkInstance, const char*)
    __asm__("vkGetInstanceProcAddr");
#endif
#if defined(OPENRAC_DKGL)
#include "renderer/dkgl/dkgl.h"
#endif

namespace openrac::platform {
Platform::Platform(unsigned systems) {
    Uint32 flags = 0;
    if (systems & kVideo) flags |= SDL_INIT_VIDEO;
    if (systems & kAudio) flags |= SDL_INIT_AUDIO;
    m_ok = SDL_Init(flags) == 0;
    if (!m_ok) m_error = SDL_GetError();
    // The clocks: logged, and set only when env.txt asks (platform/switch/clocks.h).
    switch_clocks::start();
}
Platform::~Platform() { switch_clocks::stop(); overlay_stats::shutdown(); SDL_Quit(); }
void Platform::use_offscreen_video() {}
bool Platform::display_available() { return true; }
void prefer_discrete_gpu() {}
void set_app_metadata(const char*, const char*) {}
void install_app_menus(bool) {}
std::vector<DisplayInfo> displays() { return {{"Switch", 1280, 720, 60.0f}}; }
std::vector<DisplaySize> fullscreen_sizes(int) { return {{1280, 720}}; }
GlApi default_gl_api() {
#if defined(OPENRAC_VULKAN)
    return GlApi::Vulkan;
#else
    return GlApi::Desktop;
#endif
}
bool is_software_renderer(const std::string& renderer, const std::string&) {
    return renderer.find("llvmpipe") != std::string::npos;
}
std::unique_ptr<Window> Window::open(const WindowConfig& config, std::string& error) {
    auto out = std::unique_ptr<Window>(new Window);
#if defined(OPENRAC_DKGL)
    // The deko3d test build draws with deko3d only: no OpenGL fallback to mistake for it.
    (void)config.gl_api;
    if (out->open_deko3d(config, error)) return out;
    log::error("Switch deko3d unavailable: {}", error);
    if (FILE* report = std::fopen("sdmc:/switch/openrac/deko3d-startup.log", "w")) {
        std::fprintf(report, "deko3d startup failed: %s\n", error.c_str());
        std::fclose(report);
    }
    return nullptr;
#endif
#if defined(OPENRAC_VULKAN)
    if (config.gl_api == GlApi::Vulkan) {
        if (out->open_vulkan(config, error)) return out;
        log::warn("Switch Vulkan unavailable: {}", error);
        if (FILE* report = std::fopen("sdmc:/switch/openrac/vulkan-startup.log", "w")) {
            std::fprintf(report, "Vulkan startup failed: %s\n", error.c_str());
#if OPENRAC_SWITCH_VULKAN_FALLBACK
            std::fprintf(report, "OpenGL fallback enabled. This run is NOT Vulkan.\n");
#else
            std::fprintf(report, "Stopped: OpenGL fallback is disabled in this Vulkan test.\n");
#endif
            std::fclose(report);
        }
#if !OPENRAC_SWITCH_VULKAN_FALLBACK
        return nullptr;
#endif
        error.clear();
    }
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    out->m_window = SDL_CreateWindow(config.title.c_str(), 0, 0, 1280, 720,
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    if (!out->m_window) { error = SDL_GetError(); return nullptr; }
    out->m_context = static_cast<SDL_GLContextState*>(SDL_GL_CreateContext(out->m_window));
    if (!out->m_context) { error = SDL_GetError(); return nullptr; }
    using GetString = const unsigned char* (*)(unsigned);
    auto get = reinterpret_cast<GetString>(SDL_GL_GetProcAddress("glGetString"));
    if (!get) { error = "Switch OpenGL loader has no glGetString"; return nullptr; }
    const auto text = [get](unsigned key) {
        const auto* value = get(key);
        return value ? std::string(reinterpret_cast<const char*>(value)) : std::string();
    };
    out->m_gl_version = text(0x1F02);
    out->m_gl_renderer = text(0x1F01);
    out->m_gl_vendor = text(0x1F00);
    out->m_mode = DisplayMode::Fullscreen;
    out->m_software = is_software_renderer(out->m_gl_renderer, out->m_gl_vendor);
    out->set_vsync(config.vsync);
    return out;
}
#if defined(OPENRAC_VULKAN)
bool Window::open_vulkan(const WindowConfig& config, std::string& error) {
    // This SDK is experimental on Maxwell and requires this opt-in before enumeration.
    setenv("NVK_I_WANT_A_BROKEN_VULKAN_DRIVER", "1", 0);
    const Result sized = nwindowSetDimensions(nwindowGetDefault(), 1280, 720);
    if (R_FAILED(sized)) { error = "Could not size the native Switch Vulkan window"; return false; }
    vkgl::Config vk;
    vk.native_window = nwindowGetDefault();
    vk.width = 1280;
    vk.height = 720;
    vk.vsync = config.vsync;
    vk.get_instance_proc_addr = reinterpret_cast<void*>(switch_nvk_proc);
    // Keep bring-up independent of optional presentation extensions.
    vk.present_id = false;
    vk.present_wait = false;
    vk.fifo_latest_ready = false;
    if (!vkgl::open(vk, error)) {
        return false;
    }
    m_vulkan = true;
    m_gl_api = GlApi::Vulkan;
    m_mode = DisplayMode::Fullscreen;
    m_gl_version = "Vulkan " + vkgl::api_version();
    m_gl_renderer = vkgl::device_name();
    m_gl_vendor = vkgl::driver_version();
    if (FILE* report = std::fopen("sdmc:/switch/openrac/vulkan-startup.log", "w")) {
        std::fprintf(report, "%s\n%s\n%s\n", m_gl_version.c_str(), m_gl_renderer.c_str(), m_gl_vendor.c_str());
        std::fputs(vkgl::device_report().c_str(), report);
        std::fclose(report);
    }
    return true;
}
#endif
#if defined(OPENRAC_DKGL)
bool Window::open_deko3d(const WindowConfig& config, std::string& error) {
    const Result sized = nwindowSetDimensions(nwindowGetDefault(), 1280, 720);
    if (R_FAILED(sized)) { error = "Could not size the native Switch window"; return false; }
    dkgl::Config dk;
    dk.native_window = nwindowGetDefault();
    dk.width = 1280;
    dk.height = 720;
    dk.vsync = config.vsync;
    if (!dkgl::open(dk, error)) return false;
    // OPENRAC_DK_PASS_TIMING: the renderer's kGpu sections become dkgl's timed passes.
    if (dkgl::pass_timing_enabled()) {
        stats::set_pass_hooks({[](std::string_view name) { dkgl::pass_begin(name); },
                               [] { dkgl::pass_end(); }});
    }
    m_deko3d = true;
    m_gl_api = GlApi::Desktop;
    m_mode = DisplayMode::Fullscreen;
    m_gl_version = dkgl::api_version();
    m_gl_renderer = dkgl::device_name();
    m_gl_vendor = dkgl::driver_version();
    if (FILE* report = std::fopen("sdmc:/switch/openrac/deko3d-startup.log", "w")) {
        std::fprintf(report, "%s\n%s\n%s\n", m_gl_version.c_str(), m_gl_renderer.c_str(), m_gl_vendor.c_str());
        std::fclose(report);
    }
    return true;
}
#endif
Window::~Window() {
#if defined(OPENRAC_DKGL)
    if (m_deko3d) dkgl::close();
#endif
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) vkgl::close();
#endif
    if (m_context) SDL_GL_DeleteContext(m_context);
    if (m_window) SDL_DestroyWindow(m_window);
}
bool Window::set_display_mode(DisplayMode, int, int, int) { return true; }
bool Window::set_vsync(bool on) {
#if defined(OPENRAC_DKGL)
    if (m_deko3d) return dkgl::set_swap_interval(on ? 1 : 0) == (on ? 1 : 0);
#endif
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) return vkgl::set_swap_interval(on ? 1 : 0) == (on ? 1 : 0);
#endif
    return SDL_GL_SetSwapInterval(on ? 1 : 0) == 0;
}
SwapInterval Window::set_swap_interval(SwapInterval interval) {
#if defined(OPENRAC_DKGL)
    if (m_deko3d) return dkgl::set_swap_interval(interval == SwapInterval::Immediate ? 0 : 1) == 0
        ? SwapInterval::Immediate : SwapInterval::VSync;
#endif
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) return vkgl::set_swap_interval(interval == SwapInterval::Immediate ? 0 : 1) == 0
        ? SwapInterval::Immediate : SwapInterval::VSync;
#endif
    set_vsync(interval != SwapInterval::Immediate);
    return SDL_GL_GetSwapInterval() == 0 ? SwapInterval::Immediate : SwapInterval::VSync;
}
float Window::refresh_rate() const { return 60.0f; }
bool Window::focused() const { return true; }
void Window::set_icon(const unsigned char*, int, int) {}
void Window::swap() {
#if defined(OPENRAC_DKGL)
    if (m_deko3d) {
        const auto before = dkgl::accepted_presents();
        dkgl::present();
        m_last_swap_presented = dkgl::accepted_presents() > before;
        if (!dkgl::presentation_error().empty()) {
            if (FILE* report = std::fopen("sdmc:/switch/openrac/deko3d-runtime.log", "a")) {
                std::fprintf(report, "%s\n", dkgl::presentation_error().c_str()); std::fclose(report);
            }
            throw std::runtime_error(dkgl::presentation_error());
        }
        return;
    }
#endif
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) {
        const auto before = vkgl::accepted_presents();
        vkgl::present();
        m_last_swap_presented = vkgl::accepted_presents() > before;
        if (!vkgl::presentation_error().empty()) {
            if (FILE* report = std::fopen("sdmc:/switch/openrac/vulkan-runtime.log", "a")) {
                std::fprintf(report,"%s\n",vkgl::presentation_error().c_str()); std::fclose(report);
            }
            throw std::runtime_error(vkgl::presentation_error());
        }
        return;
    }
    m_last_swap_presented = true;
#endif
    SDL_GL_SwapWindow(m_window);
}
void Window::drawable_size(int& w, int& h) const {
#if defined(OPENRAC_DKGL)
    if (m_deko3d) {
        u32 width = 1280, height = 720;
        nwindowGetDimensions(nwindowGetDefault(), &width, &height);
        w = static_cast<int>(width); h = static_cast<int>(height); return;
    }
#endif
#if defined(OPENRAC_VULKAN)
    if (m_vulkan) {
        u32 width = 1280, height = 720;
        nwindowGetDimensions(nwindowGetDefault(), &width, &height);
        w = static_cast<int>(width); h = static_cast<int>(height); return;
    }
#endif
    SDL_GL_GetDrawableSize(m_window, &w, &h);
}
GlLoader Window::gl_loader() {
#if defined(OPENRAC_DKGL)
    if (dkgl::active()) return dkgl::get_proc;
#endif
#if defined(OPENRAC_VULKAN)
    if (vkgl::active()) return vkgl::get_proc;
#endif
    return +[](const char* name) -> GlProc { return reinterpret_cast<GlProc>(SDL_GL_GetProcAddress(name)); };
}

Input::~Input() = default;
void Input::handle_event(const SDL_Event&) {}
void Input::update() {
    auto& p = m_pads[0];
    std::uint8_t analog[4];
    p.connected = switch_read_pad(0, &p.buttons, analog) != 0;
    p.right_x = analog[0]; p.right_y = analog[1];
    p.left_x = analog[2]; p.left_y = analog[3];
    for (unsigned bit = 0; bit < 16; ++bit) {
        auto button = static_cast<Button>(1u << bit);
        p.set(button, p.held(button));
    }
    m_km.focused = true;
}
int run_pumped(int (*program)(void*), void* arg) { return program(arg); }
bool poll_event(SDL_Event& event) {
    // appletMainLoop runs the applet hooks (the clocks' focus watch) on this thread; the clocks'
    // minute log runs here too, so both stay on one thread.
    if (!appletMainLoop()) { event.type = SDL_EVENT_QUIT; return true; }
    switch_clocks::tick();
    SDL2_Event input;
    if (!SDL_PollEvent(&input)) return false;
    event.type = 0;
    if (input.type == SDL_QUIT) event.type = SDL_EVENT_QUIT;
    if (input.type == SDL_KEYDOWN) {
        event.key = {SDL_EVENT_KEY_DOWN, input.key.keysym.sym, input.key.keysym.mod, input.key.repeat};
    }
    if (input.type == SDL_WINDOWEVENT) {
        switch (input.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                event.window = {SDL_EVENT_WINDOW_RESIZED, input.window.data1, input.window.data2};
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST: event.type = SDL_EVENT_WINDOW_FOCUS_LOST; break;
            case SDL_WINDOWEVENT_FOCUS_GAINED: event.type = SDL_EVENT_WINDOW_FOCUS_GAINED; break;
            default: break;
        }
    }
    return true;
}
} // namespace openrac::platform

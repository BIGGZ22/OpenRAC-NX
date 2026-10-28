// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/sdl.h"
#include <switch.h>
#include <algorithm>
#include "platform/window.h"
#include "platform/input.h"
#include "platform/pump.h"
#include "platform/switch/input.h"

namespace openrac::platform {
Platform::Platform(unsigned systems) {
    Uint32 flags = 0;
    if (systems & kVideo) flags |= SDL_INIT_VIDEO;
    if (systems & kAudio) flags |= SDL_INIT_AUDIO;
    m_ok = SDL_Init(flags) == 0;
    if (!m_ok) m_error = SDL_GetError();
}
Platform::~Platform() { SDL_Quit(); }
void Platform::use_offscreen_video() {}
bool Platform::display_available() { return true; }
void prefer_discrete_gpu() {}
void set_app_metadata(const char*, const char*) {}
void install_app_menus(bool) {}
std::vector<DisplayInfo> displays() { return {{"Switch", 1280, 720, 60.0f}}; }
std::vector<DisplaySize> fullscreen_sizes(int) { return {{1280, 720}}; }
GlApi default_gl_api() { return GlApi::Desktop; }
bool is_software_renderer(const std::string& renderer, const std::string&) {
    return renderer.find("llvmpipe") != std::string::npos;
}
std::unique_ptr<Window> Window::open(const WindowConfig& config, std::string& error) {
    auto out = std::unique_ptr<Window>(new Window);
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
Window::~Window() {
    if (m_context) SDL_GL_DeleteContext(m_context);
    if (m_window) SDL_DestroyWindow(m_window);
}
bool Window::set_display_mode(DisplayMode, int, int, int) { return true; }
bool Window::set_vsync(bool on) { return SDL_GL_SetSwapInterval(on ? 1 : 0) == 0; }
SwapInterval Window::set_swap_interval(SwapInterval interval) {
    set_vsync(interval != SwapInterval::Immediate);
    return SDL_GL_GetSwapInterval() == 0 ? SwapInterval::Immediate : SwapInterval::VSync;
}
float Window::refresh_rate() const { return 60.0f; }
bool Window::focused() const { return true; }
void Window::set_icon(const unsigned char*, int, int) {}
void Window::swap() { SDL_GL_SwapWindow(m_window); }
void Window::drawable_size(int& w, int& h) const { SDL_GL_GetDrawableSize(m_window, &w, &h); }
GlLoader Window::gl_loader() {
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
    if (!appletMainLoop()) { event.type = SDL_EVENT_QUIT; return true; }
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

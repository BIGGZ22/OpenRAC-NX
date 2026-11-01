// SPDX-License-Identifier: GPL-3.0-or-later
// Shows the launcher's CPU-drawn picture: through deko3d in the deko3d build, else through OpenGL.
#include <array>
#include <cstring>
#include <stdexcept>
#include <switch.h>

#include "platform/switch/launcher_ui.h"
#include "platform/switch/overlay_stats.h"
#include "renderer/gl.h"

#include <SDL2/SDL.h>
#if defined(OPENRAC_SWITCH_DEKO3D_BUILD)
#include <deko3d.h>
#endif

namespace openrac::platform::launcher_ui {
namespace {
class Surface {
public:
    virtual ~Surface() = default;
    virtual void present(const std::vector<std::uint32_t>& pixels) = 0;
};

class OpenGlSurface final : public Surface {
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    unsigned texture = 0, program = 0, vao = 0;

public:
    ~OpenGlSurface() override {
        if (context) {
            if (texture) {
                gl::glDeleteTextures(1, &texture);
            }
            if (vao) {
                gl::glDeleteVertexArrays(1, &vao);
            }
            if (program) {
                gl::glDeleteProgram(program);
            }
            SDL_GL_DeleteContext(context);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    void init() {
        using namespace gl;
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error(SDL_GetError());
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        window = SDL_CreateWindow(
            "OpenRAC-NX", 0, 0, kWidth, kHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
        );
        if (!window || !(context = SDL_GL_CreateContext(window))) {
            throw std::runtime_error(SDL_GetError());
        }
        std::string missing;
        if (!gl::load(
                +[](const char* name) -> gl::Proc {
                    return reinterpret_cast<gl::Proc>(SDL_GL_GetProcAddress(name));
                },
                missing
            )) {
            throw std::runtime_error("Menu OpenGL functions unavailable: " + missing);
        }
        const char* vertex =
            "#version 330 core\nout vec2 uv;void main(){vec2 "
            "p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);uv=p;gl_Position=vec4(p*2.-1.,0.,1.);}";
        const char* fragment = "#version 330 core\nin vec2 uv;out vec4 color;uniform sampler2D "
                               "picture;void main(){color=texture(picture,vec2(uv.x,1.-uv.y));}";
        auto shader = [&](unsigned stage, const char* source) {
            unsigned sh = glCreateShader(stage);
            glShaderSource(sh, 1, &source, nullptr);
            glCompileShader(sh);
            int ok = 0;
            glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                char log[512]{};
                glGetShaderInfoLog(sh, sizeof log, nullptr, log);
                glDeleteShader(sh);
                throw std::runtime_error(log);
            }
            return sh;
        };
        unsigned vs = shader(GL_VERTEX_SHADER, vertex), fs = 0;
        try {
            fs = shader(GL_FRAGMENT_SHADER, fragment);
        } catch (...) {
            glDeleteShader(vs);
            throw;
        }
        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        int ok = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            throw std::runtime_error("Cannot link graphical launcher shaders");
        }
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA8, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
        );
        SDL_GL_SetSwapInterval(1);
    }

    void present(const std::vector<std::uint32_t>& pixels) override {
        using namespace gl;
        int w, h;
        SDL_GL_GetDrawableSize(window, &w, &h);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(9.0f / 255.0f, 17.0f / 255.0f, 30.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(w * kMenuX / kWidth, h * kMenuY / kHeight,
                   w * kMenuWidth / kWidth, h * kMenuHeight / kHeight);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data()
        );
        glUseProgram(program);
        glUniform1i(glGetUniformLocation(program, "picture"), 0);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        SDL_GL_SwapWindow(window);
        int width = 0, height = 0;
        SDL_GL_GetDrawableSize(window, &width, &height);
        overlay_stats::presented(kWidth, kHeight, width, height);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
        }
    }
};


#if defined(OPENRAC_SWITCH_DEKO3D_BUILD)
// The deko3d build shows the launcher with deko3d too, so the whole program draws through the
// Switch's own GPU API (no Mesa / OpenGL context ever opens). The picture is drawn on the CPU;
// each frame is copied by the copy engine from a linear staging buffer into the swapchain image,
// with no shaders. Everything is released (after the GPU is idle) before the game opens dkgl on
// the same native window.
class DekoSurface final : public Surface {
    static constexpr unsigned kImages = 2, kSlots = 2;
    static constexpr std::uint32_t kPictureBytes = std::uint32_t(kWidth) * kHeight * 4;
    static constexpr std::uint32_t kCmdBytes = 16 * 1024;
    DkDevice device = nullptr;
    DkQueue queue = nullptr;
    DkMemBlock image_memory = nullptr, staging = nullptr, command_memory = nullptr;
    DkCmdBuf cmd[kSlots]{};
    DkFence done[kSlots]{};
    bool submitted[kSlots]{};
    DkImage images[kImages]{};
    DkSwapchain swapchain = nullptr;
    unsigned slot = 0;

    static std::uint32_t align(std::uint32_t value, std::uint32_t to) { return (value + to - 1) / to * to; }

public:
    ~DekoSurface() override {
        if (queue) dkQueueWaitIdle(queue);
        if (swapchain) dkSwapchainDestroy(swapchain);
        for (DkCmdBuf c : cmd) if (c) dkCmdBufDestroy(c);
        if (queue) dkQueueDestroy(queue);
        for (DkMemBlock b : {image_memory, staging, command_memory}) if (b) dkMemBlockDestroy(b);
        if (device) dkDeviceDestroy(device);
    }

    void init() {
        if (R_FAILED(nwindowSetDimensions(nwindowGetDefault(), kWidth, kHeight))) {
            throw std::runtime_error("Could not size the native Switch window");
        }
        DkDeviceMaker device_maker;
        dkDeviceMakerDefaults(&device_maker);
        device_maker.flags = DkDeviceFlags_OriginUpperLeft;
        if (!(device = dkDeviceCreate(&device_maker))) throw std::runtime_error("deko3d: no device");

        DkImageLayoutMaker layout_maker;
        dkImageLayoutMakerDefaults(&layout_maker, device);
        layout_maker.flags = DkImageFlags_UsageRender | DkImageFlags_UsagePresent | DkImageFlags_Usage2DEngine;
        layout_maker.format = DkImageFormat_RGBA8_Unorm;
        layout_maker.dimensions[0] = kWidth;
        layout_maker.dimensions[1] = kHeight;
        DkImageLayout layout;
        dkImageLayoutInitialize(&layout, &layout_maker);
        const std::uint32_t image_bytes = align(std::uint32_t(dkImageLayoutGetSize(&layout)), dkImageLayoutGetAlignment(&layout));
        DkMemBlockMaker memory;
        dkMemBlockMakerDefaults(&memory, device, align(image_bytes * kImages, DK_MEMBLOCK_ALIGNMENT));
        memory.flags = DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image;
        if (!(image_memory = dkMemBlockCreate(&memory))) throw std::runtime_error("deko3d: no memory for the menu");
        DkImage const* swap_images[kImages];
        for (unsigned i = 0; i < kImages; ++i) {
            dkImageInitialize(&images[i], &layout, image_memory, i * image_bytes);
            swap_images[i] = &images[i];
        }
        dkMemBlockMakerDefaults(&memory, device, align(kPictureBytes * kSlots, DK_MEMBLOCK_ALIGNMENT));
        memory.flags = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached;
        if (!(staging = dkMemBlockCreate(&memory))) throw std::runtime_error("deko3d: no memory for the menu");
        dkMemBlockMakerDefaults(&memory, device, align(kCmdBytes * kSlots, DK_MEMBLOCK_ALIGNMENT));
        memory.flags = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached;
        if (!(command_memory = dkMemBlockCreate(&memory))) throw std::runtime_error("deko3d: no memory for the menu");

        DkQueueMaker queue_maker;
        dkQueueMakerDefaults(&queue_maker, device);
        queue_maker.flags = DkQueueFlags_Graphics;
        if (!(queue = dkQueueCreate(&queue_maker))) throw std::runtime_error("deko3d: no queue");
        for (unsigned i = 0; i < kSlots; ++i) {
            DkCmdBufMaker cmd_maker;
            dkCmdBufMakerDefaults(&cmd_maker, device);
            if (!(cmd[i] = dkCmdBufCreate(&cmd_maker))) throw std::runtime_error("deko3d: no command buffer");
        }
        DkSwapchainMaker swapchain_maker;
        dkSwapchainMakerDefaults(&swapchain_maker, device, nwindowGetDefault(), swap_images, kImages);
        if (!(swapchain = dkSwapchainCreate(&swapchain_maker))) throw std::runtime_error("deko3d: no swapchain");
        dkSwapchainSetSwapInterval(swapchain, 1);
    }

    void present(const std::vector<std::uint32_t>& pixels) override {
        if (pixels.size() * 4 != kPictureBytes) return;
        // This slot's staging copy and commands are free once its last frame is done.
        if (submitted[slot]) {
            dkFenceWait(&done[slot], -1);
            submitted[slot] = false;
        }
        auto* bytes = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(staging)) + slot * kPictureBytes;
        std::memcpy(bytes, pixels.data(), kPictureBytes);
        DkCmdBuf c = cmd[slot];
        dkCmdBufClear(c);
        dkCmdBufAddMemory(c, command_memory, slot * kCmdBytes, kCmdBytes);
        const int index = dkQueueAcquireImage(queue, swapchain);
        if (index < 0 || index >= int(kImages)) throw std::runtime_error("deko3d: no screen image");
        DkImageView view;
        dkImageViewDefaults(&view, &images[index]);
        const DkCopyBuf from{dkMemBlockGetGpuAddr(staging) + slot * kPictureBytes, 0, 0};
        const DkImageRect rect{0, 0, 0, std::uint32_t(kWidth), std::uint32_t(kHeight), 1};
        // Earlier copies into this image are complete before the next one starts.
        dkCmdBufBarrier(c, DkBarrier_Full, 0);
        dkCmdBufCopyBufferToImage(c, &from, &view, &rect, 0);
        dkCmdBufBarrier(c, DkBarrier_Full, 0);
        dkQueueSubmitCommands(queue, dkCmdBufFinishList(c));
        dkQueueSignalFence(queue, &done[slot], true);
        dkQueuePresentImage(queue, swapchain, index);
        if (dkQueueIsInErrorState(queue)) throw std::runtime_error("deko3d: the menu's GPU queue failed");
        submitted[slot] = true;
        slot = (slot + 1) % kSlots;
        overlay_stats::presented(kWidth, kHeight, kWidth, kHeight);
    }
};
#endif
}  // namespace

struct Graphics::Impl {
    Canvas canvas;
    std::unique_ptr<Surface> surface;
    bool pl = false, font = false, heading = false;

    ~Impl() {
        surface.reset();
        if (pl) {
            plExit();
        }
    }
};

Graphics::Graphics() : p(std::make_unique<Impl>()) {}

Graphics::~Graphics() = default;

bool Graphics::open(std::string& error) {
    p->surface.reset();
    try {
        if (!p->font) {
            if (!p->pl) {
                if (R_FAILED(plInitialize(PlServiceType_User))) {
                    throw std::runtime_error("Shared font service unavailable");
                }
                p->pl = true;
            }
            PlFontData font{};
            if (R_FAILED(plGetSharedFontByType(&font, PlSharedFontType_Standard))
                || !p->canvas.font({static_cast<const std::uint8_t*>(font.address), font.size})) {
                throw std::runtime_error("Cannot load the system interface font");
            }
            p->font = true;
        }
        if (!p->heading) {
            // Titles use the bundled display face; without it the interface font draws them.
            p->heading = p->canvas.heading_font(artwork::heading_font());
        }
#if defined(OPENRAC_SWITCH_DEKO3D_BUILD)
        auto next = std::make_unique<DekoSurface>();
#else
        auto next = std::make_unique<OpenGlSurface>();
#endif
        next->init();
        p->surface = std::move(next);
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}

bool Graphics::show(const View& view, std::string& error) {
    try {
        p->canvas.menu(view);
        p->surface->present(p->canvas.pixels());
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}

bool Graphics::progress(
    const std::string& stage, unsigned done, unsigned total, std::string& error
) {
    try {
        p->canvas.progress(stage, done, total);
        p->surface->present(p->canvas.pixels());
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}

bool Graphics::binding(const std::string& action, const std::string& prompt, std::string& error) {
    try {
        p->canvas.binding(action, prompt);
        p->surface->present(p->canvas.pixels());
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}
}  // namespace openrac::platform::launcher_ui

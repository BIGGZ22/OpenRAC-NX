// SPDX-License-Identifier: GPL-3.0-or-later
#include <array>
#include <cstring>
#include <stdexcept>
#include <switch.h>

#include "platform/switch/launcher_ui.h"
#include "renderer/gl.h"

#include <SDL2/SDL.h>

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
        glViewport(0, 0, w, h);
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
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
        }
    }
};

}  // namespace

struct Graphics::Impl {
    Canvas canvas;
    std::unique_ptr<Surface> surface;
    bool pl = false, font = false;

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
        auto next = std::make_unique<OpenGlSurface>();
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

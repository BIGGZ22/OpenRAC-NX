// SPDX-License-Identifier: GPL-3.0-or-later
// Standalone hardware test: no ISO, settings or save data are opened.
#include <array>
#include <cstdio>
#include <filesystem>
#include <exception>
#include <string>
#include <switch.h>
#include "platform/window.h"
#include "renderer/gl.h"
#include "renderer/vulkan/vkgl.h"

int main() {
    namespace platform = openrac::platform;
    using namespace openrac::gl;
    std::filesystem::create_directories("sdmc:/switch/openrac");
    FILE* report = std::fopen("sdmc:/switch/openrac/vulkan-probe.log", "w");
    if (!report) return 1;
    const auto note = [report](const char* text) {
        std::fprintf(report, "%s\n", text); std::fflush(report);
    };
    int status = 1;
    try { status = [&]() {
        platform::Platform runtime(platform::kVideo);
        if (!runtime.ok()) { note(runtime.error().c_str()); return 1; }
        platform::WindowConfig config;
        config.title = "OpenRAC Switch Vulkan probe";
        config.gl_api = platform::GlApi::Vulkan;
        std::string error;
        auto window = platform::Window::open(config, error);
        if (!window) { note(error.c_str()); return 1; }
        if (window->gl_api() != platform::GlApi::Vulkan) {
            note("FAIL: Vulkan did not start. OpenGL fallback is NOT a Vulkan pass.");
            return 1;
        }
        note(window->gl_version().c_str());
        note(window->gl_renderer().c_str());
        if (!load(platform::Window::gl_loader(), error)) { note(error.c_str()); return 1; }
        glViewport(0, 0, 1280, 720);
        glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        std::array<unsigned char, 4> pixel{};
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
        if (pixel[0] < 62 || pixel[0] > 66 || pixel[1] < 126 || pixel[1] > 130
            || pixel[2] < 189 || pixel[2] > 193) {
            note("FAIL: clear/readback mismatch."); return 1;
        }
        note("PASS: Vulkan device, render target, submission and pixel readback.");
        // Exercise GLSL -> SPIR-V, pipeline creation, vertex fetch and rasterization.
        const char* vertex = "#version 410 core\nlayout(location=0) in vec2 p; void main(){gl_Position=vec4(p,0,1);}";
        const char* fragment = "#version 410 core\nlayout(location=0) out vec4 c; void main(){c=vec4(1,0,0,1);}";
        GLuint vs = glCreateShader(GL_VERTEX_SHADER), fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vs, 1, &vertex, nullptr); glCompileShader(vs);
        glShaderSource(fs, 1, &fragment, nullptr); glCompileShader(fs);
        GLuint program = glCreateProgram();
        glAttachShader(program, vs); glAttachShader(program, fs); glLinkProgram(program);
        GLint linked = 0; glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) { note("FAIL: triangle shader compilation/linking."); return 1; }
        GLuint vao = 0, buffer = 0;
        glGenVertexArrays(1, &vao); glBindVertexArray(vao);
        glGenBuffers(1, &buffer); glBindBuffer(GL_ARRAY_BUFFER, buffer);
        const float vertices[] = {-1, -1, 1, -1, 0, 1};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr); glEnableVertexAttribArray(0);
        glUseProgram(program); glDrawArrays(GL_TRIANGLES, 0, 3);
        glReadPixels(640, 360, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
        if (pixel[0] < 250 || pixel[1] > 3 || pixel[2] > 3 || glGetError() != GL_NO_ERROR) {
            note("FAIL: triangle rasterization/readback."); return 1;
        }
        note("PASS: shader compilation, graphics pipeline and triangle readback.");
        PadState pad; padConfigureInput(1, HidNpadStyleSet_NpadStandard); padInitializeDefault(&pad);
        for (unsigned frame = 0; frame < 600 && appletMainLoop(); ++frame) {
            padUpdate(&pad);
            if (padGetButtonsDown(&pad) & HidNpadButton_Plus) break;
            glClear(GL_COLOR_BUFFER_BIT); glDrawArrays(GL_TRIANGLES, 0, 3);
            window->swap();
        }
        const auto counters = openrac::vkgl::submission_counters();
        std::fprintf(report, "Ended frames: %llu; accepted presents: %llu; failed presents: %llu\n",
            static_cast<unsigned long long>(counters.frames_presented),
            static_cast<unsigned long long>(counters.presents_accepted),
            static_cast<unsigned long long>(counters.presents_failed));
        std::fflush(report);
        if (counters.presents_accepted == 0 || counters.presents_failed != 0) {
            note("FAIL: presentation was not accepted consistently by the driver."); return 1;
        }
        glDeleteBuffers(1, &buffer); glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program); glDeleteShader(vs); glDeleteShader(fs);
        note("Presentation loop ended; confirm the red triangle was visible on the blue background.");
        return 0;
    }(); }
    catch (const std::exception& error) { note(error.what()); }
    note(status == 0 ? "PROBE COMPLETED" : "PROBE FAILED");
    std::fclose(report);
    return status;
}

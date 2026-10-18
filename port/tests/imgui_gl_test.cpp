// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Dear ImGui drawn with the renderer's own GL (game/common/imgui_gl.h), as the Vulkan renderer's
// window draws the debug overlay and the Android drawer: a window's filled background lands where
// ImGui put it, in its colour, and nothing outside it. Run on OpenGL, ES (imgui_gl_es) and, under
// OPENRAC_GL_API=vulkan, the Vulkan renderer. Skipped (exit 77) where no context can be made.

#include <cstdio>
#include <string>

#include <imgui.h>

#include "check.h"
#include "imgui_gl.h"
#include "renderer/gl.h"
#include "renderer_gl_context.h"

namespace {

using namespace openrac;
using namespace openrac::gl;

constexpr int kWidth = 320;
constexpr int kHeight = 180;

// The pixel at (x, y) counted from the top, as ImGui counts.
void pixel(int x, int y, unsigned char rgba[4]) {
    glReadPixels(x, kHeight - 1 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

}  // namespace

int main() {
    test::GlContext context;
    std::string why;
    if (!context.open(why, kWidth, kHeight)) {
        std::printf("SKIP: no OpenGL context: %s\n", why.c_str());
        return test::kSkip;
    }
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(kWidth, kHeight);
    std::string error;
    CHECK(frontend::imgui_gl::init(error));
    if (!error.empty()) {
        std::fprintf(stderr, "%s\n", error.c_str());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(40.0f, 30.0f));
    ImGui::SetNextWindowSize(ImVec2(100.0f, 60.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::Begin("test", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::TextUnformatted("OpenRAC");
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::Render();
    frontend::imgui_gl::render(ImGui::GetDrawData());

    unsigned char inside[4] = {};
    unsigned char outside[4] = {};
    pixel(130, 80, inside);  // in the window, clear of its text
    pixel(200, 120, outside);
    CHECK(inside[0] > 200 && inside[1] < 40 && inside[2] < 40);
    CHECK(outside[0] == 0 && outside[1] == 0 && outside[2] == 0);
    CHECK(glGetError() == GL_NO_ERROR);

    frontend::imgui_gl::shutdown();
    ImGui::DestroyContext();
    return test::result();
}

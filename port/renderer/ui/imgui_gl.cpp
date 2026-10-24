// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What ImGui's OpenGL 3 backend (backends/imgui_impl_opengl3.cpp; MIT License, Copyright (c)
// 2014-2025 Omar Cornut) does, with the calls renderer/gl.h has: one program, one vertex array,
// the font's texture, a scissor per command.

#include "renderer/ui/imgui_gl.h"

#include <cstdint>

#include <imgui.h>

#include "renderer/gl.h"
#include "renderer/shader.h"

namespace openrac::renderer::ui::imgui_gl {
namespace {

using namespace openrac::gl;

constexpr const char* kVertex = R"glsl(#version 410 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 colour;
uniform mat4 projection;
out vec2 v_uv;
out vec4 v_colour;
void main() {
    v_uv = uv;
    v_colour = colour;
    gl_Position = projection * vec4(position, 0.0, 1.0);
}
)glsl";

constexpr const char* kFragment = R"glsl(#version 410 core
in vec2 v_uv;
in vec4 v_colour;
uniform sampler2D tex_T0;
out vec4 out_colour;
void main() {
    out_colour = v_colour * texture(tex_T0, v_uv);
}
)glsl";

struct State {
    renderer::Shader shader;
    GLuint vao = 0;
    GLuint vertices = 0;
    GLuint indices = 0;
    GLuint font = 0;
};

State* s = nullptr;

void set_state(const ImDrawData* data, int width, int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    s->shader.use();
    // Display points to device coordinates, y down.
    const float l = data->DisplayPos.x;
    const float r = data->DisplayPos.x + data->DisplaySize.x;
    const float t = data->DisplayPos.y;
    const float b = data->DisplayPos.y + data->DisplaySize.y;
    const float projection[16] = {
        2.0f / (r - l), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / (t - b), 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        (r + l) / (l - r), (t + b) / (b - t), 0.0f, 1.0f,
    };
    glUniformMatrix4fv(s->shader.uniform("projection"), 1, GL_FALSE, projection);
    glBindVertexArray(s->vao);
    glBindBuffer(GL_ARRAY_BUFFER, s->vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->indices);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    const auto stride = static_cast<GLsizei>(sizeof(ImDrawVert));
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(ImDrawVert, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(ImDrawVert, uv)));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
                          reinterpret_cast<const void*>(offsetof(ImDrawVert, col)));
    glActiveTexture(GL_TEXTURE0);
}

}  // namespace

bool init(std::string& error) {
    if (s != nullptr) {
        return true;
    }
    s = new State;
    if (!s->shader.build("imgui", kVertex, kFragment, error)) {
        delete s;
        s = nullptr;
        return false;
    }
    glGenVertexArrays(1, &s->vao);
    glGenBuffers(1, &s->vertices);
    glGenBuffers(1, &s->indices);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "openrac_gl_subset";
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    glGenTextures(1, &s->font);
    glBindTexture(GL_TEXTURE_2D, s->font);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    io.Fonts->SetTexID(static_cast<ImTextureID>(s->font));
    return true;
}

void render(const ImDrawData* data) {
    if (s == nullptr || data == nullptr) {
        return;
    }
    const int width = static_cast<int>(data->DisplaySize.x * data->FramebufferScale.x);
    const int height = static_cast<int>(data->DisplaySize.y * data->FramebufferScale.y);
    if (width <= 0 || height <= 0) {
        return;
    }
    set_state(data, width, height);
    const ImVec2 origin = data->DisplayPos;
    const ImVec2 scale = data->FramebufferScale;
    const GLenum index_type = sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
    for (const ImDrawList* list : data->CmdLists) {
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(list->VtxBuffer.Size) * sizeof(ImDrawVert),
                     list->VtxBuffer.Data, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(list->IdxBuffer.Size) * sizeof(ImDrawIdx),
                     list->IdxBuffer.Data, GL_STREAM_DRAW);
        for (const ImDrawCmd& cmd : list->CmdBuffer) {
            if (cmd.UserCallback != nullptr) {
                if (cmd.UserCallback == ImDrawCallback_ResetRenderState) {
                    set_state(data, width, height);
                } else {
                    cmd.UserCallback(list, &cmd);
                }
                continue;
            }
            // The clip rectangle in framebuffer pixels, GL's rows counted from the bottom.
            const float x0 = (cmd.ClipRect.x - origin.x) * scale.x;
            const float y0 = (cmd.ClipRect.y - origin.y) * scale.y;
            const float x1 = (cmd.ClipRect.z - origin.x) * scale.x;
            const float y1 = (cmd.ClipRect.w - origin.y) * scale.y;
            if (x1 <= x0 || y1 <= y0) {
                continue;
            }
            glScissor(static_cast<GLint>(x0), static_cast<GLint>(static_cast<float>(height) - y1),
                      static_cast<GLsizei>(x1 - x0), static_cast<GLsizei>(y1 - y0));
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(cmd.GetTexID()));
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cmd.ElemCount), index_type,
                           reinterpret_cast<const void*>(static_cast<std::uintptr_t>(cmd.IdxOffset) * sizeof(ImDrawIdx)));
        }
    }
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

void shutdown() {
    if (s == nullptr) {
        return;
    }
    s->shader.release();
    glDeleteVertexArrays(1, &s->vao);
    glDeleteBuffers(1, &s->vertices);
    glDeleteBuffers(1, &s->indices);
    glDeleteTextures(1, &s->font);
    ImGui::GetIO().Fonts->SetTexID(0);
    delete s;
    s = nullptr;
}

}  // namespace openrac::renderer::ui::imgui_gl

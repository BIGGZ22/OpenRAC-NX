// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Dear ImGui's draw lists drawn with the renderer's own GL (renderer/gl.h), for a window whose GL is
// the Vulkan renderer's (renderer/vulkan/vkgl.h): ImGui's OpenGL backend loads the system's GL
// itself, which that window does not have. The debug overlay and the Android drawer
// (debug_overlay.h) draw through this there; on an OpenGL window they keep ImGui's own backend.
#pragma once

#include <string>

struct ImDrawData;

namespace openrac::frontend::imgui_gl {

// Before the first frame (the context is current): the program, the buffers, the font's texture.
bool init(std::string& error);

// The frame's lists into the default framebuffer.
void render(const ImDrawData* data);

void shutdown();

}  // namespace openrac::frontend::imgui_gl

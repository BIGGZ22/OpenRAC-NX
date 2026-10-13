// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What the renderer's use of OpenGL says about a frame, for the debug overlay and the dump
// (common/stats.h), taken where every renderer's GL calls already pass: the function table of
// renderer/gl.h. install() puts a counting function in front of the few calls that matter (draws,
// texture and buffer uploads, program and framebuffer binds, shader compiles and links), so no
// renderer is changed and none can forget to count. It also keeps what the overlay lists: the
// textures alive and their size, the shader programs with their compile and link results, and it
// times sections on the GPU with timer queries where the context has them.
//
// The counting functions cost an add each; the lists are kept from the start so that an overlay
// opened later knows every texture and program made before it.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "renderer/gl.h"

namespace openrac::renderer::gl_stats {

// After gl::load(), with the same loader (for the timer queries and what reloading a shader
// needs, which gl.h does not list). Once per context.
void install(gl::GetProcAddress get);

bool installed();

// The context, as it names itself.
struct Info {
    std::string vendor, renderer, version, glsl;
    bool software = false;       // the renderer's name is a software rasteriser's
    bool timer_queries = false;  // sections can be timed on the GPU
    int max_texture_size = 0;
};

const Info& info();

// The 2D textures alive (framebuffer colour textures included) and the bytes they take, counted
// as 4 a pixel, a third more with mipmaps: what was asked of the driver, not what it reserved.
struct Textures {
    std::size_t count = 0;
    std::uint64_t bytes = 0;
};

Textures textures();

struct Program {
    unsigned id = 0;
    std::string name;            // from its sources' files ("direct.vert + direct.frag"), or its number
    bool linked = false;
    std::string log;             // the link's (or a failed reload's) log
    std::string vertex_file;     // under port/; empty when the source is not one built-in file
    std::string fragment_file;
    std::uint64_t binds = 0;     // since reset_binds()
    std::uint64_t binds_total = 0;
    int reloads = 0;
    std::string reload_status;   // what the last reload said
};

// The programs alive, in the order they were made.
std::vector<Program> programs();
void reset_binds();

// Shaders that did not compile and programs that did not link, oldest first (at most 32).
const std::vector<std::string>& failures();

// Reads a program's two files again from `root` (the port/ folder: stats::shader_root() unless
// OPENRAC_SHADER_DIR names another) and links them into the same program, so every renderer
// holding it draws with the new code. The program is only touched once the new sources compile
// and link in a trial; its uniforms' values are put back by name. False, with the reason (the
// compiler's log) in `message`, when it was left as it was.
bool reload(unsigned program, const std::string& root, std::string& message);

// Every program whose files are known: how many were reloaded; `message` gathers what failed.
int reload_all(const std::string& root, std::string& message);

}  // namespace openrac::renderer::gl_stats

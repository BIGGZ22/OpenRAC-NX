// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The OpenGL the world renderers use beyond renderer/gl.h: uniform blocks,
// texture buffers (OpenGL 4.1 has no storage buffers, so the per-instance and
// per-frame records the shaders index live in buffer textures), sampler
// objects, the stencil test (shadow volumes), texture arrays and a few draw
// calls. Loaded the same way as renderer/gl.h, through the context's loader,
// and kept to OpenGL 4.1 core.
//
// gl::load() must have succeeded first; then call world::gl::load() with the
// same loader. Every world renderer's init() fails with a clear message when
// this was not done.

#pragma once

#include <string>

#include "renderer/gl.h"

namespace openrac::renderer::world::gl {

using namespace openrac::gl;

// clang-format off
inline constexpr GLenum GL_UNIFORM_BUFFER = 0x8A11;
inline constexpr GLenum GL_TEXTURE_BUFFER = 0x8C2A;
inline constexpr GLenum GL_TEXTURE_2D_ARRAY = 0x8C1A;
inline constexpr GLuint GL_INVALID_INDEX = 0xFFFFFFFFu;

inline constexpr GLenum GL_R32UI = 0x8236;
inline constexpr GLenum GL_RGBA32UI = 0x8D70;
inline constexpr GLenum GL_RGBA32F = 0x8814;

inline constexpr GLenum GL_STENCIL_TEST = 0x0B90;
inline constexpr GLenum GL_KEEP = 0x1E00;
inline constexpr GLenum GL_REPLACE = 0x1E01;
inline constexpr GLenum GL_INCR_WRAP = 0x8507;
inline constexpr GLenum GL_DECR_WRAP = 0x8508;

inline constexpr GLenum GL_NEAREST_MIPMAP_NEAREST = 0x2700;
inline constexpr GLenum GL_LINEAR_MIPMAP_NEAREST = 0x2701;
inline constexpr GLenum GL_TEXTURE_BASE_LEVEL = 0x813C;
// clang-format on

// clang-format off
#define OPENRAC_WORLD_GL_FUNCTIONS(X)                                                              \
    X(void, glTexSubImage2D, (GLenum target, GLint level, GLint x, GLint y, GLsizei w, GLsizei h,  \
                              GLenum format, GLenum type, const void *pixels))                     \
    X(void, glTexImage3D, (GLenum target, GLint level, GLint internal_format, GLsizei w,           \
                           GLsizei h, GLsizei d, GLint border, GLenum format, GLenum type,          \
                           const void *pixels))                                                    \
    X(void, glGenSamplers, (GLsizei n, GLuint *samplers))                                          \
    X(void, glDeleteSamplers, (GLsizei n, const GLuint *samplers))                                 \
    X(void, glBindSampler, (GLuint unit, GLuint sampler))                                          \
    X(void, glSamplerParameteri, (GLuint sampler, GLenum pname, GLint param))                      \
    X(void, glStencilFunc, (GLenum func, GLint ref, GLuint mask))                                  \
    X(void, glStencilOpSeparate, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass))        \
    X(void, glStencilMask, (GLuint mask))                                                          \
    X(void, glClearStencil, (GLint s))                                                             \
    X(void, glUniform1ui, (GLint location, GLuint v0))                                             \
    X(void, glDrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei instances))
// clang-format on

#define OPENRAC_WORLD_GL_DECLARE(ret, name, params)                                                \
    using PFN_##name = ret(OPENRAC_GLAPI*) params;                                                 \
    extern PFN_##name name;
OPENRAC_WORLD_GL_FUNCTIONS(OPENRAC_WORLD_GL_DECLARE)
#undef OPENRAC_WORLD_GL_DECLARE

// Load the functions above from the current context. Returns false, and
// lists what is missing in `missing`, if any of them is absent.
bool load(GetProcAddress get, std::string& missing);

// True once load() has succeeded.
bool loaded();

}  // namespace openrac::renderer::world::gl

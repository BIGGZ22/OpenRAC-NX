// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The OpenGL the renderer uses, and nothing more: the types, the constants and
// the functions it calls, loaded through the context's own loader
// (SDL_GL_GetProcAddress). A generated loader (glad) would declare thousands
// of functions the renderer never calls; this list is the renderer's real
// dependency on GL, kept to OpenGL 4.1 core so the same code runs on macOS, and to what OpenGL ES
// 3.2 also has (Android), but for the few OPENRAC_GL_DESKTOP_FUNCTIONS below.
// Optional compute/storage entry points enable GPU probe selection on capable contexts.
//
// The functions live in namespace openrac::gl under their GL names, so a
// renderer source says `using namespace openrac::gl;` and calls glClear(...)
// as GL code always looks; nothing is declared in the global namespace, so the
// system's GL headers never collide with it.

#pragma once

#include <cstddef>
#include <string>

#if defined(_WIN32) && !defined(_WIN64)
#define OPENRAC_GLAPI __stdcall
#else
#define OPENRAC_GLAPI
#endif

namespace openrac::gl {

using GLenum = unsigned int;
using GLboolean = unsigned char;
using GLbitfield = unsigned int;
using GLint = int;
using GLsizei = int;
using GLuint = unsigned int;
using GLfloat = float;
using GLdouble = double;
using GLchar = char;
using GLubyte = unsigned char;
using GLsizeiptr = std::ptrdiff_t;
using GLintptr = std::ptrdiff_t;

// clang-format off
inline constexpr GLboolean GL_FALSE = 0;
inline constexpr GLboolean GL_TRUE = 1;
inline constexpr GLenum GL_NO_ERROR = 0;
inline constexpr GLbitfield GL_DEPTH_BUFFER_BIT = 0x00000100;
inline constexpr GLbitfield GL_STENCIL_BUFFER_BIT = 0x00000400;
inline constexpr GLbitfield GL_COLOR_BUFFER_BIT = 0x00004000;

inline constexpr GLenum GL_POINTS = 0x0000;
inline constexpr GLenum GL_LINES = 0x0001;
inline constexpr GLenum GL_LINE_STRIP = 0x0003;
inline constexpr GLenum GL_TRIANGLES = 0x0004;
inline constexpr GLenum GL_TRIANGLE_STRIP = 0x0005;
inline constexpr GLenum GL_TRIANGLE_FAN = 0x0006;

inline constexpr GLenum GL_NEVER = 0x0200;
inline constexpr GLenum GL_LESS = 0x0201;
inline constexpr GLenum GL_EQUAL = 0x0202;
inline constexpr GLenum GL_LEQUAL = 0x0203;
inline constexpr GLenum GL_GREATER = 0x0204;
inline constexpr GLenum GL_NOTEQUAL = 0x0205;
inline constexpr GLenum GL_GEQUAL = 0x0206;
inline constexpr GLenum GL_ALWAYS = 0x0207;

inline constexpr GLenum GL_ZERO = 0;
inline constexpr GLenum GL_ONE = 1;
inline constexpr GLenum GL_SRC_COLOR = 0x0300;
inline constexpr GLenum GL_ONE_MINUS_SRC_COLOR = 0x0301;
inline constexpr GLenum GL_SRC_ALPHA = 0x0302;
inline constexpr GLenum GL_ONE_MINUS_SRC_ALPHA = 0x0303;
inline constexpr GLenum GL_DST_ALPHA = 0x0304;
inline constexpr GLenum GL_ONE_MINUS_DST_ALPHA = 0x0305;
inline constexpr GLenum GL_DST_COLOR = 0x0306;
inline constexpr GLenum GL_ONE_MINUS_DST_COLOR = 0x0307;
inline constexpr GLenum GL_CONSTANT_COLOR = 0x8001;
inline constexpr GLenum GL_ONE_MINUS_CONSTANT_COLOR = 0x8002;
inline constexpr GLenum GL_CONSTANT_ALPHA = 0x8003;
inline constexpr GLenum GL_ONE_MINUS_CONSTANT_ALPHA = 0x8004;
inline constexpr GLenum GL_FUNC_ADD = 0x8006;
inline constexpr GLenum GL_FUNC_SUBTRACT = 0x800A;
inline constexpr GLenum GL_FUNC_REVERSE_SUBTRACT = 0x800B;

inline constexpr GLenum GL_FRONT = 0x0404;
inline constexpr GLenum GL_BACK = 0x0405;
inline constexpr GLenum GL_FRONT_AND_BACK = 0x0408;
inline constexpr GLenum GL_CW = 0x0900;
inline constexpr GLenum GL_CCW = 0x0901;
inline constexpr GLenum GL_LINE = 0x1B01;
inline constexpr GLenum GL_FILL = 0x1B02;

inline constexpr GLenum GL_CULL_FACE = 0x0B44;
inline constexpr GLenum GL_DEPTH_TEST = 0x0B71;
inline constexpr GLenum GL_STENCIL_TEST = 0x0B90;
inline constexpr GLenum GL_KEEP = 0x1E00;
inline constexpr GLenum GL_INCR_WRAP = 0x8507;
inline constexpr GLenum GL_DECR_WRAP = 0x8508;
inline constexpr GLenum GL_BLEND = 0x0BE2;
inline constexpr GLenum GL_SCISSOR_TEST = 0x0C11;
inline constexpr GLenum GL_CLIP_DISTANCE0 = 0x3000;
inline constexpr GLenum GL_CLIP_DISTANCE1 = 0x3001;
inline constexpr GLenum GL_MULTISAMPLE = 0x809D;
inline constexpr GLenum GL_PRIMITIVE_RESTART = 0x8F9D;

inline constexpr GLenum GL_RG = 0x8227;
inline constexpr GLenum GL_RG32F = 0x8230;
inline constexpr GLenum GL_RGBA16F = 0x881A;
inline constexpr GLenum GL_R11F_G11F_B10F = 0x8C3A;
inline constexpr GLenum GL_HALF_FLOAT = 0x140B;
inline constexpr GLenum GL_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B;
inline constexpr GLenum GL_TEXTURE_FETCH_BARRIER_BIT = 0x00000008;
inline constexpr GLenum GL_MAX_TEXTURE_BUFFER_SIZE = 0x8C2B;
inline constexpr GLenum GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 0x90DD;
inline constexpr GLenum GL_VIEWPORT = 0x0BA2;

inline constexpr GLenum GL_UNPACK_ALIGNMENT = 0x0CF5;
inline constexpr GLenum GL_PACK_ALIGNMENT = 0x0D05;
inline constexpr GLenum GL_MAX_TEXTURE_SIZE = 0x0D33;
inline constexpr GLenum GL_VENDOR = 0x1F00;
inline constexpr GLenum GL_RENDERER = 0x1F01;
inline constexpr GLenum GL_VERSION = 0x1F02;
inline constexpr GLenum GL_SHADING_LANGUAGE_VERSION = 0x8B8C;
inline constexpr GLenum GL_EXTENSIONS = 0x1F03;
inline constexpr GLenum GL_NUM_EXTENSIONS = 0x821D;
inline constexpr GLenum GL_MAJOR_VERSION = 0x821B;
inline constexpr GLenum GL_MINOR_VERSION = 0x821C;

inline constexpr GLenum GL_UNSIGNED_BYTE = 0x1401;
inline constexpr GLenum GL_SHORT = 0x1402;
inline constexpr GLenum GL_UNSIGNED_SHORT = 0x1403;
inline constexpr GLenum GL_INT = 0x1404;
inline constexpr GLenum GL_UNSIGNED_INT = 0x1405;
inline constexpr GLenum GL_FLOAT = 0x1406;

inline constexpr GLenum GL_RGB = 0x1907;
inline constexpr GLenum GL_RGBA = 0x1908;
inline constexpr GLenum GL_RGBA8 = 0x8058;
inline constexpr GLenum GL_DEPTH_STENCIL = 0x84F9;
inline constexpr GLenum GL_UNSIGNED_INT_24_8 = 0x84FA;
inline constexpr GLenum GL_DEPTH_COMPONENT = 0x1902;
inline constexpr GLenum GL_DEPTH_COMPONENT24 = 0x81A6;
inline constexpr GLenum GL_DEPTH24_STENCIL8 = 0x88F0;
inline constexpr GLenum GL_DEPTH_COMPONENT32F = 0x8CAC;

inline constexpr GLenum GL_TEXTURE_2D = 0x0DE1;
inline constexpr GLenum GL_TEXTURE0 = 0x84C0;
inline constexpr GLenum GL_TEXTURE_MAG_FILTER = 0x2800;
inline constexpr GLenum GL_TEXTURE_MIN_FILTER = 0x2801;
inline constexpr GLenum GL_TEXTURE_WRAP_S = 0x2802;
inline constexpr GLenum GL_TEXTURE_WRAP_T = 0x2803;
inline constexpr GLenum GL_TEXTURE_MAX_LEVEL = 0x813D;
inline constexpr GLenum GL_TEXTURE_MAX_ANISOTROPY = 0x84FE;
inline constexpr GLenum GL_MAX_TEXTURE_MAX_ANISOTROPY = 0x84FF;
inline constexpr GLenum GL_MAX_SAMPLES = 0x8D57;
inline constexpr GLenum GL_NEAREST = 0x2600;
inline constexpr GLenum GL_LINEAR = 0x2601;
inline constexpr GLenum GL_LINEAR_MIPMAP_LINEAR = 0x2703;
inline constexpr GLenum GL_REPEAT = 0x2901;
inline constexpr GLenum GL_CLAMP_TO_EDGE = 0x812F;

inline constexpr GLenum GL_ARRAY_BUFFER = 0x8892;
inline constexpr GLenum GL_ELEMENT_ARRAY_BUFFER = 0x8893;
inline constexpr GLenum GL_STREAM_DRAW = 0x88E0;
inline constexpr GLenum GL_STATIC_DRAW = 0x88E4;
inline constexpr GLenum GL_DYNAMIC_DRAW = 0x88E8;

inline constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30;
inline constexpr GLenum GL_VERTEX_SHADER = 0x8B31;
inline constexpr GLenum GL_COMPUTE_SHADER = 0x91B9;
inline constexpr GLenum GL_SHADER_STORAGE_BUFFER = 0x90D2;
inline constexpr GLbitfield GL_SHADER_STORAGE_BARRIER_BIT = 0x2000;
inline constexpr GLbitfield GL_UNIFORM_BARRIER_BIT = 0x0004;
inline constexpr GLenum GL_COMPILE_STATUS = 0x8B81;
inline constexpr GLenum GL_LINK_STATUS = 0x8B82;
inline constexpr GLenum GL_INFO_LOG_LENGTH = 0x8B84;

inline constexpr GLenum GL_FRAMEBUFFER = 0x8D40;
inline constexpr GLenum GL_RENDERBUFFER = 0x8D41;
inline constexpr GLenum GL_READ_FRAMEBUFFER = 0x8CA8;
inline constexpr GLenum GL_DRAW_FRAMEBUFFER = 0x8CA9;
inline constexpr GLenum GL_COLOR_ATTACHMENT0 = 0x8CE0;
inline constexpr GLenum GL_DEPTH_ATTACHMENT = 0x8D00;
inline constexpr GLenum GL_DEPTH_STENCIL_ATTACHMENT = 0x821A;
inline constexpr GLenum GL_STENCIL_ATTACHMENT = 0x8D20;
inline constexpr GLenum GL_FRAMEBUFFER_COMPLETE = 0x8CD5;
// clang-format on

// Optional GL 4.3/ES 3.1 functionality; older contexts keep the CPU selector.
#define OPENRAC_GL_COMPUTE_FUNCTIONS(X)                                                            \
    X(void, glDispatchCompute, (GLuint x, GLuint y, GLuint z))                                     \
    X(void, glMemoryBarrier, (GLbitfield barriers))

// The functions: return type, name, parameters.
// clang-format off
#define OPENRAC_GL_FUNCTIONS(X)                                                                    \
    X(void, glTexBuffer, (GLenum target, GLenum internal_format, GLuint buffer))                   \
    X(GLuint, glGetUniformBlockIndex, (GLuint program, const GLchar *name))                        \
    X(void, glUniformBlockBinding, (GLuint program, GLuint block, GLuint binding))                 \
    X(void, glBindBufferBase, (GLenum target, GLuint index, GLuint buffer))                        \
    X(const GLubyte *, glGetString, (GLenum name))                                                 \
    X(const GLubyte *, glGetStringi, (GLenum name, GLuint index))                                  \
    X(void, glGetIntegerv, (GLenum pname, GLint *data))                                            \
    X(void, glGetFloatv, (GLenum pname, GLfloat *data))                                            \
    X(GLenum, glGetError, (void))                                                                  \
    X(void, glEnable, (GLenum cap))                                                                \
    X(void, glDisable, (GLenum cap))                                                               \
    X(void, glClear, (GLbitfield mask))                                                            \
    X(void, glClearColor, (GLfloat r, GLfloat g, GLfloat b, GLfloat a))                            \
    X(void, glViewport, (GLint x, GLint y, GLsizei width, GLsizei height))                         \
    X(void, glScissor, (GLint x, GLint y, GLsizei width, GLsizei height))                          \
    X(void, glDepthFunc, (GLenum func))                                                            \
    X(void, glDepthMask, (GLboolean flag))                                                         \
    X(void, glStencilFunc, (GLenum func, GLint ref, GLuint mask))                                  \
    X(void, glStencilOpSeparate, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass))        \
    X(void, glStencilMask, (GLuint mask))                                                          \
    X(void, glClearStencil, (GLint s))                                                             \
    X(void, glColorMask, (GLboolean r, GLboolean g, GLboolean b, GLboolean a))                     \
    X(void, glBlendFuncSeparate, (GLenum src_rgb, GLenum dst_rgb, GLenum src_a, GLenum dst_a))     \
    X(void, glBlendEquation, (GLenum mode))                                                        \
    X(void, glBlendColor, (GLfloat r, GLfloat g, GLfloat b, GLfloat a))                            \
    X(void, glCullFace, (GLenum mode))                                                             \
    X(void, glFrontFace, (GLenum mode))                                                            \
    X(void, glPixelStorei, (GLenum pname, GLint param))                                            \
    X(void, glReadPixels, (GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type,     \
                           void *pixels))                                                          \
    X(void, glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoff, GLint yoff, GLint x,      \
                                  GLint y, GLsizei w, GLsizei h))                                  \
    X(void, glFinish, (void))                                                                      \
    X(void, glGenTextures, (GLsizei n, GLuint *textures))                                          \
    X(void, glDeleteTextures, (GLsizei n, const GLuint *textures))                                 \
    X(void, glBindTexture, (GLenum target, GLuint texture))                                        \
    X(void, glActiveTexture, (GLenum texture))                                                     \
    X(void, glTexImage2D, (GLenum target, GLint level, GLint internal_format, GLsizei w,           \
                           GLsizei h, GLint border, GLenum format, GLenum type,                    \
                           const void *pixels))                                                    \
    X(void, glTexSubImage2D, (GLenum target, GLint level, GLint xoff, GLint yoff, GLsizei w,       \
                              GLsizei h, GLenum format, GLenum type, const void *pixels))          \
    X(void, glTexParameteri, (GLenum target, GLenum pname, GLint param))                           \
    X(void, glTexParameterf, (GLenum target, GLenum pname, GLfloat param))                         \
    X(void, glGenerateMipmap, (GLenum target))                                                     \
    X(void, glGenSamplers, (GLsizei count, GLuint *samplers))                                      \
    X(void, glDeleteSamplers, (GLsizei count, const GLuint *samplers))                             \
    X(void, glBindSampler, (GLuint unit, GLuint sampler))                                          \
    X(void, glSamplerParameteri, (GLuint sampler, GLenum pname, GLint param))                      \
    X(void, glGenBuffers, (GLsizei n, GLuint *buffers))                                            \
    X(void, glDeleteBuffers, (GLsizei n, const GLuint *buffers))                                   \
    X(void, glBindBuffer, (GLenum target, GLuint buffer))                                          \
    X(void, glBufferData, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))        \
    X(void, glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data))  \
    X(void, glGenVertexArrays, (GLsizei n, GLuint *arrays))                                        \
    X(void, glDeleteVertexArrays, (GLsizei n, const GLuint *arrays))                               \
    X(void, glBindVertexArray, (GLuint array))                                                     \
    X(void, glEnableVertexAttribArray, (GLuint index))                                             \
    X(void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized,   \
                                    GLsizei stride, const void *pointer))                          \
    X(void, glVertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride,        \
                                     const void *pointer))                                         \
    X(void, glVertexAttribDivisor, (GLuint index, GLuint divisor))                                 \
    X(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count))                               \
    X(void, glDrawElements, (GLenum mode, GLsizei count, GLenum type, const void *indices))        \
    X(void, glDrawElementsInstanced, (GLenum mode, GLsizei count, GLenum type,                     \
                                      const void *indices, GLsizei instances))                     \
    X(GLuint, glCreateShader, (GLenum type))                                                       \
    X(void, glShaderSource, (GLuint shader, GLsizei count, const GLchar *const *strings,           \
                             const GLint *lengths))                                                \
    X(void, glCompileShader, (GLuint shader))                                                      \
    X(void, glGetShaderiv, (GLuint shader, GLenum pname, GLint *params))                           \
    X(void, glGetShaderInfoLog, (GLuint shader, GLsizei size, GLsizei *length, GLchar *log))       \
    X(void, glDeleteShader, (GLuint shader))                                                       \
    X(GLuint, glCreateProgram, (void))                                                             \
    X(void, glAttachShader, (GLuint program, GLuint shader))                                       \
    X(void, glBindAttribLocation, (GLuint program, GLuint index, const GLchar *name))              \
    X(void, glLinkProgram, (GLuint program))                                                       \
    X(void, glGetProgramiv, (GLuint program, GLenum pname, GLint *params))                         \
    X(void, glGetProgramInfoLog, (GLuint program, GLsizei size, GLsizei *length, GLchar *log))     \
    X(void, glDeleteProgram, (GLuint program))                                                     \
    X(void, glUseProgram, (GLuint program))                                                        \
    X(GLint, glGetUniformLocation, (GLuint program, const GLchar *name))                           \
    X(void, glUniform1i, (GLint location, GLint v0))                                               \
    X(void, glUniform1f, (GLint location, GLfloat v0))                                             \
    X(void, glUniform2f, (GLint location, GLfloat v0, GLfloat v1))                                 \
    X(void, glUniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2))                     \
    X(void, glUniform4f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))         \
    X(void, glUniform4fv, (GLint location, GLsizei count, const GLfloat* value))                 \
    X(void, glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose,               \
                                 const GLfloat *value))                                            \
    X(void, glGenFramebuffers, (GLsizei n, GLuint *framebuffers))                                  \
    X(void, glDeleteFramebuffers, (GLsizei n, const GLuint *framebuffers))                         \
    X(void, glBindFramebuffer, (GLenum target, GLuint framebuffer))                                \
    X(void, glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget,           \
                                     GLuint texture, GLint level))                                 \
    X(void, glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum rbtarget,         \
                                        GLuint renderbuffer))                                      \
    X(GLenum, glCheckFramebufferStatus, (GLenum target))                                           \
    X(void, glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers))                                \
    X(void, glDeleteRenderbuffers, (GLsizei n, const GLuint *renderbuffers))                       \
    X(void, glBindRenderbuffer, (GLenum target, GLuint renderbuffer))                              \
    X(void, glRenderbufferStorage, (GLenum target, GLenum format, GLsizei w, GLsizei h))           \
    X(void, glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum format,      \
                                               GLsizei w, GLsizei h))                              \
    X(void, glBlitFramebuffer, (GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0,  \
                                GLint dx1, GLint dy1, GLbitfield mask, GLenum filter))
// clang-format on

// Functions OpenGL ES 3.2 does not have. On a desktop context they are loaded as the others; on an
// ES context load() points them at stand-ins: glClearDepth at glClearDepthf, glPolygonMode at an
// extension's form (GL_NV_polygon_mode, GL_ANGLE_polygon_mode) or at nothing (the debug overlay's
// wireframe is then drawn filled).
// clang-format off
#define OPENRAC_GL_DESKTOP_FUNCTIONS(X)                                                            \
    X(void, glClearDepth, (GLdouble depth))                                                        \
    X(void, glPolygonMode, (GLenum face, GLenum mode))
// clang-format on

// Functions only some contexts have, for a tiled GPU's sake (a phone's): null where the context
// lacks them, and loaded on an ES context only (a desktop GPU gains nothing from them, and a
// desktop loader may hand out a pointer for a function its context does not have). Each caller
// tests its pointer, and the extension where it is one.
//   glInvalidateFramebuffer: OpenGL ES 3.0. The attachments a pass leaves are not written back.
//   glFramebufferTexture2DMultisampleEXT, glRenderbufferStorageMultisampleEXT:
//     GL_EXT_multisampled_render_to_texture. The samples stay in the GPU's tile memory and are
//     resolved into the texture as they are written out (FrameBuffer).
// clang-format off
#define OPENRAC_GL_ES_OPTIONAL_FUNCTIONS(X)                                                        \
    X(void, glInvalidateFramebuffer, (GLenum target, GLsizei count, const GLenum *attachments))    \
    X(void, glFramebufferTexture2DMultisampleEXT, (GLenum target, GLenum attachment,               \
                                                   GLenum textarget, GLuint texture, GLint level,  \
                                                   GLsizei samples))                               \
    X(void, glRenderbufferStorageMultisampleEXT, (GLenum target, GLsizei samples, GLenum format,   \
                                                  GLsizei w, GLsizei h))
// clang-format on

#define OPENRAC_GL_DECLARE(ret, name, params)                                                      \
    using PFN_##name = ret(OPENRAC_GLAPI*) params;                                                 \
    extern PFN_##name name;
OPENRAC_GL_FUNCTIONS(OPENRAC_GL_DECLARE)
OPENRAC_GL_DESKTOP_FUNCTIONS(OPENRAC_GL_DECLARE)
OPENRAC_GL_ES_OPTIONAL_FUNCTIONS(OPENRAC_GL_DECLARE)
OPENRAC_GL_COMPUTE_FUNCTIONS(OPENRAC_GL_DECLARE)
#undef OPENRAC_GL_DECLARE

// The context's loader, in SDL_GL_GetProcAddress's shape.
using Proc = void (*)();
using GetProcAddress = Proc (*)(const char* name);

// Load every function above from the current context. Returns false, and
// lists what is missing in `missing`, if any of them is absent.
bool load(GetProcAddress get, std::string& missing);

// True once load() has succeeded.
bool loaded();

// The native Vulkan backend exposes this GL surface, but can opt into its own
// renderer optimisations without changing desktop GL or ES behaviour.
bool is_vulkan();

// Enhanced rendering is a build opt-in and is supported only by Vulkan.
// Keep this decision at resource/pass/UI boundaries, not in every draw.
inline bool enhanced_rendering_available() {
#if defined(OPENRAC_ENHANCED_RENDERING) && OPENRAC_ENHANCED_RENDERING
    return is_vulkan();
#else
    return false;
#endif
}

// Which OpenGL the context is, as load() read it from GL_VERSION: the desktop's 4.1 core, or
// OpenGL ES 3.2 (Android's, or a desktop driver's ES context: OPENRAC_GL_API=es, platform/window.h).
enum class Api { Desktop, ES };
Api api();

inline bool is_es() {
    return api() == Api::ES;
}

// True when the context lists the extension (GL_EXTENSIONS, read once by load()).
bool has_extension(const char* name);

}  // namespace openrac::gl

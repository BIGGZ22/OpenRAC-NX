// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/gl.h"

#include <cstdlib>
#include <cstring>
#include <set>
#include <string_view>

namespace openrac::gl {

#define OPENRAC_GL_DEFINE(ret, name, params) PFN_##name name = nullptr;
OPENRAC_GL_FUNCTIONS(OPENRAC_GL_DEFINE)
OPENRAC_GL_DESKTOP_FUNCTIONS(OPENRAC_GL_DEFINE)
OPENRAC_GL_ES_OPTIONAL_FUNCTIONS(OPENRAC_GL_DEFINE)
#undef OPENRAC_GL_DEFINE

namespace {

bool g_loaded = false;
Api g_api = Api::Desktop;
std::set<std::string, std::less<>> g_extensions;

// OpenGL ES's forms of the desktop functions it lacks (OPENRAC_GL_DESKTOP_FUNCTIONS).
using PFN_glClearDepthf = void(OPENRAC_GLAPI*)(GLfloat depth);
PFN_glClearDepthf g_clear_depthf = nullptr;
PFN_glPolygonMode g_polygon_mode_extension = nullptr;

void OPENRAC_GLAPI es_clear_depth(GLdouble depth) {
    g_clear_depthf(static_cast<GLfloat>(depth));
}

void OPENRAC_GLAPI es_polygon_mode(GLenum face, GLenum mode) {
    if (g_polygon_mode_extension != nullptr) {
        g_polygon_mode_extension(face, mode);
    }
}

}  // namespace

bool load(GetProcAddress get, std::string& missing) {
    missing.clear();
    // A function pointer of type void (*)() converts to any other function
    // pointer type and back; that is what every GL loader relies on.
#define OPENRAC_GL_LOAD(ret, name, params)                                                         \
    name = reinterpret_cast<PFN_##name>(get(#name));                                               \
    if (name == nullptr) {                                                                         \
        missing += missing.empty() ? #name : " " #name;                                            \
    }
    OPENRAC_GL_FUNCTIONS(OPENRAC_GL_LOAD)

    // "OpenGL ES 3.2 ..." on an ES context, by the specification's own wording of GL_VERSION.
    const GLubyte* version = glGetString != nullptr ? glGetString(GL_VERSION) : nullptr;
    g_api = version != nullptr
                    && std::strncmp(reinterpret_cast<const char*>(version), "OpenGL ES", 9) == 0
                ? Api::ES
                : Api::Desktop;
    g_extensions.clear();
    if (glGetStringi != nullptr && glGetIntegerv != nullptr) {
        GLint count = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (GLint i = 0; i < count; ++i) {
            if (const GLubyte* name = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i))) {
                g_extensions.emplace(reinterpret_cast<const char*>(name));
            }
        }
    }
    // OPENRAC_GL_HIDE_EXTENSIONS=GL_A,GL_B: the renderer acts as if the driver lacked them, to try
    // its fallbacks on a driver that has them (a phone's driver that lacks one, on a desktop).
    if (const char* hide = std::getenv("OPENRAC_GL_HIDE_EXTENSIONS"); hide != nullptr) {
        std::string_view list(hide);
        while (!list.empty()) {
            const std::size_t comma = list.find(',');
            const std::string_view name = list.substr(0, comma);
            if (const auto found = g_extensions.find(name); found != g_extensions.end()) {
                g_extensions.erase(found);
            }
            list = comma == std::string_view::npos ? std::string_view() : list.substr(comma + 1);
        }
    }

#define OPENRAC_GL_LOAD_OPTIONAL(ret, name, params)                                                \
    name = g_api == Api::ES ? reinterpret_cast<PFN_##name>(get(#name)) : nullptr;
    OPENRAC_GL_ES_OPTIONAL_FUNCTIONS(OPENRAC_GL_LOAD_OPTIONAL)
#undef OPENRAC_GL_LOAD_OPTIONAL
    if (!has_extension("GL_EXT_multisampled_render_to_texture")) {
        glFramebufferTexture2DMultisampleEXT = nullptr;
        glRenderbufferStorageMultisampleEXT = nullptr;
    }
    if (g_api == Api::Desktop) {
        OPENRAC_GL_DESKTOP_FUNCTIONS(OPENRAC_GL_LOAD)
    } else {
        g_clear_depthf = reinterpret_cast<PFN_glClearDepthf>(get("glClearDepthf"));
        if (g_clear_depthf == nullptr) {
            missing += missing.empty() ? "glClearDepthf" : " glClearDepthf";
        }
        glClearDepth = es_clear_depth;
        g_polygon_mode_extension = nullptr;
        if (has_extension("GL_NV_polygon_mode")) {
            g_polygon_mode_extension = reinterpret_cast<PFN_glPolygonMode>(get("glPolygonModeNV"));
        } else if (has_extension("GL_ANGLE_polygon_mode")) {
            g_polygon_mode_extension =
                reinterpret_cast<PFN_glPolygonMode>(get("glPolygonModeANGLE"));
        }
        glPolygonMode = es_polygon_mode;
    }
#undef OPENRAC_GL_LOAD
    g_loaded = missing.empty();
    return g_loaded;
}

bool loaded() {
    return g_loaded;
}

Api api() {
    return g_api;
}

bool has_extension(const char* name) {
    return g_extensions.contains(std::string_view(name));
}

}  // namespace openrac::gl

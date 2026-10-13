// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/gl_stats.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <unordered_map>

#include "common/log.h"
#include "common/stats.h"

namespace openrac::renderer::gl_stats {

using namespace openrac::gl;

namespace {

// ---- OpenGL beyond gl.h's list: timer queries, and what reloading a program reads and sets ----

constexpr GLenum GL_TIME_ELAPSED = 0x88BF;
constexpr GLenum GL_QUERY_RESULT = 0x8866;
constexpr GLenum GL_QUERY_RESULT_AVAILABLE = 0x8867;
constexpr GLenum GL_ACTIVE_UNIFORMS = 0x8B86;
constexpr GLenum GL_FLOAT_VEC2 = 0x8B50;
constexpr GLenum GL_FLOAT_VEC3 = 0x8B51;
constexpr GLenum GL_FLOAT_VEC4 = 0x8B52;
constexpr GLenum GL_INT_VEC2 = 0x8B53;
constexpr GLenum GL_INT_VEC3 = 0x8B54;
constexpr GLenum GL_INT_VEC4 = 0x8B55;
constexpr GLenum GL_FLOAT_MAT3 = 0x8B5B;
constexpr GLenum GL_FLOAT_MAT4 = 0x8B5C;
using GLuint64 = std::uint64_t;

// clang-format off
#define OPENRAC_GL_STATS_FUNCTIONS(X)                                                              \
    X(void, glGenQueries, (GLsizei n, GLuint *ids))                                                \
    X(void, glDeleteQueries, (GLsizei n, const GLuint *ids))                                       \
    X(void, glBeginQuery, (GLenum target, GLuint id))                                              \
    X(void, glEndQuery, (GLenum target))                                                           \
    X(void, glGetQueryObjectuiv, (GLuint id, GLenum pname, GLuint *params))                        \
    X(void, glGetQueryObjectui64v, (GLuint id, GLenum pname, GLuint64 *params))                    \
    X(void, glGetActiveUniform, (GLuint program, GLuint index, GLsizei size, GLsizei *length,      \
                                 GLint *count, GLenum *type, GLchar *name))                        \
    X(void, glGetUniformfv, (GLuint program, GLint location, GLfloat *params))                     \
    X(void, glGetUniformiv, (GLuint program, GLint location, GLint *params))                       \
    X(void, glGetAttachedShaders, (GLuint program, GLsizei max, GLsizei *count, GLuint *shaders))  \
    X(void, glDetachShader, (GLuint program, GLuint shader))                                       \
    X(void, glUniform1fv, (GLint location, GLsizei count, const GLfloat *value))                   \
    X(void, glUniform2fv, (GLint location, GLsizei count, const GLfloat *value))                   \
    X(void, glUniform3fv, (GLint location, GLsizei count, const GLfloat *value))                   \
    X(void, glUniform1iv, (GLint location, GLsizei count, const GLint *value))                     \
    X(void, glUniform2iv, (GLint location, GLsizei count, const GLint *value))                     \
    X(void, glUniform3iv, (GLint location, GLsizei count, const GLint *value))                     \
    X(void, glUniform4iv, (GLint location, GLsizei count, const GLint *value))                     \
    X(void, glUniformMatrix3fv, (GLint location, GLsizei count, GLboolean transpose,               \
                                 const GLfloat *value))
// clang-format on

#define OPENRAC_GL_STATS_DEFINE(ret, name, params)                                                 \
    using PFN_##name = ret(OPENRAC_GLAPI*) params;                                                 \
    PFN_##name name = nullptr;
OPENRAC_GL_STATS_FUNCTIONS(OPENRAC_GL_STATS_DEFINE)
#undef OPENRAC_GL_STATS_DEFINE

// The functions of gl.h that are counted, as the context gave them.
struct Real {
    PFN_glDrawArrays glDrawArrays = nullptr;
    PFN_glDrawElements glDrawElements = nullptr;
    PFN_glDrawElementsInstanced glDrawElementsInstanced = nullptr;
    PFN_glActiveTexture glActiveTexture = nullptr;
    PFN_glBindTexture glBindTexture = nullptr;
    PFN_glTexImage2D glTexImage2D = nullptr;
    PFN_glGenerateMipmap glGenerateMipmap = nullptr;
    PFN_glDeleteTextures glDeleteTextures = nullptr;
    PFN_glBufferData glBufferData = nullptr;
    PFN_glBufferSubData glBufferSubData = nullptr;
    PFN_glUseProgram glUseProgram = nullptr;
    PFN_glBindFramebuffer glBindFramebuffer = nullptr;
    PFN_glCreateShader glCreateShader = nullptr;
    PFN_glShaderSource glShaderSource = nullptr;
    PFN_glCompileShader glCompileShader = nullptr;
    PFN_glDeleteShader glDeleteShader = nullptr;
    PFN_glAttachShader glAttachShader = nullptr;
    PFN_glLinkProgram glLinkProgram = nullptr;
    PFN_glDeleteProgram glDeleteProgram = nullptr;
};

struct Texture {
    std::uint64_t base = 0;  // level 0's bytes
    bool mipmaps = false;
};

struct ShaderObject {
    GLenum type = 0;
    std::string source;
};

struct ProgramRecord {
    Program shown;
    std::string vertex_source, fragment_source;
};

struct Pending {
    GLuint query;
    std::uint64_t serial;
    int section;
};

constexpr int kUnits = 32;
constexpr std::size_t kMaxFailures = 32;

struct State {
    bool installed = false;
    Real real;
    Info info;
    // Textures: what is bound to GL_TEXTURE_2D on each unit, and every texture given an image.
    int unit = 0;
    GLuint bound[kUnits] = {};
    std::unordered_map<GLuint, Texture> textures;
    std::uint64_t texture_bytes = 0;
    // Shaders and programs.
    std::unordered_map<GLuint, ShaderObject> shaders;
    std::map<GLuint, ProgramRecord> programs;  // by id: the order they were made in, nearly always
    GLuint program = 0;                        // in use
    std::vector<std::string> failures;
    // Timer queries: ids free to use, the one running, those waiting for their result.
    std::vector<GLuint> free_queries;
    bool query_running = false;
    std::vector<Pending> pending;
};

State& state() {
    static State s;
    return s;
}

std::uint64_t bytes_of(const Texture& t) {
    return t.mipmaps ? t.base + t.base / 3 : t.base;
}

void add_failure(std::string text) {
    State& s = state();
    while (!text.empty() && (text.back() == '\n' || text.back() == '\0' || text.back() == ' ')) {
        text.pop_back();
    }
    if (s.failures.size() >= kMaxFailures) {
        s.failures.erase(s.failures.begin());
    }
    s.failures.push_back(std::move(text));
}

// The built-in file a source is, by its text; null when it is none (put together at run time).
const stats::ShaderFile* file_of(const std::string& source) {
    for (const stats::ShaderFile& f : stats::embedded_shaders()) {
        if (source == f.text) {
            return &f;
        }
    }
    return nullptr;
}

std::string base_name(const std::string& path) {
    const std::size_t slash = path.find_last_of('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

// ---- The counting functions ----

std::uint64_t triangles_of(GLenum mode, GLsizei count) {
    if (count < 3) {
        return 0;
    }
    switch (mode) {
        case GL_TRIANGLES:
            return static_cast<std::uint64_t>(count) / 3;
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
            return static_cast<std::uint64_t>(count) - 2;
        default:
            return 0;
    }
}

void OPENRAC_GLAPI counted_glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    stats::count(stats::Counter::DrawCalls);
    stats::count(stats::Counter::Triangles, triangles_of(mode, count));
    state().real.glDrawArrays(mode, first, count);
}

void OPENRAC_GLAPI counted_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    stats::count(stats::Counter::DrawCalls);
    stats::count(stats::Counter::Triangles, triangles_of(mode, count));
    state().real.glDrawElements(mode, count, type, indices);
}

void OPENRAC_GLAPI counted_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                                   GLsizei instances) {
    stats::count(stats::Counter::DrawCalls);
    stats::count(stats::Counter::Triangles,
                 triangles_of(mode, count) * static_cast<std::uint64_t>(std::max(instances, 0)));
    state().real.glDrawElementsInstanced(mode, count, type, indices, instances);
}

void OPENRAC_GLAPI counted_glActiveTexture(GLenum texture) {
    State& s = state();
    const int unit = static_cast<int>(texture - GL_TEXTURE0);
    s.unit = unit >= 0 && unit < kUnits ? unit : 0;
    s.real.glActiveTexture(texture);
}

void OPENRAC_GLAPI counted_glBindTexture(GLenum target, GLuint texture) {
    State& s = state();
    if (target == GL_TEXTURE_2D) {
        s.bound[s.unit] = texture;
    }
    s.real.glBindTexture(target, texture);
}

void OPENRAC_GLAPI counted_glTexImage2D(GLenum target, GLint level, GLint internal_format, GLsizei w, GLsizei h,
                                        GLint border, GLenum format, GLenum type, const void* pixels) {
    State& s = state();
    const std::uint64_t bytes = static_cast<std::uint64_t>(std::max(w, 0)) * static_cast<std::uint64_t>(std::max(h, 0)) * 4;
    if (target == GL_TEXTURE_2D && level == 0 && s.bound[s.unit] != 0) {
        Texture& t = s.textures[s.bound[s.unit]];
        s.texture_bytes -= bytes_of(t);
        t.base = bytes;
        s.texture_bytes += bytes_of(t);
    }
    if (pixels != nullptr) {
        stats::count(stats::Counter::TextureUploads);
        stats::count(stats::Counter::TextureUploadBytes, bytes);
    }
    s.real.glTexImage2D(target, level, internal_format, w, h, border, format, type, pixels);
}

void OPENRAC_GLAPI counted_glGenerateMipmap(GLenum target) {
    State& s = state();
    if (target == GL_TEXTURE_2D) {
        auto found = s.textures.find(s.bound[s.unit]);
        if (found != s.textures.end() && !found->second.mipmaps) {
            s.texture_bytes -= bytes_of(found->second);
            found->second.mipmaps = true;
            s.texture_bytes += bytes_of(found->second);
        }
    }
    s.real.glGenerateMipmap(target);
}

void OPENRAC_GLAPI counted_glDeleteTextures(GLsizei n, const GLuint* textures) {
    State& s = state();
    for (GLsizei i = 0; i < n; ++i) {
        auto found = s.textures.find(textures[i]);
        if (found != s.textures.end()) {
            s.texture_bytes -= bytes_of(found->second);
            s.textures.erase(found);
        }
        for (GLuint& bound : s.bound) {
            if (bound == textures[i]) {
                bound = 0;
            }
        }
    }
    s.real.glDeleteTextures(n, textures);
}

void OPENRAC_GLAPI counted_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    if (data != nullptr && size > 0) {
        stats::count(stats::Counter::BufferUploadBytes, static_cast<std::uint64_t>(size));
    }
    state().real.glBufferData(target, size, data, usage);
}

void OPENRAC_GLAPI counted_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
    if (size > 0) {
        stats::count(stats::Counter::BufferUploadBytes, static_cast<std::uint64_t>(size));
    }
    state().real.glBufferSubData(target, offset, size, data);
}

void OPENRAC_GLAPI counted_glUseProgram(GLuint program) {
    State& s = state();
    if (program != s.program) {
        s.program = program;
        stats::count(stats::Counter::ProgramBinds);
        auto found = s.programs.find(program);
        if (found != s.programs.end()) {
            found->second.shown.binds++;
            found->second.shown.binds_total++;
        }
    }
    s.real.glUseProgram(program);
}

void OPENRAC_GLAPI counted_glBindFramebuffer(GLenum target, GLuint framebuffer) {
    stats::count(stats::Counter::FramebufferBinds);
    state().real.glBindFramebuffer(target, framebuffer);
}

GLuint OPENRAC_GLAPI counted_glCreateShader(GLenum type) {
    State& s = state();
    const GLuint shader = s.real.glCreateShader(type);
    if (shader != 0) {
        s.shaders[shader].type = type;
    }
    return shader;
}

void OPENRAC_GLAPI counted_glShaderSource(GLuint shader, GLsizei count, const GLchar* const* strings,
                                          const GLint* lengths) {
    State& s = state();
    auto found = s.shaders.find(shader);
    if (found != s.shaders.end()) {
        found->second.source.clear();
        for (GLsizei i = 0; i < count; ++i) {
            if (lengths != nullptr && lengths[i] >= 0) {
                found->second.source.append(strings[i], static_cast<std::size_t>(lengths[i]));
            } else {
                found->second.source.append(strings[i]);
            }
        }
    }
    s.real.glShaderSource(shader, count, strings, lengths);
}

std::string shader_log(GLuint shader) {
    GLint size = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
    std::string log(static_cast<std::size_t>(size > 0 ? size : 1), '\0');
    glGetShaderInfoLog(shader, size, nullptr, log.data());
    return log;
}

std::string program_log(GLuint program) {
    GLint size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
    std::string log(static_cast<std::size_t>(size > 0 ? size : 1), '\0');
    glGetProgramInfoLog(program, size, nullptr, log.data());
    while (!log.empty() && (log.back() == '\0' || log.back() == '\n')) {
        log.pop_back();
    }
    return log;
}

void OPENRAC_GLAPI counted_glCompileShader(GLuint shader) {
    State& s = state();
    s.real.glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == 0) {
        auto found = s.shaders.find(shader);
        const stats::ShaderFile* file = found != s.shaders.end() ? file_of(found->second.source) : nullptr;
        const char* kind = found != s.shaders.end() && found->second.type == GL_VERTEX_SHADER ? "vertex" : "fragment";
        add_failure(std::string(file != nullptr ? file->path : "a") + " (" + kind + " shader) did not compile: "
                    + shader_log(shader));
    }
}

void OPENRAC_GLAPI counted_glDeleteShader(GLuint shader) {
    State& s = state();
    s.shaders.erase(shader);
    s.real.glDeleteShader(shader);
}

void name_program(ProgramRecord& p) {
    const stats::ShaderFile* vertex = file_of(p.vertex_source);
    const stats::ShaderFile* fragment = file_of(p.fragment_source);
    p.shown.vertex_file = vertex != nullptr ? vertex->path : "";
    p.shown.fragment_file = fragment != nullptr ? fragment->path : "";
    if (vertex != nullptr || fragment != nullptr) {
        p.shown.name = (vertex != nullptr ? base_name(vertex->path) : std::string("?")) + " + "
                       + (fragment != nullptr ? base_name(fragment->path) : std::string("?"));
    } else {
        p.shown.name = "program " + std::to_string(p.shown.id);
    }
}

void OPENRAC_GLAPI counted_glAttachShader(GLuint program, GLuint shader) {
    State& s = state();
    auto found = s.shaders.find(shader);
    if (found != s.shaders.end()) {
        ProgramRecord& p = s.programs[program];
        p.shown.id = program;
        (found->second.type == GL_VERTEX_SHADER ? p.vertex_source : p.fragment_source) = found->second.source;
    }
    s.real.glAttachShader(program, shader);
}

void OPENRAC_GLAPI counted_glLinkProgram(GLuint program) {
    State& s = state();
    s.real.glLinkProgram(program);
    ProgramRecord& p = s.programs[program];
    p.shown.id = program;
    name_program(p);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    p.shown.linked = ok != 0;
    p.shown.log = program_log(program);
    if (ok == 0) {
        add_failure(p.shown.name + " did not link: " + p.shown.log);
    }
}

void OPENRAC_GLAPI counted_glDeleteProgram(GLuint program) {
    State& s = state();
    s.programs.erase(program);
    if (s.program == program) {
        s.program = 0;
    }
    s.real.glDeleteProgram(program);
}

// ---- GPU timers (stats::GpuHooks) ----

bool timer_begin(std::uint64_t serial, int section) {
    State& s = state();
    if (!s.info.timer_queries || s.query_running) {
        return false;
    }
    GLuint query = 0;
    if (!s.free_queries.empty()) {
        query = s.free_queries.back();
        s.free_queries.pop_back();
    } else {
        glGenQueries(1, &query);
    }
    if (query == 0) {
        return false;
    }
    glBeginQuery(GL_TIME_ELAPSED, query);
    s.query_running = true;
    s.pending.push_back({query, serial, section});
    return true;
}

void timer_end() {
    State& s = state();
    if (s.query_running) {
        glEndQuery(GL_TIME_ELAPSED);
        s.query_running = false;
    }
}

void timer_poll() {
    State& s = state();
    // In the order they were asked: a result is not back before an earlier one's.
    std::size_t done = 0;
    for (; done < s.pending.size(); ++done) {
        const Pending& p = s.pending[done];
        if (s.query_running && done + 1 == s.pending.size()) {
            break;  // still being timed
        }
        GLuint available = 0;
        glGetQueryObjectuiv(p.query, GL_QUERY_RESULT_AVAILABLE, &available);
        if (available == 0) {
            break;
        }
        GLuint64 ns = 0;
        glGetQueryObjectui64v(p.query, GL_QUERY_RESULT, &ns);
        stats::gpu_result(p.serial, p.section, static_cast<double>(ns) * 1.0e-6);
        s.free_queries.push_back(p.query);
    }
    s.pending.erase(s.pending.begin(), s.pending.begin() + static_cast<std::ptrdiff_t>(done));
}

bool contains_no_case(const std::string& text, const char* word) {
    const auto lower = [](std::string t) {
        std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return t;
    };
    return lower(text).find(lower(word)) != std::string::npos;
}

std::string gl_string(GLenum name) {
    const GLubyte* text = glGetString(name);
    return text != nullptr ? reinterpret_cast<const char*>(text) : "";
}

// ---- Reloading a program ----

bool read_text(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return false;
    }
    std::ostringstream text;
    text << in.rdbuf();
    out = text.str();
    return true;
}

// Compiles with the context's own functions: a trial is not one of the game's shaders.
GLuint compile_plain(GLenum type, const std::string& source, std::string& log) {
    State& s = state();
    const GLuint shader = s.real.glCreateShader(type);
    const GLchar* text = source.data();
    const GLint length = static_cast<GLint>(source.size());
    s.real.glShaderSource(shader, 1, &text, &length);
    s.real.glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == 0) {
        log = shader_log(shader);
        s.real.glDeleteShader(shader);
        return 0;
    }
    return shader;
}

struct UniformValue {
    std::string name;  // one element: "colour", "bones[3]"
    GLenum type = 0;
    GLfloat f[16] = {};
    GLint i[4] = {};
};

bool is_float_type(GLenum type) {
    return type == GL_FLOAT || type == GL_FLOAT_VEC2 || type == GL_FLOAT_VEC3 || type == GL_FLOAT_VEC4
           || type == GL_FLOAT_MAT3 || type == GL_FLOAT_MAT4;
}

std::vector<UniformValue> read_uniforms(GLuint program) {
    std::vector<UniformValue> values;
    GLint count = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    for (GLint index = 0; index < count; ++index) {
        GLchar name[256] = {};
        GLsizei length = 0;
        GLint size = 0;
        GLenum type = 0;
        glGetActiveUniform(program, static_cast<GLuint>(index), sizeof(name), &length, &size, &type, name);
        std::string base(name, static_cast<std::size_t>(std::max(length, 0)));
        if (base.size() > 3 && base.ends_with("[0]")) {
            base.resize(base.size() - 3);
        }
        for (GLint element = 0; element < size; ++element) {
            UniformValue v;
            v.name = size > 1 ? base + "[" + std::to_string(element) + "]" : base;
            v.type = type;
            const GLint location = glGetUniformLocation(program, v.name.c_str());
            if (location < 0) {
                continue;  // in a uniform block
            }
            if (is_float_type(type)) {
                glGetUniformfv(program, location, v.f);
            } else {
                glGetUniformiv(program, location, v.i);
            }
            values.push_back(std::move(v));
        }
    }
    return values;
}

// Into the program in use.
void write_uniforms(GLuint program, const std::vector<UniformValue>& values) {
    for (const UniformValue& v : values) {
        const GLint location = glGetUniformLocation(program, v.name.c_str());
        if (location < 0) {
            continue;  // the new code has no such uniform
        }
        switch (v.type) {
            case GL_FLOAT: glUniform1fv(location, 1, v.f); break;
            case GL_FLOAT_VEC2: glUniform2fv(location, 1, v.f); break;
            case GL_FLOAT_VEC3: glUniform3fv(location, 1, v.f); break;
            case GL_FLOAT_VEC4: glUniform4fv(location, 1, v.f); break;
            case GL_FLOAT_MAT3: glUniformMatrix3fv(location, 1, GL_FALSE, v.f); break;
            case GL_FLOAT_MAT4: glUniformMatrix4fv(location, 1, GL_FALSE, v.f); break;
            case GL_INT_VEC2: glUniform2iv(location, 1, v.i); break;
            case GL_INT_VEC3: glUniform3iv(location, 1, v.i); break;
            case GL_INT_VEC4: glUniform4iv(location, 1, v.i); break;
            default: glUniform1iv(location, 1, v.i); break;  // int, bool, the samplers
        }
    }
}

}  // namespace

void install(gl::GetProcAddress get) {
    State& s = state();
    if (s.installed || !gl::loaded()) {
        return;
    }
    s.installed = true;
    bool all = true;
#define OPENRAC_GL_STATS_LOAD(ret, name, params)                                                   \
    name = reinterpret_cast<PFN_##name>(get(#name));                                               \
    all = all && name != nullptr;
    OPENRAC_GL_STATS_FUNCTIONS(OPENRAC_GL_STATS_LOAD)
#undef OPENRAC_GL_STATS_LOAD

    s.info.vendor = gl_string(GL_VENDOR);
    s.info.renderer = gl_string(GL_RENDERER);
    s.info.version = gl_string(GL_VERSION);
    s.info.glsl = gl_string(GL_SHADING_LANGUAGE_VERSION);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s.info.max_texture_size);
    for (const char* word : {"llvmpipe", "softpipe", "swiftshader", "software", "swrast", "basic render"}) {
        s.info.software = s.info.software || contains_no_case(s.info.renderer, word);
    }
    s.info.timer_queries = glGenQueries != nullptr && glBeginQuery != nullptr && glEndQuery != nullptr
                           && glGetQueryObjectuiv != nullptr && glGetQueryObjectui64v != nullptr;
    (void)all;

#define OPENRAC_GL_STATS_WRAP(name)                                                                \
    s.real.name = gl::name;                                                                        \
    gl::name = counted_##name;
    OPENRAC_GL_STATS_WRAP(glDrawArrays)
    OPENRAC_GL_STATS_WRAP(glDrawElements)
    OPENRAC_GL_STATS_WRAP(glDrawElementsInstanced)
    OPENRAC_GL_STATS_WRAP(glActiveTexture)
    OPENRAC_GL_STATS_WRAP(glBindTexture)
    OPENRAC_GL_STATS_WRAP(glTexImage2D)
    OPENRAC_GL_STATS_WRAP(glGenerateMipmap)
    OPENRAC_GL_STATS_WRAP(glDeleteTextures)
    OPENRAC_GL_STATS_WRAP(glBufferData)
    OPENRAC_GL_STATS_WRAP(glBufferSubData)
    OPENRAC_GL_STATS_WRAP(glUseProgram)
    OPENRAC_GL_STATS_WRAP(glBindFramebuffer)
    OPENRAC_GL_STATS_WRAP(glCreateShader)
    OPENRAC_GL_STATS_WRAP(glShaderSource)
    OPENRAC_GL_STATS_WRAP(glCompileShader)
    OPENRAC_GL_STATS_WRAP(glDeleteShader)
    OPENRAC_GL_STATS_WRAP(glAttachShader)
    OPENRAC_GL_STATS_WRAP(glLinkProgram)
    OPENRAC_GL_STATS_WRAP(glDeleteProgram)
#undef OPENRAC_GL_STATS_WRAP

    if (s.info.timer_queries) {
        stats::set_gpu_hooks({timer_begin, timer_end, timer_poll});
    }
    log::info("OpenGL: {} / {} / {}{}", s.info.vendor, s.info.renderer, s.info.version,
              s.info.software ? " (a software rasteriser)" : "");
}

bool installed() {
    return state().installed;
}

const Info& info() {
    return state().info;
}

Textures textures() {
    const State& s = state();
    return {s.textures.size(), s.texture_bytes};
}

std::vector<Program> programs() {
    std::vector<Program> list;
    for (const auto& [id, p] : state().programs) {
        list.push_back(p.shown);
    }
    return list;
}

void reset_binds() {
    for (auto& [id, p] : state().programs) {
        p.shown.binds = 0;
    }
}

const std::vector<std::string>& failures() {
    return state().failures;
}

bool reload(unsigned program, const std::string& root, std::string& message) {
    State& s = state();
    auto found = s.programs.find(program);
    if (found == s.programs.end()) {
        message = "no such program";
        return false;
    }
    ProgramRecord& p = found->second;
    const auto fail = [&](std::string why) {
        message = p.shown.name + ": " + why;
        p.shown.reload_status = why;
        return false;
    };
    if (glGetAttachedShaders == nullptr || glDetachShader == nullptr || glGetActiveUniform == nullptr
        || glGetUniformfv == nullptr || glGetUniformiv == nullptr || glUniform1fv == nullptr
        || glUniformMatrix3fv == nullptr || glUniform4iv == nullptr) {
        return fail("this OpenGL context lacks what a reload needs");
    }
    if (p.shown.vertex_file.empty() || p.shown.fragment_file.empty()) {
        return fail("its sources are not two built-in files (put together at run time)");
    }
    std::string vertex, fragment;
    const std::string vertex_path = root + "/" + p.shown.vertex_file;
    const std::string fragment_path = root + "/" + p.shown.fragment_file;
    if (!read_text(vertex_path, vertex)) {
        return fail("cannot read " + vertex_path);
    }
    if (!read_text(fragment_path, fragment)) {
        return fail("cannot read " + fragment_path);
    }
    std::string log;
    const GLuint vs = compile_plain(GL_VERTEX_SHADER, vertex, log);
    if (vs == 0) {
        return fail(p.shown.vertex_file + ": " + log);
    }
    const GLuint fs = compile_plain(GL_FRAGMENT_SHADER, fragment, log);
    if (fs == 0) {
        s.real.glDeleteShader(vs);
        return fail(p.shown.fragment_file + ": " + log);
    }
    // A trial link, so that the program in use is only linked again with code that links.
    const GLuint trial = glCreateProgram();
    s.real.glAttachShader(trial, vs);
    s.real.glAttachShader(trial, fs);
    s.real.glLinkProgram(trial);
    GLint ok = 0;
    glGetProgramiv(trial, GL_LINK_STATUS, &ok);
    if (ok == 0) {
        log = program_log(trial);
        s.real.glDeleteProgram(trial);
        s.real.glDeleteShader(vs);
        s.real.glDeleteShader(fs);
        return fail("link: " + log);
    }
    s.real.glDeleteProgram(trial);

    // The program itself: its uniforms' values kept, its old shaders taken off, the new ones linked
    // in, the values put back. Attribute locations set by the code stay with the program.
    const std::vector<UniformValue> values = read_uniforms(program);
    GLuint old[8] = {};
    GLsizei old_count = 0;
    glGetAttachedShaders(program, 8, &old_count, old);
    for (GLsizei i = 0; i < old_count; ++i) {
        glDetachShader(program, old[i]);
    }
    s.real.glAttachShader(program, vs);
    s.real.glAttachShader(program, fs);
    s.real.glLinkProgram(program);
    s.real.glDeleteShader(vs);
    s.real.glDeleteShader(fs);
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    p.shown.linked = ok != 0;
    p.shown.log = program_log(program);
    if (ok == 0) {
        return fail("link: " + p.shown.log);
    }
    s.real.glUseProgram(program);
    write_uniforms(program, values);
    s.real.glUseProgram(s.program);
    p.vertex_source = std::move(vertex);
    p.fragment_source = std::move(fragment);
    p.shown.reloads++;
    p.shown.reload_status = "reloaded (" + std::to_string(p.shown.reloads) + ")";
    message = p.shown.name + ": reloaded";
    log::info("shader program {} ({}) reloaded from {}", program, p.shown.name, root);
    return true;
}

int reload_all(const std::string& root, std::string& message) {
    std::vector<unsigned> ids;
    for (const auto& [id, p] : state().programs) {
        if (!p.shown.vertex_file.empty() && !p.shown.fragment_file.empty()) {
            ids.push_back(id);
        }
    }
    int done = 0;
    message.clear();
    for (const unsigned id : ids) {
        std::string one;
        if (reload(id, root, one)) {
            ++done;
        } else {
            message += (message.empty() ? "" : "\n") + one;
        }
    }
    return done;
}

}  // namespace openrac::renderer::gl_stats

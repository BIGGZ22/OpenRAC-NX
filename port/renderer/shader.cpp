// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/shader.h"

#include <algorithm>
#include <format>
#include <utility>

#include "renderer/gl.h"

namespace openrac::renderer {

using namespace openrac::gl;

namespace {

constexpr int kSamplerUnits = 8;

}  // namespace

std::string es_source(std::string_view source, bool noperspective, bool clip_distance) {
    // The version line, and the first line after it that is not a comment, are where the ES
    // preamble goes; a source without "#version 410 core" first is given as it is.
    constexpr std::string_view kDesktop = "#version 410 core";
    if (source.substr(0, kDesktop.size()) != kDesktop) {
        return std::string(source);
    }
    std::string body(source.substr(kDesktop.size()));
    const auto replace_all = [&body](std::string_view from, std::string_view to) {
        for (std::size_t at = body.find(from); at != std::string::npos; at = body.find(from, at + to.size())) {
            body.replace(at, from.size(), to);
        }
    };
    std::string out = "#version 320 es\n";
    if (noperspective) {
        out += "#extension GL_NV_shader_noperspective_interpolation : require\n";
    } else {
        // Interpolated with perspective instead: Gouraud colours and fog differ a little across a
        // large triangle seen at an angle, and nothing else.
        replace_all("noperspective ", "");
    }
    if (body.find("gl_ClipDistance") != std::string::npos) {
        if (clip_distance) {
            out += "#extension GL_EXT_clip_cull_distance : require\n";
        } else {
            // No user clip planes: the distances are written to a variable nobody reads, and the
            // renderer leaves the planes off (gl::has_extension).
            replace_all("gl_ClipDistance", "openrac_clip_distance");
            out += "float openrac_clip_distance[8];\n";
        }
    }
    // ES has no default precision for floats in a fragment shader or for the sampler types beyond
    // sampler2D and samplerCube; the desktop's is high everywhere.
    out += "precision highp float;\nprecision highp int;\nprecision highp sampler2D;\n"
           "precision highp sampler2DArray;\nprecision highp usampler2D;\nprecision highp isampler2D;\n"
           "precision highp samplerBuffer;\nprecision highp usamplerBuffer;\nprecision highp isamplerBuffer;\n";
    // Keep the source's own line numbers in compiler logs.
    out += "#line 1\n";
    out += body;
    return out;
}

namespace {

GLuint compile(GLenum type, std::string_view desktop_source, std::string& log) {
    std::string translated;
    std::string_view source = desktop_source;
    if (is_es()) {
        translated = es_source(desktop_source, has_extension("GL_NV_shader_noperspective_interpolation"),
                               has_extension("GL_EXT_clip_cull_distance"));
        source = translated;
    }
    // Give shader stages the same single opt-in as the C++ resource gates.
    if (enhanced_rendering_available()) {
        translated = source;
        translated.insert(translated.find('\n') + 1, "#define OPENRAC_ENHANCED_RENDERING 1\n");
        source = translated;
    }
    const GLuint shader = glCreateShader(type);
    const GLchar* text = source.data();
    const auto length = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &text, &length);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == 0) {
        GLint size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
        log.assign(static_cast<std::size_t>(size > 0 ? size : 1), '\0');
        glGetShaderInfoLog(shader, size, nullptr, log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

}  // namespace

Shader::~Shader() {
    // The program belongs to a context that may be gone by now; release()
    // frees it while the context is current.
}

Shader::Shader(Shader&& other) noexcept
    : m_program(std::exchange(other.m_program, 0)), m_locations(std::move(other.m_locations)) {}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        m_program = std::exchange(other.m_program, 0);
        m_locations = std::move(other.m_locations);
        other.m_locations.clear();
    }
    return *this;
}

bool Shader::build(
    std::string_view name, std::string_view vertex, std::string_view fragment, std::string& error
) {
    std::string log;
    const GLuint vs = compile(GL_VERTEX_SHADER, vertex, log);
    if (vs == 0) {
        error = std::format("{}: vertex shader: {}", name, log);
        return false;
    }
    const GLuint fs = compile(GL_FRAGMENT_SHADER, fragment, log);
    if (fs == 0) {
        glDeleteShader(vs);
        error = std::format("{}: fragment shader: {}", name, log);
        return false;
    }
    const GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (ok == 0) {
        GLint size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
        log.assign(static_cast<std::size_t>(size > 0 ? size : 1), '\0');
        glGetProgramInfoLog(program, size, nullptr, log.data());
        glDeleteProgram(program);
        error = std::format("{}: link: {}", name, log);
        return false;
    }
    release();
    m_program = program;
    m_locations.clear();
    glUseProgram(m_program);
    for (int unit = 0; unit < kSamplerUnits; ++unit) {
        const std::string sampler = std::format("tex_T{}", unit);
        const GLint location = glGetUniformLocation(m_program, sampler.c_str());
        if (location >= 0) {
            glUniform1i(location, unit);
        }
    }
    return true;
}

bool Shader::build_compute(std::string_view name, std::string_view source, std::string& error) {
    std::string log;
    std::string es;
    if (is_es()) {
        const auto line = source.find('\n');
        es = "#version 310 es\nprecision highp float;\nprecision highp int;\nprecision highp "
             "sampler2D;\n";
        es += source.substr(line + 1);
        source = es;
    }
    const GLuint shader = compile(GL_COMPUTE_SHADER, source, log);
    if (!shader) {
        error = std::format("{}: {}", name, log);
        return false;
    }
    release();
    m_program = glCreateProgram();
    glAttachShader(m_program, shader);
    glLinkProgram(m_program);
    glDeleteShader(shader);
    GLint ok = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint size = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &size);
        log.resize(static_cast<std::size_t>(std::max(size, 1)));
        glGetProgramInfoLog(m_program, size, nullptr, log.data());
        error = std::format("{}: {}", name, log);
        release();
        return false;
    }
    return true;
}

void Shader::use() const {
    glUseProgram(m_program);
}

int Shader::uniform(const char* name) const {
    const auto found = m_locations.find(name);
    if (found != m_locations.end() && found->second.name == name) {
        return found->second.location;
    }
    const int location = glGetUniformLocation(m_program, name);
    m_locations[name] = Location{name, location};
    return location;
}

void Shader::release() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    m_locations.clear();
}

}  // namespace openrac::renderer

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/shader.h"

#include <format>
#include <utility>

#include "renderer/gl.h"

namespace openrac::renderer {

using namespace openrac::gl;

namespace {

constexpr int kSamplerUnits = 8;

GLuint compile(GLenum type, std::string_view source, std::string& log) {
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

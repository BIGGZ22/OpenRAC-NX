// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A GLSL program. Every shader is `#version 410 core`, OpenGOAL's baseline,
// so one source runs on every platform OpenGL 4.1 reaches. As in OpenGOAL,
// a sampler named tex_T<n> is bound to texture unit n when the program is
// built, so no renderer sets sampler uniforms by hand.

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace openrac::renderer {

class Shader {
public:
    Shader() = default;
    ~Shader();
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Compile and link; on failure the GL log is in `error`, prefixed with
    // the shader's name.
    bool build(
        std::string_view name,
        std::string_view vertex,
        std::string_view fragment,
        std::string& error
    );

    void use() const;

    // -1 when the program has no such uniform (or the compiler removed it). The driver is asked
    // once per name: a renderer looks its uniforms up by name every frame, and each lookup would
    // otherwise be a call into the driver that compares strings.
    int uniform(const char* name) const;

    unsigned id() const { return m_program; }

    // Delete the program (the context must be current).
    void release();

private:
    unsigned m_program = 0;
    // By the name's address (callers pass literals), with the name itself kept to tell a buffer
    // that was written again apart.
    struct Location {
        std::string name;
        int location = -1;
    };
    mutable std::unordered_map<const char*, Location> m_locations;
};

}  // namespace openrac::renderer

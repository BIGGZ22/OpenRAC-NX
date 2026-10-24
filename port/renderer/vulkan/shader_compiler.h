// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The renderer's GLSL (#version 410 core, as renderer/shader.h writes it) made into SPIR-V for
// Vulkan, with what an OpenGL program would answer about its uniforms: the Vulkan side of
// renderer/gl.h (vkgl.h) links programs with this. glslang (BSD-3-Clause) compiles under its
// relaxed Vulkan rules, the ones made for OpenGL's GLSL: the loose uniforms go into one uniform
// block (gl_DefaultUniformBlock, std140), samplers and blocks get bindings, inputs and outputs
// locations matched by name between the stages.
//
// Two changes to the source make it mean in Vulkan what it meant in OpenGL: the vertex stage
// maps OpenGL's depth (-w..w) to Vulkan's (0..w) after the shader's own main(), and nothing
// else is needed for y, since both APIs put a framebuffer's row 0 at NDC y = -1 (vkgl.cpp turns
// the picture right way up only on its way to the screen).

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace openrac::vkgl {

// What a sampler uniform reads: Vulkan's descriptor and image view kind.
enum class SamplerKind { Texture2D, Texture2DArray, Cube, Buffer };

struct ProgramUniform {
    std::string name;          // without "[0]"
    std::uint32_t gl_type = 0;  // GL_FLOAT_VEC4, GL_SAMPLER_2D...
    int offset = -1;           // in the default block; -1 for a sampler
    int array_size = 1;
    int array_stride = 0;      // bytes between elements (std140)
    // A sampler: its binding and kind.
    bool sampler = false;
    int binding = -1;
    SamplerKind kind = SamplerKind::Texture2D;
};

struct ProgramBlock {
    std::string name;
    int binding = -1;
    int size = 0;
};

struct CompiledProgram {
    std::vector<std::uint32_t> vertex;
    std::vector<std::uint32_t> fragment;
    std::vector<std::uint32_t> compute;
    int default_binding = -1;  // the default block's binding; -1 when the program has no loose uniforms
    int default_size = 0;
    std::vector<ProgramUniform> uniforms;
    std::vector<ProgramBlock> blocks;
    std::vector<ProgramBlock> storage;
    // The vertex stage's inputs (glBindAttribLocation is not used by the renderer: its shaders
    // give every input a location), with their GL type: one the vertex array does not feed reads
    // GL's default (0, 0, 0, 1) of that kind.
    struct Attribute {
        std::string name;
        int location = -1;
        std::uint32_t gl_type = 0;
    };
    std::vector<Attribute> attributes;
};

// Compiles one stage alone, for glCompileShader's status and log (no SPIR-V is kept). A stage
// that compiled before is known from the SPIR-V cache and not compiled again (R-12).
bool check_stage(bool vertex, std::string_view source, std::string& log);
bool check_compute_stage(std::string_view source, std::string& log);

// Returns the compiler's version text the SPIR-V cache is made for: glslang's version and the
// version of the backend's compile options (R-12). A cache of another is dropped whole.
std::string spirv_compiler_version();

// Compiles and links the two stages. False, with the compiler's words in `log`, on failure. With
// spirv-disk-cache a program compiled before (the same sources, the same compiler) comes from the
// open context's SPIR-V cache, and only a program compiled now counts in shader_compiles.
bool compile_program(
    std::string_view vertex,
    std::string_view fragment,
    CompiledProgram& out,
    std::string& log,
    std::string_view compute = {}
);

}  // namespace openrac::vkgl

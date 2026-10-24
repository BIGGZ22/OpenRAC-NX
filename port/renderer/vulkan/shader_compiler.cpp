// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The shader compiler (shader_compiler.h): GLSL to SPIR-V with glslang, and the reflection that
// lays out a program's uniforms, blocks and inputs. Each compile counts in the open context's
// render counters (shader_compiles, shader_compile_s; docs/port/RENDER_PERF.md).

#include "renderer/vulkan/shader_compiler.h"

#include <memory>
#include <mutex>
#include <regex>

#include "renderer/vulkan/vkgl_internal.h"

#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
// After the public headers it needs: the GLSL resolver, which gives a uniform both stages use one
// binding (the default mapper numbers each stage's from zero).
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/MachineIndependent/iomapper.h>

namespace openrac::vkgl {
namespace {

// The GL_* types reflection answers with (glDefineType), the samplers among them.
constexpr int kSampler2D = 0x8B5E;
constexpr int kSamplerCube = 0x8B60;
constexpr int kSampler2DArray = 0x8DC1;
constexpr int kSamplerBuffer = 0x8DC2;
constexpr int kIntSampler2D = 0x8DCA;
constexpr int kIntSamplerCube = 0x8DCC;
constexpr int kIntSampler2DArray = 0x8DCF;
constexpr int kIntSamplerBuffer = 0x8DD0;
constexpr int kUnsignedSampler2D = 0x8DD2;
constexpr int kUnsignedSamplerCube = 0x8DD4;
constexpr int kUnsignedSampler2DArray = 0x8DD7;
constexpr int kUnsignedSamplerBuffer = 0x8DD8;

bool sampler_kind(int type, SamplerKind& kind) {
    switch (type) {
        case kSampler2D:
        case kIntSampler2D:
        case kUnsignedSampler2D: kind = SamplerKind::Texture2D; return true;
        case kSampler2DArray:
        case kIntSampler2DArray:
        case kUnsignedSampler2DArray: kind = SamplerKind::Texture2DArray; return true;
        case kSamplerCube:
        case kIntSamplerCube:
        case kUnsignedSamplerCube: kind = SamplerKind::Cube; return true;
        case kSamplerBuffer:
        case kIntSamplerBuffer:
        case kUnsignedSamplerBuffer: kind = SamplerKind::Buffer; return true;
        default: return false;
    }
}

// Bytes of one element of a non-opaque uniform type, for an array's std140 stride.
int element_bytes(int type) {
    switch (type) {
        case 0x8B5C: return 64;  // mat4
        case 0x8B5B: return 48;  // mat3, three padded columns
        case 0x8B5A: return 32;  // mat2
        case 0x8B52: case 0x8B55: case 0x8DC8: return 16;  // vec4, ivec4, uvec4
        case 0x8B51: case 0x8B54: case 0x8DC7: return 12;
        case 0x8B50: case 0x8B53: case 0x8DC6: return 8;
        default: return 4;
    }
}

// Counts one run of glslang and the time it takes, from its making to its end, in the open
// context's render counters. Nothing is counted without a context.
class CompileCount {
public:
    CompileCount() = default;
    CompileCount(const CompileCount&) = delete;
    CompileCount& operator=(const CompileCount&) = delete;

    // Adds the compile and its seconds to the context's counters.
    ~CompileCount() {
        // The compiler runs without a context only where nothing reads the counters.
        if (detail::g == nullptr) {
            return;
        }

        ++detail::g->submission.shader_compiles;
        detail::g->submission.shader_compile_s += detail::seconds_since(m_start);
    }

private:
    detail::Clock::time_point m_start = detail::Clock::now();  // When the compile began.
};

void initialise() {
    static std::once_flag once;
    std::call_once(once, [] { glslang::InitializeProcess(); });
}

// The vertex stage's depth from OpenGL's clip space (-w..w) to Vulkan's (0..w), after the
// shader's own main(), which may set its points' size over the default.
std::string vertex_source(std::string_view source) {
    static const std::regex main_decl(R"(\bvoid\s+main\s*\(\s*(void)?\s*\))");
    std::string out = std::regex_replace(std::string(source), main_decl, "void openrac_vk_main()",
                                         std::regex_constants::format_first_only);
    // A point's size is the shader's to say in Vulkan; OpenGL's is 1 unless the shader says.
    out += "\nvoid main() {\n    gl_PointSize = 1.0;\n    openrac_vk_main();\n"
           "    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n}\n";
    return out;
}

// A stage and its source: glslang keeps pointers to the text and to the array of its strings, not
// copies, until it is parsed; they live beside the shader, where moving the Stage does not move them.
struct Stage {
    struct Text {
        std::string source;
        const char* pointer = nullptr;
        int length = 0;
    };
    std::unique_ptr<Text> text;
    std::unique_ptr<glslang::TShader> shader;
    glslang::TShader* operator->() const { return shader.get(); }
    glslang::TShader& operator*() const { return *shader; }
    glslang::TShader* get() const { return shader.get(); }
};

Stage make_shader(EShLanguage stage, std::string source) {
    Stage out{std::make_unique<Stage::Text>(), std::make_unique<glslang::TShader>(stage)};
    out.text->source = std::move(source);
    out.text->pointer = out.text->source.c_str();
    out.text->length = static_cast<int>(out.text->source.size());
    glslang::TShader* shader = out.get();
    shader->setStringsWithLengths(&out.text->pointer, &out.text->length, 1);
    shader->setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
    // Vertex viewport outputs use the Vulkan 1.2 / SPIR-V 1.5 core
    // capabilities, rather than requiring the older device extension.
    const bool viewport_output =
        out.text->source.find("GL_ARB_shader_viewport_layer_array") != std::string::npos;
    shader->setEnvClient(glslang::EShClientVulkan, viewport_output ? glslang::EShTargetVulkan_1_2
                                                               : glslang::EShTargetVulkan_1_1);
    shader->setEnvTarget(glslang::EShTargetSpv, viewport_output ? glslang::EShTargetSpv_1_5
                                                            : glslang::EShTargetSpv_1_3);
    shader->setEnvInputVulkanRulesRelaxed();
    shader->setGlobalUniformBlockName("gl_DefaultUniformBlock");
    shader->setGlobalUniformSet(0);
    shader->setAutoMapBindings(true);
    shader->setAutoMapLocations(true);
    return out;
}

constexpr EShMessages kMessages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

bool parse(glslang::TShader& shader, std::string& log) {
    if (!shader.parse(GetDefaultResources(), 410, ECoreProfile, false, false, kMessages)) {
        log += shader.getInfoLog();
        log += shader.getInfoDebugLog();
        return false;
    }
    return true;
}

}  // namespace

bool check_stage(bool vertex, std::string_view source, std::string& log) {
    const CompileCount counted;
    initialise();
    const EShLanguage stage = vertex ? EShLangVertex : EShLangFragment;
    auto shader = make_shader(stage, vertex ? vertex_source(source) : std::string(source));
    return parse(*shader, log);
}

bool check_compute_stage(std::string_view source, std::string& log) {
    const CompileCount counted;
    initialise();
    auto shader = make_shader(EShLangCompute, std::string(source));
    return parse(*shader, log);
}

bool compile_program(std::string_view vertex, std::string_view fragment, CompiledProgram& out, std::string& log,
                     std::string_view compute) {
    const CompileCount counted;
    initialise();
    Stage vs, fs, cs;
    glslang::TProgram program;
    if (compute.empty()) {
        vs = make_shader(EShLangVertex, vertex_source(vertex));
        fs = make_shader(EShLangFragment, std::string(fragment));
        if (!parse(*vs, log) || !parse(*fs, log)) { return false; }
        program.addShader(vs.get());
        program.addShader(fs.get());
    } else {
        cs = make_shader(EShLangCompute, std::string(compute));
        if (!parse(*cs, log)) { return false; }
        program.addShader(cs.get());
    }
    // Bindings across both stages: a uniform both use has one, the stages' others their own.
    if (!program.link(kMessages)) {
        log += program.getInfoLog();
        log += program.getInfoDebugLog();
        return false;
    }
    const std::unique_ptr<glslang::TIoMapper> mapper(glslang::GetGlslIoMapper());
    glslang::TDefaultGlslIoResolver resolver(*program.getIntermediate(compute.empty() ? EShLangVertex : EShLangCompute));
    if (!program.mapIO(&resolver, mapper.get())) {
        log += program.getInfoLog();
        log += program.getInfoDebugLog();
        return false;
    }
    if (!program.buildReflection(EShReflectionDefault | EShReflectionSeparateBuffers)) {
        log += "could not reflect the program's uniforms";
        return false;
    }
    spv::SpvBuildLogger logger;
    glslang::SpvOptions options;
    options.generateDebugInfo = false;
    options.disableOptimizer = true;
    out = CompiledProgram{};
    if (compute.empty()) {
        glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), out.vertex, &logger, &options);
        glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), out.fragment, &logger, &options);
    } else {
        glslang::GlslangToSpv(*program.getIntermediate(EShLangCompute), out.compute, &logger, &options);
    }
    for (int i = 0; i < program.getNumBufferBlocks(); ++i) {
        const auto& b = program.getBufferBlock(i);
        out.storage.push_back({b.name, b.getBinding(), b.size});
    }

    int default_index = -1;
    for (int i = 0; i < program.getNumUniformBlocks(); ++i) {
        const glslang::TObjectReflection& b = program.getUniformBlock(i);
        if (b.name == "gl_DefaultUniformBlock") {
            default_index = i;
            out.default_binding = b.getBinding();
            out.default_size = b.size;
        } else {
            std::string name = b.name;
            out.blocks.push_back(ProgramBlock{name, b.getBinding(), b.size});
        }
    }
    for (int i = 0; i < program.getNumUniformVariables(); ++i) {
        const glslang::TObjectReflection& u = program.getUniform(i);
        ProgramUniform pu;
        pu.name = u.name;
        if (const auto bracket = pu.name.find('['); bracket != std::string::npos) {
            pu.name.resize(bracket);
        }
        pu.gl_type = static_cast<std::uint32_t>(u.glDefineType);
        pu.array_size = u.size > 0 ? u.size : 1;
        if (sampler_kind(u.glDefineType, pu.kind)) {
            pu.sampler = true;
            pu.binding = u.getBinding();
        } else {
            if (u.index != default_index || default_index < 0) {
                continue;  // a member of a named block: read through the block's buffer
            }
            pu.offset = u.offset;
            const int bytes = element_bytes(u.glDefineType);
            pu.array_stride = u.arrayStride > 0 ? u.arrayStride : (bytes + 15) / 16 * 16;
        }
        out.uniforms.push_back(std::move(pu));
    }
    for (int i = 0; compute.empty() && i < program.getNumPipeInputs(); ++i) {
        const glslang::TObjectReflection& a = program.getPipeInput(i);
        // A matrix takes a location for each column, as GL's attributes do.
        int columns = 1;
        std::uint32_t type = static_cast<std::uint32_t>(a.glDefineType);
        switch (type) {
            case 0x8B5C: columns = 4; type = 0x8B52; break;  // mat4: four vec4
            case 0x8B5B: columns = 3; type = 0x8B51; break;  // mat3: three vec3
            case 0x8B5A: columns = 2; type = 0x8B50; break;  // mat2: two vec2
            default: break;
        }
        const int location = static_cast<int>(a.layoutLocation());
        for (int c = 0; c < columns; ++c) {
            out.attributes.push_back({a.name, location + c, type});
        }
    }
    return true;
}

}  // namespace openrac::vkgl

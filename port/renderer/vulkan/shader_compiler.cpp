// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The shader compiler (shader_compiler.h): GLSL to SPIR-V with glslang, and the reflection that
// lays out a program's uniforms, blocks and inputs. Each compile counts in the open context's
// render counters (shader_compiles, shader_compile_s; docs/port/RENDER_PERF.md). With
// spirv-disk-cache, a program or stage compiled before (this run or an earlier one, the open
// context's SpirvCache) is taken from the cache and glslang does not run (R-12).

#include "renderer/vulkan/shader_compiler.h"

#include <array>
#include <cstdint>
#include <format>
#include <memory>
#include <mutex>
#include <regex>
#include <string>

#include "renderer/render_opt.h"
#include "renderer/vulkan/shader_compiler_internal.h"
#include "renderer/vulkan/spirv_cache_file.h"
#include "renderer/vulkan/vkgl_internal.h"

#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/build_info.h>
// After the public headers it needs: the GLSL resolver, which gives a uniform both stages use one
// binding (the default mapper numbers each stage's from zero).
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/MachineIndependent/iomapper.h>

namespace openrac::vkgl {
namespace compiler_detail {

void initialise() {
    static std::once_flag once;
    std::call_once(once, [] { glslang::InitializeProcess(); });
}

}  // namespace compiler_detail

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

bool parse(glslang::TShader& shader, std::string& log) {
    if (!shader.parse(
            GetDefaultResources(), 410, ECoreProfile, false, false, compiler_detail::kMessages
        )) {
        log += shader.getInfoLog();
        log += shader.getInfoDebugLog();
        return false;
    }
    return true;
}

// check_stage without the cache: glslang parses the stage.
bool check_stage_fresh(bool vertex, std::string_view source, std::string& log) {
    const CompileCount counted;
    compiler_detail::initialise();
    const EShLanguage stage = vertex ? EShLangVertex : EShLangFragment;
    auto shader = make_shader(stage, vertex ? vertex_source(source) : std::string(source));
    return parse(*shader, log);
}

// check_compute_stage without the cache.
bool check_compute_stage_fresh(std::string_view source, std::string& log) {
    const CompileCount counted;
    compiler_detail::initialise();
    auto shader = make_shader(EShLangCompute, std::string(source));
    return parse(*shader, log);
}

// compile_program without the cache: glslang parses, links, maps and reflects the program.
bool compile_program_fresh(
    std::string_view vertex,
    std::string_view fragment,
    CompiledProgram& out,
    std::string& log,
    std::string_view compute
) {
    const CompileCount counted;
    compiler_detail::initialise();
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
    if (!program.link(compiler_detail::kMessages)) {
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

// ---- The SPIR-V cache (R-12) ----

// What the compiler is, for the cache: glslang's version and the version of the options above.
// Raise kOptionsVersion with any change to make_shader, vertex_source or the SPIR-V options: a
// cache made by other options is then dropped whole.
constexpr int kOptionsVersion = 1;

// FNV-1a's 64-bit offset basis and prime (documented: the FNV hash's specification).
constexpr std::uint64_t kFnvBasis = 0xcbf29ce484222325u;
constexpr std::uint64_t kFnvPrime = 0x100000001b3u;

// Returns glslang's version and kOptionsVersion as one text.
std::string compiler_version() {
    return std::format(
        "glslang {}.{}.{}{} options {}",
        GLSLANG_VERSION_MAJOR,
        GLSLANG_VERSION_MINOR,
        GLSLANG_VERSION_PATCH,
        GLSLANG_VERSION_FLAVOR,
        kOptionsVersion
    );
}

// What a cache entry is found and checked by: the text compiled (its kind, its sources and the
// compiler) hashed forwards for the key and backwards for the check, and its length.
struct CacheText {
    std::uint64_t key = kFnvBasis;    // FNV-1a over the text, first byte first.
    std::uint64_t check = kFnvBasis;  // FNV-1a over the text, last byte first.
    std::uint64_t length = 0;         // The text's bytes.
};

// Returns the CacheText of `kind` and `parts`, each part followed by a NUL so that two splits of
// the same characters differ.
template <std::size_t Parts>
CacheText cache_text(std::string_view kind, const std::array<std::string_view, Parts>& parts) {
    std::string text = compiler_version();
    text += '\0';
    text += kind;
    text += '\0';

    // Each source, ended by a NUL.
    for (const std::string_view part : parts) {
        text += part;
        text += '\0';
    }

    CacheText out;
    out.length = text.size();

    // The two hashes, from either end.
    for (std::size_t n = 0; n < text.size(); ++n) {
        out.key = (out.key ^ static_cast<unsigned char>(text[n])) * kFnvPrime;
        out.check = (out.check ^ static_cast<unsigned char>(text[text.size() - 1 - n])) * kFnvPrime;
    }

    return out;
}

// Returns the open context's SPIR-V cache when the cache is on and kept; null otherwise.
SpirvCache* spirv_cache() {
    // Without a context, or with the entry off, every compile runs glslang.
    if (detail::g == nullptr
        || !renderer::render_opt::enabled(renderer::render_opt::Opt::SpirvDiskCache)) {
        return nullptr;
    }

    SpirvCache& cache = detail::g->spirv_cache;
    return cache.active() ? &cache : nullptr;
}

// Returns whether the stage of `kind` and `source` compiled before: a kept stage is one that did,
// so its log was empty of errors and nothing is left to say.
bool stage_cached(std::string_view kind, std::string_view source) {
    const SpirvCache* cache = spirv_cache();

    // No cache: the stage is compiled.
    if (cache == nullptr) {
        return false;
    }

    const CacheText text = cache_text<1>(kind, {source});
    return cache->find(text.key, text.check, text.length) != nullptr;
}

// Keeps that the stage of `kind` and `source` compiled.
void keep_stage(std::string_view kind, std::string_view source) {
    SpirvCache* cache = spirv_cache();

    // No cache to keep it in.
    if (cache == nullptr) {
        return;
    }

    const CacheText text = cache_text<1>(kind, {source});
    cache->insert({text.key, text.check, text.length, {}});
}

}  // namespace

std::string spirv_compiler_version() {
    return compiler_version();
}

bool check_stage(bool vertex, std::string_view source, std::string& log) {
    const std::string_view kind = vertex ? "vertex stage" : "fragment stage";

    // Compiled before: it compiles.
    if (stage_cached(kind, source)) {
        return true;
    }

    const bool ok = check_stage_fresh(vertex, source, log);

    // Only a stage that compiled is kept; a failure is compiled again for its log.
    if (ok) {
        keep_stage(kind, source);
    }

    return ok;
}

bool check_compute_stage(std::string_view source, std::string& log) {
    constexpr std::string_view kKind = "compute stage";

    // Compiled before: it compiles.
    if (stage_cached(kKind, source)) {
        return true;
    }

    const bool ok = check_compute_stage_fresh(source, log);

    // Only a stage that compiled is kept.
    if (ok) {
        keep_stage(kKind, source);
    }

    return ok;
}

bool compile_program(
    std::string_view vertex,
    std::string_view fragment,
    CompiledProgram& out,
    std::string& log,
    std::string_view compute
) {
    SpirvCache* cache = spirv_cache();
    const CacheText text = cache_text<3>("program", {vertex, fragment, compute});

    // Compiled before, and its encoding whole: glslang does not run.
    if (cache != nullptr) {
        // The kept program, unless its bytes are not one (it is then compiled and replaced).
        if (const std::vector<char>* kept = cache->find(text.key, text.check, text.length);
            kept != nullptr && decode_program(*kept, out)) {
            return true;
        }
    }

    const bool ok = compile_program_fresh(vertex, fragment, out, log, compute);

    // Kept for the next link of the same program, here or in a later run.
    if (ok && cache != nullptr) {
        cache->insert({text.key, text.check, text.length, encode_program(out)});
    }

    return ok;
}

}  // namespace openrac::vkgl

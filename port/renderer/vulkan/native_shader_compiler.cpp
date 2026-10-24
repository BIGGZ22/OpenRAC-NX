// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Compiles native Vulkan compute shaders with embedded includes and explicit descriptor reflection.
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>

#include "renderer/vulkan/shader_compiler.h"
#include "renderer/vulkan/shader_compiler_internal.h"

#include <SPIRV/GlslangToSpv.h>
#include <glslang/Include/Types.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

namespace openrac::vkgl {
namespace {
// Keeps include results alive until glslang releases them; source views belong to the caller.
class NativeIncludes final : public glslang::TShader::Includer {
public:
    // Borrows the source lookup for one synchronous compilation.
    explicit NativeIncludes(const std::function<std::string_view(std::string_view)>& lookup)
        : m_lookup(lookup) {}

    // Resolves SDK headers from embedded sources rather than the player's filesystem.
    IncludeResult* includeLocal(const char* name, const char* includer, std::size_t) override {
        std::string resolved(name);
        auto source = m_lookup(resolved);

        // Quoted SDK headers may refer to a neighbour in the including header's directory.
        if (source.empty() && includer) {
            const std::string_view parent(includer);
            const auto slash = parent.find_last_of('/');

            // Top-level headers already use the lookup root.
            if (slash != std::string_view::npos) {
                resolved = std::string(parent.substr(0, slash + 1)) + name;
                source = m_lookup(resolved);
            }
        }

        // A missing header must produce a compiler error, not silently compile an empty include.
        if (source.empty()) {
            return nullptr;
        }
        auto result =
            std::make_unique<IncludeResult>(resolved, source.data(), source.size(), nullptr);
        auto* pointer = result.get();
        m_results.emplace(pointer, std::move(result));
        return pointer;
    }

    // Both include spellings use the same bounded source lookup.
    IncludeResult* includeSystem(
        const char* name, const char* includer, std::size_t depth
    ) override {
        return includeLocal(name, includer, depth);
    }

    // Drops only the result wrapper; the embedded source remains owned by the caller.
    void releaseInclude(IncludeResult* result) override { m_results.erase(result); }

private:
    const std::function<std::string_view(std::string_view)>&
        m_lookup;  // Borrowed until parsing completes.
    std::unordered_map<IncludeResult*, std::unique_ptr<IncludeResult>>
        m_results;  // Live parser results.
};

// Records explicit native resources; SDK users retain their own binding names and numbers.
void reflect_native_compute(glslang::TProgram& program, NativeComputeShader& out) {
    // Named blocks are constant-buffer descriptors, not loose GL uniforms.
    for (int i = 0; i < program.getNumUniformBlocks(); ++i) {
        const auto& block = program.getUniformBlock(i);
        out.bindings.push_back(
            {block.name, NativeBindingKind::Constants, static_cast<unsigned>(block.getBinding()), 1}
        );
    }

    // Only opaque types represent standalone Vulkan descriptors.
    for (int i = 0; i < program.getNumUniformVariables(); ++i) {
        const auto& uniform = program.getUniform(i);
        const auto* type = uniform.getType();

        // Members of constant blocks are already represented by their parent descriptor.
        if (!type || type->getBasicType() != glslang::EbtSampler) {
            continue;
        }
        const auto& sampler = type->getSampler();
        auto kind = NativeBindingKind::SampledImage;

        // Samplers and storage images have distinct Vulkan descriptor types.
        if (sampler.isPureSampler()) {
            kind = NativeBindingKind::Sampler;
        } else if (sampler.isImage()) {
            kind = NativeBindingKind::StorageImage;
        }
        std::string name = uniform.name;
        name.resize(name.find('[') == std::string::npos ? name.size() : name.find('['));
        out.bindings.push_back(
            {std::move(name),
             kind,
             static_cast<unsigned>(uniform.getBinding()),
             static_cast<unsigned>(std::max(uniform.size, 1))}
        );
    }
}
}  // namespace

bool compile_native_compute(
    std::string_view source,
    std::string_view definitions,
    const std::function<std::string_view(std::string_view)>& include,
    NativeComputeShader& out,
    std::string& error
) {
    compiler_detail::initialise();
    glslang::TShader shader(EShLangCompute);
    const char* text = source.data();
    const int length = static_cast<int>(source.size());
    const std::string preamble(definitions);
    shader.setStringsWithLengths(&text, &length, 1);
    shader.setPreamble(preamble.c_str());
    shader.setEnvInput(glslang::EShSourceGlsl, EShLangCompute, glslang::EShClientVulkan, 450);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    NativeIncludes includes(include);
    glslang::TProgram program;

    // Native shaders keep their explicit descriptor layout, without relaxed GL conversion rules.
    if (!shader.parse(
            GetDefaultResources(),
            450,
            ECoreProfile,
            false,
            false,
            compiler_detail::kMessages,
            includes
        )) {
        error = shader.getInfoLog();
        return false;
    }
    program.addShader(&shader);

    // Reflection must describe the same linked program that produces the executable SPIR-V.
    if (!program.link(compiler_detail::kMessages)
        || !program.buildReflection(EShReflectionDefault)) {
        error = program.getInfoLog();
        return false;
    }
    out = {};
    reflect_native_compute(program, out);
    glslang::SpvOptions options;
    options.generateDebugInfo = false;
    options.disableOptimizer = true;
    spv::SpvBuildLogger logger;
    glslang::GlslangToSpv(*program.getIntermediate(EShLangCompute), out.code, &logger, &options);
    return !out.code.empty();
}

}  // namespace openrac::vkgl

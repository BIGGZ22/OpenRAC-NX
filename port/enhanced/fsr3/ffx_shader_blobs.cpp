// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Implements AMD's ffx_shader_blobs.h contract using its original GLSL and native glslang reflection.
#include <array>
#include <bit>
#include <cstring>
#include <vector>

#include "enhanced/fsr3/sdk_shaders.h"
#include "fsr3_sources.h"
#include "renderer/vulkan/shader_compiler.h"

#include <FidelityFX/host/ffx_fsr3upscaler.h>

// AMD's private declarations require the complete public context types above.
#include <ffx_fsr3upscaler_private.h>
#include <ffx_shader_blobs.h>

namespace {
// AMD's shader-blob callback has no user pointer. ShaderLibrary::activate lends this owner
// only while the graphics owner synchronously creates the SDK context.
openrac::enhanced::fsr3_detail::ShaderLibrary* g_shader_library = nullptr;
}  // namespace

namespace openrac::enhanced::fsr3_detail {
namespace {
// Only algorithm switches affect these FP32/default-subgroup shaders; no frame-generation effects are compiled.
constexpr unsigned kShaderSwitches = (1u << 6) - 1;

// Maps the upscaler's supported passes to unmodified AMD sources.
constexpr std::array<const char*, FFX_FSR3UPSCALER_PASS_TCR_AUTOGENERATE> kPassSources{
    "prepare_inputs",
    "luma_pyramid",
    "shading_change_pyramid",
    "shading_change",
    "prepare_reactivity",
    "luma_instability",
    "accumulate",
    "accumulate",
    "rcas",
    "debug_view",
    "autogen_reactive"
};

// Translates the SDK's permutation flags to its original GLSL preprocessor switches.
std::string shader_definitions(unsigned permutation) {
    std::string result = "#define FFX_GPU 1\n#define FFX_GLSL 1\n#define FFX_HALF 0\n";
    constexpr std::array<const char*, 6> kSwitches{
        "REPROJECT_USE_LANCZOS_TYPE",
        "HDR_COLOR_INPUT",
        "LOW_RESOLUTION_MOTION_VECTORS",
        "JITTERED_MOTION_VECTORS",
        "INVERTED_DEPTH",
        "APPLY_SHARPENING"
    };

    // These positions are the pinned SDK's Fs3UpscalerShaderPermutationOptions bits.
    for (unsigned bit = 0; bit < kSwitches.size(); ++bit) {
        result += "#define FFX_FSR3UPSCALER_OPTION_";
        result += kSwitches[bit];
        result += (permutation & (1u << bit)) ? " 1\n" : " 0\n";
    }

    return result;
}

// Resource reflection arrays kept stable until all SDK pipeline descriptions have been copied.
struct Bindings {
    std::vector<std::string> names;     // Names owned by this shader.
    std::vector<const char*> pointers;  // Frozen views populated after collecting names.
    std::vector<std::uint32_t> slots;   // Explicit descriptor bindings.
    std::vector<std::uint32_t> counts;  // Descriptor array lengths.
    std::vector<std::uint32_t> spaces;  // All descriptors belong to set zero.

    // Copies a descriptor and translates GLSL block type names to the SDK's instance names.
    void add(const vkgl::NativeShaderBinding& binding) {
        std::string name = binding.name;

        // glslang exposes the block type; the SDK matches its instance name.
        if (name == "cbFSR3UPSCALER_t") {
            name = "cbFSR3Upscaler";
        } else if (name.ends_with("_t")) {
            name.resize(name.size() - 2);
        }

        names.push_back(std::move(name));
        slots.push_back(binding.binding);
        counts.push_back(binding.count);
        spaces.push_back(0);
    }

    // Freezes views only after vector growth can no longer move short resource names.
    void freeze() {
        // Each pointer refers into the shader owner's final names vector.
        for (const auto& name : names) {
            pointers.push_back(name.c_str());
        }
    }

    // Narrows the small, bounded shader resource count to the SDK's public type.
    std::uint32_t count() const { return static_cast<std::uint32_t>(slots.size()); }
};
}  // namespace

// One linked shader and its four supported descriptor categories.
struct ShaderLibrary::Shader {
    vkgl::NativeComputeShader program;  // Owned executable bytecode and reflection.
    std::array<Bindings, 4> resources;  // Constants, sampled images, storage images and samplers.

    // Compiles the original pass without changing its reconstruction algorithm.
    bool compile(
        unsigned pass,
        unsigned permutation,
        const std::function<std::string_view(std::string_view)>& source,
        std::string& error
    ) {
        const std::string name =
            std::string("ffx_fsr3upscaler_") + kPassSources.at(pass) + "_pass.glsl";

        // Compiler diagnostics identify the upstream pass rather than a generated filename.
        if (!vkgl::compile_native_compute(
                source(name), shader_definitions(permutation), source, program, error
            )) {
            error = name + ": " + error;
            return false;
        }

        // NativeBindingKind uses the same category order as this resource array.
        for (const auto& binding : program.bindings) {
            resources.at(static_cast<unsigned>(binding.kind)).add(binding);
        }

        // SDK callbacks retain these views during pipeline creation.
        for (auto& bindings : resources) {
            bindings.freeze();
        }

        return true;
    }

    // Describes the shader in AMD's stable blob ABI, with no unused buffer or acceleration bindings.
    FfxShaderBlob blob() {
        auto& constants = resources[0];
        auto& sampled = resources[1];
        auto& storage = resources[2];
        auto& samplers = resources[3];
        // SPIR-V words cross AMD's byte-oriented shader-blob interface without copying.
        return FfxShaderBlob{
            .data = reinterpret_cast<const std::uint8_t*>(program.code.data()),
            .size = static_cast<std::uint32_t>(program.code.size() * sizeof(std::uint32_t)),
            .cbvCount = resources[0].count(),
            .srvTextureCount = resources[1].count(),
            .uavTextureCount = resources[2].count(),
            .srvBufferCount = 0,
            .uavBufferCount = 0,
            .samplerCount = resources[3].count(),
            .rtAccelStructCount = 0,
            .boundConstantBufferNames = constants.pointers.data(),
            .boundConstantBuffers = constants.slots.data(),
            .boundConstantBufferCounts = constants.counts.data(),
            .boundConstantBufferSpaces = constants.spaces.data(),
            .boundSRVTextureNames = sampled.pointers.data(),
            .boundSRVTextures = sampled.slots.data(),
            .boundSRVTextureCounts = sampled.counts.data(),
            .boundSRVTextureSpaces = sampled.spaces.data(),
            .boundUAVTextureNames = storage.pointers.data(),
            .boundUAVTextures = storage.slots.data(),
            .boundUAVTextureCounts = storage.counts.data(),
            .boundUAVTextureSpaces = storage.spaces.data(),
            .boundSRVBufferNames = nullptr,
            .boundSRVBuffers = nullptr,
            .boundSRVBufferCounts = nullptr,
            .boundSRVBufferSpaces = nullptr,
            .boundUAVBufferNames = nullptr,
            .boundUAVBuffers = nullptr,
            .boundUAVBufferCounts = nullptr,
            .boundUAVBufferSpaces = nullptr,
            .boundSamplerNames = samplers.pointers.data(),
            .boundSamplers = samplers.slots.data(),
            .boundSamplerCounts = samplers.counts.data(),
            .boundSamplerSpaces = samplers.spaces.data(),
            .boundRTAccelerationStructureNames = nullptr,
            .boundRTAccelerationStructures = nullptr,
            .boundRTAccelerationStructureCounts = nullptr,
            .boundRTAccelerationStructureSpaces = nullptr
        };
    }
};

ShaderLibrary::ShaderLibrary() {
    // Bounded chunks avoid compiler string-literal limits without changing any upstream source bytes.
    for (const auto& entry : kSdkSources) {
        auto& text = m_sources[std::string(entry.name)];

        // Concatenation preserves the original shader and include text exactly.
        for (const auto chunk : entry.chunks) {
            text.append(chunk);
        }
    }
}

ShaderLibrary::~ShaderLibrary() = default;

std::string_view ShaderLibrary::source(std::string_view name) const {
    const auto found = m_sources.find(name);
    return found == m_sources.end() ? std::string_view{} : std::string_view(found->second);
}

bool ShaderLibrary::prepare(std::string& error) {
    constexpr unsigned kBase = FSR3UPSCALER_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS
                               | FSR3UPSCALER_SHADER_PERMUTATION_DEPTH_INVERTED;

    // Only LDR, unjittered render-resolution motion is exposed by this integration.
    for (unsigned pass = 0; pass < kPassSources.size(); ++pass) {
        // Lanczos lookup and sharpening are independently selected by the SDK.
        for (unsigned extra : {0u, 1u, 32u, 33u}) {
            const auto key = std::pair{pass, kBase | extra};

            // Changing quality or resolution reuses the already compiled permutations.
            if (m_shaders.contains(key)) {
                continue;
            }

            auto shader = std::make_unique<Shader>();

            // A failed compilation prevents SDK context creation rather than returning an empty blob.
            if (!shader->compile(
                    pass, key.second, [this](std::string_view name) { return source(name); }, error
                )) {
                return false;
            }

            m_shaders.emplace(key, std::move(shader));
        }
    }

    return true;
}

bool ShaderLibrary::find(unsigned pass, unsigned permutation, FfxShaderBlob& out) const {
    const auto found = m_shaders.find({pass, permutation & kShaderSwitches});

    // Reject SDK modes the renderer has not supplied matching inputs for.
    if (found == m_shaders.end()) {
        return false;
    }

    const auto blob = found->second->blob();
    std::memcpy(&out, &blob, sizeof(blob));
    return true;
}

void ShaderLibrary::activate() {
    g_shader_library = this;
}

void ShaderLibrary::deactivate() {
    g_shader_library = nullptr;
}
}  // namespace openrac::enhanced::fsr3_detail

FfxErrorCode ffxGetPermutationBlobByIndex(
    FfxEffect effect, FfxPass pass, FfxBindStage, std::uint32_t permutation, FfxShaderBlob* out
) {
    // Only the prepared temporal-upscaler contract may cross this SDK callback boundary.
    if (effect != FFX_EFFECT_FSR3UPSCALER || !out || !g_shader_library
        || !g_shader_library->find(static_cast<unsigned>(pass), permutation, *out)) {
        return FFX_ERROR_BACKEND_API_ERROR;
    }

    return FFX_OK;
}

FfxErrorCode ffxIsWave64(FfxEffect, std::uint32_t, bool& wave64) {
    wave64 = false;
    return FFX_OK;
}

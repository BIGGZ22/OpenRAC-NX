// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Pipelines and descriptor sets (vkgl_internal.h): GL's vertex formats and fixed-function state as
// Vulkan pipelines, made once for each key and kept in the pipeline cache; the descriptor sets that
// bind a program's textures and blocks; glDispatchCompute and glMemoryBarrier; the warm-up after
// which a pipeline made counts as a hitch (vkgl.h, begin_pipeline_warmup); and the pre-warm, which
// makes a run's pipelines again in the next run on a worker thread, from the manifest of the keys
// made (R-10). The worker's thread runs only create_graphics_pipeline and run_prewarm_worker.

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <filesystem>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <volk.h>

#include "renderer/gl.h"
#include "renderer/render_opt.h"
#include "renderer/vulkan/pipeline_cache_file.h"
#include "renderer/vulkan/shader_compiler.h"
#include "renderer/vulkan/vkgl_internal.h"
#include "renderer/world/gl_world.h"

namespace openrac::vkgl::detail {

using namespace openrac::gl;

namespace {

// ---- Pipelines ----

VkBlendFactor blend_factor(GLenum f) {
    switch (f) {
        case GL_ZERO:
            return VK_BLEND_FACTOR_ZERO;
        case GL_ONE:
            return VK_BLEND_FACTOR_ONE;
        case GL_SRC_COLOR:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case GL_ONE_MINUS_SRC_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case GL_SRC_ALPHA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case GL_ONE_MINUS_SRC_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case GL_DST_ALPHA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case GL_ONE_MINUS_DST_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case GL_DST_COLOR:
            return VK_BLEND_FACTOR_DST_COLOR;
        case GL_ONE_MINUS_DST_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case GL_CONSTANT_COLOR:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case GL_ONE_MINUS_CONSTANT_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case GL_CONSTANT_ALPHA:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case GL_ONE_MINUS_CONSTANT_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case 0x0308:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        default:
            return VK_BLEND_FACTOR_ONE;
    }
}

VkBlendOp blend_op(GLenum e) {
    switch (e) {
        case GL_FUNC_SUBTRACT:
            return VK_BLEND_OP_SUBTRACT;
        case GL_FUNC_REVERSE_SUBTRACT:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case 0x8007:
            return VK_BLEND_OP_MIN;
        case 0x8008:
            return VK_BLEND_OP_MAX;
        default:
            return VK_BLEND_OP_ADD;
    }
}

VkCompareOp compare_op(GLenum f) {
    switch (f) {
        case GL_NEVER:
            return VK_COMPARE_OP_NEVER;
        case GL_LESS:
            return VK_COMPARE_OP_LESS;
        case GL_EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case GL_LEQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case GL_GREATER:
            return VK_COMPARE_OP_GREATER;
        case GL_NOTEQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case GL_GEQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        default:
            return VK_COMPARE_OP_ALWAYS;
    }
}

VkStencilOp stencil_op(GLenum op) {
    switch (op) {
        case GL_ZERO:
            return VK_STENCIL_OP_ZERO;
        case wgl::GL_REPLACE:
            return VK_STENCIL_OP_REPLACE;
        case 0x1E02:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case 0x1E03:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case 0x150A:
            return VK_STENCIL_OP_INVERT;
        case wgl::GL_INCR_WRAP:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case wgl::GL_DECR_WRAP:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default:
            return VK_STENCIL_OP_KEEP;
    }
}

VkPrimitiveTopology topology_of(GLenum mode) {
    switch (mode) {
        case GL_POINTS:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case GL_LINES:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case GL_LINE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case GL_TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case GL_TRIANGLE_FAN:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

// Keeps the complete graphics key after either cache or pre-warm lookup.
VkPipeline remember_pipeline(Program& program, std::string_view key, VkPipeline pipeline) {
    program.last_pipeline_key.assign(key);
    program.last_pipeline = pipeline;
    return pipeline;
}

// ---- Descriptors ----

VkDescriptorPool new_pool() {
    const VkDescriptorPoolSize sizes[] =
        {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4096},
         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4096},
         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096},
         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2048},
         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4096}};
    VkDescriptorPoolCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    info.maxSets = 2048;
    info.poolSizeCount = static_cast<std::uint32_t>(std::size(sizes));
    info.pPoolSizes = sizes;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    check(vkCreateDescriptorPool(g->device, &info, nullptr, &pool), "a descriptor pool");
    return pool;
}

BufferRef block_buffer(GLuint binding, int size) {
    Buffer* b =
        binding < kUniformBindings ? g->buffers.find(g->state.uniform_bindings[binding]) : nullptr;
    if (b == nullptr || b->data.empty()) {
        return {g->dummy_buffer, 0, 256};
    }
    if (b->data.size() < static_cast<std::size_t>(size)) {
        // Shorter than the block: the rest reads as zeros.
        Slice s = ring(static_cast<VkDeviceSize>(size));
        std::memset(s.data, 0, static_cast<std::size_t>(size));
        std::memcpy(s.data, b->data.data(), b->data.size());
        return {s.buffer, s.offset, static_cast<VkDeviceSize>(size)};
    }
    BufferRef r = resolve(*b);
    r.size = static_cast<VkDeviceSize>(size);
    return r;
}

}  // namespace

// ---- Pipelines by key ----

namespace {

// FNV-1a's 64-bit offset basis and prime (documented: the FNV hash's specification).
constexpr std::uint64_t kFnvBasis = 0xcbf29ce484222325u;
constexpr std::uint64_t kFnvPrime = 0x100000001b3u;

// The finaliser of SplitMix64 (documented: Steele, Lea and Flood's SplitMix), which spreads FNV's
// weak low bits over the whole word before they pick a slot.
constexpr std::uint64_t kMixFirst = 0xbf58476d1ce4e5b9u;
constexpr std::uint64_t kMixSecond = 0x94d049bb133111ebu;
constexpr int kMixShiftFirst = 30;
constexpr int kMixShiftSecond = 27;
constexpr int kMixShiftLast = 31;

// The slots PipelineTable starts with, a power of two.
constexpr std::size_t kFirstSlots = 64;

}  // namespace

std::uint64_t hash_words(std::span<const std::uint64_t> words) {
    std::uint64_t hash = kFnvBasis;

    // FNV-1a a word at a time rather than a byte: a key is read once a draw.
    for (const std::uint64_t word : words) {
        hash = (hash ^ word) * kFnvPrime;
    }

    hash = (hash ^ (hash >> kMixShiftFirst)) * kMixFirst;
    hash = (hash ^ (hash >> kMixShiftSecond)) * kMixSecond;
    return hash ^ (hash >> kMixShiftLast);
}

std::uint64_t hash_key(const PipelineKey& key) {
    constexpr std::size_t kWords = sizeof(PipelineKey) / sizeof(std::uint64_t);
    std::array<std::uint64_t, kWords> words{};
    std::memcpy(words.data(), &key, sizeof key);
    return hash_words(words);
}

const VkPipeline* PipelineTable::find(const PipelineKey& key, std::uint64_t hash) const {
    // Nothing made yet.
    if (m_slots.empty()) {
        return nullptr;
    }

    const std::size_t mask = m_slots.size() - 1;

    // Linear probing from the hash's slot; a free slot ends the search, and the table is never
    // more than half full, so one is always reached.
    for (std::size_t at = hash & mask;; at = (at + 1) & mask) {
        const std::uint32_t slot = m_slots[at];

        // A free slot: the key was never added.
        if (slot == 0) {
            return nullptr;
        }

        const std::size_t index = slot - 1;

        // A hash match is only a candidate: the whole key decides, so a collision never returns
        // another key's pipeline.
        if (m_hashes[index] == hash
            && std::memcmp(&m_entries[index].first, &key, sizeof key) == 0) {
            return &m_entries[index].second;
        }
    }
}

const VkPipeline* PipelineTable::find_reference(const std::string& key) const {
    const auto found = m_reference.find(key);
    return found != m_reference.end() ? &m_entries[found->second].second : nullptr;
}

void PipelineTable::insert(const PipelineKey& key, std::uint64_t hash, VkPipeline pipeline) {
    m_entries.emplace_back(key, pipeline);
    m_hashes.push_back(hash);

    // The slots are kept at least twice the entries, so a probe meets a free slot soon.
    if (m_entries.size() * 2 > m_slots.size()) {
        rebuild();
        return;
    }

    const std::size_t index = m_entries.size() - 1;
    place(index, hash);
    m_reference.emplace(
        std::string(reinterpret_cast<const char*>(&m_entries[index].first), sizeof key), index
    );
}

void PipelineTable::place(std::size_t index, std::uint64_t hash) {
    const std::size_t mask = m_slots.size() - 1;
    std::size_t at = hash & mask;

    // The first free slot from the hash's.
    while (m_slots[at] != 0) {
        at = (at + 1) & mask;
    }

    m_slots[at] = static_cast<std::uint32_t>(index + 1);
}

void PipelineTable::rebuild() {
    std::size_t slots = kFirstSlots;

    // The smallest power of two at least twice the entries.
    while (slots < m_entries.size() * 2) {
        slots *= 2;
    }

    m_slots.assign(slots, 0);
    m_hashes.resize(m_entries.size());
    m_reference.clear();

    // Each entry into both indexes again.
    for (std::size_t index = 0; index < m_entries.size(); ++index) {
        const PipelineKey& key = m_entries[index].first;
        m_hashes[index] = hash_key(key);
        place(index, m_hashes[index]);
        m_reference.emplace(std::string(reinterpret_cast<const char*>(&key), sizeof key), index);
    }
}

// ---- A program's descriptor sets by key ----

void DescriptorCache::clear() {
    words.clear();
    items.clear();
    slots.fill(0);
}

VkDescriptorSet DescriptorCache::find(
    std::span<const std::uint64_t> key, std::uint64_t hash
) const {
    constexpr std::size_t kMask = kSlots - 1;

    // Linear probing from the hash's slot; at most half the slots are full, so one is free.
    for (std::size_t at = hash & kMask;; at = (at + 1) & kMask) {
        const std::uint16_t slot = slots[at];

        // A free slot: no set has this key.
        if (slot == 0) {
            return VK_NULL_HANDLE;
        }

        const Item& item = items[slot - 1u];
        const std::span<const std::uint64_t> held(words.data() + item.at, item.size);

        // The hash picks candidates; the key's words decide.
        if (item.hash == hash && std::ranges::equal(held, key)) {
            return item.set;
        }
    }
}

void DescriptorCache::insert(
    std::span<const std::uint64_t> key, std::uint64_t hash, VkDescriptorSet set
) {
    // Full: the reference path stops keeping sets at the same count.
    if (items.size() >= kCapacity) {
        return;
    }

    constexpr std::size_t kMask = kSlots - 1;
    std::size_t at = hash & kMask;

    // The first free slot from the hash's.
    while (slots[at] != 0) {
        at = (at + 1) & kMask;
    }

    items.push_back(
        {hash,
         static_cast<std::uint32_t>(words.size()),
         static_cast<std::uint32_t>(key.size()),
         set}
    );
    words.insert(words.end(), key.begin(), key.end());
    slots[at] = static_cast<std::uint16_t>(items.size());
}

// ---- Pipelines ----

VkFormat vertex_format(GLenum type, int size, bool normalized, bool integer) {
    size = std::clamp(size, 1, 4);
    auto pick = [&](VkFormat a, VkFormat b, VkFormat c, VkFormat d) {
        const VkFormat f[] = {a, b, c, d};
        return f[size - 1];
    };
    switch (type) {
        case GL_FLOAT:
            return pick(
                VK_FORMAT_R32_SFLOAT,
                VK_FORMAT_R32G32_SFLOAT,
                VK_FORMAT_R32G32B32_SFLOAT,
                VK_FORMAT_R32G32B32A32_SFLOAT
            );
        case kGlHalfFloat:
            return pick(
                VK_FORMAT_R16_SFLOAT,
                VK_FORMAT_R16G16_SFLOAT,
                VK_FORMAT_R16G16B16_SFLOAT,
                VK_FORMAT_R16G16B16A16_SFLOAT
            );
        case GL_UNSIGNED_BYTE:
            if (integer) {
                return pick(
                    VK_FORMAT_R8_UINT,
                    VK_FORMAT_R8G8_UINT,
                    VK_FORMAT_R8G8B8_UINT,
                    VK_FORMAT_R8G8B8A8_UINT
                );
            }
            return normalized ? pick(
                                    VK_FORMAT_R8_UNORM,
                                    VK_FORMAT_R8G8_UNORM,
                                    VK_FORMAT_R8G8B8_UNORM,
                                    VK_FORMAT_R8G8B8A8_UNORM
                                )
                              : pick(
                                    VK_FORMAT_R8_USCALED,
                                    VK_FORMAT_R8G8_USCALED,
                                    VK_FORMAT_R8G8B8_USCALED,
                                    VK_FORMAT_R8G8B8A8_USCALED
                                );
        case kGlByte:
            if (integer) {
                return pick(
                    VK_FORMAT_R8_SINT,
                    VK_FORMAT_R8G8_SINT,
                    VK_FORMAT_R8G8B8_SINT,
                    VK_FORMAT_R8G8B8A8_SINT
                );
            }
            return normalized ? pick(
                                    VK_FORMAT_R8_SNORM,
                                    VK_FORMAT_R8G8_SNORM,
                                    VK_FORMAT_R8G8B8_SNORM,
                                    VK_FORMAT_R8G8B8A8_SNORM
                                )
                              : pick(
                                    VK_FORMAT_R8_SSCALED,
                                    VK_FORMAT_R8G8_SSCALED,
                                    VK_FORMAT_R8G8B8_SSCALED,
                                    VK_FORMAT_R8G8B8A8_SSCALED
                                );
        case GL_UNSIGNED_SHORT:
            if (integer) {
                return pick(
                    VK_FORMAT_R16_UINT,
                    VK_FORMAT_R16G16_UINT,
                    VK_FORMAT_R16G16B16_UINT,
                    VK_FORMAT_R16G16B16A16_UINT
                );
            }
            return normalized ? pick(
                                    VK_FORMAT_R16_UNORM,
                                    VK_FORMAT_R16G16_UNORM,
                                    VK_FORMAT_R16G16B16_UNORM,
                                    VK_FORMAT_R16G16B16A16_UNORM
                                )
                              : pick(
                                    VK_FORMAT_R16_USCALED,
                                    VK_FORMAT_R16G16_USCALED,
                                    VK_FORMAT_R16G16B16_USCALED,
                                    VK_FORMAT_R16G16B16A16_USCALED
                                );
        case GL_SHORT:
            if (integer) {
                return pick(
                    VK_FORMAT_R16_SINT,
                    VK_FORMAT_R16G16_SINT,
                    VK_FORMAT_R16G16B16_SINT,
                    VK_FORMAT_R16G16B16A16_SINT
                );
            }
            return normalized ? pick(
                                    VK_FORMAT_R16_SNORM,
                                    VK_FORMAT_R16G16_SNORM,
                                    VK_FORMAT_R16G16B16_SNORM,
                                    VK_FORMAT_R16G16B16A16_SNORM
                                )
                              : pick(
                                    VK_FORMAT_R16_SSCALED,
                                    VK_FORMAT_R16G16_SSCALED,
                                    VK_FORMAT_R16G16B16_SSCALED,
                                    VK_FORMAT_R16G16B16A16_SSCALED
                                );
        case GL_UNSIGNED_INT:
            return pick(
                VK_FORMAT_R32_UINT,
                VK_FORMAT_R32G32_UINT,
                VK_FORMAT_R32G32B32_UINT,
                VK_FORMAT_R32G32B32A32_UINT
            );
        case GL_INT:
            return pick(
                VK_FORMAT_R32_SINT,
                VK_FORMAT_R32G32_SINT,
                VK_FORMAT_R32G32B32_SINT,
                VK_FORMAT_R32G32B32A32_SINT
            );
        default:
            return VK_FORMAT_UNDEFINED;
    }
}

int type_bytes(GLenum type) {
    switch (type) {
        case GL_UNSIGNED_BYTE:
        case kGlByte:
            return 1;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case kGlHalfFloat:
            return 2;
        default:
            return 4;
    }
}

int input_kind(std::uint32_t gl_type) {
    switch (gl_type) {
        case GL_UNSIGNED_INT:
        case 0x8DC6:
        case 0x8DC7:
        case 0x8DC8:
            return 1;
        case GL_INT:
        case 0x8B53:
        case 0x8B54:
        case 0x8B55:
            return 2;
        default:
            return 0;
    }
}

// ---- Pipelines made ahead (R-10) ----

namespace {

// The keys the manifest holds at most; past it a new key is not added (assumed far above a run's
// few hundred pipelines).
constexpr std::size_t kManifestKeys = 4096;

// The runs a key stays in the manifest without being made or used: a program that is no longer
// built (its shaders changed) leaves the manifest after so many runs.
constexpr std::uint32_t kManifestMaxAge = 8;

// The largest sample count a key may name (VK_SAMPLE_COUNT_64_BIT).
constexpr std::uint32_t kMostSamples = 64;

// Returns whether the pre-warm is on (pipeline-prewarm); off, every pipeline is made at its draw.
bool prewarm_enabled() {
    return renderer::render_opt::enabled(renderer::render_opt::Opt::PipelinePrewarm);
}

// Returns `key`'s bytes, the manifest's index of it.
std::string key_bytes(const PipelineKey& key) {
    return std::string(reinterpret_cast<const char*>(&key), sizeof key);
}

// Returns whether `a` and `b` are the same key, byte for byte.
bool same_key(const PipelineKey& a, const PipelineKey& b) {
    return std::memcmp(&a, &b, sizeof a) == 0;
}

// Returns whether a key read from a manifest is one this device could make: its inputs, its
// viewports and its samples in range. A whole file can still hold a key of another device.
bool plausible_key(const PipelineKey& key) {
    const bool samples =
        key.samples != 0 && key.samples <= kMostSamples && (key.samples & (key.samples - 1)) == 0;
    return key.inputs <= static_cast<std::uint32_t>(kAttributes) && key.viewports >= 1
           && key.viewports <= g->properties.limits.maxViewports && samples
           && key.topology <= VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
}

// Makes the graphics pipeline of key `k` with the stages `vertex_module` and `fragment_module`
// and `layout`, through the shared pipeline cache; null when the driver refuses it. It reads only
// its arguments and the device, so the pre-warm worker calls it as the thread that draws does:
// whether the pass has colour and stencil comes from the key's formats (VK_FORMAT_UNDEFINED, 0,
// for none). Counts nothing: the caller does.
VkPipeline create_graphics_pipeline(
    const PipelineKey& k,
    VkShaderModule vertex_module,
    VkShaderModule fragment_module,
    VkPipelineLayout layout
) {
    const bool colour = k.colour_format != VK_FORMAT_UNDEFINED;
    const auto depth_format = static_cast<VkFormat>(k.depth_format);
    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0] =
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         nullptr,
         0,
         VK_SHADER_STAGE_VERTEX_BIT,
         vertex_module,
         "main",
         nullptr};
    stages[1] =
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         nullptr,
         0,
         VK_SHADER_STAGE_FRAGMENT_BIT,
         fragment_module,
         "main",
         nullptr};
    VkVertexInputBindingDescription bindings[kAttributes]{};
    VkVertexInputAttributeDescription attributes[kAttributes]{};
    for (std::uint32_t i = 0; i < k.inputs; ++i) {
        bindings[i] = {
            i,
            k.input[i].stride,
            k.input[i].instanced != 0 ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX
        };
        attributes[i] = {k.input[i].location, i, static_cast<VkFormat>(k.input[i].format), 0};
    }
    VkPipelineVertexInputStateCreateInfo vertex{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    vertex.vertexBindingDescriptionCount = k.inputs;
    vertex.pVertexBindingDescriptions = bindings;
    vertex.vertexAttributeDescriptionCount = k.inputs;
    vertex.pVertexAttributeDescriptions = attributes;
    VkPipelineInputAssemblyStateCreateInfo assembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
    };
    assembly.topology = static_cast<VkPrimitiveTopology>(k.topology);
    assembly.primitiveRestartEnable = k.restart;
    VkPipelineViewportStateCreateInfo viewport{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
    };
    viewport.viewportCount = k.viewports;
    viewport.scissorCount = k.viewports;
    VkPipelineRasterizationStateCreateInfo raster{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
    };
    raster.polygonMode = k.polygon != 0 ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    raster.cullMode = k.fixed.cull;
    raster.frontFace = static_cast<VkFrontFace>(k.fixed.front);
    raster.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo multisample{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
    };
    multisample.rasterizationSamples = static_cast<VkSampleCountFlagBits>(k.samples);
    VkPipelineDepthStencilStateCreateInfo depth{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
    };
    depth.depthTestEnable = k.fixed.depth_test;
    depth.depthWriteEnable = k.fixed.depth_write;
    depth.depthCompareOp = k.fixed.depth_test != 0 ? static_cast<VkCompareOp>(k.fixed.depth_func)
                                                   : VK_COMPARE_OP_ALWAYS;
    depth.stencilTestEnable = k.fixed.stencil;
    depth.front =
        {static_cast<VkStencilOp>(k.fixed.front_ops[0]),
         static_cast<VkStencilOp>(k.fixed.front_ops[1]),
         static_cast<VkStencilOp>(k.fixed.front_ops[2]),
         static_cast<VkCompareOp>(k.fixed.stencil_func),
         0,
         0,
         0};
    depth.back =
        {static_cast<VkStencilOp>(k.fixed.back_ops[0]),
         static_cast<VkStencilOp>(k.fixed.back_ops[1]),
         static_cast<VkStencilOp>(k.fixed.back_ops[2]),
         static_cast<VkCompareOp>(k.fixed.stencil_func),
         0,
         0,
         0};
    depth.maxDepthBounds = 1.0f;
    VkPipelineColorBlendAttachmentState blend{};
    blend.blendEnable = k.blend;
    blend.srcColorBlendFactor = static_cast<VkBlendFactor>(k.src_rgb);
    blend.dstColorBlendFactor = static_cast<VkBlendFactor>(k.dst_rgb);
    blend.colorBlendOp = static_cast<VkBlendOp>(k.equation);
    blend.srcAlphaBlendFactor = static_cast<VkBlendFactor>(k.src_a);
    blend.dstAlphaBlendFactor = static_cast<VkBlendFactor>(k.dst_a);
    blend.alphaBlendOp = static_cast<VkBlendOp>(k.equation);
    blend.colorWriteMask = k.colour_mask;
    VkPipelineColorBlendStateCreateInfo blending{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
    };
    blending.attachmentCount = colour ? 1 : 0;
    blending.pAttachments = &blend;
    // The first six are dynamic in every pipeline; the rest only where the key says the fixed
    // state is (R-11): Vulkan 1.3's core extended dynamic state, the items MoltenVK takes in full.
    const VkDynamicState dynamic_states[] =
        {VK_DYNAMIC_STATE_VIEWPORT,
         VK_DYNAMIC_STATE_SCISSOR,
         VK_DYNAMIC_STATE_BLEND_CONSTANTS,
         VK_DYNAMIC_STATE_STENCIL_REFERENCE,
         VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
         VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
         VK_DYNAMIC_STATE_CULL_MODE,
         VK_DYNAMIC_STATE_FRONT_FACE,
         VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
         VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
         VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
         VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
         VK_DYNAMIC_STATE_STENCIL_OP};
    constexpr std::uint32_t kAlwaysDynamic = 6;
    VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic.dynamicStateCount = k.dynamic_fixed != 0
                                    ? static_cast<std::uint32_t>(std::size(dynamic_states))
                                    : kAlwaysDynamic;
    dynamic.pDynamicStates = dynamic_states;
    const VkFormat colour_format = static_cast<VkFormat>(k.colour_format);
    VkPipelineRenderingCreateInfo rendering{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    rendering.colorAttachmentCount = colour ? 1 : 0;
    rendering.pColorAttachmentFormats = &colour_format;
    rendering.depthAttachmentFormat = static_cast<VkFormat>(k.depth_format);
    rendering.stencilAttachmentFormat =
        has_stencil(depth_format) ? depth_format : VK_FORMAT_UNDEFINED;
    VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    info.pNext = &rendering;
    info.stageCount = 2;
    info.pStages = stages;
    info.pVertexInputState = &vertex;
    info.pInputAssemblyState = &assembly;
    info.pViewportState = &viewport;
    info.pRasterizationState = &raster;
    info.pMultisampleState = &multisample;
    info.pDepthStencilState = &depth;
    info.pColorBlendState = &blending;
    info.pDynamicState = &dynamic;
    info.layout = layout;
    VkPipeline pipeline = VK_NULL_HANDLE;

    // A pipeline the driver refuses is null: its draws are skipped, as before a cache existed.
    if (!check(
            vkCreateGraphicsPipelines(g->device, g->pipeline_cache, 1, &info, nullptr, &pipeline),
            "a pipeline"
        )) {
        pipeline = VK_NULL_HANDLE;
    }

    return pipeline;
}

// Moves what the worker made into Context::pipelines. A key already there (made by a draw while
// the entry was off) keeps its pipeline, and the worker's is destroyed: no command used it.
// `w.mutex` must be held.
void take_ready(PipelinePrewarm& w) {
    // Each pipeline made; a null one (refused by the driver) is left for the draw to make.
    for (const auto& [key, pipeline] : w.ready) {
        // Refused: the draw makes it, and logs the refusal again.
        if (pipeline == VK_NULL_HANDLE) {
            continue;
        }

        const std::uint64_t hash = hash_key(key);

        // A draw made it first.
        if (g->pipelines.find(key, hash) != nullptr) {
            vkDestroyPipeline(g->device, pipeline, nullptr);
            continue;
        }

        g->pipelines.insert(key, hash, pipeline);
    }

    w.ready.clear();
}

// Moves the job of key `k` to the front of `queue`. False when no job has that key.
bool move_to_front(std::deque<PrewarmJob>& queue, const PipelineKey& k) {
    const auto found =
        std::ranges::find_if(queue, [&k](const PrewarmJob& job) { return same_key(job.key, k); });

    // Not queued.
    if (found == queue.end()) {
        return false;
    }

    // Already next.
    if (found == queue.begin()) {
        return true;
    }

    PrewarmJob job = *found;
    queue.erase(found);
    queue.push_front(job);
    return true;
}

// Returns the pipeline of key `k` (whose hash_key is `hash`) if the pre-warm made it, waiting for
// it when it is queued or being made; nothing when the pre-warm has no such key or is off, and
// the draw makes it. Hot path: one lock and three tests once the worker has nothing left.
std::optional<VkPipeline> prewarm_take(const PipelineKey& k, std::uint64_t hash) {
    // Off: every pipeline is made by its draw.
    if (!prewarm_enabled()) {
        return std::nullopt;
    }

    PipelinePrewarm& w = g->prewarm;
    std::unique_lock lock(w.mutex);

    // Ends when the key is in the table, or is neither queued nor being made.
    for (;;) {
        take_ready(w);

        // Made by the worker.
        if (const VkPipeline* made = g->pipelines.find(k, hash)) {
            return *made;
        }

        const bool queued = move_to_front(w.queue, k);
        const bool making = w.making.has_value() && same_key(*w.making, k);

        // Not the pre-warm's: the draw makes it.
        if (!queued && !making) {
            return std::nullopt;
        }

        // Waiting costs at most the job being made: this key is the next one.
        w.changed.wait(lock);
    }
}

// Adds key `k` of program `p` to the manifest, as made by this run, unless the program has no
// identity or the manifest is full; a key already there becomes this run's again.
void record_manifest_key(const Program& p, const PipelineKey& k) {
    // A compute program, or one linked before the manifest knew it.
    if (p.identity == 0) {
        return;
    }

    PipelineManifest& manifest = g->manifest;
    ManifestKey entry{k, 0};
    entry.key.program = p.identity;
    std::string bytes = key_bytes(entry.key);

    // Known: made again this run.
    if (const auto found = manifest.index.find(bytes); found != manifest.index.end()) {
        manifest.keys[found->second].age = 0;
        return;
    }

    // Full: older keys are not pushed out by a run's new ones.
    if (manifest.keys.size() >= kManifestKeys) {
        return;
    }

    manifest.index.emplace(std::move(bytes), manifest.keys.size());
    manifest.keys.push_back(entry);
}

// The pre-warm worker: makes each queued job's pipeline, in order, until stop is set. Runs on its
// own thread (PipelinePrewarm::worker) and touches nothing but `w` and the device.
void run_prewarm_worker(PipelinePrewarm& w) {
    std::unique_lock lock(w.mutex);

    // Ends when stop_pipeline_prewarm sets stop.
    for (;;) {
        w.changed.wait(lock, [&w] { return w.stop || !w.queue.empty(); });

        // Closing: what is still queued is not made.
        if (w.stop) {
            return;
        }

        const PrewarmJob job = w.queue.front();
        w.queue.pop_front();
        w.making = job.key;

        // The pipeline is made without the lock, so a draw may queue, find or wait meanwhile.
        lock.unlock();
        const VkPipeline pipeline =
            create_graphics_pipeline(job.key, job.vertex, job.fragment, job.layout);
        lock.lock();

        w.ready.emplace_back(job.key, pipeline);
        ++w.made;
        w.making.reset();
        w.changed.notify_all();
    }
}

}  // namespace

std::uint64_t pipeline_identity(const Program& p) {
    const CompiledProgram& c = p.compiled;

    // Returns the hash of one stage's SPIR-V words, two to each 64-bit word, and their count.
    const auto stage_hash = [](const std::vector<std::uint32_t>& words) {
        std::uint64_t hash = kFnvBasis;

        // Each pair of words as one, the odd last one alone.
        for (std::size_t n = 0; n < words.size(); n += 2) {
            const std::uint64_t high = n + 1 < words.size() ? words[n + 1] : 0;
            hash = (hash ^ (words[n] | (high << 32U))) * kFnvPrime;
        }

        return hash ^ words.size();
    };

    // The layout follows the material pages and the dynamic uniforms, so they are part of it.
    const std::uint64_t layout = (p.gpu_materials ? 1U : 0U) | (p.dynamic_uniforms ? 2U : 0U);
    const std::array<std::uint64_t, 3> parts =
        {stage_hash(c.vertex), stage_hash(c.fragment), layout};
    const std::uint64_t identity = hash_words(parts);

    // 0 means no identity: a program that hashes to it takes the next value.
    return identity != 0 ? identity : 1;
}

void prewarm_program(Program& p) {
    const bool graphics = p.linked && p.vertex != VK_NULL_HANDLE && p.fragment != VK_NULL_HANDLE;
    p.identity = graphics ? pipeline_identity(p) : 0;

    // A compute program has no graphics pipelines; off, nothing is made ahead.
    if (p.identity == 0 || !prewarm_enabled()) {
        return;
    }

    PipelinePrewarm& w = g->prewarm;
    std::lock_guard lock(w.mutex);
    const std::size_t queued = w.queue.size();

    // Each key of this program in the manifest, as a job for this link's serial.
    for (ManifestKey& entry : g->manifest.keys) {
        // Another program's key.
        if (entry.key.program != p.identity) {
            continue;
        }

        PrewarmJob job{entry.key, p.vertex, p.fragment, p.layout};
        job.key.program = p.serial;
        w.queue.push_back(job);
        entry.age = 0;
    }

    // Nothing of this program was made in an earlier run.
    if (w.queue.size() == queued) {
        return;
    }

    // The first jobs start the worker; it lives until the backend closes.
    if (!w.started) {
        // The worker's body, on its own thread until stop_pipeline_prewarm.
        w.worker = JoiningThread([&w] { run_prewarm_worker(w); });
        w.started = true;
    }

    w.changed.notify_all();
}

void prewarm_forget(std::uint64_t serial) {
    PipelinePrewarm& w = g->prewarm;
    std::unique_lock lock(w.mutex);
    std::erase_if(w.queue, [serial](const PrewarmJob& job) { return job.key.program == serial; });

    // The program's modules must outlive the job that uses them.
    w.changed.wait(lock, [&w, serial] {
        return !w.making.has_value() || w.making->program != serial;
    });

    // What was made for it and never drawn with: no command buffer holds it.
    std::erase_if(w.ready, [serial](const std::pair<PipelineKey, VkPipeline>& done) {
        // Another program's pipeline stays.
        if (done.first.program != serial) {
            return false;
        }

        vkDestroyPipeline(g->device, done.second, nullptr);
        return true;
    });
}

void stop_pipeline_prewarm() {
    PipelinePrewarm& w = g->prewarm;

    // The worker stops after the job it is making, if any.
    {
        std::lock_guard lock(w.mutex);
        w.stop = true;
        w.queue.clear();
        w.changed.notify_all();
    }

    w.worker.join();

    // Made, never drawn with.
    for (const auto& [key, pipeline] : w.ready) {
        vkDestroyPipeline(g->device, pipeline, nullptr);
    }

    w.ready.clear();

    // A run that made pipelines ahead says how many.
    if (w.made != 0) {
        log::info("vulkan: {} pipelines made ahead on the pre-warm worker", w.made);
    }
}

void load_pipeline_manifest(const std::filesystem::path& cache) {
    PipelineManifest& manifest = g->manifest;
    manifest.path.clear();

    // No cache file, so no manifest beside it; or the pre-warm is off.
    if (cache.empty() || !prewarm_enabled()) {
        return;
    }

    manifest.path = cache;
    manifest.path += ".manifest";
    const SavedManifest saved = read_pipeline_manifest(manifest.path, sizeof(PipelineKey));

    // The first run: no manifest yet.
    if (saved.state == SavedCacheState::Missing) {
        return;
    }

    // A manifest of another build, or a damaged one: the next save replaces it.
    if (saved.state == SavedCacheState::Damaged) {
        log::warn(
            "vulkan: pipeline manifest {} is damaged or stale, dropped", manifest.path.string()
        );
        return;
    }

    // Each key, a run older, unless a key cannot be this device's.
    for (const ManifestEntry& entry : saved.entries) {
        ManifestKey key{};
        std::memcpy(&key.key, entry.key.data(), sizeof key.key);
        key.key.program = entry.program;
        key.age = entry.age + 1;
        std::string bytes = key_bytes(key.key);

        // Out of range for this device, or a repeat.
        if (!plausible_key(key.key) || manifest.index.contains(bytes)
            || manifest.keys.size() >= kManifestKeys) {
            continue;
        }

        manifest.index.emplace(std::move(bytes), manifest.keys.size());
        manifest.keys.push_back(key);
    }

    log::info(
        "vulkan: {} pipeline keys read from {}", manifest.keys.size(), manifest.path.string()
    );
}

void save_pipeline_manifest() {
    const PipelineManifest& manifest = g->manifest;

    // No file to keep, or the pre-warm is off and the manifest stays as it was.
    if (manifest.path.empty() || !prewarm_enabled()) {
        return;
    }

    std::vector<ManifestEntry> entries;
    entries.reserve(manifest.keys.size());

    // Each key some run made or used in the last kManifestMaxAge runs.
    for (const ManifestKey& key : manifest.keys) {
        // A program not built for that long: its shaders changed, or its level is gone.
        if (key.age > kManifestMaxAge) {
            continue;
        }

        const auto* bytes = reinterpret_cast<const char*>(&key.key);
        entries.push_back(
            {key.key.program, key.age, std::vector<char>(bytes, bytes + sizeof key.key)}
        );
    }

    std::string error;

    // Saved like the cache: whole, through a file of its own.
    if (!write_pipeline_manifest(manifest.path, entries, sizeof(PipelineKey), error)) {
        log::warn("vulkan: could not save the pipeline manifest: {}", error);
    }
}

bool fixed_state_dynamic() {
    return g->extended_dynamic_state
           && renderer::render_opt::enabled(renderer::render_opt::Opt::VkExtendedDynamicState);
}

FixedState fixed_state(const Target& t) {
    const State& s = g->state;
    FixedState f{};
    f.cull = !s.cull                            ? VK_CULL_MODE_NONE
             : s.cull_face == GL_FRONT          ? VK_CULL_MODE_FRONT_BIT
             : s.cull_face == GL_FRONT_AND_BACK ? VK_CULL_MODE_FRONT_AND_BACK
                                                : VK_CULL_MODE_BACK_BIT;
    // The same rows as OpenGL's, counted from the other side: what GL calls counter-clockwise,
    // Vulkan's sign of the area calls clockwise.
    f.front = s.front_face == GL_CCW ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

    // No depth attachment: no depth or stencil test.
    if (t.depth == nullptr) {
        return f;
    }

    // GL writes depth only while it tests it.
    f.depth_test = s.depth_test ? 1 : 0;
    f.depth_write = s.depth_test && s.depth_mask ? 1 : 0;
    f.depth_func = s.depth_test ? compare_op(s.depth_func) : 0;

    // The stencil test, where the target has stencil.
    if (s.stencil_test && has_stencil(t.depth->format)) {
        f.stencil = 1;
        f.stencil_func = compare_op(s.stencil_func);
        f.front_ops[0] = stencil_op(s.stencil_front.sfail);
        f.front_ops[1] = stencil_op(s.stencil_front.dppass);
        f.front_ops[2] = stencil_op(s.stencil_front.dpfail);
        f.back_ops[0] = stencil_op(s.stencil_back.sfail);
        f.back_ops[1] = stencil_op(s.stencil_back.dppass);
        f.back_ops[2] = stencil_op(s.stencil_back.dpfail);
    }

    return f;
}

VkPipeline pipeline_for(
    Program& p, GLenum mode, const Target& t, const Input* inputs, std::uint32_t count
) {
    const State& s = g->state;
    PipelineKey k;
    std::memset(&k, 0, sizeof k);
    k.program = p.serial;
    k.topology = static_cast<std::uint32_t>(topology_of(mode));
    const bool strip =
        mode == GL_LINE_STRIP || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN;
    k.restart = s.primitive_restart && strip ? 1 : 0;
    k.polygon = s.polygon_mode == GL_LINE && g->features.fillModeNonSolid ? 1 : 0;
    k.dynamic_fixed = fixed_state_dynamic() ? 1 : 0;

    // Baked into the pipeline, unless the draw records it as dynamic state (R-11).
    if (k.dynamic_fixed == 0) {
        k.fixed = fixed_state(t);
    }

    if (t.colour != nullptr) {
        k.blend = s.blend ? 1 : 0;
        if (s.blend) {
            k.src_rgb = blend_factor(s.blend_src_rgb);
            k.dst_rgb = blend_factor(s.blend_dst_rgb);
            k.src_a = blend_factor(s.blend_src_a);
            k.dst_a = blend_factor(s.blend_dst_a);
            k.equation = blend_op(s.blend_equation);
        }
        k.colour_mask = (s.colour_mask[0] ? 1u : 0u) | (s.colour_mask[1] ? 2u : 0u)
                        | (s.colour_mask[2] ? 4u : 0u) | (s.colour_mask[3] ? 8u : 0u);
        k.colour_format = t.colour->format;
    }
    if (t.depth != nullptr) {
        k.depth_format = t.depth->format;
    }
    k.samples = t.samples;
    k.inputs = count;
    k.viewports = static_cast<std::uint32_t>(std::max<std::size_t>(1, g->indexed_viewports.size()));
    std::memcpy(k.input, inputs, sizeof(Input) * count);
    // Repeated draws bypass hashing only when every baked graphics field matches.
    const std::string_view bytes(reinterpret_cast<const char*>(&k), sizeof k);
    if (p.last_pipeline != VK_NULL_HANDLE && bytes == p.last_pipeline_key) {
        return p.last_pipeline;
    }
    const std::uint64_t hash = hash_key(k);

    // A key made before: by its hash and a compare of the whole key (vk-draw-cache), or by its
    // bytes as a string, which allocates for every draw (the reference path).
    if (renderer::render_opt::enabled(renderer::render_opt::Opt::VkDrawCache)) {
        // Found by its hash.
        if (const VkPipeline* made = g->pipelines.find(k, hash)) {
            return remember_pipeline(p, bytes, *made);
        }
    } else if (const VkPipeline* made = g->pipelines.find_reference(
                   std::string(reinterpret_cast<const char*>(&k), sizeof k)
               )) {
        // Found by its bytes.
        return remember_pipeline(p, bytes, *made);
    }

    // Made ahead by the pre-warm worker, or being made: taken from it rather than made again.
    if (const std::optional<VkPipeline> warmed = prewarm_take(k, hash)) {
        return remember_pipeline(p, bytes, *warmed);
    }

    const VkPipeline pipeline = create_graphics_pipeline(k, p.vertex, p.fragment, p.layout);
    ++g->measures.pipelines;
    count_pipeline();
    record_manifest_key(p, k);
    g->pipelines.insert(k, hash, pipeline);
    return remember_pipeline(p, bytes, pipeline);
}

// ---- Descriptors ----

VkDescriptorSet allocate_set(VkDescriptorSetLayout layout) {
    Frame& f = frame();
    while (true) {
        if (f.pool_at >= f.pools.size()) {
            const VkDescriptorPool pool = new_pool();
            if (pool == VK_NULL_HANDLE) {
                return VK_NULL_HANDLE;
            }
            f.pools.push_back(pool);
        }
        VkDescriptorSetAllocateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        info.descriptorPool = f.pools[f.pool_at];
        info.descriptorSetCount = 1;
        info.pSetLayouts = &layout;
        VkDescriptorSet set = VK_NULL_HANDLE;
        if (vkAllocateDescriptorSets(g->device, &info, &set) == VK_SUCCESS) {
            ++g->submission.descriptor_sets_allocated;
            return set;
        }
        ++f.pool_at;
    }
}

Sampled sampled(const Program& p, std::size_t i) {
    const ProgramUniform& u = p.compiled.uniforms[i];
    const int unit = std::clamp(p.units[i], 0, kUnits - 1);
    const State& s = g->state;
    Sampled out;
    if (u.kind == SamplerKind::Buffer) {
        Texture* t = g->textures.find(s.unit_buffer[static_cast<std::size_t>(unit)]);
        // An absent/incomplete texture must still supply a descriptor with
        // the shader's numeric type, including uniforms behind disabled paths.
        const auto fallback = g->dummy_views
                                  [u.gl_type == kGlUnsignedSamplerBuffer ? 0
                                   : u.gl_type == kGlIntSamplerBuffer    ? 3
                                                                         : 2];
        out.texels = t != nullptr ? texel_view(t->buffer, t->buffer_format, fallback) : fallback;
        return out;
    }
    Texture* t = g->textures.find(
        u.kind == SamplerKind::Texture2DArray ? s.unit_array[static_cast<std::size_t>(unit)]
                                              : s.unit_2d[static_cast<std::size_t>(unit)]
    );
    Image* image = t != nullptr && t->image ? t->image.get() : g->dummy_image.get();
    SamplerParams params = t != nullptr ? t->params : SamplerParams{};
    if (const SamplerParams* own =
            g->samplers.find(s.unit_sampler[static_cast<std::size_t>(unit)])) {
        params = *own;
    }
    const bool integer =
        image->format == VK_FORMAT_R32_UINT || image->format == VK_FORMAT_R32G32B32A32_UINT;
    out.image = image;
    out.view =
        sample_view(*image, t != nullptr ? t->base_level : 0, t != nullptr ? t->max_level : 0);
    out.sampler = sampler_for(params, integer);
    return out;
}

namespace {

// Forgets program `p`'s sets of an earlier frame and uploads its default uniforms when they
// changed or the frame is new: the start of descriptors() on either path.
void begin_program_sets(Program& p) {
    const CompiledProgram& c = p.compiled;

    // A new frame: the sets of the last were made from pools that are reset with their frame.
    if (p.last_set_frame != g->frame_count) {
        // Pools and ring allocations retire together after the frame-slot fence.
        p.descriptor_sets.clear();
        p.set_cache.clear();
        p.last_set = VK_NULL_HANDLE;
    }

    // The default uniforms go into the frame's ring when they changed, or once a frame.
    if (c.default_binding >= 0 && (p.uniforms_dirty || p.uniforms_frame != g->frame_count)) {
        const Slice s = ring_copy(p.uniforms.data(), static_cast<std::size_t>(c.default_size));
        p.uniforms_buffer = s.buffer;
        p.uniforms_offset = s.offset;
        p.uniforms_frame = g->frame_count;
        p.uniforms_dirty = false;
        ++g->submission.uniform_uploads;
        g->submission.uniform_upload_bytes += static_cast<std::size_t>(c.default_size);
    }
}

// Fills `blocks` with what program `p`'s uniform blocks and storage blocks bind now, in that
// order, and `key` with the set's key: the uniforms' buffer and offset, then each block's buffer,
// offset and size, then each sampler's view, sampler and texel view.
void descriptor_key(
    const Program& p,
    std::span<const Sampled> samplers,
    std::vector<BufferRef>& blocks,
    std::vector<std::uint64_t>& key
) {
    const CompiledProgram& c = p.compiled;
    blocks.clear();
    key.clear();

    // Each uniform block at its GL binding point.
    for (std::size_t i = 0; i < c.blocks.size(); ++i) {
        blocks.push_back(block_buffer(p.block_bindings[i], c.blocks[i].size));
    }

    // Each storage block, or the dummy buffer when its binding names no buffer.
    for (const auto& block : c.storage) {
        auto* b = block.binding >= 0 && block.binding < kUniformBindings
                      ? g->buffers.find(g->state.storage_bindings[block.binding])
                      : nullptr;
        blocks.push_back(b ? resolve(*b) : dummy_buffer_ref());
    }

    // A dynamic uniform buffer's offset is given at the bind, so it is not part of the set.
    key.push_back(reinterpret_cast<std::uint64_t>(p.uniforms_buffer));
    key.push_back(p.dynamic_uniforms ? 0 : p.uniforms_offset);

    // The blocks' ranges.
    for (const BufferRef& b : blocks) {
        key.push_back(reinterpret_cast<std::uint64_t>(b.buffer));
        key.push_back(b.offset);
        key.push_back(b.size);
    }

    // The samplers' descriptors.
    for (const Sampled& s : samplers) {
        key.push_back(reinterpret_cast<std::uint64_t>(s.view));
        key.push_back(reinterpret_cast<std::uint64_t>(s.sampler));
        key.push_back(reinterpret_cast<std::uint64_t>(s.texels));
    }
}

// Writes into the new `set` program `p`'s default uniforms, `blocks` and `samplers`, through
// `scratch`'s writes and infos.
void write_set(
    const Program& p,
    VkDescriptorSet set,
    std::span<const BufferRef> blocks,
    std::span<const Sampled> samplers,
    DrawScratch& scratch
) {
    const CompiledProgram& c = p.compiled;
    std::vector<VkWriteDescriptorSet>& writes = scratch.writes;
    std::vector<VkDescriptorBufferInfo>& buffer_infos = scratch.buffer_infos;
    std::vector<VkDescriptorImageInfo>& image_infos = scratch.image_infos;
    writes.clear();
    buffer_infos.clear();
    image_infos.clear();

    // The writes point into the infos, so these must not grow while they are filled.
    buffer_infos.reserve(blocks.size() + 1);
    image_infos.reserve(samplers.size());

    // Adds one write of `type` at `binding` and returns it for its info pointer.
    auto write = [&](int binding, VkDescriptorType type) -> VkWriteDescriptorSet& {
        VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        w.dstSet = set;
        w.dstBinding = static_cast<std::uint32_t>(binding);
        w.descriptorCount = 1;
        w.descriptorType = type;
        writes.push_back(w);
        return writes.back();
    };

    // The default uniforms, dynamic or at their ring offset.
    if (c.default_binding >= 0) {
        buffer_infos.push_back(
            {p.uniforms_buffer,
             p.dynamic_uniforms ? 0 : p.uniforms_offset,
             static_cast<VkDeviceSize>(c.default_size)}
        );
        write(
            c.default_binding,
            p.dynamic_uniforms ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                               : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        )
            .pBufferInfo = &buffer_infos.back();
    }

    // The uniform blocks.
    for (std::size_t i = 0; i < c.blocks.size(); ++i) {
        buffer_infos.push_back({blocks[i].buffer, blocks[i].offset, blocks[i].size});
        write(c.blocks[i].binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).pBufferInfo =
            &buffer_infos.back();
    }

    // The storage blocks, after the uniform blocks in `blocks`.
    for (std::size_t i = 0; i < c.storage.size(); ++i) {
        const auto& b = blocks[c.blocks.size() + i];
        buffer_infos.push_back({b.buffer, b.offset, b.size});
        write(c.storage[i].binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).pBufferInfo =
            &buffer_infos.back();
    }

    std::size_t at = 0;

    // Each sampler uniform with a binding, in the order `samplers` holds them.
    for (const ProgramUniform& u : c.uniforms) {
        // Not a sampler, or one the compiler gave no binding.
        if (!u.sampler || u.binding < 0) {
            continue;
        }

        const Sampled& s = samplers[at++];

        // A buffer texture is a texel view; the rest an image and its sampler.
        if (u.kind == SamplerKind::Buffer) {
            write(u.binding, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER).pTexelBufferView = &s.texels;
        } else {
            image_infos.push_back({s.sampler, s.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
            write(u.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER).pImageInfo =
                &image_infos.back();
        }
    }

    vkUpdateDescriptorSets(
        g->device, static_cast<std::uint32_t>(writes.size()), writes.data(), 0, nullptr
    );
    g->submission.descriptor_writes += writes.size();
}

// The reference path of descriptors(), as it was before R-04: its key, blocks, writes and infos
// are vectors of its own, and Program::descriptor_sets is keyed by a copy of the key.
VkDescriptorSet descriptors_reference(Program& p, std::span<const Sampled> samplers) {
    if (p.set_layout == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    const CompiledProgram& c = p.compiled;
    begin_program_sets(p);
    std::vector<BufferRef> blocks;
    blocks.reserve(c.blocks.size());
    for (std::size_t i = 0; i < c.blocks.size(); ++i) {
        blocks.push_back(block_buffer(p.block_bindings[i], c.blocks[i].size));
    }
    for (const auto& block : c.storage) {
        auto* b = block.binding >= 0 && block.binding < kUniformBindings
                      ? g->buffers.find(g->state.storage_bindings[block.binding])
                      : nullptr;
        blocks.push_back(b ? resolve(*b) : dummy_buffer_ref());
    }
    std::vector<std::uint64_t> key;
    key.reserve(4 + blocks.size() * 3 + samplers.size() * 3);
    key.push_back(reinterpret_cast<std::uint64_t>(p.uniforms_buffer));
    key.push_back(p.dynamic_uniforms ? 0 : p.uniforms_offset);
    for (const BufferRef& b : blocks) {
        key.push_back(reinterpret_cast<std::uint64_t>(b.buffer));
        key.push_back(b.offset);
        key.push_back(b.size);
    }
    for (const Sampled& s : samplers) {
        key.push_back(reinterpret_cast<std::uint64_t>(s.view));
        key.push_back(reinterpret_cast<std::uint64_t>(s.sampler));
        key.push_back(reinterpret_cast<std::uint64_t>(s.texels));
    }
    if (p.last_set != VK_NULL_HANDLE && p.last_set_frame == g->frame_count && key == p.last_key) {
        ++g->submission.descriptor_reuses;
        return p.last_set;
    }
    if (reuse_uniforms()) {
        const auto found = p.descriptor_sets.find(key);
        if (found != p.descriptor_sets.end()) {
            p.last_key = std::move(key);
            p.last_set = found->second;
            ++g->submission.descriptor_reuses;
            return p.last_set;
        }
    }
    const VkDescriptorSet set = allocate_set(p.set_layout);
    if (set == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorImageInfo> image_infos;
    buffer_infos.reserve(blocks.size() + 1);
    image_infos.reserve(samplers.size());
    auto write = [&](int binding, VkDescriptorType type) -> VkWriteDescriptorSet& {
        VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        w.dstSet = set;
        w.dstBinding = static_cast<std::uint32_t>(binding);
        w.descriptorCount = 1;
        w.descriptorType = type;
        writes.push_back(w);
        return writes.back();
    };
    if (c.default_binding >= 0) {
        buffer_infos.push_back(
            {p.uniforms_buffer,
             p.dynamic_uniforms ? 0 : p.uniforms_offset,
             static_cast<VkDeviceSize>(c.default_size)}
        );
        write(
            c.default_binding,
            p.dynamic_uniforms ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                               : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        )
            .pBufferInfo = &buffer_infos.back();
    }
    for (std::size_t i = 0; i < c.blocks.size(); ++i) {
        buffer_infos.push_back({blocks[i].buffer, blocks[i].offset, blocks[i].size});
        write(c.blocks[i].binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).pBufferInfo =
            &buffer_infos.back();
    }
    for (std::size_t i = 0; i < c.storage.size(); ++i) {
        const auto& b = blocks[c.blocks.size() + i];
        buffer_infos.push_back({b.buffer, b.offset, b.size});
        write(c.storage[i].binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).pBufferInfo =
            &buffer_infos.back();
    }
    std::size_t at = 0;
    for (const ProgramUniform& u : c.uniforms) {
        if (!u.sampler || u.binding < 0) {
            continue;
        }
        const Sampled& s = samplers[at++];
        if (u.kind == SamplerKind::Buffer) {
            write(u.binding, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER).pTexelBufferView = &s.texels;
        } else {
            image_infos.push_back({s.sampler, s.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
            write(u.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER).pImageInfo =
                &image_infos.back();
        }
    }
    vkUpdateDescriptorSets(
        g->device, static_cast<std::uint32_t>(writes.size()), writes.data(), 0, nullptr
    );
    g->submission.descriptor_writes += writes.size();
    ++g->submission.descriptor_sets;
    if (reuse_uniforms() && p.descriptor_sets.size() < 256) {
        p.descriptor_sets.emplace(key, set);
    }
    p.last_key = std::move(key);
    p.last_set = set;
    p.last_set_frame = g->frame_count;
    return set;
}

// ---- Descriptor sets kept across frames (R-14) ----

// Returns whether `buffer` lives as long as the backend: a frame's ring chunk, the dummy buffer,
// or none. A set naming only such buffers can be kept; a GL buffer's may be deleted and its
// handle given out again.
bool long_lived_buffer(VkBuffer buffer) {
    // None, or the zeros every unbound input reads.
    if (buffer == VK_NULL_HANDLE || buffer == g->dummy_buffer) {
        return true;
    }

    // Each frame's ring chunks, which go only when the backend closes.
    for (const Frame& f : g->frames) {
        // A chunk of this frame's ring.
        for (const Chunk& c : f.chunks) {
            // This one.
            if (c.buffer == buffer) {
                return true;
            }
        }
    }

    return false;
}

// Returns whether the texel view `view` lives as long as the backend: none, or a dummy view.
bool long_lived_texels(VkBufferView view) {
    return view == VK_NULL_HANDLE
           || std::ranges::find(g->dummy_views, view) != std::end(g->dummy_views);
}

// Fills `out` with the key a set of program `p` binding `blocks` and `samplers` is kept under:
// the program's serial, the set's `key`, then each sampled image's serial. False when the set
// names a resource that may die and give its handle to another (a GL buffer, a texture buffer):
// such a set is made each frame, as before.
bool persistent_key(
    const Program& p,
    std::span<const BufferRef> blocks,
    std::span<const Sampled> samplers,
    std::span<const std::uint64_t> key,
    std::vector<std::uint64_t>& out
) {
    // The default uniforms come from the ring, but a program may bind none.
    if (!long_lived_buffer(p.uniforms_buffer)) {
        return false;
    }

    // Each block's buffer.
    for (const BufferRef& block : blocks) {
        // A GL buffer: not kept.
        if (!long_lived_buffer(block.buffer)) {
            return false;
        }
    }

    // Each sampler's texel view and image.
    for (const Sampled& s : samplers) {
        // A texture buffer's view, or an image destroy_image_now emptied.
        if (!long_lived_texels(s.texels) || (s.image != nullptr && s.image->serial == 0)) {
            return false;
        }
    }

    out.clear();
    out.push_back(p.serial);
    out.insert(out.end(), key.begin(), key.end());

    // The images by their serials, which no later image takes.
    for (const Sampled& s : samplers) {
        out.push_back(s.image != nullptr ? s.image->serial : 0);
    }

    return true;
}

// Makes a pool for kept sets, of PersistentSets::kPoolSets sets that can be freed one by one.
VkDescriptorPool new_persistent_pool() {
    constexpr std::uint32_t kSets = PersistentSets::kPoolSets;
    // Each set binds a few of each kind at most (assumed: the port's programs bind up to a
    // handful of blocks and samplers), so twice the sets of each.
    const VkDescriptorPoolSize sizes[] =
        {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * kSets},
         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, kSets},
         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 * kSets},
         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, kSets},
         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, kSets}};
    VkDescriptorPoolCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    info.maxSets = kSets;
    info.poolSizeCount = static_cast<std::uint32_t>(std::size(sizes));
    info.pPoolSizes = sizes;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    check(vkCreateDescriptorPool(g->device, &info, nullptr, &pool), "a descriptor pool");
    return pool;
}

// Allocates a set of `layout` from the kept sets' pools, making a pool when all are full; null
// with `pool` untouched when none can be made. `pool` is set to the pool it came from.
VkDescriptorSet allocate_persistent(VkDescriptorSetLayout layout, VkDescriptorPool& pool) {
    PersistentSets& kept = g->persistent_sets;
    VkDescriptorSetAllocateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;

    // The newest pool first, then a new one: older pools have room only where sets were freed.
    for (int attempt = 0; attempt < 2; ++attempt) {
        // No pool, or the newest is full: one more.
        if (attempt == 1 || kept.pools.empty()) {
            const VkDescriptorPool fresh = new_persistent_pool();

            // The device cannot make another.
            if (fresh == VK_NULL_HANDLE) {
                return VK_NULL_HANDLE;
            }

            kept.pools.push_back(fresh);
        }

        info.descriptorPool = kept.pools.back();
        VkDescriptorSet set = VK_NULL_HANDLE;

        // Allocated.
        if (vkAllocateDescriptorSets(g->device, &info, &set) == VK_SUCCESS) {
            ++g->submission.descriptor_sets_allocated;
            pool = info.descriptorPool;
            return set;
        }
    }

    return VK_NULL_HANDLE;
}

// Frees the kept set bound least recently, if no frame in flight may still use it. False when
// every set was bound too recently: the new one is then not kept.
bool evict_persistent_set() {
    PersistentSets& kept = g->persistent_sets;
    auto oldest = kept.sets.end();

    // The set bound longest ago.
    for (auto it = kept.sets.begin(); it != kept.sets.end(); ++it) {
        // Older than the oldest so far.
        if (oldest == kept.sets.end() || it->second.used_frame < oldest->second.used_frame) {
            oldest = it;
        }
    }

    // A frame still in flight may bind it: the kFrames before this one.
    if (oldest == kept.sets.end()
        || oldest->second.used_frame + static_cast<std::uint64_t>(kFrames) >= g->frame_count) {
        return false;
    }

    vkFreeDescriptorSets(g->device, oldest->second.pool, 1, &oldest->second.set);
    kept.sets.erase(oldest);
    return true;
}

// Returns the kept set of program `p` for `blocks` and `samplers`, whose per-frame key is `key`,
// making and writing it when there is none; null when the set may not be kept (persistent_key)
// or cannot be made, and the caller makes one for this frame.
VkDescriptorSet persistent_set(
    Program& p,
    std::span<const BufferRef> blocks,
    std::span<const Sampled> samplers,
    std::span<const std::uint64_t> key
) {
    PersistentSets& kept = g->persistent_sets;

    // A set of resources that may die is not kept.
    if (!persistent_key(p, blocks, samplers, key, kept.key)) {
        return VK_NULL_HANDLE;
    }

    const std::string_view bytes(
        reinterpret_cast<const char*>(kept.key.data()), kept.key.size() * sizeof(std::uint64_t)
    );

    // Written in an earlier frame: bound again, unwritten.
    if (const auto found = kept.sets.find(bytes); found != kept.sets.end()) {
        found->second.used_frame = g->frame_count;
        ++g->submission.descriptor_reuses;
        return found->second.set;
    }

    // Full: the least recently bound goes, unless every set was bound too lately.
    if (kept.sets.size() >= PersistentSets::kCapacity && !evict_persistent_set()) {
        return VK_NULL_HANDLE;
    }

    VkDescriptorPool pool = VK_NULL_HANDLE;
    const VkDescriptorSet set = allocate_persistent(p.set_layout, pool);

    // No pool could be made.
    if (set == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }

    write_set(p, set, blocks, samplers, g->scratch);
    ++g->submission.descriptor_sets;
    kept.sets.emplace(std::string(bytes), PersistentSet{set, pool, g->frame_count});
    return set;
}

// descriptors() with vk-draw-cache: the reference path's sets, found and made with
// Context::scratch's arrays and Program::set_cache, so a set found allocates nothing.
VkDescriptorSet descriptors_cached(Program& p, std::span<const Sampled> samplers) {
    begin_program_sets(p);
    DrawScratch& scratch = g->scratch;
    descriptor_key(p, samplers, scratch.blocks, scratch.key);
    const std::span<const std::uint64_t> key = scratch.key;

    // The draw before bound the same resources this frame: its set again.
    if (p.last_set != VK_NULL_HANDLE && p.last_set_frame == g->frame_count
        && std::ranges::equal(key, p.last_key)) {
        ++g->submission.descriptor_reuses;
        return p.last_set;
    }

    const std::uint64_t hash = hash_words(key);

    // A set made earlier this frame for these resources.
    if (reuse_uniforms()) {
        // Found: it becomes the last set.
        if (const VkDescriptorSet found = p.set_cache.find(key, hash); found != VK_NULL_HANDLE) {
            p.last_key.assign(key.begin(), key.end());
            p.last_set = found;
            ++g->submission.descriptor_reuses;
            return p.last_set;
        }
    }

    // A set kept from an earlier frame, or kept from now on (vk-persistent-descriptors).
    VkDescriptorSet set = VK_NULL_HANDLE;

    // Kept sets need the reuse as well: without it every set is made anew, the reference.
    if (reuse_uniforms()
        && renderer::render_opt::enabled(renderer::render_opt::Opt::VkPersistentDescriptors)) {
        set = persistent_set(p, scratch.blocks, samplers, key);
    }

    // Not kept: made for this frame, as before.
    if (set == VK_NULL_HANDLE) {
        set = allocate_set(p.set_layout);

        // The pools are exhausted and a new one could not be made.
        if (set == VK_NULL_HANDLE) {
            return VK_NULL_HANDLE;
        }

        write_set(p, set, scratch.blocks, samplers, scratch);
        ++g->submission.descriptor_sets;
    }

    // Kept for the rest of the frame, unless the reuse is the reference that is switched off.
    if (reuse_uniforms()) {
        p.set_cache.insert(key, hash, set);
    }

    p.last_key.assign(key.begin(), key.end());
    p.last_set = set;
    p.last_set_frame = g->frame_count;
    return set;
}

}  // namespace

VkDescriptorSet descriptors(Program& p, std::span<const Sampled> samplers) {
    // A program with no resources has no set.
    if (p.set_layout == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }

    // The allocation-free path, or the reference.
    if (renderer::render_opt::enabled(renderer::render_opt::Opt::VkDrawCache)) {
        return descriptors_cached(p, samplers);
    }

    return descriptors_reference(p, samplers);
}

void OPENRAC_GLAPI dispatch_compute(GLuint x, GLuint y, GLuint z) {
    auto* p = g->programs.find(g->state.program);
    if (!p || !p->linked || !p->compute_pipeline || !x || !y || !z) {
        return;
    }
    end_rendering();
    std::vector<Sampled> samplers;
    for (std::size_t i = 0; i < p->compiled.uniforms.size(); ++i) {
        const auto& u = p->compiled.uniforms[i];
        if (!u.sampler || u.binding < 0) {
            continue;
        }
        auto sm = sampled(*p, i);
        if (sm.image) {
            if (sm.image->layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier(frame().main, *sm.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            sm.image->touched = g->recording;
        }
        samplers.push_back(sm);
    }
    const auto set = descriptors(*p, samplers);
    vkCmdBindPipeline(frame().main, VK_PIPELINE_BIND_POINT_COMPUTE, p->compute_pipeline);
    count_command(Command::BindPipeline);
    const auto dynamic_offset = static_cast<std::uint32_t>(p->uniforms_offset);
    vkCmdBindDescriptorSets(
        frame().main,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        p->layout,
        0,
        1,
        &set,
        p->dynamic_uniforms ? 1u : 0u,
        p->dynamic_uniforms ? &dynamic_offset : nullptr
    );
    count_command(Command::BindDescriptors);
    vkCmdDispatch(frame().main, x, y, z);
    count_command(Command::Dispatch);
    g->bound_pipeline = VK_NULL_HANDLE;
}

void OPENRAC_GLAPI memory_barrier(GLbitfield) {
    end_rendering();
    // Shader writes (compute's, the only shaders that write) before every later use of what they
    // wrote: shader and uniform reads, vertices and indices, indirect arguments, copies (the R-08
    // table). The reference path: every stage.
    const bool tight = renderer::render_opt::enabled(renderer::render_opt::Opt::VkTightSync);
    const VkPipelineStageFlags shaders = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
                                         | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                                         | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    const VkPipelineStageFlags source = tight ? shaders : VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    const VkPipelineStageFlags destination = tight ? shaders | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
                                                         | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
                                                         | VK_PIPELINE_STAGE_TRANSFER_BIT
                                                   : VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkMemoryBarrier b{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    b.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
    b.dstAccessMask =
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_UNIFORM_READ_BIT;

    // The narrow form also names the reads the reference path's every-stage scope covered.
    if (tight) {
        b.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        b.dstAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
                           | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    }

    vkCmdPipelineBarrier(frame().main, source, destination, 0, 1, &b, 0, nullptr, 0, nullptr);
    count_barrier(source, destination);
}

}  // namespace openrac::vkgl::detail

namespace openrac::vkgl {

void begin_pipeline_warmup(std::uint32_t frames) {
    // Nothing to warm up without a context.
    if (detail::g == nullptr) {
        return;
    }

    detail::g->warmup_end = detail::g->submission.frames_presented + frames;
}

std::uint64_t wait_for_pipeline_prewarm() {
    // Closed: no worker.
    if (detail::g == nullptr) {
        return 0;
    }

    detail::PipelinePrewarm& w = detail::g->prewarm;
    std::unique_lock lock(w.mutex);
    w.changed.wait(lock, [&w] { return w.queue.empty() && !w.making.has_value(); });
    return w.made;
}

}  // namespace openrac::vkgl

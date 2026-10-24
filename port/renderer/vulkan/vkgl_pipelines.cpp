// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Pipelines and descriptor sets (vkgl_internal.h): GL's vertex formats and fixed-function state as
// Vulkan pipelines, made once for each key and kept in the pipeline cache; the descriptor sets that
// bind a program's textures and blocks; glDispatchCompute and glMemoryBarrier; and the warm-up
// after which a pipeline made counts as a hitch (vkgl.h, begin_pipeline_warmup).

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <volk.h>

#include "renderer/gl.h"
#include "renderer/render_opt.h"
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

std::uint64_t hash_key(const PipelineKey& key) {
    constexpr std::size_t kWords = sizeof(PipelineKey) / sizeof(std::uint64_t);
    std::array<std::uint64_t, kWords> words{};
    std::memcpy(words.data(), &key, sizeof key);
    std::uint64_t hash = kFnvBasis;

    // FNV-1a a word at a time rather than a byte: the key is about 380 bytes, read once a draw.
    for (const std::uint64_t word : words) {
        hash = (hash ^ word) * kFnvPrime;
    }

    hash = (hash ^ (hash >> kMixShiftFirst)) * kMixFirst;
    hash = (hash ^ (hash >> kMixShiftSecond)) * kMixSecond;
    return hash ^ (hash >> kMixShiftLast);
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
    k.cull = !s.cull                            ? VK_CULL_MODE_NONE
             : s.cull_face == GL_FRONT          ? VK_CULL_MODE_FRONT_BIT
             : s.cull_face == GL_FRONT_AND_BACK ? VK_CULL_MODE_FRONT_AND_BACK
                                                : VK_CULL_MODE_BACK_BIT;
    // The same rows as OpenGL's, counted from the other side: what GL calls counter-clockwise,
    // Vulkan's sign of the area calls clockwise.
    k.front = s.front_face == GL_CCW ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
        // GL writes depth only while it tests it.
        k.depth_test = s.depth_test ? 1 : 0;
        k.depth_write = s.depth_test && s.depth_mask ? 1 : 0;
        k.depth_func = s.depth_test ? compare_op(s.depth_func) : 0;
        if (s.stencil_test && has_stencil(t.depth->format)) {
            k.stencil = 1;
            k.stencil_func = compare_op(s.stencil_func);
            k.front_ops[0] = stencil_op(s.stencil_front.sfail);
            k.front_ops[1] = stencil_op(s.stencil_front.dppass);
            k.front_ops[2] = stencil_op(s.stencil_front.dpfail);
            k.back_ops[0] = stencil_op(s.stencil_back.sfail);
            k.back_ops[1] = stencil_op(s.stencil_back.dppass);
            k.back_ops[2] = stencil_op(s.stencil_back.dpfail);
        }
    }
    k.samples = t.samples;
    k.inputs = count;
    k.viewports = static_cast<std::uint32_t>(std::max<std::size_t>(1, g->indexed_viewports.size()));
    std::memcpy(k.input, inputs, sizeof(Input) * count);
    const std::uint64_t hash = hash_key(k);

    // A key made before: by its hash and a compare of the whole key (vk-draw-cache), or by its
    // bytes as a string, which allocates for every draw (the reference path).
    if (renderer::render_opt::enabled(renderer::render_opt::Opt::VkDrawCache)) {
        // Found by its hash.
        if (const VkPipeline* made = g->pipelines.find(k, hash)) {
            return *made;
        }
    } else if (const VkPipeline* made = g->pipelines.find_reference(
                   std::string(reinterpret_cast<const char*>(&k), sizeof k)
               )) {
        // Found by its bytes.
        return *made;
    }

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0] =
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         nullptr,
         0,
         VK_SHADER_STAGE_VERTEX_BIT,
         p.vertex,
         "main",
         nullptr};
    stages[1] =
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         nullptr,
         0,
         VK_SHADER_STAGE_FRAGMENT_BIT,
         p.fragment,
         "main",
         nullptr};
    VkVertexInputBindingDescription bindings[kAttributes]{};
    VkVertexInputAttributeDescription attributes[kAttributes]{};
    for (std::uint32_t i = 0; i < count; ++i) {
        bindings[i] = {
            i,
            inputs[i].stride,
            inputs[i].instanced != 0 ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX
        };
        attributes[i] = {inputs[i].location, i, static_cast<VkFormat>(inputs[i].format), 0};
    }
    VkPipelineVertexInputStateCreateInfo vertex{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    vertex.vertexBindingDescriptionCount = count;
    vertex.pVertexBindingDescriptions = bindings;
    vertex.vertexAttributeDescriptionCount = count;
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
    raster.cullMode = k.cull;
    raster.frontFace = static_cast<VkFrontFace>(k.front);
    raster.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo multisample{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
    };
    multisample.rasterizationSamples = t.samples;
    VkPipelineDepthStencilStateCreateInfo depth{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
    };
    depth.depthTestEnable = k.depth_test;
    depth.depthWriteEnable = k.depth_write;
    depth.depthCompareOp =
        k.depth_test != 0 ? static_cast<VkCompareOp>(k.depth_func) : VK_COMPARE_OP_ALWAYS;
    depth.stencilTestEnable = k.stencil;
    depth.front =
        {static_cast<VkStencilOp>(k.front_ops[0]),
         static_cast<VkStencilOp>(k.front_ops[1]),
         static_cast<VkStencilOp>(k.front_ops[2]),
         static_cast<VkCompareOp>(k.stencil_func),
         0,
         0,
         0};
    depth.back =
        {static_cast<VkStencilOp>(k.back_ops[0]),
         static_cast<VkStencilOp>(k.back_ops[1]),
         static_cast<VkStencilOp>(k.back_ops[2]),
         static_cast<VkCompareOp>(k.stencil_func),
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
    blending.attachmentCount = t.colour != nullptr ? 1 : 0;
    blending.pAttachments = &blend;
    const VkDynamicState dynamic_states[] =
        {VK_DYNAMIC_STATE_VIEWPORT,
         VK_DYNAMIC_STATE_SCISSOR,
         VK_DYNAMIC_STATE_BLEND_CONSTANTS,
         VK_DYNAMIC_STATE_STENCIL_REFERENCE,
         VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
         VK_DYNAMIC_STATE_STENCIL_WRITE_MASK};
    VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic.dynamicStateCount = static_cast<std::uint32_t>(std::size(dynamic_states));
    dynamic.pDynamicStates = dynamic_states;
    const VkFormat colour_format = static_cast<VkFormat>(k.colour_format);
    VkPipelineRenderingCreateInfo rendering{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    rendering.colorAttachmentCount = t.colour != nullptr ? 1 : 0;
    rendering.pColorAttachmentFormats = &colour_format;
    rendering.depthAttachmentFormat = static_cast<VkFormat>(k.depth_format);
    rendering.stencilAttachmentFormat =
        t.depth != nullptr && has_stencil(t.depth->format) ? t.depth->format : VK_FORMAT_UNDEFINED;
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
    info.layout = p.layout;
    VkPipeline pipeline = VK_NULL_HANDLE;
    ++g->measures.pipelines;
    count_pipeline();
    if (!check(
            vkCreateGraphicsPipelines(g->device, g->pipeline_cache, 1, &info, nullptr, &pipeline),
            "a pipeline"
        )) {
        pipeline = VK_NULL_HANDLE;
    }
    g->pipelines.insert(k, hash, pipeline);
    return pipeline;
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

VkDescriptorSet descriptors(Program& p, const std::vector<Sampled>& samplers) {
    if (p.set_layout == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    const CompiledProgram& c = p.compiled;
    if (p.last_set_frame != g->frame_count) {
        // Pools and ring allocations retire together after the frame-slot fence.
        p.descriptor_sets.clear();
        p.last_set = VK_NULL_HANDLE;
    }
    if (c.default_binding >= 0 && (p.uniforms_dirty || p.uniforms_frame != g->frame_count)) {
        const Slice s = ring_copy(p.uniforms.data(), static_cast<std::size_t>(c.default_size));
        p.uniforms_buffer = s.buffer;
        p.uniforms_offset = s.offset;
        p.uniforms_frame = g->frame_count;
        p.uniforms_dirty = false;
        ++g->submission.uniform_uploads;
        g->submission.uniform_upload_bytes += static_cast<std::size_t>(c.default_size);
    }
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
    VkMemoryBarrier b{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    b.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
    b.dstAccessMask =
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_UNIFORM_READ_BIT;
    vkCmdPipelineBarrier(
        frame().main,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        1,
        &b,
        0,
        nullptr,
        0,
        nullptr
    );
    count_barrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
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

}  // namespace openrac::vkgl

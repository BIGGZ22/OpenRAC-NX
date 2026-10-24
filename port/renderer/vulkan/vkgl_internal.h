// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The Vulkan backend's shared declarations, for its own files only (CODING_CONVENTIONS.md 9.1):
// the constants, the GL objects and the per-frame state the vkgl_*.cpp files share, the one Context
// `g` they all work on, and the functions each file offers the others, grouped by the file that
// defines them. vkgl.h says how GL's model becomes Vulkan's; vkgl.cpp says how a frame is recorded.
//
// The Vulkan Memory Allocator is included here, after the two settings it is built with in vma.cpp
// (Vulkan's functions come from volk), so a file that calls it includes this header, not VMA's.

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <volk.h>

#include <SDL3/SDL.h>

#include "common/log.h"
#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/gpu_draw.h"
#include "renderer/vulkan/shader_compiler.h"
#include "renderer/vulkan/vkgl.h"
#include "renderer/world/gl_world.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

namespace openrac::vkgl::detail {

// The names of renderer/gl.h this header uses (a header has no `using namespace`).
using openrac::gl::GL_ALWAYS;
using openrac::gl::GL_BACK;
using openrac::gl::GL_CCW;
using openrac::gl::GL_FILL;
using openrac::gl::GL_FLOAT;
using openrac::gl::GL_FUNC_ADD;
using openrac::gl::GL_LESS;
using openrac::gl::GL_LINEAR;
using openrac::gl::GL_NEAREST;
using openrac::gl::GL_NO_ERROR;
using openrac::gl::GL_ONE;
using openrac::gl::GL_REPEAT;
using openrac::gl::GL_ZERO;
using openrac::gl::GLbitfield;
using openrac::gl::GLenum;
using openrac::gl::GLint;
using openrac::gl::GLsizei;
using openrac::gl::GLuint;

namespace wgl = openrac::renderer::world::gl;

// The frames in flight; the slots of the per-frame state that is reused when its fence signals.
// kUnits, kAttributes and kUniformBindings are the GL limits the backend reports and keeps
// state for; kChunk is the size of one chunk of a frame's ring.
inline constexpr int kFrames = 2;
inline constexpr int kUnits = 32;
inline constexpr int kAttributes = 16;
inline constexpr int kUniformBindings = 16;
inline constexpr VkDeviceSize kChunk = 8u << 20;

// GL enums the headers do not list, which the functions below are asked about.
inline constexpr GLenum kGlActiveUniforms = 0x8B86;
inline constexpr GLenum kGlInvalidEnum = 0x0500;
inline constexpr GLenum kGlUniformBufferOffsetAlignment = 0x8A34;
inline constexpr GLenum kGlMaxCombinedTextureUnits = 0x8B4D;
inline constexpr GLenum kGlMaxTextureBufferSize = 0x8C2B;
inline constexpr GLenum kGlMaxVertexAttribs = 0x8869;
inline constexpr GLenum kGlMaxUniformBlockSize = 0x8A30;
inline constexpr GLenum kGlTextureBindingSlot = 0x8069;  // GL_TEXTURE_BINDING_2D
inline constexpr GLenum kGlFramebufferBinding = 0x8CA6;
inline constexpr GLenum kGlViewportQuery = 0x0BA2;
inline constexpr GLenum kGlHalfFloat = 0x140B;
inline constexpr GLenum kGlByte = 0x1400;
inline constexpr GLenum kGlDepthStencil = 0x84F9;
inline constexpr GLenum kGlRed = 0x1903;
inline constexpr GLenum kGlR8 = 0x8229;
inline constexpr GLenum kGlRgba16f = 0x881A;
inline constexpr GLenum kGlR11fG11fB10f = 0x8C3A;
inline constexpr GLenum kGlRg32f = 0x8230;
inline constexpr GLenum kGlUnpackRowLength = 0x0CF2;
inline constexpr GLenum kGlNearestMipmapLinear = 0x2702;
inline constexpr GLenum kGlMirroredRepeat = 0x8370;
inline constexpr GLenum kGlIntSamplerBuffer = 0x8DD0;
inline constexpr GLenum kGlUnsignedSamplerBuffer = 0x8DD8;
inline constexpr GLenum kGlStaticRead = 0x88E5;
inline constexpr GLenum kGlStaticCopy = 0x88E6;

// ---- The device ----

// A buffer and its memory, mapped when the host writes it: a piece of a frame's ring, or a page
// of GPU-driven or material data. Plain data: whoever made it destroys it.
struct Chunk {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    std::uint8_t* mapped = nullptr;
    VkDeviceSize size = 0;
};

// Bounded, fence-owned pages. Growing a frame adds a page rather than moving
// buffers already referenced by recorded draws. Reuse only after slot retirement.
inline constexpr unsigned kGpuPageInstances = 32768, kGpuPageGroups = 4096, kGpuPageDraws = 32768;
inline constexpr unsigned kGpuInputBytes = 8u << 20;

// The native material path's pages: the draws one page holds, and the words and bytes of one
// material record.
inline constexpr unsigned kMaterialDraws = 32768;
inline constexpr unsigned kMaterialWords = 20;
inline constexpr unsigned kMaterialBytes = kMaterialWords * sizeof(std::uint32_t);

// A hash of a fixed-size array (FNV-1a over its elements), for the material pages' maps.
template <class T, std::size_t N>
struct ArrayHash {
    std::size_t operator()(const std::array<T, N>& key) const {
        std::size_t hash = 1469598103934665603ull;
        for (auto value : key) {
            hash = (hash ^ std::size_t(value)) * 1099511628211ull;
        }
        return hash;
    }
};

// One page of the native material path: the material records and texture slots of up to
// kMaterialDraws draws, their buffer and the descriptor set that binds them.
struct MaterialPage {
    Chunk data;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet set = VK_NULL_HANDLE;
    std::unordered_map<std::array<std::uint64_t, 2>, unsigned, ArrayHash<std::uint64_t, 2>>
        textures;
    std::unordered_map<
        std::array<std::uint32_t, kMaterialWords>,
        unsigned,
        ArrayHash<std::uint32_t, kMaterialWords>>
        materials;
    unsigned draws = 0;
};

// One page of GPU-driven instances in a frame: the culling input and output, the indirect
// arguments and their readback, and the groups and draw commands recorded into it.
struct GpuPage {
    Chunk input, output, arguments, readback;
    std::vector<std::array<std::uint32_t, 4>> groups, commands;
    std::vector<std::uint32_t> payload;
    unsigned instances = 0;
    VkDescriptorSet descriptors = VK_NULL_HANDLE;
};

// A frame slot's GPU-driven pages, how many this recording used, and its counters' state.
struct GpuFrame {
    std::vector<GpuPage> pages;
    unsigned used = 0;
    bool counters = false;
    std::uint64_t recording = 0, reported = 0;
    // The resident instance buffer this recording's culling reads (GpuResident), null until a
    // batch of it is made; a later upload of another one waits for the next recording.
    VkBuffer resident = VK_NULL_HANDLE;
};

// The static instances registered once (gpu_register_instances, gpu-resident-instances in
// docs/port/RENDER_PERF.md): each instance's bounds and data in the culling's own layout, kept in
// device memory, so that a view's batch of them carries only its planes, PVS and a group record.
struct GpuResident {
    // One registered GL instance buffer: where its instances are in `words`.
    struct Group {
        std::uint32_t first = 0;    // Its first instance's first word in `words`.
        std::uint32_t count = 0;    // Its instances.
        std::uint64_t version = 0;  // The buffer's version when registered: another is stale.
    };

    std::unordered_map<unsigned, Group> groups;  // By GL buffer name.
    std::vector<std::uint32_t> words;  // Per instance, 8 words of bounds then 30 of its data.
    Chunk buffer;                      // `words` in device memory; empty until a batch needs it.
    bool dirty = false;                // `words` changed since `buffer` was made.
};

// The GPU-driven engine (gpu_draw.inc): the culling pipelines, the indirect draws merged while
// they share their bindings, and the counters.
struct GpuEngine {
    GpuDrawCounters completed;
    std::uint64_t counter_floor = 0;

    struct Bindings {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkDescriptorSet descriptors = VK_NULL_HANDLE;
        VkDescriptorSet materials = VK_NULL_HANDLE;
        std::uint32_t dynamic_offset = 0, inputs = 0;
        std::array<VkBuffer, kAttributes> vertices{};
        std::array<VkDeviceSize, kAttributes> offsets{};
        VkBuffer index = VK_NULL_HANDLE;
        VkDeviceSize index_offset = 0;
        VkIndexType index_type = VK_INDEX_TYPE_UINT32;
        std::array<int, 4> viewport{}, scissor{};
        std::array<float, 4> blend{};
        std::array<std::uint32_t, 3> stencil{};
        bool operator==(const Bindings&) const = default;
    };

    Bindings pending_bindings;
    VkBuffer pending_arguments = VK_NULL_HANDLE;
    VkDeviceSize pending_offset = 0;
    unsigned pending_count = 0, pending_material_base = 0;
    VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline compact = VK_NULL_HANDLE, arguments = VK_NULL_HANDLE, summary = VK_NULL_HANDLE;
    bool failed = false, timing_upload = false;
    std::uint64_t batches = 0, candidates = 0, indirect = 0, indirect_calls = 0, merged = 0;
    std::array<std::uint64_t, 10> batch_breaks{};
    GpuResident resident;  // The registered static instances, until the level releases them.
};

// The state of one frame slot: its command pool and two command buffers, its fence and
// semaphore, timestamp queries, descriptor pools, the ring's chunks, the deferred work, and its
// GPU-driven and material pages. Reused when its fence has signalled.
struct Frame {
    GpuFrame gpu;
    std::vector<MaterialPage> material_pages;
    unsigned material_pages_used = 0;

    struct Timing {
        std::uint64_t serial;
        int section;
        std::uint32_t first;
        bool complete = false, reported = false;
    };

    VkQueryPool timestamps = VK_NULL_HANDLE;
    std::uint32_t timestamp_used = 0;
    int active_timing = -1;
    std::vector<Timing> timings;
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandBuffer upload = VK_NULL_HANDLE;
    VkCommandBuffer main = VK_NULL_HANDLE;
    VkCommandBuffer present = VK_NULL_HANDLE;  // The copy to the screen's (present_cmd), if made.
    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore acquired = VK_NULL_HANDLE;
    std::vector<VkDescriptorPool> pools;
    std::size_t pool_at = 0;
    std::vector<Chunk> chunks;
    std::size_t chunk_at = 0;
    VkDeviceSize chunk_used = 0;
    std::vector<std::function<void()>> deferred;
    bool submitted = false;  // the fence will be signalled
    // Secondary command buffers the passes of attachment-discard are recorded into, from `pool`;
    // the first `secondary_at` are this recording's.
    std::vector<VkCommandBuffer> secondaries;
    std::size_t secondary_at = 0;
};

// A Vulkan image, its memory, views and the layout it was last left in. Owned through ImagePtr.
struct Image {
    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    int width = 0;
    int height = 0;
    int levels = 1;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    std::uint64_t touched = 0;  // the recording that last used it in `main`
    // The aspects whose contents glInvalidateFramebuffer let go and nothing wrote since: the next
    // pass on the image does not load them (attachment-discard, docs/port/RENDER_PERF.md "R-06").
    VkImageAspectFlags discarded = 0;
    VkImageView attachment = VK_NULL_HANDLE;
    std::vector<std::pair<std::uint32_t, VkImageView>>
        views;  // (base | top << 16) -> view for sampling
};

// The owner of an image: the last one to go destroys it (make_image), or retire defers that.
using ImagePtr = std::shared_ptr<Image>;

// GL's sampler parameters for a texture or a sampler object.
struct SamplerParams {
    GLenum min = GL_NEAREST;
    GLenum mag = GL_LINEAR;
    GLenum wrap_s = GL_REPEAT;
    GLenum wrap_t = GL_REPEAT;
    float anisotropy = 1.0f;
    bool operator==(const SamplerParams&) const = default;
};

// A GL texture: its image and format, sampler parameters and levels, or a buffer texture's
// buffer and format.
struct Texture {
    GLenum target = 0;
    ImagePtr image;
    GLenum internal = 0;
    SamplerParams params{kGlNearestMipmapLinear, GL_LINEAR};  // GL's defaults
    int base_level = 0;
    int max_level = 1000;
    GLuint buffer = 0;  // a buffer texture's buffer and format
    GLenum buffer_format = 0;
};

// A GL renderbuffer: its image.
struct Renderbuffer {
    ImagePtr image;
};

// A GL buffer: the data as GL was given it, the version that changes with each write, where that
// version is on the GPU, and the texel views made of it this frame.
//
// `data` is the CPU's copy (the mirror). A static buffer that has a copy of its own in device
// memory lets it go (static-buffers-device-local, docs/port/RENDER_PERF.md "R-05"); a reader of
// its bytes calls buffer_bytes, which reads them back from `own`. The GPU-driven instances
// (gpu_make_batch) read `data` directly: their buffers are drawn per instance and never given a
// copy of their own, so their mirror stays.
struct Buffer {
    bool storage = false;  // GPU-written storage must keep one allocation across frames.
    std::vector<std::uint8_t> data;
    std::size_t size = 0;       // The bytes GL gave it, which `data` holds unless it was let go.
    bool static_hint = false;   // Filled with a GL_STATIC_* usage: drawn often, written seldom.
    bool rewritten = false;     // glBufferSubData wrote into it since its glBufferData.
    bool keep_mirror = false;   // Read as a uniform block or by the CPU: `data` is kept.
    bool mirror_freed = false;  // `data` was let go; `own` holds this version.
    std::uint8_t* staged = nullptr;      // The staging copy that fills `own`, while it is alive.
    std::uint64_t staged_recording = 0;  // The recording whose `upload` copies `staged` into `own`.
    std::uint64_t version = 1;
    std::uint64_t changed_frame = 0;
    // Where this version is on the GPU: a frame's ring (valid during that recording), or a copy
    // of its own once it stays the same.
    VkBuffer gpu = VK_NULL_HANDLE;
    VkDeviceSize gpu_offset = 0;
    std::uint64_t gpu_version = 0;
    std::uint64_t gpu_frame = 0;
    VkBuffer own = VK_NULL_HANDLE;
    VmaAllocation own_allocation = nullptr;
    std::uint64_t own_version = 0;
    std::uint8_t* own_mapped = nullptr;  // Where the host sees `own`; null in device-only memory.

    struct View {
        VkFormat format;
        VkBuffer buffer;
        VkDeviceSize offset;
        VkBufferView view;
    };

    std::vector<View> views;
    std::uint64_t views_frame = 0;
};

// One vertex attribute of a vertex array, as glVertexAttribPointer set it.
struct Attribute {
    bool enabled = false;
    GLint size = 4;
    GLenum type = GL_FLOAT;
    bool normalized = false;
    bool integer = false;
    GLsizei stride = 0;
    std::uintptr_t offset = 0;
    GLuint buffer = 0;
    GLuint divisor = 0;
};

// A GL vertex array: its attributes and its element buffer.
struct VertexArray {
    std::array<Attribute, kAttributes> attributes{};
    GLuint elements = 0;
};

// A GL shader: its type and source, and whether it compiled with what log.
struct Shader {
    GLenum type = 0;
    std::string source;
    bool compiled = false;
    std::string log;
};

// A hash of a descriptor set's key (the resources it binds), for Program::descriptor_sets.
struct DescriptorKeyHash {
    std::size_t operator()(const std::vector<std::uint64_t>& key) const {
        std::size_t hash = 0;
        for (auto value : key) {
            hash ^= std::hash<std::uint64_t>{}(value) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

// True unless OPENRAC_VK_UNIFORM_REUSE_OFF is set (read once): the profiling reference that
// uploads every uniform and makes descriptor sets anew.
inline bool reuse_uniforms() {
    static const bool& off = stats::probe(
        "OPENRAC_VK_UNIFORM_REUSE_OFF",
        "Profiling reference: upload repeated uniforms and recreate descriptors"
    );
    return !off;
}

// The descriptor sets one program made this frame, by key, for vk-draw-cache: the reference path's
// Program::descriptor_sets without its heap allocations. The keys' words sit one after another in
// one array and a fixed table of slots indexes them, so clear() keeps every array's capacity and a
// program's steady state allocates nothing. Holds at most kCapacity sets, as the reference does.
struct DescriptorCache {
    // The sets it holds at most, the reference path's limit; past it a set is made each time.
    static constexpr std::size_t kCapacity = 256;

    // Its slots: a power of two, twice kCapacity, so a probe meets a free slot soon.
    static constexpr std::size_t kSlots = 2 * kCapacity;

    // One set and where its key is in `words`.
    struct Item {
        std::uint64_t hash = 0;                // hash_words of its key.
        std::uint32_t at = 0;                  // Its key's first word in `words`.
        std::uint32_t size = 0;                // Its key's words.
        VkDescriptorSet set = VK_NULL_HANDLE;  // The set.
    };

    std::vector<std::uint64_t> words;           // Every key's words, in the order they were added.
    std::vector<Item> items;                    // Every set, in the order they were added.
    std::array<std::uint16_t, kSlots> slots{};  // 0 a free slot, n the item n - 1; linear probing.

    // Forgets every set (the frame's pools are reset with the frame), keeping the capacity.
    void clear();

    // Returns the set made for `key`, whose hash_words is `hash`, or null. The whole key decides.
    [[nodiscard]] VkDescriptorSet find(
        std::span<const std::uint64_t> key, std::uint64_t hash
    ) const;

    // Adds `set` for `key`, of hash `hash`, unless kCapacity sets are held already.
    void insert(std::span<const std::uint64_t> key, std::uint64_t hash, VkDescriptorSet set);
};

// A range of a buffer on the GPU that a draw binds.
struct BufferRef {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
};

// What a sampler uniform reads now: its image (null for a buffer texture) and the descriptor.
struct Sampled {
    Image* image = nullptr;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkBufferView texels = VK_NULL_HANDLE;
};

// The arrays a draw and descriptors() fill afresh each time, kept in the Context so that their
// capacity stays and a draw allocates nothing once they have grown (vk-draw-cache). Each user
// clears what it uses first; none is used by two functions at once.
struct DrawScratch {
    std::vector<Sampled> samplers;                     // The draw's sampled textures.
    std::vector<BufferRef> blocks;                     // descriptors(): the blocks it binds.
    std::vector<std::uint64_t> key;                    // descriptors(): the set's key.
    std::vector<VkWriteDescriptorSet> writes;          // descriptors(): a new set's writes.
    std::vector<VkDescriptorBufferInfo> buffer_infos;  // descriptors(): what the writes point at.
    std::vector<VkDescriptorImageInfo> image_infos;    // descriptors(): what the writes point at.
};

// A GL program: its shaders, the compiled SPIR-V and what it declares (shader_compiler.h), its
// shader modules and layouts, the default uniform block, and the descriptor sets made this frame.
struct Program {
    std::vector<GLuint> shaders;
    bool linked = false;
    std::string log;
    std::uint64_t serial = 0;  // a new one each link, for the pipelines' keys
    CompiledProgram compiled;
    bool gpu_materials = false;
    unsigned material_start = 0;
    VkShaderModule vertex = VK_NULL_HANDLE;
    VkShaderModule fragment = VK_NULL_HANDLE;
    VkShaderModule compute = VK_NULL_HANDLE;
    VkPipeline compute_pipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    std::vector<std::uint8_t> uniforms;  // the default block, std140
    std::vector<int> units;              // per entry of compiled.uniforms: a sampler's unit
    std::vector<GLuint> block_bindings;  // per entry of compiled.blocks: its GL binding point
    bool uniforms_dirty = true;
    VkBuffer uniforms_buffer = VK_NULL_HANDLE;
    VkDeviceSize uniforms_offset = 0;
    std::uint64_t uniforms_frame = 0;
    bool dynamic_uniforms = false;
    // At most 256 resource combinations per program, valid only this frame.
    std::unordered_map<std::vector<std::uint64_t>, VkDescriptorSet, DescriptorKeyHash>
        descriptor_sets;
    DescriptorCache set_cache;            // The same sets as descriptor_sets, for vk-draw-cache.
    std::vector<std::uint64_t> last_key;  // what the last descriptor set held
    VkDescriptorSet last_set = VK_NULL_HANDLE;
    std::uint64_t last_set_frame = 0;
};

// A GL framebuffer object: the textures or renderbuffers attached as colour and depth.
struct Framebuffer {
    GLuint colour_texture = 0;
    GLuint colour_renderbuffer = 0;
    GLuint depth_texture = 0;
    GLuint depth_renderbuffer = 0;
};

// What a pass of attachment-discard has to know when it ends, which is when its load and store
// operations are chosen (vkgl_commands.cpp): its draws go into a secondary command buffer
// meanwhile, and `main` points at it.
struct OpenPass {
    bool deferred = false;  // This pass is recorded into a secondary command buffer.
    VkCommandBuffer primary = VK_NULL_HANDLE;  // The frame's `main`, given back at the end.
    bool clear_colour = false;                 // The pass began with a clear of its colour,
    bool clear_depth = false;                  // of its depth,
    bool clear_stencil = false;                // and of its stencil.
    VkClearColorValue colour_value{};          // The clear values as they were at its begin.
    VkClearDepthStencilValue depth_value{};
    // The aspects glInvalidateFramebuffer let go since the pass's last draw or clear: not stored.
    VkImageAspectFlags drop_colour = 0;
    VkImageAspectFlags drop_depth = 0;
    // vk_cmds_recorded when the pass began: the same at its end means its buffer holds nothing.
    std::uint64_t commands_at_begin = 0;
};

// A copy into the default framebuffer that waits for present() (present-single-copy,
// docs/port/RENDER_PERF.md "R-07"): the frame's picture, blitted into a box of the default
// framebuffer whose rest was cleared to `clear`. present_to_screen makes it one blit into the
// screen; anything else that touches the default framebuffer or the picture makes it first.
struct PendingPresent {
    bool pending = false;
    ImagePtr source;       // The picture, kept until the copy is made.
    VkImageBlit region{};  // From the picture into the default framebuffer.
    VkFilter filter = VK_FILTER_NEAREST;
    VkClearColorValue clear{};  // The default framebuffer's colour around the box.
};

// What a draw or a clear draws into.
struct Target {
    Image* colour = nullptr;
    Image* depth = nullptr;
    int width = 0;
    int height = 0;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    bool operator==(const Target&) const = default;
};

// The stencil operations of one face, as glStencilOpSeparate set them.
struct StencilFace {
    GLenum sfail = wgl::GL_KEEP;
    GLenum dpfail = wgl::GL_KEEP;
    GLenum dppass = wgl::GL_KEEP;
};

// GL's state as the GL functions set it: capabilities, blending, depth, stencil, viewport and
// scissor, pixel store, and the bindings of units, buffers, program and framebuffers.
struct State {
    bool blend = false, depth_test = false, cull = false, scissor_test = false,
         stencil_test = false;
    bool primitive_restart = false;
    GLenum blend_src_rgb = GL_ONE, blend_dst_rgb = GL_ZERO, blend_src_a = GL_ONE,
           blend_dst_a = GL_ZERO;
    GLenum blend_equation = GL_FUNC_ADD;
    float blend_colour[4] = {0, 0, 0, 0};
    GLenum depth_func = GL_LESS;
    bool depth_mask = true;
    bool colour_mask[4] = {true, true, true, true};
    GLenum cull_face = GL_BACK;
    GLenum front_face = GL_CCW;
    GLenum polygon_mode = GL_FILL;
    int viewport[4] = {0, 0, 0, 0};
    int scissor[4] = {0, 0, 0, 0};
    float clear_colour[4] = {0, 0, 0, 0};
    float clear_depth = 1.0f;
    int clear_stencil = 0;
    GLenum stencil_func = GL_ALWAYS;
    int stencil_ref = 0;
    GLuint stencil_value_mask = 0xFFFFFFFFu;
    GLuint stencil_write_mask = 0xFFFFFFFFu;
    StencilFace stencil_front, stencil_back;
    int unpack_alignment = 4;
    int pack_alignment = 4;
    int unpack_row_length = 0;
    GLuint active_unit = 0;
    std::array<GLuint, kUnits> unit_2d{};
    std::array<GLuint, kUnits> unit_buffer{};
    std::array<GLuint, kUnits> unit_array{};
    std::array<GLuint, kUnits> unit_sampler{};
    GLuint vertex_array = 0;
    GLuint array_buffer = 0;
    GLuint uniform_buffer = 0;
    GLuint texture_buffer = 0;
    std::array<GLuint, kUniformBindings> uniform_bindings{};
    std::array<GLuint, kUniformBindings> storage_bindings{};
    GLuint storage_buffer = 0;
    GLuint program = 0;
    GLuint draw_framebuffer = 0;
    GLuint read_framebuffer = 0;
    GLuint renderbuffer = 0;
};

// GL's names for one kind of object: a map from name to object, and the next name to give.
template <class T>
struct Names {
    std::unordered_map<GLuint, T> objects;
    GLuint next = 1;

    GLuint add() {
        const GLuint name = next++;
        objects.emplace(name, T{});
        return name;
    }

    T* find(GLuint name) {
        if (name == 0) {
            return nullptr;
        }
        auto it = objects.find(name);
        return it == objects.end() ? nullptr : &it->second;
    }
};

// Which memory a static buffer's copy of its own goes into, worked out once from the device's
// memory types (static_memory_of, vkgl_resources.cpp).
struct StaticMemory {
    bool known = false;  // Worked out yet.
    // Memory both device-local and mapped by the host lies on the largest device-local heap:
    // unified memory (Apple silicon, an integrated GPU) or a discrete GPU with resizable BAR.
    // One mapped allocation is then device memory; nothing needs copying.
    bool mappable = false;
    std::uint32_t device_only_types = 0;  // The memory types device-local and not host-visible.
};

// What the layer cost in a frame, given to the debug tools (common/stats.h) at present(): the
// measure of whether a path of the renderer should be written for Vulkan itself.
struct Measures {
    std::uint64_t draws = 0;
    std::uint64_t native_draws = 0;
    std::uint64_t pipelines = 0;   // made this frame (each a compile on the driver)
    std::uint64_t ring_bytes = 0;  // uploaded through the frame's ring
    std::uint64_t flushes = 0;     // waits for the GPU inside the frame (glReadPixels)
    double draw_s = 0.0;           // in draw(): state to pipeline, descriptors, uploads, recording
    double wait_s = 0.0;           // for the frame two back to finish
    double present_s = 0.0;        // acquiring, copying to the screen, submitting
};

// The clock the measures are taken with.
using Clock = std::chrono::steady_clock;

// The seconds since `start`, for the measures.
inline double seconds_since(Clock::time_point start) {
    return std::chrono::duration<double>(Clock::now() - start).count();
}

// ---- Pipelines by key ----

// One vertex input of a pipeline's key: location, format, stride and whether it is per instance.
struct Input {
    std::uint32_t location, format, stride, instanced;
};

// Everything a pipeline bakes in, as plain numbers: the cache's key. pipeline_for zeroes it whole
// before it fills it, so its bytes (padding included) are the key: it is compared and hashed as
// bytes. Trivially copyable, with no padding between its 8-byte words.
struct PipelineKey {
    std::uint64_t program;
    std::uint32_t topology, restart, polygon, cull, front;
    std::uint32_t blend, src_rgb, dst_rgb, src_a, dst_a, equation, colour_mask;
    std::uint32_t depth_test, depth_write, depth_func;
    std::uint32_t stencil, stencil_func, front_ops[3], back_ops[3];
    std::uint32_t colour_format, depth_format, samples;
    std::uint32_t inputs;
    std::uint32_t viewports;
    Input input[kAttributes];
};

static_assert(sizeof(PipelineKey) % sizeof(std::uint64_t) == 0, "PipelineKey hashes as words");

// Returns the 64-bit hash of `key`'s bytes: FNV-1a over its 8-byte words, then mixed, so that its
// low bits pick a slot of PipelineTable. Equal keys hash alike; a lookup still compares the key.
std::uint64_t hash_key(const PipelineKey& key);

// Returns the same hash of `words`, for DescriptorCache's keys.
std::uint64_t hash_words(std::span<const std::uint64_t> words);

// The graphics pipelines made so far, one per key, and two ways to find them: by the key's hash
// in a flat table of slots that compares the whole key on a hit (vk-draw-cache), or by the key's
// bytes as a std::string (the reference path, as the backend looked them up before R-04). Both
// indexes name the same entries, so the two paths find the same pipelines.
//
// Owned by the Context; the pipelines themselves are destroyed by whoever erases them (a deleted
// program, vkgl_gl.cpp) or by the backend's close (vkgl_device.cpp). Only the thread that draws
// uses it.
class PipelineTable {
public:
    using Entry = std::pair<PipelineKey, VkPipeline>;  // A key and the pipeline made for it.

    // Returns the pipeline made for `key`, whose hash_key is `hash`, or null when none was.
    // Hot path: one slot probe or a few, and one compare of the key; no allocation.
    [[nodiscard]] const VkPipeline* find(const PipelineKey& key, std::uint64_t hash) const;

    // Returns the pipeline made for the key whose bytes are `key`, or null: the reference path.
    [[nodiscard]] const VkPipeline* find_reference(const std::string& key) const;

    // Adds `pipeline` for `key`, whose hash_key is `hash`. The key must not be in the table yet.
    void insert(const PipelineKey& key, std::uint64_t hash, VkPipeline pipeline);

    // Removes every pipeline whose key names program `serial`, handing each to `retire`.
    template <typename Retire>
    void erase_program(std::uint64_t serial, Retire retire) {
        // The program's entries to the back, whole (remove_if would leave the back unspecified).
        const auto ends =
            std::stable_partition(m_entries.begin(), m_entries.end(), [serial](const Entry& e) {
                return e.first.program != serial;
            });

        // Each erased pipeline, before its entry goes.
        for (auto it = ends; it != m_entries.end(); ++it) {
            retire(it->second);
        }

        m_entries.erase(ends, m_entries.end());
        rebuild();
    }

    // The entries, for destroying every pipeline at close.
    [[nodiscard]] std::vector<Entry>::iterator begin() { return m_entries.begin(); }

    [[nodiscard]] std::vector<Entry>::iterator end() { return m_entries.end(); }

private:
    // Puts entry `index`, of hash `hash`, into a free slot. The slots must have room.
    void place(std::size_t index, std::uint64_t hash);

    // Makes both indexes again from the entries, the slots at least twice the entries' count.
    void rebuild();

    std::vector<Entry> m_entries;         // Every pipeline, in the order they were made.
    std::vector<std::uint64_t> m_hashes;  // Each entry's hash_key, at the entry's index.
    // Open addressing over a power of two: 0 is a free slot, n the entry n - 1. Probed linearly.
    std::vector<std::uint32_t> m_slots;
    std::unordered_map<std::string, std::size_t> m_reference;  // Key bytes to entry index.
};

// The viewports (and scissors) a draw can set at once, which RecordedState can remember; a draw
// with more indexed viewports than this records them every time.
inline constexpr std::size_t kTrackedViewports = 16;

// What the frame's main command buffer holds of the state a draw records: the dynamic state and
// the graphics binds last recorded into it, so that a draw records only what differs
// (render_opt's vk-state-skip, docs/port/RENDER_PERF.md, R-03). The pipeline itself is
// Context::bound_pipeline, which was tracked before.
//
// Vulkan keeps dynamic state and binds for the life of a command buffer, across draws and
// pipeline binds; a new command buffer starts with none. The tracker is cleared (forget()) at
// every begin of rendering and every begin of the command buffers, which is more often than
// Vulkan needs: a draw always follows a begin of rendering, so nothing recorded in another pass
// or another command buffer is ever trusted. Only the thread that draws touches it.
struct RecordedState {
    // The viewports last set from viewport 0, `viewport_count` of them; 0 when nothing is known.
    std::uint32_t viewport_count = 0;
    std::array<VkViewport, kTrackedViewports> viewports{};

    // The scissors last set from scissor 0, `scissor_count` of them; 0 when nothing is known.
    std::uint32_t scissor_count = 0;
    std::array<VkRect2D, kTrackedViewports> scissors{};

    // The blend constants last set, as their bits: a float compare would take -0 for +0.
    std::optional<std::array<std::uint32_t, 4>> blend_bits;

    std::optional<std::uint32_t> stencil_reference;     // vkCmdSetStencilReference's value.
    std::optional<std::uint32_t> stencil_compare_mask;  // vkCmdSetStencilCompareMask's value.
    std::optional<std::uint32_t> stencil_write_mask;    // vkCmdSetStencilWriteMask's value.

    // The layout the descriptor sets and the push constant below were recorded with. Binding a
    // set with another layout may disturb the others, so a change of layout forgets them.
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkDescriptorSet set = VK_NULL_HANDLE;           // Set 0, the program's; null when not known.
    std::optional<std::uint32_t> dynamic_offset;    // Set 0's dynamic offset, when it has one.
    VkDescriptorSet material_set = VK_NULL_HANDLE;  // Set 1, the material pages'; null: unknown.
    std::optional<std::uint32_t> push_constant;     // The material draw's index, pushed.

    // The vertex buffers last bound from binding 0, `vertex_count` of them; 0 when not known.
    std::uint32_t vertex_count = 0;
    std::array<VkBuffer, kAttributes> vertex_buffers{};
    std::array<VkDeviceSize, kAttributes> vertex_offsets{};

    VkBuffer index_buffer = VK_NULL_HANDLE;         // The index buffer bound; null: not known.
    VkDeviceSize index_offset = 0;                  // Its offset.
    VkIndexType index_type = VK_INDEX_TYPE_UINT32;  // Its index type.

    // Forgets everything: the next draw records all of its state.
    void forget() { *this = RecordedState{}; }

    // Forgets the descriptor sets and the push constant when `to` is not the layout they were
    // recorded with, and notes `to` as theirs.
    void use_layout(VkPipelineLayout to) {
        // The same layout keeps what was bound with it.
        if (to == layout) {
            return;
        }

        set = VK_NULL_HANDLE;
        dynamic_offset.reset();
        material_set = VK_NULL_HANDLE;
        push_constant.reset();
        layout = to;
    }
};

// All of the backend's state, one of it while the backend is open (`g`).
struct Context {
    GpuEngine gpu;
    bool bindless_supported = false;
    unsigned material_texture_capacity = 0;
    VkDescriptorSetLayout material_layout = VK_NULL_HANDLE;
    std::uint64_t material_draws = 0, material_records = 0, material_textures = 0;
    bool open = false;
    Measures measures;
    SubmissionCounters submission, reported_submission;
    SubmissionCounters published;  // The render counters as the last frame's gauges left them.
    // The frames_presented at which begin_pipeline_warmup's warm-up ends; never, until it is called.
    std::uint64_t warmup_end = UINT64_MAX;
    SDL_Window* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties properties{};
    std::uint32_t timestamp_bits = 0;
    VkPhysicalDeviceFeatures features{};
    VkDevice device = VK_NULL_HANDLE;
    std::uint32_t family = 0;
    VkQueue queue = VK_NULL_HANDLE;
    std::atomic_uint64_t validation_errors = 0;
    VmaAllocator allocator = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchain_format = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchain_extent{};
    std::vector<VkImage> swapchain_images;
    std::vector<VkSemaphore> rendered;  // per swapchain image
    bool swapchain_stale = false;
    // The window's surface went with the app to the background (Android lets go of its window
    // there): a new one is made from the window SDL holds before the next frame is shown.
    std::atomic<bool> surface_lost{false};
    int interval = 1;  // -1 adaptive, 0 immediate, 1 vsync
    VkFormat depth_format = VK_FORMAT_UNDEFINED;
    VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
    StaticMemory static_memory;  // Where a static buffer's own copy goes (vkgl_resources.cpp).

    std::array<Frame, kFrames> frames;
    int frame = 0;
    std::uint64_t frame_count = 1;  // frames begun
    std::uint64_t recording = 1;    // command buffers begun (a flush begins new ones)
    bool upload_used = false;

    ImagePtr default_colour, default_depth;
    int default_width = 0, default_height = 0;

    bool rendering = false;
    Target rendering_target;
    OpenPass pass;                 // The pass `main` renders in, while `rendering`.
    PendingPresent present_copy;   // The copy to the default framebuffer that waits, if any.
    bool poison_discards = false;  // test_poison_discards (vkgl.h): discarded data made garbage.
    VkPipeline bound_pipeline = VK_NULL_HANDLE;
    RecordedState recorded;  // What the main command buffer holds of a draw's state (R-03).

    State state;
    Names<Texture> textures;
    Names<Buffer> buffers;
    Names<VertexArray> vertex_arrays;
    Names<Framebuffer> framebuffers;
    Names<Renderbuffer> renderbuffers;
    Names<SamplerParams> samplers;
    Names<Shader> shaders;
    Names<Program> programs;
    std::uint64_t program_serial = 0;

    PipelineTable pipelines;           // Every graphics pipeline, by key (pipeline_for).
    DrawScratch scratch;               // The draw's temporaries, kept for their capacity (R-04).
    bool byte_indices_logged = false;  // The 8-bit index emulation has been reported once.
    std::vector<std::pair<std::uint64_t, VkSampler>> vk_samplers;  // by a packed key

    ImagePtr dummy_image;
    VkBuffer dummy_buffer = VK_NULL_HANDLE;  // zeros, and the default attributes' (0, 0, 0, 1)
    VmaAllocation dummy_allocation = nullptr;
    VkBufferView dummy_views[4] = {};  // R32UI, RGBA32UI, RGBA32F, RGBA32I

    std::string device_name, driver_version, api_version, gl_renderer, gl_version;
    bool software = false;
    bool viewport_index_supported = false;
    std::vector<std::array<int, 4>> indexed_viewports;
    GLenum error = GL_NO_ERROR;
};

// The backend's state: made by open() and destroyed by close() (vkgl.cpp), null while closed.
// The thread that draws uses it; on_event, from SDL's event thread, only sets `surface_lost`.
extern Context* g;

// Records `error` for glGetError, unless an earlier one is still waiting to be read.
inline void fail(GLenum error) {
    if (g->error == GL_NO_ERROR) {
        g->error = error;
    }
}

// True when `result` is VK_SUCCESS; otherwise logs that `what` failed and returns false.
inline bool check(VkResult result, const char* what) {
    if (result == VK_SUCCESS) {
        return true;
    }
    log::error("vulkan: {} failed ({})", what, static_cast<int>(result));
    return false;
}

// The frame slot being recorded.
inline Frame& frame() {
    return g->frames[static_cast<std::size_t>(g->frame)];
}

// ---- The render counters (SubmissionCounters, docs/port/RENDER_PERF.md) ----
//
// Hot path: each is an add or two on `g->submission`, with no clock, no allocation and no test of
// stats::enabled(), so a draw may call them as often as it records. present() publishes the totals
// once per frame (publish_counters, vkgl_present.cpp), the one place that tests stats::enabled().

// Counts `count` recorded commands of `kind`.
inline void count_command(Command kind, std::uint64_t count = 1) {
    g->submission.commands[static_cast<std::size_t>(kind)] += count;
}

// Counts one vkCmdPipelineBarrier that waits on `source` stages for `destination` stages.
inline void count_barrier(VkPipelineStageFlags source, VkPipelineStageFlags destination) {
    count_command(Command::Barrier);
    ++g->submission.barriers;

    // A barrier on every stage, which waits for more than the work it orders needs.
    if (((source | destination) & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) != 0) {
        ++g->submission.barriers_all_commands;
    }
}

// Counts one copy, blit or resolve into a region `width` by `height` texels: a full-screen copy
// as well when that is the default framebuffer's size or the screen's.
inline void count_copy(std::uint32_t width, std::uint32_t height) {
    count_command(Command::Copy);
    const bool frame_sized = static_cast<int>(width) == g->default_width
                             && static_cast<int>(height) == g->default_height;
    const bool screen_sized =
        width == g->swapchain_extent.width && height == g->swapchain_extent.height;

    // The whole picture read and written again, the costliest copy on a tiled GPU.
    if (frame_sized || screen_sized) {
        ++g->submission.fullscreen_copies;
    }
}

// Counts the beginning of one render pass (vkCmdBeginRendering); its end is a count_command.
inline void count_render_pass() {
    count_command(Command::RenderPass);
    ++g->submission.render_passes;
}

// Counts a graphics or compute pipeline just made, after the warm-up as well when it has passed.
inline void count_pipeline() {
    ++g->submission.pipelines_created;

    // Made once the warm-up begin_pipeline_warmup started was over: a hitch a warm cache avoids.
    if (g->submission.frames_presented >= g->warmup_end) {
        ++g->submission.pipelines_created_after_warmup;
    }
}

// Counts what one attachment of a pass moves between memory and the GPU: `image`'s texel bytes
// over `area` and its samples, read in when `load` and written back when `store`. Defined in
// vkgl_commands.cpp, beside the passes it counts.
void count_attachment(const Image& image, VkExtent2D area, bool load, bool store);

// ---- Types the files hand each other ----

// A piece of a frame's ring: the buffer, the offset into it, and where the host writes it.
struct Slice {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    std::uint8_t* data = nullptr;
};

// A GL internal format as Vulkan has it: the format, its aspect, the bytes of one texel.
struct Format {
    VkFormat vk = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    int bytes = 4;  // a texel's
};

inline constexpr VkDeviceSize kDefaultFloat = 256;    // in the dummy buffer: (0, 0, 0, 1) as floats
inline constexpr VkDeviceSize kDefaultInteger = 272;  // and as integers

// ---- Defined in vkgl_commands.cpp ----

// Records the indirect draws merged so far (GpuEngine::pending_*) into `main` as one call;
// nothing when none wait. Called before anything that must come after them.
void gpu_flush_draws();

// Runs `f` when this frame slot's fence has signalled (run_deferred): for destroying what this
// frame's commands may still use.
void defer(std::function<void()> f);

// Destroys the image's views and the image now and leaves `image` empty. Only for an image no
// submitted command still uses; retire is for the others.
void destroy_image_now(Image& image);

// Drops a reference to an image at the end of the frame: what this frame recorded may use it.
void retire(ImagePtr image);

// Takes `size` bytes (at least 4) of this frame's ring, aligned for any use, adding a chunk
// when the ones there are full. An empty Slice when no chunk can be made.
Slice ring(VkDeviceSize size);

// Takes `size` bytes of the ring (ring) and copies `data` into them, when `data` is not null.
Slice ring_copy(const void* data, VkDeviceSize size);

// This frame's `upload` command buffer, marked used so that it is submitted.
VkCommandBuffer upload_cmd();

// Flushes the merged draws and ends the rendering pass `main` is in, if any, forgetting the
// bound pipeline.
void end_rendering();

// The command buffer for an operation on `image`, outside any drawing: `upload` while this
// recording has not used it in `main` (it then runs before everything `main` does), else `main`.
VkCommandBuffer cmd_for(Image* image, bool force_main = false);

// Records into `cmd` a barrier that moves the whole image from its last layout to `to`, and
// notes `to` in `image`. It waits on all stages and all memory: correct, not narrow.
void barrier(VkCommandBuffer cmd, Image& image, VkImageLayout to);

// Begins this frame's two command buffers afresh, empties its GPU-driven and material pages,
// resets its timestamp queries when unused, and counts a new recording.
void begin_buffers();

// Native timestamp pairs back the existing non-nesting stats::Scope API.
// Read only after the frame-slot fence signals; profiling never waits on a
// query or adds per-pass submissions. Slots also survive readback flushes.
bool timing_begin(std::uint64_t serial, int section);

// Ends the GPU timing section timing_begin opened (and its debug label with
// OPENRAC_GPU_MARKERS). Handed to stats::set_gpu_hooks by open().
void timing_end();

// Hands frame slot `f`'s finished timing sections and GPU-driven counters to common/stats.h.
// Its fence must have signalled.
void timing_results(Frame& f);

// Ends both command buffers and submits them, `upload` first; `wait` waits for them to finish
// (a read of pixels, glFinish), and the next ones are begun. With `present` (present_cmd), that
// buffer is ended and goes in a second batch of the same submit, which alone waits for
// `wait_semaphore`, at the transfer stage; otherwise the one batch waits for it at every stage.
void submit(
    VkSemaphore wait_semaphore, VkSemaphore signal, bool wait, VkCommandBuffer present = nullptr
);

// The frame slot's command buffer for the copy to the screen, begun (vk-tight-sync), or null when
// none can be made. Submitted by submit() after the frame's two.
VkCommandBuffer present_cmd();

// Submits what is recorded and waits for the GPU to finish it (glReadPixels, glFinish).
void flush_and_wait();

// What framebuffer `name` draws into (0: the default framebuffer), with its size and samples.
Target target_of(GLuint name);

// Begins rendering into `t`, keeping its contents, unless `main` already renders into it.
void ensure_rendering(const Target& t);

// The part of `t` drawing may touch: the scissor box clamped to `t`, or all of `t` without the
// scissor test.
VkRect2D scissor_rect(const Target& t);

// glInvalidateFramebuffer carried out on `t`: `colour` and the depth image's `depth` aspects are
// let go. In the pass that renders into them, they are not stored at its end unless a draw or a
// clear comes first; otherwise the next pass on them does not load them.
void discard_attachments(const Target& t, bool colour, VkImageAspectFlags depth);

// Ends the open pass without recording it when it is a pass into the default framebuffer that
// only clears its colour and holds nothing yet; true then, with that colour in `clear`.
bool drop_clear_pass(VkClearColorValue& clear);

// glClear: clears the buffers in `mask` that the write masks leave writable, inside the scissor
// box. A clear of the whole target becomes the load operation of the rendering pass it begins.
void clear(GLbitfield mask);

// ---- Defined in vkgl_resources.cpp ----

// The Vulkan format, aspect and texel size for GL internal format `internal`.
Format format_of(GLenum internal);

// True for the depth formats that have a stencil part.
bool has_stencil(VkFormat f);

// The bytes one texel of `f` takes, for the attachment counters (count_attachment); 4 for a
// format the backend does not make, which is only the screen's.
int texel_bytes(VkFormat f);

// Makes a 2D image of `levels` levels and `samples` samples that can be copied to and from,
// sampled when single-sampled, and drawn into when the format allows. Null when Vulkan fails
// (logged). The image goes with its last owner.
ImagePtr make_image(
    const Format& format, int width, int height, int levels, VkSampleCountFlagBits samples
);

// The view of level 0 that draws into the image, made the first time it is asked for.
VkImageView attachment_view(Image& image);

// For sampling: levels base..top; a depth image's depth.
VkImageView sample_view(Image& image, int base, int top);

// Pixels as the GL call gave them (format, type, the unpack alignment and row length) into the
// ring, as the image's format lays them out, tightly.
Slice stage_pixels(
    const Format& to, int width, int height, GLenum format, GLenum type, const void* pixels
);

// Records into `cmd` a copy of the staged pixels `from` into the `width` by `height` rectangle
// at (`x`, `y`) of level `level`, moving the image to TRANSFER_DST_OPTIMAL first.
void copy_to_image(
    VkCommandBuffer cmd,
    Image& image,
    const Slice& from,
    int level,
    int x,
    int y,
    int width,
    int height
);

// The image again with a whole mip chain, level 0 copied over: for a texture that is given a
// level past its first, or mipmaps.
ImagePtr with_chain(const ImagePtr& old);

// Fills each level past the first with a blit of the level above (glGenerateMipmap).
void generate_mipmaps(Image& image);

// The sampler for GL's parameters `p`, made once and kept under a packed key. An `integer`
// texture is sampled with nearest filtering only.
VkSampler sampler_for(const SamplerParams& p, bool integer);

// The first 256 bytes of the dummy buffer (zeros): what a draw binds where no buffer is.
BufferRef dummy_buffer_ref();

// Where buffer `b`'s current data is on the GPU: its own copy once it has stayed the same for
// a few frames (or at once for GPU-written storage, and for a static buffer of 64 KiB or more
// under static-buffers-device-local), else a copy in this frame's ring made once per version.
// With `persistent` false only storage gets its own copy. The dummy buffer when `b` is empty.
BufferRef resolve(Buffer& b, bool persistent = true);

// Buffer `b`'s bytes on the CPU: its mirror, read back from its own copy first when it was let
// go, and kept from then on. The read-back of device-only memory submits a command buffer of its
// own and waits for it, leaving the frame's recording as it is.
const std::vector<std::uint8_t>& buffer_bytes(Buffer& b);

// Drops the texel views made of `b`, destroyed once the frame is done with them (defer).
void forget_views(Buffer& b);

// A texel view of buffer `name` as `gl_format` (RGBA32UI, RGBA32F, RGBA8, otherwise R32UI),
// kept for this frame; `fallback` when the buffer is missing or too small.
VkBufferView texel_view(GLuint name, GLenum gl_format, VkBufferView fallback);

// ---- Defined in vkgl_pipelines.cpp ----

// The Vulkan vertex format for `size` components (clamped to 4 at most) of GL type `type`,
// normalised or integer as glVertexAttribPointer was told.
VkFormat vertex_format(GLenum type, int size, bool normalized, bool integer);

// The bytes of one component of GL type `type`.
int type_bytes(GLenum type);

// The kind of number a shader input of this GL type holds: 0 float, 1 unsigned, 2 signed.
int input_kind(std::uint32_t gl_type);

// The graphics pipeline for program `p` drawing `mode` into `t` with the current state and the
// `count` vertex `inputs`, made once per key (through the pipeline cache) and kept.
VkPipeline pipeline_for(
    Program& p, GLenum mode, const Target& t, const Input* inputs, std::uint32_t count
);

// A descriptor set of `layout` from this frame's pools, adding a pool when they are full.
// VK_NULL_HANDLE when none can be made.
VkDescriptorSet allocate_set(VkDescriptorSetLayout layout);

// What sampler uniform `i` of program `p` reads now (what is bound to its unit), or the fallback of
// the shader's type when nothing usable is bound.
Sampled sampled(const Program& p, std::size_t i);

// The descriptor set with program `p`'s uniform blocks, default uniforms and `samplers` for
// this draw, reused while the same resources are bound this frame. VK_NULL_HANDLE for a
// program that binds nothing.
//
// Hot path: with vk-draw-cache its temporaries are Context::scratch's and its sets
// Program::set_cache, so a draw that finds its set allocates nothing; off, the reference path
// builds them as vectors and keys Program::descriptor_sets by a copy of the key.
VkDescriptorSet descriptors(Program& p, std::span<const Sampled> samplers);

// glDispatchCompute: runs the current program's compute pipeline outside a rendering pass.
void OPENRAC_GLAPI dispatch_compute(GLuint x, GLuint y, GLuint z);

// glMemoryBarrier: orders shader writes before later shader and uniform reads, whatever bits
// were asked for.
void OPENRAC_GLAPI memory_barrier(GLbitfield);

// ---- Defined in vkgl_draw.cpp ----

// The bound vertex array, or vertex array 0.
VertexArray& vertex_array();

// Records one draw with the current GL state: `count` vertices or indices from `first`, or the
// indices of `index_type` at `index_offset` when `indexed`, `instances` times. With `gpu_page`
// it is command `gpu_command` of that GPU-driven page: the instances come from the page's
// culled output and the draw is indirect, merged with its neighbours where it can be.
void draw(
    GLenum mode,
    GLsizei count,
    GLsizei instances,
    bool indexed,
    GLenum index_type,
    std::uintptr_t index_offset,
    GLint first,
    GpuPage* gpu_page = nullptr,
    unsigned gpu_command = 0
);

// True when the native material path is on: the device can index its textures and
// OPENRAC_GPU_MATERIALS_OFF is unset.
bool material_enabled();

// Prepares program `p` for the native material path when its shaders draw meshes (a model
// matrix and material uniforms); true, with nothing done, for any other program. False when
// the preparation fails.
bool material_program(Program& p, const std::string& vertex, const std::string& fragment);

// Empties frame slot `f`'s material pages for a new recording.
void material_reset(Frame& f);

// Destroys every frame's material pages, when the backend closes.
void material_release();

// Makes the GPU-driven culling pipelines the first time; false, for good, once that failed.
bool gpu_make_engine();

// Makes a buffer of `size` bytes for `usage` in device memory, mapped for writing when
// `mapped`. An empty Chunk on failure.
Chunk gpu_buffer(VkDeviceSize size, VkBufferUsageFlags usage, bool mapped);

// Empties frame slot `f`'s GPU-driven pages for a new recording.
void gpu_reset(Frame& f);

// gpu_instances (vkgl.h) carried out: the instances of vertex buffer `buffer` with `bounds`,
// to be culled on the GPU against `view`. An empty batch when GPU-driven drawing is off.
renderer::GpuInstanceBatch gpu_make_batch(
    unsigned buffer,
    std::span<const renderer::GpuCullBounds> bounds,
    const renderer::GpuCullView& view
);

// gpu_register_instances (vkgl.h) carried out: the instances of vertex buffer `buffer` with
// `bounds` appended to the resident words. False when GPU-driven drawing is off.
bool gpu_register(unsigned buffer, std::span<const renderer::GpuCullBounds> bounds);

// gpu_release_instances (vkgl.h) carried out: every registration forgotten, the device copy let
// go once the frames that read it have retired.
void gpu_release_resident();

// draw_gpu_instances (vkgl.h) carried out: `count` indices from `first` for each instance of
// `batch` the culling keeps. False for a batch of another recording, or nothing to draw.
bool gpu_submit_draw(renderer::GpuInstanceBatch batch, unsigned first, unsigned count);

// Records this frame's culling dispatches into `upload`, which runs before `main` draws.
void gpu_record();

// Reads frame slot `f`'s GPU-driven counters for the statistics; its fence has signalled.
void gpu_results(Frame& f);

// Destroys every frame's GPU-driven pages and the culling pipelines, when the backend closes.
void gpu_release();

// ---- Defined in vkgl_gl.cpp ----

// Records the copy into the default framebuffer that waited (PendingPresent): its clear, then the
// blit, as glBlitFramebuffer would have recorded them. Nothing when none waits.
void make_present_copy();

// Lets go of buffer `b`'s GPU copy and texel views, destroyed once the frame is done with them.
void destroy_buffer(Buffer& b);

// Destroys program `p`'s shader modules, compute pipeline and layouts once the frame is done
// with them.
void release_program(Program& p);

// ---- Defined in vkgl_present.cpp ----

// Runs and empties the work deferred in frame slot `f` (defer). Its fence has signalled.
void run_deferred(Frame& f);

// Makes the default framebuffer's colour and depth images, `width` by `height`.
bool make_default_framebuffer(int width, int height);

// The present mode for the swap interval asked for (`interval`): immediate, FIFO relaxed or
// FIFO, as the surface offers them; FIFO when nothing else fits.
VkPresentModeKHR present_mode();

// Makes the swapchain anew for the surface at its current size, once the device is idle. True
// with no surface (no window).
bool make_swapchain();

// Publishes the render counters as the frame's vk_* gauges, the frame's own work for counts and
// bytes, the totals for the pipelines and compiles, while the debug tools measure; counts the
// frame as presented in any case. Called by present(), once per frame, before advance().
void publish_counters();

// The next frame: its slot's last use waited for, its leftovers freed, its buffers begun.
void advance();

// Copies the default framebuffer into the next swapchain image and presents it, making a new
// surface or swapchain first when the old one went. False when it cannot.
bool present_to_screen();

// ---- Defined in vkgl_device.cpp ----

// True when the Vulkan loader offers instance layer `name`.
bool layer_available(const char* name);

// Makes the instance with the extensions the window needs, and the validation layer and its
// messenger when `validation`. False with `error` set when it cannot.
bool make_instance(const Config& config, bool validation, std::string& error);

// The device: a graphics card before an integrated one before one that draws on the CPU, with
// Vulkan 1.3 (dynamic rendering) and a queue that draws and shows the window.
bool pick_device(std::string& error);

// Makes the logical device with the features and extensions the backend uses, its queue, the
// memory allocator and the pipeline cache. False with `error` set when it cannot.
bool make_device(std::string& error);

// Makes each frame slot's command pool and buffers, fence, semaphore and timestamp queries.
bool make_frames();

// Makes the dummy buffer (zeros, and the default attributes), its texel views and the 1 by 1
// dummy image, bound where nothing else is.
bool make_dummies();

// A new surface for the window, where the old one went (renew_surface).
bool make_surface(SDL_Window* window, std::string& error);

// Fills in the device's name, driver and API versions and the GL strings from its properties.
void describe_device();

// Waits for the device, then destroys all the backend made, the device and the instance last.
void destroy_all();

}  // namespace openrac::vkgl::detail

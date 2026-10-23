// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// renderer/gl.h and renderer/world/gl_world.h carried out with Vulkan (vkgl.h says how GL's model
// becomes Vulkan's). Two command buffers a frame: `upload`, where what changes a texture nothing
// this frame has used yet goes (so it needs no pause of the drawing), run first, then `main`,
// where everything else goes in GL's order. Every image keeps the layout it was last left in,
// and is moved to the one an operation needs just before it.

#include "renderer/vulkan/vkgl.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <volk.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "common/float_encoding.h"
#include "common/log.h"
#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/vulkan/shader_compiler.h"
#include "renderer/world/gl_world.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

namespace openrac::vkgl {
namespace {

using namespace openrac::gl;
namespace wgl = openrac::renderer::world::gl;

constexpr int kFrames = 2;
constexpr int kUnits = 32;
constexpr int kAttributes = 16;
constexpr int kUniformBindings = 16;
constexpr VkDeviceSize kChunk = 8u << 20;

// GL enums the headers do not list, which the functions below are asked about.
constexpr GLenum kGlActiveUniforms = 0x8B86;
constexpr GLenum kGlInvalidEnum = 0x0500;
constexpr GLenum kGlUniformBufferOffsetAlignment = 0x8A34;
constexpr GLenum kGlMaxCombinedTextureUnits = 0x8B4D;
constexpr GLenum kGlMaxTextureBufferSize = 0x8C2B;
constexpr GLenum kGlMaxVertexAttribs = 0x8869;
constexpr GLenum kGlMaxUniformBlockSize = 0x8A30;
constexpr GLenum kGlTextureBindingSlot = 0x8069;  // GL_TEXTURE_BINDING_2D
constexpr GLenum kGlFramebufferBinding = 0x8CA6;
constexpr GLenum kGlViewportQuery = 0x0BA2;
constexpr GLenum kGlHalfFloat = 0x140B;
constexpr GLenum kGlByte = 0x1400;
constexpr GLenum kGlDepthStencil = 0x84F9;
constexpr GLenum kGlRed = 0x1903;
constexpr GLenum kGlR8 = 0x8229;
constexpr GLenum kGlRgba16f = 0x881A;
constexpr GLenum kGlR11fG11fB10f = 0x8C3A;
constexpr GLenum kGlRg32f = 0x8230;
constexpr GLenum kGlUnpackRowLength = 0x0CF2;
constexpr GLenum kGlNearestMipmapLinear = 0x2702;
constexpr GLenum kGlMirroredRepeat = 0x8370;
constexpr GLenum kGlIntSamplerBuffer = 0x8DD0;
constexpr GLenum kGlUnsignedSamplerBuffer = 0x8DD8;

// ---- The device ----

struct Chunk {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    std::uint8_t* mapped = nullptr;
    VkDeviceSize size = 0;
};

// Bounded, fence-owned pages. Growing a frame adds a page rather than moving
// buffers already referenced by recorded draws. Reuse only after slot retirement.
constexpr unsigned kGpuPageInstances = 32768, kGpuPageGroups = 4096, kGpuPageDraws = 32768;
constexpr unsigned kGpuInputBytes = 8u << 20;

constexpr unsigned kMaterialDraws = 32768;
constexpr unsigned kMaterialWords = 20;
constexpr unsigned kMaterialBytes = kMaterialWords * sizeof(std::uint32_t);

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

struct MaterialPage {
    Chunk data;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet set = VK_NULL_HANDLE;
    std::unordered_map<std::array<std::uint64_t, 2>, unsigned, ArrayHash<std::uint64_t, 2>>
        textures;
    std::unordered_map<std::array<std::uint32_t, kMaterialWords>, unsigned, ArrayHash<std::uint32_t, kMaterialWords>>
        materials;
    unsigned draws = 0;
};

struct GpuPage {
    Chunk input, output, arguments, readback;
    std::vector<std::array<std::uint32_t, 4>> groups, commands;
    std::vector<std::uint32_t> payload;
    unsigned instances = 0;
    VkDescriptorSet descriptors = VK_NULL_HANDLE;
};

struct GpuFrame {
    std::vector<GpuPage> pages;
    unsigned used = 0;
    bool counters = false;
    std::uint64_t recording = 0, reported = 0;
};

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
};

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
    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore acquired = VK_NULL_HANDLE;
    std::vector<VkDescriptorPool> pools;
    std::size_t pool_at = 0;
    std::vector<Chunk> chunks;
    std::size_t chunk_at = 0;
    VkDeviceSize chunk_used = 0;
    std::vector<std::function<void()>> deferred;
    bool submitted = false;  // the fence will be signalled
};

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
    VkImageView attachment = VK_NULL_HANDLE;
    std::vector<std::pair<std::uint32_t, VkImageView>> views;  // (base | top << 16) -> view for sampling
};

using ImagePtr = std::shared_ptr<Image>;

struct SamplerParams {
    GLenum min = GL_NEAREST;
    GLenum mag = GL_LINEAR;
    GLenum wrap_s = GL_REPEAT;
    GLenum wrap_t = GL_REPEAT;
    float anisotropy = 1.0f;
    bool operator==(const SamplerParams&) const = default;
};

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

struct Renderbuffer {
    ImagePtr image;
};

struct Buffer {
    bool storage = false;  // GPU-written storage must keep one allocation across frames.
    std::vector<std::uint8_t> data;
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
    struct View {
        VkFormat format;
        VkBuffer buffer;
        VkDeviceSize offset;
        VkBufferView view;
    };
    std::vector<View> views;
    std::uint64_t views_frame = 0;
};

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

struct VertexArray {
    std::array<Attribute, kAttributes> attributes{};
    GLuint elements = 0;
};

struct Shader {
    GLenum type = 0;
    std::string source;
    bool compiled = false;
    std::string log;
};

struct DescriptorKeyHash {
    std::size_t operator()(const std::vector<std::uint64_t>& key) const {
        std::size_t hash = 0;
        for (auto value : key) {
            hash ^= std::hash<std::uint64_t>{}(value) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

bool reuse_uniforms() {
    static const bool& off = stats::probe(
        "OPENRAC_VK_UNIFORM_REUSE_OFF",
        "Profiling reference: upload repeated uniforms and recreate descriptors"
    );
    return !off;
}

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
    std::vector<std::uint64_t> last_key;  // what the last descriptor set held
    VkDescriptorSet last_set = VK_NULL_HANDLE;
    std::uint64_t last_set_frame = 0;
};

struct Framebuffer {
    GLuint colour_texture = 0;
    GLuint colour_renderbuffer = 0;
    GLuint depth_texture = 0;
    GLuint depth_renderbuffer = 0;
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

struct StencilFace {
    GLenum sfail = wgl::GL_KEEP;
    GLenum dpfail = wgl::GL_KEEP;
    GLenum dppass = wgl::GL_KEEP;
};

struct State {
    bool blend = false, depth_test = false, cull = false, scissor_test = false, stencil_test = false;
    bool primitive_restart = false;
    GLenum blend_src_rgb = GL_ONE, blend_dst_rgb = GL_ZERO, blend_src_a = GL_ONE, blend_dst_a = GL_ZERO;
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

using Clock = std::chrono::steady_clock;

double seconds_since(Clock::time_point start) {
    return std::chrono::duration<double>(Clock::now() - start).count();
}

struct Context {
    GpuEngine gpu;
    bool bindless_supported = false;
    unsigned material_texture_capacity = 0;
    VkDescriptorSetLayout material_layout = VK_NULL_HANDLE;
    std::uint64_t material_draws = 0, material_records = 0, material_textures = 0;
    bool open = false;
    Measures measures;
    SubmissionCounters submission, reported_submission;
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

    std::array<Frame, kFrames> frames;
    int frame = 0;
    std::uint64_t frame_count = 1;  // frames begun
    std::uint64_t recording = 1;    // command buffers begun (a flush begins new ones)
    bool upload_used = false;

    ImagePtr default_colour, default_depth;
    int default_width = 0, default_height = 0;

    bool rendering = false;
    Target rendering_target;
    VkPipeline bound_pipeline = VK_NULL_HANDLE;

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

    std::unordered_map<std::string, VkPipeline> pipelines;
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

Context* g = nullptr;

void fail(GLenum error) {
    if (g->error == GL_NO_ERROR) {
        g->error = error;
    }
}

bool check(VkResult result, const char* what) {
    if (result == VK_SUCCESS) {
        return true;
    }
    log::error("vulkan: {} failed ({})", what, static_cast<int>(result));
    return false;
}

Frame& frame() {
    return g->frames[static_cast<std::size_t>(g->frame)];
}

Chunk gpu_buffer(VkDeviceSize size, VkBufferUsageFlags usage, bool mapped);
void material_reset(Frame& f);
void material_release();
void gpu_record();
void gpu_reset(Frame& f);
void gpu_release();
void gpu_results(Frame& f);

void gpu_flush_draws() {
    auto& gpu = g->gpu;
    if (!gpu.pending_count) {
        return;
    }
    vkCmdDrawIndexedIndirect(
        frame().main, gpu.pending_arguments, gpu.pending_offset, gpu.pending_count, 20
    );
    ++g->measures.native_draws;
    ++gpu.indirect_calls;
    gpu.pending_count = 0;
}

void defer(std::function<void()> f) {
    frame().deferred.push_back(std::move(f));
}

void destroy_image_now(Image& image) {
    for (auto& [key, view] : image.views) {
        vkDestroyImageView(g->device, view, nullptr);
    }
    if (image.attachment != VK_NULL_HANDLE) {
        vkDestroyImageView(g->device, image.attachment, nullptr);
    }
    if (image.image != VK_NULL_HANDLE) {
        vmaDestroyImage(g->allocator, image.image, image.allocation);
    }
    image = Image{};
}

// Drops a reference to an image at the end of the frame: what this frame recorded may use it.
void retire(ImagePtr image) {
    if (image) {
        defer([image]() mutable { image.reset(); });
    }
}

// ---- The frame's ring: vertices, indices, uniforms, texels on their way ----

struct Slice {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    std::uint8_t* data = nullptr;
};

VkDeviceSize ring_alignment() {
    const VkPhysicalDeviceLimits& l = g->properties.limits;
    return std::max<VkDeviceSize>({256, l.minUniformBufferOffsetAlignment, l.minTexelBufferOffsetAlignment,
                                   l.optimalBufferCopyOffsetAlignment, 16});
}

bool new_chunk(Frame& f, VkDeviceSize size) {
    Chunk c;
    c.size = std::max(size, kChunk);
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = c.size;
    info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                 | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
                 | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo alloc{};
    alloc.usage = VMA_MEMORY_USAGE_AUTO;
    alloc.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    alloc.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VmaAllocationInfo got{};
    if (!check(vmaCreateBuffer(g->allocator, &info, &alloc, &c.buffer, &c.allocation, &got), "a ring buffer")) {
        return false;
    }
    c.mapped = static_cast<std::uint8_t*>(got.pMappedData);
    f.chunks.push_back(c);
    return true;
}

Slice ring(VkDeviceSize size) {
    Frame& f = frame();
    const VkDeviceSize align = ring_alignment();
    size = std::max<VkDeviceSize>(size, 4);
    while (true) {
        if (f.chunk_at < f.chunks.size()) {
            Chunk& c = f.chunks[f.chunk_at];
            const VkDeviceSize at = (f.chunk_used + align - 1) / align * align;
            if (at + size <= c.size) {
                f.chunk_used = at + size;
                g->measures.ring_bytes += size;
                return {c.buffer, at, c.mapped + at};
            }
            ++f.chunk_at;
            f.chunk_used = 0;
            continue;
        }
        if (!new_chunk(f, size)) {
            return {};
        }
    }
}

Slice ring_copy(const void* data, VkDeviceSize size) {
    Slice s = ring(size);
    if (s.data != nullptr && data != nullptr && size > 0) {
        std::memcpy(s.data, data, static_cast<std::size_t>(size));
    }
    return s;
}

// ---- Command buffers and the order of things ----

VkCommandBuffer upload_cmd() {
    g->upload_used = true;
    return frame().upload;
}

void end_rendering() {
    gpu_flush_draws();
    if (g->rendering) {
        vkCmdEndRendering(frame().main);
        g->rendering = false;
        g->bound_pipeline = VK_NULL_HANDLE;
    }
}

// The command buffer for an operation on `image`, outside any drawing: `upload` while this
// recording has not used it in `main` (it then runs before everything `main` does), else `main`.
VkCommandBuffer cmd_for(Image* image, bool force_main = false) {
    if (!force_main && image != nullptr && image->touched != g->recording) {
        return upload_cmd();
    }
    end_rendering();
    if (image != nullptr) {
        image->touched = g->recording;
    }
    return frame().main;
}

void barrier(VkCommandBuffer cmd, Image& image, VkImageLayout to) {
    VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    b.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    b.oldLayout = image.layout;
    b.newLayout = to;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image.image;
    b.subresourceRange = {image.aspect, 0, VK_REMAINING_MIP_LEVELS, 0, 1};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &b);
    image.layout = to;
}

void begin_buffers() {
    Frame& f = frame();
    gpu_reset(f);
    material_reset(f);
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(f.upload, &begin);
    vkBeginCommandBuffer(f.main, &begin);
    if (f.timestamps != VK_NULL_HANDLE && f.timestamp_used == 0) {
        vkCmdResetQueryPool(f.upload, f.timestamps, 0, 512);
    }
    g->upload_used = false;
    ++g->recording;
}

// Native timestamp pairs back the existing non-nesting stats::Scope API.
// Read only after the frame-slot fence signals; profiling never waits on a
// query or adds per-pass submissions. Slots also survive readback flushes.
bool timing_begin(std::uint64_t serial, int section) {
    if (!g || !g->timestamp_bits) {
        return false;
    }
    gpu_flush_draws();
    auto& f = frame();
    if (f.active_timing >= 0 || f.timestamp_used + 2 > 512) {
        return false;
    }
    f.active_timing = static_cast<int>(f.timings.size());
    f.timings.push_back({serial, section, f.timestamp_used});
#if defined(OPENRAC_GPU_MARKERS)
    if (std::getenv("OPENRAC_GPU_MARKERS") && vkCmdBeginDebugUtilsLabelEXT) {
        const auto name = stats::gpu_section_name(serial, section);
        VkDebugUtilsLabelEXT label{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
        label.pLabelName = name.c_str();
        label.color[0] = 0.25f; label.color[1] = 0.6f; label.color[2] = 0.9f; label.color[3] = 1;
        vkCmdBeginDebugUtilsLabelEXT(g->gpu.timing_upload ? f.upload : f.main, &label);
    }
#endif
    // Bound both ends at command completion. A TOP start followed by a
    // BOTTOM end includes earlier in-flight work, so adjacent scopes can
    // overlap and their sum inflates when a pass is split into subpasses.
    // This measures completion intervals, without serialising the passes.
    vkCmdWriteTimestamp(
        g->gpu.timing_upload ? f.upload : f.main,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        f.timestamps,
        f.timestamp_used
    );
    f.timestamp_used += 2;
    return true;
}

void timing_end() {
    if (!g) {
        return;
    }
    gpu_flush_draws();
    auto& f = frame();
    if (f.active_timing < 0) {
        return;
    }
    auto& timing = f.timings[static_cast<std::size_t>(f.active_timing)];
    vkCmdWriteTimestamp(
        g->gpu.timing_upload ? f.upload : f.main,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        f.timestamps,
        timing.first + 1
    );
#if defined(OPENRAC_GPU_MARKERS)
    if (std::getenv("OPENRAC_GPU_MARKERS") && vkCmdEndDebugUtilsLabelEXT) {
        vkCmdEndDebugUtilsLabelEXT(g->gpu.timing_upload ? f.upload : f.main);
    }
#endif
    timing.complete = true;
    f.active_timing = -1;
}

void timing_results(Frame& f) {
    gpu_results(f);
    for (auto& timing : f.timings) {
        if (!timing.complete || timing.reported) {
            continue;
        }
        std::uint64_t values[2]{};
        if (vkGetQueryPoolResults(
                g->device,
                f.timestamps,
                timing.first,
                2,
                sizeof(values),
                values,
                sizeof(std::uint64_t),
                VK_QUERY_RESULT_64_BIT
            )
            == VK_SUCCESS) {
            const auto mask =
                g->timestamp_bits == 64 ? UINT64_MAX : (std::uint64_t{1} << g->timestamp_bits) - 1;
            const double ticks = static_cast<double>((values[1] - values[0]) & mask);
            stats::gpu_result(
                timing.serial, timing.section, ticks * g->properties.limits.timestampPeriod / 1e6
            );
            timing.reported = true;
        }
    }
}

// Ends both command buffers and submits them, `upload` first; `wait` waits for them to finish
// (a read of pixels, glFinish), and the next ones are begun.
void submit(VkSemaphore wait_semaphore, VkSemaphore signal, bool wait) {
    end_rendering();
    Frame& f = frame();
    gpu_record();
    // What `upload` did is seen by all that `main` does.
    VkMemoryBarrier m{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    m.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    m.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    vkCmdPipelineBarrier(
        f.upload,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        1,
        &m,
        0,
        nullptr,
        0,
        nullptr
    );
    vkEndCommandBuffer(f.upload);
    vkEndCommandBuffer(f.main);
    const VkCommandBuffer buffers[] = {f.upload, f.main};
    VkSubmitInfo info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    const VkPipelineStageFlags stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    if (wait_semaphore != VK_NULL_HANDLE) {
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &wait_semaphore;
        info.pWaitDstStageMask = &stage;
    }
    info.commandBufferCount = 2;
    info.pCommandBuffers = buffers;
    if (signal != VK_NULL_HANDLE) {
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &signal;
    }
    vkResetFences(g->device, 1, &f.fence);
    check(vkQueueSubmit(g->queue, 1, &info, f.fence), "a submit");
    f.submitted = true;
    if (wait) {
        vkWaitForFences(g->device, 1, &f.fence, VK_TRUE, UINT64_MAX);
        timing_results(f);
        vkResetCommandPool(g->device, f.pool, 0);
        begin_buffers();
    }
}

void flush_and_wait() {
    ++g->measures.flushes;
    submit(VK_NULL_HANDLE, VK_NULL_HANDLE, true);
}


// ---- Images ----

struct Format {
    VkFormat vk = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    int bytes = 4;  // a texel's
};

VkFormat depth_only_format() {
    for (const VkFormat f : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D24_UNORM_S8_UINT}) {
        VkFormatProperties p{};
        vkGetPhysicalDeviceFormatProperties(g->physical, f, &p);
        if ((p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
            return f;
        }
    }
    return g->depth_format;
}

Format format_of(GLenum internal) {
    switch (internal) {
        case GL_RGBA8:
        case GL_RGBA:
        case GL_RGB: return {VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 4};
        case kGlR8:
        case kGlRed: return {VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1};
        case wgl::GL_R32UI: return {VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 4};
        case wgl::GL_RGBA32UI: return {VK_FORMAT_R32G32B32A32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 16};
        case wgl::GL_RGBA32F: return {VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 16};
        case kGlRgba16f: return {VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 8};
        case kGlR11fG11fB10f:
            return {VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_IMAGE_ASPECT_COLOR_BIT, 4};
        case kGlRg32f:
            return {VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 8};
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32F: return {depth_only_format(), VK_IMAGE_ASPECT_DEPTH_BIT, 4};
        case GL_DEPTH24_STENCIL8:
        case kGlDepthStencil:
            return {g->depth_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 4};
        default: return {};
    }
}

bool has_stencil(VkFormat f) {
    return f == VK_FORMAT_D24_UNORM_S8_UINT || f == VK_FORMAT_D32_SFLOAT_S8_UINT || f == VK_FORMAT_D16_UNORM_S8_UINT;
}

int full_chain(int width, int height) {
    int levels = 1;
    for (int size = std::max(width, height); size > 1; size >>= 1) {
        ++levels;
    }
    return levels;
}

ImagePtr make_image(const Format& format, int width, int height, int levels, VkSampleCountFlagBits samples) {
    auto image = std::shared_ptr<Image>(new Image, [](Image* i) {
        if (g != nullptr && g->device != VK_NULL_HANDLE) {
            destroy_image_now(*i);
        }
        delete i;
    });
    const bool depth = (format.aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
    VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = format.vk;
    info.extent = {static_cast<std::uint32_t>(std::max(width, 1)), static_cast<std::uint32_t>(std::max(height, 1)), 1};
    info.mipLevels = static_cast<std::uint32_t>(levels);
    info.arrayLayers = 1;
    info.samples = samples;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                 | (depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    if (samples == VK_SAMPLE_COUNT_1_BIT) {
        info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    // A format nothing can draw into (32-bit integers on some phones) is only sampled.
    VkFormatProperties props{};
    vkGetPhysicalDeviceFormatProperties(g->physical, format.vk, &props);
    if (!depth && (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0) {
        info.usage &= ~VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VmaAllocationCreateInfo alloc{};
    alloc.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    if (!check(vmaCreateImage(g->allocator, &info, &alloc, &image->image, &image->allocation, nullptr), "an image")) {
        return nullptr;
    }
    image->format = format.vk;
    image->aspect = format.aspect;
    image->width = std::max(width, 1);
    image->height = std::max(height, 1);
    image->levels = levels;
    image->samples = samples;
    return image;
}

VkImageView make_view(Image& image, std::uint32_t base, std::uint32_t count, VkImageAspectFlags aspect) {
    VkImageViewCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    info.image = image.image;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = image.format;
    info.subresourceRange = {aspect, base, count, 0, 1};
    VkImageView view = VK_NULL_HANDLE;
    check(vkCreateImageView(g->device, &info, nullptr, &view), "an image view");
    return view;
}

VkImageView attachment_view(Image& image) {
    if (image.attachment == VK_NULL_HANDLE) {
        image.attachment = make_view(image, 0, 1, image.aspect);
    }
    return image.attachment;
}

// For sampling: levels base..top; a depth image's depth.
VkImageView sample_view(Image& image, int base, int top) {
    base = std::clamp(base, 0, image.levels - 1);
    top = std::clamp(top, base, image.levels - 1);
    const std::uint32_t key = static_cast<std::uint32_t>(base) | static_cast<std::uint32_t>(top) << 16;
    for (const auto& [k, view] : image.views) {
        if (k == key) {
            return view;
        }
    }
    const VkImageAspectFlags aspect =
        (image.aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0 ? VK_IMAGE_ASPECT_DEPTH_BIT : image.aspect;
    const VkImageView view =
        make_view(image, static_cast<std::uint32_t>(base), static_cast<std::uint32_t>(top - base + 1), aspect);
    image.views.emplace_back(key, view);
    return view;
}

// Pixels as the GL call gave them (format, type, the unpack alignment and row length) into the
// ring, as the image's format lays them out, tightly.
Slice stage_pixels(const Format& to, int width, int height, GLenum format, GLenum type, const void* pixels) {
    const bool float_rgba = format == GL_RGBA && type == GL_FLOAT;
    int from_bytes = to.bytes;
    if (float_rgba) {
        from_bytes = 16;
    }
    if (format == GL_RGB && type == GL_UNSIGNED_BYTE) {
        from_bytes = 3;
    }
    const int row_pixels = g->state.unpack_row_length > 0 ? g->state.unpack_row_length : width;
    const int align = std::max(g->state.unpack_alignment, 1);
    const std::size_t pitch = (static_cast<std::size_t>(row_pixels) * static_cast<std::size_t>(from_bytes) + align - 1)
                              / static_cast<std::size_t>(align) * static_cast<std::size_t>(align);
    const std::size_t row = static_cast<std::size_t>(width) * static_cast<std::size_t>(to.bytes);
    Slice s = ring(static_cast<VkDeviceSize>(row * static_cast<std::size_t>(height)));
    if (s.data == nullptr) {
        return s;
    }
    const auto* src = static_cast<const std::uint8_t*>(pixels);
    for (int y = 0; y < height; ++y) {
        const std::uint8_t* in = src + pitch * static_cast<std::size_t>(y);
        std::uint8_t* out = s.data + row * static_cast<std::size_t>(y);
        if (float_rgba
            && (to.vk == VK_FORMAT_B10G11R11_UFLOAT_PACK32 || to.vk == VK_FORMAT_R32G32_SFLOAT
                || to.vk == VK_FORMAT_R16G16B16A16_SFLOAT)) {
            for (int x = 0; x < width; ++x) {
                float rgba[4];
                std::memcpy(rgba, in + x * 16, sizeof rgba);
                if (to.vk == VK_FORMAT_B10G11R11_UFLOAT_PACK32) {
                    const std::uint32_t packed = float_encoding::pack_small_float(rgba[0], 6, false)
                                                 | (float_encoding::pack_small_float(rgba[1], 6, false) << 11)
                                                 | (float_encoding::pack_small_float(rgba[2], 5, false) << 22);
                    std::memcpy(out + x * 4, &packed, sizeof packed);
                } else if (to.vk == VK_FORMAT_R32G32_SFLOAT) {
                    std::memcpy(out + x * 8, rgba, sizeof(float) * 2);
                } else {
                    std::uint16_t packed[4];
                    for (unsigned c = 0; c < 4; ++c) {
                        packed[c] = static_cast<std::uint16_t>(float_encoding::pack_small_float(rgba[c], 10, true));
                    }
                    std::memcpy(out + x * 8, packed, sizeof packed);
                }
            }
        } else if (from_bytes == 3 && to.bytes == 4) {
            for (int x = 0; x < width; ++x) {
                out[x * 4 + 0] = in[x * 3 + 0];
                out[x * 4 + 1] = in[x * 3 + 1];
                out[x * 4 + 2] = in[x * 3 + 2];
                out[x * 4 + 3] = 0xFF;
            }
        } else {
            std::memcpy(out, in, row);
        }
    }
    return s;
}

void copy_to_image(VkCommandBuffer cmd, Image& image, const Slice& from, int level, int x, int y, int width,
                   int height) {
    if (image.layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    }
    VkBufferImageCopy copy{};
    copy.bufferOffset = from.offset;
    copy.imageSubresource = {image.aspect, static_cast<std::uint32_t>(level), 0, 1};
    copy.imageOffset = {x, y, 0};
    copy.imageExtent = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1};
    vkCmdCopyBufferToImage(cmd, from.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

// The image again with a whole mip chain, level 0 copied over: for a texture that is given a
// level past its first, or mipmaps.
ImagePtr with_chain(const ImagePtr& old) {
    const int levels = full_chain(old->width, old->height);
    if (old->levels >= levels) {
        return old;
    }
    Format f{old->format, old->aspect, 4};
    ImagePtr image = make_image(f, old->width, old->height, levels, old->samples);
    if (!image) {
        return old;
    }
    VkCommandBuffer cmd = cmd_for(old.get());
    image->touched = old->touched;
    barrier(cmd, *old, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    barrier(cmd, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkImageCopy copy{};
    copy.srcSubresource = {old->aspect, 0, 0, 1};
    copy.dstSubresource = {image->aspect, 0, 0, 1};
    copy.extent = {static_cast<std::uint32_t>(old->width), static_cast<std::uint32_t>(old->height), 1};
    vkCmdCopyImage(cmd, old->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image->image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
    retire(old);
    return image;
}

void generate_mipmaps(Image& image) {
    VkCommandBuffer cmd = cmd_for(&image);
    barrier(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    int w = image.width;
    int h = image.height;
    for (int level = 1; level < image.levels; ++level) {
        // The level above to TRANSFER_SRC, alone.
        VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        b.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        b.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.image = image.image;
        b.subresourceRange = {image.aspect, static_cast<std::uint32_t>(level - 1), 1, 0, 1};
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                             nullptr, 1, &b);
        const int nw = std::max(w / 2, 1);
        const int nh = std::max(h / 2, 1);
        VkImageBlit blit{};
        blit.srcSubresource = {image.aspect, static_cast<std::uint32_t>(level - 1), 0, 1};
        blit.srcOffsets[1] = {w, h, 1};
        blit.dstSubresource = {image.aspect, static_cast<std::uint32_t>(level), 0, 1};
        blit.dstOffsets[1] = {nw, nh, 1};
        vkCmdBlitImage(cmd, image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
        w = nw;
        h = nh;
    }
    // All levels to one layout again: the ones above the last are TRANSFER_SRC, the last DST.
    VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    b.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image.image;
    b.subresourceRange = {image.aspect, 0, static_cast<std::uint32_t>(image.levels - 1), 0, 1};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &b);
    image.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier(cmd, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

// ---- Samplers ----

VkFilter filter_of(GLenum f) {
    return f == GL_NEAREST || f == wgl::GL_NEAREST_MIPMAP_NEAREST || f == kGlNearestMipmapLinear ? VK_FILTER_NEAREST
                                                                                                  : VK_FILTER_LINEAR;
}

VkSamplerAddressMode wrap_of(GLenum w) {
    switch (w) {
        case GL_CLAMP_TO_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case kGlMirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VkSampler sampler_for(const SamplerParams& p, bool integer) {
    const std::uint64_t key = static_cast<std::uint64_t>(p.min - 0x2600) | static_cast<std::uint64_t>(p.mag - 0x2600) << 8
                              | static_cast<std::uint64_t>(wrap_of(p.wrap_s)) << 16
                              | static_cast<std::uint64_t>(wrap_of(p.wrap_t)) << 20
                              | static_cast<std::uint64_t>(std::clamp(p.anisotropy, 1.0f, 64.0f) * 4.0f) << 24
                              | static_cast<std::uint64_t>(integer) << 40;
    for (const auto& [k, s] : g->vk_samplers) {
        if (k == key) {
            return s;
        }
    }
    const bool mipmapped = p.min != GL_NEAREST && p.min != GL_LINEAR;
    VkSamplerCreateInfo info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    // An integer texture is read with nearest filtering only.
    info.magFilter = integer ? VK_FILTER_NEAREST : filter_of(p.mag);
    info.minFilter = integer ? VK_FILTER_NEAREST : filter_of(p.min);
    info.mipmapMode = p.min == wgl::GL_LINEAR_MIPMAP_NEAREST || p.min == wgl::GL_NEAREST_MIPMAP_NEAREST || !mipmapped
                          ? VK_SAMPLER_MIPMAP_MODE_NEAREST
                          : VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.addressModeU = wrap_of(p.wrap_s);
    info.addressModeV = wrap_of(p.wrap_t);
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.minLod = 0.0f;
    // GL's NEAREST and LINEAR minification read the base level only.
    info.maxLod = mipmapped ? VK_LOD_CLAMP_NONE : 0.25f;
    if (p.anisotropy > 1.0f && g->features.samplerAnisotropy && mipmapped && !integer) {
        info.anisotropyEnable = VK_TRUE;
        info.maxAnisotropy = std::min(p.anisotropy, g->properties.limits.maxSamplerAnisotropy);
    }
    VkSampler sampler = VK_NULL_HANDLE;
    check(vkCreateSampler(g->device, &info, nullptr, &sampler), "a sampler");
    g->vk_samplers.emplace_back(key, sampler);
    return sampler;
}

// ---- Buffers on the GPU ----

struct BufferRef {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
};

BufferRef dummy_buffer_ref() {
    return {g->dummy_buffer, 0, 256};
}

BufferRef resolve(Buffer& b, bool persistent = true) {
    if (b.data.empty()) {
        return dummy_buffer_ref();
    }
    const VkDeviceSize size = b.data.size();
    if (persistent && b.own != VK_NULL_HANDLE && b.own_version == b.version) {
        return {b.own, 0, size};
    }
    if (b.gpu != VK_NULL_HANDLE && b.gpu_frame == g->frame_count && b.gpu_version == b.version) {
        return {b.gpu, b.gpu_offset, size};
    }
    // Unchanged for a few frames: a copy of its own, which stays where it is.
    if (b.storage || (persistent && g->frame_count >= b.changed_frame + 3 && size >= 4096)) {
        VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        info.size = size;
        info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                     | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
                     | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc{};
        alloc.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        alloc.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        alloc.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;
        VmaAllocationInfo got{};
        if (vmaCreateBuffer(g->allocator, &info, &alloc, &buffer, &allocation, &got) == VK_SUCCESS) {
            std::memcpy(got.pMappedData, b.data.data(), b.data.size());
            if (b.own != VK_NULL_HANDLE) {
                defer([buffer = b.own, allocation = b.own_allocation] {
                    vmaDestroyBuffer(g->allocator, buffer, allocation);
                });
            }
            b.own = buffer;
            b.own_allocation = allocation;
            b.own_version = b.version;
            return {b.own, 0, size};
        }
    }
    const Slice s = ring_copy(b.data.data(), size);
    b.gpu = s.buffer;
    b.gpu_offset = s.offset;
    b.gpu_version = b.version;
    b.gpu_frame = g->frame_count;
    return {s.buffer, s.offset, size};
}

void forget_views(Buffer& b) {
    for (const Buffer::View& v : b.views) {
        defer([view = v.view] { vkDestroyBufferView(g->device, view, nullptr); });
    }
    b.views.clear();
}

VkBufferView texel_view(GLuint name, GLenum gl_format, VkBufferView fallback) {
    VkFormat format = VK_FORMAT_R32_UINT;
    int texel = 4;
    if (gl_format == wgl::GL_RGBA32UI) {
        format = VK_FORMAT_R32G32B32A32_UINT;
        texel = 16;
    } else if (gl_format == wgl::GL_RGBA32F) {
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
        texel = 16;
    } else if (gl_format == GL_RGBA8) {
        // Compute writes packed UNORM bytes; material shaders read RGBA8.
        format = VK_FORMAT_R8G8B8A8_UNORM;
    }
    Buffer* b = g->buffers.find(name);
    if (b == nullptr || b->data.size() < static_cast<std::size_t>(texel)) {
        return fallback;
    }
    if (b->views_frame != g->frame_count) {
        forget_views(*b);
        b->views_frame = g->frame_count;
    }
    const BufferRef r = resolve(*b);
    for (const Buffer::View& v : b->views) {
        if (v.format == format && v.buffer == r.buffer && v.offset == r.offset) {
            return v.view;
        }
    }
    VkBufferViewCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
    info.buffer = r.buffer;
    info.format = format;
    info.offset = r.offset;
    const VkDeviceSize most =
        static_cast<VkDeviceSize>(g->properties.limits.maxTexelBufferElements) * texel;
    info.range = std::min<VkDeviceSize>(r.size / texel * texel, most);
    VkBufferView view = VK_NULL_HANDLE;
    if (!check(vkCreateBufferView(g->device, &info, nullptr, &view), "a buffer view")) {
        return fallback;
    }
    b->views.push_back({format, r.buffer, r.offset, view});
    return view;
}

// ---- Where drawing goes ----

Image* image_of(GLuint texture, GLuint renderbuffer) {
    if (Texture* t = g->textures.find(texture); t != nullptr && t->image) {
        return t->image.get();
    }
    if (Renderbuffer* r = g->renderbuffers.find(renderbuffer); r != nullptr && r->image) {
        return r->image.get();
    }
    return nullptr;
}

Target target_of(GLuint name) {
    Target t;
    if (name == 0) {
        t.colour = g->default_colour.get();
        t.depth = g->default_depth.get();
    } else if (Framebuffer* fb = g->framebuffers.find(name)) {
        t.colour = image_of(fb->colour_texture, fb->colour_renderbuffer);
        t.depth = image_of(fb->depth_texture, fb->depth_renderbuffer);
    }
    if (Image* any = t.colour != nullptr ? t.colour : t.depth) {
        t.width = any->width;
        t.height = any->height;
        t.samples = any->samples;
    }
    return t;
}

struct Clear {
    bool colour = false, depth = false, stencil = false;
};

void begin_rendering(const Target& t, const Clear& clear) {
    end_rendering();
    VkCommandBuffer cmd = frame().main;
    VkRenderingAttachmentInfo colour{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingAttachmentInfo depth{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingAttachmentInfo stencil{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
    info.renderArea = {{0, 0}, {static_cast<std::uint32_t>(t.width), static_cast<std::uint32_t>(t.height)}};
    info.layerCount = 1;
    const State& s = g->state;
    if (t.colour != nullptr) {
        t.colour->touched = g->recording;
        // Also when it is in that layout already: what an earlier pass wrote is then done.
        barrier(cmd, *t.colour, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        colour.imageView = attachment_view(*t.colour);
        colour.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colour.loadOp = clear.colour ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        colour.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        std::memcpy(colour.clearValue.color.float32, s.clear_colour, sizeof s.clear_colour);
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &colour;
    }
    if (t.depth != nullptr) {
        t.depth->touched = g->recording;
        barrier(cmd, *t.depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        depth.imageView = attachment_view(*t.depth);
        depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth.loadOp = clear.depth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth.clearValue.depthStencil = {s.clear_depth, static_cast<std::uint32_t>(s.clear_stencil)};
        info.pDepthAttachment = &depth;
        if (has_stencil(t.depth->format)) {
            stencil = depth;
            stencil.loadOp = clear.stencil ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            info.pStencilAttachment = &stencil;
        }
    }
    vkCmdBeginRendering(cmd, &info);
    g->rendering = true;
    g->rendering_target = t;
    g->bound_pipeline = VK_NULL_HANDLE;
}

void ensure_rendering(const Target& t) {
    if (!g->rendering || !(g->rendering_target == t)) {
        begin_rendering(t, Clear{});
    }
}

VkRect2D scissor_rect(const Target& t) {
    const State& s = g->state;
    int x0 = 0, y0 = 0, x1 = t.width, y1 = t.height;
    if (s.scissor_test) {
        x0 = std::clamp(s.scissor[0], 0, t.width);
        y0 = std::clamp(s.scissor[1], 0, t.height);
        x1 = std::clamp(s.scissor[0] + std::max(s.scissor[2], 0), x0, t.width);
        y1 = std::clamp(s.scissor[1] + std::max(s.scissor[3], 0), y0, t.height);
    }
    return {{x0, y0}, {static_cast<std::uint32_t>(x1 - x0), static_cast<std::uint32_t>(y1 - y0)}};
}

void clear(GLbitfield mask) {
    gpu_flush_draws();
    const Target t = target_of(g->state.draw_framebuffer);
    const State& s = g->state;
    Clear c;
    c.colour = t.colour != nullptr && (mask & GL_COLOR_BUFFER_BIT) != 0
               && (s.colour_mask[0] || s.colour_mask[1] || s.colour_mask[2] || s.colour_mask[3]);
    c.depth = t.depth != nullptr && (mask & GL_DEPTH_BUFFER_BIT) != 0 && s.depth_mask;
    c.stencil = t.depth != nullptr && has_stencil(t.depth->format) && (mask & GL_STENCIL_BUFFER_BIT) != 0
                && (s.stencil_write_mask & 0xFF) != 0;
    if (!c.colour && !c.depth && !c.stencil) {
        return;
    }
    const VkRect2D rect = scissor_rect(t);
    if (rect.extent.width == 0 || rect.extent.height == 0) {
        return;
    }
    const bool whole = rect.offset.x == 0 && rect.offset.y == 0 && static_cast<int>(rect.extent.width) == t.width
                       && static_cast<int>(rect.extent.height) == t.height;
    if (whole && (!g->rendering || !(g->rendering_target == t))) {
        begin_rendering(t, c);  // cleared as the drawing begins: no work of its own on a tiler
        return;
    }
    ensure_rendering(t);
    VkClearAttachment attachments[2]{};
    std::uint32_t n = 0;
    if (c.colour) {
        attachments[n].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        attachments[n].colorAttachment = 0;
        std::memcpy(attachments[n].clearValue.color.float32, s.clear_colour, sizeof s.clear_colour);
        ++n;
    }
    if (c.depth || c.stencil) {
        attachments[n].aspectMask =
            (c.depth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0u) | (c.stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0u);
        attachments[n].clearValue.depthStencil = {s.clear_depth, static_cast<std::uint32_t>(s.clear_stencil)};
        ++n;
    }
    VkClearRect r{rect, 0, 1};
    vkCmdClearAttachments(frame().main, n, attachments, 1, &r);
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
            return pick(VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT,
                        VK_FORMAT_R32G32B32A32_SFLOAT);
        case kGlHalfFloat:
            return pick(VK_FORMAT_R16_SFLOAT, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R16G16B16_SFLOAT,
                        VK_FORMAT_R16G16B16A16_SFLOAT);
        case GL_UNSIGNED_BYTE:
            if (integer) {
                return pick(VK_FORMAT_R8_UINT, VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8A8_UINT);
            }
            return normalized ? pick(VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM,
                                     VK_FORMAT_R8G8B8A8_UNORM)
                              : pick(VK_FORMAT_R8_USCALED, VK_FORMAT_R8G8_USCALED, VK_FORMAT_R8G8B8_USCALED,
                                     VK_FORMAT_R8G8B8A8_USCALED);
        case kGlByte:
            if (integer) {
                return pick(VK_FORMAT_R8_SINT, VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8A8_SINT);
            }
            return normalized ? pick(VK_FORMAT_R8_SNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8B8_SNORM,
                                     VK_FORMAT_R8G8B8A8_SNORM)
                              : pick(VK_FORMAT_R8_SSCALED, VK_FORMAT_R8G8_SSCALED, VK_FORMAT_R8G8B8_SSCALED,
                                     VK_FORMAT_R8G8B8A8_SSCALED);
        case GL_UNSIGNED_SHORT:
            if (integer) {
                return pick(VK_FORMAT_R16_UINT, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16B16_UINT,
                            VK_FORMAT_R16G16B16A16_UINT);
            }
            return normalized ? pick(VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM,
                                     VK_FORMAT_R16G16B16A16_UNORM)
                              : pick(VK_FORMAT_R16_USCALED, VK_FORMAT_R16G16_USCALED, VK_FORMAT_R16G16B16_USCALED,
                                     VK_FORMAT_R16G16B16A16_USCALED);
        case GL_SHORT:
            if (integer) {
                return pick(VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16B16_SINT,
                            VK_FORMAT_R16G16B16A16_SINT);
            }
            return normalized ? pick(VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16_SNORM,
                                     VK_FORMAT_R16G16B16A16_SNORM)
                              : pick(VK_FORMAT_R16_SSCALED, VK_FORMAT_R16G16_SSCALED, VK_FORMAT_R16G16B16_SSCALED,
                                     VK_FORMAT_R16G16B16A16_SSCALED);
        case GL_UNSIGNED_INT:
            return pick(VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT,
                        VK_FORMAT_R32G32B32A32_UINT);
        case GL_INT:
            return pick(VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT,
                        VK_FORMAT_R32G32B32A32_SINT);
        default: return VK_FORMAT_UNDEFINED;
    }
}

int type_bytes(GLenum type) {
    switch (type) {
        case GL_UNSIGNED_BYTE:
        case kGlByte: return 1;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case kGlHalfFloat: return 2;
        default: return 4;
    }
}

// The kind of number a shader input of this GL type holds: 0 float, 1 unsigned, 2 signed.
int input_kind(std::uint32_t gl_type) {
    switch (gl_type) {
        case GL_UNSIGNED_INT:
        case 0x8DC6:
        case 0x8DC7:
        case 0x8DC8: return 1;
        case GL_INT:
        case 0x8B53:
        case 0x8B54:
        case 0x8B55: return 2;
        default: return 0;
    }
}

constexpr VkDeviceSize kDefaultFloat = 256;  // in the dummy buffer: (0, 0, 0, 1) as floats
constexpr VkDeviceSize kDefaultInteger = 272;  // and as integers

VkBlendFactor blend_factor(GLenum f) {
    switch (f) {
        case GL_ZERO: return VK_BLEND_FACTOR_ZERO;
        case GL_ONE: return VK_BLEND_FACTOR_ONE;
        case GL_SRC_COLOR: return VK_BLEND_FACTOR_SRC_COLOR;
        case GL_ONE_MINUS_SRC_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case GL_SRC_ALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
        case GL_ONE_MINUS_SRC_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case GL_DST_ALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
        case GL_ONE_MINUS_DST_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case GL_DST_COLOR: return VK_BLEND_FACTOR_DST_COLOR;
        case GL_ONE_MINUS_DST_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case GL_CONSTANT_COLOR: return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case GL_ONE_MINUS_CONSTANT_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case GL_CONSTANT_ALPHA: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case GL_ONE_MINUS_CONSTANT_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case 0x0308: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        default: return VK_BLEND_FACTOR_ONE;
    }
}

VkBlendOp blend_op(GLenum e) {
    switch (e) {
        case GL_FUNC_SUBTRACT: return VK_BLEND_OP_SUBTRACT;
        case GL_FUNC_REVERSE_SUBTRACT: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case 0x8007: return VK_BLEND_OP_MIN;
        case 0x8008: return VK_BLEND_OP_MAX;
        default: return VK_BLEND_OP_ADD;
    }
}

VkCompareOp compare_op(GLenum f) {
    switch (f) {
        case GL_NEVER: return VK_COMPARE_OP_NEVER;
        case GL_LESS: return VK_COMPARE_OP_LESS;
        case GL_EQUAL: return VK_COMPARE_OP_EQUAL;
        case GL_LEQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case GL_GREATER: return VK_COMPARE_OP_GREATER;
        case GL_NOTEQUAL: return VK_COMPARE_OP_NOT_EQUAL;
        case GL_GEQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        default: return VK_COMPARE_OP_ALWAYS;
    }
}

VkStencilOp stencil_op(GLenum op) {
    switch (op) {
        case GL_ZERO: return VK_STENCIL_OP_ZERO;
        case wgl::GL_REPLACE: return VK_STENCIL_OP_REPLACE;
        case 0x1E02: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case 0x1E03: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case 0x150A: return VK_STENCIL_OP_INVERT;
        case wgl::GL_INCR_WRAP: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case wgl::GL_DECR_WRAP: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default: return VK_STENCIL_OP_KEEP;
    }
}

VkPrimitiveTopology topology_of(GLenum mode) {
    switch (mode) {
        case GL_POINTS: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case GL_LINES: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case GL_LINE_STRIP: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case GL_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case GL_TRIANGLE_FAN: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

struct Input {
    std::uint32_t location, format, stride, instanced;
};

// Everything a pipeline bakes in, as plain numbers: the cache's key.
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

VkPipeline pipeline_for(Program& p, GLenum mode, const Target& t, const Input* inputs, std::uint32_t count) {
    const State& s = g->state;
    PipelineKey k;
    std::memset(&k, 0, sizeof k);
    k.program = p.serial;
    k.topology = static_cast<std::uint32_t>(topology_of(mode));
    const bool strip = mode == GL_LINE_STRIP || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN;
    k.restart = s.primitive_restart && strip ? 1 : 0;
    k.polygon = s.polygon_mode == GL_LINE && g->features.fillModeNonSolid ? 1 : 0;
    k.cull = !s.cull                          ? VK_CULL_MODE_NONE
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
        k.colour_mask = (s.colour_mask[0] ? 1u : 0u) | (s.colour_mask[1] ? 2u : 0u) | (s.colour_mask[2] ? 4u : 0u)
                        | (s.colour_mask[3] ? 8u : 0u);
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
    std::string key(reinterpret_cast<const char*>(&k), sizeof k);
    if (auto it = g->pipelines.find(key); it != g->pipelines.end()) {
        return it->second;
    }

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT,
                 p.vertex, "main", nullptr};
    stages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT,
                 p.fragment, "main", nullptr};
    VkVertexInputBindingDescription bindings[kAttributes]{};
    VkVertexInputAttributeDescription attributes[kAttributes]{};
    for (std::uint32_t i = 0; i < count; ++i) {
        bindings[i] = {i, inputs[i].stride,
                       inputs[i].instanced != 0 ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX};
        attributes[i] = {inputs[i].location, i, static_cast<VkFormat>(inputs[i].format), 0};
    }
    VkPipelineVertexInputStateCreateInfo vertex{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertex.vertexBindingDescriptionCount = count;
    vertex.pVertexBindingDescriptions = bindings;
    vertex.vertexAttributeDescriptionCount = count;
    vertex.pVertexAttributeDescriptions = attributes;
    VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    assembly.topology = static_cast<VkPrimitiveTopology>(k.topology);
    assembly.primitiveRestartEnable = k.restart;
    VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewport.viewportCount = k.viewports;
    viewport.scissorCount = k.viewports;
    VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.polygonMode = k.polygon != 0 ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    raster.cullMode = k.cull;
    raster.frontFace = static_cast<VkFrontFace>(k.front);
    raster.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisample.rasterizationSamples = t.samples;
    VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depth.depthTestEnable = k.depth_test;
    depth.depthWriteEnable = k.depth_write;
    depth.depthCompareOp = k.depth_test != 0 ? static_cast<VkCompareOp>(k.depth_func) : VK_COMPARE_OP_ALWAYS;
    depth.stencilTestEnable = k.stencil;
    depth.front = {static_cast<VkStencilOp>(k.front_ops[0]), static_cast<VkStencilOp>(k.front_ops[1]),
                   static_cast<VkStencilOp>(k.front_ops[2]), static_cast<VkCompareOp>(k.stencil_func), 0, 0, 0};
    depth.back = {static_cast<VkStencilOp>(k.back_ops[0]), static_cast<VkStencilOp>(k.back_ops[1]),
                  static_cast<VkStencilOp>(k.back_ops[2]), static_cast<VkCompareOp>(k.stencil_func), 0, 0, 0};
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
    VkPipelineColorBlendStateCreateInfo blending{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blending.attachmentCount = t.colour != nullptr ? 1 : 0;
    blending.pAttachments = &blend;
    const VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,           VK_DYNAMIC_STATE_SCISSOR,
                                             VK_DYNAMIC_STATE_BLEND_CONSTANTS,    VK_DYNAMIC_STATE_STENCIL_REFERENCE,
                                             VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK};
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
    if (!check(vkCreateGraphicsPipelines(g->device, g->pipeline_cache, 1, &info, nullptr, &pipeline), "a pipeline")) {
        pipeline = VK_NULL_HANDLE;
    }
    g->pipelines.emplace(std::move(key), pipeline);
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
            return set;
        }
        ++f.pool_at;
    }
}

// What a sampler uniform reads now: its image (null for a buffer texture) and the descriptor.
struct Sampled {
    Image* image = nullptr;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkBufferView texels = VK_NULL_HANDLE;
};

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
    if (const SamplerParams* own = g->samplers.find(s.unit_sampler[static_cast<std::size_t>(unit)])) {
        params = *own;
    }
    const bool integer = image->format == VK_FORMAT_R32_UINT || image->format == VK_FORMAT_R32G32B32A32_UINT;
    out.image = image;
    out.view = sample_view(*image, t != nullptr ? t->base_level : 0, t != nullptr ? t->max_level : 0);
    out.sampler = sampler_for(params, integer);
    return out;
}

BufferRef block_buffer(GLuint binding, int size) {
    Buffer* b = binding < kUniformBindings ? g->buffers.find(g->state.uniform_bindings[binding]) : nullptr;
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
        write(c.blocks[i].binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).pBufferInfo = &buffer_infos.back();
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
            write(u.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER).pImageInfo = &image_infos.back();
        }
    }
    vkUpdateDescriptorSets(g->device, static_cast<std::uint32_t>(writes.size()), writes.data(), 0, nullptr);
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
    vkCmdDispatch(frame().main, x, y, z);
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
}

// ---- Draws ----

VertexArray g_default_vertex_array;  // vertex array 0

VertexArray& vertex_array() {
    if (VertexArray* va = g->vertex_arrays.find(g->state.vertex_array)) {
        return *va;
    }
    return g_default_vertex_array;
}

#include "gpu_materials.inc"

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
) {
    struct Timed {
        Clock::time_point start = Clock::now();
        ~Timed() {
            g->measures.draw_s += seconds_since(start);
            ++g->measures.draws;
        }
    } timed;
    Program* p = g->programs.find(g->state.program);
    if (p == nullptr || !p->linked || count <= 0 || instances <= 0) {
        return;
    }
    const Target t = target_of(g->state.draw_framebuffer);
    if ((t.colour == nullptr && t.depth == nullptr) || t.width <= 0 || t.height <= 0) {
        return;
    }
    const State& s = g->state;
    if (s.viewport[2] <= 0 || s.viewport[3] <= 0) {
        return;
    }
    // What it samples, in the layout for sampling: moved before the drawing begins.
    std::vector<Sampled> samplers;
    for (std::size_t i = 0; i < p->compiled.uniforms.size(); ++i) {
        if (p->compiled.uniforms[i].sampler && p->compiled.uniforms[i].binding >= 0) {
            samplers.push_back(sampled(*p, i));
        }
    }
    for (Sampled& sm : samplers) {
        Image* image = sm.image;
        if (image != nullptr && (image == t.colour || image == t.depth)) {
            // A texture that is also what the draw writes, left bound to a unit the draw does not
            // read (the 2D path's untextured quads keep the frame's texture on unit 0): GL leaves
            // a read of it undefined, and Vulkan cannot describe an attachment as sampled. GL's
            // incomplete texture stands in.
            sm.image = g->dummy_image.get();
            sm.view = sample_view(*sm.image, 0, 0);
            continue;
        }
        if (image == nullptr) {
            continue;
        }
        if (image->layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            end_rendering();
            barrier(frame().main, *image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        image->touched = g->recording;
    }

    // The vertex inputs: each from its own binding, or GL's (0, 0, 0, 1).
    const VertexArray& va = vertex_array();
    Input inputs[kAttributes];
    VkBuffer buffers[kAttributes];
    VkDeviceSize offsets[kAttributes];
    std::uint32_t n = 0;
    for (const CompiledProgram::Attribute& a : p->compiled.attributes) {
        if (a.location < 0 || a.location >= kAttributes || n >= kAttributes) {
            continue;
        }
        const Attribute& attr = va.attributes[static_cast<std::size_t>(a.location)];
        Buffer* b = attr.enabled ? g->buffers.find(attr.buffer) : nullptr;
        const VkFormat format = attr.enabled ? vertex_format(attr.type, attr.size, attr.normalized, attr.integer)
                                             : VK_FORMAT_UNDEFINED;
        if (b != nullptr && format != VK_FORMAT_UNDEFINED) {
            // Small persistent instance buffers corrupt vertex fetches on the
            // AMD Windows driver. Keep instance data in the frame ring; large
            // per-vertex and index buffers can still use persistent storage.
            const BufferRef r = gpu_page && attr.divisor
                                    ? BufferRef{gpu_page->output.buffer, 0, gpu_page->output.size}
                                    : resolve(*b, attr.divisor == 0);
            const GLsizei stride = attr.stride != 0 ? attr.stride : attr.size * type_bytes(attr.type);
            inputs[n] = {static_cast<std::uint32_t>(a.location), static_cast<std::uint32_t>(format),
                         static_cast<std::uint32_t>(stride), attr.divisor != 0 ? 1u : 0u};
            buffers[n] = r.buffer;
            offsets[n] = r.offset + attr.offset;
        } else {
            const int kind = input_kind(a.gl_type);
            inputs[n] = {static_cast<std::uint32_t>(a.location),
                         static_cast<std::uint32_t>(kind == 0   ? VK_FORMAT_R32G32B32A32_SFLOAT
                                                    : kind == 1 ? VK_FORMAT_R32G32B32A32_UINT
                                                                : VK_FORMAT_R32G32B32A32_SINT),
                         0u, 0u};
            buffers[n] = g->dummy_buffer;
            offsets[n] = kind == 0 ? kDefaultFloat : kDefaultInteger;
        }
        ++n;
    }

    const VkPipeline pipeline = pipeline_for(*p, mode, t, inputs, n);
    if (pipeline == VK_NULL_HANDLE) {
        return;
    }
    const auto material = material_binding(*p, t);
    const VkDescriptorSet set = descriptors(*p, samplers);
    BufferRef index{};
    VkIndexType vk_index = VK_INDEX_TYPE_UINT32;
    std::uint32_t first_index = 0;
    if (indexed) {
        Buffer* eb = g->buffers.find(va.elements);
        if (eb == nullptr || eb->data.empty()) {
            return;
        }
        if (index_type == GL_UNSIGNED_BYTE) {
            // Vulkan's 8-bit indices are an extension: made 16-bit.
            const std::size_t from = std::min<std::size_t>(index_offset, eb->data.size());
            const std::size_t have = std::min<std::size_t>(static_cast<std::size_t>(count), eb->data.size() - from);
            Slice sl = ring(static_cast<VkDeviceSize>(have * 2));
            auto* out = reinterpret_cast<std::uint16_t*>(sl.data);
            for (std::size_t i = 0; i < have; ++i) {
                const std::uint8_t v = eb->data[from + i];
                out[i] = v == 0xFF ? 0xFFFF : v;
            }
            index = {sl.buffer, sl.offset, have * 2};
            vk_index = VK_INDEX_TYPE_UINT16;
            count = static_cast<GLsizei>(have);
        } else {
            index = resolve(*eb);
            const std::uint32_t bytes = index_type == GL_UNSIGNED_SHORT ? 2 : 4;
            vk_index = bytes == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
            first_index = static_cast<std::uint32_t>(index_offset / bytes);
        }
    }

    ensure_rendering(t);
    VkCommandBuffer cmd = frame().main;
    GpuEngine::Bindings gpu_bindings;
    VkDeviceSize argument_offset = VkDeviceSize(kGpuPageGroups + 5 * gpu_command) * 4;
    if (gpu_page) {
        gpu_bindings.pipeline = pipeline;
        gpu_bindings.descriptors = set;
        gpu_bindings.materials = material.set;
        gpu_bindings.dynamic_offset = static_cast<std::uint32_t>(p->uniforms_offset);
        gpu_bindings.inputs = n;
        std::copy_n(buffers, n, gpu_bindings.vertices.begin());
        std::copy_n(offsets, n, gpu_bindings.offsets.begin());
        gpu_bindings.index = index.buffer;
        gpu_bindings.index_offset = index.offset;
        gpu_bindings.index_type = vk_index;
        std::copy_n(s.viewport, 4, gpu_bindings.viewport.begin());
        const auto sc = scissor_rect(t);
        gpu_bindings
            .scissor = {sc.offset.x, sc.offset.y, int(sc.extent.width), int(sc.extent.height)};
        std::copy_n(s.blend_colour, 4, gpu_bindings.blend.begin());
        gpu_bindings.stencil =
            {static_cast<unsigned>(s.stencil_ref), s.stencil_value_mask, s.stencil_write_mask};
        auto& pending = g->gpu;
        if (stats::enabled() && pending.pending_count) {
            const auto& before = pending.pending_bindings;
            const auto& after = gpu_bindings;
            const bool reasons[] =
                {before.pipeline != after.pipeline,
                 before.descriptors != after.descriptors,
                 before.dynamic_offset != after.dynamic_offset,
                 before.vertices != after.vertices || before.offsets != after.offsets,
                 before.index != after.index || before.index_offset != after.index_offset,
                 before.materials != after.materials,
                 before.viewport != after.viewport || before.scissor != after.scissor,
                 before.blend != after.blend || before.stencil != after.stencil,
                 pending.pending_arguments != gpu_page->arguments.buffer
                     || argument_offset != pending.pending_offset + pending.pending_count * 20,
                 material.set
                     && material.draw != pending.pending_material_base + pending.pending_count};
            for (std::size_t i = 0; i < pending.batch_breaks.size(); ++i) {
                pending.batch_breaks[i] += reasons[i] ? 1u : 0u;
            }
        }
        if (g->features.multiDrawIndirect && pending.pending_count
            && pending.pending_count < g->properties.limits.maxDrawIndirectCount
            && pending.pending_arguments == gpu_page->arguments.buffer
            && argument_offset == pending.pending_offset + pending.pending_count * 20
            && pending.pending_bindings == gpu_bindings
            && (!material.set
                || material.draw == pending.pending_material_base + pending.pending_count)) {
            ++pending.pending_count;
            ++pending.merged;
            return;
        }
    }
    gpu_flush_draws();
    if (pipeline != g->bound_pipeline) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        g->bound_pipeline = pipeline;
    }
    if (set != VK_NULL_HANDLE) {
        const auto dynamic_offset = static_cast<std::uint32_t>(p->uniforms_offset);
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            p->layout,
            0,
            1,
            &set,
            p->dynamic_uniforms ? 1u : 0u,
            p->dynamic_uniforms ? &dynamic_offset : nullptr
        );
    }
    if (material.set) {
        vkCmdBindDescriptorSets(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p->layout, 1, 1, &material.set, 0, nullptr
        );
        vkCmdPushConstants(cmd, p->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 4, &material.draw);
    }
    if (n > 0) {
        vkCmdBindVertexBuffers(cmd, 0, n, buffers, offsets);
    }
    VkViewport viewport{static_cast<float>(s.viewport[0]), static_cast<float>(s.viewport[1]),
                        static_cast<float>(s.viewport[2]), static_cast<float>(s.viewport[3]), 0.0f, 1.0f};
    const VkRect2D scissor = scissor_rect(t);
    if (g->indexed_viewports.empty()) {
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    } else {
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;
        for (const auto& r : g->indexed_viewports) {
            viewports.push_back({float(r[0]), float(r[1]), float(r[2]), float(r[3]), 0, 1});
            scissors.push_back({{r[0], r[1]}, {unsigned(r[2]), unsigned(r[3])}});
        }
        vkCmdSetViewport(cmd, 0, static_cast<std::uint32_t>(viewports.size()), viewports.data());
        vkCmdSetScissor(cmd, 0, static_cast<std::uint32_t>(scissors.size()), scissors.data());
    }
    vkCmdSetBlendConstants(cmd, s.blend_colour);
    vkCmdSetStencilReference(cmd, VK_STENCIL_FACE_FRONT_AND_BACK, static_cast<std::uint32_t>(s.stencil_ref));
    vkCmdSetStencilCompareMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK, s.stencil_value_mask);
    vkCmdSetStencilWriteMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK, s.stencil_write_mask);
    if (indexed) {
        vkCmdBindIndexBuffer(cmd, index.buffer, index.offset, vk_index);
        if (gpu_page) {
            g->gpu.pending_bindings = gpu_bindings;
            g->gpu.pending_arguments = gpu_page->arguments.buffer;
            g->gpu.pending_offset = argument_offset;
            g->gpu.pending_count = 1;
            g->gpu.pending_material_base = material.draw;
        } else {
            vkCmdDrawIndexed(
                cmd,
                static_cast<std::uint32_t>(count),
                static_cast<std::uint32_t>(instances),
                first_index,
                0,
                0
            );
            ++g->measures.native_draws;
        }
    } else {
        vkCmdDraw(cmd, static_cast<std::uint32_t>(count), static_cast<std::uint32_t>(instances),
                  static_cast<std::uint32_t>(first), 0);
        ++g->measures.native_draws;
    }
}

#include "gpu_draw.inc"

// ---- The GL functions ----

#define VKGL OPENRAC_GLAPI

const GLubyte* VKGL get_string(GLenum name) {
    static const char vendor[] = "OpenRAC";
    static const char glsl[] = "4.10";
    switch (name) {
        case GL_VENDOR: return reinterpret_cast<const GLubyte*>(vendor);
        case GL_RENDERER: return reinterpret_cast<const GLubyte*>(g->gl_renderer.c_str());
        case GL_VERSION: return reinterpret_cast<const GLubyte*>(g->gl_version.c_str());
        case GL_SHADING_LANGUAGE_VERSION: return reinterpret_cast<const GLubyte*>(glsl);
        default: fail(kGlInvalidEnum); return nullptr;
    }
}

const GLubyte* VKGL get_stringi(GLenum, GLuint) {
    return nullptr;  // no extensions: none of the GL ones the renderer looks for applies
}

void VKGL get_integerv(GLenum name, GLint* data) {
    const VkPhysicalDeviceLimits& l = g->properties.limits;
    switch (name) {
        case GL_MAX_TEXTURE_SIZE: *data = static_cast<GLint>(l.maxImageDimension2D); return;
        case GL_MAX_SAMPLES: {
            const VkSampleCountFlags counts = l.framebufferColorSampleCounts & l.framebufferDepthSampleCounts;
            GLint most = 1;
            for (GLint c = 64; c > 1; c >>= 1) {
                if ((counts & static_cast<VkSampleCountFlags>(c)) != 0) {
                    most = c;
                    break;
                }
            }
            *data = most;
            return;
        }
        case GL_NUM_EXTENSIONS: *data = 0; return;
        case GL_MAJOR_VERSION: *data = 4; return;
        case GL_MINOR_VERSION: *data = 1; return;
        case 0x90DD:
            *data = kUniformBindings;
            return;  // GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS
        case kGlUniformBufferOffsetAlignment: *data = static_cast<GLint>(l.minUniformBufferOffsetAlignment); return;
        case kGlMaxCombinedTextureUnits: *data = kUnits; return;
        case kGlMaxTextureBufferSize: *data = static_cast<GLint>(std::min<std::uint32_t>(l.maxTexelBufferElements, 0x7FFFFFFF)); return;
        case kGlMaxVertexAttribs: *data = kAttributes; return;
        case kGlMaxUniformBlockSize: *data = static_cast<GLint>(std::min<std::uint32_t>(l.maxUniformBufferRange, 0x7FFFFFFF)); return;
        case kGlFramebufferBinding: *data = static_cast<GLint>(g->state.draw_framebuffer); return;
        case kGlTextureBindingSlot: *data = static_cast<GLint>(g->state.unit_2d[g->state.active_unit]); return;
        case kGlViewportQuery: std::memcpy(data, g->state.viewport, sizeof g->state.viewport); return;
        default: *data = 0; return;
    }
}

void VKGL get_floatv(GLenum name, GLfloat* data) {
    if (name == GL_MAX_TEXTURE_MAX_ANISOTROPY) {
        *data = g->features.samplerAnisotropy ? g->properties.limits.maxSamplerAnisotropy : 1.0f;
        return;
    }
    GLint v = 0;
    get_integerv(name, &v);
    *data = static_cast<GLfloat>(v);
}

GLenum VKGL get_error() {
    return std::exchange(g->error, GL_NO_ERROR);
}

void enable_cap(GLenum cap, bool on) {
    State& s = g->state;
    switch (cap) {
        case GL_BLEND: s.blend = on; break;
        case GL_DEPTH_TEST: s.depth_test = on; break;
        case GL_CULL_FACE: s.cull = on; break;
        case GL_SCISSOR_TEST: s.scissor_test = on; break;
        case wgl::GL_STENCIL_TEST: s.stencil_test = on; break;
        case GL_PRIMITIVE_RESTART: s.primitive_restart = on; break;
        // Clip distances are always on in Vulkan for a shader that writes them; multisampling is
        // the target's.
        default: break;
    }
}

void VKGL enable(GLenum cap) {
    enable_cap(cap, true);
}

void VKGL disable(GLenum cap) {
    enable_cap(cap, false);
}

void VKGL clear_gl(GLbitfield mask) {
    clear(mask);
}

void VKGL clear_color(GLfloat r, GLfloat gr, GLfloat b, GLfloat a) {
    g->state.clear_colour[0] = r;
    g->state.clear_colour[1] = gr;
    g->state.clear_colour[2] = b;
    g->state.clear_colour[3] = a;
}

void VKGL clear_depth(GLdouble d) {
    g->state.clear_depth = static_cast<float>(std::clamp(d, 0.0, 1.0));
}

void VKGL viewport(GLint x, GLint y, GLsizei w, GLsizei h) {
    g->state.viewport[0] = x;
    g->state.viewport[1] = y;
    g->state.viewport[2] = w;
    g->state.viewport[3] = h;
}

void VKGL scissor(GLint x, GLint y, GLsizei w, GLsizei h) {
    g->state.scissor[0] = x;
    g->state.scissor[1] = y;
    g->state.scissor[2] = w;
    g->state.scissor[3] = h;
}

void VKGL depth_func(GLenum f) {
    g->state.depth_func = f;
}

void VKGL depth_mask(GLboolean flag) {
    g->state.depth_mask = flag != GL_FALSE;
}

void VKGL colour_mask(GLboolean r, GLboolean gr, GLboolean b, GLboolean a) {
    g->state.colour_mask[0] = r != GL_FALSE;
    g->state.colour_mask[1] = gr != GL_FALSE;
    g->state.colour_mask[2] = b != GL_FALSE;
    g->state.colour_mask[3] = a != GL_FALSE;
}

void VKGL blend_func_separate(GLenum sr, GLenum dr, GLenum sa, GLenum da) {
    State& s = g->state;
    s.blend_src_rgb = sr;
    s.blend_dst_rgb = dr;
    s.blend_src_a = sa;
    s.blend_dst_a = da;
}

void VKGL blend_equation(GLenum mode) {
    g->state.blend_equation = mode;
}

void VKGL blend_color(GLfloat r, GLfloat gr, GLfloat b, GLfloat a) {
    g->state.blend_colour[0] = r;
    g->state.blend_colour[1] = gr;
    g->state.blend_colour[2] = b;
    g->state.blend_colour[3] = a;
}

void VKGL cull_face(GLenum mode) {
    g->state.cull_face = mode;
}

void VKGL front_face(GLenum mode) {
    g->state.front_face = mode;
}

void VKGL polygon_mode(GLenum, GLenum mode) {
    g->state.polygon_mode = mode;
}

void VKGL pixel_store(GLenum name, GLint value) {
    switch (name) {
        case GL_UNPACK_ALIGNMENT: g->state.unpack_alignment = value; break;
        case GL_PACK_ALIGNMENT: g->state.pack_alignment = value; break;
        case kGlUnpackRowLength: g->state.unpack_row_length = value; break;
        default: break;
    }
}

Image* read_image() {
    return target_of(g->state.read_framebuffer).colour;
}

void VKGL read_pixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, void* pixels) {
    Image* image = read_image();
    if (image == nullptr || w <= 0 || h <= 0 || format != GL_RGBA || type != GL_UNSIGNED_BYTE
        || image->samples != VK_SAMPLE_COUNT_1_BIT) {
        fail(kGlInvalidEnum);
        return;
    }
    const int x0 = std::clamp(x, 0, image->width), y0 = std::clamp(y, 0, image->height);
    const int x1 = std::clamp(x + w, x0, image->width), y1 = std::clamp(y + h, y0, image->height);
    if (x1 <= x0 || y1 <= y0) {
        return;
    }
    const int cw = x1 - x0, ch = y1 - y0;
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = static_cast<VkDeviceSize>(cw) * static_cast<VkDeviceSize>(ch) * 4;
    info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo alloc{};
    alloc.usage = VMA_MEMORY_USAGE_AUTO;
    alloc.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    VmaAllocationInfo got{};
    if (!check(vmaCreateBuffer(g->allocator, &info, &alloc, &buffer, &allocation, &got), "a read-back buffer")) {
        return;
    }
    VkCommandBuffer cmd = cmd_for(image, true);
    barrier(cmd, *image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.imageOffset = {x0, y0, 0};
    copy.imageExtent = {static_cast<std::uint32_t>(cw), static_cast<std::uint32_t>(ch), 1};
    vkCmdCopyImageToBuffer(cmd, image->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &copy);
    flush_and_wait();
    vmaInvalidateAllocation(g->allocator, allocation, 0, VK_WHOLE_SIZE);
    // Rows come as GL gives them, from the bottom: the same rows (vkgl.h, "Rows").
    const int align = std::max(g->state.pack_alignment, 1);
    const std::size_t pitch = (static_cast<std::size_t>(w) * 4 + align - 1) / align * align;
    const auto* src = static_cast<const std::uint8_t*>(got.pMappedData);
    auto* dst = static_cast<std::uint8_t*>(pixels);
    for (int row = 0; row < ch; ++row) {
        std::memcpy(dst + pitch * static_cast<std::size_t>(row + (y0 - y)) + static_cast<std::size_t>(x0 - x) * 4,
                    src + static_cast<std::size_t>(row) * static_cast<std::size_t>(cw) * 4,
                    static_cast<std::size_t>(cw) * 4);
    }
    vmaDestroyBuffer(g->allocator, buffer, allocation);
}

Texture* bound_texture(GLenum target) {
    const State& s = g->state;
    const std::size_t unit = s.active_unit;
    switch (target) {
        case GL_TEXTURE_2D: return g->textures.find(s.unit_2d[unit]);
        case wgl::GL_TEXTURE_BUFFER: return g->textures.find(s.unit_buffer[unit]);
        case wgl::GL_TEXTURE_2D_ARRAY: return g->textures.find(s.unit_array[unit]);
        default: return nullptr;
    }
}

void VKGL copy_tex_sub_image(GLenum target, GLint level, GLint xoff, GLint yoff, GLint x, GLint y, GLsizei w,
                             GLsizei h) {
    Texture* t = bound_texture(target);
    Image* from = read_image();
    if (t == nullptr || !t->image || from == nullptr || w <= 0 || h <= 0 || from->samples != VK_SAMPLE_COUNT_1_BIT) {
        return;
    }
    Image& to = *t->image;
    // Clipped to both images, as GL leaves what lies outside the source undefined.
    const int sx = std::max(x, 0), sy = std::max(y, 0);
    const int dx = xoff + (sx - x), dy = yoff + (sy - y);
    const int cw = std::min({w - (sx - x), from->width - sx, (to.width >> level) - dx});
    const int ch = std::min({h - (sy - y), from->height - sy, (to.height >> level) - dy});
    if (cw <= 0 || ch <= 0 || dx < 0 || dy < 0 || level >= to.levels) {
        return;
    }
    VkCommandBuffer cmd = cmd_for(from, true);
    to.touched = g->recording;
    barrier(cmd, *from, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    barrier(cmd, to, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkImageCopy copy{};
    copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.srcOffset = {sx, sy, 0};
    copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, static_cast<std::uint32_t>(level), 0, 1};
    copy.dstOffset = {dx, dy, 0};
    copy.extent = {static_cast<std::uint32_t>(cw), static_cast<std::uint32_t>(ch), 1};
    if (from->format == to.format) {
        vkCmdCopyImage(cmd, from->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, to.image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
    } else {
        VkImageBlit blit{};
        blit.srcSubresource = copy.srcSubresource;
        blit.srcOffsets[0] = {sx, sy, 0};
        blit.srcOffsets[1] = {sx + cw, sy + ch, 1};
        blit.dstSubresource = copy.dstSubresource;
        blit.dstOffsets[0] = {dx, dy, 0};
        blit.dstOffsets[1] = {dx + cw, dy + ch, 1};
        vkCmdBlitImage(cmd, from->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, to.image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);
    }
}

void VKGL finish() {
    flush_and_wait();
}

void VKGL gen_textures(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->textures.add();
    }
}

void VKGL delete_textures(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        auto it = g->textures.objects.find(names[i]);
        if (it == g->textures.objects.end()) {
            continue;
        }
        retire(std::move(it->second.image));
        g->textures.objects.erase(it);
        for (GLuint& b : g->state.unit_2d) {
            b = b == names[i] ? 0 : b;
        }
        for (GLuint& b : g->state.unit_buffer) {
            b = b == names[i] ? 0 : b;
        }
    }
}

void VKGL bind_texture(GLenum target, GLuint name) {
    State& s = g->state;
    const std::size_t unit = s.active_unit;
    if (Texture* t = g->textures.find(name); t != nullptr && t->target == 0) {
        t->target = target;
    }
    switch (target) {
        case GL_TEXTURE_2D: s.unit_2d[unit] = name; break;
        case wgl::GL_TEXTURE_BUFFER: s.unit_buffer[unit] = name; break;
        case wgl::GL_TEXTURE_2D_ARRAY: s.unit_array[unit] = name; break;
        default: break;
    }
}

void VKGL active_texture(GLenum unit) {
    g->state.active_unit = std::min<GLuint>(unit - GL_TEXTURE0, kUnits - 1);
}

void VKGL tex_image_2d(GLenum target, GLint level, GLint internal, GLsizei w, GLsizei h, GLint, GLenum format,
                       GLenum type, const void* pixels) {
    Texture* t = bound_texture(target);
    if (t == nullptr || w <= 0 || h <= 0 || level < 0) {
        return;
    }
    const Format f = format_of(static_cast<GLenum>(internal));
    if (f.vk == VK_FORMAT_UNDEFINED) {
        fail(kGlInvalidEnum);
        return;
    }
    if (level == 0) {
        Image* old = t->image.get();
        const bool same = old != nullptr && old->width == w && old->height == h && old->format == f.vk;
        // Its contents replaced where nothing has drawn with it yet; a new image where something
        // has (what was recorded keeps the old one), unless levels past the first must be kept.
        if (!same || (old->touched == g->recording && old->levels == 1)) {
            retire(std::move(t->image));
            t->image = make_image(f, w, h, same ? old->levels : 1, VK_SAMPLE_COUNT_1_BIT);
        }
        t->internal = static_cast<GLenum>(internal);
    } else {
        if (!t->image || std::max(t->image->width >> level, 1) != w || std::max(t->image->height >> level, 1) != h) {
            return;  // a level that does not belong to the first one's chain
        }
        if (t->image->levels <= level) {
            t->image = with_chain(t->image);
        }
        if (t->image->levels <= level) {
            return;
        }
    }
    if (!t->image || pixels == nullptr) {
        return;
    }
    Image& image = *t->image;
    const Slice staged = stage_pixels(f, w, h, format, type, pixels);
    if (staged.data == nullptr) {
        return;
    }
    VkCommandBuffer cmd = cmd_for(&image);
    copy_to_image(cmd, image, staged, level, 0, 0, w, h);
    barrier(cmd, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VKGL tex_sub_image_2d(GLenum target, GLint level, GLint x, GLint y, GLsizei w, GLsizei h, GLenum format,
                           GLenum type, const void* pixels) {
    Texture* t = bound_texture(target);
    if (t == nullptr || !t->image || pixels == nullptr || w <= 0 || h <= 0 || level >= t->image->levels) {
        return;
    }
    Image& image = *t->image;
    const Format f{image.format, image.aspect, format_of(t->internal).bytes};
    if (x < 0 || y < 0 || x + w > std::max(image.width >> level, 1) || y + h > std::max(image.height >> level, 1)) {
        return;
    }
    const Slice staged = stage_pixels(f, w, h, format, type, pixels);
    if (staged.data == nullptr) {
        return;
    }
    VkCommandBuffer cmd = cmd_for(&image);
    copy_to_image(cmd, image, staged, level, x, y, w, h);
    barrier(cmd, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void set_param(SamplerParams& p, int* base, int* max, GLenum name, float value) {
    const GLenum e = static_cast<GLenum>(value);
    switch (name) {
        case GL_TEXTURE_MIN_FILTER: p.min = e; break;
        case GL_TEXTURE_MAG_FILTER: p.mag = e; break;
        case GL_TEXTURE_WRAP_S: p.wrap_s = e; break;
        case GL_TEXTURE_WRAP_T: p.wrap_t = e; break;
        case GL_TEXTURE_MAX_ANISOTROPY: p.anisotropy = value; break;
        case wgl::GL_TEXTURE_BASE_LEVEL:
            if (base != nullptr) {
                *base = static_cast<int>(value);
            }
            break;
        case GL_TEXTURE_MAX_LEVEL:
            if (max != nullptr) {
                *max = static_cast<int>(value);
            }
            break;
        default: break;
    }
}

void VKGL tex_parameter_i(GLenum target, GLenum name, GLint value) {
    if (Texture* t = bound_texture(target)) {
        set_param(t->params, &t->base_level, &t->max_level, name, static_cast<float>(value));
    }
}

void VKGL tex_parameter_f(GLenum target, GLenum name, GLfloat value) {
    if (Texture* t = bound_texture(target)) {
        set_param(t->params, &t->base_level, &t->max_level, name, value);
    }
}

void VKGL generate_mipmap(GLenum target) {
    Texture* t = bound_texture(target);
    if (t == nullptr || !t->image) {
        return;
    }
    t->image = with_chain(t->image);
    if (t->image->levels > 1) {
        generate_mipmaps(*t->image);
    }
}

void VKGL gen_buffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->buffers.add();
    }
}

void destroy_buffer(Buffer& b) {
    forget_views(b);
    if (b.own != VK_NULL_HANDLE) {
        defer([buffer = b.own, allocation = b.own_allocation] { vmaDestroyBuffer(g->allocator, buffer, allocation); });
    }
    b.own = VK_NULL_HANDLE;
}

void VKGL delete_buffers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        auto it = g->buffers.objects.find(names[i]);
        if (it != g->buffers.objects.end()) {
            destroy_buffer(it->second);
            g->buffers.objects.erase(it);
        }
    }
}

GLuint* buffer_binding(GLenum target) {
    State& s = g->state;
    switch (target) {
        case GL_ARRAY_BUFFER: return &s.array_buffer;
        case GL_ELEMENT_ARRAY_BUFFER: return &vertex_array().elements;
        case wgl::GL_UNIFORM_BUFFER: return &s.uniform_buffer;
        case wgl::GL_TEXTURE_BUFFER: return &s.texture_buffer;
        case GL_SHADER_STORAGE_BUFFER:
            return &s.storage_buffer;
        default: return nullptr;
    }
}

void VKGL bind_buffer(GLenum target, GLuint name) {
    if (target == GL_SHADER_STORAGE_BUFFER) {
        if (auto* b = g->buffers.find(name)) {
            b->storage = true;
        }
    }
    if (GLuint* b = buffer_binding(target)) {
        *b = name;
    }
}

void changed(Buffer& b) {
    ++b.version;
    b.changed_frame = g->frame_count;
}

void VKGL buffer_data(GLenum target, GLsizeiptr size, const void* data, GLenum) {
    GLuint* binding = buffer_binding(target);
    Buffer* b = binding != nullptr ? g->buffers.find(*binding) : nullptr;
    if (b == nullptr || size < 0) {
        return;
    }
    b->data.assign(static_cast<std::size_t>(size), 0);
    if (data != nullptr && size > 0) {
        std::memcpy(b->data.data(), data, static_cast<std::size_t>(size));
    }
    changed(*b);
}

void VKGL buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
    GLuint* binding = buffer_binding(target);
    Buffer* b = binding != nullptr ? g->buffers.find(*binding) : nullptr;
    if (b == nullptr || offset < 0 || size <= 0 || static_cast<std::size_t>(offset + size) > b->data.size()) {
        return;
    }
    std::memcpy(b->data.data() + offset, data, static_cast<std::size_t>(size));
    changed(*b);
}

void VKGL gen_vertex_arrays(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->vertex_arrays.add();
    }
}

void VKGL delete_vertex_arrays(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        g->vertex_arrays.objects.erase(names[i]);
        if (g->state.vertex_array == names[i]) {
            g->state.vertex_array = 0;
        }
    }
}

void VKGL bind_vertex_array(GLuint name) {
    g->state.vertex_array = name;
}

void VKGL enable_vertex_attrib_array(GLuint index) {
    if (index < kAttributes) {
        vertex_array().attributes[index].enabled = true;
    }
}

void VKGL vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                                const void* pointer) {
    if (index >= kAttributes) {
        return;
    }
    Attribute& a = vertex_array().attributes[index];
    a.size = size;
    a.type = type;
    a.normalized = normalized != GL_FALSE;
    a.integer = false;
    a.stride = stride;
    a.offset = reinterpret_cast<std::uintptr_t>(pointer);
    a.buffer = g->state.array_buffer;
}

void VKGL vertex_attrib_i_pointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    vertex_attrib_pointer(index, size, type, GL_FALSE, stride, pointer);
    if (index < kAttributes) {
        vertex_array().attributes[index].integer = true;
    }
}

void VKGL vertex_attrib_divisor(GLuint index, GLuint divisor) {
    if (index < kAttributes) {
        vertex_array().attributes[index].divisor = divisor;
    }
}

void VKGL draw_arrays(GLenum mode, GLint first, GLsizei count) {
    draw(mode, count, 1, false, 0, 0, first);
}

void VKGL draw_arrays_instanced(GLenum mode, GLint first, GLsizei count, GLsizei instances) {
    draw(mode, count, instances, false, 0, 0, first);
}

void VKGL draw_elements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    draw(mode, count, 1, true, type, reinterpret_cast<std::uintptr_t>(indices), 0);
}

void VKGL draw_elements_instanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instances) {
    draw(mode, count, instances, true, type, reinterpret_cast<std::uintptr_t>(indices), 0);
}

// ---- Shaders and programs ----

GLuint VKGL create_shader(GLenum type) {
    const GLuint name = g->shaders.add();
    g->shaders.find(name)->type = type;
    return name;
}

void VKGL shader_source(GLuint name, GLsizei count, const GLchar* const* strings, const GLint* lengths) {
    Shader* sh = g->shaders.find(name);
    if (sh == nullptr) {
        return;
    }
    sh->source.clear();
    for (GLsizei i = 0; i < count; ++i) {
        if (lengths != nullptr && lengths[i] >= 0) {
            sh->source.append(strings[i], static_cast<std::size_t>(lengths[i]));
        } else {
            sh->source.append(strings[i]);
        }
    }
}

void VKGL compile_shader(GLuint name) {
    if (Shader* sh = g->shaders.find(name)) {
        sh->log.clear();
        sh->compiled = sh->type == GL_COMPUTE_SHADER
                           ? check_compute_stage(sh->source, sh->log)
                           : check_stage(sh->type == GL_VERTEX_SHADER, sh->source, sh->log);
    }
}

void write_log(const std::string& log, GLsizei size, GLsizei* length, GLchar* out) {
    const GLsizei n = size > 0 ? std::min<GLsizei>(size - 1, static_cast<GLsizei>(log.size())) : 0;
    if (out != nullptr && size > 0) {
        std::memcpy(out, log.data(), static_cast<std::size_t>(n));
        out[n] = '\0';
    }
    if (length != nullptr) {
        *length = n;
    }
}

void VKGL get_shader_iv(GLuint name, GLenum what, GLint* out) {
    Shader* sh = g->shaders.find(name);
    *out = 0;
    if (sh == nullptr) {
        return;
    }
    if (what == GL_COMPILE_STATUS) {
        *out = sh->compiled ? GL_TRUE : GL_FALSE;
    } else if (what == GL_INFO_LOG_LENGTH) {
        *out = sh->log.empty() ? 0 : static_cast<GLint>(sh->log.size() + 1);
    }
}

void VKGL get_shader_info_log(GLuint name, GLsizei size, GLsizei* length, GLchar* log) {
    Shader* sh = g->shaders.find(name);
    write_log(sh != nullptr ? sh->log : std::string(), size, length, log);
}

void VKGL delete_shader(GLuint name) {
    g->shaders.objects.erase(name);
}

GLuint VKGL create_program() {
    return g->programs.add();
}

void VKGL attach_shader(GLuint program, GLuint shader) {
    if (Program* p = g->programs.find(program)) {
        p->shaders.push_back(shader);
    }
}

void VKGL bind_attrib_location(GLuint, GLuint, const GLchar*) {
    // The renderer's shaders give their inputs locations of their own.
}

void release_program(Program& p) {
    defer([vertex = p.vertex,
           fragment = p.fragment,
           compute = p.compute,
           pipeline = p.compute_pipeline,
           set_layout = p.set_layout,
           layout = p.layout] {
        vkDestroyShaderModule(g->device, vertex, nullptr);
        vkDestroyShaderModule(g->device, fragment, nullptr);
        vkDestroyShaderModule(g->device, compute, nullptr);
        vkDestroyPipeline(g->device, pipeline, nullptr);
        vkDestroyPipelineLayout(g->device, layout, nullptr);
        vkDestroyDescriptorSetLayout(g->device, set_layout, nullptr);
    });
    // Its pipelines go with it.
    const std::uint64_t serial = p.serial;
    for (auto it = g->pipelines.begin(); it != g->pipelines.end();) {
        std::uint64_t key_serial = 0;
        std::memcpy(&key_serial, it->first.data(), sizeof key_serial);
        if (key_serial == serial) {
            defer([pipeline = it->second] { vkDestroyPipeline(g->device, pipeline, nullptr); });
            it = g->pipelines.erase(it);
        } else {
            ++it;
        }
    }
    p.vertex = p.fragment = VK_NULL_HANDLE;
    p.compute = VK_NULL_HANDLE;
    p.compute_pipeline = VK_NULL_HANDLE;
    p.set_layout = VK_NULL_HANDLE;
    p.layout = VK_NULL_HANDLE;
}

VkShaderModule module_of(const std::vector<std::uint32_t>& code) {
    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize = code.size() * sizeof(std::uint32_t);
    info.pCode = code.data();
    VkShaderModule module = VK_NULL_HANDLE;
    check(vkCreateShaderModule(g->device, &info, nullptr, &module), "a shader module");
    return module;
}

void VKGL link_program(GLuint name) {
    Program* p = g->programs.find(name);
    if (p == nullptr) {
        return;
    }
    std::string vs, fs, cs;
    for (GLuint s : p->shaders) {
        if (Shader* sh = g->shaders.find(s)) {
            (sh->type == GL_VERTEX_SHADER    ? vs
             : sh->type == GL_COMPUTE_SHADER ? cs
                                             : fs) = sh->source;
        }
    }
    if (p->serial != 0) {
        release_program(*p);
    }
    p->log.clear();
    p->linked = false;
    p->serial = ++g->program_serial;
    if (!compile_program(vs, fs, p->compiled, p->log, cs)) {
        return;
    }
    if (!material_program(*p, vs, fs)) {
        return;
    }
    const CompiledProgram& c = p->compiled;
    if (!c.compute.empty()) {
        p->compute = module_of(c.compute);
    } else {
        p->vertex = module_of(c.vertex);
        p->fragment = module_of(c.fragment);
    }
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    auto add = [&](int binding, VkDescriptorType type) {
        if (binding < 0) {
            return;
        }
        bindings.push_back(
            {static_cast<std::uint32_t>(binding),
             type,
             1,
             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
                 | VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr}
        );
    };
    p->dynamic_uniforms = c.default_binding >= 0 && reuse_uniforms();
    add(c.default_binding,
        p->dynamic_uniforms ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                            : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    for (const ProgramBlock& b : c.blocks) {
        add(b.binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    }
    for (const auto& b : c.storage) {
        add(b.binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
    for (const ProgramUniform& u : c.uniforms) {
        if (u.sampler && u.binding >= 0) {
            add(u.binding, u.kind == SamplerKind::Buffer ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
                                                         : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }
    }
    VkDescriptorSetLayoutCreateInfo set_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    set_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
    set_info.pBindings = bindings.data();
    check(vkCreateDescriptorSetLayout(g->device, &set_info, nullptr, &p->set_layout), "a descriptor set layout");
    VkPipelineLayoutCreateInfo layout_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    const VkDescriptorSetLayout layouts[] = {p->set_layout, g->material_layout};
    const VkPushConstantRange material_push{VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
    layout_info.setLayoutCount = p->gpu_materials ? 2u : 1u;
    layout_info.pSetLayouts = layouts;
    if (p->gpu_materials) {
        layout_info.pushConstantRangeCount = 1;
        layout_info.pPushConstantRanges = &material_push;
    }
    check(vkCreatePipelineLayout(g->device, &layout_info, nullptr, &p->layout), "a pipeline layout");
    // Uniforms start at zero, as GL's do; a relink keeps nothing, as GL's does.
    p->uniforms.assign(
        p->gpu_materials ? p->material_start + kMaterialBytes
                         : static_cast<unsigned>(std::max(c.default_size, 0)),
        0
    );
    p->units.assign(c.uniforms.size(), 0);
    p->block_bindings.assign(c.blocks.size(), 0);
    p->uniforms_dirty = true;
    p->last_set = VK_NULL_HANDLE;
    p->last_key.clear();
    p->descriptor_sets.clear();
    if (p->compute) {
        VkComputePipelineCreateInfo info{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        info.stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        info.stage.module = p->compute;
        info.stage.pName = "main";
        info.layout = p->layout;
        check(
            vkCreateComputePipelines(
                g->device, VK_NULL_HANDLE, 1, &info, nullptr, &p->compute_pipeline
            ),
            "a compute pipeline"
        );
    }
    p->linked = p->layout && ((p->vertex && p->fragment) || p->compute_pipeline);
}

void VKGL get_program_iv(GLuint name, GLenum what, GLint* out) {
    Program* p = g->programs.find(name);
    *out = 0;
    if (p == nullptr) {
        return;
    }
    if (what == GL_LINK_STATUS) {
        *out = p->linked ? GL_TRUE : GL_FALSE;
    } else if (what == GL_INFO_LOG_LENGTH) {
        *out = p->log.empty() ? 0 : static_cast<GLint>(p->log.size() + 1);
    } else if (what == kGlActiveUniforms) {
        *out = 0;  // not listed (gl_stats' uniform listing and shader reloading are GL's alone)
    }
}

void VKGL get_program_info_log(GLuint name, GLsizei size, GLsizei* length, GLchar* log) {
    Program* p = g->programs.find(name);
    write_log(p != nullptr ? p->log : std::string(), size, length, log);
}

void VKGL delete_program(GLuint name) {
    auto it = g->programs.objects.find(name);
    if (it == g->programs.objects.end()) {
        return;
    }
    if (it->second.serial != 0) {
        release_program(it->second);
    }
    g->programs.objects.erase(it);
    if (g->state.program == name) {
        g->state.program = 0;
    }
}

void VKGL use_program(GLuint name) {
    g->state.program = name;
}

// A uniform's location: its entry in the program's list, and the element of an array.
constexpr int kElementBits = 12;

GLint VKGL get_uniform_location(GLuint program, const GLchar* name) {
    Program* p = g->programs.find(program);
    if (p == nullptr || !p->linked || name == nullptr) {
        return -1;
    }
    std::string_view n(name);
    int element = 0;
    if (const std::size_t bracket = n.find('['); bracket != std::string_view::npos) {
        element = std::atoi(std::string(n.substr(bracket + 1)).c_str());
        n = n.substr(0, bracket);
    }
    const auto& list = p->compiled.uniforms;
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (list[i].name == n && element >= 0 && element < list[i].array_size) {
            return static_cast<GLint>(i << kElementBits | static_cast<std::size_t>(element));
        }
    }
    return -1;
}

// Where a location's value goes in the current program's default block; null for none.
std::uint8_t* uniform_at(GLint location, int count_bytes, const ProgramUniform** out_uniform = nullptr,
                         int* out_index = nullptr) {
    Program* p = g->programs.find(g->state.program);
    if (p == nullptr || !p->linked || location < 0) {
        return nullptr;
    }
    const std::size_t index = static_cast<std::size_t>(location) >> kElementBits;
    const int element = location & ((1 << kElementBits) - 1);
    if (index >= p->compiled.uniforms.size()) {
        return nullptr;
    }
    const ProgramUniform& u = p->compiled.uniforms[index];
    if (out_uniform != nullptr) {
        *out_uniform = &u;
    }
    if (out_index != nullptr) {
        *out_index = static_cast<int>(index);
    }
    if (u.sampler || u.offset < 0) {
        return nullptr;
    }
    const std::size_t at = static_cast<std::size_t>(u.offset) + static_cast<std::size_t>(element) * static_cast<std::size_t>(u.array_stride);
    if (at + static_cast<std::size_t>(count_bytes) > p->uniforms.size()) {
        return nullptr;
    }
    if (element >= u.array_size) {
        return nullptr;
    }
    return p->uniforms.data() + at;
}

void write_uniform(std::uint8_t* destination, const void* source, std::size_t bytes) {
    ++g->submission.uniform_writes;
    // Compare bytes, preserving signed zero and NaN payloads exactly.
    if (reuse_uniforms() && std::memcmp(destination, source, bytes) == 0) {
        ++g->submission.uniform_writes_skipped;
        return;
    }
    std::memcpy(destination, source, bytes);
    auto* p = g->programs.find(g->state.program);
    if (!p->gpu_materials || destination < p->uniforms.data() + p->material_start) {
        p->uniforms_dirty = true;
    }
}

void VKGL uniform_1i(GLint location, GLint v) {
    const ProgramUniform* u = nullptr;
    int index = -1;
    if (std::uint8_t* at = uniform_at(location, 4, &u, &index)) {
        write_uniform(at, &v, 4);
    } else if (u != nullptr && u->sampler) {
        g->programs.find(g->state.program)->units[static_cast<std::size_t>(index)] = v;
    }
}

void VKGL uniform_1ui(GLint location, GLuint v) {
    if (std::uint8_t* at = uniform_at(location, 4)) {
        write_uniform(at, &v, 4);
    }
}

void VKGL uniform_1f(GLint location, GLfloat v) {
    if (std::uint8_t* at = uniform_at(location, 4)) {
        write_uniform(at, &v, 4);
    }
}

void VKGL uniform_2f(GLint location, GLfloat a, GLfloat b) {
    const float v[] = {a, b};
    if (std::uint8_t* at = uniform_at(location, 8)) {
        write_uniform(at, v, 8);
    }
}

void VKGL uniform_3f(GLint location, GLfloat a, GLfloat b, GLfloat c) {
    const float v[] = {a, b, c};
    if (std::uint8_t* at = uniform_at(location, 12)) {
        write_uniform(at, v, 12);
    }
}

void VKGL uniform_4f(GLint location, GLfloat a, GLfloat b, GLfloat c, GLfloat d) {
    const float v[] = {a, b, c, d};
    if (std::uint8_t* at = uniform_at(location, 16)) {
        write_uniform(at, v, 16);
    }
}

void VKGL uniform_4fv(GLint location, GLsizei count, const GLfloat* v) {
    if (location < 0 || count <= 0 || !v) {
        return;
    }
    for (GLsizei i = 0; i < count; ++i) {
        if (std::uint8_t* at = uniform_at(location + i, 16)) {
            write_uniform(at, v + i * 4, 16);
        }
    }
}

void VKGL uniform_matrix_4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* v) {
    if (location < 0 || count <= 0 || !v) {
        return;
    }
    for (GLsizei i = 0; i < count; ++i) {
        std::uint8_t* at = uniform_at(location + i, 64);
        if (at == nullptr) {
            continue;
        }
        const GLfloat* m = v + i * 16;
        if (transpose == GL_FALSE) {
            write_uniform(at, m, 64);
        } else {
            float t[16];
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 4; ++c) {
                    t[c * 4 + r] = m[r * 4 + c];
                }
            }
            write_uniform(at, t, 64);
        }
    }
}

GLuint VKGL get_uniform_block_index(GLuint program, const GLchar* name) {
    Program* p = g->programs.find(program);
    if (p == nullptr || !p->linked || name == nullptr) {
        return wgl::GL_INVALID_INDEX;
    }
    for (std::size_t i = 0; i < p->compiled.blocks.size(); ++i) {
        if (p->compiled.blocks[i].name == name) {
            return static_cast<GLuint>(i);
        }
    }
    return wgl::GL_INVALID_INDEX;
}

void VKGL uniform_block_binding(GLuint program, GLuint block, GLuint binding) {
    Program* p = g->programs.find(program);
    if (p != nullptr && block < p->block_bindings.size()) {
        p->block_bindings[block] = binding;
    }
}

void VKGL bind_buffer_base(GLenum target, GLuint index, GLuint buffer) {
    if (target == GL_SHADER_STORAGE_BUFFER && index < kUniformBindings) {
        g->state.storage_bindings[index] = buffer;
        if (auto* b = g->buffers.find(buffer)) {
            b->storage = true;
        }
    }
    if (target == wgl::GL_UNIFORM_BUFFER && index < kUniformBindings) {
        g->state.uniform_bindings[index] = buffer;
    }
    if (GLuint* b = buffer_binding(target)) {
        *b = buffer;
    }
}

void VKGL tex_buffer(GLenum target, GLenum format, GLuint buffer) {
    if (Texture* t = bound_texture(target)) {
        t->buffer = buffer;
        t->buffer_format = format;
    }
}

void VKGL tex_image_3d(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) {
    fail(kGlInvalidEnum);  // no renderer draws with a texture array yet
}

void VKGL gen_samplers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->samplers.add();
    }
}

void VKGL delete_samplers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        g->samplers.objects.erase(names[i]);
    }
}

void VKGL bind_sampler(GLuint unit, GLuint sampler) {
    if (unit < kUnits) {
        g->state.unit_sampler[unit] = sampler;
    }
}

void VKGL sampler_parameter_i(GLuint sampler, GLenum name, GLint value) {
    if (SamplerParams* p = g->samplers.find(sampler)) {
        set_param(*p, nullptr, nullptr, name, static_cast<float>(value));
    }
}

void VKGL stencil_func(GLenum func, GLint ref, GLuint mask) {
    g->state.stencil_func = func;
    g->state.stencil_ref = ref;
    g->state.stencil_value_mask = mask;
}

void VKGL stencil_op_separate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
    const StencilFace f{sfail, dpfail, dppass};
    if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
        g->state.stencil_front = f;
    }
    if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
        g->state.stencil_back = f;
    }
}

void VKGL stencil_mask(GLuint mask) {
    g->state.stencil_write_mask = mask;
}

void VKGL clear_stencil(GLint s) {
    g->state.clear_stencil = s;
}

// ---- Framebuffers ----

void VKGL gen_framebuffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->framebuffers.add();
    }
}

void VKGL delete_framebuffers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        g->framebuffers.objects.erase(names[i]);
        if (g->state.draw_framebuffer == names[i]) {
            g->state.draw_framebuffer = 0;
        }
        if (g->state.read_framebuffer == names[i]) {
            g->state.read_framebuffer = 0;
        }
    }
}

void VKGL bind_framebuffer(GLenum target, GLuint name) {
    if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER) {
        g->state.draw_framebuffer = name;
    }
    if (target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER) {
        g->state.read_framebuffer = name;
    }
}

Framebuffer* bound_framebuffer(GLenum target) {
    return g->framebuffers.find(target == GL_READ_FRAMEBUFFER ? g->state.read_framebuffer : g->state.draw_framebuffer);
}

void VKGL framebuffer_texture_2d(GLenum target, GLenum attachment, GLenum, GLuint texture, GLint) {
    Framebuffer* fb = bound_framebuffer(target);
    if (fb == nullptr) {
        return;
    }
    if (attachment == GL_COLOR_ATTACHMENT0) {
        fb->colour_texture = texture;
        fb->colour_renderbuffer = 0;
    } else if (attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
        fb->depth_texture = texture;
        fb->depth_renderbuffer = 0;
    }
}

void VKGL framebuffer_renderbuffer(GLenum target, GLenum attachment, GLenum, GLuint renderbuffer) {
    Framebuffer* fb = bound_framebuffer(target);
    if (fb == nullptr) {
        return;
    }
    if (attachment == GL_COLOR_ATTACHMENT0) {
        fb->colour_renderbuffer = renderbuffer;
        fb->colour_texture = 0;
    } else if (attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
        fb->depth_renderbuffer = renderbuffer;
        fb->depth_texture = 0;
    }
}

GLenum VKGL check_framebuffer_status(GLenum target) {
    const Target t = target_of(target == GL_READ_FRAMEBUFFER ? g->state.read_framebuffer : g->state.draw_framebuffer);
    return t.colour != nullptr || t.depth != nullptr ? GL_FRAMEBUFFER_COMPLETE : 0x8CD7;  // INCOMPLETE_MISSING_ATTACHMENT
}

void VKGL gen_renderbuffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->renderbuffers.add();
    }
}

void VKGL delete_renderbuffers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        auto it = g->renderbuffers.objects.find(names[i]);
        if (it != g->renderbuffers.objects.end()) {
            retire(std::move(it->second.image));
            g->renderbuffers.objects.erase(it);
        }
    }
}

void VKGL bind_renderbuffer(GLenum, GLuint name) {
    g->state.renderbuffer = name;
}

void VKGL renderbuffer_storage_multisample(GLenum, GLsizei samples, GLenum format, GLsizei w, GLsizei h) {
    Renderbuffer* r = g->renderbuffers.find(g->state.renderbuffer);
    const Format f = format_of(format);
    if (r == nullptr || f.vk == VK_FORMAT_UNDEFINED) {
        return;
    }
    VkSampleCountFlagBits count = VK_SAMPLE_COUNT_1_BIT;
    for (int c = 64; c > 1; c >>= 1) {
        if (samples >= c) {
            count = static_cast<VkSampleCountFlagBits>(c);
            break;
        }
    }
    retire(std::move(r->image));
    r->image = make_image(f, w, h, 1, count);
}

void VKGL renderbuffer_storage(GLenum target, GLenum format, GLsizei w, GLsizei h) {
    renderbuffer_storage_multisample(target, 0, format, w, h);
}

void VKGL blit_framebuffer(GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0, GLint dx1, GLint dy1,
                           GLbitfield mask, GLenum filter) {
    const Target from = target_of(g->state.read_framebuffer);
    const Target to = target_of(g->state.draw_framebuffer);
    auto blit_one = [&](Image* src, Image* dst, VkImageAspectFlags aspect, VkFilter f) {
        if (src == nullptr || dst == nullptr || src == dst) {
            return;
        }
        VkCommandBuffer cmd = cmd_for(src, true);
        dst->touched = g->recording;
        if (src->samples != VK_SAMPLE_COUNT_1_BIT && (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
            // vkCmdResolveImage is colour-only. Resolve depth/stencil through
            // dynamic rendering; SAMPLE_ZERO is required by Vulkan 1.2+.
            if (dst->samples != VK_SAMPLE_COUNT_1_BIT || sx0 != dx0 || sy0 != dy0 || sx1 != dx1
                || sy1 != dy1 || sx0 < 0 || sy0 < 0 || sx1 <= sx0 || sy1 <= sy0) {
                fail(0x0502);  // GL_INVALID_OPERATION: unsupported depth resolve region
                return;
            }
            barrier(cmd, *src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            barrier(cmd, *dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            VkRenderingAttachmentInfo depth{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            depth.imageView = attachment_view(*src);
            depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            depth.resolveImageView = attachment_view(*dst);
            depth.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
            info.renderArea =
                {{sx0, sy0},
                 {static_cast<std::uint32_t>(std::min({sx1, src->width, dst->width}) - sx0),
                  static_cast<std::uint32_t>(std::min({sy1, src->height, dst->height}) - sy0)}};
            info.layerCount = 1;
            info.pDepthAttachment = &depth;
            if ((aspect & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
                info.pStencilAttachment = &depth;
            }
            vkCmdBeginRendering(cmd, &info);
            vkCmdEndRendering(cmd);
            return;
        }
        barrier(cmd, *src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        barrier(cmd, *dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        if ((aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0 && sx1 - sx0 == dx1 - dx0
            && sy1 - sy0 == dy1 - dy0 && sx1 > sx0 && sy1 > sy0) {
            // Depth/stencil formats need not support BLIT_DST. An unscaled
            // copy has no such requirement; each aspect is copied separately.
            VkImageCopy copies[2]{};
            std::uint32_t count = 0;
            for (const auto bit : {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT}) {
                if ((aspect & bit) == 0) {
                    continue;
                }
                auto& copy = copies[count++];
                copy.srcSubresource = {static_cast<VkImageAspectFlags>(bit), 0, 0, 1};
                copy.dstSubresource = copy.srcSubresource;
                copy.srcOffset = {sx0, sy0, 0};
                copy.dstOffset = {dx0, dy0, 0};
                copy.extent = {
                    static_cast<std::uint32_t>(sx1 - sx0), static_cast<std::uint32_t>(sy1 - sy0), 1
                };
            }
            vkCmdCopyImage(
                cmd,
                src->image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dst->image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                count,
                copies
            );
            return;
        }
        if (src->samples != VK_SAMPLE_COUNT_1_BIT) {
            // A multisampled source is resolved: one size, no flip (as GL requires).
            VkImageResolve r{};
            r.srcSubresource = {aspect, 0, 0, 1};
            r.srcOffset = {std::min(sx0, sx1), std::min(sy0, sy1), 0};
            r.dstSubresource = {aspect, 0, 0, 1};
            r.dstOffset = {std::min(dx0, dx1), std::min(dy0, dy1), 0};
            r.extent = {static_cast<std::uint32_t>(std::abs(sx1 - sx0)), static_cast<std::uint32_t>(std::abs(sy1 - sy0)),
                        1};
            r.extent.width = std::min<std::uint32_t>(r.extent.width, static_cast<std::uint32_t>(dst->width - r.dstOffset.x));
            r.extent.height = std::min<std::uint32_t>(r.extent.height, static_cast<std::uint32_t>(dst->height - r.dstOffset.y));
            vkCmdResolveImage(cmd, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &r);
            return;
        }
        VkImageBlit b{};
        b.srcSubresource = {aspect, 0, 0, 1};
        b.srcOffsets[0] = {std::clamp(sx0, 0, src->width), std::clamp(sy0, 0, src->height), 0};
        b.srcOffsets[1] = {std::clamp(sx1, 0, src->width), std::clamp(sy1, 0, src->height), 1};
        b.dstSubresource = {aspect, 0, 0, 1};
        b.dstOffsets[0] = {std::clamp(dx0, 0, dst->width), std::clamp(dy0, 0, dst->height), 0};
        b.dstOffsets[1] = {std::clamp(dx1, 0, dst->width), std::clamp(dy1, 0, dst->height), 1};
        if (b.srcOffsets[0].x == b.srcOffsets[1].x || b.srcOffsets[0].y == b.srcOffsets[1].y
            || b.dstOffsets[0].x == b.dstOffsets[1].x || b.dstOffsets[0].y == b.dstOffsets[1].y) {
            return;
        }
        vkCmdBlitImage(cmd, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &b, f);
    };
    if ((mask & GL_COLOR_BUFFER_BIT) != 0) {
        blit_one(from.colour, to.colour, VK_IMAGE_ASPECT_COLOR_BIT, filter == GL_LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
    }
    if ((mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0 && from.depth != nullptr && to.depth != nullptr
        && from.depth->format == to.depth->format) {
        blit_one(from.depth, to.depth, from.depth->aspect, VK_FILTER_NEAREST);
    }
}

#undef VKGL

struct Entry {
    const char* name;
    Proc proc;
};

#define VKGL_ENTRY(name, fn) {#name, reinterpret_cast<Proc>(&fn)}
const Entry kEntries[] = {
    VKGL_ENTRY(glGetString, get_string),
    VKGL_ENTRY(glGetStringi, get_stringi),
    VKGL_ENTRY(glGetIntegerv, get_integerv),
    VKGL_ENTRY(glGetFloatv, get_floatv),
    VKGL_ENTRY(glGetError, get_error),
    VKGL_ENTRY(glEnable, enable),
    VKGL_ENTRY(glDisable, disable),
    VKGL_ENTRY(glClear, clear_gl),
    VKGL_ENTRY(glClearColor, clear_color),
    VKGL_ENTRY(glClearDepth, clear_depth),
    VKGL_ENTRY(glViewport, viewport),
    VKGL_ENTRY(glScissor, scissor),
    VKGL_ENTRY(glDepthFunc, depth_func),
    VKGL_ENTRY(glDepthMask, depth_mask),
    VKGL_ENTRY(glColorMask, colour_mask),
    VKGL_ENTRY(glBlendFuncSeparate, blend_func_separate),
    VKGL_ENTRY(glBlendEquation, blend_equation),
    VKGL_ENTRY(glBlendColor, blend_color),
    VKGL_ENTRY(glCullFace, cull_face),
    VKGL_ENTRY(glFrontFace, front_face),
    VKGL_ENTRY(glPolygonMode, polygon_mode),
    VKGL_ENTRY(glPixelStorei, pixel_store),
    VKGL_ENTRY(glReadPixels, read_pixels),
    VKGL_ENTRY(glCopyTexSubImage2D, copy_tex_sub_image),
    VKGL_ENTRY(glFinish, finish),
    VKGL_ENTRY(glGenTextures, gen_textures),
    VKGL_ENTRY(glDeleteTextures, delete_textures),
    VKGL_ENTRY(glBindTexture, bind_texture),
    VKGL_ENTRY(glActiveTexture, active_texture),
    VKGL_ENTRY(glTexImage2D, tex_image_2d),
    VKGL_ENTRY(glTexSubImage2D, tex_sub_image_2d),
    VKGL_ENTRY(glTexParameteri, tex_parameter_i),
    VKGL_ENTRY(glTexParameterf, tex_parameter_f),
    VKGL_ENTRY(glGenerateMipmap, generate_mipmap),
    VKGL_ENTRY(glGenBuffers, gen_buffers),
    VKGL_ENTRY(glDeleteBuffers, delete_buffers),
    VKGL_ENTRY(glBindBuffer, bind_buffer),
    VKGL_ENTRY(glBufferData, buffer_data),
    VKGL_ENTRY(glBufferSubData, buffer_sub_data),
    VKGL_ENTRY(glGenVertexArrays, gen_vertex_arrays),
    VKGL_ENTRY(glDeleteVertexArrays, delete_vertex_arrays),
    VKGL_ENTRY(glBindVertexArray, bind_vertex_array),
    VKGL_ENTRY(glEnableVertexAttribArray, enable_vertex_attrib_array),
    VKGL_ENTRY(glVertexAttribPointer, vertex_attrib_pointer),
    VKGL_ENTRY(glVertexAttribIPointer, vertex_attrib_i_pointer),
    VKGL_ENTRY(glVertexAttribDivisor, vertex_attrib_divisor),
    VKGL_ENTRY(glDrawArrays, draw_arrays),
    VKGL_ENTRY(glDispatchCompute, dispatch_compute),
    VKGL_ENTRY(glMemoryBarrier, memory_barrier),
    VKGL_ENTRY(glDrawArraysInstanced, draw_arrays_instanced),
    VKGL_ENTRY(glDrawElements, draw_elements),
    VKGL_ENTRY(glDrawElementsInstanced, draw_elements_instanced),
    VKGL_ENTRY(glCreateShader, create_shader),
    VKGL_ENTRY(glShaderSource, shader_source),
    VKGL_ENTRY(glCompileShader, compile_shader),
    VKGL_ENTRY(glGetShaderiv, get_shader_iv),
    VKGL_ENTRY(glGetShaderInfoLog, get_shader_info_log),
    VKGL_ENTRY(glDeleteShader, delete_shader),
    VKGL_ENTRY(glCreateProgram, create_program),
    VKGL_ENTRY(glAttachShader, attach_shader),
    VKGL_ENTRY(glBindAttribLocation, bind_attrib_location),
    VKGL_ENTRY(glLinkProgram, link_program),
    VKGL_ENTRY(glGetProgramiv, get_program_iv),
    VKGL_ENTRY(glGetProgramInfoLog, get_program_info_log),
    VKGL_ENTRY(glDeleteProgram, delete_program),
    VKGL_ENTRY(glUseProgram, use_program),
    VKGL_ENTRY(glGetUniformLocation, get_uniform_location),
    VKGL_ENTRY(glUniform1i, uniform_1i),
    VKGL_ENTRY(glUniform1ui, uniform_1ui),
    VKGL_ENTRY(glUniform1f, uniform_1f),
    VKGL_ENTRY(glUniform2f, uniform_2f),
    VKGL_ENTRY(glUniform3f, uniform_3f),
    VKGL_ENTRY(glUniform4f, uniform_4f),
    VKGL_ENTRY(glUniform4fv, uniform_4fv),
    VKGL_ENTRY(glUniformMatrix4fv, uniform_matrix_4fv),
    VKGL_ENTRY(glGetUniformBlockIndex, get_uniform_block_index),
    VKGL_ENTRY(glUniformBlockBinding, uniform_block_binding),
    VKGL_ENTRY(glBindBufferBase, bind_buffer_base),
    VKGL_ENTRY(glTexBuffer, tex_buffer),
    VKGL_ENTRY(glTexImage3D, tex_image_3d),
    VKGL_ENTRY(glGenSamplers, gen_samplers),
    VKGL_ENTRY(glDeleteSamplers, delete_samplers),
    VKGL_ENTRY(glBindSampler, bind_sampler),
    VKGL_ENTRY(glSamplerParameteri, sampler_parameter_i),
    VKGL_ENTRY(glStencilFunc, stencil_func),
    VKGL_ENTRY(glStencilOpSeparate, stencil_op_separate),
    VKGL_ENTRY(glStencilMask, stencil_mask),
    VKGL_ENTRY(glClearStencil, clear_stencil),
    VKGL_ENTRY(glGenFramebuffers, gen_framebuffers),
    VKGL_ENTRY(glDeleteFramebuffers, delete_framebuffers),
    VKGL_ENTRY(glBindFramebuffer, bind_framebuffer),
    VKGL_ENTRY(glFramebufferTexture2D, framebuffer_texture_2d),
    VKGL_ENTRY(glFramebufferRenderbuffer, framebuffer_renderbuffer),
    VKGL_ENTRY(glCheckFramebufferStatus, check_framebuffer_status),
    VKGL_ENTRY(glGenRenderbuffers, gen_renderbuffers),
    VKGL_ENTRY(glDeleteRenderbuffers, delete_renderbuffers),
    VKGL_ENTRY(glBindRenderbuffer, bind_renderbuffer),
    VKGL_ENTRY(glRenderbufferStorage, renderbuffer_storage),
    VKGL_ENTRY(glRenderbufferStorageMultisample, renderbuffer_storage_multisample),
    VKGL_ENTRY(glBlitFramebuffer, blit_framebuffer),
};
#undef VKGL_ENTRY

// ---- The device, the screen and the frames ----

void run_deferred(Frame& f) {
    std::vector<std::function<void()>> list = std::move(f.deferred);
    f.deferred.clear();
    for (auto& fn : list) {
        fn();
    }
}

ImagePtr default_image(const Format& f, int width, int height) {
    ImagePtr image = make_image(f, width, height, 1, VK_SAMPLE_COUNT_1_BIT);
    return image;
}

bool make_default_framebuffer(int width, int height) {
    g->default_colour = default_image({VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 4}, width, height);
    g->default_depth = default_image(
        {g->depth_format, VK_IMAGE_ASPECT_DEPTH_BIT | (has_stencil(g->depth_format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0u), 4},
        width, height);
    g->default_width = width;
    g->default_height = height;
    return g->default_colour && g->default_depth;
}

VkPresentModeKHR present_mode() {
    std::uint32_t n = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(g->physical, g->surface, &n, nullptr);
    std::vector<VkPresentModeKHR> modes(n);
    vkGetPhysicalDeviceSurfacePresentModesKHR(g->physical, g->surface, &n, modes.data());
    auto has = [&](VkPresentModeKHR m) { return std::find(modes.begin(), modes.end(), m) != modes.end(); };
    if (g->interval == 0) {
        if (has(VK_PRESENT_MODE_IMMEDIATE_KHR)) {
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
        if (has(VK_PRESENT_MODE_MAILBOX_KHR)) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    if (g->interval < 0 && has(VK_PRESENT_MODE_FIFO_RELAXED_KHR)) {
        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

bool make_swapchain() {
    if (g->surface == VK_NULL_HANDLE) {
        return true;
    }
    vkDeviceWaitIdle(g->device);
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g->physical, g->surface, &caps);
    std::uint32_t n = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g->physical, g->surface, &n, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(n);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g->physical, g->surface, &n, formats.data());
    if (formats.empty()) {
        return false;
    }
    // A linear format: the picture is in the display's own (gamma) space already, as on the GS.
    VkSurfaceFormatKHR format = formats[0];
    for (const VkSurfaceFormatKHR& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM || f.format == VK_FORMAT_R8G8B8A8_UNORM
            || f.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32) {
            format = f;
            break;
        }
    }
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(g->window, &w, &h);
    VkExtent2D extent{static_cast<std::uint32_t>(std::max(w, 1)), static_cast<std::uint32_t>(std::max(h, 1))};
#if !defined(__ANDROID__)
    if (caps.currentExtent.width != 0xFFFFFFFFu) {
        extent = caps.currentExtent;
    }
#endif
    extent.width = std::clamp(extent.width, caps.minImageExtent.width, std::max(caps.maxImageExtent.width, 1u));
    extent.height = std::clamp(extent.height, caps.minImageExtent.height, std::max(caps.maxImageExtent.height, 1u));
    if (extent.width == 0 || extent.height == 0) {
        return false;  // minimised: made again when it is shown
    }
    std::uint32_t count = std::max(caps.minImageCount + 1, 3u);
    if (caps.maxImageCount != 0) {
        count = std::min(count, caps.maxImageCount);
    }
    VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    info.surface = g->surface;
    info.minImageCount = count;
    info.imageFormat = format.format;
    info.imageColorSpace = format.colorSpace;
    info.imageExtent = extent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // The system turns the picture to the screen's way up (no pre-rotation of our own).
    info.preTransform = (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0
                            ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
                            : caps.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    for (const VkCompositeAlphaFlagBitsKHR a :
         {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
          VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR}) {
        if ((caps.supportedCompositeAlpha & a) != 0) {
            info.compositeAlpha = a;
            break;
        }
    }
    info.presentMode = present_mode();
    info.clipped = VK_TRUE;
    info.oldSwapchain = g->swapchain;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    if (!check(vkCreateSwapchainKHR(g->device, &info, nullptr, &swapchain), "the swapchain")) {
        return false;
    }
    if (g->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(g->device, g->swapchain, nullptr);
    }
    for (VkSemaphore s : g->rendered) {
        vkDestroySemaphore(g->device, s, nullptr);
    }
    g->swapchain = swapchain;
    g->swapchain_format = format.format;
    g->swapchain_extent = extent;
    vkGetSwapchainImagesKHR(g->device, swapchain, &n, nullptr);
    g->swapchain_images.resize(n);
    vkGetSwapchainImagesKHR(g->device, swapchain, &n, g->swapchain_images.data());
    g->rendered.assign(n, VK_NULL_HANDLE);
    for (VkSemaphore& s : g->rendered) {
        VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(g->device, &si, nullptr, &s);
    }
    g->swapchain_stale = false;
    return true;
}

// The next frame: its slot's last use waited for, its leftovers freed, its buffers begun.
void advance() {
    g->frame = (g->frame + 1) % kFrames;
    Frame& f = frame();
    if (f.submitted) {
        const Clock::time_point start = Clock::now();
        vkWaitForFences(g->device, 1, &f.fence, VK_TRUE, UINT64_MAX);
        g->measures.wait_s += seconds_since(start);
        timing_results(f);
        f.submitted = false;
    }
    run_deferred(f);
    f.timings.clear();
    f.timestamp_used = 0;
    f.active_timing = -1;
    vkResetCommandPool(g->device, f.pool, 0);
    for (VkDescriptorPool pool : f.pools) {
        vkResetDescriptorPool(g->device, pool, 0);
    }
    f.pool_at = 0;
    f.chunk_at = 0;
    f.chunk_used = 0;
    ++g->frame_count;
    begin_buffers();
}

void swapchain_barrier(VkCommandBuffer cmd, VkImage image, VkImageLayout from, VkImageLayout to) {
    VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    b.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    b.oldLayout = from;
    b.newLayout = to;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image;
    b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &b);
}

// A new surface for the window, where the old one went (renew_surface).
bool make_surface(SDL_Window* window, std::string& error);

// The surface and swapchain made again: back from the background on Android, the window is a new
// one and the old surface draws nowhere (the screen stayed black while the game went on).
bool renew_surface() {
    vkDeviceWaitIdle(g->device);
    if (g->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(g->device, g->swapchain, nullptr);
        g->swapchain = VK_NULL_HANDLE;
    }
    if (g->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(g->instance, g->surface, nullptr);
        g->surface = VK_NULL_HANDLE;
    }
    std::string error;
    if (!make_surface(g->window, error)) {
        return false;  // no window yet: tried again with the next frame
    }
    g->surface_lost = false;
    g->swapchain_stale = true;
    log::info("vulkan: the window's surface made again");
    return true;
}

bool present_to_screen() {
    if (g->surface_lost && !renew_surface()) {
        return false;
    }
    if (g->swapchain_stale || g->swapchain == VK_NULL_HANDLE) {
        if (!make_swapchain()) {
            return false;
        }
    }
    Frame& f = frame();
    std::uint32_t index = 0;
    VkResult r = vkAcquireNextImageKHR(g->device, g->swapchain, UINT64_MAX, f.acquired, VK_NULL_HANDLE, &index);
    if (r == VK_ERROR_OUT_OF_DATE_KHR) {
        if (!make_swapchain()) {
            return false;
        }
        r = vkAcquireNextImageKHR(g->device, g->swapchain, UINT64_MAX, f.acquired, VK_NULL_HANDLE, &index);
    }
    if (r == VK_ERROR_SURFACE_LOST_KHR) {
        g->surface_lost = true;
    }
    if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) {
        return false;
    }
    end_rendering();
    VkCommandBuffer cmd = f.main;
    Image& colour = *g->default_colour;
    barrier(cmd, colour, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    const VkImage screen = g->swapchain_images[index];
    swapchain_barrier(cmd, screen, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // Turned the right way up: row 0 of the picture is the bottom of the screen, as in GL.
    VkImageBlit blit{};
    blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit.srcOffsets[1] = {colour.width, colour.height, 1};
    blit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit.dstOffsets[0] = {0, static_cast<std::int32_t>(g->swapchain_extent.height), 0};
    blit.dstOffsets[1] = {static_cast<std::int32_t>(g->swapchain_extent.width), 0, 1};
    const bool same = colour.width == static_cast<int>(g->swapchain_extent.width)
                      && colour.height == static_cast<int>(g->swapchain_extent.height);
    vkCmdBlitImage(cmd, colour.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, screen, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &blit, same ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);
    swapchain_barrier(cmd, screen, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    submit(f.acquired, g->rendered[index], false);
    VkPresentInfoKHR info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &g->rendered[index];
    info.swapchainCount = 1;
    info.pSwapchains = &g->swapchain;
    info.pImageIndices = &index;
    r = vkQueuePresentKHR(g->queue, &info);
#if defined(__ANDROID__)
    // Android says "suboptimal" for every frame of a turned screen that the system turns for us.
    const bool stale = r == VK_ERROR_OUT_OF_DATE_KHR;
    if (r == VK_ERROR_SURFACE_LOST_KHR) {
        g->surface_lost = true;
    }
#else
    const bool stale = r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR;
#endif
    if (stale) {
        g->swapchain_stale = true;
    }
    return true;
}

std::string version_text(std::uint32_t v) {
    return std::format("{}.{}.{}", VK_API_VERSION_MAJOR(v), VK_API_VERSION_MINOR(v), VK_API_VERSION_PATCH(v));
}

bool layer_available(const char* name) {
    std::uint32_t n = 0;
    vkEnumerateInstanceLayerProperties(&n, nullptr);
    std::vector<VkLayerProperties> layers(n);
    vkEnumerateInstanceLayerProperties(&n, layers.data());
    for (const VkLayerProperties& l : layers) {
        if (std::strcmp(l.layerName, name) == 0) {
            return true;
        }
    }
    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL on_message(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                          VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data,
                                          void*) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        if (g) ++g->validation_errors;
        log::error("vulkan: {}", data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log::warn("vulkan: {}", data->pMessage);
    }
    return VK_FALSE;
}

bool make_instance(const Config& config, bool validation, std::string& error) {
    std::vector<const char*> extensions;
#if defined(__ANDROID__)
    // Asked of the driver itself (a player's own driver is not the one SDL would load).
    if (config.window != nullptr) {
        extensions = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
    }
#else
    if (config.window != nullptr) {
        Uint32 n = 0;
        const char* const* names = SDL_Vulkan_GetInstanceExtensions(&n);
        if (names == nullptr) {
            error = std::format("SDL has no Vulkan surface here: {}", SDL_GetError());
            return false;
        }
        extensions.assign(names, names + n);
    }
#endif
    std::vector<const char*> layers;
    bool debug_utils = validation;
    if (validation) layers.push_back("VK_LAYER_KHRONOS_validation");
#if defined(OPENRAC_GPU_MARKERS)
    debug_utils |= std::getenv("OPENRAC_GPU_MARKERS") != nullptr;
#endif
    if (debug_utils) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    // A driver that carries out only part of Vulkan (MoltenVK, on Metal) is listed by the loader
    // only to an instance that asks for these.
    VkInstanceCreateFlags flags = 0;
    {
        std::uint32_t n = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &n, nullptr);
        std::vector<VkExtensionProperties> offered(n);
        vkEnumerateInstanceExtensionProperties(nullptr, &n, offered.data());
        for (const VkExtensionProperties& e : offered) {
            if (std::strcmp(e.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0) {
                extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
        }
    }
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "OpenRAC";
    app.pEngineName = "OpenRAC";
    app.apiVersion = VK_API_VERSION_1_3;
    VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    info.flags = flags;
    info.pApplicationInfo = &app;
    info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    info.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
    info.ppEnabledLayerNames = layers.data();
    const VkResult r = vkCreateInstance(&info, nullptr, &g->instance);
    if (r != VK_SUCCESS) {
        error = std::format("no Vulkan instance ({})", static_cast<int>(r));
        return false;
    }
    volkLoadInstance(g->instance);
    if (validation) {
        VkDebugUtilsMessengerCreateInfoEXT m{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        m.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        m.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        m.pfnUserCallback = on_message;
        vkCreateDebugUtilsMessengerEXT(g->instance, &m, nullptr, &g->messenger);
    }
    return true;
}

// The device: a graphics card before an integrated one before one that draws on the CPU, with
// Vulkan 1.3 (dynamic rendering) and a queue that draws and shows the window.
bool pick_device(std::string& error) {
    std::uint32_t n = 0;
    vkEnumeratePhysicalDevices(g->instance, &n, nullptr);
    std::vector<VkPhysicalDevice> devices(n);
    vkEnumeratePhysicalDevices(g->instance, &n, devices.data());
    int best = -1;
    for (VkPhysicalDevice d : devices) {
        VkPhysicalDeviceProperties p{};
        vkGetPhysicalDeviceProperties(d, &p);
        if (p.apiVersion < VK_API_VERSION_1_3) {
            continue;
        }
        VkPhysicalDeviceVulkan13Features f13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
        VkPhysicalDeviceFeatures2 f2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        f2.pNext = &f13;
        vkGetPhysicalDeviceFeatures2(d, &f2);
        if (!f13.dynamicRendering) {
            continue;
        }
        std::uint32_t qn = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qn, nullptr);
        std::vector<VkQueueFamilyProperties> queues(qn);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qn, queues.data());
        for (std::uint32_t i = 0; i < qn; ++i) {
            if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                continue;
            }
            VkBool32 present = VK_TRUE;
            if (g->surface != VK_NULL_HANDLE) {
                vkGetPhysicalDeviceSurfaceSupportKHR(d, i, g->surface, &present);
            }
            if (!present) {
                continue;
            }
            const int score = p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU     ? 4
                              : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 3
                              : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU            ? 1
                                                                                       : 2;
            if (score > best) {
                best = score;
                g->physical = d;
                g->family = i;
            }
            break;
        }
    }
    if (g->physical == VK_NULL_HANDLE) {
        error = "no Vulkan 1.3 device with dynamic rendering that can show the window";
        return false;
    }
    vkGetPhysicalDeviceProperties(g->physical, &g->properties);
    std::uint32_t families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g->physical, &families, nullptr);
    std::vector<VkQueueFamilyProperties> queues(families);
    vkGetPhysicalDeviceQueueFamilyProperties(g->physical, &families, queues.data());
    g->timestamp_bits = queues.at(g->family).timestampValidBits;
    vkGetPhysicalDeviceFeatures(g->physical, &g->features);
    return true;
}

std::filesystem::path pipeline_cache_file() {
    if (g == nullptr) {
        return {};
    }
    if (const char* env = std::getenv("OPENRAC_VULKAN_PIPELINE_CACHE"); env != nullptr) {
        if (std::strcmp(env, "0") == 0 || std::strcmp(env, "off") == 0 || std::strcmp(env, "none") == 0) {
            return {};
        }
        if (env[0] != '\0') {
            return env;
        }
    }
    std::filesystem::path base;
#if defined(__APPLE__)
    const char* home = std::getenv("HOME");
    base = std::filesystem::path(home != nullptr ? home : ".") / "Library/Caches/OpenRAC";
#elif defined(__ANDROID__)
    const char* home = std::getenv("HOME");
    base = std::filesystem::path(home != nullptr ? home : ".") / "cache";
#else
    if (const char* xdg = std::getenv("XDG_CACHE_HOME"); xdg != nullptr && xdg[0] != '\0') {
        base = std::filesystem::path(xdg) / "openrac";
    } else {
        const char* home = std::getenv("USERPROFILE");
        if (home == nullptr || home[0] == '\0') {
            home = std::getenv("HOME");
        }
        base = std::filesystem::path(home != nullptr ? home : ".") / ".cache/openrac";
    }
#endif
    return base / std::format("vulkan_pipelines_{:04x}_{:04x}.bin", g->properties.vendorID, g->properties.deviceID);
}

bool make_pipeline_cache() {
    std::vector<char> initial_data;
    const std::filesystem::path path = pipeline_cache_file();
    if (!path.empty()) {
        std::ifstream in(path, std::ios::binary | std::ios::ate);
        if (in.is_open()) {
            const auto size = in.tellg();
            if (size >= static_cast<std::streamoff>(sizeof(VkPipelineCacheHeaderVersionOne))) {
                in.seekg(0, std::ios::beg);
                initial_data.resize(static_cast<std::size_t>(size));
                if (!in.read(initial_data.data(), size)) {
                    initial_data.clear();
                }
            }
        }
    }
    bool use_cache = false;
    if (!initial_data.empty()) {
        VkPipelineCacheHeaderVersionOne header{};
        std::memcpy(&header, initial_data.data(), sizeof(header));
        if (header.headerSize == sizeof(VkPipelineCacheHeaderVersionOne)
            && header.headerVersion == VK_PIPELINE_CACHE_HEADER_VERSION_ONE
            && header.vendorID == g->properties.vendorID
            && header.deviceID == g->properties.deviceID
            && std::memcmp(header.pipelineCacheUUID, g->properties.pipelineCacheUUID, VK_UUID_SIZE) == 0) {
            use_cache = true;
        } else {
            log::info("vulkan: pipeline cache UUID changed, starting a fresh cache");
        }
    }
    VkPipelineCacheCreateInfo cache{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
    if (use_cache) {
        cache.initialDataSize = initial_data.size();
        cache.pInitialData = initial_data.data();
    }
    if (!check(vkCreatePipelineCache(g->device, &cache, nullptr, &g->pipeline_cache), "the pipeline cache")) {
        if (use_cache) {
            log::warn("vulkan: driver refused cached pipelines, starting a fresh cache");
            cache.initialDataSize = 0;
            cache.pInitialData = nullptr;
            if (!check(vkCreatePipelineCache(g->device, &cache, nullptr, &g->pipeline_cache), "the pipeline cache")) {
                return false;
            }
        } else {
            return false;
        }
    } else if (use_cache) {
        log::info("vulkan: loaded {} bytes of cached pipelines from {}", initial_data.size(), path.string());
    }
    return true;
}

void save_pipeline_cache() {
    if (g == nullptr || g->device == VK_NULL_HANDLE || g->pipeline_cache == VK_NULL_HANDLE || !g->open) {
        return;
    }
    const std::filesystem::path path = pipeline_cache_file();
    if (path.empty()) {
        return;
    }
    std::size_t size = 0;
    if (vkGetPipelineCacheData(g->device, g->pipeline_cache, &size, nullptr) != VK_SUCCESS
        || size <= sizeof(VkPipelineCacheHeaderVersionOne)) {
        return;
    }
    std::vector<char> buffer(size);
    if (vkGetPipelineCacheData(g->device, g->pipeline_cache, &size, buffer.data()) != VK_SUCCESS) {
        return;
    }
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        log::warn("vulkan: could not make cache folder {}: {}", path.parent_path().string(), ec.message());
        return;
    }
    const std::filesystem::path temp = path.string() + ".tmp";
    std::ofstream out(temp, std::ios::binary | std::ios::trunc);
    if (!out.is_open() || !out.write(buffer.data(), static_cast<std::streamsize>(buffer.size()))) {
        log::warn("vulkan: could not write pipeline cache to {}", temp.string());
        return;
    }
    out.close();
    std::filesystem::rename(temp, path, ec);
    if (ec) {
        log::warn("vulkan: could not rename pipeline cache to {}: {}", path.string(), ec.message());
        return;
    }
    log::info("vulkan: saved {} bytes of pipeline cache to {}", size, path.string());
}

bool make_device(std::string& error) {
    const float priority = 1.0f;
    VkDeviceQueueCreateInfo queue{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queue.queueFamilyIndex = g->family;
    queue.queueCount = 1;
    queue.pQueuePriorities = &priority;
    VkPhysicalDeviceFeatures features{};
    features.fillModeNonSolid = g->features.fillModeNonSolid;
    features.samplerAnisotropy = g->features.samplerAnisotropy;
    features.shaderClipDistance = g->features.shaderClipDistance;  // the world's clip planes
    features.shaderCullDistance = g->features.shaderCullDistance;
    features.drawIndirectFirstInstance = g->features.drawIndirectFirstInstance;
    features.multiDrawIndirect = g->features.multiDrawIndirect;
    features.multiViewport = g->features.multiViewport;
    g->features = features;
    VkPhysicalDeviceVulkan13Features f13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    f13.dynamicRendering = VK_TRUE;
    VkPhysicalDeviceVulkan11Features f11{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    VkPhysicalDeviceVulkan12Features f12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    VkPhysicalDeviceFeatures2 available{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    available.pNext = &f11;
    f11.pNext = &f12;
    vkGetPhysicalDeviceFeatures2(g->physical, &available);
    g->viewport_index_supported = features.multiViewport && f12.shaderOutputViewportIndex;
    g->bindless_supported =
        f11.shaderDrawParameters && f12.shaderSampledImageArrayNonUniformIndexing
        && f12.descriptorBindingSampledImageUpdateAfterBind && f12.descriptorBindingPartiallyBound;
    VkPhysicalDeviceDescriptorIndexingProperties indexing{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES
    };
    VkPhysicalDeviceProperties2 properties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    properties.pNext = &indexing;
    vkGetPhysicalDeviceProperties2(g->physical, &properties);
    const auto limit = std::min(
        {indexing.maxPerStageDescriptorUpdateAfterBindSamplers,
         indexing.maxPerStageDescriptorUpdateAfterBindSampledImages,
         indexing.maxDescriptorSetUpdateAfterBindSamplers,
         indexing.maxDescriptorSetUpdateAfterBindSampledImages}
    );
    g->material_texture_capacity = limit > 32 ? std::min(1024u, limit - 32u) : 0;
    g->bindless_supported = g->bindless_supported && g->material_texture_capacity >= 16;
    f11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    f12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    f12.shaderOutputViewportIndex = g->viewport_index_supported ? VK_TRUE : VK_FALSE;
    f13.pNext = &f11;
    f11.pNext = &f12;
    if (g->bindless_supported) {
        f11.shaderDrawParameters = VK_TRUE;
        f12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        f12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        f12.descriptorBindingPartiallyBound = VK_TRUE;
        f13.pNext = &f11;
        f11.pNext = &f12;
    }
    std::vector<const char*> extensions;
    if (g->surface != VK_NULL_HANDLE) {
        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    // A driver that offers it (MoltenVK) must be told the application knows it carries out only
    // part of Vulkan. What it leaves out (point polygons, wide lines, cull distances, logic ops)
    // the renderer does not use; triangle fans it has.
    {
        constexpr const char* kPortabilitySubset = "VK_KHR_portability_subset";  // vulkan_beta.h
        std::uint32_t n = 0;
        vkEnumerateDeviceExtensionProperties(g->physical, nullptr, &n, nullptr);
        std::vector<VkExtensionProperties> offered(n);
        vkEnumerateDeviceExtensionProperties(g->physical, nullptr, &n, offered.data());
        for (const VkExtensionProperties& e : offered) {
            if (std::strcmp(e.extensionName, kPortabilitySubset) == 0) {
                extensions.push_back(kPortabilitySubset);
            }
        }
    }
    VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    info.pNext = &f13;
    info.queueCreateInfoCount = 1;
    info.pQueueCreateInfos = &queue;
    info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    info.pEnabledFeatures = &features;
    const VkResult r = vkCreateDevice(g->physical, &info, nullptr, &g->device);
    if (r != VK_SUCCESS) {
        error = std::format("no Vulkan device ({})", static_cast<int>(r));
        return false;
    }
    volkLoadDevice(g->device);
    vkGetDeviceQueue(g->device, g->family, 0, &g->queue);

    VmaAllocatorCreateInfo alloc{};
    alloc.vulkanApiVersion = VK_API_VERSION_1_3;
    alloc.physicalDevice = g->physical;
    alloc.device = g->device;
    alloc.instance = g->instance;
    VmaVulkanFunctions functions{};
    vmaImportVulkanFunctionsFromVolk(&alloc, &functions);
    alloc.pVulkanFunctions = &functions;
    if (!check(vmaCreateAllocator(&alloc, &g->allocator), "the memory allocator")) {
        error = "no memory allocator";
        return false;
    }
    g->depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    for (const VkFormat f : {VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT}) {
        VkFormatProperties p{};
        vkGetPhysicalDeviceFormatProperties(g->physical, f, &p);
        if ((p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
            g->depth_format = f;
            break;
        }
    }
    return make_pipeline_cache();
}

bool make_frames() {
    for (Frame& f : g->frames) {
        if (g->timestamp_bits) {
            VkQueryPoolCreateInfo queries{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
            queries.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queries.queryCount = 512;
            if (!check(
                    vkCreateQueryPool(g->device, &queries, nullptr, &f.timestamps), "timestamp pool"
                )) {
                return false;
            }
        }
        VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        pool.queueFamilyIndex = g->family;
        if (!check(vkCreateCommandPool(g->device, &pool, nullptr, &f.pool), "a command pool")) {
            return false;
        }
        VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        alloc.commandPool = f.pool;
        alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandBufferCount = 2;
        VkCommandBuffer buffers[2];
        if (!check(vkAllocateCommandBuffers(g->device, &alloc, buffers), "command buffers")) {
            return false;
        }
        f.upload = buffers[0];
        f.main = buffers[1];
        VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(g->device, &fence, nullptr, &f.fence);
        VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(g->device, &semaphore, nullptr, &f.acquired);
    }
    return true;
}

bool make_dummies() {
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = 512;
    info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                 | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    VmaAllocationCreateInfo alloc{};
    alloc.usage = VMA_MEMORY_USAGE_AUTO;
    alloc.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    alloc.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VmaAllocationInfo got{};
    if (!check(vmaCreateBuffer(g->allocator, &info, &alloc, &g->dummy_buffer, &g->dummy_allocation, &got),
               "the default buffer")) {
        return false;
    }
    auto* bytes = static_cast<std::uint8_t*>(got.pMappedData);
    std::memset(bytes, 0, 512);
    const float one_f[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    const std::uint32_t one_u[4] = {0, 0, 0, 1};
    std::memcpy(bytes + kDefaultFloat, one_f, sizeof one_f);
    std::memcpy(bytes + kDefaultInteger, one_u, sizeof one_u);
    const VkFormat formats[] =
        {VK_FORMAT_R32_UINT,
         VK_FORMAT_R32G32B32A32_UINT,
         VK_FORMAT_R32G32B32A32_SFLOAT,
         VK_FORMAT_R32G32B32A32_SINT};
    for (std::size_t i = 0; i < std::size(formats); ++i) {
        VkBufferViewCreateInfo v{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
        v.buffer = g->dummy_buffer;
        v.format = formats[i];
        v.range = 256;
        vkCreateBufferView(g->device, &v, nullptr, &g->dummy_views[i]);
    }
    // GL's incomplete texture reads opaque black.
    g->dummy_image = make_image({VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 4}, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT);
    if (!g->dummy_image) {
        return false;
    }
    const std::uint8_t black[4] = {0, 0, 0, 0xFF};
    const Slice s = ring_copy(black, 4);
    VkCommandBuffer cmd = upload_cmd();
    copy_to_image(cmd, *g->dummy_image, s, 0, 0, 0, 1, 1);
    barrier(cmd, *g->dummy_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return true;
}

bool make_surface(SDL_Window* window, std::string& error) {
#if defined(__ANDROID__)
    // Made with the instance's own functions, from the window SDL holds.
    auto* native = static_cast<ANativeWindow*>(
        SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr));
    VkAndroidSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
    info.window = native;
    if (native == nullptr || vkCreateAndroidSurfaceKHR(g->instance, &info, nullptr, &g->surface) != VK_SUCCESS) {
        error = "no Vulkan surface for the window";
        return false;
    }
    return true;
#else
    if (!SDL_Vulkan_CreateSurface(window, g->instance, nullptr, &g->surface)) {
        error = std::format("no Vulkan surface for the window: {}", SDL_GetError());
        return false;
    }
    return true;
#endif
}

void describe_device() {
    const VkPhysicalDeviceProperties& p = g->properties;
    g->device_name = p.deviceName;
    g->api_version = version_text(p.apiVersion);
    VkPhysicalDeviceDriverProperties driver{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES};
    VkPhysicalDeviceProperties2 p2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    p2.pNext = &driver;
    vkGetPhysicalDeviceProperties2(g->physical, &p2);
    g->driver_version = driver.driverInfo[0] != '\0' ? std::format("{} {}", driver.driverName, driver.driverInfo)
                                                     : std::string(driver.driverName);
    g->software = p.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
    g->gl_renderer = std::format("{} (Vulkan)", g->device_name);
    g->gl_version = std::format("4.1 OpenRAC on Vulkan {}, {}", g->api_version, g->driver_version);
}

void destroy_all() {
    if (g->device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(g->device);
        gpu_release();
        material_release();
        save_pipeline_cache();
        for (Frame& f : g->frames) {
            // Window teardown can run after the statistics singleton's
            // destructor. Drain measurements only at live frame-slot reuse,
            // never call stats from static shutdown.
            run_deferred(f);
        }
        for (auto& [name, t] : g->textures.objects) {
            t.image.reset();
        }
        for (auto& [name, r] : g->renderbuffers.objects) {
            r.image.reset();
        }
        for (auto& [name, b] : g->buffers.objects) {
            destroy_buffer(b);
        }
        for (auto& [name, p] : g->programs.objects) {
            if (p.serial != 0) {
                release_program(p);
            }
        }
        for (Frame& f : g->frames) {
            run_deferred(f);
        }
        for (auto& [key, pipeline] : g->pipelines) {
            vkDestroyPipeline(g->device, pipeline, nullptr);
        }
        for (auto& [key, sampler] : g->vk_samplers) {
            vkDestroySampler(g->device, sampler, nullptr);
        }
        g->default_colour.reset();
        g->default_depth.reset();
        g->dummy_image.reset();
        for (VkBufferView v : g->dummy_views) {
            vkDestroyBufferView(g->device, v, nullptr);
        }
        if (g->dummy_buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(g->allocator, g->dummy_buffer, g->dummy_allocation);
        }
        for (Frame& f : g->frames) {
            for (const Chunk& c : f.chunks) {
                vmaDestroyBuffer(g->allocator, c.buffer, c.allocation);
            }
            for (VkDescriptorPool pool : f.pools) {
                vkDestroyDescriptorPool(g->device, pool, nullptr);
            }
            vkDestroyCommandPool(g->device, f.pool, nullptr);
            vkDestroyQueryPool(g->device, f.timestamps, nullptr);
            vkDestroyFence(g->device, f.fence, nullptr);
            vkDestroySemaphore(g->device, f.acquired, nullptr);
        }
        for (VkSemaphore s : g->rendered) {
            vkDestroySemaphore(g->device, s, nullptr);
        }
        if (g->swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(g->device, g->swapchain, nullptr);
        }
        if (g->pipeline_cache != VK_NULL_HANDLE) {
            vkDestroyPipelineCache(g->device, g->pipeline_cache, nullptr);
            g->pipeline_cache = VK_NULL_HANDLE;
        }
        if (g->allocator != nullptr) {
            vmaDestroyAllocator(g->allocator);
        }
        vkDestroyDevice(g->device, nullptr);
        g->device = VK_NULL_HANDLE;
    }
    if (g->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(g->instance, g->surface, nullptr);
    }
    if (g->messenger != VK_NULL_HANDLE) {
        vkDestroyDebugUtilsMessengerEXT(g->instance, g->messenger, nullptr);
    }
    if (g->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(g->instance, nullptr);
    }
}

}  // namespace

// Back in the foreground: the window SDL holds is a new one (Android).
bool SDLCALL on_event(void*, SDL_Event* event) {
    if (g != nullptr && event->type == SDL_EVENT_DID_ENTER_FOREGROUND) {
        g->surface_lost = true;
    }
    return true;
}

bool open(const Config& config, std::string& error) {
    if (g != nullptr) {
        error = "a Vulkan context is open already";
        return false;
    }
    const VkResult loaded = config.get_instance_proc_addr != nullptr
                                ? (volkInitializeCustom(reinterpret_cast<PFN_vkGetInstanceProcAddr>(config.get_instance_proc_addr)),
                                   VK_SUCCESS)
                                : volkInitialize();
    if (loaded != VK_SUCCESS) {
        error = "no Vulkan loader on this system";
        return false;
    }
    g = new Context();
    g->window = config.window;
    g->interval = config.vsync ? 1 : 0;
    const char* env = std::getenv("OPENRAC_VULKAN_VALIDATION");
    const bool validation = (config.validation || (env != nullptr && env[0] == '1'))
                            && layer_available("VK_LAYER_KHRONOS_validation");
    bool ok = make_instance(config, validation, error);
    if (ok && config.window != nullptr) {
        ok = make_surface(config.window, error);
    }
    ok = ok && pick_device(error) && make_device(error) && make_frames();
    if (ok) {
        int w = config.width, h = config.height;
        if (config.window != nullptr) {
            SDL_GetWindowSizeInPixels(config.window, &w, &h);
        }
        begin_buffers();
        ok = make_dummies() && make_default_framebuffer(std::max(w, 1), std::max(h, 1));
        if (!ok) {
            error = "could not make the default framebuffer";
        }
    }
    if (ok && config.window != nullptr && !make_swapchain()) {
        // Minimised at the start: the swapchain comes with the first frame shown.
        g->swapchain_stale = true;
    }
    if (!ok) {
        destroy_all();
        delete g;
        g = nullptr;
        return false;
    }
    describe_device();
    if (g->timestamp_bits) {
        stats::set_gpu_hooks({timing_begin, timing_end, nullptr});
    }
    // GL's defaults that are not zero.
    g->state.viewport[2] = g->state.scissor[2] = g->default_width;
    g->state.viewport[3] = g->state.scissor[3] = g->default_height;
    g->open = true;
#if defined(__ANDROID__)
    if (config.window != nullptr) {
        SDL_AddEventWatch(on_event, nullptr);
    }
#endif
    log::info("vulkan: {} ({}), Vulkan {}{}", g->device_name, g->driver_version, g->api_version,
              validation ? ", with the validation layer" : "");
    return true;
}

void close() {
    if (g == nullptr) {
        return;
    }
#if defined(__ANDROID__)
    SDL_RemoveEventWatch(on_event, nullptr);
#endif
    destroy_all();
    delete g;
    g = nullptr;
}

bool active() {
    return g != nullptr && g->open;
}

bool gpu_driven_active() {
    if (!active() || !g->features.drawIndirectFirstInstance) {
        return false;
    }
    static const bool& off = stats::probe(
        "OPENRAC_GPU_DRIVEN_OFF", "Reference: CPU instance visibility and direct mesh draws"
    );
    return !off && gpu_make_engine();
}

bool indexed_viewports_supported(unsigned count) {
    return active() && g->viewport_index_supported && count > 0
           && count <= g->properties.limits.maxViewports && material_enabled();
}

bool gpu_materials_active() {
    return active() && material_enabled();
}

void indexed_viewports(std::span<const std::array<int, 4>> rectangles) {
    if (!rectangles.empty() && !indexed_viewports_supported(static_cast<unsigned>(rectangles.size()))) {
        throw std::runtime_error("indexed viewports unavailable");
    }
    gpu_flush_draws();
    g->indexed_viewports.assign(rectangles.begin(), rectangles.end());
    g->bound_pipeline = VK_NULL_HANDLE;
}

renderer::GpuInstanceBatch gpu_instances(
    unsigned source,
    std::span<const renderer::GpuCullBounds> bounds,
    const renderer::GpuCullView& view
) {
    if (view.plane_count > 6) {
        throw std::runtime_error("invalid GPU culling plane count");
    }
    return gpu_make_batch(source, bounds, view);
}

bool draw_gpu_instances(renderer::GpuInstanceBatch batch, unsigned first, unsigned count) {
    return active() && gpu_submit_draw(batch, first, count);
}

GpuDrawCounters gpu_completed_draws() {
    return active() && stats::enabled() ? g->gpu.completed : GpuDrawCounters{};
}

GpuMaterialCounters gpu_material_counters() {
    return active() ? GpuMaterialCounters{g->bindless_supported, g->material_draws,
        g->material_records, g->material_textures, g->measures.native_draws}
        : GpuMaterialCounters{};
}

Proc get_proc(const char* name) {
    if (name == nullptr) {
        return nullptr;
    }
    for (const Entry& e : kEntries) {
        if (std::strcmp(e.name, name) == 0) {
            return e.proc;
        }
    }
    return nullptr;
}

// The frame's measures, as gauges of the frame the debug tools are measuring.
void report_measures() {
    const auto& current = g->submission;
    const auto& previous = g->reported_submission;
    stats::gauge("vk_uniform_uploads", double(current.uniform_uploads - previous.uniform_uploads));
    stats::gauge(
        "vk_uniform_upload_kb",
        double(current.uniform_upload_bytes - previous.uniform_upload_bytes) / 1024.0
    );
    stats::gauge(
        "vk_uniform_writes_skipped",
        double(current.uniform_writes_skipped - previous.uniform_writes_skipped)
    );
    stats::gauge("vk_descriptor_sets", double(current.descriptor_sets - previous.descriptor_sets));
    stats::gauge(
        "vk_descriptor_reuses", double(current.descriptor_reuses - previous.descriptor_reuses)
    );
    g->reported_submission = current;
    Measures& m = g->measures;
    stats::gauge("vk_draws", static_cast<double>(m.draws));
    stats::gauge("vk_draw_api_calls", double(m.native_draws));
    stats::gauge("vk_material_draws", double(g->material_draws));
    stats::gauge("vk_material_records", double(g->material_records));
    stats::gauge("vk_material_textures", double(g->material_textures));
    g->material_draws = g->material_records = g->material_textures = 0;
    stats::gauge("vk_gpu_instance_batches", double(g->gpu.batches));
    stats::gauge("vk_gpu_instance_candidates", double(g->gpu.candidates));
    stats::gauge("vk_gpu_indirect_commands", double(g->gpu.indirect));
    stats::gauge("vk_gpu_indirect_calls", double(g->gpu.indirect_calls));
    stats::gauge("vk_gpu_indirect_merged", double(g->gpu.merged));
    const char* break_names[] =
        {"pipeline",
         "descriptors",
         "uniforms",
         "vertices",
         "indices",
         "materials",
         "viewport",
         "dynamic",
         "arguments",
         "material_gap"};
    for (std::size_t i = 0; i < g->gpu.batch_breaks.size(); ++i) {
        stats::gauge(
            std::string("vk_gpu_batch_break_") + break_names[i], double(g->gpu.batch_breaks[i])
        );
    }
    g->gpu.batch_breaks.fill(0);
    stats::gauge("vk_gpu_completed_recording", double(g->gpu.completed.recording));
    stats::gauge("vk_gpu_completed_draws", double(g->gpu.completed.draws));
    stats::gauge("vk_gpu_completed_triangles", double(g->gpu.completed.triangles));
    stats::gauge("vk_gpu_completed_visible_instances", double(g->gpu.completed.visible_instances));
    stats::gauge("vk_gpu_counter_readback_bytes", double(g->gpu.completed.readback_bytes));
    g->gpu.batches = g->gpu.candidates = g->gpu.indirect = g->gpu.indirect_calls = g->gpu.merged =
        0;
    stats::gauge("vk_pipelines_made", static_cast<double>(m.pipelines));
    stats::gauge("vk_ring_kb", static_cast<double>(m.ring_bytes) / 1024.0);
    stats::gauge("vk_flushes", static_cast<double>(m.flushes));
    stats::gauge("vk_draw_ms", m.draw_s * 1000.0);
    stats::gauge("vk_draw_us_each", m.draws != 0 ? m.draw_s * 1e6 / static_cast<double>(m.draws) : 0.0);
    stats::gauge("vk_present_ms", m.present_s * 1000.0);
    stats::gauge("vk_wait_ms", m.wait_s * 1000.0);
    m = Measures{};
}

SubmissionCounters submission_counters() {
    if (!g) return {};
    auto counters = g->submission;
    counters.validation_errors = g->validation_errors.load();
    return counters;
}

void present() {
    if (!active()) {
        return;
    }
    const Clock::time_point start = Clock::now();
    bool shown = false;
    if (g->window != nullptr) {
        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(g->window, &w, &h);
        if (w > 0 && h > 0) {
            shown = present_to_screen();
        }
    }
    if (!shown) {
        submit(VK_NULL_HANDLE, VK_NULL_HANDLE, false);
    }
    g->measures.present_s += seconds_since(start);
    report_measures();
    advance();
    // A window of another size: the default framebuffer follows, as a GL window's does.
    if (g->window != nullptr) {
        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(g->window, &w, &h);
        if (w > 0 && h > 0 && (w != g->default_width || h != g->default_height)) {
            retire(std::move(g->default_colour));
            retire(std::move(g->default_depth));
            make_default_framebuffer(w, h);
            g->swapchain_stale = true;
        }
    }
}

int set_swap_interval(int interval) {
    if (!active()) {
        return interval;
    }
    g->interval = interval;
    g->swapchain_stale = true;
    if (g->surface == VK_NULL_HANDLE) {
        return interval;
    }
    const VkPresentModeKHR mode = present_mode();
    return mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR ? -1 : mode == VK_PRESENT_MODE_FIFO_KHR ? 1 : 0;
}

const std::string& device_name() {
    static const std::string none;
    return g != nullptr ? g->device_name : none;
}

const std::string& driver_version() {
    static const std::string none;
    return g != nullptr ? g->driver_version : none;
}

const std::string& api_version() {
    static const std::string none;
    return g != nullptr ? g->api_version : none;
}

bool software() {
    return g != nullptr && g->software;
}

}  // namespace openrac::vkgl

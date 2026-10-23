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

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
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

// ---- The device ----

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

inline constexpr unsigned kMaterialDraws = 32768;
inline constexpr unsigned kMaterialWords = 20;
inline constexpr unsigned kMaterialBytes = kMaterialWords * sizeof(std::uint32_t);

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
    std::unordered_map<
        std::array<std::uint32_t, kMaterialWords>,
        unsigned,
        ArrayHash<std::uint32_t, kMaterialWords>>
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
    std::vector<std::pair<std::uint32_t, VkImageView>>
        views;  // (base | top << 16) -> view for sampling
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

inline bool reuse_uniforms() {
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

inline double seconds_since(Clock::time_point start) {
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

extern Context* g;

inline void fail(GLenum error) {
    if (g->error == GL_NO_ERROR) {
        g->error = error;
    }
}

inline bool check(VkResult result, const char* what) {
    if (result == VK_SUCCESS) {
        return true;
    }
    log::error("vulkan: {} failed ({})", what, static_cast<int>(result));
    return false;
}

inline Frame& frame() {
    return g->frames[static_cast<std::size_t>(g->frame)];
}

struct Slice {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    std::uint8_t* data = nullptr;
};

struct Format {
    VkFormat vk = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    int bytes = 4;  // a texel's
};

struct BufferRef {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
};

inline constexpr VkDeviceSize kDefaultFloat = 256;    // in the dummy buffer: (0, 0, 0, 1) as floats
inline constexpr VkDeviceSize kDefaultInteger = 272;  // and as integers

struct Input {
    std::uint32_t location, format, stride, instanced;
};

// What a sampler uniform reads now: its image (null for a buffer texture) and the descriptor.
struct Sampled {
    Image* image = nullptr;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkBufferView texels = VK_NULL_HANDLE;
};

// ---- vkgl_commands.cpp ----

void gpu_flush_draws();

void defer(std::function<void()> f);

void destroy_image_now(Image& image);

// Drops a reference to an image at the end of the frame: what this frame recorded may use it.
void retire(ImagePtr image);

Slice ring(VkDeviceSize size);

Slice ring_copy(const void* data, VkDeviceSize size);

VkCommandBuffer upload_cmd();

void end_rendering();

// The command buffer for an operation on `image`, outside any drawing: `upload` while this
// recording has not used it in `main` (it then runs before everything `main` does), else `main`.
VkCommandBuffer cmd_for(Image* image, bool force_main = false);

void barrier(VkCommandBuffer cmd, Image& image, VkImageLayout to);

void begin_buffers();

// Native timestamp pairs back the existing non-nesting stats::Scope API.
// Read only after the frame-slot fence signals; profiling never waits on a
// query or adds per-pass submissions. Slots also survive readback flushes.
bool timing_begin(std::uint64_t serial, int section);

void timing_end();

void timing_results(Frame& f);

// Ends both command buffers and submits them, `upload` first; `wait` waits for them to finish
// (a read of pixels, glFinish), and the next ones are begun.
void submit(VkSemaphore wait_semaphore, VkSemaphore signal, bool wait);

void flush_and_wait();

Target target_of(GLuint name);

void ensure_rendering(const Target& t);

VkRect2D scissor_rect(const Target& t);

void clear(GLbitfield mask);

// ---- vkgl_resources.cpp ----

Format format_of(GLenum internal);

bool has_stencil(VkFormat f);

ImagePtr make_image(
    const Format& format, int width, int height, int levels, VkSampleCountFlagBits samples
);

VkImageView attachment_view(Image& image);

// For sampling: levels base..top; a depth image's depth.
VkImageView sample_view(Image& image, int base, int top);

// Pixels as the GL call gave them (format, type, the unpack alignment and row length) into the
// ring, as the image's format lays them out, tightly.
Slice stage_pixels(
    const Format& to, int width, int height, GLenum format, GLenum type, const void* pixels
);

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

void generate_mipmaps(Image& image);

VkSampler sampler_for(const SamplerParams& p, bool integer);

BufferRef dummy_buffer_ref();

BufferRef resolve(Buffer& b, bool persistent = true);

void forget_views(Buffer& b);

VkBufferView texel_view(GLuint name, GLenum gl_format, VkBufferView fallback);

// ---- vkgl_pipelines.cpp ----

VkFormat vertex_format(GLenum type, int size, bool normalized, bool integer);

int type_bytes(GLenum type);

// The kind of number a shader input of this GL type holds: 0 float, 1 unsigned, 2 signed.
int input_kind(std::uint32_t gl_type);

VkPipeline pipeline_for(
    Program& p, GLenum mode, const Target& t, const Input* inputs, std::uint32_t count
);

VkDescriptorSet allocate_set(VkDescriptorSetLayout layout);

Sampled sampled(const Program& p, std::size_t i);

VkDescriptorSet descriptors(Program& p, const std::vector<Sampled>& samplers);

void OPENRAC_GLAPI dispatch_compute(GLuint x, GLuint y, GLuint z);

void OPENRAC_GLAPI memory_barrier(GLbitfield);

// ---- vkgl_draw.cpp ----

VertexArray& vertex_array();

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

bool material_enabled();

bool material_program(Program& p, const std::string& vertex, const std::string& fragment);

void material_reset(Frame& f);

void material_release();

bool gpu_make_engine();

Chunk gpu_buffer(VkDeviceSize size, VkBufferUsageFlags usage, bool mapped);

void gpu_reset(Frame& f);

renderer::GpuInstanceBatch gpu_make_batch(
    unsigned buffer,
    std::span<const renderer::GpuCullBounds> bounds,
    const renderer::GpuCullView& view
);

bool gpu_submit_draw(renderer::GpuInstanceBatch batch, unsigned first, unsigned count);

void gpu_record();

void gpu_results(Frame& f);

void gpu_release();

// ---- vkgl_gl.cpp ----

void destroy_buffer(Buffer& b);

void release_program(Program& p);

// ---- vkgl_present.cpp ----

void run_deferred(Frame& f);

bool make_default_framebuffer(int width, int height);

VkPresentModeKHR present_mode();

bool make_swapchain();

// The next frame: its slot's last use waited for, its leftovers freed, its buffers begun.
void advance();

bool present_to_screen();

// ---- vkgl_device.cpp ----

bool layer_available(const char* name);

bool make_instance(const Config& config, bool validation, std::string& error);

// The device: a graphics card before an integrated one before one that draws on the CPU, with
// Vulkan 1.3 (dynamic rendering) and a queue that draws and shows the window.
bool pick_device(std::string& error);

bool make_device(std::string& error);

bool make_frames();

bool make_dummies();

// A new surface for the window, where the old one went (renew_surface).
bool make_surface(SDL_Window* window, std::string& error);

void describe_device();

void destroy_all();

}  // namespace openrac::vkgl::detail

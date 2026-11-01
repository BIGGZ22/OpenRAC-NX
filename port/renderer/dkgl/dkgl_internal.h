// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// dkgl's own types and state, shared by its files and included by nothing else.

#pragma once

#include <deko3d.h>

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "renderer/dkgl/dkgl.h"
#include "renderer/dkgl/diagnostics.h"
#include "renderer/gl.h"
#include "common/timing_samples.h"

namespace openrac::dkgl::detail {

using namespace openrac::gl;

inline constexpr unsigned kSlots = 2;                 // frames in flight
inline constexpr unsigned kSwapImages = 3;            // swapchain images
inline constexpr std::uint32_t kRingBytes = 48u << 20;  // per slot: uniforms, dynamic data, uploads
inline constexpr std::uint32_t kCmdChunk = 1u << 20;  // command memory, grown in chunks
inline constexpr unsigned kImageDescriptors = 8192;
inline constexpr unsigned kSamplerDescriptors = 2048;
inline constexpr unsigned kUnits = 32;       // texture units
inline constexpr unsigned kAttributes = 16;  // vertex attributes (and vertex buffer bindings)

inline constexpr std::uint64_t align_up(std::uint64_t v, std::uint64_t a) {
    return (v + a - 1) / a * a;
}

// ---- Reflection of a precompiled program (vkgl's encode_program format) ----

struct Uniform {
    std::string name;
    std::uint32_t gl_type = 0;
    int offset = -1;  // in the default block; -1 for a sampler
    int array_size = 1;
    int array_stride = 0;
    bool sampler = false;
    int binding = -1;
};

struct Reflection {
    int default_binding = -1;
    int default_size = 0;
    std::vector<Uniform> uniforms;
    struct Attribute {
        std::string name;
        int location = -1;
        std::uint32_t gl_type = 0;
    };
    std::vector<Attribute> attributes;
};

bool decode_reflection(const std::vector<std::uint8_t>& bytes, Reflection& out);

// ---- Memory: pieces of a few large blocks ----
//
// Every texture and settled buffer in a memblock of its own ran a level load out of memblocks
// (dkMemBlockCreate failed). A heap takes 32 MiB blocks of one kind and hands out pieces.

struct Heap;

struct Allocation {
    Heap* heap = nullptr;
    std::uint32_t chunk = 0;
    std::uint32_t offset = 0;
    std::uint32_t size = 0;
    explicit operator bool() const { return heap != nullptr; }
    DkMemBlock block() const;
    DkGpuAddr gpu() const;
    void* cpu() const;
};

struct Heap {
    std::uint32_t flags = 0;
    std::uint32_t chunk_size = 32u << 20;
    struct Chunk {
        DkMemBlock block = nullptr;
        std::uint32_t size = 0;
        std::map<std::uint32_t, std::uint32_t> free;  // offset -> size
    };
    std::vector<Chunk> chunks;
};

Allocation allocate(Heap& heap, std::uint32_t size, std::uint32_t align);
void release(const Allocation& a);  // now: only for memory the GPU no longer uses
void retire(const Allocation& a);   // after the frame being recorded has finished

// ---- Objects ----

struct Image {
    Allocation memory;
    DkImageLayout layout{};
    DkImage image{};
    DkImageFormat format = DkImageFormat_None;
    int width = 0, height = 0;
    unsigned levels = 1;
    bool depth = false, stencil = false;
    std::uint32_t descriptor = 0;  // image descriptor index
    std::uint64_t written_frame = ~0ull;  // last frame rendered into (barrier before sampling)
    std::uint64_t sampled_epoch = 0;      // the barrier epoch a draw last sampled it in
    // The ordered epoch (Context::ordered_epoch) in which a draw or clear last wrote it as a
    // render target, masked writes included: while it is the current one, no barrier that waits
    // and invalidates the texture cache has followed that write.
    std::uint64_t draw_write_epoch = 0;
    // The full epoch (Context::full_epoch) in which 3D or 2D work last read or wrote it: while it
    // is still the current one, that work may be unfinished when the copy engine touches it.
    std::uint64_t touched_full_epoch = 0;
    ~Image();
};
using ImagePtr = std::shared_ptr<Image>;

struct SamplerParams {
    GLenum min = 0x2702;  // GL_NEAREST_MIPMAP_LINEAR
    GLenum mag = GL_LINEAR;
    GLenum wrap_s = GL_REPEAT, wrap_t = GL_REPEAT;
    float anisotropy = 1.0f;
    bool operator==(const SamplerParams&) const = default;
};

struct Texture {
    ImagePtr image;
    GLenum internal = 0;
    SamplerParams params;
    int max_level = 1000;
    // RGBA32F level 0 as the CPU sent it (the joint palette): what the draw trace compares the
    // GPU's copy with and skins with on the CPU.
    std::vector<float> shadow;
    std::uint64_t shadow_version = 0;
};

struct Buffer {
    std::vector<std::uint8_t> data;
    std::uint64_t version = 1;
    std::uint64_t changed_frame = 0;
    unsigned changes = 0;
    Allocation block;  // memory of its own once it stopped changing
    std::uint64_t block_version = 0;
    std::uint64_t ring_epoch = 0, ring_version = 0;
    DkGpuAddr ring_addr = 0;
};

struct Attribute {
    bool enabled = false;
    GLuint buffer = 0;
    GLint size = 4;
    GLenum type = GL_FLOAT;
    bool normalized = false, integer = false;
    GLsizei stride = 0;
    std::uintptr_t offset = 0;
    GLuint divisor = 0;
};

struct VertexArray {
    std::uint64_t revision=1;
    std::array<Attribute, kAttributes> attributes{};
    GLuint elements = 0;
};

struct Shader {
    GLenum type = 0;
    std::string source;
};

struct Program {
    std::uint64_t link_revision=0;
    std::vector<GLuint> shaders;
    bool linked = false;
    std::string log;
    std::uint64_t key = 0;
    DkMemBlock code = nullptr;
    DkShader vertex{}, fragment{};
    Reflection reflection;
    ProgramRole role = ProgramRole::Other;
    int skinning_offset = -1;
    std::vector<std::uint8_t> uniforms;  // the default block's bytes
    bool uniforms_dirty = true;
    std::array<int, 64> units{};  // a sampler uniform's unit, by uniform index (first 64)
    std::uint64_t sampler_layout_revision = ~0ull;
    std::size_t sampler_layout_size = 0;
    std::vector<unsigned> sampler_uniforms;
    std::uint64_t uploaded_epoch = 0;
    DkGpuAddr uploaded_addr = 0;
};

struct Renderbuffer {
    ImagePtr image;
};

struct Attachment {
    GLuint texture = 0, renderbuffer = 0;
};

struct Framebuffer {
    Attachment colour, depth;  // depth also stands for depth-stencil
};

// A draw target: the images and their size.
struct Target {
    Image* colour = nullptr;
    Image* depth = nullptr;
    int width = 0, height = 0;
    bool operator==(const Target& o) const { return colour == o.colour && depth == o.depth; }
};

template <typename T>
struct Names {
    std::unordered_map<GLuint, T> objects;
    GLuint next = 1;
    GLuint make() {
        const GLuint name = next++;
        objects.emplace(name, T{});
        return name;
    }
    T* find(GLuint name) {
        auto it = objects.find(name);
        return it == objects.end() ? nullptr : &it->second;
    }
};

// What GL's state machine holds.
struct State {
    std::array<bool, 16> caps{};  // by cap_index
    float clear_colour[4] = {0, 0, 0, 0};
    double clear_depth = 1.0;
    int clear_stencil = 0;
    int viewport[4] = {0, 0, 0, 0};
    int scissor[4] = {0, 0, 0, 0};
    GLenum depth_func = GL_LESS;
    bool depth_mask = true;
    GLenum stencil_func = GL_ALWAYS;
    int stencil_ref = 0;
    GLuint stencil_value_mask = 0xFF, stencil_write_mask = 0xFF;
    GLenum stencil_ops[2][3] = {{GL_KEEP, GL_KEEP, GL_KEEP}, {GL_KEEP, GL_KEEP, GL_KEEP}};
    bool colour_mask[4] = {true, true, true, true};
    GLenum blend_src_rgb = GL_ONE, blend_dst_rgb = GL_ZERO, blend_src_a = GL_ONE,
           blend_dst_a = GL_ZERO;
    GLenum blend_equation = GL_FUNC_ADD;
    float blend_colour[4] = {0, 0, 0, 0};
    GLenum cull_face = GL_BACK, front_face = GL_CCW;
    GLenum polygon_mode = GL_FILL;
    int unpack_alignment = 4, pack_alignment = 4;
    GLuint active_unit = 0;
    std::array<GLuint, kUnits> textures{};
    std::array<GLuint, kUnits> samplers{};
    GLuint array_buffer = 0;
    GLuint vertex_array = 0;
    GLuint program = 0;
    GLuint draw_framebuffer = 0, read_framebuffer = 0;
    GLuint renderbuffer = 0;
    GLenum error = GL_NO_ERROR;
};

struct Retired {
    std::uint64_t frame = 0;
    DkMemBlock block = nullptr;
    Allocation piece;
    std::uint32_t image_descriptor = ~0u;
};

// Per-pass timestamps a slot holds (OPENRAC_DK_PASS_TIMING): 16-byte reports, one memory block.
inline constexpr unsigned kPassReports = 256;       // without OPENRAC_DK_PASS_STATS
inline constexpr unsigned kPassStatsReports = 1536; // with it: each mark is 1 + kPassStatCount reports
inline constexpr unsigned kPassStatCount = 5;
// OPENRAC_DK_PASS_STATS=1: the pipeline counters written beside each pass mark (deko3d 0.5
// DkCounter). Samples passed is counted on this GPU (deko3d enables SampleCounterEnable at
// queue start); the others are what the hardware reports, possibly 0 if it does not count them.
inline constexpr DkCounter kPassStatCounters[kPassStatCount] = {
    DkCounter_VertexShaderInvocations, DkCounter_ClipperInputPrimitives, DkCounter_ClipperOutputPrimitives,
    DkCounter_FragmentShaderInvocations, DkCounter_SamplesPassed,
};
inline constexpr unsigned kPassNames = 64;  // distinct names kept; the rest share "(other)"

struct Slot {
    // Four 16-byte {counter,timestamp} records (deko3d 0.5 FourWords reports): 0 frame start at
    // the top of the pipe, 16 frame end after the present blit, 32 frame start at the end of the
    // 3D pipe (written once the previous frame's 3D work has passed it), 48 the scene's end, just
    // before present() acquires the screen image.
    DkMemBlock timing = nullptr;
    bool timing_started=false,timing_pending=false;
    // OPENRAC_DK_PASS_TIMING: the reports, and which pairs of them bracket which named pass.
    struct PassPair { std::uint16_t name, depth, begin, end; };
    DkMemBlock pass = nullptr;
    std::vector<PassPair> pass_pairs;
    std::uint32_t pass_reports = 0;
    DkCmdBuf cmd = nullptr;
    std::vector<DkMemBlock> cmd_memory;
    unsigned cmd_used = 0;  // chunks of cmd_memory handed to cmd this frame
    DkMemBlock ring = nullptr;
    std::uint32_t ring_used = 0;
    DkFence fence{};
    bool submitted = false;
    std::uint64_t frame = 0;
};

struct Context {
    struct VertexSetupCache {
        bool valid=false;
        GLuint vao=0,program=0;
        std::uint64_t vao_revision=0,link_revision=0,epoch=0;
        std::size_t reflected_inputs=0;
        std::uint32_t count=0;
        std::array<std::uint64_t,kAttributes> buffer_versions{};
    } vertex_setup;
    int timing_every=0;
    TimingSamples gpu_span,gpu_span_eop,slot_wait,acquire_wait,present_interval;
    // Pacing (OPENRAC_DK_TIMING): the GPU's gap from one frame's end to the next frame's top (time
    // the queue spent on neither, e.g. waiting for a screen image), and each frame split at the
    // scene's end into scene and present tail.
    TimingSamples gpu_gap,scene_span,present_tail;
    std::uint64_t last_end_ts=0,gap_overlaps=0;
    std::uint64_t timing_rejected=0;
    // OPENRAC_DK_PASS_TIMING (see dkgl.h): names, open passes, and per-name sums since the last report.
    int pass_every=0;
    bool pass_stats=false;               // OPENRAC_DK_PASS_STATS=1
    unsigned pass_capacity=kPassReports; // reports a slot holds
    unsigned pass_marks=1;               // reports a mark takes
    std::vector<std::array<double,kPassStatCount>> pass_stat;  // per name, sums since the last report
    std::uint64_t pass_stat_rejected=0;
    struct OpenPass { std::uint16_t name; int report; unsigned slot; std::uint64_t frame; };
    std::vector<std::string> pass_names;
    std::vector<OpenPass> pass_stack;
    std::vector<double> pass_ms;
    std::vector<std::uint64_t> pass_calls;
    std::vector<int> pass_depth;
    double pass_top_ms=0;
    // OPENRAC_DK_COUNTERS: texture uploads by shape since the last counters line ({image width,
    // height, format, level, rectangle width, height} -> {uploads, into a busy texture}), so the
    // recurring gameplay uploads can be named whatever came before them.
    std::map<std::array<std::uint32_t,6>,std::array<std::uint64_t,2>> upload_shapes;
    std::uint64_t pass_frames=0,pass_dropped=0,pass_rejected=0,pass_unbalanced=0;
    double previous_present_ms=0;
    DkDevice device = nullptr;
    DkQueue queue = nullptr;
    DkSwapchain swapchain = nullptr;
    void* window = nullptr;
    int screen_width = 1280, screen_height = 720;
    DkMemBlock swap_memory = nullptr;
    std::array<DkImage, kSwapImages> swap_images{};
    int interval = 1;

    std::array<Slot, kSlots> slots;
    unsigned slot = 0;
    std::uint64_t frame = 1;      // the frame being recorded
    std::uint64_t completed = 0;  // every frame up to this one has finished on the GPU
    std::uint64_t ring_epoch = 1;  // changes whenever a ring starts again: ring copies of an older one are gone
    std::vector<Retired> retired;

    Heap image_heap, buffer_heap;
    DkMemBlock descriptor_memory = nullptr;  // image then sampler descriptors
    std::vector<std::uint32_t> free_image_descriptors;
    std::uint32_t next_image_descriptor = 1;  // 0: the dummy image
    std::vector<std::pair<SamplerParams, std::uint32_t>> sampler_cache;
    struct UnitSamplerCache {
        bool valid = false;
        SamplerParams params;
        unsigned levels = 0;
        std::uint32_t descriptor = 0;
    };
    std::array<UnitSamplerCache,kUnits> unit_sampler_cache{};
    std::map<std::uint32_t,std::array<std::uint32_t,7>> blend_commands;
    bool descriptors_dirty = true;
    // Samplers read since the sampler set was last bound (sms-switch f73ec5b: on the console a sampler
    // descriptor first read partway through a list can come out stale, and InvalidateSamplerCache
    // does not help; binding the set again does). OPENRAC_DK_SAMPLER_REBIND=0 turns the rebinds off.
    std::vector<bool> sampler_read = std::vector<bool>(kSamplerDescriptors, false);
    // How many sampler descriptors existed when the set was last bound (begin_list or a refresh):
    // OPENRAC_DK_SAMPLER_REBIND=new refreshes only for a sampler written after that.
    std::uint32_t samplers_at_bind = 0;
    bool list_open = false;  // the current list has its descriptor sets bound
    ImagePtr dummy;               // 1x1 opaque black: what an unbound unit samples
    DkCmdBuf vertex_capture = nullptr; // CPU-only scratch encoder for the SDK 0.5 reservation workaround
    DkMemBlock constant_attributes = nullptr;  // (0,0,0,1) floats and ints for unfed inputs

    ImagePtr default_colour, default_depth;
    int default_width = 0, default_height = 0;

    Diagnostic diagnostic = Diagnostic::None;
    State state;
    Names<Texture> textures;
    Names<Buffer> buffers;
    Names<VertexArray> vertex_arrays;
    Names<Program> programs;  // programs and shaders share GL's name space
    Names<Shader> shaders;
    Names<Framebuffer> framebuffers;
    Names<Renderbuffer> renderbuffers;
    Names<SamplerParams> samplers;
    GLuint next_shader_name = 1;

    // What the command list holds now.
    Target bound_target{};
    bool target_bound = false;
    bool need_image_barrier = false;  // an image was written by a copy or a pass: invalidate before sampling
    std::uint64_t barrier_epoch = 1;  // counts barriers: work before one is ordered before work after
    // Counts only the barriers that both wait and invalidate the texture cache (Image): a target
    // write stamped in an earlier one is finished and visible to sampling. The lazy target
    // barriers (OPENRAC_DK_LAZY_TARGET_BARRIERS=1) read this, not barrier_epoch, so a future
    // barrier without the invalidation (before_transfer's Full, 0) can never let one be skipped.
    std::uint64_t ordered_epoch = 1;
    unsigned list_breaks = 0;         // barriers that end a GPFIFO entry in the current list
    // The copy engine (dkCmdBufCopyBufferToImage, dkCmdBufCopyImage: deko3d 0.5 BlitCopyEngine)
    // runs beside the 3D and 2D engines. DkBarrier_Primitives is only the 3D engine's WaitForIdle
    // ("completes ... primitives and compute jobs"); only DkBarrier_Full (the channel's
    // SetReference) orders copy-engine work against draws and 2D blits, either way round.
    // A skinned draw dkgl skinned on the CPU (OPENRAC_DK_CPU_SKINNING): positions and normals come
    // from `skinned` (24 bytes a vertex, vertex `first` first), the other per-vertex inputs start
    // at vertex `first`, the per-instance inputs at `instance`, and the draw's skinning is off.
    struct DrawOverride {
        bool active = false;
        bool normals = false;
        std::uint32_t first = 0, instance = 0, vertices = 0;
        DkGpuAddr skinned = 0;
    } draw_override;
    // What the GPU was last given, so a draw that repeats it sends nothing (OPENRAC_DK_STATE_CACHE,
    // default on). Forgotten whenever it might not hold: a new list (reset_bound_state), a new
    // target, a new program.
    struct Sent {
        bool valid = false;
        const void* program = nullptr;
        DkGpuAddr ubo = 0;
        std::uint32_t ubo_size = 0;
        std::array<DkResHandle, 32> textures[2]{};
        std::array<bool, 32> textures_valid[2]{};
        std::uint32_t vertex_count = ~0u;
        std::array<DkVtxAttribState, 16> attribs{};
        std::array<DkVtxBufferState, 16> states{};
        std::array<DkBufExtents, 16> buffers{};
        std::array<std::uint8_t, 160> fixed{};
        bool fixed_valid = false;
        int viewport[4] = {}, scissor[4] = {};
        bool view_valid = false;
        DkGpuAddr index = 0;
        int index_bytes = 0, restart = -1;
    } sent;
    std::uint64_t full_epoch = 1;   // counts Full barriers (and waits for an idle GPU)
    bool pending_transfer = false;  // a copy-engine write no Full barrier has ordered yet
    bool feedback = false;          // the draw being prepared samples its own colour target

    std::uint64_t accepted = 0;
    std::string error;  // sticky presentation error
    std::string device_name = "NVIDIA Tegra X1 (deko3d)", driver_version = "deko3d 0.5",
                api_version = "4.1 OpenRAC on deko3d";
    Counters counters;
};

inline Context* g = nullptr;

// 3D or 2D work reads or writes `image` now (see Image::touched_full_epoch).
inline void touch(Image* image) {
    if (image != nullptr) {
        image->touched_full_epoch = g->full_epoch;
    }
}

// ---- dkgl_core.cpp ----
DkMemBlock make_block(std::uint32_t size, std::uint32_t flags);
void retire(DkMemBlock block);
void retire_descriptor(std::uint32_t index);
Slot& slot();
DkCmdBuf cmd();
// Frame-ring space: CPU pointer and GPU address, `align`-aligned. Null pointer when full.
struct RingSlice {
    void* cpu = nullptr;
    DkGpuAddr gpu = 0;
};
RingSlice ring(std::uint32_t bytes, std::uint32_t align = 256);
// Images: made with a descriptor of their own.
ImagePtr make_image(DkImageFormat format, int width, int height, unsigned levels, bool render);
std::uint32_t sampler_descriptor(const SamplerParams& p, unsigned levels);
void write_image_descriptor(Image& image);
// Submits what is recorded and waits for the GPU (glFinish, glReadPixels).
void flush_and_wait();
// Barrier before sampling what a pass or a copy wrote.
void image_barrier();
// Copy-engine ordering. before_transfer: before the copy engine reads or writes `a` or `b`, a Full
// barrier when 3D/2D work since the last Full barrier (this frame or the one before, still on the
// GPU) touched either. after_transfer: after it wrote an image. transfer_fence: before 3D or 2D
// work, a Full barrier when a copy-engine write is still unordered.
// `source` only feeds the per-source counters (OPENRAC_DK_COUNTERS); ordering is the same for all.
// before_transfer returns whether it recorded the Full barrier (an image still busy).
enum class CopySource { Upload, Mipmap, TextureCopy, BlitColour, BlitDepth, Other };
bool before_transfer(const Image* a, const Image* b, CopySource source = CopySource::Other);
void after_transfer(CopySource source = CopySource::Other);
void transfer_fence();
// OPENRAC_DK_TRANSFER_SYNC=legacy: the earlier 3D-only ordering, for an A/B check on hardware.
bool legacy_transfer_sync();
// A barrier of `mode` with `invalidate`, counted; long lists are sent to the GPU in parts.
void barrier(DkBarrier mode, std::uint32_t invalidate);
// dkCmdBufBarrier with deko3d 0.5's texture cache invalidation fixed: for DkBarrier_Fragments /
// _Tiles with DkInvalidateFlags_Image it writes method 0x3DD, which the GPU does not have (NVIDIA's
// INVALIDATE_TEXTURE_DATA_CACHE is 0x4CE; sms-switch deko3d-texture-cache-invalidate.patch), so a
// target drawn and then sampled could come out of the texture cache stale. Recorded instead as
// Mesa's texture barrier: wait for idle, then the no-wait invalidation (0x4A2), which works.
// OPENRAC_DK_TEXINVAL=deko3d records deko3d's own sequence.
void cmd_barrier(DkCmdBuf c, DkBarrier mode, std::uint32_t invalidate);
bool load_program(Program& p, const std::string& vertex, const std::string& fragment);

// ---- dkgl_gl.cpp ----
Proc gl_proc(const char* name);
Target current_target(GLuint framebuffer);
void reset_bound_state();

}  // namespace openrac::dkgl::detail

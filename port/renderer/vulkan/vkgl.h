// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The renderer's OpenGL (renderer/gl.h, renderer/world/gl_world.h, and the few calls gl_stats.h
// adds) carried out with Vulkan: the port's Vulkan renderer. Every renderer is written against
// that short list of GL calls, so this is the one place that knows Vulkan; the renderers, their
// shaders and the frontend run on it unchanged (docs/port/ANDROID.md, "A Vulkan renderer"). It is
// what draws on a custom Vulkan driver (platform/gpu_driver.h), which no GLES driver can load.
//
// How GL's model becomes Vulkan's:
//   - GL objects are names in tables here; a texture is a VkImage, a buffer a CPU copy uploaded
//     where a draw needs it (into the frame's ring while it changes, into memory of its own once it
//     stays), a framebuffer a render pass begun at its first draw or clear.
//   - A program is compiled by glslang (shader_compiler.h); its loose uniforms are a block that
//     each draw that changed them copies into the frame's ring.
//   - The state a pipeline bakes in (blend, depth, stencil, cull, the vertex layout, the
//     primitive, the program, the render pass) is a key into a cache of pipelines; viewport,
//     scissor, blend constant and stencil reference are dynamic.
//   - Rows: both APIs put a framebuffer's row 0 at NDC y = -1, so draws, viewports, scissors,
//     reads and copies keep their numbers; the default framebuffer is an image of its own, turned
//     right way up when it is copied to the screen at present().
//   - Everything is recorded in one command buffer a frame, two frames in flight.
//
// One context at a time, on the thread that opened it, as the renderer uses GL.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

#include "renderer/gpu_draw.h"

struct SDL_Window;

namespace openrac::vkgl {

struct Config {
    // The window to present to (made with SDL_WINDOW_VULKAN), or null: drawing without a screen,
    // for the tests (glReadPixels reads the default framebuffer).
    SDL_Window* window = nullptr;
    int width = 0;   // the default framebuffer without a window
    int height = 0;
    bool vsync = true;
    // A Vulkan driver of the player's (platform/gpu_driver.h): its vkGetInstanceProcAddr; null for
    // the system's loader.
    void* get_instance_proc_addr = nullptr;
    bool validation = false;  // the Khronos validation layer, where it is installed
};

// Opens the device and the context. False, with why in `error`, when Vulkan cannot be used: the
// caller falls back to OpenGL.
bool open(const Config& config, std::string& error);
void close();
bool active();

// GPU culling/compaction and device-generated indexed draw arguments. No
// readback participates in visibility decisions. Handles last one recording
// (including readback flush boundaries), not merely one displayed frame.
bool gpu_driven_active();
bool gpu_materials_active();
// Indexed viewports route shadow draws to atlas rectangles from the vertex
// shader. Empty rectangles restore the ordinary single-viewport path.
bool indexed_viewports_supported(unsigned count);
void indexed_viewports(std::span<const std::array<int, 4>> rectangles);
renderer::GpuInstanceBatch gpu_instances(
    unsigned instance_buffer,
    std::span<const renderer::GpuCullBounds> bounds,
    const renderer::GpuCullView& view
);
bool draw_gpu_instances(
    renderer::GpuInstanceBatch batch, unsigned first_index, unsigned index_count
);

// Registers the instances of a static instance buffer once, with their culling bounds: while the
// buffer is unchanged and gpu-resident-instances is on, gpu_instances of it sends the GPU only the
// view and a group record, and the culling reads the instances from device memory
// (docs/port/RENDER_PERF.md, R-15). False when GPU-driven drawing is off or the sizes disagree.
bool gpu_register_instances(
    unsigned instance_buffer, std::span<const renderer::GpuCullBounds> bounds
);

// Forgets every registration (a level's release). The device copy goes once every frame that
// reads it has retired.
void gpu_release_instances();

struct GpuDrawCounters {
    std::uint64_t recording = 0, draws = 0, triangles = 0, visible_instances = 0;
    std::uint64_t readback_bytes = 0;
};

// Last completed collected recording, read after its existing frame-slot fence.
// Counters are collected only while stats are enabled; otherwise returns zero.
GpuDrawCounters gpu_completed_draws();

struct GpuMaterialCounters {
    bool supported = false;
    std::uint64_t draws = 0, records = 0, textures = 0, native_draws = 0;
};

GpuMaterialCounters gpu_material_counters();

// Copy an RGBA8 compute output to an allocated 2D texture without a readback.
// Includes compute-write -> transfer-read and image-sampling dependencies.
bool copy_buffer_to_texture(unsigned buffer, unsigned texture, int width, int height);

// renderer/gl.h's loader: the GL functions by name, null for one this does not carry out.
using Proc = void (*)();
Proc get_proc(const char* name);

// The kinds of Vulkan command the backend records, as SubmissionCounters::commands splits them
// (docs/port/RENDER_PERF.md, "The counters").
enum class Command : std::uint8_t {
    BindPipeline,     // vkCmdBindPipeline
    BindDescriptors,  // vkCmdBindDescriptorSets, vkCmdPushConstants
    BindBuffers,      // vkCmdBindVertexBuffers, vkCmdBindIndexBuffer
    DynamicState,     // vkCmdSetViewport, vkCmdSetScissor, vkCmdSetBlendConstants, vkCmdSetStencil*
    Draw,             // vkCmdDraw, vkCmdDrawIndexed, vkCmdDrawIndexedIndirect
    Barrier,          // vkCmdPipelineBarrier
    Copy,             // vkCmdCopy*, vkCmdBlitImage, vkCmdResolveImage
    Dispatch,         // vkCmdDispatch
    RenderPass,       // vkCmdBeginRendering, vkCmdEndRendering
    Query,            // vkCmdWriteTimestamp, vkCmdResetQueryPool
    Other,            // vkCmdClearAttachments and the debug labels
};

// The number of kinds: Other is the last enumerator of Command.
inline constexpr std::size_t kCommandKinds = static_cast<std::size_t>(Command::Other) + 1;

// Cumulative submission counters for profiling and offscreen regression tests.
// Zero without an active context; reset when a new context opens.
//
// The members from `commands` on are the render counters of docs/port/RENDER_PERF.md: exact counts
// and bytes, the same on every machine for the same work, which decide whether a render
// optimisation pays. Counting them is an add where the work is recorded; present() publishes them
// as the frame's vk_* gauges while the debug tools measure (common/stats.h).
struct SubmissionCounters {
    std::uint64_t validation_errors = 0;
    std::uint64_t uniform_writes = 0, uniform_writes_skipped = 0;
    std::uint64_t uniform_uploads = 0, uniform_upload_bytes = 0;
    std::uint64_t descriptor_sets = 0, descriptor_reuses = 0;
    std::uint64_t gpu_counter_dispatches = 0, gpu_counter_copies = 0;

    // The vkCmd* recorded, by Command; commands_recorded() adds them up.
    std::array<std::uint64_t, kCommandKinds> commands{};
    std::uint64_t pipelines_created = 0;  // Graphics and compute pipelines made (each a compile).
    // Pipelines made once the warm-up begin_pipeline_warmup started had passed.
    std::uint64_t pipelines_created_after_warmup = 0;
    std::uint64_t descriptor_sets_allocated = 0;  // Every vkAllocateDescriptorSets set.
    std::uint64_t descriptor_writes = 0;          // VkWriteDescriptorSet entries written.
    std::uint64_t render_passes = 0;              // vkCmdBeginRendering calls.
    // Bytes the passes read in from memory (load op LOAD) and write back (store op STORE, and a
    // resolve's target): each attachment's texel bytes times the render area times its samples.
    std::uint64_t attachment_load_bytes = 0;
    std::uint64_t attachment_store_bytes = 0;
    std::uint64_t barriers = 0;               // vkCmdPipelineBarrier calls.
    std::uint64_t barriers_all_commands = 0;  // Of those, the ones with an ALL_COMMANDS stage.
    std::uint64_t submits = 0;                // vkQueueSubmit calls.
    std::uint64_t frames_presented = 0;       // present() calls: frames ended.
    // Copies, blits and resolves whose extent is the default framebuffer's or the screen's.
    std::uint64_t fullscreen_copies = 0;
    std::uint64_t ring_bytes = 0;            // Bytes taken from the frames' upload rings.
    std::uint64_t texture_creations = 0;     // Images made: textures, render targets, mip chains.
    std::uint64_t texture_upload_bytes = 0;  // Texel bytes staged for upload into images.
    // Bytes of the buffers that stopped changing and got memory of their own, in memory the host
    // can see (VmaAllocationInfo's memory type), made so far.
    std::uint64_t buffer_static_bytes_host_visible = 0;
    std::uint64_t cpu_mirror_bytes = 0;  // Now: the CPU copies GL's buffers keep (glBufferData).
    // Bytes the CPU writes for the GPU-driven instances: each batch's payload as it is built, and
    // its copy into the mapped input page at the end of the recording.
    std::uint64_t gpu_payload_bytes = 0;
    std::uint64_t shader_compiles = 0;  // glslang runs: shaders checked, programs compiled.
    double shader_compile_s = 0.0;      // The seconds those compiles took.
};

SubmissionCounters submission_counters();

// Returns the commands of `counters` added up over every kind: vk_cmds_recorded.
inline std::uint64_t commands_recorded(const SubmissionCounters& counters) {
    std::uint64_t total = 0;
    for (const std::uint64_t kind : counters.commands) {
        total += kind;
    }
    return total;
}

// Starts a warm-up of `frames` frames now, for the benchmark after it uploads a level: a pipeline
// made once that many frames have been presented counts in pipelines_created_after_warmup as well.
// Until the first call no pipeline counts there.
void begin_pipeline_warmup(std::uint32_t frames);

// For the tests: with `on`, an attachment glInvalidateFramebuffer let go is overwritten with
// garbage (colour 1.0 in every channel, depth 0, stencil 0xA5) instead of being left undefined, so
// that a later read of it shows in the picture. False when no context is open.
bool test_poison_discards(bool on);

// Shows the default framebuffer and starts the next frame.
void present();

// The wait for the display, as platform::SwapInterval counts it: -1 adaptive, 0 none, 1 vsync.
// Returns what the screen allows: the nearest of those.
int set_swap_interval(int interval);

// The device and driver, for the log: "Adreno (TM) 740", "Qualcomm 512.744.0", "1.3.284".
const std::string& device_name();
const std::string& driver_version();
const std::string& api_version();
bool software();

// True when the open device has memory that is device-local and not host-visible, where a static
// buffer's staged copy goes (docs/port/RENDER_PERF.md, "R-05"). False with no context, and on a
// driver whose every memory type the host sees (lavapipe, some integrated GPUs).
bool device_only_memory();

}  // namespace openrac::vkgl

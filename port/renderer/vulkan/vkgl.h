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

// renderer/gl.h's loader: the GL functions by name, null for one this does not carry out.
using Proc = void (*)();
Proc get_proc(const char* name);

// Cumulative submission counters for profiling and offscreen regression tests.
// Zero without an active context; reset when a new context opens.
struct SubmissionCounters {
    std::uint64_t validation_errors = 0;
    std::uint64_t uniform_writes = 0, uniform_writes_skipped = 0;
    std::uint64_t uniform_uploads = 0, uniform_upload_bytes = 0;
    std::uint64_t descriptor_sets = 0, descriptor_reuses = 0;
    std::uint64_t gpu_counter_dispatches = 0, gpu_counter_copies = 0;
};

SubmissionCounters submission_counters();

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

}  // namespace openrac::vkgl

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The renderer's OpenGL (renderer/gl.h) carried out with deko3d, the Switch's native GPU API.
// Like renderer/vulkan's vkgl, every renderer and shader runs on it unchanged; unlike vkgl there
// is no driver between it and the GPU: no pipelines (state is set in the command stream), no
// descriptor sets (a texture is a 32-bit handle), and every draw is a few command words.
//
// How GL's model becomes deko3d's:
//   - Programs are precompiled. Each linked program is found by a hash of its sources in the
//     shader pack (romfs:/dkgl/<key>.{vert,frag}.dksh and .prog, made by
//     port/tools/deko3d/dk_pack.py from programs the Vulkan build dumped). Its loose uniforms are
//     one std140 block that each draw which changed them copies into the frame's ring. A program
//     the pack lacks fails to link, and its sources go to sdmc:/switch/openrac/dkgl-missing/.
//   - Rows as in vkgl (and Vulkan): a framebuffer's row 0 is at NDC y = -1, so draws, viewports,
//     scissors, reads and copies keep GL's numbers; the default framebuffer is an image of its
//     own, turned right way up when it is copied to the screen at present().
//   - A buffer is a CPU copy uploaded where a draw needs it: into the frame's ring while it
//     changes, into memory of its own once it stays.
//   - Everything is recorded in one command list a frame, two frames in flight.
//
// One context, one thread. Opened on the window's thread after the launcher released the display.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace openrac::dkgl {

struct Config {
    void* native_window = nullptr;  // the NWindow to present to (nwindowGetDefault())
    int width = 1280;
    int height = 720;
    bool vsync = true;
};

// Opens the device and the context. False, with why in `error`, when deko3d cannot be used.
bool open(const Config& config, std::string& error);
void close();
bool active();

// renderer/gl.h's loader: the GL functions by name, null for one this does not carry out.
using Proc = void (*)();
Proc get_proc(const char* name);

// Ends the frame: the default framebuffer to the screen, the frame's commands to the GPU.
void present();
// Accepted presents so far, and the sticky error that stopped presenting (empty while fine).
std::uint64_t accepted_presents();
const std::string& presentation_error();

// 1 for vsync, 0 for none; returns what the screen now does.
int set_swap_interval(int interval);

const std::string& device_name();
const std::string& driver_version();
const std::string& api_version();

// Per-frame counters for the log (OPENRAC_DK_COUNTERS=N logs them every N frames).
struct Counters {
    std::uint64_t vertex_setup_hits=0,vertex_setup_misses=0;
    std::uint64_t sampler_lookup_hits=0,sampler_lookup_misses=0,sampler_layout_rebuilds=0;
    std::uint64_t target_barriers_skipped=0;
    std::uint64_t texture_bind_calls=0,texture_bind_slots=0;
    // Copy-engine transfers by source (each also counts in engine_copies), and Full barriers by
    // cause (each also counts in full_barriers; the rest are "other").
    std::uint64_t copy_uploads=0,copy_uploads_busy=0,copy_mipmap=0,copy_texture=0,copy_blit_colour=0,
                  copy_blit_depth=0,copy_other=0;
    std::uint64_t full_before_transfer=0,full_transfer_fence=0,full_mipmap=0,full_readback=0;
    std::uint64_t frames = 0, draws = 0, uniform_uploads = 0, ring_bytes = 0, texture_uploads = 0,
                  barriers = 0, target_changes = 0, flushes = 0, missing_programs = 0, diagnostic_omissions = 0,
                  full_barriers = 0, engine_copies = 0, texture_invalidations = 0,
                  sampler_rebinds = 0;
};
Counters counters();

// Per-pass GPU timestamps, OPENRAC_DK_PASS_TIMING=N (30..3600: a report every N frames; off by
// default). pass_begin/pass_end bracket a named stretch of GL commands on the GL thread;
// stats::Scope with kGpu forwards to them once installed (platform/switch/graphics.cpp). Each mark
// is a timestamp written when the 3D pipe's earlier work has left the ROP (deko3d DkCounter_
// Timestamp), read back after the slot's own fence: no added wait. Bounded per frame.
void pass_begin(std::string_view name);
void pass_end();
bool pass_timing_enabled();

}  // namespace openrac::dkgl

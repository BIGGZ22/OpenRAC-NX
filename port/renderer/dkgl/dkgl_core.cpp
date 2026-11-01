// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// dkgl's device, memory, frames, presentation and shader pack (dkgl.h).

#include <switch.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "common/env_interval.h"
#include "common/log.h"
#include "renderer/dkgl/dkgl_internal.h"

namespace openrac::dkgl::detail {

namespace {

void driver_message(void*, const char* context, DkResult result, const char* message) {
    // May run on a driver thread: only the log file.
    if (std::FILE* f = std::fopen("sdmc:/switch/openrac/dkgl-driver.log", "a")) {
        std::fprintf(f, "%s: result %u: %s\n", context ? context : "deko3d", unsigned(result),
                     message ? message : "");
        std::fclose(f);
    }
}

// More command memory when a frame's list outgrows what it has.
void add_command_memory(void*, DkCmdBuf cmdbuf, size_t needed) {
    Slot& s = slot();
    const std::uint32_t size = std::uint32_t(align_up(std::max<size_t>(needed, kCmdChunk), DK_MEMBLOCK_ALIGNMENT));
    if (s.cmd_used >= s.cmd_memory.size()) {
        DkMemBlock block = make_block(size, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
        if (block == nullptr) {
            g->error = "deko3d: out of command memory";
            return;
        }
        s.cmd_memory.push_back(block);
    }
    DkMemBlock block = s.cmd_memory[s.cmd_used++];
    dkCmdBufAddMemory(cmdbuf, block, 0, dkMemBlockGetSize(block));
}

// Frees what the GPU has finished with.
void collect() {
    auto& list = g->retired;
    auto keep = list.begin();
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (it->frame <= g->completed) {
            if (it->block != nullptr) {
                dkMemBlockDestroy(it->block);
            }
            if (it->piece) {
                release(it->piece);
            }
            if (it->image_descriptor != ~0u) {
                g->free_image_descriptors.push_back(it->image_descriptor);
            }
        } else {
            *keep++ = *it;
        }
    }
    list.erase(keep, list.end());
}

// Starts recording into the current slot: waits for its last frame, resets its memory.
double steady_ms() {
    return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// GPU command span of a finished frame (OPENRAC_DK_TIMING), read only after the slot's own fence.
// Each report is deko3d 0.5's FourWords structure: a 64-bit counter, then the 64-bit timestamp at
// +8, in GPU ticks that dkTimestampToNs converts (625/384 ns a tick, in the SDK header).
//   top: frame start as the command processor reached it (night100's convention). With two frames
//        in flight it can precede the end of the previous frame's 3D work, so spans may overlap.
//   eop: frame start as it left the 3D pipe, i.e. after the previous frame's 3D work drained.
//   end: after the present blit, before the final submit. A report on the 3D engine does not wait
//        for the 2D engine's blit or the copy engine, so their tails are not guaranteed inside it.
void read_timing(Slot& s) {
    if(!s.timing_pending || !s.timing) return;
    s.timing_pending=false;
    const auto* data=static_cast<const std::uint8_t*>(dkMemBlockGetCpuAddr(s.timing));
    std::uint64_t top=0,end=0,eop=0,scene=0;
    std::memcpy(&top,data+8,8);std::memcpy(&end,data+24,8);std::memcpy(&eop,data+40,8);
    std::memcpy(&scene,data+56,8);
    const auto valid=[](std::uint64_t a,std::uint64_t b) {
        return a!=~0ull && b!=~0ull && a!=0 && b>=a && b-a<=dkNsToTimestamp(60000000000ull);
    };
    const auto ms=[](std::uint64_t a,std::uint64_t b) { return double(dkTimestampToNs(b-a))/1.0e6; };
    if(!valid(top,end)) {
        if(++g->timing_rejected<=4)
            log::warn("dk timing rejected frame {} raw top {:#x} end {:#x} eop {:#x}",s.frame,top,end,eop);
        g->last_end_ts=0;  // no gap across an unknown frame
        return;
    }
    g->gpu_span.add(ms(top,end));
    if(valid(top,eop) && valid(eop,end))
        g->gpu_span_eop.add(ms(eop,end));
    if(valid(top,scene) && valid(scene,end)) {
        g->scene_span.add(ms(top,scene));
        g->present_tail.add(ms(scene,end));
    }
    // Frames are read in order (each slot after its own fence), so the last end read is the
    // previous frame's. A top before it means the two frames overlapped in the pipe: no gap.
    if(g->last_end_ts!=0) {
        if(top>=g->last_end_ts && top-g->last_end_ts<=dkNsToTimestamp(1000000000ull)) g->gpu_gap.add(ms(g->last_end_ts,top));
        else if(top<g->last_end_ts) ++g->gap_overlaps;
    }
    g->last_end_ts=end;
}

// One line every OPENRAC_DK_TIMING presented frames, counted by frames (not by valid GPU samples),
// so the CPU-side waits are still reported when every timestamp is rejected.
void report_timing() {
    if(!g->timing_every || g->slot_wait.values.size()<unsigned(g->timing_every)) return;
    log::info("dk timing {} frames: GPU command span top->end avg {:.3f} p95 {:.3f} max {:.3f} ms ({} samples); "
              "eop->end avg {:.3f} p95 {:.3f} ms ({} samples); CPU slot-fence wait avg {:.3f} p95 {:.3f}; "
              "acquire avg {:.3f}; CPU present-to-present avg {:.3f} p95 {:.3f} ms; rejected {} "
              "(spans are not GPU busy time; present interval is measured at acquire, not scanout)",
        g->slot_wait.values.size(),g->gpu_span.mean(),g->gpu_span.p95(),g->gpu_span.maximum(),g->gpu_span.values.size(),
        g->gpu_span_eop.mean(),g->gpu_span_eop.p95(),g->gpu_span_eop.values.size(),
        g->slot_wait.mean(),g->slot_wait.p95(),g->acquire_wait.mean(),g->present_interval.mean(),
        g->present_interval.p95(),g->timing_rejected);
    log::info("dk timing pacing: GPU gap end->next top avg {:.3f} p95 {:.3f} max {:.3f} ms ({} samples, {} overlapped); "
              "scene top->scene end avg {:.3f} p95 {:.3f}; present tail scene end->end avg {:.3f} p95 {:.3f} ms "
              "(the gap holds what the queue waited for between frames, such as a screen image; "
              "with OPENRAC_DK_LATE_ACQUIRE=1 that wait moves into the present tail)",
        g->gpu_gap.mean(),g->gpu_gap.p95(),g->gpu_gap.maximum(),g->gpu_gap.values.size(),g->gap_overlaps,
        g->scene_span.mean(),g->scene_span.p95(),g->present_tail.mean(),g->present_tail.p95());
    g->gpu_span.clear();g->gpu_span_eop.clear();g->slot_wait.clear();g->acquire_wait.clear();g->present_interval.clear();
    g->gpu_gap.clear();g->scene_span.clear();g->present_tail.clear();g->gap_overlaps=0;
}

// ---- Per-pass GPU timestamps (OPENRAC_DK_PASS_TIMING) ----

// The pairs a finished slot recorded, read after its own fence (never an added wait).
void read_passes(Slot& s) {
    if(!g->pass_every) return;
    if(s.pass) {
        const auto* data=static_cast<const std::uint8_t*>(dkMemBlockGetCpuAddr(s.pass));
        for(const auto& p:s.pass_pairs) {
            std::uint64_t a=0,b=0;
            std::memcpy(&a,data+16*p.begin+8,8);std::memcpy(&b,data+16*p.end+8,8);
            if(a==~0ull || b==~0ull || a==0 || b<a || b-a>dkNsToTimestamp(1000000000ull)) {
                ++g->pass_rejected;
                continue;
            }
            const double ms=double(dkTimestampToNs(b-a))/1.0e6;
            if(g->pass_stats) {
                // The counter word (+0) of the reports after each mark's timestamp.
                std::array<double,kPassStatCount> d{};
                bool ok=true;
                for(unsigned k=0;k<kPassStatCount && ok;++k) {
                    std::uint64_t c0=0,c1=0;
                    std::memcpy(&c0,data+16*(p.begin+1+k),8);std::memcpy(&c1,data+16*(p.end+1+k),8);
                    ok=c0!=~0ull && c1!=~0ull && c1>=c0;
                    d[k]=double(c1-c0);
                }
                if(ok) {
                    for(unsigned k=0;k<kPassStatCount;++k) g->pass_stat[p.name][k]+=d[k];
                } else {
                    ++g->pass_stat_rejected;
                }
            }
            g->pass_ms[p.name]+=ms;
            ++g->pass_calls[p.name];
            g->pass_depth[p.name]=std::min(g->pass_depth[p.name],int(p.depth));
            if(p.depth==0) g->pass_top_ms+=ms;
        }
        std::memset(dkMemBlockGetCpuAddr(s.pass),0xff,g->pass_capacity*16);
    }
    s.pass_pairs.clear();
    s.pass_reports=0;
    ++g->pass_frames;
}

void report_passes() {
    if(!g->pass_every || g->pass_frames<std::uint64_t(g->pass_every)) return;
    const double f=double(g->pass_frames);
    log::info("dk pass timing over {} frames: top-level passes {:.3f} ms/frame; dropped {} rejected {} unbalanced {} "
              "(ROP-end timestamps around stats kGpu sections, read after the slot fence; a pass includes its nested "
              "passes; boundaries blur where the pipe overlaps passes; work outside any pass is not attributed; "
              "the 2D-engine present blit may end after its mark)",
              g->pass_frames,g->pass_top_ms/f,g->pass_dropped,g->pass_rejected,g->pass_unbalanced);
    std::vector<std::size_t> order;
    for(std::size_t i=0;i<g->pass_names.size();++i) if(g->pass_calls[i]) order.push_back(i);
    std::sort(order.begin(),order.end(),[](std::size_t a,std::size_t b) { return g->pass_ms[a]>g->pass_ms[b]; });
    if(g->pass_stats) {
        log::info("dk pass statistics (OPENRAC_DK_PASS_STATS): counts between a pass's marks, per frame; a 0 can be a "
                  "counter this GPU does not count; fragment invocations above samples passed are fragments shaded "
                  "and then discarded or failing depth; {} pairs rejected",g->pass_stat_rejected);
    }
    for(std::size_t k=0;k<order.size() && k<24;++k) {
        const auto i=order[k];
        if(g->pass_stats) {
            const auto& st=g->pass_stat[i];
            log::info("  dk pass {}: {:.3f} ms/frame, {:.1f} calls/frame, depth {}; vertex shader {:.0f}, primitives "
                      "into clipper {:.0f} out {:.0f}, fragment shader {:.0f}, samples passed {:.0f}",
                      g->pass_names[i],g->pass_ms[i]/f,double(g->pass_calls[i])/f,g->pass_depth[i],
                      st[0]/f,st[1]/f,st[2]/f,st[3]/f,st[4]/f);
        } else {
            log::info("  dk pass {}: {:.3f} ms/frame, {:.1f} calls/frame, depth {}",
                      g->pass_names[i],g->pass_ms[i]/f,double(g->pass_calls[i])/f,g->pass_depth[i]);
        }
    }
    for(auto& st:g->pass_stat) st.fill(0.0);
    g->pass_stat_rejected=0;
    std::fill(g->pass_ms.begin(),g->pass_ms.end(),0.0);
    std::fill(g->pass_calls.begin(),g->pass_calls.end(),0);
    std::fill(g->pass_depth.begin(),g->pass_depth.end(),1<<30);
    g->pass_top_ms=0;g->pass_frames=0;g->pass_dropped=0;g->pass_rejected=0;g->pass_unbalanced=0;
}

void begin_slot() {
    Slot& s = slot();
    if (s.submitted) {
        const auto wait_start=g->timing_every ? steady_ms() : 0;
        dkFenceWait(&s.fence, -1);
        if(g->timing_every) g->slot_wait.add(steady_ms()-wait_start);
        read_timing(s); // existing fence guarantees completion; never add a diagnostic wait
        report_timing();
        read_passes(s);
        report_passes();
        s.submitted = false;
        g->completed = std::max(g->completed, s.frame);
    }
    collect();
    s.frame = g->frame;
    s.timing_started=false;
    if(s.timing) std::memset(dkMemBlockGetCpuAddr(s.timing),0xff,64);
    // A pass still open across a present (none should be) is not timed: its end is ignored.
    g->pass_unbalanced+=g->pass_stack.size();
    g->pass_stack.clear();
    s.ring_used = 0;
    ++g->ring_epoch;
    dkCmdBufClear(s.cmd);
    s.cmd_used = 0;
    add_command_memory(nullptr, s.cmd, kCmdChunk);
    g->descriptors_dirty = true;
    reset_bound_state();
}

// Records the per-list setup every list needs: descriptor sets, invalidations.
void begin_list() {
    DkCmdBuf c = slot().cmd;
    const DkGpuAddr base = dkMemBlockGetGpuAddr(g->descriptor_memory);
    dkCmdBufBindImageDescriptorSet(c, base, kImageDescriptors);
    dkCmdBufBindSamplerDescriptorSet(
        c, base + kImageDescriptors * sizeof(DkImageDescriptor), kSamplerDescriptors
    );
    // CPU writes (ring, descriptors) must not meet stale GPU cache lines of an earlier frame.
    dkCmdBufBarrier(
        c, DkBarrier_None,
        DkInvalidateFlags_Descriptors | DkInvalidateFlags_Image | DkInvalidateFlags_Shader
            | DkInvalidateFlags_L2Cache
    );
    g->descriptors_dirty = false;
    std::fill(g->sampler_read.begin(), g->sampler_read.end(), false);
    g->samplers_at_bind = std::uint32_t(g->sampler_cache.size());
    g->list_open = true;
    if(g->timing_every && slot().timing && !slot().timing_started) {
        dkCmdBufReportCounter(c,DkCounter_TimestampPipelineTop,dkMemBlockGetGpuAddr(slot().timing));
        dkCmdBufReportCounter(c,DkCounter_Timestamp,dkMemBlockGetGpuAddr(slot().timing)+32);
        slot().timing_started=true;
    }
}

// Ends the current list and hands it to the queue, with the slot's fence after it.
void submit_list() {
    Slot& s = slot();
    g->list_open = false;
    g->list_breaks = 0;
    DkCmdList list = dkCmdBufFinishList(s.cmd);
    dkQueueSubmitCommands(g->queue, list);
    dkQueueSignalFence(g->queue, &s.fence, true);
    dkQueueFlush(g->queue);
    s.submitted = true;
}

ImagePtr default_target(DkImageFormat format, int w, int h) {
    return make_image(format, w, h, 1, true);
}

bool make_default_framebuffer(int w, int h) {
    g->default_colour = default_target(DkImageFormat_RGBA8_Unorm, w, h);
    g->default_depth = default_target(DkImageFormat_Z24S8, w, h);
    g->default_width = w;
    g->default_height = h;
    return g->default_colour && g->default_depth;
}

bool make_swapchain(std::string& error) {
    DkImageLayoutMaker maker;
    dkImageLayoutMakerDefaults(&maker, g->device);
    maker.flags = DkImageFlags_UsageRender | DkImageFlags_UsagePresent | DkImageFlags_Usage2DEngine;
    maker.format = DkImageFormat_RGBA8_Unorm;
    maker.dimensions[0] = std::uint32_t(g->screen_width);
    maker.dimensions[1] = std::uint32_t(g->screen_height);
    DkImageLayout layout;
    dkImageLayoutInitialize(&layout, &maker);
    const std::uint64_t stride = align_up(dkImageLayoutGetSize(&layout), dkImageLayoutGetAlignment(&layout));
    g->swap_memory = make_block(std::uint32_t(stride * kSwapImages), DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image);
    if (g->swap_memory == nullptr) {
        error = "deko3d: no memory for the screen's images";
        return false;
    }
    std::array<const DkImage*, kSwapImages> images{};
    for (unsigned i = 0; i < kSwapImages; ++i) {
        dkImageInitialize(&g->swap_images[i], &layout, g->swap_memory, std::uint32_t(i * stride));
        images[i] = &g->swap_images[i];
    }
    DkSwapchainMaker swap;
    dkSwapchainMakerDefaults(&swap, g->device, g->window, images.data(), kSwapImages);
    g->swapchain = dkSwapchainCreate(&swap);
    if (g->swapchain == nullptr) {
        error = "deko3d: no swapchain";
        return false;
    }
    dkSwapchainSetSwapInterval(g->swapchain, std::uint32_t(g->interval));
    return true;
}

}  // namespace

Image::~Image() {
    if (g == nullptr) {
        return;
    }
    Retired r;
    r.frame = g->frame;
    r.piece = memory;
    r.image_descriptor = descriptor != 0 ? descriptor : ~0u;
    g->retired.push_back(r);
}

DkMemBlock Allocation::block() const {
    return heap->chunks[chunk].block;
}

DkGpuAddr Allocation::gpu() const {
    return dkMemBlockGetGpuAddr(block()) + offset;
}

void* Allocation::cpu() const {
    return static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(block())) + offset;
}

Allocation allocate(Heap& heap, std::uint32_t size, std::uint32_t align) {
    size = std::uint32_t(align_up(std::max<std::uint32_t>(size, 1), 256));
    align = std::max<std::uint32_t>(align, 256);
    // First fit in the blocks there are.
    for (std::uint32_t c = 0; c < heap.chunks.size(); ++c) {
        auto& free = heap.chunks[c].free;
        for (auto it = free.begin(); it != free.end(); ++it) {
            const std::uint32_t start = it->first, length = it->second;
            const auto at = std::uint32_t(align_up(start, align));
            if (at + std::uint64_t(size) > std::uint64_t(start) + length) {
                continue;
            }
            free.erase(it);
            if (at > start) {
                free.emplace(start, at - start);
            }
            if (at + size < start + length) {
                free.emplace(at + size, start + length - (at + size));
            }
            return {&heap, c, at, size};
        }
    }
    // A new block: the usual size, or one of the allocation's own size when it is larger.
    const auto block_size = std::uint32_t(align_up(std::max<std::uint64_t>(heap.chunk_size, std::uint64_t(size) + align), DK_MEMBLOCK_ALIGNMENT));
    DkMemBlock block = make_block(block_size, heap.flags);
    if (block == nullptr) {
        log::error("dkgl: no memory for a {} KiB block ({} blocks so far)", block_size >> 10, heap.chunks.size());
        return {};
    }
    Heap::Chunk chunk;
    chunk.block = block;
    chunk.size = block_size;
    chunk.free.emplace(0u, block_size);
    heap.chunks.push_back(std::move(chunk));
    return allocate(heap, size, align);
}

void release(const Allocation& a) {
    if (!a) {
        return;
    }
    auto& free = a.heap->chunks[a.chunk].free;
    std::uint32_t start = a.offset, length = a.size;
    // Joined with the free pieces on either side.
    auto next = free.lower_bound(start);
    if (next != free.end() && next->first == start + length) {
        length += next->second;
        next = free.erase(next);
    }
    if (next != free.begin()) {
        auto before = std::prev(next);
        if (before->first + before->second == start) {
            start = before->first;
            length += before->second;
            free.erase(before);
        }
    }
    free.emplace(start, length);
}

void retire(const Allocation& a) {
    if (a) {
        Retired r;
        r.frame = g->frame;
        r.piece = a;
        g->retired.push_back(r);
    }
}

DkMemBlock make_block(std::uint32_t size, std::uint32_t flags) {
    DkMemBlockMaker maker;
    dkMemBlockMakerDefaults(&maker, g->device, std::uint32_t(align_up(size, DK_MEMBLOCK_ALIGNMENT)));
    maker.flags = flags;
    return dkMemBlockCreate(&maker);
}

void retire(DkMemBlock block) {
    if (block != nullptr) {
        Retired r;
        r.frame = g->frame;
        r.block = block;
        g->retired.push_back(r);
    }
}

Slot& slot() {
    return g->slots[g->slot];
}

DkCmdBuf cmd() {
    if (!g->list_open) {
        begin_list();
    } else if (g->descriptors_dirty) {
        // A descriptor written since: the descriptor cache forgets what it held.
        dkCmdBufBarrier(slot().cmd, DkBarrier_None, DkInvalidateFlags_Descriptors);
        g->descriptors_dirty = false;
    }
    return slot().cmd;
}

RingSlice ring(std::uint32_t bytes, std::uint32_t align) {
    Slot& s = slot();
    if (bytes > kRingBytes) {
        log::error("dkgl: {} bytes do not fit the frame ring", bytes);
        return {};
    }
    const std::uint32_t at = std::uint32_t(align_up(s.ring_used, align));
    if (std::uint64_t(at) + bytes > kRingBytes) {
        // Full: what is recorded goes to the GPU and is waited for, and the ring starts again.
        flush_and_wait();
        return ring(bytes, align);
    }
    s.ring_used = at + bytes;
    g->counters.ring_bytes += bytes;
    auto* base = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(s.ring));
    return {base + at, dkMemBlockGetGpuAddr(s.ring) + at};
}

void write_image_descriptor(Image& image) {
    DkImageView view;
    dkImageViewDefaults(&view, &image.image);
    auto* set = static_cast<DkImageDescriptor*>(dkMemBlockGetCpuAddr(g->descriptor_memory));
    dkImageDescriptorInitialize(&set[image.descriptor], &view, false, false);
    g->descriptors_dirty = true;
}

ImagePtr make_image(DkImageFormat format, int width, int height, unsigned levels, bool render) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }
    auto image = std::make_shared<Image>();
    image->format = format;
    image->width = width;
    image->height = height;
    image->levels = std::max(1u, levels);
    image->depth = format == DkImageFormat_Z24S8 || format == DkImageFormat_Z24X8
                   || format == DkImageFormat_ZF32 || format == DkImageFormat_Z16
                   || format == DkImageFormat_ZF32_X24S8;
    image->stencil = format == DkImageFormat_Z24S8 || format == DkImageFormat_ZF32_X24S8;
    DkImageLayoutMaker maker;
    dkImageLayoutMakerDefaults(&maker, g->device);
    maker.flags = render || image->depth ? DkImageFlags_UsageRender : 0;
    if (!image->depth) {
        maker.flags |= DkImageFlags_Usage2DEngine;
    }
    maker.format = format;
    maker.dimensions[0] = std::uint32_t(width);
    maker.dimensions[1] = std::uint32_t(height);
    maker.mipLevels = image->levels;
    dkImageLayoutInitialize(&image->layout, &maker);
    const std::uint64_t size = dkImageLayoutGetSize(&image->layout);
    image->memory = allocate(g->image_heap, std::uint32_t(size), dkImageLayoutGetAlignment(&image->layout));
    if (!image->memory) {
        log::error("dkgl: no memory for a {}x{} image", width, height);
        image->descriptor = 0;
        return nullptr;
    }
    dkImageInitialize(&image->image, &image->layout, image->memory.block(), image->memory.offset);
    if (!g->free_image_descriptors.empty()) {
        image->descriptor = g->free_image_descriptors.back();
        g->free_image_descriptors.pop_back();
    } else if (g->next_image_descriptor < kImageDescriptors) {
        image->descriptor = g->next_image_descriptor++;
    } else {
        log::error("dkgl: out of image descriptors");
        image->descriptor = 0;
        return image;
    }
    write_image_descriptor(*image);
    return image;
}

std::uint32_t sampler_descriptor(const SamplerParams& p, unsigned levels) {
    SamplerParams key = p;
    if (levels <= 1) {
        // No mip levels to choose between: GL's mipmapped minifications sample level 0.
        key.min = (p.min == GL_NEAREST || p.min == 0x2700 || p.min == 0x2702) ? GL_NEAREST : GL_LINEAR;
    }
    for (const auto& [params, index] : g->sampler_cache) {
        if (params == key) {
            return index;
        }
    }
    const auto index = std::uint32_t(g->sampler_cache.size());
    if (index >= kSamplerDescriptors) {
        return 0;
    }
    DkSampler s;
    dkSamplerDefaults(&s);
    const auto filter = [](GLenum f) {
        return f == GL_NEAREST || f == 0x2700 || f == 0x2702 ? DkFilter_Nearest : DkFilter_Linear;
    };
    s.magFilter = key.mag == GL_NEAREST ? DkFilter_Nearest : DkFilter_Linear;
    s.minFilter = filter(key.min);
    // GL_NEAREST_MIPMAP_NEAREST 0x2700, GL_LINEAR_MIPMAP_NEAREST 0x2701,
    // GL_NEAREST_MIPMAP_LINEAR 0x2702, GL_LINEAR_MIPMAP_LINEAR 0x2703.
    s.mipFilter = key.min == 0x2700 || key.min == 0x2701   ? DkMipFilter_Nearest
                  : key.min == 0x2702 || key.min == 0x2703 ? DkMipFilter_Linear
                                                           : DkMipFilter_None;
    const auto wrap = [](GLenum w) {
        return w == GL_CLAMP_TO_EDGE ? DkWrapMode_ClampToEdge
               : w == 0x8370         ? DkWrapMode_MirroredRepeat
                                     : DkWrapMode_Repeat;
    };
    s.wrapMode[0] = wrap(key.wrap_s);
    s.wrapMode[1] = wrap(key.wrap_t);
    s.wrapMode[2] = DkWrapMode_ClampToEdge;
    s.maxAnisotropy = std::clamp(key.anisotropy, 1.0f, 16.0f);
    auto* set = reinterpret_cast<DkSamplerDescriptor*>(
        static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(g->descriptor_memory))
        + kImageDescriptors * sizeof(DkImageDescriptor)
    );
    dkSamplerDescriptorInitialize(&set[index], &s);
    g->sampler_cache.emplace_back(key, index);
    g->descriptors_dirty = true;
    return index;
}

void cmd_barrier(DkCmdBuf c, DkBarrier mode, std::uint32_t invalidate) {
    static const bool deko3d_own = [] {
        const char* v = std::getenv("OPENRAC_DK_TEXINVAL");
        const bool own = v != nullptr && std::string_view(v) == "deko3d";
        log::info("dkgl: texture cache invalidated {}", own
            ? "by deko3d's sequence (OPENRAC_DK_TEXINVAL=deko3d; method 0x3DD, which the GPU lacks)"
            : "as Mesa does after drawing: wait for idle, then 0x4A2 (OPENRAC_DK_TEXINVAL=deko3d for deko3d's)");
        return own;
    }();
    if (!deko3d_own && (invalidate & DkInvalidateFlags_Image) != 0
        && (mode == DkBarrier_Fragments || mode == DkBarrier_Tiles)) {
        // DkBarrier_Primitives is a wait for idle (all 3D work, fragments included), after which
        // deko3d writes the no-wait invalidation; the other flags go with it unchanged.
        dkCmdBufBarrier(c, DkBarrier_Primitives, invalidate);
        ++g->counters.texture_invalidations;
        return;
    }
    dkCmdBufBarrier(c, mode, invalidate);
}

void barrier(DkBarrier mode, std::uint32_t invalidate) {
    cmd_barrier(cmd(), mode, invalidate);
    ++g->counters.barriers;
    ++g->barrier_epoch;
    if (mode != DkBarrier_None && (invalidate & DkInvalidateFlags_Image) != 0) {
        ++g->ordered_epoch;
    }
    if (mode == DkBarrier_Full) {
        // Everything recorded before it, on every engine and from earlier lists, is complete.
        ++g->full_epoch;
        g->pending_transfer = false;
        ++g->counters.full_barriers;
    }
    // Each barrier that waits ends a GPFIFO entry, and the queue holds a few thousand entries
    // until a flush (deko3d 0.5 stops the program past that). A long list goes to the GPU in parts:
    // the GPU runs them in order, and the slot's fence still follows the last.
    if (mode != DkBarrier_None && ++g->list_breaks >= 256) {
        Slot& s = slot();
        DkCmdList list = dkCmdBufFinishList(s.cmd);
        dkQueueSubmitCommands(g->queue, list);
        dkQueueFlush(g->queue);
        g->list_breaks = 0;
        g->list_open = false;
        reset_bound_state();
    }
}

void image_barrier() {
    barrier(DkBarrier_Primitives, DkInvalidateFlags_Image);
    g->need_image_barrier = false;
}

bool legacy_transfer_sync() {
    static const bool legacy = [] {
        const char* v = std::getenv("OPENRAC_DK_TRANSFER_SYNC");
        const bool on = v != nullptr && std::string_view(v) == "legacy";
        if (on) {
            log::warn("dkgl: OPENRAC_DK_TRANSFER_SYNC=legacy: copy-engine transfers ordered by 3D barriers only (A/B test)");
        }
        return on;
    }();
    return legacy;
}

bool before_transfer(const Image* a, const Image* b, CopySource source) {
    const bool busy = (a != nullptr && a->touched_full_epoch == g->full_epoch)
                      || (b != nullptr && b->touched_full_epoch == g->full_epoch);
    if (busy) {
        barrier(legacy_transfer_sync() ? DkBarrier_Primitives : DkBarrier_Full, 0);
        if (!legacy_transfer_sync()) {
            ++g->counters.full_before_transfer;
        }
        if (source == CopySource::Upload) {
            ++g->counters.copy_uploads_busy;
        }
    }
    return busy;
}

void after_transfer(CopySource source) {
    g->pending_transfer = true;
    g->need_image_barrier = true;
    ++g->counters.engine_copies;
    switch (source) {
        case CopySource::Upload: ++g->counters.copy_uploads; break;
        case CopySource::Mipmap: ++g->counters.copy_mipmap; break;
        case CopySource::TextureCopy: ++g->counters.copy_texture; break;
        case CopySource::BlitColour: ++g->counters.copy_blit_colour; break;
        case CopySource::BlitDepth: ++g->counters.copy_blit_depth; break;
        case CopySource::Other: ++g->counters.copy_other; break;
    }
}

void transfer_fence() {
    if (!g->pending_transfer) {
        return;
    }
    if (legacy_transfer_sync()) {
        g->pending_transfer = false;  // the draw's image_barrier (3D only) follows, as before
        return;
    }
    barrier(DkBarrier_Full, DkInvalidateFlags_Image);
    ++g->counters.full_transfer_fence;
    g->need_image_barrier = false;
}

void flush_and_wait() {
    ++g->counters.flushes;
    submit_list();
    Slot& s = slot();
    dkFenceWait(&s.fence, -1);
    s.submitted = false;
    // Everything before this frame is done too: one queue, in order.
    g->completed = std::max(g->completed, g->frame - 1);
    // The GPU is idle: no engine's work is outstanding.
    ++g->full_epoch;
    g->pending_transfer = false;
    s.ring_used = 0;
    ++g->ring_epoch;
    dkCmdBufClear(s.cmd);
    s.cmd_used = 0;
    add_command_memory(nullptr, s.cmd, kCmdChunk);
    g->descriptors_dirty = true;
    reset_bound_state();
}

// ---- The shader pack ----

namespace {

std::uint64_t fnv(std::string_view a, std::string_view b) {
    std::uint64_t h = 0xcbf29ce484222325u;
    const auto mix = [&](std::string_view s) {
        for (unsigned char c : s) {
            h = (h ^ c) * 0x100000001b3u;
        }
        h = (h ^ 0u) * 0x100000001b3u;  // each part ended by a NUL
    };
    mix(a);
    mix(b);
    return h;
}

bool read_file(const std::string& path, std::vector<std::uint8_t>& out) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (f == nullptr) {
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    out.resize(size > 0 ? std::size_t(size) : 0);
    const bool ok = size > 0 && std::fread(out.data(), 1, out.size(), f) == out.size();
    std::fclose(f);
    return ok;
}

void write_missing(std::uint64_t key, const std::string& vertex, const std::string& fragment) {
    const std::string dir = "sdmc:/switch/openrac/dkgl-missing";
    std::error_code ignored;
    std::filesystem::create_directories(dir, ignored);
    const std::string base = std::format("{}/{:016x}", dir, key);
    for (const auto& [ext, text] : {std::pair{".vert", &vertex}, std::pair{".frag", &fragment}}) {
        if (std::FILE* f = std::fopen((base + ext).c_str(), "wb")) {
            std::fwrite(text->data(), 1, text->size(), f);
            std::fclose(f);
        }
    }
}

}  // namespace

// Which shader pack programs load from. romfs:/dkgl is compiled with uam's scheduling fixed (no dual
// issue, stall before a new block, initialised sched words: port/tools/deko3d/uam-sched-fix.patch).
// OPENRAC_DK_PACK=stock loads romfs:/dkgl-stock, the same programs from devkitPro's unpatched uam
// (the pack used up to 2026-10-29), to compare on hardware. Only scheduling words differ.
static const std::string& pack_dir() {
    static const std::string dir = [] {
        const char* v = std::getenv("OPENRAC_DK_PACK");
        const bool stock = v != nullptr && std::string_view(v) == "stock";
        std::string d = stock ? "romfs:/dkgl-stock" : "romfs:/dkgl";
        log::info("dkgl: shader pack {} ({})", d,
                  stock ? "devkitPro uam as shipped, OPENRAC_DK_PACK=stock" : "uam scheduling fixed; OPENRAC_DK_PACK=stock for the old one");
        return d;
    }();
    return dir;
}

bool load_program(Program& p, const std::string& vertex, const std::string& fragment) {
    ++p.link_revision;
    p.key = fnv(vertex, fragment);
    const std::string base = std::format("{}/{:016x}", pack_dir(), p.key);
    std::vector<std::uint8_t> vs, fs, prog;
    if (!read_file(base + ".vert.dksh", vs) || !read_file(base + ".frag.dksh", fs)
        || !read_file(base + ".prog", prog)) {
        p.log = std::format("no precompiled deko3d program {:016x} (sources in dkgl-missing)", p.key);
        ++g->counters.missing_programs;
        write_missing(p.key, vertex, fragment);
        log::error("dkgl: {}", p.log);
        return false;
    }
    if (!decode_reflection(prog, p.reflection)) {
        p.log = std::format("deko3d program {:016x}: damaged reflection", p.key);
        return false;
    }
    p.role=program_role(vertex,fragment);
    for(const auto& uniform:p.reflection.uniforms)
        if(uniform.name=="skinning" && !uniform.sampler && uniform.gl_type==GL_INT)
            p.skinning_offset=uniform.offset;
    if(g->diagnostic!=Diagnostic::None)
        log::info("dkgl diagnostic: program {:016x}, role {}, skinning offset {}",p.key,int(p.role),p.skinning_offset);
    const std::uint64_t fs_at = align_up(vs.size(), DK_SHADER_CODE_ALIGNMENT);
    const std::uint64_t size = align_up(fs_at + fs.size() + DK_SHADER_CODE_UNUSABLE_SIZE, DK_MEMBLOCK_ALIGNMENT);
    p.code = make_block(std::uint32_t(size), DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached | DkMemBlockFlags_Code);
    if (p.code == nullptr) {
        p.log = "deko3d: no memory for shader code";
        return false;
    }
    auto* bytes = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(p.code));
    std::memcpy(bytes, vs.data(), vs.size());
    std::memcpy(bytes + fs_at, fs.data(), fs.size());
    DkShaderMaker maker;
    dkShaderMakerDefaults(&maker, p.code, 0);
    dkShaderInitialize(&p.vertex, &maker);
    dkShaderMakerDefaults(&maker, p.code, std::uint32_t(fs_at));
    dkShaderInitialize(&p.fragment, &maker);
    if (!dkShaderIsValid(&p.vertex) || !dkShaderIsValid(&p.fragment)) {
        p.log = std::format("deko3d program {:016x}: invalid DKSH", p.key);
        return false;
    }
    const auto fingerprint=[](const std::vector<std::uint8_t>& bytes) {
        std::uint64_t hash=0xcbf29ce484222325ull;
        for(const auto byte:bytes) hash=(hash^byte)*0x100000001b3ull;
        return hash;
    };
    log::info("dkgl shader binary {:016x}: vertex {} bytes fnv {:016x}; fragment {} bytes fnv {:016x}",
        p.key,vs.size(),fingerprint(vs),fs.size(),fingerprint(fs));
    p.uniforms.assign(std::size_t(std::max(0, p.reflection.default_size)), 0);
    p.uniforms_dirty = true;
    p.units.fill(0);
    return true;
}

namespace {

struct Reader {
    const std::vector<std::uint8_t>& b;
    std::size_t at = 0;
    bool ok = true;
    std::uint32_t u32() {
        if (at + 4 > b.size()) {
            ok = false;
            return 0;
        }
        std::uint32_t v;
        std::memcpy(&v, b.data() + at, 4);
        at += 4;
        return v;
    }
    int i32() { return int(u32()); }
    void skip_words() {
        const std::uint32_t n = u32();
        at += std::size_t(n) * 4;
        ok = ok && at <= b.size();
    }
    std::string text() {
        const std::uint32_t n = u32();
        if (!ok || at + n > b.size()) {
            ok = false;
            return {};
        }
        std::string s(reinterpret_cast<const char*>(b.data() + at), n);
        at += n;
        return s;
    }
};

}  // namespace

bool decode_reflection(const std::vector<std::uint8_t>& bytes, Reflection& out) {
    Reader in{bytes};
    out = Reflection{};
    in.skip_words();  // vertex SPIR-V
    in.skip_words();  // fragment
    in.skip_words();  // compute
    out.default_binding = in.i32();
    out.default_size = in.i32();
    const std::uint32_t uniforms = in.u32();
    for (std::uint32_t i = 0; in.ok && i < uniforms; ++i) {
        Uniform u;
        u.name = in.text();
        u.gl_type = in.u32();
        u.offset = in.i32();
        u.array_size = in.i32();
        u.array_stride = in.i32();
        u.sampler = in.u32() != 0;
        u.binding = in.i32();
        in.u32();  // kind
        out.uniforms.push_back(std::move(u));
    }
    for (int list = 0; list < 2 && in.ok; ++list) {  // uniform blocks, storage blocks
        const std::uint32_t n = in.u32();
        for (std::uint32_t i = 0; in.ok && i < n; ++i) {
            in.text();
            in.i32();
            in.i32();
        }
    }
    const std::uint32_t attributes = in.u32();
    for (std::uint32_t i = 0; in.ok && i < attributes; ++i) {
        Reflection::Attribute a;
        a.name = in.text();
        a.location = in.i32();
        a.gl_type = in.u32();
        out.attributes.push_back(std::move(a));
    }
    return in.ok;
}

}  // namespace openrac::dkgl::detail

namespace openrac::dkgl {

using namespace detail;

bool open(const Config& config, std::string& error) {
    if (g != nullptr) {
        error = "deko3d is already open";
        return false;
    }
    g = new Context();
    const char* diagnostic=std::getenv("OPENRAC_DK_DIAGNOSTIC");
    g->diagnostic=parse_diagnostic(diagnostic ? diagnostic : "");
    if(g->diagnostic==Diagnostic::Invalid) {
        log::warn("dkgl: unknown OPENRAC_DK_DIAGNOSTIC '{}'; diagnostics disabled",diagnostic);
        g->diagnostic=Diagnostic::None;
    } else if(g->diagnostic!=Diagnostic::None) {
        log::warn("dkgl: DIAGNOSTIC MODE '{}' active; intentionally incomplete picture, not a graphics fix",diagnostic);
    }
    g->window = config.native_window;
    g->screen_width = config.width;
    g->screen_height = config.height;
    g->interval = config.vsync ? 1 : 0;
    g->image_heap.flags = DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image;
    g->buffer_heap.flags = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached;
    if(const auto* v=std::getenv("OPENRAC_DK_TIMING");v && std::atoi(v)>0)
        g->timing_every=std::clamp(std::atoi(v),30,3600);
    if(const auto* v=std::getenv("OPENRAC_DK_PASS_TIMING");v && std::atoi(v)>0) {
        g->pass_every=std::clamp(std::atoi(v),30,3600);
        g->pass_names.reserve(kPassNames);
        g->pass_ms.assign(kPassNames,0.0);
        g->pass_calls.assign(kPassNames,0);
        g->pass_depth.assign(kPassNames,1<<30);
        if(const auto* st=std::getenv("OPENRAC_DK_PASS_STATS");st && std::string_view(st)=="1") {
            g->pass_stats=true;
            g->pass_capacity=kPassStatsReports;
            g->pass_marks=1+kPassStatCount;
            g->pass_stat.assign(kPassNames,{});
            log::info("dkgl: per-pass pipeline statistics on (OPENRAC_DK_PASS_STATS=1: vertex shader invocations, clipper "
                      "primitives in/out, fragment shader invocations, samples passed beside each pass mark; diagnostic, "
                      "each mark writes {} reports)",g->pass_marks);
        }
        log::info("dkgl: per-pass GPU timestamps every {} frames (OPENRAC_DK_PASS_TIMING; at most {} marks a frame, "
                  "read after each slot's own fence)",g->pass_every,g->pass_capacity/g->pass_marks);
    }
    if (R_FAILED(romfsInit())) {
        log::warn("dkgl: romfs did not mount; the shader pack is unavailable");
    }
    DkDeviceMaker device;
    dkDeviceMakerDefaults(&device);
    device.cbDebug = driver_message;
    // Vulkan's (and vkgl's) conventions, which the SPIR-V the shaders came from assumes: depth
    // 0..1, row 0 at the top. deko3d 0.5 has no YAxisPointsDown flag; the pack's vertex shaders
    // negate gl_Position.y instead (dk_pack.py, SPIRV-Cross --flip-vert-y), the same transform.
    device.flags = DkDeviceFlags_DepthZeroToOne | DkDeviceFlags_OriginUpperLeft;
    g->device = dkDeviceCreate(&device);
    if (g->device == nullptr) {
        error = "deko3d: no device";
        close();
        return false;
    }
    DkQueueMaker queue;
    dkQueueMakerDefaults(&queue, g->device);
    queue.flags = DkQueueFlags_Graphics | DkQueueFlags_Compute | DkQueueFlags_HighPrio;
    // Zcull (coarse early depth rejection by screen tile) off unless OPENRAC_DK_ZCULL=1. deko3d 0.5
    // sets it up only from depth clears and target bindings; the renderers draw with reversed depth
    // (clear 0, GL_GEQUAL, nearer is larger) and switch depth targets and directions within a frame.
    // On hardware, near-camera geometry went missing while farther geometry behind it drew
    // (2026-10-29 clips: ground under the camera, the landing ship, vendor models). Mesa's GL
    // (which draws these right) has no Zcull. It only saves work: off cannot change a picture.
    const char* zcull = std::getenv("OPENRAC_DK_ZCULL");
    const bool zcull_on = zcull != nullptr && std::string_view(zcull) == "1";
    if (!zcull_on) {
        queue.flags |= DkQueueFlags_DisableZcull;
    }
    log::info("dkgl: Zcull {}", zcull_on ? "ON (OPENRAC_DK_ZCULL=1)" : "off (OPENRAC_DK_ZCULL=1 turns it on)");
    queue.commandMemorySize = 4u << 20;
    queue.flushThreshold = 512u << 10;
    g->queue = dkQueueCreate(&queue);
    if (g->queue == nullptr) {
        error = "deko3d: no queue";
        close();
        return false;
    }
    if (!make_swapchain(error)) {
        close();
        return false;
    }
    g->descriptor_memory = make_block(
        kImageDescriptors * sizeof(DkImageDescriptor) + kSamplerDescriptors * sizeof(DkSamplerDescriptor),
        DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached
    );
    for (Slot& s : g->slots) {
        if(g->timing_every) {
            s.timing=make_block(DK_MEMBLOCK_ALIGNMENT,DkMemBlockFlags_CpuUncached|DkMemBlockFlags_GpuCached);
            if(!s.timing) log::warn("dkgl: timing allocation failed; this slot has no GPU timestamps");
        }
        if(g->pass_every) {
            static_assert(kPassReports*16<=DK_MEMBLOCK_ALIGNMENT,"pass reports fit one memory block");
            const auto bytes=std::uint32_t(align_up(std::uint64_t(g->pass_capacity)*16,DK_MEMBLOCK_ALIGNMENT));
            s.pass=make_block(bytes,DkMemBlockFlags_CpuUncached|DkMemBlockFlags_GpuCached);
            if(s.pass) std::memset(dkMemBlockGetCpuAddr(s.pass),0xff,g->pass_capacity*16);
            else log::warn("dkgl: pass timing allocation failed; this slot has no pass timestamps");
            s.pass_pairs.reserve(kPassReports/2);
        }
        DkCmdBufMaker maker;
        dkCmdBufMakerDefaults(&maker, g->device);
        maker.cbAddMem = add_command_memory;
        s.cmd = dkCmdBufCreate(&maker);
        s.ring = make_block(kRingBytes, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
        if (s.cmd == nullptr || s.ring == nullptr) {
            error = "deko3d: no memory for the frames";
            close();
            return false;
        }
    }
    DkCmdBufMaker capture_maker;
    dkCmdBufMakerDefaults(&capture_maker,g->device);
    g->vertex_capture=dkCmdBufCreate(&capture_maker);
    if (!g->vertex_capture) {
        error="deko3d: could not create vertex-binding scratch encoder";
        close(); return false;
    }
    log::info("dkgl: SDK 0.5 vertex binding capture/replay workaround enabled");
    log::info("dkgl: packed-Y winding correction and separate-alpha blend correction enabled");
    g->constant_attributes = make_block(DK_MEMBLOCK_ALIGNMENT, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    if (g->descriptor_memory == nullptr || g->constant_attributes == nullptr) {
        error = "deko3d: no memory for descriptors";
        close();
        return false;
    }
    {
        auto* c = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(g->constant_attributes));
        const float f[4] = {0, 0, 0, 1};
        const std::int32_t i[4] = {0, 0, 0, 1};
        std::memcpy(c, f, 16);
        std::memcpy(c + 16, i, 16);
    }
    begin_slot();
    // The dummy image (descriptor 0): one opaque black texel.
    g->next_image_descriptor = 0;
    g->dummy = make_image(DkImageFormat_RGBA8_Unorm, 1, 1, 1, false);
    if (!g->dummy) {
        error = "deko3d: no dummy image";
        close();
        return false;
    }
    {
        RingSlice s = ring(4, 32);
        const std::uint32_t black = 0xFF000000u;
        std::memcpy(s.cpu, &black, 4);
        DkImageView view;
        dkImageViewDefaults(&view, &g->dummy->image);
        const DkCopyBuf src{s.gpu, 0, 0};
        const DkImageRect rect{0, 0, 0, 1, 1, 1};
        dkCmdBufCopyBufferToImage(cmd(), &src, &view, &rect, 0);
        after_transfer();
    }
    if (!make_default_framebuffer(config.width, config.height)) {
        error = "deko3d: no default framebuffer";
        close();
        return false;
    }
    g->state.viewport[2] = g->state.scissor[2] = config.width;
    g->state.viewport[3] = g->state.scissor[3] = config.height;
    log::info("dkgl: deko3d open, {}x{}, {} frames in flight", config.width, config.height, kSlots);
    log::info("dkgl: copy-engine transfers ordered with DkBarrier_Full{}", legacy_transfer_sync() ? " (DISABLED by OPENRAC_DK_TRANSFER_SYNC=legacy)" : "");
    return true;
}

void close() {
    if (g == nullptr) {
        return;
    }
    if (g->queue != nullptr) {
        dkQueueWaitIdle(g->queue);
    }
    // Objects first (their images retire into the list), then the list, then the device.
    g->textures.objects.clear();
    g->renderbuffers.objects.clear();
    g->dummy.reset();
    g->default_colour.reset();
    g->default_depth.reset();
    for (auto& [name, p] : g->programs.objects) {
        if (p.code != nullptr) {
            dkMemBlockDestroy(p.code);
        }
    }
    g->buffers.objects.clear();
    g->completed = ~0ull;
    collect();
    for (Heap* heap : {&g->image_heap, &g->buffer_heap}) {
        for (Heap::Chunk& c : heap->chunks) {
            dkMemBlockDestroy(c.block);
        }
        heap->chunks.clear();
    }
    for (Slot& s : g->slots) {
        if(s.timing) dkMemBlockDestroy(s.timing);
        if(s.pass) dkMemBlockDestroy(s.pass);
        if (s.cmd != nullptr) {
            dkCmdBufDestroy(s.cmd);
        }
        for (DkMemBlock b : s.cmd_memory) {
            dkMemBlockDestroy(b);
        }
        if (s.ring != nullptr) {
            dkMemBlockDestroy(s.ring);
        }
    }
    if (g->swapchain != nullptr) {
        dkSwapchainDestroy(g->swapchain);
    }
    for (DkMemBlock b : {g->swap_memory, g->descriptor_memory, g->constant_attributes}) {
        if (b != nullptr) {
            dkMemBlockDestroy(b);
        }
    }
    if (g->queue != nullptr) {
        dkQueueDestroy(g->queue);
    }
    if (g->vertex_capture != nullptr) {
        dkCmdBufDestroy(g->vertex_capture);
    }
    if (g->device != nullptr) {
        dkDeviceDestroy(g->device);
    }
    Context* old = g;
    g = nullptr;
    delete old;
}

bool active() {
    return g != nullptr;
}

Proc get_proc(const char* name) {
    return name != nullptr ? gl_proc(name) : nullptr;
}

void present() {
    if (g == nullptr || !g->error.empty()) {
        return;
    }
    if (dkQueueIsInErrorState(g->queue)) {
        g->error = "deko3d: the GPU queue is in an error state";
        return;
    }
    // OPENRAC_DK_LATE_ACQUIRE=1 (experiment, off by default): submit the frame's scene before
    // acquiring the screen image. dkQueueAcquireImage makes the queue wait for the image's release
    // fence before anything submitted after it; acquired first, that wait sits in front of the whole
    // scene, so a frame whose GPU work misses a refresh waits for the next one before it starts
    // (sessions 31/32: GPU spans of 30 and 36 ms presented at 33.3 and 50 ms). Submitted first,
    // the scene runs and only the small present blit waits for the display.
    static const bool late_acquire = [] {
        const char* v = std::getenv("OPENRAC_DK_LATE_ACQUIRE");
        const bool on = v != nullptr && std::string_view(v) == "1";
        log::info("dkgl: screen image acquired {}", on
            ? "after the scene is submitted (EXPERIMENT OPENRAC_DK_LATE_ACQUIRE=1: only the present blit waits for it)"
            : "before the frame is submitted (default; OPENRAC_DK_LATE_ACQUIRE=1 submits the scene first)");
        return on;
    }();
    // The scene's end, for the pacing line (OPENRAC_DK_TIMING): before the screen image is waited for.
    if(g->timing_every && slot().timing && slot().timing_started) {
        dkCmdBufReportCounter(cmd(),DkCounter_Timestamp,dkMemBlockGetGpuAddr(slot().timing)+48);
    }
    if (late_acquire && g->list_open) {
        transfer_fence();  // the copy engine's last write is ordered before the scene list ends
        Slot& s = slot();
        DkCmdList list = dkCmdBufFinishList(s.cmd);
        dkQueueSubmitCommands(g->queue, list);
        dkQueueFlush(g->queue);
        g->list_open = false;
        g->list_breaks = 0;
        reset_bound_state();
    }
    const auto acquire_start=g->timing_every ? steady_ms() : 0;
    const int index = dkQueueAcquireImage(g->queue, g->swapchain);
    if(g->timing_every) {
        g->acquire_wait.add(steady_ms()-acquire_start);
        const auto now=steady_ms();
        if(g->previous_present_ms) g->present_interval.add(now-g->previous_present_ms);
        g->previous_present_ms=now;
    }
    if (index < 0 || index >= int(kSwapImages)) {
        g->error = "deko3d: no screen image";
        return;
    }
    // The default framebuffer, turned right way up (its row 0 is the picture's bottom).
    transfer_fence();  // a copy-engine write into it (glBlitFramebuffer's copy) comes first
    pass_begin("dk present blit");
    DkCmdBuf c = cmd();
    touch(g->default_colour.get());
    cmd_barrier(c, DkBarrier_Fragments, DkInvalidateFlags_Image);
    DkImageView src, dst;
    dkImageViewDefaults(&src, &g->default_colour->image);
    dkImageViewDefaults(&dst, &g->swap_images[std::size_t(index)]);
    const DkImageRect from{0, 0, 0, std::uint32_t(g->default_width), std::uint32_t(g->default_height), 1};
    const DkImageRect to{0, 0, 0, std::uint32_t(g->screen_width), std::uint32_t(g->screen_height), 1};
    dkCmdBufBlitImage(c, &src, &from, &dst, &to, DkBlitFlag_FlipY | DkBlitFlag_FilterLinear, 0);
    pass_end();
    if(g->timing_every && slot().timing && slot().timing_started) {
        dkCmdBufReportCounter(c,DkCounter_Timestamp,dkMemBlockGetGpuAddr(slot().timing)+16);
        slot().timing_pending=true;
    }
    submit_list();
    dkQueuePresentImage(g->queue, g->swapchain, index);
    ++g->accepted;
    ++g->counters.frames;
    // The next frame, in the next slot.
    ++g->frame;
    g->slot = (g->slot + 1) % kSlots;
    begin_slot();
    // OPENRAC_DK_COUNTERS=N: these lines every N frames. Unset, empty, 0 or negative is off
    // (common/env_interval.h); it was max(1, N) when set, so "=0" logged every frame (session 55).
    static const int every = env_interval::parse(std::getenv("OPENRAC_DK_COUNTERS"));
    if (every > 0 && g->counters.frames % std::uint64_t(every) == 0) {
        static Counters last{};
        static Counters last_copy{};
        const Counters& n = g->counters;
        const auto f = static_cast<double>(n.frames - last.frames);
        const auto per = [f](std::uint64_t now, std::uint64_t before) { return static_cast<double>(now - before) / f; };
        log::info(
            "dk frame avg over {}: draws {:.0f}, uniform uploads {:.0f}, ring {:.0f} KiB, texture uploads {:.1f}, "
            "barriers {:.1f} (full {:.1f}, texture cache {:.1f}), sampler set rebinds {:.1f}, engine copies {:.1f}, target changes {:.1f}, flushes {:.2f}; vertex setup hits {:.0f} misses {:.0f}; missing programs {}, diagnostic omissions {}",
            n.frames - last.frames, per(n.draws, last.draws), per(n.uniform_uploads, last.uniform_uploads),
            per(n.ring_bytes, last.ring_bytes) / 1024.0, per(n.texture_uploads, last.texture_uploads),
            per(n.barriers, last.barriers), per(n.full_barriers, last.full_barriers),
            per(n.texture_invalidations, last.texture_invalidations),
            per(n.sampler_rebinds, last.sampler_rebinds),
            per(n.engine_copies, last.engine_copies), per(n.target_changes, last.target_changes),
            per(n.flushes, last.flushes),per(n.vertex_setup_hits,last.vertex_setup_hits),
            per(n.vertex_setup_misses,last.vertex_setup_misses),n.missing_programs, n.diagnostic_omissions
        );
        last = n;
        log::info("dk CPU cache totals: sampler hits {} misses {}, sampler layouts {}, target barriers skipped {}",
                  n.sampler_lookup_hits,n.sampler_lookup_misses,n.sampler_layout_rebuilds,n.target_barriers_skipped);
        log::info("dk texture binding totals: calls {} slots {} (both shader stages counted)",
                  n.texture_bind_calls,n.texture_bind_slots);
        const auto full_known=(n.full_before_transfer-last_copy.full_before_transfer)+(n.full_transfer_fence-last_copy.full_transfer_fence)
                              +(n.full_mipmap-last_copy.full_mipmap)+(n.full_readback-last_copy.full_readback);
        const auto full_all=n.full_barriers-last_copy.full_barriers;
        log::info("dk copy sources per frame: texture uploads {:.2f} (into busy textures {:.2f}), mipmap level 0 {:.2f}, "
                  "texture copies {:.2f}, framebuffer blits colour {:.2f} depth {:.2f}, other {:.2f}; Full barriers per frame: "
                  "before a transfer {:.2f}, transfer fence {:.2f}, mipmap {:.2f}, readback {:.2f}, other {:.2f}",
                  per(n.copy_uploads,last_copy.copy_uploads),per(n.copy_uploads_busy,last_copy.copy_uploads_busy),
                  per(n.copy_mipmap,last_copy.copy_mipmap),per(n.copy_texture,last_copy.copy_texture),
                  per(n.copy_blit_colour,last_copy.copy_blit_colour),per(n.copy_blit_depth,last_copy.copy_blit_depth),
                  per(n.copy_other,last_copy.copy_other),
                  per(n.full_before_transfer,last_copy.full_before_transfer),per(n.full_transfer_fence,last_copy.full_transfer_fence),
                  per(n.full_mipmap,last_copy.full_mipmap),per(n.full_readback,last_copy.full_readback),
                  full_all>full_known ? double(full_all-full_known)/f : 0.0);
        last_copy = n;
        // The upload shapes of this window, the most frequent first (a per-frame upload shows as
        // about one a frame: e.g. 120 over 120 frames).
        if (!g->upload_shapes.empty()) {
            std::vector<std::pair<std::array<std::uint32_t,6>,std::array<std::uint64_t,2>>> shapes(
                g->upload_shapes.begin(), g->upload_shapes.end());
            std::sort(shapes.begin(), shapes.end(), [](const auto& a, const auto& b) { return a.second[0] > b.second[0]; });
            std::string list;
            for (std::size_t i = 0; i < shapes.size() && i < 6; ++i) {
                const auto& [k, v] = shapes[i];
                list += std::format("{}{}x{} format {} level {} rect {}x{}: {} (busy {})", i ? "; " : "",
                                    k[0], k[1], k[2], k[3], k[4], k[5], v[0], v[1]);
            }
            log::info("dk upload shapes over {:.0f} frames ({} shapes): {}", f, shapes.size(), list);
            g->upload_shapes.clear();
        }
    }
}

std::uint64_t accepted_presents() {
    return g != nullptr ? g->accepted : 0;
}

const std::string& presentation_error() {
    static const std::string none;
    return g != nullptr ? g->error : none;
}

int set_swap_interval(int interval) {
    if (g == nullptr) {
        return interval;
    }
    g->interval = interval > 0 ? 1 : 0;
    dkSwapchainSetSwapInterval(g->swapchain, std::uint32_t(g->interval));
    return g->interval;
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

// A pass mark at report `at`: the timestamp, then (OPENRAC_DK_PASS_STATS) the pipeline counters.
void write_pass_mark(Slot& s, unsigned at) {
    DkCmdBuf c = cmd();
    const DkGpuAddr base = dkMemBlockGetGpuAddr(s.pass);
    dkCmdBufReportCounter(c, DkCounter_Timestamp, base + 16u * at);
    if (g->pass_stats) {
        for (unsigned k = 0; k < kPassStatCount; ++k) {
            dkCmdBufReportCounter(c, kPassStatCounters[k], base + 16u * (at + 1 + k));
        }
    }
}

void pass_begin(std::string_view name) {
    if (g == nullptr || g->pass_every == 0) {
        return;
    }
    // The name's index: kept names first, then one shared bucket once kPassNames are in use.
    std::uint16_t id = 0;
    const auto known = std::find(g->pass_names.begin(), g->pass_names.end(), name);
    if (known != g->pass_names.end()) {
        id = std::uint16_t(known - g->pass_names.begin());
    } else if (g->pass_names.size() + 1 < kPassNames) {
        g->pass_names.emplace_back(name);
        id = std::uint16_t(g->pass_names.size() - 1);
    } else {
        const auto other = std::find(g->pass_names.begin(), g->pass_names.end(), "(other)");
        if (other == g->pass_names.end()) {
            g->pass_names.emplace_back("(other)");
            id = std::uint16_t(g->pass_names.size() - 1);
        } else {
            id = std::uint16_t(other - g->pass_names.begin());
        }
    }
    Slot& s = slot();
    int report = -1;
    // Two marks a pass: a begin is taken only while its end, and the ends of the passes already
    // open, still fit, so every recorded begin gets its end.
    if (s.pass != nullptr && s.pass_reports + (g->pass_stack.size() + 2) * g->pass_marks <= g->pass_capacity) {
        report = int(s.pass_reports);
        write_pass_mark(s, s.pass_reports);
        s.pass_reports += g->pass_marks;
    } else {
        ++g->pass_dropped;
    }
    g->pass_stack.push_back({id, report, g->slot, g->frame});
}

void pass_end() {
    if (g == nullptr || g->pass_every == 0 || g->pass_stack.empty()) {
        return;
    }
    const Context::OpenPass open = g->pass_stack.back();
    g->pass_stack.pop_back();
    // Begun in another frame (a present between), or its begin was dropped: nothing to pair.
    if (open.report < 0 || open.slot != g->slot || open.frame != g->frame) {
        return;
    }
    Slot& s = slot();
    if (s.pass_reports + g->pass_marks > g->pass_capacity) {
        ++g->pass_dropped;
        return;
    }
    const auto report = s.pass_reports;
    write_pass_mark(s, s.pass_reports);
    s.pass_reports += g->pass_marks;
    s.pass_pairs.push_back({open.name, std::uint16_t(g->pass_stack.size()), std::uint16_t(open.report),
                            std::uint16_t(report)});
}

bool pass_timing_enabled() {
    return g != nullptr && g->pass_every != 0;
}

Counters counters() {
    return g != nullptr ? g->counters : Counters{};
}

}  // namespace openrac::dkgl

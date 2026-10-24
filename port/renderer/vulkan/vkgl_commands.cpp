// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frame's work for the rest of the backend (vkgl_internal.h): what waits for the frame's end,
// the ring that carries vertices, indices, uniforms and texels to the GPU, the two command buffers
// and the order between them, GPU timing, submission, and the render target drawing goes to.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <utility>
#include <volk.h>

#include "common/env.h"
#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/render_opt.h"
#include "renderer/vulkan/vkgl_internal.h"

namespace openrac::vkgl::detail {

using namespace openrac::gl;

namespace {

namespace render_opt = renderer::render_opt;

// ---- The frame's ring: vertices, indices, uniforms, texels on their way ----

VkDeviceSize ring_alignment() {
    const VkPhysicalDeviceLimits& l = g->properties.limits;
    return std::max<VkDeviceSize>(
        {256,
         l.minUniformBufferOffsetAlignment,
         l.minTexelBufferOffsetAlignment,
         l.optimalBufferCopyOffsetAlignment,
         16}
    );
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
    alloc.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    alloc.requiredFlags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VmaAllocationInfo got{};
    if (!check(
            vmaCreateBuffer(g->allocator, &info, &alloc, &c.buffer, &c.allocation, &got),
            "a ring buffer"
        )) {
        return false;
    }
    c.mapped = static_cast<std::uint8_t*>(got.pMappedData);
    f.chunks.push_back(c);
    return true;
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

struct Clear {
    bool colour = false, depth = false, stencil = false;
};

// The garbage test_poison_discards writes over discarded data: every colour channel at its top,
// depth at the near plane and a stencil pattern, none of them what a pass of the port clears to.
constexpr VkClearColorValue kPoisonColour = {{1.0f, 1.0f, 1.0f, 1.0f}};
constexpr VkClearDepthStencilValue kPoisonDepth = {0.0f, 0xA5};

// Overwrites the `aspects` of `image` with the poison, outside any pass, in `main` (the order of
// the reads a later pass makes). Only for test_poison_discards.
void poison(Image& image, VkImageAspectFlags aspects) {
    VkCommandBuffer cmd = cmd_for(&image, true);
    barrier(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    const VkImageSubresourceRange range = {aspects, 0, VK_REMAINING_MIP_LEVELS, 0, 1};

    // A colour image, or the depth and stencil one.
    if ((aspects & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
        vkCmdClearColorImage(
            cmd, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &kPoisonColour, 1, &range
        );
    } else {
        vkCmdClearDepthStencilImage(
            cmd, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &kPoisonDepth, 1, &range
        );
    }

    count_command(Command::Copy);
}

// Notes that `aspects` of `image` hold nothing worth keeping, and with the poison on, makes them
// garbage at once. Called once the pass that drew into them has ended, or outside any pass.
void mark_discarded(Image& image, VkImageAspectFlags aspects) {
    // Nothing let go: the image keeps what it holds.
    if (aspects == 0) {
        return;
    }

    // The poison's clear moves the image to a transfer layout, which barrier counts as a write.
    if (g->poison_discards) {
        poison(image, aspects);
    }

    image.discarded |= aspects;
}

// The load operation of one aspect of an attachment: a clear the pass began with, nothing for
// contents that were let go (the poison's clear under test_poison_discards), else a load.
VkAttachmentLoadOp load_op(const Image& image, VkImageAspectFlags aspect, bool cleared) {
    // The clear the pass began with comes first: it overwrites whatever was there.
    if (cleared) {
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    }

    // Let go: no load, or under test the poison, so that a pass that reads it differs.
    if ((image.discarded & aspect) != 0) {
        return g->poison_discards ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_LOAD_OP_LOAD;
}

// The secondary command buffer for this recording's next deferred pass into `t`, begun to
// continue a pass of `t`'s formats and samples. Allocated from the frame's pool the first time.
VkCommandBuffer secondary_for(const Target& t) {
    Frame& f = frame();

    // A buffer of an earlier recording of this slot, reset with the pool, or a new one.
    if (f.secondary_at == f.secondaries.size()) {
        VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        alloc.commandPool = f.pool;
        alloc.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        alloc.commandBufferCount = 1;
        VkCommandBuffer made = VK_NULL_HANDLE;

        // No secondary buffer: the caller records the pass directly.
        if (!check(vkAllocateCommandBuffers(g->device, &alloc, &made), "a secondary buffer")) {
            return VK_NULL_HANDLE;
        }

        f.secondaries.push_back(made);
    }

    VkCommandBuffer cmd = f.secondaries[f.secondary_at++];
    const VkFormat colour = t.colour != nullptr ? t.colour->format : VK_FORMAT_UNDEFINED;
    VkCommandBufferInheritanceRenderingInfo rendering{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO
    };
    rendering.colorAttachmentCount = t.colour != nullptr ? 1 : 0;
    rendering.pColorAttachmentFormats = &colour;
    rendering.depthAttachmentFormat = t.depth != nullptr ? t.depth->format : VK_FORMAT_UNDEFINED;
    rendering.stencilAttachmentFormat =
        t.depth != nullptr && has_stencil(t.depth->format) ? t.depth->format : VK_FORMAT_UNDEFINED;
    rendering.rasterizationSamples = t.samples;
    VkCommandBufferInheritanceInfo inheritance{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
    inheritance.pNext = &rendering;
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
                  | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin.pInheritanceInfo = &inheritance;
    vkBeginCommandBuffer(cmd, &begin);
    return cmd;
}

// Moves `t`'s attachments into their layouts for drawing, in `cmd` before the pass.
void attachment_barriers(VkCommandBuffer cmd, const Target& t) {
    // The colour image, also when it is in that layout already: an earlier pass's writes are done.
    if (t.colour != nullptr) {
        t.colour->touched = g->recording;
        barrier(cmd, *t.colour, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    // The depth and stencil image.
    if (t.depth != nullptr) {
        t.depth->touched = g->recording;
        barrier(cmd, *t.depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
}

// Begins rendering into `t` as recording goes, the reference path: every attachment stored, and
// loaded unless `clear` clears it.
void begin_direct(const Target& t, const Clear& clear) {
    VkCommandBuffer cmd = frame().main;
    VkRenderingAttachmentInfo colour{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingAttachmentInfo depth{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingAttachmentInfo stencil{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
    info.renderArea =
        {{0, 0}, {static_cast<std::uint32_t>(t.width), static_cast<std::uint32_t>(t.height)}};
    info.layerCount = 1;
    const State& s = g->state;
    const VkExtent2D area = info.renderArea.extent;
    attachment_barriers(cmd, t);
    if (t.colour != nullptr) {
        colour.imageView = attachment_view(*t.colour);
        colour.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colour.loadOp = clear.colour ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        colour.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        std::memcpy(colour.clearValue.color.float32, s.clear_colour, sizeof s.clear_colour);
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &colour;
        count_attachment(*t.colour, area, !clear.colour, true);
    }
    if (t.depth != nullptr) {
        depth.imageView = attachment_view(*t.depth);
        depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth.loadOp = clear.depth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth.clearValue
            .depthStencil = {s.clear_depth, static_cast<std::uint32_t>(s.clear_stencil)};
        info.pDepthAttachment = &depth;
        if (has_stencil(t.depth->format)) {
            stencil = depth;
            stencil.loadOp =
                clear.stencil ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            info.pStencilAttachment = &stencil;
        }
        // One image holds both aspects: read in when either is kept.
        const bool kept = !clear.depth || (has_stencil(t.depth->format) && !clear.stencil);
        count_attachment(*t.depth, area, kept, true);
    }
    vkCmdBeginRendering(cmd, &info);
    count_render_pass();
    g->pass = OpenPass{};
}

// Begins a pass into `t` whose load and store operations wait for its end (attachment-discard):
// the barriers go into `main` now, and `main` points at a secondary buffer until end_rendering.
// False when no secondary buffer can be had, and nothing is begun.
bool begin_deferred(const Target& t, const Clear& clear) {
    const VkCommandBuffer secondary = secondary_for(t);

    // No buffer to defer into: the pass is recorded directly.
    if (secondary == VK_NULL_HANDLE) {
        return false;
    }

    Frame& f = frame();
    const State& s = g->state;
    attachment_barriers(f.main, t);
    OpenPass& pass = g->pass;
    pass = OpenPass{};
    pass.deferred = true;
    pass.primary = f.main;
    pass.clear_colour = clear.colour;
    pass.clear_depth = clear.depth;
    pass.clear_stencil = clear.stencil;
    std::memcpy(pass.colour_value.float32, s.clear_colour, sizeof s.clear_colour);
    pass.depth_value = {s.clear_depth, static_cast<std::uint32_t>(s.clear_stencil)};
    f.main = secondary;
    return true;
}

// The store operation of an aspect the open pass may have let go (`dropped`).
VkAttachmentStoreOp store_op(VkImageAspectFlags dropped, VkImageAspectFlags aspect) {
    return (dropped & aspect) != 0 ? VK_ATTACHMENT_STORE_OP_DONT_CARE
                                   : VK_ATTACHMENT_STORE_OP_STORE;
}

// Ends the deferred pass into `t`: begins it in the frame's `main` with its operations chosen
// now (a let-go aspect not stored, a discarded one not loaded), runs its secondary buffer and
// ends it. The images then know which of their aspects hold nothing.
void end_deferred(const Target& t) {
    Frame& f = frame();
    OpenPass& pass = g->pass;
    const VkCommandBuffer secondary = f.main;
    vkEndCommandBuffer(secondary);
    f.main = pass.primary;
    VkRenderingAttachmentInfo colour{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingAttachmentInfo depth{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingAttachmentInfo stencil{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
    info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    info.renderArea =
        {{0, 0}, {static_cast<std::uint32_t>(t.width), static_cast<std::uint32_t>(t.height)}};
    info.layerCount = 1;
    const VkExtent2D area = info.renderArea.extent;

    // The colour attachment: cleared, not loaded or loaded; stored unless let go.
    if (t.colour != nullptr) {
        colour.imageView = attachment_view(*t.colour);
        colour.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colour.loadOp = load_op(*t.colour, VK_IMAGE_ASPECT_COLOR_BIT, pass.clear_colour);
        colour.storeOp = store_op(pass.drop_colour, VK_IMAGE_ASPECT_COLOR_BIT);
        // A pass's own clear keeps its value; a load of let-go data under test is the poison.
        colour.clearValue.color =
            pass.clear_colour || !g->poison_discards ? pass.colour_value : kPoisonColour;
        info.colorAttachmentCount = 1;
        info.pColorAttachments = &colour;
        count_attachment(
            *t.colour,
            area,
            colour.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD,
            colour.storeOp == VK_ATTACHMENT_STORE_OP_STORE
        );
    }

    // The depth and stencil image: each aspect's operations of its own, counted once.
    if (t.depth != nullptr) {
        depth.imageView = attachment_view(*t.depth);
        depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth.loadOp = load_op(*t.depth, VK_IMAGE_ASPECT_DEPTH_BIT, pass.clear_depth);
        depth.storeOp = store_op(pass.drop_depth, VK_IMAGE_ASPECT_DEPTH_BIT);
        depth.clearValue.depthStencil = pass.depth_value;

        // A load of let-go depth under test is cleared to the poison rather than to the value.
        if (g->poison_discards && !pass.clear_depth) {
            depth.clearValue.depthStencil.depth = kPoisonDepth.depth;
        }

        info.pDepthAttachment = &depth;
        bool loaded = depth.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD;
        bool stored = depth.storeOp == VK_ATTACHMENT_STORE_OP_STORE;

        // The stencil aspect of the same image.
        if (has_stencil(t.depth->format)) {
            stencil = depth;
            stencil.loadOp = load_op(*t.depth, VK_IMAGE_ASPECT_STENCIL_BIT, pass.clear_stencil);
            stencil.storeOp = store_op(pass.drop_depth, VK_IMAGE_ASPECT_STENCIL_BIT);

            // The same for let-go stencil.
            if (g->poison_discards && !pass.clear_stencil) {
                stencil.clearValue.depthStencil.stencil = kPoisonDepth.stencil;
            }

            info.pStencilAttachment = &stencil;
            loaded = loaded || stencil.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD;
            stored = stored || stencil.storeOp == VK_ATTACHMENT_STORE_OP_STORE;
        }

        // One image holds both aspects: read in when either is loaded, written when either is.
        count_attachment(*t.depth, area, loaded, stored);
    }

    vkCmdBeginRendering(f.main, &info);
    count_render_pass();
    vkCmdExecuteCommands(f.main, 1, &secondary);
    count_command(Command::Other);
    vkCmdEndRendering(f.main);
    count_command(Command::RenderPass);
    const OpenPass ended = pass;
    pass = OpenPass{};

    // What the pass wrote is whole again; what it let go holds nothing (or the poison).
    if (t.colour != nullptr) {
        t.colour->discarded = 0;
        mark_discarded(*t.colour, ended.drop_colour);
    }

    // The same for the depth and stencil image.
    if (t.depth != nullptr) {
        t.depth->discarded = 0;
        mark_discarded(*t.depth, ended.drop_depth);
    }
}

void begin_rendering(const Target& t, const Clear& clear) {
    end_rendering();

    // The operations chosen at the pass's end, or as it begins (the reference path).
    if (!render_opt::enabled(render_opt::Opt::AttachmentDiscard) || !begin_deferred(t, clear)) {
        begin_direct(t, clear);
    }

    g->rendering = true;
    g->rendering_target = t;
    g->bound_pipeline = VK_NULL_HANDLE;

    // A new pass trusts nothing recorded before it (RecordedState): more than Vulkan asks.
    g->recorded.forget();
}

}  // namespace

// ---- The render counters ----

void count_attachment(const Image& image, VkExtent2D area, bool load, bool store) {
    const std::uint64_t bytes = std::uint64_t{area.width} * area.height
                                * static_cast<std::uint64_t>(texel_bytes(image.format))
                                * static_cast<std::uint64_t>(image.samples);

    // Kept from before the pass: read from memory into the GPU's attachment storage.
    if (load) {
        g->submission.attachment_load_bytes += bytes;
    }

    // Kept after the pass: written back to memory.
    if (store) {
        g->submission.attachment_store_bytes += bytes;
    }
}

void gpu_flush_draws() {
    auto& gpu = g->gpu;
    if (!gpu.pending_count) {
        return;
    }
    vkCmdDrawIndexedIndirect(
        frame().main, gpu.pending_arguments, gpu.pending_offset, gpu.pending_count, 20
    );
    count_command(Command::Draw);
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

void retire(ImagePtr image) {
    if (image) {
        defer([image]() mutable { image.reset(); });
    }
}

// ---- The frame's ring: vertices, indices, uniforms, texels on their way ----

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
                g->submission.ring_bytes += size;
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
        // Not rendering from here on: the poison of a deferred pass's end records outside it.
        g->rendering = false;
        g->bound_pipeline = VK_NULL_HANDLE;

        // A deferred pass ends with its operations chosen; a direct one as it was begun.
        if (g->pass.deferred) {
            end_deferred(g->rendering_target);

            // A secondary buffer leaves the state of the buffer that ran it undefined.
            g->recorded.forget();
        } else {
            vkCmdEndRendering(frame().main);
            count_command(Command::RenderPass);
        }
    }
}

VkCommandBuffer cmd_for(Image* image, bool force_main) {
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
    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &b
    );
    count_barrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    image.layout = to;

    // A copy, blit or upload into the image comes next: what it writes is kept.
    if (to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        image.discarded = 0;
    }
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
        count_command(Command::Query);
    }
    g->upload_used = false;
    f.secondary_at = 0;
    ++g->recording;

    // A command buffer begun afresh holds no dynamic state and no binds.
    g->recorded.forget();
}

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
    if (env::flag(env::Var::GpuMarkers) && vkCmdBeginDebugUtilsLabelEXT) {
        const auto name = stats::gpu_section_name(serial, section);
        VkDebugUtilsLabelEXT label{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
        label.pLabelName = name.c_str();
        label.color[0] = 0.25f;
        label.color[1] = 0.6f;
        label.color[2] = 0.9f;
        label.color[3] = 1;
        vkCmdBeginDebugUtilsLabelEXT(g->gpu.timing_upload ? f.upload : f.main, &label);
        count_command(Command::Other);
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
    count_command(Command::Query);
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
    count_command(Command::Query);
#if defined(OPENRAC_GPU_MARKERS)
    if (env::flag(env::Var::GpuMarkers) && vkCmdEndDebugUtilsLabelEXT) {
        vkCmdEndDebugUtilsLabelEXT(g->gpu.timing_upload ? f.upload : f.main);
        count_command(Command::Other);
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
    count_barrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
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
    ++g->submission.submits;
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

// ---- Where drawing goes ----

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

void ensure_rendering(const Target& t) {
    if (!g->rendering || !(g->rendering_target == t)) {
        begin_rendering(t, Clear{});
    }

    // A draw or a clear after glInvalidateFramebuffer: what it writes is kept.
    g->pass.drop_colour = 0;
    g->pass.drop_depth = 0;
}

void discard_attachments(const Target& t, bool colour, VkImageAspectFlags depth) {
    const bool open = g->rendering && g->pass.deferred;

    // The colour image: not stored by the open pass on it, or not loaded by the next one.
    if (t.colour != nullptr && colour) {
        // The pass drawing into it lets it go at its end; any other is done with it already.
        if (open && g->rendering_target.colour == t.colour) {
            g->pass.drop_colour |= VK_IMAGE_ASPECT_COLOR_BIT;
        } else {
            mark_discarded(*t.colour, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    const VkImageAspectFlags aspects =
        t.depth != nullptr ? depth & t.depth->aspect : VkImageAspectFlags{0};

    // The depth and stencil image, the aspects it has of those asked.
    if (aspects != 0) {
        // The same choice as for the colour image.
        if (open && g->rendering_target.depth == t.depth) {
            g->pass.drop_depth |= aspects;
        } else {
            mark_discarded(*t.depth, aspects);
        }
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
    c.stencil = t.depth != nullptr && has_stencil(t.depth->format)
                && (mask & GL_STENCIL_BUFFER_BIT) != 0 && (s.stencil_write_mask & 0xFF) != 0;
    if (!c.colour && !c.depth && !c.stencil) {
        return;
    }
    const VkRect2D rect = scissor_rect(t);
    if (rect.extent.width == 0 || rect.extent.height == 0) {
        return;
    }
    const bool whole = rect.offset.x == 0 && rect.offset.y == 0
                       && static_cast<int>(rect.extent.width) == t.width
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
        attachments[n].aspectMask = (c.depth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0u)
                                    | (c.stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0u);
        attachments[n]
            .clearValue.depthStencil = {s.clear_depth, static_cast<std::uint32_t>(s.clear_stencil)};
        ++n;
    }
    VkClearRect r{rect, 0, 1};
    vkCmdClearAttachments(frame().main, n, attachments, 1, &r);
    count_command(Command::Other);
}

}  // namespace openrac::vkgl::detail

namespace openrac::vkgl {

bool test_poison_discards(bool on) {
    // No context: nothing to poison.
    if (detail::g == nullptr) {
        return false;
    }

    detail::g->poison_discards = on;
    return true;
}

}  // namespace openrac::vkgl

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frame's work for the rest of the backend (vkgl_internal.h): what waits for the frame's end, the
// ring that carries vertices, indices, uniforms and texels to the GPU, the two command buffers and the
// order between them, GPU timing, submission, and the render target drawing goes to.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <utility>
#include <volk.h>

#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/vulkan/vkgl_internal.h"

namespace openrac::vkgl::detail {

using namespace openrac::gl;

namespace {

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

void begin_rendering(const Target& t, const Clear& clear) {
    end_rendering();
    VkCommandBuffer cmd = frame().main;
    VkRenderingAttachmentInfo colour{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingAttachmentInfo depth{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingAttachmentInfo stencil{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
    info.renderArea =
        {{0, 0}, {static_cast<std::uint32_t>(t.width), static_cast<std::uint32_t>(t.height)}};
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
        depth.clearValue
            .depthStencil = {s.clear_depth, static_cast<std::uint32_t>(s.clear_stencil)};
        info.pDepthAttachment = &depth;
        if (has_stencil(t.depth->format)) {
            stencil = depth;
            stencil.loadOp =
                clear.stencil ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            info.pStencilAttachment = &stencil;
        }
    }
    vkCmdBeginRendering(cmd, &info);
    g->rendering = true;
    g->rendering_target = t;
    g->bound_pipeline = VK_NULL_HANDLE;
}

}  // namespace

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
        label.color[0] = 0.25f;
        label.color[1] = 0.6f;
        label.color[2] = 0.9f;
        label.color[3] = 1;
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
}

}  // namespace openrac::vkgl::detail

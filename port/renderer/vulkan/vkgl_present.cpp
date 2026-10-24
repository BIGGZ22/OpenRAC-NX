// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The screen (vkgl_internal.h): the default framebuffer, the swapchain and its present mode, moving
// on to the next frame, a new surface when the old one went, copying a finished frame to the
// window, and the render counters published as the frame's gauges.

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <volk.h>

#include <SDL3/SDL.h>

#include "common/log.h"
#include "common/stats.h"
#include "renderer/vulkan/vkgl.h"
#include "renderer/vulkan/vkgl_internal.h"

namespace openrac::vkgl::detail {

using namespace openrac::gl;

namespace {

// ---- The render counters ----

// How a render counter's gauge reads it: the part the frame added, or the total so far (the
// cumulative counts, and the bytes held now for the mirror of GL's buffers).
enum class Span : std::uint8_t {
    Frame,
    Total,
};

// One render counter published as a gauge (docs/port/RENDER_PERF.md, "The counters").
struct Gauge {
    std::string_view name;                      // The gauge's name, vk_ and the counter's.
    std::uint64_t SubmissionCounters::* value;  // The counter it reads.
    Span span;                                  // Whether it shows the frame's part or the total.
};

// The render counters other than the commands, in the order of SubmissionCounters.
// clang-format off: one gauge per row.
constexpr auto kGauges = std::to_array<Gauge>({
    {"vk_pipelines_created", &SubmissionCounters::pipelines_created, Span::Total},
    {"vk_pipelines_created_after_warmup", &SubmissionCounters::pipelines_created_after_warmup,
     Span::Total},
    {"vk_descriptor_sets_allocated", &SubmissionCounters::descriptor_sets_allocated, Span::Frame},
    {"vk_descriptor_writes", &SubmissionCounters::descriptor_writes, Span::Frame},
    {"vk_render_passes", &SubmissionCounters::render_passes, Span::Frame},
    {"vk_attachment_load_bytes", &SubmissionCounters::attachment_load_bytes, Span::Frame},
    {"vk_attachment_store_bytes", &SubmissionCounters::attachment_store_bytes, Span::Frame},
    {"vk_barriers", &SubmissionCounters::barriers, Span::Frame},
    {"vk_barriers_all_commands", &SubmissionCounters::barriers_all_commands, Span::Frame},
    {"vk_submits", &SubmissionCounters::submits, Span::Frame},
    {"vk_frames_presented", &SubmissionCounters::frames_presented, Span::Total},
    {"vk_fullscreen_copies", &SubmissionCounters::fullscreen_copies, Span::Frame},
    {"vk_ring_bytes", &SubmissionCounters::ring_bytes, Span::Frame},
    {"vk_texture_creations", &SubmissionCounters::texture_creations, Span::Frame},
    {"vk_texture_upload_bytes", &SubmissionCounters::texture_upload_bytes, Span::Frame},
    {"vk_buffer_static_bytes_host_visible", &SubmissionCounters::buffer_static_bytes_host_visible,
     Span::Total},
    {"vk_cpu_mirror_bytes", &SubmissionCounters::cpu_mirror_bytes, Span::Total},
    {"vk_gpu_payload_bytes", &SubmissionCounters::gpu_payload_bytes, Span::Frame},
    {"vk_shader_compiles", &SubmissionCounters::shader_compiles, Span::Total},
});
// clang-format on

// The gauge of each kind of command, in the order of Command.
constexpr std::array<std::string_view, kCommandKinds> kCommandGauges = {
    "vk_cmds_bind_pipeline",
    "vk_cmds_bind_descriptors",
    "vk_cmds_bind_buffers",
    "vk_cmds_dynamic_state",
    "vk_cmds_draw",
    "vk_cmds_barrier",
    "vk_cmds_copy",
    "vk_cmds_dispatch",
    "vk_cmds_render_pass",
    "vk_cmds_query",
    "vk_cmds_other",
};

// Returns the value of `gauge` from the counters `now`, given those of the frame before.
double gauge_value(
    const Gauge& gauge, const SubmissionCounters& now, const SubmissionCounters& before
) {
    const std::uint64_t total = now.*gauge.value;
    return static_cast<double>(gauge.span == Span::Total ? total : total - before.*gauge.value);
}

// ---- The device, the screen and the frames ----

ImagePtr default_image(const Format& f, int width, int height) {
    ImagePtr image = make_image(f, width, height, 1, VK_SAMPLE_COUNT_1_BIT);
    return image;
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
}

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

}  // namespace

// ---- The device, the screen and the frames ----

void run_deferred(Frame& f) {
    std::vector<std::function<void()>> list = std::move(f.deferred);
    f.deferred.clear();
    for (auto& fn : list) {
        fn();
    }
}

bool make_default_framebuffer(int width, int height) {
    g->default_colour =
        default_image({VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 4}, width, height);
    g->default_depth = default_image(
        {g->depth_format,
         VK_IMAGE_ASPECT_DEPTH_BIT
             | (has_stencil(g->depth_format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0u),
         4},
        width,
        height
    );
    g->default_width = width;
    g->default_height = height;
    return g->default_colour && g->default_depth;
}

VkPresentModeKHR present_mode() {
    std::uint32_t n = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(g->physical, g->surface, &n, nullptr);
    std::vector<VkPresentModeKHR> modes(n);
    vkGetPhysicalDeviceSurfacePresentModesKHR(g->physical, g->surface, &n, modes.data());
    auto has = [&](VkPresentModeKHR m) {
        return std::find(modes.begin(), modes.end(), m) != modes.end();
    };
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
    VkExtent2D extent{
        static_cast<std::uint32_t>(std::max(w, 1)), static_cast<std::uint32_t>(std::max(h, 1))
    };
#if !defined(__ANDROID__)
    if (caps.currentExtent.width != 0xFFFFFFFFu) {
        extent = caps.currentExtent;
    }
#endif
    extent.width = std::clamp(
        extent.width, caps.minImageExtent.width, std::max(caps.maxImageExtent.width, 1u)
    );
    extent.height = std::clamp(
        extent.height, caps.minImageExtent.height, std::max(caps.maxImageExtent.height, 1u)
    );
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
         {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
          VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
          VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
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

void publish_counters() {
    ++g->submission.frames_presented;
    const SubmissionCounters& now = g->submission;

    // Nobody is measuring: the next frame that is measured shows only its own part.
    if (!stats::enabled()) {
        g->published = now;
        return;
    }

    const SubmissionCounters& before = g->published;

    // The counts and bytes, each as its row of the table says.
    for (const Gauge& gauge : kGauges) {
        stats::gauge(gauge.name, gauge_value(gauge, now, before));
    }

    stats::gauge(
        "vk_cmds_recorded", static_cast<double>(commands_recorded(now) - commands_recorded(before))
    );

    // The frame's commands of each kind.
    for (std::size_t kind = 0; kind < kCommandKinds; ++kind) {
        stats::gauge(
            kCommandGauges[kind], static_cast<double>(now.commands[kind] - before.commands[kind])
        );
    }

    stats::gauge("shader_compile_s", now.shader_compile_s);
    stats::gauge("vk_gpu_timestamps", g->timestamp_bits != 0 ? 1.0 : 0.0);
    g->published = now;
}

void advance() {
    g->frame = (g->frame + 1) % kFrames;
    Frame& f = frame();
    if (f.submitted) {
        const stats::Scope wait("vulkan frame wait");
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

bool present_to_screen() {
    const stats::Scope pass("vulkan present");
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
    VkResult r = vkAcquireNextImageKHR(
        g->device, g->swapchain, UINT64_MAX, f.acquired, VK_NULL_HANDLE, &index
    );
    if (r == VK_ERROR_OUT_OF_DATE_KHR) {
        if (!make_swapchain()) {
            return false;
        }
        r = vkAcquireNextImageKHR(
            g->device, g->swapchain, UINT64_MAX, f.acquired, VK_NULL_HANDLE, &index
        );
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
    vkCmdBlitImage(
        cmd,
        colour.image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        screen,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &blit,
        same ? VK_FILTER_NEAREST : VK_FILTER_LINEAR
    );
    count_copy(g->swapchain_extent.width, g->swapchain_extent.height);
    swapchain_barrier(
        cmd, screen, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );
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

}  // namespace openrac::vkgl::detail

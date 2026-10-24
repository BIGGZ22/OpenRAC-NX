// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Images, samplers and buffers on the GPU behind the GL objects (vkgl_internal.h): the formats, the
// views, pixel uploads and mip chains, the samplers for GL's parameters, and the buffers and texel
// views behind GL's buffers.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <volk.h>

#include "common/float_encoding.h"
#include "renderer/gl.h"
#include "renderer/render_opt.h"
#include "renderer/vulkan/vkgl.h"
#include "renderer/vulkan/vkgl_internal.h"
#include "renderer/world/gl_world.h"

namespace openrac::vkgl::detail {

using namespace openrac::gl;

namespace {

// ---- Images ----

VkFormat depth_only_format() {
    for (const VkFormat f :
         {VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D24_UNORM_S8_UINT}) {
        VkFormatProperties p{};
        vkGetPhysicalDeviceFormatProperties(g->physical, f, &p);
        if ((p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
            return f;
        }
    }
    return g->depth_format;
}

int full_chain(int width, int height) {
    int levels = 1;
    for (int size = std::max(width, height); size > 1; size >>= 1) {
        ++levels;
    }
    return levels;
}

VkImageView make_view(
    Image& image, std::uint32_t base, std::uint32_t count, VkImageAspectFlags aspect
) {
    VkImageViewCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    info.image = image.image;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = image.format;
    info.subresourceRange = {aspect, base, count, 0, 1};
    VkImageView view = VK_NULL_HANDLE;
    check(vkCreateImageView(g->device, &info, nullptr, &view), "an image view");
    return view;
}

// ---- Samplers ----

VkFilter filter_of(GLenum f) {
    return f == GL_NEAREST || f == wgl::GL_NEAREST_MIPMAP_NEAREST || f == kGlNearestMipmapLinear
               ? VK_FILTER_NEAREST
               : VK_FILTER_LINEAR;
}

VkSamplerAddressMode wrap_of(GLenum w) {
    switch (w) {
        case GL_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case kGlMirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

}  // namespace

// ---- Images ----

Format format_of(GLenum internal) {
    switch (internal) {
        case GL_RGBA8:
        case GL_RGBA:
        case GL_RGB:
            return {VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 4};
        case kGlR8:
        case kGlRed:
            return {VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1};
        case wgl::GL_R32UI:
            return {VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 4};
        case wgl::GL_RGBA32UI:
            return {VK_FORMAT_R32G32B32A32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 16};
        case wgl::GL_RGBA32F:
            return {VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 16};
        case kGlRgba16f:
            return {VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 8};
        case kGlR11fG11fB10f:
            return {VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_IMAGE_ASPECT_COLOR_BIT, 4};
        case kGlRg32f:
            return {VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 8};
        // Temporal reconstruction uses float depth and two-channel half-float motion outputs.
        case 0x822E:  // GL_R32F
            return {VK_FORMAT_R32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 4};
        case 0x822F:  // GL_RG16F
            return {VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 4};
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32F:
            return {depth_only_format(), VK_IMAGE_ASPECT_DEPTH_BIT, 4};
        case GL_DEPTH24_STENCIL8:
        case kGlDepthStencil:
            return {g->depth_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 4};
        default:
            return {};
    }
}

bool has_stencil(VkFormat f) {
    return f == VK_FORMAT_D24_UNORM_S8_UINT || f == VK_FORMAT_D32_SFLOAT_S8_UINT
           || f == VK_FORMAT_D16_UNORM_S8_UINT;
}

int texel_bytes(VkFormat f) {
    // The sizes are the formats' own (the Vulkan specification's format table); a depth and
    // stencil format counts the bytes it is laid out in, its stencil byte included.
    switch (f) {
        case VK_FORMAT_R8_UNORM:
            return 1;
        case VK_FORMAT_D16_UNORM:
            return 2;
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return 3;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return 8;
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return 16;
        default:
            // Every other format the backend makes takes 4 (RGBA8, R32, B10G11R11, D24S8, D32,
            // X8_D24), and so do the screen's.
            return 4;
    }
}

ImagePtr make_image(
    const Format& format,
    int width,
    int height,
    int levels,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags extra_usage
) {
    // Called by the last owner to let go: the image's Vulkan objects go first, while the device
    // is open, then the Image itself.
    auto image = std::shared_ptr<Image>(std::make_unique<Image>().release(), [](Image* i) {
        std::unique_ptr<Image> owned(i);
        if (g != nullptr && g->device != VK_NULL_HANDLE) {
            destroy_image_now(*i);
        }
    });
    image->serial = ++g->image_serial;
    const bool depth = (format.aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
    VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = format.vk;
    info.extent =
        {static_cast<std::uint32_t>(std::max(width, 1)),
         static_cast<std::uint32_t>(std::max(height, 1)),
         1};
    info.mipLevels = static_cast<std::uint32_t>(levels);
    info.arrayLayers = 1;
    info.samples = samples;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = extra_usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                 | (depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                          : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
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
    if (!check(
            vmaCreateImage(g->allocator, &info, &alloc, &image->image, &image->allocation, nullptr),
            "an image"
        )) {
        return nullptr;
    }
    image->format = format.vk;
    image->usage = info.usage;
    image->aspect = format.aspect;
    image->width = std::max(width, 1);
    image->height = std::max(height, 1);
    image->levels = levels;
    image->samples = samples;
    ++g->submission.texture_creations;
    return image;
}

VkImageView attachment_view(Image& image) {
    if (image.attachment == VK_NULL_HANDLE) {
        image.attachment = make_view(image, 0, 1, image.aspect);
    }
    return image.attachment;
}

VkImageView sample_view(Image& image, int base, int top) {
    base = std::clamp(base, 0, image.levels - 1);
    top = std::clamp(top, base, image.levels - 1);
    const std::uint32_t key =
        static_cast<std::uint32_t>(base) | static_cast<std::uint32_t>(top) << 16;
    for (const auto& [k, view] : image.views) {
        if (k == key) {
            return view;
        }
    }
    const VkImageAspectFlags aspect =
        (image.aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0 ? VK_IMAGE_ASPECT_DEPTH_BIT : image.aspect;
    const VkImageView view = make_view(
        image, static_cast<std::uint32_t>(base), static_cast<std::uint32_t>(top - base + 1), aspect
    );
    image.views.emplace_back(key, view);
    return view;
}

Slice stage_pixels(
    const Format& to, int width, int height, GLenum format, GLenum type, const void* pixels
) {
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
    const std::size_t pitch =
        (static_cast<std::size_t>(row_pixels) * static_cast<std::size_t>(from_bytes) + align - 1)
        / static_cast<std::size_t>(align) * static_cast<std::size_t>(align);
    const std::size_t row = static_cast<std::size_t>(width) * static_cast<std::size_t>(to.bytes);
    Slice s = ring(static_cast<VkDeviceSize>(row * static_cast<std::size_t>(height)));
    if (s.data == nullptr) {
        return s;
    }
    g->submission.texture_upload_bytes += row * static_cast<std::size_t>(height);
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
                    const std::uint32_t packed =
                        float_encoding::pack_small_float(rgba[0], 6, false)
                        | (float_encoding::pack_small_float(rgba[1], 6, false) << 11)
                        | (float_encoding::pack_small_float(rgba[2], 5, false) << 22);
                    std::memcpy(out + x * 4, &packed, sizeof packed);
                } else if (to.vk == VK_FORMAT_R32G32_SFLOAT) {
                    std::memcpy(out + x * 8, rgba, sizeof(float) * 2);
                } else {
                    std::uint16_t packed[4];
                    for (unsigned c = 0; c < 4; ++c) {
                        packed[c] = static_cast<std::uint16_t>(
                            float_encoding::pack_small_float(rgba[c], 10, true)
                        );
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

void copy_to_image(
    VkCommandBuffer cmd,
    Image& image,
    const Slice& from,
    int level,
    int x,
    int y,
    int width,
    int height
) {
    if (image.layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    }
    VkBufferImageCopy copy{};
    copy.bufferOffset = from.offset;
    copy.imageSubresource = {image.aspect, static_cast<std::uint32_t>(level), 0, 1};
    copy.imageOffset = {x, y, 0};
    copy.imageExtent = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1};
    vkCmdCopyBufferToImage(
        cmd, from.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy
    );
    count_copy(copy.imageExtent.width, copy.imageExtent.height);
}

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
    copy.extent =
        {static_cast<std::uint32_t>(old->width), static_cast<std::uint32_t>(old->height), 1};
    vkCmdCopyImage(
        cmd,
        old->image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        image->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &copy
    );
    count_copy(copy.extent.width, copy.extent.height);
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
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &b
        );
        count_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        const int nw = std::max(w / 2, 1);
        const int nh = std::max(h / 2, 1);
        VkImageBlit blit{};
        blit.srcSubresource = {image.aspect, static_cast<std::uint32_t>(level - 1), 0, 1};
        blit.srcOffsets[1] = {w, h, 1};
        blit.dstSubresource = {image.aspect, static_cast<std::uint32_t>(level), 0, 1};
        blit.dstOffsets[1] = {nw, nh, 1};
        vkCmdBlitImage(
            cmd,
            image.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blit,
            VK_FILTER_LINEAR
        );
        count_copy(static_cast<std::uint32_t>(nw), static_cast<std::uint32_t>(nh));
        w = nw;
        h = nh;
    }
    // All levels to one layout again: the ones above the last are TRANSFER_SRC, the last DST.
    VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    // The next barrier moves every level on to be sampled, out of TRANSFER_DST: under vk-tight-sync
    // this one orders the blits before that transfer-stage barrier (the R-08 table).
    const bool tight = renderer::render_opt::enabled(renderer::render_opt::Opt::VkTightSync);
    const VkPipelineStageFlags next =
        tight ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    b.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    b.dstAccessMask =
        tight ? VkAccessFlags{VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT}
              : VkAccessFlags{VK_ACCESS_MEMORY_READ_BIT};
    b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image.image;
    b.subresourceRange = {image.aspect, 0, static_cast<std::uint32_t>(image.levels - 1), 0, 1};
    vkCmdPipelineBarrier(
        cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, next, 0, 0, nullptr, 0, nullptr, 1, &b
    );
    count_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, next);
    image.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier(cmd, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

// ---- Samplers ----

VkSampler sampler_for(const SamplerParams& p, bool integer) {
    const std::uint64_t key =
        static_cast<std::uint64_t>(p.min - 0x2600) | static_cast<std::uint64_t>(p.mag - 0x2600) << 8
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
    info.mipmapMode = p.min == wgl::GL_LINEAR_MIPMAP_NEAREST
                              || p.min == wgl::GL_NEAREST_MIPMAP_NEAREST || !mipmapped
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

namespace {

namespace render_opt = renderer::render_opt;

// The smallest static buffer given device memory at its first draw: below it, the copy and its
// barrier cost more than drawing from the ring for a few frames saves.
constexpr VkDeviceSize kResidentBytes = 64 * 1024;

// What a buffer's own copy may be used as: every way a draw or a texel view binds a GL buffer,
// and the source of a copy (into an image, and the read-back of buffer_bytes).
constexpr VkBufferUsageFlags kOwnUsage =
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
    | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

// Works out, once, which memory a static buffer's own copy goes into (StaticMemory).
const StaticMemory& static_memory_of() {
    StaticMemory& memory = g->static_memory;

    // The device's memory does not change while it is open.
    if (memory.known) {
        return memory;
    }

    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(g->physical, &properties);
    std::uint32_t largest = 0;
    VkDeviceSize largest_size = 0;

    // The largest device-local heap: where a discrete GPU keeps its own memory.
    for (std::uint32_t heap = 0; heap < properties.memoryHeapCount; ++heap) {
        const VkMemoryHeap& h = properties.memoryHeaps[heap];

        // A bigger device-local heap than any before it.
        if ((h.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0 && h.size > largest_size) {
            largest = heap;
            largest_size = h.size;
        }
    }

    constexpr VkMemoryPropertyFlags kMapped =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    // Each type: mapped device memory on that heap, or device memory the host cannot see.
    for (std::uint32_t type = 0; type < properties.memoryTypeCount; ++type) {
        const VkMemoryType& t = properties.memoryTypes[type];
        const bool device = (t.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;

        // Device memory the host maps, on the device's main heap.
        if (device && (t.propertyFlags & kMapped) == kMapped && t.heapIndex == largest) {
            memory.mappable = true;
        }

        // Device memory the host cannot see, which only a copy on the GPU fills.
        if (device && (t.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
            memory.device_only_types |= 1u << type;
        }
    }

    memory.known = true;
    return memory;
}

// True when buffer `b` goes into device memory at its first draw and lets its mirror go: a
// static buffer of kResidentBytes or more, filled by glBufferData alone, not GPU-written.
bool goes_resident(const Buffer& b) {
    return render_opt::enabled(render_opt::Opt::StaticBuffersDeviceLocal) && b.static_hint
           && !b.storage && !b.rewritten && b.size >= kResidentBytes;
}

// A host-visible buffer of `size` bytes for `usage`, mapped, from VMA; false when it cannot be
// made (logged as `what`).
bool make_host_buffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaAllocationCreateFlags access,
    const char* what,
    Chunk& out
) {
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = size;
    info.usage = usage;
    VmaAllocationCreateInfo alloc{};
    alloc.usage = VMA_MEMORY_USAGE_AUTO;
    alloc.flags = access | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VmaAllocationInfo got{};

    // No buffer: the caller goes on without what it wanted it for.
    if (!check(
            vmaCreateBuffer(g->allocator, &info, &alloc, &out.buffer, &out.allocation, &got), what
        )) {
        return false;
    }

    out.mapped = static_cast<std::uint8_t*>(got.pMappedData);
    out.size = size;
    return true;
}

// Gives buffer `b` a copy of its own in device memory, filled with its data: one mapped
// allocation where device memory is mappable (StaticMemory), else device-only memory filled by a
// copy from a staging buffer in this recording's `upload`, which runs before `main` draws. Lets
// the mirror go unless something keeps it. False when Vulkan cannot make it.
bool make_resident(Buffer& b) {
    const StaticMemory& memory = static_memory_of();
    const bool force = render_opt::enabled(render_opt::Opt::StaticBuffersForceStaging);
    const bool staged = memory.device_only_types != 0 && (!memory.mappable || force);
    const VkDeviceSize size = b.size;
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = size;
    info.usage = kOwnUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo alloc{};
    alloc.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    // Device-only memory, filled by the GPU; or one allocation the host writes, as before.
    if (staged) {
        alloc.memoryTypeBits = memory.device_only_types;
    } else {
        alloc.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                      | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        alloc.requiredFlags =
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    VmaAllocationInfo got{};

    // No device memory: the caller takes the ring or the older path.
    if (vmaCreateBuffer(g->allocator, &info, &alloc, &buffer, &allocation, &got) != VK_SUCCESS) {
        return false;
    }

    Chunk staging;

    // The staging buffer, without which device-only memory cannot be filled.
    if (staged
        && !make_host_buffer(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            "a staging buffer",
            staging
        )) {
        vmaDestroyBuffer(g->allocator, buffer, allocation);
        return false;
    }

    VkMemoryPropertyFlags flags = 0;
    vmaGetAllocationMemoryProperties(g->allocator, allocation, &flags);

    // In memory the host can see, which a discrete GPU without resizable BAR reads over the bus.
    if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
        g->submission.buffer_static_bytes_host_visible += size;
    }

    // Filled through the staging buffer by the GPU, or written in place.
    if (staged) {
        std::memcpy(staging.mapped, b.data.data(), b.data.size());
        const VkBufferCopy region{0, 0, size};
        vkCmdCopyBuffer(upload_cmd(), staging.buffer, buffer, 1, &region);
        count_command(Command::Copy);
        defer([staging] { vmaDestroyBuffer(g->allocator, staging.buffer, staging.allocation); });
        b.staged = staging.mapped;
        b.staged_recording = g->recording;
    } else {
        std::memcpy(got.pMappedData, b.data.data(), b.data.size());
    }

    // The copy it replaces may still be drawn from by this frame.
    if (b.own != VK_NULL_HANDLE) {
        defer([old = b.own, old_allocation = b.own_allocation] {
            vmaDestroyBuffer(g->allocator, old, old_allocation);
        });
    }

    b.own = buffer;
    b.own_allocation = allocation;
    b.own_version = b.version;
    b.own_mapped = staged ? nullptr : static_cast<std::uint8_t*>(got.pMappedData);

    // The mirror goes, unless a uniform block or the CPU reads it.
    if (!b.keep_mirror) {
        g->submission.cpu_mirror_bytes -= b.data.size();
        std::vector<std::uint8_t>().swap(b.data);
        b.mirror_freed = true;
    }

    return true;
}

// Reads buffer `b`'s own copy in device-only memory into `out` (b.size bytes): a command buffer
// of its own copies it to a mapped buffer, after a barrier that orders it behind every transfer
// submitted before, and the CPU waits for it. The frame's command buffers are not touched.
void read_back(const Buffer& b, std::uint8_t* out) {
    Chunk readback;

    // Nothing to read into: the bytes stay zero, as the log says.
    if (!make_host_buffer(
            b.size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            "a buffer read-back",
            readback
        )) {
        return;
    }

    VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    pool_info.queueFamilyIndex = g->family;
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo cmd_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmd_info.commandBufferCount = 1;
    const VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    bool made = check(vkCreateCommandPool(g->device, &pool_info, nullptr, &pool), "a pool");
    cmd_info.commandPool = pool;
    made = made && check(vkAllocateCommandBuffers(g->device, &cmd_info, &cmd), "a read-back");
    made = made && check(vkCreateFence(g->device, &fence_info, nullptr, &fence), "a fence");

    // Every object made: record, submit and wait.
    if (made) {
        VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &begin);
        // The staging copy that filled `own` was submitted earlier: this waits for its write.
        VkMemoryBarrier filled{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        filled.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        filled.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            1,
            &filled,
            0,
            nullptr,
            0,
            nullptr
        );
        count_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        const VkBufferCopy region{0, 0, b.size};
        vkCmdCopyBuffer(cmd, b.own, readback.buffer, 1, &region);
        count_command(Command::Copy);
        vkEndCommandBuffer(cmd);
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;
        check(vkQueueSubmit(g->queue, 1, &submit, fence), "a read-back submit");
        ++g->submission.submits;
        ++g->measures.flushes;
        vkWaitForFences(g->device, 1, &fence, VK_TRUE, UINT64_MAX);
        vmaInvalidateAllocation(g->allocator, readback.allocation, 0, VK_WHOLE_SIZE);
        std::memcpy(out, readback.mapped, b.size);
    }

    vkDestroyFence(g->device, fence, nullptr);
    vkDestroyCommandPool(g->device, pool, nullptr);
    vmaDestroyBuffer(g->allocator, readback.buffer, readback.allocation);
}

}  // namespace

BufferRef dummy_buffer_ref() {
    return {g->dummy_buffer, 0, 256};
}

BufferRef resolve(Buffer& b, bool persistent) {
    if (b.size == 0) {
        return dummy_buffer_ref();
    }

    const VkDeviceSize size = b.size;

    if (persistent && b.own != VK_NULL_HANDLE && b.own_version == b.version) {
        return {b.own, 0, size};
    }

    if (b.gpu != VK_NULL_HANDLE && b.gpu_frame == g->frame_count && b.gpu_version == b.version) {
        return {b.gpu, b.gpu_offset, size};
    }

    // A large static buffer: into device memory at its first draw, instead of the ring for three.
    if (persistent && goes_resident(b) && make_resident(b)) {
        return {b.own, 0, size};
    }

    // Unchanged for a few frames: a copy of its own, which stays where it is.
    if (b.storage || (persistent && g->frame_count >= b.changed_frame + 3 && size >= 4096)) {
        VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        info.size = size;
        info.usage = kOwnUsage;
        VmaAllocationCreateInfo alloc{};
        alloc.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        alloc.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                      | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        alloc.requiredFlags =
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;
        VmaAllocationInfo got{};

        if (vmaCreateBuffer(g->allocator, &info, &alloc, &buffer, &allocation, &got)
            == VK_SUCCESS) {
            std::memcpy(got.pMappedData, buffer_bytes(b).data(), size);
            VkMemoryPropertyFlags memory = 0;
            vmaGetAllocationMemoryProperties(g->allocator, allocation, &memory);

            // In memory the host can see, which a discrete GPU without resizable BAR reads over
            // the bus at each draw (docs/port/RENDER_PERF.md).
            if ((memory & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
                g->submission.buffer_static_bytes_host_visible += size;
            }

            if (b.own != VK_NULL_HANDLE) {
                defer([buffer = b.own, allocation = b.own_allocation] {
                    vmaDestroyBuffer(g->allocator, buffer, allocation);
                });
            }

            b.own = buffer;
            b.own_allocation = allocation;
            b.own_version = b.version;
            b.own_mapped = static_cast<std::uint8_t*>(got.pMappedData);
            return {b.own, 0, size};
        }
    }

    const Slice s = ring_copy(buffer_bytes(b).data(), size);
    b.gpu = s.buffer;
    b.gpu_offset = s.offset;
    b.gpu_version = b.version;
    b.gpu_frame = g->frame_count;
    return {s.buffer, s.offset, size};
}

const std::vector<std::uint8_t>& buffer_bytes(Buffer& b) {
    // The mirror is there: nothing to read back.
    if (!b.mirror_freed) {
        return b.data;
    }

    b.data.assign(b.size, 0);

    // Mapped memory, the staging copy this recording has not submitted yet, or a read-back.
    if (b.own_mapped != nullptr) {
        std::memcpy(b.data.data(), b.own_mapped, b.size);
    } else if (b.staged != nullptr && b.staged_recording == g->recording) {
        std::memcpy(b.data.data(), b.staged, b.size);
    } else {
        read_back(b, b.data.data());
    }

    g->submission.cpu_mirror_bytes += b.data.size();
    b.mirror_freed = false;
    b.keep_mirror = true;
    return b.data;
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
    if (b == nullptr || b->size < static_cast<std::size_t>(texel)) {
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

}  // namespace openrac::vkgl::detail

namespace openrac::vkgl {
using namespace detail;

bool device_only_memory() {
    return g != nullptr && static_memory_of().device_only_types != 0;
}

// Make compute output visible to subsequent sampled texture reads.
bool copy_buffer_to_texture(unsigned buffer, unsigned texture, int width, int height) {
    if (!g || width <= 0 || height <= 0) {
        return false;
    }
    auto* b = g->buffers.find(buffer);
    auto* t = g->textures.find(texture);
    const auto bytes = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * 4;
    if (!b || !t || !t->image || width <= 0 || height <= 0 || t->image->width != width
        || t->image->height != height || t->image->format != VK_FORMAT_R8G8B8A8_UNORM
        || b->size < bytes) {
        return false;
    }
    end_rendering();
    const BufferRef src = resolve(*b);
    if (!src.buffer || src.size < bytes) {
        return false;
    }
    VkBufferMemoryBarrier dependency{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    dependency.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    dependency.srcQueueFamilyIndex = dependency.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dependency.buffer = src.buffer;
    dependency.offset = src.offset;
    dependency.size = bytes;
    auto cmd = frame().main;
    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        1,
        &dependency,
        0,
        nullptr
    );
    count_barrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    Image& image = *t->image;
    barrier(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.bufferOffset = src.offset;
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.imageExtent = {static_cast<unsigned>(width), static_cast<unsigned>(height), 1};
    vkCmdCopyBufferToImage(
        cmd, src.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy
    );
    count_copy(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
    barrier(cmd, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image.touched = g->recording;
    return true;
}

}  // namespace openrac::vkgl

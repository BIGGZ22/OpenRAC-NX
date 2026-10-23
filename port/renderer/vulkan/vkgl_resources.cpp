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
#include <volk.h>

#include "common/float_encoding.h"
#include "renderer/gl.h"
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

ImagePtr make_image(
    const Format& format, int width, int height, int levels, VkSampleCountFlagBits samples
) {
    // Called by the last owner to let go: the image's Vulkan objects go first, while the device
    // is open, then the Image itself.
    auto image = std::shared_ptr<Image>(std::make_unique<Image>().release(), [](Image* i) {
        std::unique_ptr<Image> owned(i);
        if (g != nullptr && g->device != VK_NULL_HANDLE) {
            destroy_image_now(*i);
        }
    });
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
    info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
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
    image->aspect = format.aspect;
    image->width = std::max(width, 1);
    image->height = std::max(height, 1);
    image->levels = levels;
    image->samples = samples;
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
        w = nw;
        h = nh;
    }
    // All levels to one layout again: the ones above the last are TRANSFER_SRC, the last DST.
    VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    b.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image.image;
    b.subresourceRange = {image.aspect, 0, static_cast<std::uint32_t>(image.levels - 1), 0, 1};
    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &b
    );
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

BufferRef dummy_buffer_ref() {
    return {g->dummy_buffer, 0, 256};
}

BufferRef resolve(Buffer& b, bool persistent) {
    if (b.data.empty()) {
        return dummy_buffer_ref();
    }
    const VkDeviceSize size = b.data.size();
    if (persistent && b.own != VK_NULL_HANDLE && b.own_version == b.version) {
        return {b.own, 0, size};
    }
    if (b.gpu != VK_NULL_HANDLE && b.gpu_frame == g->frame_count && b.gpu_version == b.version) {
        return {b.gpu, b.gpu_offset, size};
    }
    // Unchanged for a few frames: a copy of its own, which stays where it is.
    if (b.storage || (persistent && g->frame_count >= b.changed_frame + 3 && size >= 4096)) {
        VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        info.size = size;
        info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                     | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
                     | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
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
            std::memcpy(got.pMappedData, b.data.data(), b.data.size());
            if (b.own != VK_NULL_HANDLE) {
                defer([buffer = b.own, allocation = b.own_allocation] {
                    vmaDestroyBuffer(g->allocator, buffer, allocation);
                });
            }
            b.own = buffer;
            b.own_allocation = allocation;
            b.own_version = b.version;
            return {b.own, 0, size};
        }
    }
    const Slice s = ring_copy(b.data.data(), size);
    b.gpu = s.buffer;
    b.gpu_offset = s.offset;
    b.gpu_version = b.version;
    b.gpu_frame = g->frame_count;
    return {s.buffer, s.offset, size};
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
    if (b == nullptr || b->data.size() < static_cast<std::size_t>(texel)) {
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
        || b->data.size() < bytes) {
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
    Image& image = *t->image;
    barrier(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.bufferOffset = src.offset;
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.imageExtent = {static_cast<unsigned>(width), static_cast<unsigned>(height), 1};
    vkCmdCopyBufferToImage(
        cmd, src.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy
    );
    barrier(cmd, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image.touched = g->recording;
    return true;
}

}  // namespace openrac::vkgl

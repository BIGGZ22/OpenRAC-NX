// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Native compute access to the renderer's device, images and existing command recording.
#include "renderer/vulkan/native_compute.h"

#include "renderer/vulkan/vkgl_internal.h"

namespace openrac::vkgl {
using namespace detail;

bool native_device(NativeDevice& out) {
    // Optional providers cannot create or outlive the renderer's graphics context.
    if (!g || !g->open) {
        return false;
    }

    out =
        {g->device,
         g->physical,
         vkGetDeviceProcAddr,
         bool(
             g->features.shaderStorageImageExtendedFormats
             && g->features.shaderStorageImageWriteWithoutFormat
         )};
    return true;
}

bool storage_texture(unsigned texture) {
    const auto* found = g ? g->textures.find(texture) : nullptr;
    return found && found->image && (found->image->usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0;
}

bool allocate_storage_texture(unsigned texture, unsigned format, int width, int height) {
    // Storage allocations require a live name and positive dimensions.
    if (!g || width <= 0 || height <= 0) {
        return false;
    }

    auto* destination = g->textures.find(texture);
    const auto description = format_of(format);

    // Invalid texture names and unknown formats cannot be queried or allocated.
    if (!destination || description.vk == VK_FORMAT_UNDEFINED) {
        return false;
    }

    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(g->physical, description.vk, &properties);

    // Unsupported storage formats must fall back before any commands use the image.
    if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        return false;
    }

    auto image = make_image(
        description, width, height, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_STORAGE_BIT
    );

    // Allocation failures leave the previous texture intact.
    if (!image) {
        return false;
    }

    retire(std::move(destination->image));
    destination->image = std::move(image);
    destination->internal = format;
    destination->target = gl::GL_TEXTURE_2D;
    return true;
}

bool record_native_compute(
    std::span<const NativeImageUse> textures,
    const std::function<bool(VkCommandBuffer, std::span<const NativeImage>)>& record
) {
    // A native callback borrows a single active command recording.
    if (!g || !g->open || !record) {
        return false;
    }

    std::vector<ImagePtr> owners;
    std::vector<NativeImage> images;

    // Validate all inputs before changing layouts or ending the current render pass.
    for (const auto& use : textures) {
        const auto* texture = g->textures.find(use.texture);

        // Multisampled images must be resolved before a compute provider reads them.
        if (!texture || !texture->image || texture->image->samples != VK_SAMPLE_COUNT_1_BIT) {
            return false;
        }

        // An ordinary colour attachment cannot become a storage image through a layout change.
        if (use.write && !(texture->image->usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
            return false;
        }

        owners.push_back(texture->image);
        const auto& image = *texture->image;
        images.push_back(
            {image.image,
             image.format,
             image.aspect,
             static_cast<unsigned>(image.width),
             static_cast<unsigned>(image.height)}
        );
    }

    end_rendering(PassEnd::Compute);
    const auto command = frame().main;

    // Queue order and the renderer's image tracker remain authoritative across native dispatches.
    for (std::size_t i = 0; i < owners.size(); ++i) {
        barrier(
            command,
            *owners[i],
            textures[i].write ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
        owners[i]->touched = g->recording;
    }

    return record(command, images);
}
}  // namespace openrac::vkgl

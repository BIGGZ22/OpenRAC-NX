# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Prepare pinned AMD shaders and Vulkan portability fixes for CMake."""

import argparse
from pathlib import Path


def write_changed(path, text):
    """Avoid recompiling generated sources when their content has not changed."""
    data = text.encode('utf-8')
    if not path.exists() or path.read_bytes() != data:
        path.write_bytes(data)


def prepare(source, output):
    """Embed original GLSL and allow SDK device-local allocations on unified-memory GPUs."""
    output.mkdir(parents=True, exist_ok=True)
    # Linux and macOS use four-byte wchar_t, enlarging the SDK's private resource-name arrays.
    host = output / 'FidelityFX/host'
    host.mkdir(parents=True, exist_ok=True)
    context = (source / 'sdk/include/FidelityFX/host/ffx_fsr3upscaler.h').read_text(
        encoding='utf-8-sig')
    context = context.replace(
        '#define FFX_FSR3UPSCALER_CONTEXT_SIZE (FFX_SDK_DEFAULT_CONTEXT_SIZE)',
        '#define FFX_FSR3UPSCALER_CONTEXT_SIZE '
        '(FFX_SDK_DEFAULT_CONTEXT_SIZE * (sizeof(wchar_t) / sizeof(uint16_t)))')
    write_changed(host / 'ffx_fsr3upscaler.h', context)
    gpu = source / 'sdk/include/FidelityFX/gpu'
    shaders = source / 'sdk/src/backends/vk/shaders/fsr3upscaler'
    files = list(gpu.glob('*.h'))
    for folder in ('fsr3upscaler', 'fsr1', 'spd'):
        files.extend((gpu / folder).glob('*.h'))
    entries = []
    for path in sorted(files):
        text = path.read_text(encoding='utf-8-sig')
        # The host allocates FP16 luma history; the GLSL declaration must match its Vulkan view.
        if path.name == 'ffx_fsr3upscaler_callbacks_glsl.h':
            text = text.replace(
                'FSR3UPSCALER_BIND_UAV_LUMA_HISTORY, rgba8)',
                'FSR3UPSCALER_BIND_UAV_LUMA_HISTORY, rgba16f)')
        entries.append((path.relative_to(gpu).as_posix(), text))
    for path in sorted(shaders.glob('*.glsl')):
        entries.append((path.name, path.read_text(encoding='utf-8-sig')))
    header = ['// Generated from AMD FidelityFX SDK sources; do not edit.',
              '// AMD source text retains its MIT licence.', '#pragma once',
              '#include <span>', '#include <string_view>',
              'namespace openrac::enhanced::fsr3_detail {',
              'struct SdkSource { std::string_view name; std::span<const std::string_view> chunks; };']
    for index, (name, text) in enumerate(entries):
        if ')fsrsdk"' in text:
            raise ValueError('source collides with the raw-string delimiter')
        header.append(f'inline constexpr std::string_view kSource{index}[] = {{')
        for offset in range(0, len(text), 16384):
            header.append(f'R"fsrsdk({text[offset:offset + 16384]})fsrsdk",')
        header.append('};')
    header.append('inline constexpr SdkSource kSdkSources[] = {')
    for index, (name, _) in enumerate(entries):
        header.append(f'{{"{name}", kSource{index}}},')
    header.extend(['};', '}'])
    write_changed(output / 'fsr3_sources.h', '\n'.join(header) + '\n')

    backend = (source / 'sdk/src/backends/vk/ffx_vk.cpp').read_text(encoding='utf-8-sig')
    start = backend.index('uint32_t findMemoryTypeIndex(')
    end = backend.index('\nVkBufferUsageFlags ', start)
    replacement = '''// OpenRAC: retain invisible-device-local preference, with a unified-memory fallback.
uint32_t findMemoryTypeIndex(VkPhysicalDevice device, VkMemoryRequirements requirements,
                            VkMemoryPropertyFlags requested, VkMemoryPropertyFlags& selected)
{
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(device, &memory);
    uint32_t fallback = UINT32_MAX;
    for (uint32_t i = 0; i < memory.memoryTypeCount; ++i) {
        const auto flags = memory.memoryTypes[i].propertyFlags;
        if (!(requirements.memoryTypeBits & (1u << i)) || (flags & requested) != requested) {
            continue;
        }
        fallback = i;
        if (requested == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            && !(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            selected = flags;
            return i;
        }
        if ((requested & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            && (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            selected = flags;
            return i;
        }
    }
    if (fallback != UINT32_MAX) {
        selected = memory.memoryTypes[fallback].propertyFlags;
    }
    return fallback;
}
'''
    backend = backend[:start] + replacement + backend[end:]
    # The temporal-only build has no frame-generation swapchain implementation.
    backend = backend.replace(
        'backendInterface->fpSwapChainConfigureFrameGeneration = ffxSetFrameGenerationConfigToSwapchainVK;',
        'backendInterface->fpSwapChainConfigureFrameGeneration = nullptr;')
    # The renderer requires Vulkan 1.1; promoted functions need no optional KHR extension enabled.
    backend = backend.replace('"vkGetBufferMemoryRequirements2KHR"', '"vkGetBufferMemoryRequirements2"')
    write_changed(output / 'ffx_vk.cpp', backend)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    prepare(args.source, args.output)

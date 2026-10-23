// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Setting the device up and taking it down (vkgl_internal.h): the instance and the validation
// layer, the physical device, the logical device and its queue, the pipeline cache on disk, the
// frames' objects, the dummy resources, the window's surface, and destroy_all.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>
#include <volk.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "common/env.h"
#include "common/log.h"
#include "renderer/vulkan/pipeline_cache_file.h"
#include "renderer/vulkan/vkgl_internal.h"

namespace openrac::vkgl::detail {

using namespace openrac::gl;

namespace {

// ---- The device, the screen and the frames ----

std::string version_text(std::uint32_t v) {
    return std::format(
        "{}.{}.{}", VK_API_VERSION_MAJOR(v), VK_API_VERSION_MINOR(v), VK_API_VERSION_PATCH(v)
    );
}

VKAPI_ATTR VkBool32 VKAPI_CALL on_message(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*
) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        if (g) {
            ++g->validation_errors;
        }
        log::error("vulkan: {}", data->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log::warn("vulkan: {}", data->pMessage);
    }
    return VK_FALSE;
}

std::filesystem::path pipeline_cache_file() {
    if (g == nullptr) {
        return {};
    }
    // 0, off or none keep no cache; the empty string is as if the variable were unset.
    if (const std::optional<std::string_view> chosen = env::value(env::Var::VulkanPipelineCache)) {
        if (*chosen == "0" || *chosen == "off" || *chosen == "none") {
            return {};
        }
        if (!chosen->empty()) {
            return std::filesystem::path(*chosen);
        }
    }
    std::filesystem::path base;
#if defined(__APPLE__)
    const std::string home = env::system_variable("HOME").value_or(".");
    base = std::filesystem::path(home) / "Library/Caches/OpenRAC";
#elif defined(__ANDROID__)
    const std::string home = env::system_variable("HOME").value_or(".");
    base = std::filesystem::path(home) / "cache";
#else
    if (const std::optional<std::string> xdg = env::system_variable("XDG_CACHE_HOME");
        xdg.has_value() && !xdg->empty()) {
        base = std::filesystem::path(*xdg) / "openrac";
    } else {
        std::optional<std::string> home = env::system_variable("USERPROFILE");
        if (!home.has_value() || home->empty()) {
            home = env::system_variable("HOME");
        }
        base = std::filesystem::path(home.value_or(".")) / ".cache/openrac";
    }
#endif
    return base
           / std::format(
               "vulkan_pipelines_{:04x}_{:04x}_{}.bin",
               g->properties.vendorID,
               g->properties.deviceID,
               OPENRAC_VK_PIPELINE_KEY
           );
}

// Caches of this device that other builds saved (another key, or none): no longer read.
void remove_other_pipeline_caches(const std::filesystem::path& path) {
    const std::string prefix = std::format(
        "vulkan_pipelines_{:04x}_{:04x}", g->properties.vendorID, g->properties.deviceID
    );
    if (!path.filename().string().starts_with(prefix)) {
        return;  // OPENRAC_VULKAN_PIPELINE_CACHE names a file of its own: leave its folder alone
    }
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(path.parent_path(), ec)) {
        const std::string name = entry.path().filename().string();
        if (entry.path() != path && name.starts_with(prefix) && name.ends_with(".bin")) {
            std::filesystem::remove(entry.path(), ec);
        }
    }
}

bool make_pipeline_cache() {
    std::vector<char> initial_data;
    const std::filesystem::path path = pipeline_cache_file();
    if (!path.empty()) {
        SavedCache saved = read_pipeline_cache(path);
        // A save cut short or mixed from two programs is dropped; this run's save replaces it.
        if (saved.state == SavedCacheState::Damaged) {
            log::warn(
                "vulkan: pipeline cache {} is damaged, starting a fresh cache", path.string()
            );
        }
        if (saved.data.size() >= sizeof(VkPipelineCacheHeaderVersionOne)) {
            initial_data = std::move(saved.data);
        }
    }
    bool use_cache = false;
    if (!initial_data.empty()) {
        VkPipelineCacheHeaderVersionOne header{};
        std::memcpy(&header, initial_data.data(), sizeof(header));
        if (header.headerSize == sizeof(VkPipelineCacheHeaderVersionOne)
            && header.headerVersion == VK_PIPELINE_CACHE_HEADER_VERSION_ONE
            && header.vendorID == g->properties.vendorID
            && header.deviceID == g->properties.deviceID
            && std::memcmp(header.pipelineCacheUUID, g->properties.pipelineCacheUUID, VK_UUID_SIZE)
                   == 0) {
            use_cache = true;
        } else {
            log::info("vulkan: pipeline cache UUID changed, starting a fresh cache");
        }
    }
    VkPipelineCacheCreateInfo cache{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
    if (use_cache) {
        cache.initialDataSize = initial_data.size();
        cache.pInitialData = initial_data.data();
    }
    if (!check(
            vkCreatePipelineCache(g->device, &cache, nullptr, &g->pipeline_cache),
            "the pipeline cache"
        )) {
        if (use_cache) {
            log::warn("vulkan: driver refused cached pipelines, starting a fresh cache");
            cache.initialDataSize = 0;
            cache.pInitialData = nullptr;
            if (!check(
                    vkCreatePipelineCache(g->device, &cache, nullptr, &g->pipeline_cache),
                    "the pipeline cache"
                )) {
                return false;
            }
        } else {
            return false;
        }
    } else if (use_cache) {
        log::info(
            "vulkan: loaded {} bytes of cached pipelines from {}",
            initial_data.size(),
            path.string()
        );
    }
    return true;
}

void save_pipeline_cache() {
    if (g == nullptr || g->device == VK_NULL_HANDLE || g->pipeline_cache == VK_NULL_HANDLE
        || !g->open) {
        return;
    }
    const std::filesystem::path path = pipeline_cache_file();
    if (path.empty()) {
        return;
    }
    std::size_t size = 0;
    if (vkGetPipelineCacheData(g->device, g->pipeline_cache, &size, nullptr) != VK_SUCCESS
        || size <= sizeof(VkPipelineCacheHeaderVersionOne)) {
        return;
    }
    std::vector<char> buffer(size);
    if (vkGetPipelineCacheData(g->device, g->pipeline_cache, &size, buffer.data()) != VK_SUCCESS) {
        return;
    }
    std::string error;
    // Other programs may save the same file now: each saves whole, through a file of its own.
    if (!write_pipeline_cache(path, buffer, error)) {
        log::warn("vulkan: could not save the pipeline cache: {}", error);
        return;
    }
    log::info("vulkan: saved {} bytes of pipeline cache to {}", size, path.string());
    remove_other_pipeline_caches(path);
}

}  // namespace

// ---- The device, the screen and the frames ----

bool layer_available(const char* name) {
    std::uint32_t n = 0;
    vkEnumerateInstanceLayerProperties(&n, nullptr);
    std::vector<VkLayerProperties> layers(n);
    vkEnumerateInstanceLayerProperties(&n, layers.data());
    for (const VkLayerProperties& l : layers) {
        if (std::strcmp(l.layerName, name) == 0) {
            return true;
        }
    }
    return false;
}

bool make_instance(const Config& config, bool validation, std::string& error) {
    std::vector<const char*> extensions;
#if defined(__ANDROID__)
    // Asked of the driver itself (a player's own driver is not the one SDL would load).
    if (config.window != nullptr) {
        extensions = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
    }
#else
    if (config.window != nullptr) {
        Uint32 n = 0;
        const char* const* names = SDL_Vulkan_GetInstanceExtensions(&n);
        if (names == nullptr) {
            error = std::format("SDL has no Vulkan surface here: {}", SDL_GetError());
            return false;
        }
        extensions.assign(names, names + n);
    }
#endif
    std::vector<const char*> layers;
    bool debug_utils = validation;
    if (validation) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }
#if defined(OPENRAC_GPU_MARKERS)
    debug_utils |= env::flag(env::Var::GpuMarkers);
#endif
    if (debug_utils) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    // A driver that carries out only part of Vulkan (MoltenVK, on Metal) is listed by the loader
    // only to an instance that asks for these.
    VkInstanceCreateFlags flags = 0;
    {
        std::uint32_t n = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &n, nullptr);
        std::vector<VkExtensionProperties> offered(n);
        vkEnumerateInstanceExtensionProperties(nullptr, &n, offered.data());
        for (const VkExtensionProperties& e : offered) {
            if (std::strcmp(e.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0) {
                extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
        }
    }
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "OpenRAC";
    app.pEngineName = "OpenRAC";
    app.apiVersion = VK_API_VERSION_1_3;
    VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    info.flags = flags;
    info.pApplicationInfo = &app;
    info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    info.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
    info.ppEnabledLayerNames = layers.data();
    const VkResult r = vkCreateInstance(&info, nullptr, &g->instance);
    if (r != VK_SUCCESS) {
        error = std::format("no Vulkan instance ({})", static_cast<int>(r));
        return false;
    }
    volkLoadInstance(g->instance);
    if (validation) {
        VkDebugUtilsMessengerCreateInfoEXT m{
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
        };
        m.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        m.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        m.pfnUserCallback = on_message;
        vkCreateDebugUtilsMessengerEXT(g->instance, &m, nullptr, &g->messenger);
    }
    return true;
}

bool pick_device(std::string& error) {
    std::uint32_t n = 0;
    vkEnumeratePhysicalDevices(g->instance, &n, nullptr);
    std::vector<VkPhysicalDevice> devices(n);
    vkEnumeratePhysicalDevices(g->instance, &n, devices.data());
    int best = -1;
    for (VkPhysicalDevice d : devices) {
        VkPhysicalDeviceProperties p{};
        vkGetPhysicalDeviceProperties(d, &p);
        if (p.apiVersion < VK_API_VERSION_1_3) {
            continue;
        }
        VkPhysicalDeviceVulkan13Features f13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
        VkPhysicalDeviceFeatures2 f2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        f2.pNext = &f13;
        vkGetPhysicalDeviceFeatures2(d, &f2);
        if (!f13.dynamicRendering) {
            continue;
        }
        std::uint32_t qn = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qn, nullptr);
        std::vector<VkQueueFamilyProperties> queues(qn);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qn, queues.data());
        for (std::uint32_t i = 0; i < qn; ++i) {
            if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                continue;
            }
            VkBool32 present = VK_TRUE;
            if (g->surface != VK_NULL_HANDLE) {
                vkGetPhysicalDeviceSurfaceSupportKHR(d, i, g->surface, &present);
            }
            if (!present) {
                continue;
            }
            const int score = p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU     ? 4
                              : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 3
                              : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU            ? 1
                                                                                       : 2;
            if (score > best) {
                best = score;
                g->physical = d;
                g->family = i;
            }
            break;
        }
    }
    if (g->physical == VK_NULL_HANDLE) {
        error = "no Vulkan 1.3 device with dynamic rendering that can show the window";
        return false;
    }
    vkGetPhysicalDeviceProperties(g->physical, &g->properties);
    std::uint32_t families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g->physical, &families, nullptr);
    std::vector<VkQueueFamilyProperties> queues(families);
    vkGetPhysicalDeviceQueueFamilyProperties(g->physical, &families, queues.data());
    g->timestamp_bits = queues.at(g->family).timestampValidBits;
    vkGetPhysicalDeviceFeatures(g->physical, &g->features);
    return true;
}

bool make_device(std::string& error) {
    const float priority = 1.0f;
    VkDeviceQueueCreateInfo queue{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queue.queueFamilyIndex = g->family;
    queue.queueCount = 1;
    queue.pQueuePriorities = &priority;
    VkPhysicalDeviceFeatures features{};
    features.fillModeNonSolid = g->features.fillModeNonSolid;
    features.samplerAnisotropy = g->features.samplerAnisotropy;
    features.shaderClipDistance = g->features.shaderClipDistance;  // the world's clip planes
    features.shaderCullDistance = g->features.shaderCullDistance;
    features.drawIndirectFirstInstance = g->features.drawIndirectFirstInstance;
    features.multiDrawIndirect = g->features.multiDrawIndirect;
    features.multiViewport = g->features.multiViewport;
    g->features = features;
    VkPhysicalDeviceVulkan13Features f13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    f13.dynamicRendering = VK_TRUE;
    VkPhysicalDeviceVulkan11Features f11{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    VkPhysicalDeviceVulkan12Features f12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    VkPhysicalDeviceFeatures2 available{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    available.pNext = &f11;
    f11.pNext = &f12;
    vkGetPhysicalDeviceFeatures2(g->physical, &available);
    g->viewport_index_supported = features.multiViewport && f12.shaderOutputViewportIndex;
    g->bindless_supported =
        f11.shaderDrawParameters && f12.shaderSampledImageArrayNonUniformIndexing
        && f12.descriptorBindingSampledImageUpdateAfterBind && f12.descriptorBindingPartiallyBound;
    VkPhysicalDeviceDescriptorIndexingProperties indexing{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES
    };
    VkPhysicalDeviceProperties2 properties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    properties.pNext = &indexing;
    vkGetPhysicalDeviceProperties2(g->physical, &properties);
    const auto limit = std::min(
        {indexing.maxPerStageDescriptorUpdateAfterBindSamplers,
         indexing.maxPerStageDescriptorUpdateAfterBindSampledImages,
         indexing.maxDescriptorSetUpdateAfterBindSamplers,
         indexing.maxDescriptorSetUpdateAfterBindSampledImages}
    );
    g->material_texture_capacity = limit > 32 ? std::min(1024u, limit - 32u) : 0;
    g->bindless_supported = g->bindless_supported && g->material_texture_capacity >= 16;
    f11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    f12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    f12.shaderOutputViewportIndex = g->viewport_index_supported ? VK_TRUE : VK_FALSE;
    f13.pNext = &f11;
    f11.pNext = &f12;
    if (g->bindless_supported) {
        f11.shaderDrawParameters = VK_TRUE;
        f12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        f12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        f12.descriptorBindingPartiallyBound = VK_TRUE;
        f13.pNext = &f11;
        f11.pNext = &f12;
    }
    std::vector<const char*> extensions;
    if (g->surface != VK_NULL_HANDLE) {
        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    // A driver that offers it (MoltenVK) must be told the application knows it carries out only
    // part of Vulkan. What it leaves out (point polygons, wide lines, cull distances, logic ops)
    // the renderer does not use; triangle fans it has.
    {
        constexpr const char* kPortabilitySubset = "VK_KHR_portability_subset";  // vulkan_beta.h
        std::uint32_t n = 0;
        vkEnumerateDeviceExtensionProperties(g->physical, nullptr, &n, nullptr);
        std::vector<VkExtensionProperties> offered(n);
        vkEnumerateDeviceExtensionProperties(g->physical, nullptr, &n, offered.data());
        for (const VkExtensionProperties& e : offered) {
            if (std::strcmp(e.extensionName, kPortabilitySubset) == 0) {
                extensions.push_back(kPortabilitySubset);
            }
        }
    }
    VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    info.pNext = &f13;
    info.queueCreateInfoCount = 1;
    info.pQueueCreateInfos = &queue;
    info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    info.pEnabledFeatures = &features;
    const VkResult r = vkCreateDevice(g->physical, &info, nullptr, &g->device);
    if (r != VK_SUCCESS) {
        error = std::format("no Vulkan device ({})", static_cast<int>(r));
        return false;
    }
    volkLoadDevice(g->device);
    vkGetDeviceQueue(g->device, g->family, 0, &g->queue);

    VmaAllocatorCreateInfo alloc{};
    alloc.vulkanApiVersion = VK_API_VERSION_1_3;
    alloc.physicalDevice = g->physical;
    alloc.device = g->device;
    alloc.instance = g->instance;
    VmaVulkanFunctions functions{};
    vmaImportVulkanFunctionsFromVolk(&alloc, &functions);
    alloc.pVulkanFunctions = &functions;
    if (!check(vmaCreateAllocator(&alloc, &g->allocator), "the memory allocator")) {
        error = "no memory allocator";
        return false;
    }
    g->depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    for (const VkFormat f : {VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT}) {
        VkFormatProperties p{};
        vkGetPhysicalDeviceFormatProperties(g->physical, f, &p);
        if ((p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
            g->depth_format = f;
            break;
        }
    }
    return make_pipeline_cache();
}

bool make_frames() {
    for (Frame& f : g->frames) {
        if (g->timestamp_bits) {
            VkQueryPoolCreateInfo queries{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
            queries.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queries.queryCount = 512;
            if (!check(
                    vkCreateQueryPool(g->device, &queries, nullptr, &f.timestamps), "timestamp pool"
                )) {
                return false;
            }
        }
        VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        pool.queueFamilyIndex = g->family;
        if (!check(vkCreateCommandPool(g->device, &pool, nullptr, &f.pool), "a command pool")) {
            return false;
        }
        VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        alloc.commandPool = f.pool;
        alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandBufferCount = 2;
        VkCommandBuffer buffers[2];
        if (!check(vkAllocateCommandBuffers(g->device, &alloc, buffers), "command buffers")) {
            return false;
        }
        f.upload = buffers[0];
        f.main = buffers[1];
        VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(g->device, &fence, nullptr, &f.fence);
        VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(g->device, &semaphore, nullptr, &f.acquired);
    }
    return true;
}

bool make_dummies() {
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = 512;
    info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                 | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    VmaAllocationCreateInfo alloc{};
    alloc.usage = VMA_MEMORY_USAGE_AUTO;
    alloc.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    alloc.requiredFlags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VmaAllocationInfo got{};
    if (!check(
            vmaCreateBuffer(
                g->allocator, &info, &alloc, &g->dummy_buffer, &g->dummy_allocation, &got
            ),
            "the default buffer"
        )) {
        return false;
    }
    auto* bytes = static_cast<std::uint8_t*>(got.pMappedData);
    std::memset(bytes, 0, 512);
    const float one_f[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    const std::uint32_t one_u[4] = {0, 0, 0, 1};
    std::memcpy(bytes + kDefaultFloat, one_f, sizeof one_f);
    std::memcpy(bytes + kDefaultInteger, one_u, sizeof one_u);
    const VkFormat formats[] =
        {VK_FORMAT_R32_UINT,
         VK_FORMAT_R32G32B32A32_UINT,
         VK_FORMAT_R32G32B32A32_SFLOAT,
         VK_FORMAT_R32G32B32A32_SINT};
    for (std::size_t i = 0; i < std::size(formats); ++i) {
        VkBufferViewCreateInfo v{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
        v.buffer = g->dummy_buffer;
        v.format = formats[i];
        v.range = 256;
        vkCreateBufferView(g->device, &v, nullptr, &g->dummy_views[i]);
    }
    // GL's incomplete texture reads opaque black.
    g->dummy_image = make_image(
        {VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 4}, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT
    );
    if (!g->dummy_image) {
        return false;
    }
    const std::uint8_t black[4] = {0, 0, 0, 0xFF};
    const Slice s = ring_copy(black, 4);
    VkCommandBuffer cmd = upload_cmd();
    copy_to_image(cmd, *g->dummy_image, s, 0, 0, 0, 1, 1);
    barrier(cmd, *g->dummy_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return true;
}

bool make_surface(SDL_Window* window, std::string& error) {
#if defined(__ANDROID__)
    // Made with the instance's own functions, from the window SDL holds.
    auto* native = static_cast<ANativeWindow*>(SDL_GetPointerProperty(
        SDL_GetWindowProperties(window), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr
    ));
    VkAndroidSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
    info.window = native;
    if (native == nullptr
        || vkCreateAndroidSurfaceKHR(g->instance, &info, nullptr, &g->surface) != VK_SUCCESS) {
        error = "no Vulkan surface for the window";
        return false;
    }
    return true;
#else
    if (!SDL_Vulkan_CreateSurface(window, g->instance, nullptr, &g->surface)) {
        error = std::format("no Vulkan surface for the window: {}", SDL_GetError());
        return false;
    }
    return true;
#endif
}

void describe_device() {
    const VkPhysicalDeviceProperties& p = g->properties;
    g->device_name = p.deviceName;
    g->api_version = version_text(p.apiVersion);
    VkPhysicalDeviceDriverProperties driver{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES};
    VkPhysicalDeviceProperties2 p2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    p2.pNext = &driver;
    vkGetPhysicalDeviceProperties2(g->physical, &p2);
    g->driver_version = driver.driverInfo[0] != '\0'
                            ? std::format("{} {}", driver.driverName, driver.driverInfo)
                            : std::string(driver.driverName);
    g->software = p.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
    g->gl_renderer = std::format("{} (Vulkan)", g->device_name);
    g->gl_version = std::format("4.1 OpenRAC on Vulkan {}, {}", g->api_version, g->driver_version);
}

void destroy_all() {
    if (g->device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(g->device);
        gpu_release();
        material_release();
        save_pipeline_cache();
        for (Frame& f : g->frames) {
            // Window teardown can run after the statistics singleton's
            // destructor. Drain measurements only at live frame-slot reuse,
            // never call stats from static shutdown.
            run_deferred(f);
        }
        for (auto& [name, t] : g->textures.objects) {
            t.image.reset();
        }
        for (auto& [name, r] : g->renderbuffers.objects) {
            r.image.reset();
        }
        for (auto& [name, b] : g->buffers.objects) {
            destroy_buffer(b);
        }
        for (auto& [name, p] : g->programs.objects) {
            if (p.serial != 0) {
                release_program(p);
            }
        }
        for (Frame& f : g->frames) {
            run_deferred(f);
        }
        for (auto& [key, pipeline] : g->pipelines) {
            vkDestroyPipeline(g->device, pipeline, nullptr);
        }
        for (auto& [key, sampler] : g->vk_samplers) {
            vkDestroySampler(g->device, sampler, nullptr);
        }
        g->default_colour.reset();
        g->default_depth.reset();
        g->dummy_image.reset();
        for (VkBufferView v : g->dummy_views) {
            vkDestroyBufferView(g->device, v, nullptr);
        }
        if (g->dummy_buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(g->allocator, g->dummy_buffer, g->dummy_allocation);
        }
        for (Frame& f : g->frames) {
            for (const Chunk& c : f.chunks) {
                vmaDestroyBuffer(g->allocator, c.buffer, c.allocation);
            }
            for (VkDescriptorPool pool : f.pools) {
                vkDestroyDescriptorPool(g->device, pool, nullptr);
            }
            vkDestroyCommandPool(g->device, f.pool, nullptr);
            vkDestroyQueryPool(g->device, f.timestamps, nullptr);
            vkDestroyFence(g->device, f.fence, nullptr);
            vkDestroySemaphore(g->device, f.acquired, nullptr);
        }
        for (VkSemaphore s : g->rendered) {
            vkDestroySemaphore(g->device, s, nullptr);
        }
        if (g->swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(g->device, g->swapchain, nullptr);
        }
        if (g->pipeline_cache != VK_NULL_HANDLE) {
            vkDestroyPipelineCache(g->device, g->pipeline_cache, nullptr);
            g->pipeline_cache = VK_NULL_HANDLE;
        }
        if (g->allocator != nullptr) {
            vmaDestroyAllocator(g->allocator);
        }
        vkDestroyDevice(g->device, nullptr);
        g->device = VK_NULL_HANDLE;
    }
    if (g->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(g->instance, g->surface, nullptr);
    }
    if (g->messenger != VK_NULL_HANDLE) {
        vkDestroyDebugUtilsMessengerEXT(g->instance, g->messenger, nullptr);
    }
    if (g->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(g->instance, nullptr);
    }
}

}  // namespace openrac::vkgl::detail

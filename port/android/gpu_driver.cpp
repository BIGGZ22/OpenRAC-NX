// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Android's side of platform/gpu_driver.h: the player's Vulkan driver package, loaded through
// libadrenotools (BSD-2-Clause), as Android emulators load Mesa's Turnip and Qualcomm's newer
// drivers. Android lets an app load only the system's driver into the Vulkan loader's linker
// namespace; libadrenotools opens a private copy of the system's libvulkan.so whose driver lookup
// it redirects to the package's library (with hook libraries from the app's native library
// folder). It needs Qualcomm's kernel driver (/dev/kgsl-3d0): on Mali the system's driver stays.
//
// Nothing draws with Vulkan yet (the renderer is OpenGL ES there; docs/port/ANDROID.md): the
// driver is loaded, a Vulkan instance made on it to name its device, and its loader kept for the
// renderer that will. A package that does not load is logged and the system's driver used.

#include "platform/gpu_driver.h"

#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

#include <android/api-level.h>
#include <vulkan/vulkan.h>

#include "common/log.h"

#if OPENRAC_ADRENOTOOLS
#include <adrenotools/driver.h>
#endif

namespace openrac::platform {
namespace {

namespace fs = std::filesystem;

PFN_vkGetInstanceProcAddr g_get_instance_proc_addr = nullptr;

std::string env(const char* name) {
    const char* value = std::getenv(name);
    return value != nullptr ? std::string(value) : std::string();
}

// libadrenotools wants folders with their final slash.
std::string folder(std::string path) {
    if (!path.empty() && path.back() != '/') {
        path += '/';
    }
    return path;
}

// "Turnip Adreno (TM) 740, driver 25.2.0, Vulkan 1.4" from a Vulkan instance made on the driver.
std::string describe(PFN_vkGetInstanceProcAddr get) {
    auto create = reinterpret_cast<PFN_vkCreateInstance>(get(nullptr, "vkCreateInstance"));
    if (create == nullptr) {
        return "no vkCreateInstance";
    }
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "OpenRAC";
    app.apiVersion = VK_API_VERSION_1_1;
    VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    info.pApplicationInfo = &app;
    VkInstance instance = VK_NULL_HANDLE;
    if (const VkResult result = create(&info, nullptr, &instance); result != VK_SUCCESS) {
        return std::format("vkCreateInstance failed ({})", static_cast<int>(result));
    }
    auto destroy = reinterpret_cast<PFN_vkDestroyInstance>(get(instance, "vkDestroyInstance"));
    auto enumerate =
        reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(get(instance, "vkEnumeratePhysicalDevices"));
    auto properties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
        get(instance, "vkGetPhysicalDeviceProperties"));
    std::string out = "no Vulkan device";
    std::uint32_t count = 0;
    if (enumerate != nullptr && properties != nullptr && enumerate(instance, &count, nullptr) == VK_SUCCESS
        && count > 0) {
        std::vector<VkPhysicalDevice> devices(count);
        enumerate(instance, &count, devices.data());
        VkPhysicalDeviceProperties p{};
        properties(devices[0], &p);
        out = std::format("{}, driver {:#x}, Vulkan {}.{}.{}", p.deviceName, p.driverVersion,
                          VK_API_VERSION_MAJOR(p.apiVersion), VK_API_VERSION_MINOR(p.apiVersion),
                          VK_API_VERSION_PATCH(p.apiVersion));
    }
    if (destroy != nullptr) {
        destroy(instance, nullptr);
    }
    return out;
}

GpuDriver load() {
    GpuDriver driver;
    const std::string dir = env("OPENRAC_GPU_DRIVER_DIR");
    const std::string library = env("OPENRAC_GPU_DRIVER_LIBRARY");
    if (dir.empty() || library.empty()) {
        driver.detail = "the system's driver";
        return driver;
    }
    std::error_code ec;
    if (!fs::exists("/dev/kgsl-3d0", ec)) {
        driver.detail = std::format(
            "the system's driver: {} is a driver package for Adreno GPUs (Qualcomm's kernel driver, "
            "/dev/kgsl-3d0), which this device does not have",
            library);
        return driver;
    }
    if (!fs::exists(fs::path(dir) / library, ec)) {
        driver.detail = std::format("the system's driver: {} is not in {}", library, dir);
        return driver;
    }
#if OPENRAC_ADRENOTOOLS
    if (android_get_device_api_level() < 28) {
        driver.detail = "the system's driver: driver packages need Android 9 or later";
        return driver;
    }
    const std::string hooks = folder(env("OPENRAC_NATIVE_LIBRARY_DIR"));
    // A folder of the package's own for the files the driver writes (its shader cache).
    const std::string redirect = folder((fs::path(dir) / "redirect").string());
    fs::create_directories(redirect, ec);
    void* vulkan = adrenotools_open_libvulkan(RTLD_NOW, ADRENOTOOLS_DRIVER_CUSTOM | ADRENOTOOLS_DRIVER_FILE_REDIRECT,
                                              nullptr, hooks.c_str(), folder(dir).c_str(), library.c_str(),
                                              redirect.c_str(), nullptr);
    if (vulkan == nullptr) {
        driver.detail = std::format("the system's driver: libadrenotools could not load {} ({})", library,
                                    dlerror() != nullptr ? "see the log above" : "no reason given");
        return driver;
    }
    auto get = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(vulkan, "vkGetInstanceProcAddr"));
    if (get == nullptr) {
        driver.detail = std::format("the system's driver: {} has no vkGetInstanceProcAddr", library);
        return driver;
    }
    driver.custom = true;
    driver.name = describe(get);
    driver.detail = std::format("{} from {}", library, dir);
    g_get_instance_proc_addr = get;
#else
    driver.detail = "the system's driver: this build has no libadrenotools (OPENRAC_ANDROID_ADRENOTOOLS=OFF)";
#endif
    return driver;
}

}  // namespace

const GpuDriver& prepare_gpu_driver() {
    static const GpuDriver driver = [] {
        GpuDriver d = load();
        if (d.custom) {
            log::info("GPU driver: the player's package, {} ({})", d.name, d.detail);
            log::header("gpu driver  " + d.name + " (custom package)", false);
        } else {
            log::info("GPU driver: {}", d.detail);
        }
        return d;
    }();
    return driver;
}

void* custom_vulkan_get_instance_proc_addr() {
    return reinterpret_cast<void*>(g_get_instance_proc_addr);
}

}  // namespace openrac::platform

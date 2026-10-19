// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The player's own GPU driver (docs/port/ANDROID.md, "GPU drivers"). On Android a driver package
// the launcher unpacked (a Vulkan driver such as Mesa's Turnip, with its meta.json) is loaded on
// Adreno through libadrenotools, before SDL starts; everywhere else, and when there is none or it
// does not load, the system's driver is used and this does nothing. The launcher names the package
// in the environment:
//
//   OPENRAC_GPU_DRIVER_DIR      the unpacked package, in the app's private storage
//   OPENRAC_GPU_DRIVER_LIBRARY  its driver library (meta.json's libraryName)
//   OPENRAC_NATIVE_LIBRARY_DIR  the app's native library folder, where libadrenotools' hooks are

#pragma once

#include <string>

namespace openrac::platform {

struct GpuDriver {
    bool custom = false;     // the player's package is loaded
    std::string name;        // the Vulkan device and driver, as the driver says
    std::string detail;      // why the system's driver is used, or what was loaded
};

// Load the driver the environment names, once; what happened is logged and returned.
const GpuDriver& prepare_gpu_driver();

// The custom driver's vkGetInstanceProcAddr, for a renderer that draws with Vulkan; null when the
// system's driver is used (load Vulkan the usual way then).
void* custom_vulkan_get_instance_proc_addr();

}  // namespace openrac::platform

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The desktop's side of gpu_driver.h: the system's driver, always. Android's is
// android/gpu_driver.cpp.

#include "platform/gpu_driver.h"

#if !defined(__ANDROID__)

namespace openrac::platform {

const GpuDriver& prepare_gpu_driver() {
    static const GpuDriver system{false, {}, "the system's driver"};
    return system;
}

void* custom_vulkan_get_instance_proc_addr() {
    return nullptr;
}

}  // namespace openrac::platform

#endif

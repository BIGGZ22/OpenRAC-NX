// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Shared by the Switch menu, package metadata and build information.
#define OPENRAC_NX_VERSION "0.2.0"
namespace openrac::platform::switch_build {
inline constexpr const char* kLabel = "v0.2.0";
#if defined(OPENRAC_SWITCH_VULKAN_BUILD)
inline constexpr const char* kBuildLabel = "VULKAN TEST";
#elif defined(OPENRAC_SWITCH_DEKO3D_BUILD)
inline constexpr const char* kBuildLabel = "DEKO3D TEST";
#else
inline constexpr const char* kBuildLabel = "SWITCH";
#endif
}

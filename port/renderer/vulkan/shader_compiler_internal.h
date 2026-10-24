// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Shared glslang initialisation and validation flags for GL translation and native compute shaders.
#pragma once

#include <glslang/Public/ShaderLang.h>

namespace openrac::vkgl::compiler_detail {

// Validates both shader entry points against Vulkan and SPIR-V rules.
inline constexpr EShMessages kMessages =
    static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

// Initialises glslang once across both compiler entry points and calling threads.
void initialise();

}  // namespace openrac::vkgl::compiler_detail

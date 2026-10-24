// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Builds without the enhanced Vulkan upscaler expose no temporal mode and allocate no SDK resources.
#include "enhanced/fsr3/fsr3.h"

namespace openrac::enhanced {
struct Fsr3::State {};

Fsr3::Fsr3() = default;
Fsr3::~Fsr3() = default;

bool Fsr3::available() {
    return false;
}

void Fsr3::set_scene(std::span<const scene::Instance>, std::span<const renderer::Mat4>) {}

bool Fsr3::begin(TemporalFrame, renderer::TemporalQuality, float, std::string&) {
    return false;
}

void Fsr3::reset() {}

void Fsr3::release() {}
}  // namespace openrac::enhanced

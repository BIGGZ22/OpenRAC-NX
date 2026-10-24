// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Compiles AMD's supported temporal-upscaler permutations and checks their native resource reflection.
#include <bit>
#include <iostream>

#include "enhanced/fsr3/sdk_shaders.h"
#include "tests/check.h"

#include <FidelityFX/host/ffx_fsr3upscaler.h>

namespace {
// Every supported pass must compile before an SDK context can create Vulkan pipelines.
void test_upscaler_shaders_have_explicit_resources() {
    openrac::enhanced::fsr3_detail::ShaderLibrary shaders;
    std::string error;
    const bool ready = shaders.prepare(error);
    CHECK(ready);

    // Failed compilation is useful only when its original shader diagnostic is visible.
    if (!ready) {
        std::cerr << error << '\n';
        return;
    }

    FfxShaderBlob blob{};
    // SDK flags: render-resolution motion (bit 2) and reversed depth (bit 4).
    constexpr unsigned kFlags = (1u << 2) | (1u << 4);
    CHECK(shaders.find(FFX_FSR3UPSCALER_PASS_PREPARE_INPUTS, kFlags, blob));
    CHECK(blob.data != nullptr && blob.size > 0);
    CHECK(blob.cbvCount == 1);
    // Prepare inputs reads motion, depth and colour, and writes five intermediate images.
    CHECK(blob.srvTextureCount == 3);
    CHECK(blob.uavTextureCount == 5);
    CHECK(std::string(blob.boundConstantBufferNames[0]) == "cbFSR3Upscaler");
    CHECK(!shaders.find(FFX_FSR3UPSCALER_PASS_COUNT, kFlags, blob));
}
}  // namespace

int main() {
    try {
        test_upscaler_shaders_have_explicit_resources();
        return openrac::test::result();
    } catch (...) {
        return 1;
    }
}

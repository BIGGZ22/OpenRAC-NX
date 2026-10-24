// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// AMD FSR 3.1.4 context creation and dispatch over renderer-owned Vulkan images.
#include "enhanced/fsr3/sdk_context.h"

#include <bit>
#include <vector>

#include "enhanced/fsr3/sdk_shaders.h"
#include "renderer/gl.h"

#include <FidelityFX/host/backends/vk/ffx_vk.h>
#include <FidelityFX/host/ffx_fsr3upscaler.h>

namespace openrac::enhanced::fsr3_detail {
namespace {
// Describes a borrowed renderer image without taking over allocation or queue ownership.
FfxResource resource(const vkgl::NativeImage& image, bool write) {
    FfxResourceDescription description{};
    description.type = FFX_RESOURCE_TYPE_TEXTURE2D;
    description.format = ffxGetSurfaceFormatVK(image.format);
    description.width = image.width;
    description.height = image.height;
    description.depth = 1;
    description.mipCount = 1;
    description.usage = write ? FFX_RESOURCE_USAGE_UAV : FFX_RESOURCE_USAGE_READ_ONLY;

    // The SDK needs the depth attachment usage to select a depth-only sampled view.
    if (image.aspect & VK_IMAGE_ASPECT_DEPTH_BIT) {
        description.usage =
            static_cast<FfxResourceUsage>(description.usage | FFX_RESOURCE_USAGE_DEPTHTARGET);
    }

    return ffxGetResourceVK(
        reinterpret_cast<void*>(image.image),
        description,
        nullptr,
        write ? FFX_RESOURCE_STATE_UNORDERED_ACCESS : FFX_RESOURCE_STATE_COMPUTE_READ
    );
}
}  // namespace

// SDK-owned histories and descriptor storage; the renderer owns all imported frame images.
struct SdkContext::State {
    ShaderLibrary shaders;              // Cached across resolution and quality changes.
    FfxFsr3UpscalerContext context{};   // Live only after successful SDK creation.
    std::vector<std::byte> scratch;     // Backend bookkeeping kept at a stable address while live.
    std::array<int, 2> render_size{};   // Active render extent.
    std::array<int, 2> display_size{};  // Active display extent.
    bool live = false;                  // Whether the SDK resource context needs destruction.
    std::array<unsigned, 3>
        shared{};  // Required dilated depth, velocity and reconstructed depth outputs.

    // Allocates the upscaler's required outputs even though no later effect consumes them.
    bool create_shared() {
        gl::glGenTextures(static_cast<int>(shared.size()), shared.data());
        // R32F, RG16F and R32UI are the SDK's shared-resource description formats.
        constexpr std::array<unsigned, 3> kFormats{0x822E, 0x822F, 0x8236};

        // Each shared surface has the same render extent but a different SDK-required format.
        for (std::size_t n = 0; n < shared.size(); ++n) {
            gl::glBindTexture(gl::GL_TEXTURE_2D, shared[n]);

            // A partially allocated context is released by its caller before retrying.
            if (!vkgl::allocate_storage_texture(
                    shared[n], kFormats[n], render_size[0], render_size[1]
                )) {
                return false;
            }
        }

        return true;
    }

    // Creates only the temporal-upscaler effect; no swapchain or frame interpolation is registered.
    bool create(const vkgl::NativeDevice& device, std::string& error) {
        VkDeviceContext native{device.device, device.physical, device.device_proc};
        const auto bytes = ffxGetScratchMemorySizeVK(device.physical, 1);
        scratch.resize(bytes);
        FfxFsr3UpscalerContextDescription description{};
        description.flags = FFX_FSR3UPSCALER_ENABLE_DEPTH_INVERTED;
        description.maxRenderSize =
            {static_cast<unsigned>(render_size[0]), static_cast<unsigned>(render_size[1])};
        description.maxUpscaleSize =
            {static_cast<unsigned>(display_size[0]), static_cast<unsigned>(display_size[1])};
        auto result = ffxGetInterfaceVK(
            &description.backendInterface,
            ffxGetDeviceVK(&native),
            scratch.data(),
            scratch.size(),
            1
        );

        // A failed interface must not be handed to the effect's context constructor.
        if (result != FFX_OK) {
            error = "cannot create FSR Vulkan interface: " + std::to_string(result);
            return false;
        }

        shaders.activate();

        // SDK creation is synchronous; the callback owner never escapes this render-thread scope.
        try {
            result = ffxFsr3UpscalerContextCreate(&context, &description);
        } catch (...) {
            shaders.deactivate();
            throw;
        }

        shaders.deactivate();
        live = result == FFX_OK;

        // Preserve the SDK error instead of exposing an unusable temporal mode.
        if (!live) {
            error = "cannot create FSR temporal context: " + std::to_string(result);
        }

        return live;
    }

    // Maps one rendered frame into the SDK's reversed-depth, unjittered-motion convention.
    FfxErrorCode record(
        VkCommandBuffer command, std::span<const vkgl::NativeImage> images, const Dispatch& frame
    ) {
        // Combined depth/stencil has no SDK format; the caller must supply a float colour copy.
        if (images[1].aspect != VK_IMAGE_ASPECT_COLOR_BIT) {
            return FFX_ERROR_INVALID_ARGUMENT;
        }

        FfxFsr3UpscalerDispatchDescription description{};
        description.commandList = ffxGetCommandListVK(command);
        description.color = resource(images[0], false);
        description.depth = resource(images[1], false);
        description.motionVectors = resource(images[2], false);
        description.reactive = resource(images[3], false);
        description.output = resource(images[4], true);
        description.dilatedDepth = resource(images[5], true);
        description.dilatedMotionVectors = resource(images[6], true);
        description.reconstructedPrevNearestDepth = resource(images[7], true);
        description.jitterOffset = {frame.jitter[0], frame.jitter[1]};
        description.motionVectorScale = {1, 1};
        description.renderSize =
            {static_cast<unsigned>(render_size[0]), static_cast<unsigned>(render_size[1])};
        description.upscaleSize =
            {static_cast<unsigned>(display_size[0]), static_cast<unsigned>(display_size[1])};
        description.enableSharpening = frame.sharpness > 0;
        description.sharpness = frame.sharpness;
        description.frameTimeDelta = frame.milliseconds;
        description.preExposure = 1;
        description.reset = frame.reset;
        // AMD's reversed-depth contract puts the farthest plane in cameraNear.
        description.cameraNear = frame.far_plane;
        description.cameraFar = frame.near_plane;
        description.cameraFovAngleVertical = frame.vertical_fov;
        description.viewSpaceToMetersFactor = 1;
        return ffxFsr3UpscalerContextDispatch(&context, &description);
    }
};

SdkContext::SdkContext() : m_state(std::make_unique<State>()) {}

SdkContext::~SdkContext() {
    release();
}

bool SdkContext::configure(
    std::array<int, 2> render_size, std::array<int, 2> display_size, std::string& error
) {
    // Temporal upscaling accepts positive input dimensions no larger than its output.
    if (render_size[0] <= 0 || render_size[1] <= 0 || display_size[0] < render_size[0]
        || display_size[1] < render_size[1]) {
        error = "invalid FSR render or display dimensions";
        return false;
    }

    // Reuse GPU histories when only the camera or sharpening changes.
    if (m_state->live && m_state->render_size == render_size
        && m_state->display_size == display_size) {
        return true;
    }

    vkgl::NativeDevice device;

    // Unsupported devices keep the original renderer without allocating temporal resources.
    if (!gl::enhanced_rendering_available() || !vkgl::native_device(device)
        || !device.storage_formats) {
        error = "FSR requires enhanced Vulkan rendering with storage-image support";
        return false;
    }

    // Compilation fails before SDK pipeline creation, with the original pass's diagnostic.
    if (!m_state->shaders.prepare(error)) {
        return false;
    }

    release();
    m_state->render_size = render_size;
    m_state->display_size = display_size;

    // Required outputs are allocated before creating the histories that consume them.
    if (!m_state->create_shared()) {
        error = "cannot allocate required FSR shared outputs";
        release();
        return false;
    }

    // Context creation only begins after all mandatory imported outputs exist.
    if (!m_state->create(device, error)) {
        release();
        return false;
    }

    return true;
}

bool SdkContext::dispatch(
    std::array<unsigned, 5> textures, const Dispatch& frame, std::string& error
) {
    // No temporal work may be recorded before successful configuration.
    if (!m_state->live) {
        error = "FSR temporal context is not configured";
        return false;
    }

    const std::array<vkgl::NativeImageUse, 8> uses{
        {{textures[0]},
         {textures[1]},
         {textures[2]},
         {textures[3]},
         {textures[4], true},
         {m_state->shared[0], true},
         {m_state->shared[1], true},
         {m_state->shared[2], true}}
    };
    FfxErrorCode result = FFX_OK;
    // The renderer lends the command buffer and images only for this synchronous dispatch.
    const bool recorded = vkgl::record_native_compute(uses, [&](auto command, auto images) {
        result = m_state->record(command, images, frame);
        return result == FFX_OK;
    });

    // Preserve native import failures and SDK dispatch errors as separate diagnostics.
    if (!recorded) {
        error = "cannot dispatch FSR temporal upscaling: " + std::to_string(result);
    }

    return recorded;
}

void SdkContext::release() {
    // Histories may still be referenced by recorded or submitted GPU work.
    if (m_state->live) {
        gl::glFinish();
        ffxFsr3UpscalerContextDestroy(&m_state->context);
        m_state->live = false;
    }

    gl::glDeleteTextures(static_cast<int>(m_state->shared.size()), m_state->shared.data());
    m_state->shared = {};
}
}  // namespace openrac::enhanced::fsr3_detail

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# AMD's temporal upscaler, limited to enhanced Vulkan builds; no frame-generation sources.
include_guard(GLOBAL)
option(OPENRAC_FSR3 "Build AMD FSR 3.1 temporal upscaling for enhanced Vulkan rendering" ${OPENRAC_ENHANCED_RENDERING})
if(NOT OPENRAC_FSR3 OR NOT OPENRAC_ENHANCED_RENDERING OR NOT OPENRAC_VULKAN)
  add_library(openrac_fsr3 STATIC enhanced/fsr3/fsr3_stub.cpp)
  target_link_libraries(openrac_fsr3 PUBLIC openrac_renderer)
  openrac_warnings(openrac_fsr3)
  return()
endif()

# SDK 1.1.4 is the pinned release with the open Vulkan backend.
FetchContent_Declare(fsr3_sdk
  URL https://codeload.github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/tar.gz/c6efa6bf7f2027b3ec94f28578bb5965eabb9e55
  URL_HASH SHA256=eff7e8e371b2d461b980f8c75583133f771e940ed7aa00779dbbaf6dd2f88746
  SOURCE_SUBDIR openrac-download-only)
FetchContent_MakeAvailable(fsr3_sdk)
set(sdk "${fsr3_sdk_SOURCE_DIR}/sdk")
set(generated "${CMAKE_CURRENT_BINARY_DIR}/generated/fsr3")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/prepare_sdk.py"
    "${fsr3_sdk_SOURCE_DIR}" "${generated}"
  COMMAND_ERROR_IS_FATAL ANY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/prepare_sdk.py")

set(sdk_includes "${generated}" "${sdk}/include" "${sdk}/src/shared"
  "${sdk}/src/components/fsr3upscaler" "${sdk}/src/backends/shared")

# Original GLSL is compiled through the renderer's existing glslang dependency, not an extra SDK toolchain.
add_library(openrac_fsr3_shaders STATIC enhanced/fsr3/ffx_shader_blobs.cpp)
target_include_directories(openrac_fsr3_shaders SYSTEM PUBLIC ${sdk_includes})
target_include_directories(openrac_fsr3_shaders PRIVATE "${generated}")
target_link_libraries(openrac_fsr3_shaders PUBLIC openrac_renderer_vulkan)
openrac_warnings(openrac_fsr3_shaders)

add_library(openrac_fsr3_sdk STATIC
  "${sdk}/src/shared/ffx_assert.cpp"
  "${sdk}/src/shared/ffx_message.cpp"
  "${sdk}/src/shared/ffx_object_management.cpp"
  "${sdk}/src/shared/ffx_breadcrumbs_list.cpp"
  "${sdk}/src/components/fsr3upscaler/ffx_fsr3upscaler.cpp"
  "${generated}/ffx_vk.cpp")
target_include_directories(openrac_fsr3_sdk SYSTEM PUBLIC ${sdk_includes})
target_link_libraries(openrac_fsr3_sdk PRIVATE openrac_vulkan openrac_fsr3_shaders)
target_compile_definitions(openrac_fsr3_sdk PRIVATE FFX_FSR3UPSCALER NOMINMAX WIN32_LEAN_AND_MEAN)
if(MSVC)
  target_compile_options(openrac_fsr3_sdk PRIVATE
    "/FI${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/sdk_compat.h" /W0)
else()
  target_compile_options(openrac_fsr3_sdk PRIVATE
    -include "${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/sdk_compat.h" -w)
endif()

add_library(openrac_fsr3 STATIC enhanced/fsr3/sdk_context.cpp enhanced/fsr3/frame_target.cpp
  enhanced/fsr3/fsr3.cpp enhanced/fsr3/fsr3_frame.cpp)
target_link_libraries(openrac_fsr3 PUBLIC openrac_renderer_vulkan PRIVATE openrac_renderer
  openrac_scene_renderer openrac_fsr3_sdk)
openrac_embed_shaders(openrac_fsr3 fsr3 openrac::enhanced
  "${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/shaders/fullscreen.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/shaders/camera_motion.frag"
  "${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/shaders/motion.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/shaders/motion.frag"
  "${CMAKE_CURRENT_SOURCE_DIR}/enhanced/fsr3/shaders/reactive.frag")
target_include_directories(openrac_fsr3 SYSTEM PRIVATE ${sdk_includes})
openrac_warnings(openrac_fsr3)

openrac_add_test(fsr3_shaders SOURCES enhanced/tests/fsr3_shaders_test.cpp
  LIBS openrac_fsr3_shaders)
openrac_add_test(fsr3_sdk SOURCES enhanced/tests/fsr3_sdk_test.cpp
  LIBS openrac_fsr3 openrac_platform)
set_tests_properties(fsr3_sdk PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
  "OPENRAC_ENHANCED=1;OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1;OPENRAC_VULKAN_PIPELINE_CACHE=0")
openrac_add_test(fsr3_frame SOURCES enhanced/tests/fsr3_frame_test.cpp
  LIBS openrac_fsr3 openrac_platform)
set_tests_properties(fsr3_frame PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
  "OPENRAC_ENHANCED=1;OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1;OPENRAC_VULKAN_PIPELINE_CACHE=0")
openrac_add_test(fsr3_motion SOURCES enhanced/tests/fsr3_motion_test.cpp
  INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/generated" LIBS openrac_fsr3 openrac_platform openrac_enhanced_viewer)
set_tests_properties(fsr3_motion PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
  "OPENRAC_ENHANCED=1;OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1;OPENRAC_VULKAN_PIPELINE_CACHE=0")

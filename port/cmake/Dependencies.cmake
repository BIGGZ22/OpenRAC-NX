# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# The third-party libraries of the programs that open a window. Each is used
# from the system when it is installed there, and otherwise fetched by CMake
# at a pinned release, so a fresh checkout builds with nothing but a compiler.
# Nothing is vendored in the repository.
#
#   SDL3      window, OpenGL context, input, audio (zlib license)
#   cgltf     glTF 2.0 reader, for the level viewer (MIT)
#   stb       stb_image and stb_image_write, for the viewer's PNGs (MIT or public domain)
#   Dear ImGui  the games' debug overlay (MIT)
#
# To build without network access, point CMake at local clones:
#   -DFETCHCONTENT_SOURCE_DIR_SDL3=... -DFETCHCONTENT_SOURCE_DIR_CGLTF=...
#   -DFETCHCONTENT_SOURCE_DIR_STB=... -DFETCHCONTENT_SOURCE_DIR_IMGUI=...

include_guard(GLOBAL)
include(FetchContent)

# ---- SDL3 ----
# A system SDL3 is used as it is (shared or static). A fetched one is built
# static, without its test library, tests, examples or install rules: the
# port links it into each program and ships no SDL of its own.
# OPENRAC_STATIC_SDL (the `standalone` preset): never the system's, so that the program an
# installer carries needs no SDL on the player's machine.
option(OPENRAC_STATIC_SDL "Build SDL3 into the programs even when the system has one" OFF)
if(NOT OPENRAC_STATIC_SDL)
  find_package(SDL3 CONFIG QUIET)
endif()
if(SDL3_FOUND)
  message(STATUS "SDL3: system (${SDL3_VERSION})")
else()
  message(STATUS "SDL3: not installed; fetching release-3.2.24")
  # Android: libSDL3.so, which SDL's activity loads before the game (SDLActivity.getLibraries) and
  # whose Java side (tools/build_android.py copies it from this checkout of SDL) calls into it.
  if(ANDROID)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
  else()
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
  endif()
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  set(SDL_TESTS OFF CACHE BOOL "" FORCE)
  set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
  # SYSTEM keeps the port's warnings off SDL's headers (CMake 3.25 and
  # later); EXCLUDE_FROM_ALL builds only the parts the port links (3.28).
  set(openrac_sdl_options)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.25)
    list(APPEND openrac_sdl_options SYSTEM)
  endif()
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.28)
    list(APPEND openrac_sdl_options EXCLUDE_FROM_ALL)
  endif()
  FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.24
    GIT_SHALLOW TRUE
    ${openrac_sdl_options}
  )
  FetchContent_MakeAvailable(SDL3)
endif()

# One name for either kind, so the port's targets need not care.
if(NOT TARGET openrac_sdl3)
  add_library(openrac_sdl3 INTERFACE)
  if(TARGET SDL3::SDL3)
    target_link_libraries(openrac_sdl3 INTERFACE SDL3::SDL3)
  elseif(TARGET SDL3::SDL3-static)
    target_link_libraries(openrac_sdl3 INTERFACE SDL3::SDL3-static)
  else()
    message(FATAL_ERROR "SDL3 provides neither SDL3::SDL3 nor SDL3::SDL3-static")
  endif()
endif()

# ---- cgltf and stb: single headers, read by the viewer ----
# Both repositories are headers with no build of their own; each becomes an
# interface target whose headers are SYSTEM, so the port's warnings do not
# apply to them. Their implementations are compiled once, in
# viewer/third_party.c, with warnings off.
FetchContent_Declare(cgltf
  GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
  GIT_TAG v1.15
  GIT_SHALLOW TRUE
)
# stb has no releases; a commit is pinned (2026-08-01, stb_image 2.30).
FetchContent_Declare(stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG 2c980bb59875b0d32144a71867fbdebb2f77cd20
)
# Neither has a CMakeLists.txt at its top, so this only downloads them.
FetchContent_MakeAvailable(cgltf stb)

if(NOT TARGET openrac_cgltf)
  add_library(openrac_cgltf INTERFACE)
  target_include_directories(openrac_cgltf SYSTEM INTERFACE "${cgltf_SOURCE_DIR}")
  add_library(openrac_stb INTERFACE)
  target_include_directories(openrac_stb SYSTEM INTERFACE "${stb_SOURCE_DIR}")
endif()

# ---- Dear ImGui: the debug overlay of the games (game/common/debug_overlay.cpp) ----
# What OpenGOAL draws its debug windows with. The repository has no build of
# its own: its four sources and the SDL3 and OpenGL 3 backends are compiled
# here into one static library, without the port's warnings. The OpenGL
# backend brings its own small loader, so it needs nothing of renderer/gl.h.
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9b
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

if(NOT TARGET openrac_imgui)
  add_library(openrac_imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
  )
  target_include_directories(openrac_imgui SYSTEM PUBLIC "${imgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}/backends")
  target_link_libraries(openrac_imgui PUBLIC openrac_sdl3)
  if(ANDROID)
    # OpenGL ES 3 there (platform/window.h, GlApi), linked as the system's libGLESv3.
    target_compile_definitions(openrac_imgui PUBLIC IMGUI_IMPL_OPENGL_ES3)
    target_link_libraries(openrac_imgui PRIVATE GLESv3)
  elseif(UNIX AND NOT APPLE)
    target_link_libraries(openrac_imgui PRIVATE ${CMAKE_DL_LIBS})  # the backend's loader opens libGL
  endif()
  if(MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(openrac_imgui PRIVATE /w)
  else()
    target_compile_options(openrac_imgui PRIVATE -w)
  endif()
endif()

# ---- Vulkan: the renderer's other API (renderer/vulkan/vkgl.h) ----
# Vulkan-Headers (Apache-2.0 or MIT), volk (MIT: the loader, opened at run time, so a machine
# without Vulkan still runs the game with OpenGL), the Vulkan Memory Allocator (MIT) and glslang
# (BSD-3-Clause and others: the GLSL compiler, built without its optimiser, HLSL or programs).
# Not on Apple's systems, which have no Vulkan of their own.
if(APPLE)
  set(openrac_vulkan_default OFF)
else()
  set(openrac_vulkan_default ON)
endif()
option(OPENRAC_VULKAN "Build the Vulkan renderer (OPENRAC_GL_API=vulkan)" ${openrac_vulkan_default})
if(OPENRAC_VULKAN AND NOT TARGET openrac_vulkan)
  FetchContent_Declare(vulkan_headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG vulkan-sdk-1.4.350.1
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR openrac-download-only
  )
  FetchContent_Declare(volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG vulkan-sdk-1.4.350.1
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR openrac-download-only
  )
  FetchContent_Declare(vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.3.0
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR openrac-download-only
  )
  FetchContent_Declare(glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG vulkan-sdk-1.4.350.1
    GIT_SHALLOW TRUE
    ${openrac_sdl_options}
  )
  # The first three are only downloaded (SOURCE_SUBDIR names no directory): their headers and
  # volk's one source are compiled below.
  FetchContent_MakeAvailable(vulkan_headers volk vma)
  set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
  set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
  set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
  set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
  set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
  set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)
  set(openrac_shared_libs ${BUILD_SHARED_LIBS})
  set(BUILD_SHARED_LIBS OFF)
  FetchContent_MakeAvailable(glslang)
  set(BUILD_SHARED_LIBS ${openrac_shared_libs})

  add_library(openrac_vulkan STATIC "${volk_SOURCE_DIR}/volk.c")
  target_include_directories(openrac_vulkan SYSTEM PUBLIC
    "${vulkan_headers_SOURCE_DIR}/include" "${volk_SOURCE_DIR}" "${vma_SOURCE_DIR}/include")
  target_compile_definitions(openrac_vulkan PUBLIC VK_NO_PROTOTYPES)
  if(ANDROID)
    target_compile_definitions(openrac_vulkan PUBLIC VK_USE_PLATFORM_ANDROID_KHR)
  endif()
  target_link_libraries(openrac_vulkan PUBLIC ${CMAKE_DL_LIBS})
  if(MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(openrac_vulkan PRIVATE /w)
  else()
    target_compile_options(openrac_vulkan PRIVATE -w)
  endif()
  add_library(openrac_glslang INTERFACE)
  target_link_libraries(openrac_glslang INTERFACE glslang SPIRV glslang-default-resource-limits)
endif()

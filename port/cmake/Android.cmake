# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# What an Android build adds (docs/port/ANDROID.md); included from port/CMakeLists.txt when the
# NDK's toolchain file is in use (the android-arm64 preset). Nothing here touches a desktop build.
#
#   openrac_android_compat  what bionic lacks below the APIs the port's code assumes (android/compat)
#   openrac_gpu_driver      the player's own Vulkan driver package, loaded on Adreno through
#                           libadrenotools (android/gpu_driver.cpp; platform/gpu_driver.h)
#   openrac-python          the Python the launcher runs OpenRAC's tools with, when
#                           OPENRAC_ANDROID_PYTHON names an unpacked CPython Android release
#
# The games are shared libraries here, loaded by SDL's activity (cmake/Games.cmake), and SDL3 is
# libSDL3.so beside them (cmake/Dependencies.cmake), as SDL's Android project expects.

include_guard(GLOBAL)

if(NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
  message(FATAL_ERROR "OpenRAC's Android build is arm64-v8a only (game memory is a 4 GB reservation)")
endif()

# ---- bionic ----
# <execinfo.h> below API 33 (android/compat/execinfo.h falls through to bionic's from 33 on): first
# on every target's include path, so common/ and runtime/ include it as on Linux.
include_directories(BEFORE "${CMAKE_CURRENT_LIST_DIR}/../android/compat")
add_library(openrac_android_compat STATIC "${CMAKE_CURRENT_LIST_DIR}/../android/compat/execinfo.c")
target_link_libraries(openrac_android_compat PUBLIC log)

# ---- custom GPU drivers ----
# libadrenotools (BSD-2-Clause) at a pinned commit, fetched as the other dependencies are
# (cmake/Dependencies.cmake). Its hook libraries (libhook_impl.so, libmain_hook.so,
# libfile_redirect_hook.so, libgsl_alloc_hook.so) go into the APK beside the game: the library
# loads them from the app's native library folder (tools/build_android.py copies them).
option(OPENRAC_ANDROID_ADRENOTOOLS "Load the player's own Vulkan driver packages on Adreno" ON)
if(OPENRAC_ANDROID_ADRENOTOOLS)
  include(FetchContent)
  FetchContent_Declare(adrenotools
    GIT_REPOSITORY https://github.com/bylaws/libadrenotools.git
    GIT_TAG 8fae8ce254dfc1344527e05301e43f37dea2df80
    GIT_SUBMODULES_RECURSE TRUE
  )
  FetchContent_MakeAvailable(adrenotools)
endif()

# ---- the Python the launcher's tools run with ----
# The launcher runs tools/extractor.py and editor/extract.py as it does on the desktop
# (launcher/core/src/jobs.rs), so it needs a Python program. CPython's Android release is a library
# and its standard library; openrac-python is the program around it, named lib*.so so that Android
# installs it in the app's native library folder, the one place an app may run a program from.
# Either layout of an unpacked Android CPython: a prefix (include/python3.N, lib/libpython3.N.so)
# or Chaquopy's target package (include/python3.N, jniLibs/arm64-v8a/libpython3.N.so), which
# tools/build_android.py fetches from Maven Central.
set(OPENRAC_ANDROID_PYTHON "" CACHE PATH
    "An unpacked CPython for Android (include/python3.*, libpython3.*.so); empty: none")
if(OPENRAC_ANDROID_PYTHON)
  file(GLOB openrac_python_library "${OPENRAC_ANDROID_PYTHON}/lib/libpython3.*.so"
                                   "${OPENRAC_ANDROID_PYTHON}/jniLibs/arm64-v8a/libpython3.*.so")
  file(GLOB openrac_python_include "${OPENRAC_ANDROID_PYTHON}/include/python3.*")
  if(NOT openrac_python_library OR NOT openrac_python_include)
    message(FATAL_ERROR "OPENRAC_ANDROID_PYTHON=${OPENRAC_ANDROID_PYTHON} has no libpython3.*.so "
                        "or no include/python3.*")
  endif()
  list(GET openrac_python_library 0 openrac_python_library)
  list(GET openrac_python_include 0 openrac_python_include)
  add_executable(openrac-python "${CMAKE_CURRENT_LIST_DIR}/../android/python_main.c")
  target_include_directories(openrac-python SYSTEM PRIVATE "${openrac_python_include}")
  target_link_libraries(openrac-python PRIVATE "${openrac_python_library}")
  # A program, but named as a library: libopenrac-python.so.
  set_target_properties(openrac-python PROPERTIES OUTPUT_NAME "libopenrac-python.so")
  message(STATUS "Python for the launcher's tools: ${openrac_python_library}")
endif()

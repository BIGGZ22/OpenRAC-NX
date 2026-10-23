# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# The native PC port (README.md). Configure from this directory:
#
#   cmake --preset release && cmake --build --preset release && ctest --preset release
#
# The game programs (openrac-rac1-pal, one per port/game/<id>) are built from a decompilation's C by
# tools/hostgen; they need Clang (to read that C) and Python 3. The programs
# that open a window need SDL3, found on the system or fetched by CMake.
cmake_minimum_required(VERSION 3.24)
project(openrac_port LANGUAGES C CXX)
# macOS: the menu bar's code is Objective-C++ (platform/macos_app.mm), and an SDL3 that is built
# here (Dependencies.cmake) is Objective-C. Enabled here, for every directory below.
if(APPLE)
  enable_language(OBJC OBJCXX)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON) # the guest contract uses GNU statement expressions and cleanup
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "The port is 64-bit only: game memory is a 4 GB reservation.")
endif()
if(MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
  message(FATAL_ERROR "Build the port with Clang (clang-cl on Windows): the guest contract "
                      "needs __int128 and GNU C extensions that MSVC's C compiler lacks.")
endif()

# Experimental lighting (port/enhanced) is compiled in and chosen when the game starts: the
# `enhanced_rendering` setting, which the launcher's Mods entry writes, or OPENRAC_ENHANCED=1. Off
# by default, so a build with it draws as one without it until a player turns it on (renderer/gl.h).
# OFF leaves it out altogether.
option(OPENRAC_ENHANCED_RENDERING "Compile in the experimental Vulkan lighting and shadows (off at run time by default)" ON)
if(OPENRAC_ENHANCED_RENDERING)
  add_compile_definitions(OPENRAC_ENHANCED_RENDERING=1)
endif()

option(OPENRAC_PORT_WINDOW "Build the programs that open a window (needs SDL3)" ON)
option(OPENRAC_GPU_MARKERS "Compile opt-in Vulkan GPU profiler labels" OFF)
option(OPENRAC_PORT_GAMES "Build the games from their decompilations (needs Clang and Python 3)" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Warnings)
include(Tests)
include(FastBuild)
include(Optimize)
# Android (the android-arm64 preset, the NDK's toolchain file): docs/port/ANDROID.md.
if(ANDROID)
  include(Android)
endif()

enable_testing()

# ---- common: the log, crash reports, JSON, PS2 float arithmetic, helpers; no dependencies ----
# OpenRAC's version and the checkout's commit (common/build_info.h), written at every build and
# replaced only when they change (cmake/BuildInfo.cmake).
set(OPENRAC_BUILD_INFO "${CMAKE_BINARY_DIR}/generated/build_info.cpp")
add_custom_target(openrac_build_info
  COMMAND "${CMAKE_COMMAND}" "-DREPO=${PROJECT_SOURCE_DIR}/.." "-DOUT=${OPENRAC_BUILD_INFO}"
          -P "${PROJECT_SOURCE_DIR}/cmake/BuildInfo.cmake"
  BYPRODUCTS "${OPENRAC_BUILD_INFO}"
  COMMENT "OpenRAC's version and commit"
  VERBATIM)
add_library(openrac_common STATIC
  common/crash.cpp
  common/env.cpp
  common/json.cpp
  common/log.cpp
  common/process.cpp
  common/ps2_float.cpp
  common/stats.cpp
  common/system_info.cpp
  "${OPENRAC_BUILD_INFO}"
)
add_dependencies(openrac_common openrac_build_info)
target_include_directories(openrac_common PUBLIC .)
if(OPENRAC_GPU_MARKERS)
  target_compile_definitions(openrac_common PRIVATE OPENRAC_GPU_MARKERS=1)
endif()
if(UNIX AND NOT APPLE)
  target_link_libraries(openrac_common PUBLIC ${CMAKE_DL_LIBS})  # dladdr, for the crash report
endif()
if(ANDROID)
  target_link_libraries(openrac_common PUBLIC openrac_android_compat)  # backtrace below API 33
endif()
openrac_warnings(openrac_common)

openrac_add_test(log LIBS openrac_common)
find_package(Threads REQUIRED)
openrac_add_test(joining_thread LIBS openrac_common Threads::Threads)
openrac_add_test(env LIBS openrac_common)
openrac_add_test(frame_pacing INCLUDES .)
openrac_add_test(settings SOURCES tests/settings_test.cpp game/common/settings.cpp
                          LIBS openrac_common)
# The launcher's colours for the options drawer (game/common/ui_colors.h).
openrac_add_test(ui_colors SOURCES tests/ui_colors_test.cpp game/common/ui_colors.cpp
                           INCLUDES . LIBS openrac_common)
openrac_add_test(scripted_moby_colour SOURCES enhanced/tests/scripted_moby_colour_test.cpp
                                      INCLUDES .)
openrac_add_test(pause_options INCLUDES .)

# The game fixes (game/common/fixes.h): rac1's list of differences, checked as the configure reads it
# (cmake/Differences.cmake), and its fixes through the settings file. Needs no game.
include(Differences)
openrac_write_fixes(rac1 pal "${CMAKE_BINARY_DIR}/generated/openrac_fixes_test_rac1_pal.cpp")
# settings.cpp now persists the upstream game-fix registry as well.
target_sources(settings_test PRIVATE game/common/fixes.cpp
  "${CMAKE_BINARY_DIR}/generated/openrac_fixes_test_rac1_pal.cpp")
target_include_directories(settings_test PRIVATE game/common)

openrac_add_test(game_fixes
  SOURCES tests/game_fixes_test.cpp game/common/fixes.cpp game/common/settings.cpp
          "${CMAKE_BINARY_DIR}/generated/openrac_fixes_test_rac1_pal.cpp"
  INCLUDES . game/common game/common/include runtime/include
  DEFINITIONS "OPENRAC_RAC1_HOST=\"${PROJECT_SOURCE_DIR}/game/rac1/host\""
  LIBS openrac_common)

# ---- runtime: game memory, the guest contract, the executable loader ----
add_library(openrac_runtime STATIC
  runtime/crash.cpp
  runtime/ee.cpp
  runtime/elf.cpp
  runtime/guest.cpp
  runtime/memory.cpp
)
target_include_directories(openrac_runtime PUBLIC runtime/include)
target_link_libraries(openrac_runtime PUBLIC openrac_common)
if(MINGW)
  target_link_libraries(openrac_common PUBLIC dbghelp advapi32)
endif()
openrac_warnings(openrac_runtime)

openrac_add_test(runtime SOURCES tests/runtime_test.cpp tests/guest_c.c LIBS openrac_runtime)

# ---- native mods (docs/port/MODS.md): manifests, the list, the loader ----
include(Luau)
add_library(openrac_mods STATIC
  game/common/mod_library.cpp
  game/common/mod_manifest.cpp
  game/common/mod_scripts.cpp
  game/common/mods.cpp
)
target_include_directories(openrac_mods PUBLIC game/common game/common/include "${PROJECT_SOURCE_DIR}/../sdk/include")
target_link_libraries(openrac_mods PUBLIC openrac_runtime ${CMAKE_DL_LIBS}
                      PRIVATE openrac_luau)
openrac_warnings(openrac_mods)

openrac_add_test(mod_manifest LIBS openrac_mods)

# A mod built as a mod is (a shared library against the SDK's header alone), loaded by the real
# loader with hook points of the test's own.
add_executable(mod_loader_test tests/mod_loader_test.cpp)
target_link_libraries(mod_loader_test PRIVATE openrac_mods)
foreach(which a b)
  add_library(mod_loader_test_${which} MODULE tests/mod_loader_test_mod.c)
  target_include_directories(mod_loader_test_${which} PRIVATE "${PROJECT_SOURCE_DIR}/../sdk/include")
  target_compile_definitions(mod_loader_test_${which} PRIVATE "TEST_MOD_ID=\"test-${which}\"")
  set_target_properties(mod_loader_test_${which} PROPERTIES C_VISIBILITY_PRESET hidden PREFIX "")
  string(TOUPPER "${which}" upper)
  target_compile_definitions(mod_loader_test PRIVATE
    "OPENRAC_TEST_MOD_${upper}=\"$<TARGET_FILE:mod_loader_test_${which}>\"")
  add_dependencies(mod_loader_test mod_loader_test_${which})
endforeach()
openrac_warnings(mod_loader_test)
add_test(NAME mod_loader COMMAND mod_loader_test)

# Script mods (docs/port/MODS.md, section 11): the real loader and Luau with a game of the test's own,
# its binding generated from tests/script_binding_test.json, beside the native test mod.
add_executable(mod_scripts_test tests/mod_scripts_test.cpp tests/script_binding_test.c)
target_link_libraries(mod_scripts_test PRIVATE openrac_mods)
target_compile_definitions(mod_scripts_test PRIVATE "OPENRAC_TEST_MOD_A=\"$<TARGET_FILE:mod_loader_test_a>\"")
add_dependencies(mod_scripts_test mod_loader_test_a)
openrac_warnings(mod_scripts_test)
add_test(NAME mod_scripts COMMAND mod_scripts_test)

# The bindings the generator writes are what their tables give, and the tables' hook points the
# hook rows of every disc's libraries.tsv.
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
  add_test(NAME mod_binding COMMAND "${Python3_EXECUTABLE}" -m unittest discover
           -s "${PROJECT_SOURCE_DIR}/../sdk/tools" -p "test_gen_binding.py")
endif()

openrac_add_c_test(rac4_io
  SOURCES tests/rac4_io_test.c game/rac4-ntsc/host/pad.c game/rac4-ntsc/host/sound.c
          game/common/lib/pad.c
  INCLUDES runtime/include game/common/include)
openrac_add_c_test(graph_dma INCLUDES runtime/include game/common/include)

# ---- assets, media, audio, the extractor: no window ----
openrac_add_c_test(crate_init NO_STRICT_ALIASING WRAPV NO_FP_CONTRACT)

include(Assets)

# The EE FPU's float arithmetic for C (openrac/ps2float.h): the model of common/ps2_float.h behind C
# entry points, for the functions hostgen translates with it (hostgen.json "ps2_float") and the
# port's own C that computes as the console does. The games' library links it (cmake/Games.cmake).
add_library(openrac_ps2float STATIC runtime/ps2float.cpp)
target_link_libraries(openrac_ps2float PUBLIC openrac_runtime)
openrac_warnings(openrac_ps2float)

openrac_add_c_test(ps2float NO_FP_CONTRACT LIBS openrac_ps2float)

# Probe assets and CPU sampling need neither a graphics context nor Embree.
add_library(openrac_lighting STATIC enhanced/lighting/probes.cpp enhanced/lighting/probe_atlas.cpp enhanced/lighting/probe_regions.cpp)
target_include_directories(openrac_lighting PUBLIC .)
openrac_warnings(openrac_lighting)
foreach(name IN ITEMS probes probe_atlas probe_regions)
  openrac_add_test(lighting_${name} SOURCES enhanced/tests/lighting/${name}.cpp
                                    LIBS openrac_lighting)
  set_tests_properties(lighting_${name} PROPERTIES SKIP_RETURN_CODE 77)
endforeach()
include(ProbeBake)

openrac_add_c_test(joint_matrices NO_STRICT_ALIASING)
openrac_add_c_test(particle_allocate)
openrac_add_c_test(hero_animation NO_STRICT_ALIASING NO_FP_CONTRACT)
openrac_add_c_test(hero_effects NO_STRICT_ALIASING NO_FP_CONTRACT)
openrac_add_c_test(hero_items NO_STRICT_ALIASING NO_FP_CONTRACT)
openrac_add_c_test(hero_attach NO_STRICT_ALIASING NO_FP_CONTRACT)
openrac_add_c_test(moby_bounds NO_STRICT_ALIASING NO_FP_CONTRACT)
openrac_add_c_test(moby_view NO_FP_CONTRACT MATH)
openrac_add_c_test(wide_view NO_FP_CONTRACT MATH)
openrac_add_c_test(hero_manipulators NO_STRICT_ALIASING NO_FP_CONTRACT)
openrac_add_c_test(map_reveal NO_STRICT_ALIASING NO_FP_CONTRACT)
openrac_add_c_test(moby_lighting NO_STRICT_ALIASING NO_FP_CONTRACT)
openrac_add_c_test(particle_update)
openrac_add_c_test(scale_matrix)
openrac_add_c_test(rows_multiply NO_FP_CONTRACT)
openrac_add_c_test(vendor_draw NO_STRICT_ALIASING NO_FP_CONTRACT MATH)
openrac_add_c_test(effect_quad NO_STRICT_ALIASING NO_FP_CONTRACT
  INCLUDES game/common/include runtime/include game/rac1-pal/host)

# The shared rac1 host C's scene actors, with PAL's table and a stand-in for its prototypes.
openrac_add_c_test(scene_actors NO_STRICT_ALIASING WRAPV
  INCLUDES tests/rac1_host game/common/include runtime/include game/rac1-pal/host)

# The shared rac1 host C's bigger moby pool, with PAL's table and the same stand-in.
openrac_add_c_test(moby_pool NO_STRICT_ALIASING WRAPV
  INCLUDES tests/rac1_host game/common/include runtime/include game/rac1-pal/host)

openrac_add_c_test(resource_bank NO_STRICT_ALIASING)
openrac_add_c_test(exe_moby_grid NO_STRICT_ALIASING)
openrac_add_c_test(hero_lean NO_STRICT_ALIASING NO_FP_CONTRACT MATH)
openrac_add_c_test(hero_gait NO_STRICT_ALIASING NO_FP_CONTRACT MATH)
openrac_add_c_test(hero_heading NO_STRICT_ALIASING NO_FP_CONTRACT MATH)
openrac_add_c_test(hero_contact NO_STRICT_ALIASING NO_FP_CONTRACT MATH)
openrac_add_c_test(hero_vertical NO_STRICT_ALIASING NO_FP_CONTRACT MATH)
openrac_add_c_test(particle53 NO_STRICT_ALIASING NO_FP_CONTRACT MATH)
openrac_add_c_test(planar_limit NO_STRICT_ALIASING NO_FP_CONTRACT MATH)
openrac_add_c_test(checkpoint_restore NO_STRICT_ALIASING NO_FP_CONTRACT MATH)

if(OPENRAC_PORT_WINDOW)
  include(Window)
endif()

# ---- the games, from their decompilations (port/game/*/hostgen.json) ----
if(OPENRAC_PORT_GAMES)
  include(Games)
  openrac_add_games()
  # Each rac1 version's table names every function and address the shared host C uses, and nothing
  # else (port/game/rac1/README.md); no disc needed.
  foreach(version rac1-pal rac1-ntsc)
    add_test(NAME ${version}_host_table
             COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/versionmap/hostnames.py" check
                     "${PROJECT_SOURCE_DIR}/game/${version}")
  endforeach()
  # NTSC-U's own C (hand.tsv's rows) against the US code: every call and global the C reaches is
  # one the retail function reaches, or allow.tsv says why not (port/game/rac1-ntsc/README.md).
  # Skipped (77) without the retail program the decompilation's setup places.
  set(xcheck_args "")
  if(OPENRAC_RAC1_NTSC_ASM_SOURCE)
    set(xcheck_args --asm-source "${OPENRAC_RAC1_NTSC_ASM_SOURCE}")
  endif()
  add_test(NAME rac1-ntsc_hand_xcheck
           COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/versionmap/xcheck.py" check
                   "${PROJECT_SOURCE_DIR}/game/rac1-ntsc" --gen "${CMAKE_BINARY_DIR}/games/rac1-ntsc/gen" ${xcheck_args})
  set_tests_properties(rac1-ntsc_hand_xcheck PROPERTIES SKIP_RETURN_CODE 77)
  add_test(NAME xcheck_tool
           COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/versionmap/test_xcheck.py")
endif()

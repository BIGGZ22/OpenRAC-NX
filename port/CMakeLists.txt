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

option(OPENRAC_PORT_WINDOW "Build the programs that open a window (needs SDL3)" ON)
option(OPENRAC_PORT_GAMES "Build the games from their decompilations (needs Clang and Python 3)" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Warnings)
include(FastBuild)
include(Optimize)
# Android (the android-arm64 preset, the NDK's toolchain file): docs/port/ANDROID.md.
if(ANDROID)
  include(Android)
endif()

enable_testing()

# ---- common: the log, crash reports and small helpers, no dependencies ----
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
  common/log.cpp
  common/process.cpp
  common/stats.cpp
  common/system_info.cpp
  "${OPENRAC_BUILD_INFO}"
)
add_dependencies(openrac_common openrac_build_info)
target_include_directories(openrac_common PUBLIC .)
if(UNIX AND NOT APPLE)
  target_link_libraries(openrac_common PUBLIC ${CMAKE_DL_LIBS})  # dladdr, for the crash report
endif()
if(ANDROID)
  target_link_libraries(openrac_common PUBLIC openrac_android_compat)  # backtrace below API 33
endif()
openrac_warnings(openrac_common)

add_executable(log_test tests/log_test.cpp)
target_link_libraries(log_test PRIVATE openrac_common)
openrac_warnings(log_test)
add_test(NAME log COMMAND log_test)

add_executable(pause_options_test tests/pause_options_test.cpp)
target_include_directories(pause_options_test PRIVATE .)
openrac_warnings(pause_options_test)
add_test(NAME pause_options COMMAND pause_options_test)

# The game fixes (game/common/fixes.h): rac1's list of differences, checked as the configure reads it
# (cmake/Differences.cmake), and its fixes through the settings file. Needs no game.
include(Differences)
openrac_write_fixes(rac1 pal "${CMAKE_BINARY_DIR}/generated/openrac_fixes_test_rac1_pal.cpp")
add_executable(game_fixes_test tests/game_fixes_test.cpp game/common/fixes.cpp game/common/settings.cpp
                               "${CMAKE_BINARY_DIR}/generated/openrac_fixes_test_rac1_pal.cpp")
target_include_directories(game_fixes_test PRIVATE . game/common game/common/include runtime/include)
target_compile_definitions(game_fixes_test PRIVATE "OPENRAC_RAC1_HOST=\"${PROJECT_SOURCE_DIR}/game/rac1/host\"")
target_link_libraries(game_fixes_test PRIVATE openrac_common)
openrac_warnings(game_fixes_test)
add_test(NAME game_fixes COMMAND game_fixes_test)

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

add_executable(runtime_test tests/runtime_test.cpp tests/guest_c.c)
target_link_libraries(runtime_test PRIVATE openrac_runtime)
openrac_warnings(runtime_test)
add_test(NAME runtime COMMAND runtime_test)

# ---- native mods (docs/port/MODS.md): manifests, the list, the loader ----
# The JSON reader the viewer and the mod loader share.
add_library(openrac_json STATIC viewer/json.cpp)
target_include_directories(openrac_json PUBLIC .)
openrac_warnings(openrac_json)

add_library(openrac_mods STATIC
  game/common/mod_library.cpp
  game/common/mod_manifest.cpp
  game/common/mods.cpp
)
target_include_directories(openrac_mods PUBLIC game/common "${PROJECT_SOURCE_DIR}/../sdk/include")
target_link_libraries(openrac_mods PUBLIC openrac_runtime openrac_json ${CMAKE_DL_LIBS})
openrac_warnings(openrac_mods)

add_executable(mod_manifest_test tests/mod_manifest_test.cpp)
target_link_libraries(mod_manifest_test PRIVATE openrac_mods)
openrac_warnings(mod_manifest_test)
add_test(NAME mod_manifest COMMAND mod_manifest_test)

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

add_executable(rac4_io_test tests/rac4_io_test.c
  game/rac4-ntsc/host/pad.c game/rac4-ntsc/host/sound.c game/common/lib/pad.c)
target_include_directories(rac4_io_test PRIVATE runtime/include game/common/include)
openrac_warnings(rac4_io_test)
add_test(NAME rac4_io COMMAND rac4_io_test)

add_executable(graph_dma_test tests/graph_dma_test.c)
target_include_directories(graph_dma_test PRIVATE runtime/include game/common/include)
openrac_warnings(graph_dma_test)
add_test(NAME graph_dma COMMAND graph_dma_test)

# ---- assets, media, audio, the extractor: no window ----
add_executable(crate_init_test tests/crate_init_test.c)
target_compile_options(crate_init_test PRIVATE -fno-strict-aliasing -fwrapv -ffp-contract=off)
openrac_warnings(crate_init_test)
add_test(NAME crate_init COMMAND crate_init_test)

include(Assets)

add_executable(joint_matrices_test tests/joint_matrices_test.c)
target_compile_options(joint_matrices_test PRIVATE -fno-strict-aliasing)
openrac_warnings(joint_matrices_test)
add_test(NAME joint_matrices COMMAND joint_matrices_test)

add_executable(particle_allocate_test tests/particle_allocate_test.c)
openrac_warnings(particle_allocate_test)
add_test(NAME particle_allocate COMMAND particle_allocate_test)

add_executable(hero_animation_test tests/hero_animation_test.c)
target_compile_options(hero_animation_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(hero_animation_test)
add_test(NAME hero_animation COMMAND hero_animation_test)

add_executable(hero_effects_test tests/hero_effects_test.c)
target_compile_options(hero_effects_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(hero_effects_test)
add_test(NAME hero_effects COMMAND hero_effects_test)

add_executable(hero_items_test tests/hero_items_test.c)
target_compile_options(hero_items_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(hero_items_test)
add_test(NAME hero_items COMMAND hero_items_test)

add_executable(hero_attach_test tests/hero_attach_test.c)
target_compile_options(hero_attach_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(hero_attach_test)
add_test(NAME hero_attach COMMAND hero_attach_test)

add_executable(moby_bounds_test tests/moby_bounds_test.c)
target_compile_options(moby_bounds_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(moby_bounds_test)
add_test(NAME moby_bounds COMMAND moby_bounds_test)

add_executable(moby_view_test tests/moby_view_test.c)
target_compile_options(moby_view_test PRIVATE -ffp-contract=off)
if(UNIX)
  target_link_libraries(moby_view_test PRIVATE m)
endif()
openrac_warnings(moby_view_test)
add_test(NAME moby_view COMMAND moby_view_test)

add_executable(wide_view_test tests/wide_view_test.c)
target_compile_options(wide_view_test PRIVATE -ffp-contract=off)
if(UNIX)
  target_link_libraries(wide_view_test PRIVATE m)
endif()
openrac_warnings(wide_view_test)
add_test(NAME wide_view COMMAND wide_view_test)

add_executable(hero_manipulators_test tests/hero_manipulators_test.c)
target_compile_options(hero_manipulators_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(hero_manipulators_test)
add_test(NAME hero_manipulators COMMAND hero_manipulators_test)

add_executable(map_reveal_test tests/map_reveal_test.c)
target_compile_options(map_reveal_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(map_reveal_test)
add_test(NAME map_reveal COMMAND map_reveal_test)

add_executable(moby_lighting_test tests/moby_lighting_test.c)
target_compile_options(moby_lighting_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(moby_lighting_test)
add_test(NAME moby_lighting COMMAND moby_lighting_test)

add_executable(particle_update_test tests/particle_update_test.c)
openrac_warnings(particle_update_test)
add_test(NAME particle_update COMMAND particle_update_test)

add_executable(scale_matrix_test tests/scale_matrix_test.c)
openrac_warnings(scale_matrix_test)
add_test(NAME scale_matrix COMMAND scale_matrix_test)

add_executable(rows_multiply_test tests/rows_multiply_test.c)
target_compile_options(rows_multiply_test PRIVATE -ffp-contract=off)
openrac_warnings(rows_multiply_test)
add_test(NAME rows_multiply COMMAND rows_multiply_test)

add_executable(vendor_draw_test tests/vendor_draw_test.c)
target_compile_options(vendor_draw_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(vendor_draw_test PRIVATE m)
endif()
openrac_warnings(vendor_draw_test)
add_test(NAME vendor_draw COMMAND vendor_draw_test)

add_executable(effect_quad_test tests/effect_quad_test.c)
target_include_directories(effect_quad_test PRIVATE game/common/include runtime/include game/rac1-pal/host)
target_compile_options(effect_quad_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
openrac_warnings(effect_quad_test)
add_test(NAME effect_quad COMMAND effect_quad_test)

# The shared rac1 host C's scene actors, with PAL's table and a stand-in for its prototypes.
add_executable(scene_actors_test tests/scene_actors_test.c)
target_include_directories(scene_actors_test PRIVATE tests/rac1_host game/common/include runtime/include
                                                     game/rac1-pal/host)
target_compile_options(scene_actors_test PRIVATE -fno-strict-aliasing -fwrapv)
openrac_warnings(scene_actors_test)
add_test(NAME scene_actors COMMAND scene_actors_test)

add_executable(resource_bank_test tests/resource_bank_test.c)
target_compile_options(resource_bank_test PRIVATE -fno-strict-aliasing)
openrac_warnings(resource_bank_test)
add_test(NAME resource_bank COMMAND resource_bank_test)

add_executable(exe_moby_grid_test tests/exe_moby_grid_test.c)
target_compile_options(exe_moby_grid_test PRIVATE -fno-strict-aliasing)
openrac_warnings(exe_moby_grid_test)
add_test(NAME exe_moby_grid COMMAND exe_moby_grid_test)

add_executable(hero_lean_test tests/hero_lean_test.c)
target_compile_options(hero_lean_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(hero_lean_test PRIVATE m)
endif()
openrac_warnings(hero_lean_test)
add_test(NAME hero_lean COMMAND hero_lean_test)

add_executable(hero_gait_test tests/hero_gait_test.c)
target_compile_options(hero_gait_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(hero_gait_test PRIVATE m)
endif()
openrac_warnings(hero_gait_test)
add_test(NAME hero_gait COMMAND hero_gait_test)

add_executable(hero_heading_test tests/hero_heading_test.c)
target_compile_options(hero_heading_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(hero_heading_test PRIVATE m)
endif()
openrac_warnings(hero_heading_test)
add_test(NAME hero_heading COMMAND hero_heading_test)

add_executable(hero_contact_test tests/hero_contact_test.c)
target_compile_options(hero_contact_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(hero_contact_test PRIVATE m)
endif()
openrac_warnings(hero_contact_test)
add_test(NAME hero_contact COMMAND hero_contact_test)

add_executable(hero_vertical_test tests/hero_vertical_test.c)
target_compile_options(hero_vertical_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(hero_vertical_test PRIVATE m)
endif()
openrac_warnings(hero_vertical_test)
add_test(NAME hero_vertical COMMAND hero_vertical_test)

add_executable(particle53_test tests/particle53_test.c)
target_compile_options(particle53_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(particle53_test PRIVATE m)
endif()
openrac_warnings(particle53_test)
add_test(NAME particle53 COMMAND particle53_test)

add_executable(planar_limit_test tests/planar_limit_test.c)
target_compile_options(planar_limit_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(planar_limit_test PRIVATE m)
endif()
openrac_warnings(planar_limit_test)
add_test(NAME planar_limit COMMAND planar_limit_test)

add_executable(checkpoint_restore_test tests/checkpoint_restore_test.c)
target_compile_options(checkpoint_restore_test PRIVATE -fno-strict-aliasing -ffp-contract=off)
if(UNIX)
  target_link_libraries(checkpoint_restore_test PRIVATE m)
endif()
openrac_warnings(checkpoint_restore_test)
add_test(NAME checkpoint_restore COMMAND checkpoint_restore_test)

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

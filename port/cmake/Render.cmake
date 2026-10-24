# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# What drawing costs, measured and switched: the optimisation registry, which every render
# optimisation registers with (renderer/render_opt.h, docs/port/RENDER_PERF.md), and its tests.
#
#   openrac_render_opt  the registry: a library of its own, apart from openrac_renderer, because it
#                       has no GL or window in it and its tests run in the headless preset too
#                       (the renderer module's one other target besides openrac_renderer_world
#                       and openrac_renderer_vulkan)
#
# The renderers link it where they exist, so that their draw code can ask it. Tests:
#
#   render_opt              the table and OPENRAC_RENDER_OPT_OFF's parsing, everywhere
#   render_opt_environment  the variable read and applied, as this file sets it
#   render_opt_vulkan       the Vulkan backend's render counters follow the draws (skipped, 77,
#                           where Vulkan cannot open)
#   vulkan_buffer           a static GL buffer's data drawn as GL would through the backend's
#                           changes to where it keeps it (skipped, 77, where Vulkan cannot open)
#   vulkan_present          the one copy to the screen against the two it replaces, on stand-ins
#                           of the screen (skipped, 77, where Vulkan cannot open)
#
# Included from port/CMakeLists.txt after cmake/Window.cmake, with or without the window.

include_guard(GLOBAL)

add_library(openrac_render_opt STATIC renderer/render_opt.cpp)
target_link_libraries(openrac_render_opt PUBLIC openrac_common)
openrac_warnings(openrac_render_opt)

# The envelope of the Vulkan backend's cache files (renderer/vulkan/cache_file.h), which the
# pipeline cache and its manifest share (R-10). Added here so that cmake/Window.cmake, which makes
# the target, is left to its own lane.
if(TARGET openrac_renderer_vulkan)
  target_sources(openrac_renderer_vulkan PRIVATE renderer/vulkan/cache_file.cpp)
endif()

# The renderers whose draw code asks the registry.
foreach(renderer openrac_renderer openrac_renderer_vulkan)
  if(TARGET ${renderer})
    target_link_libraries(${renderer} PUBLIC openrac_render_opt)
  endif()
endforeach()

if(TARGET openrac_renderer_vulkan)
  # With the Vulkan renderer, the same program also draws to check the counters (--vulkan).
  openrac_add_test(render_opt
    DEFINITIONS OPENRAC_RENDER_OPT_TEST_VULKAN=1
    LIBS openrac_renderer openrac_platform openrac_renderer_vulkan)
  add_test(NAME render_opt_vulkan COMMAND render_opt_test --vulkan)
  set_tests_properties(render_opt_vulkan PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
    "OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1")
else()
  openrac_add_test(render_opt LIBS openrac_render_opt)
endif()

# One entry and one name that is no entry's: the unknown one is warned of and ignored
# (tests/render_opt_test.cpp, kUnknownInEnvironment).
add_test(NAME render_opt_environment COMMAND render_opt_test --environment)
set_tests_properties(render_opt_environment PROPERTIES ENVIRONMENT
  "OPENRAC_RENDER_OPT_OFF=registry-selftest,no-such-optimisation")

# ---- The benchmark and the equivalence harness (tests/render_fixture.h's synthetic scene) ----
#
#   openrac_render_bench       draws the scene and prints its frames and counters as JSON lines
#                              (port/tools/perf/render_bench.py runs it over a matrix)
#   render_bench_smoke         ten frames of it at 640x360, to keep it working
#   render_equivalence_vulkan  every registry entry on against off, frame by frame
#
# Both draw on the Vulkan renderer through the scene's renderer, so they exist where both do; both
# exit 77 (skipped) where Vulkan cannot open. The equivalence test is written out rather than
# made by openrac_add_test, whose test takes the program's name (PROGRESS.md's follow-ups ask
# cmake/Tests.cmake for a NAME option).
if(TARGET openrac_renderer_vulkan AND TARGET openrac_viewer_lib)
  add_executable(openrac_render_bench tests/render_bench.cpp)
  target_link_libraries(openrac_render_bench PRIVATE
    openrac_viewer_lib openrac_platform)
  target_compile_definitions(openrac_render_bench PRIVATE OPENRAC_VULKAN=1)
  openrac_warnings(openrac_render_bench)
  # A build of the test programs alone (the *_test targets, as CI's Fedora job builds them) builds
  # the benchmark too, which render_bench_smoke runs.
  add_custom_target(render_bench_test)
  add_dependencies(render_bench_test openrac_render_bench)
  add_test(NAME render_bench_smoke
           COMMAND openrac_render_bench --frames 10 --warmup 2 --size 640x360)
  set_tests_properties(render_bench_smoke PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
    "OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1")

  add_executable(render_equivalence_test tests/render_equivalence_test.cpp)
  target_link_libraries(render_equivalence_test PRIVATE
    openrac_viewer_lib openrac_platform)
  target_compile_definitions(render_equivalence_test PRIVATE OPENRAC_VULKAN=1)
  openrac_warnings(render_equivalence_test)
  # No pipeline cache: its frames do not depend on what the user's cache holds, and its many
  # pipelines do not go into the cache the other tests read while they run beside it.
  add_test(NAME render_equivalence_vulkan COMMAND render_equivalence_test)
  set_tests_properties(render_equivalence_vulkan PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
    "OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1;OPENRAC_VULKAN_PIPELINE_CACHE=off")
endif()

# ---- The backend's own behaviour, pinned for the render packages ----
#
#   vulkan_buffer   a static GL buffer drawn, rewritten in part, filled again, read through 8-bit
#                   indices and as a uniform block, frame after frame (tests/vulkan_buffer_test.cpp)
#   vulkan_present  the one copy to the screen against the two (tests/vulkan_present_test.cpp)
#   cull_coverage   the zero-coverage cull's CPU reference (renderer/gpu_draw.h) against a software
#                   rasteriser (tests/cull_coverage_test.cpp); no GPU, so every preset runs it
openrac_add_test(cull_coverage INCLUDES .)
if(TARGET openrac_renderer_vulkan)
  openrac_add_test(vulkan_buffer
    LIBS openrac_renderer openrac_platform openrac_renderer_vulkan)
  set_tests_properties(vulkan_buffer PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
    "OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1")
  openrac_add_test(vulkan_present
    LIBS openrac_renderer openrac_platform openrac_renderer_vulkan)
  set_tests_properties(vulkan_present PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
    "OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1")
  # vulkan_prewarm  a run's pipelines made again on the pre-warm worker in the next run, a damaged
  #                 manifest and one of a program no longer built ignored
  #                 (tests/vulkan_prewarm_test.cpp); its pipeline cache and manifest are its own
  openrac_add_test(vulkan_prewarm
    LIBS openrac_renderer openrac_platform openrac_renderer_vulkan)
  set_tests_properties(vulkan_prewarm PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT
    "OPENRAC_GL_API=vulkan;OPENRAC_VULKAN_VALIDATION=1")
endif()

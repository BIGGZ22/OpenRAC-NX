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
#
# Included from port/CMakeLists.txt after cmake/Window.cmake, with or without the window.

include_guard(GLOBAL)

add_library(openrac_render_opt STATIC renderer/render_opt.cpp)
target_link_libraries(openrac_render_opt PUBLIC openrac_common)
openrac_warnings(openrac_render_opt)

# The renderers whose draw code asks the registry.
foreach(renderer openrac_renderer openrac_renderer_vulkan)
  if(TARGET ${renderer})
    target_link_libraries(${renderer} PUBLIC openrac_render_opt)
  endif()
endforeach()

openrac_add_test(render_opt LIBS openrac_render_opt)

# One entry and one name that is no entry's: the unknown one is warned of and ignored
# (tests/render_opt_test.cpp, kUnknownInEnvironment).
add_test(NAME render_opt_environment COMMAND render_opt_test --environment)
set_tests_properties(render_opt_environment PROPERTIES ENVIRONMENT
  "OPENRAC_RENDER_OPT_OFF=registry-selftest,no-such-optimisation")

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# ThinLTO for the builds that players run. Linux with Clang only: other
# systems and compilers build exactly as before, and the dev presets keep
# their fast links. Debug info stays whole, so crash reports symbolize.

option(OPENRAC_PORT_LTO "ThinLTO for the game programs (Linux, Clang; the standalone preset)" OFF)

if(OPENRAC_PORT_LTO)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux"
     AND CMAKE_CXX_COMPILER_ID MATCHES "Clang"
     AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-flto=thin)
    add_link_options(-flto=thin)
    message(STATUS "ThinLTO: on")
  else()
    message(STATUS "ThinLTO: requested but skipped (Linux with Clang, not Debug, only)")
  endif()
endif()

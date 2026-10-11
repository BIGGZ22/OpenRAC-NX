# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# Faster builds on Linux when the tools are installed: mold or lld links the game
# programs (tens of MB) in a fraction of GNU ld's time, ccache skips unchanged
# objects. Nothing changes when neither is found; OPENRAC_PORT_FAST_BUILD=OFF turns it off.

option(OPENRAC_PORT_FAST_BUILD "Use mold or lld, and ccache, when found (Linux)" ON)

if(OPENRAC_PORT_FAST_BUILD AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT MSVC)
  if(NOT CMAKE_C_COMPILER_LAUNCHER AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
    find_program(OPENRAC_CCACHE ccache)
    if(OPENRAC_CCACHE)
      set(CMAKE_C_COMPILER_LAUNCHER "${OPENRAC_CCACHE}")
      set(CMAKE_CXX_COMPILER_LAUNCHER "${OPENRAC_CCACHE}")
      message(STATUS "Compiler cache: ${OPENRAC_CCACHE}")
    endif()
  endif()

  if(NOT CMAKE_EXE_LINKER_FLAGS MATCHES "-fuse-ld")
    include(CheckLinkerFlag)
    foreach(linker mold lld)
      check_linker_flag(CXX "-fuse-ld=${linker}" OPENRAC_HAS_LD_${linker})
      if(OPENRAC_HAS_LD_${linker})
        add_link_options("-fuse-ld=${linker}")
        message(STATUS "Linker: ${linker}")
        break()
      endif()
    endforeach()
  endif()
endif()

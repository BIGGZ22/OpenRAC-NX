# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# ThinLTO for the builds that players run. Linux and Android with Clang only:
# other systems and compilers build exactly as before, and the dev presets
# keep their fast links. Debug info stays whole, so crash reports symbolize.
# On a handheld whose GPU has time to spare, the frame waits on the processor
# (the game's code, and the renderer's calls whichever API carries them):
# there what the compiler gains, OpenGL and Vulkan both gain.

option(OPENRAC_PORT_LTO "ThinLTO for the game programs (Linux and Android, Clang; the standalone and android-arm64 presets)" OFF)

if(OPENRAC_PORT_LTO)
  if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|Android)$"
     AND CMAKE_CXX_COMPILER_ID MATCHES "Clang"
     AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    # The archives hold LLVM bitcode, which only LLVM's own archiver reads for certain: the
    # system's ar reads it through a plugin of whatever LLVM binutils was built with, and an
    # older one than the compiler's aborts (Ubuntu 22.04's LLVM 13 against Clang 19).
    # Searched here too, not only taken from CMake's compiler checks: a build directory
    # configured before llvm-ar was installed keeps those checks' answer.
    string(REGEX MATCH "^[0-9]+" llvm_major "${CMAKE_CXX_COMPILER_VERSION}")
    get_filename_component(llvm_bin "${CMAKE_CXX_COMPILER}" REALPATH)
    get_filename_component(llvm_bin "${llvm_bin}" DIRECTORY)
    find_program(OPENRAC_LLVM_AR NAMES llvm-ar-${llvm_major} llvm-ar HINTS "${llvm_bin}")
    find_program(OPENRAC_LLVM_RANLIB NAMES llvm-ranlib-${llvm_major} llvm-ranlib HINTS "${llvm_bin}")
    if(OPENRAC_LLVM_AR AND OPENRAC_LLVM_RANLIB)
      set(CMAKE_AR "${OPENRAC_LLVM_AR}")
      set(CMAKE_RANLIB "${OPENRAC_LLVM_RANLIB}")
      add_compile_options(-flto=thin)
      add_link_options(-flto=thin)
      # What the link optimises is kept, a module at a time, beside the build (CI keeps it
      # between runs): a link where little changed redoes only that, not minutes of the whole
      # game. lld's option, checked with the linker FastBuild.cmake chose (check_linker_flag
      # does not use it by itself); another linker builds as before.
      include(CheckLinkerFlag)
      set(thinlto_cache "${CMAKE_BINARY_DIR}/thinlto-cache")
      get_directory_property(chosen_link_options LINK_OPTIONS)
      list(JOIN chosen_link_options " " CMAKE_REQUIRED_FLAGS)
      check_linker_flag(CXX "-Wl,--thinlto-cache-dir=${thinlto_cache}" OPENRAC_HAS_THINLTO_CACHE)
      unset(CMAKE_REQUIRED_FLAGS)
      if(OPENRAC_HAS_THINLTO_CACHE)
        add_link_options("-Wl,--thinlto-cache-dir=${thinlto_cache}"
                         "-Wl,--thinlto-cache-policy=cache_size_bytes=2g")
        message(STATUS "ThinLTO: on (archiver ${CMAKE_AR}, cache ${thinlto_cache})")
      else()
        message(STATUS "ThinLTO: on (archiver ${CMAKE_AR})")
      endif()
    else()
      message(STATUS "ThinLTO: requested but skipped (no llvm-ar beside the compiler)")
    endif()
  else()
    message(STATUS "ThinLTO: requested but skipped (Linux with Clang, not Debug, only)")
  endif()
endif()

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# The OpenRAC mod SDK as a CMake package (docs/port/MODS.md, section 8; sdk/README.md).
#
#   find_package(OpenRACMod 1.0 REQUIRED)        # -DOpenRACMod_DIR=<OpenRAC>/sdk/cmake
#   openrac_add_mod(my_mod MANIFEST openrac-mod.json SOURCES src/mod.c)
#
# gives:
#
#   OpenRAC::mod                the interface's headers (openrac/openrac_mod.h, openrac/games/*.h)
#   OPENRAC_MOD_PLATFORM        this build's platform key: macos-arm64, macos-x86_64, linux-x86_64,
#                               linux-aarch64, windows-x86_64 or windows-arm64
#   OPENRAC_MOD_SDK_DIR         the SDK's folder (its tools: ${OPENRAC_MOD_SDK_DIR}/tools)
#   openrac_add_mod(<target> MANIFEST <file> SOURCES <file>... [PACKAGE_DIR <dir>])
#
# The library a mod builds is named after its target, with no prefix on any platform:
# <target>.dylib, <target>.so, <target>.dll. The build writes its folder as the mod's archive holds
# it (by default <build>/package/<target>): the manifest, its "plugin" naming this platform's
# library alone (lib/<platform>/<file>, whatever the source manifest listed), the library there, and
# the README and LICENSE files beside the manifest. sdk/tools/package_mod.py turns that
# folder into the .zip.

cmake_minimum_required(VERSION 3.20)

get_filename_component(OPENRAC_MOD_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT TARGET OpenRAC::mod)
  add_library(OpenRAC::mod INTERFACE IMPORTED)
  set_target_properties(OpenRAC::mod PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${OPENRAC_MOD_SDK_DIR}/include")
endif()

# The platform key: the system, then the processor the build targets.
function(_openrac_mod_platform out)
  if(APPLE)
    set(system macos)
    set(arch "${CMAKE_OSX_ARCHITECTURES}")
    list(LENGTH arch count)
    if(count GREATER 1)
      message(FATAL_ERROR "openrac_add_mod: one architecture per build (CMAKE_OSX_ARCHITECTURES is ${arch}); "
                          "build each and join the archives with package_mod.py --merge")
    endif()
    if(arch STREQUAL "")
      set(arch "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
  elseif(WIN32)
    set(system windows)
    # The target processor: Visual Studio's platform when it names one, else the system's.
    if(CMAKE_GENERATOR_PLATFORM)
      set(arch "${CMAKE_GENERATOR_PLATFORM}")
    elseif(CMAKE_C_COMPILER_ARCHITECTURE_ID)
      set(arch "${CMAKE_C_COMPILER_ARCHITECTURE_ID}")
    else()
      set(arch "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(system linux)
    set(arch "${CMAKE_SYSTEM_PROCESSOR}")
  else()
    message(FATAL_ERROR "openrac_add_mod: OpenRAC mods are built for macOS, Linux or Windows, not ${CMAKE_SYSTEM_NAME}")
  endif()
  string(TOLOWER "${arch}" arch)
  if(arch MATCHES "^(x86_64|amd64|x64)$")
    set(arch x86_64)
  elseif(arch MATCHES "^(arm64|aarch64|armv8.*)$")
    if(system STREQUAL "linux")
      set(arch aarch64)
    else()
      set(arch arm64)
    endif()
  else()
    message(FATAL_ERROR "openrac_add_mod: no OpenRAC platform for the processor ${arch} on ${system}")
  endif()
  set(${out} "${system}-${arch}" PARENT_SCOPE)
endfunction()

_openrac_mod_platform(OPENRAC_MOD_PLATFORM)

function(openrac_add_mod target)
  cmake_parse_arguments(PARSE_ARGV 1 MOD "" "MANIFEST;PACKAGE_DIR" "SOURCES")
  if(NOT MOD_MANIFEST)
    message(FATAL_ERROR "openrac_add_mod(${target}): MANIFEST is required")
  endif()
  if(NOT MOD_SOURCES)
    message(FATAL_ERROR "openrac_add_mod(${target}): SOURCES is required")
  endif()
  get_filename_component(manifest "${MOD_MANIFEST}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  if(NOT MOD_PACKAGE_DIR)
    set(MOD_PACKAGE_DIR "${CMAKE_BINARY_DIR}/package/${target}")
  endif()

  add_library(${target} SHARED ${MOD_SOURCES})
  target_link_libraries(${target} PRIVATE OpenRAC::mod)
  set_target_properties(${target} PROPERTIES
    PREFIX ""
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
  # A mod takes nothing from the program that loads it but the table it is handed: anything left
  # undefined is a mistake, found here rather than when the game opens the library. Apple's and
  # Microsoft's linkers refuse it already (ld64 deprecates saying -undefined error).
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(${target} PRIVATE "LINKER:--no-undefined")
  endif()

  # The manifest as the archive holds it: its plugin entry for this platform names the library.
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${manifest}")
  file(READ "${manifest}" json)
  string(JSON id ERROR_VARIABLE err GET "${json}" id)
  if(err)
    message(FATAL_ERROR "openrac_add_mod(${target}): ${manifest} has no id (${err})")
  endif()
  set(file_name "${target}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set(plugin_path "lib/${OPENRAC_MOD_PLATFORM}/${file_name}")
  # This build's platform only: the archives of the others are joined later (package_mod.py --merge).
  string(JSON json SET "${json}" plugin "{}")
  string(JSON json SET "${json}" plugin "${OPENRAC_MOD_PLATFORM}" "\"${plugin_path}\"")
  file(MAKE_DIRECTORY "${MOD_PACKAGE_DIR}/lib/${OPENRAC_MOD_PLATFORM}")
  file(WRITE "${MOD_PACKAGE_DIR}/openrac-mod.json" "${json}\n")
  # People's files beside the manifest (README, LICENSE) go in the archive too.
  get_filename_component(manifest_dir "${manifest}" DIRECTORY)
  file(GLOB extras "${manifest_dir}/README*" "${manifest_dir}/LICENSE*" "${manifest_dir}/COPYING*")
  foreach(extra IN LISTS extras)
    file(COPY "${extra}" DESTINATION "${MOD_PACKAGE_DIR}")
  endforeach()

  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${target}>" "${MOD_PACKAGE_DIR}/${plugin_path}"
    COMMENT "Packaging ${id} for ${OPENRAC_MOD_PLATFORM} in ${MOD_PACKAGE_DIR}"
    VERBATIM)
  set_target_properties(${target} PROPERTIES OPENRAC_MOD_PACKAGE_DIR "${MOD_PACKAGE_DIR}")
endfunction()

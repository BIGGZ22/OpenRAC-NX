# SPDX-License-Identifier: MIT
# Copyright (c) 2026 the OpenRAC contributors
#
# The OpenRAC mod SDK as a CMake package (docs/port/MODS.md, section 8; sdk/README.md).
#
#   find_package(OpenRACMod 1.0 REQUIRED)        # -DOpenRACMod_DIR=<OpenRAC>/sdk/cmake
#   openrac_add_mod(my_mod MANIFEST openrac-mod.json SOURCES src/mod.cpp)   # C or C++ sources
#
# gives:
#
#   OpenRAC::mod                the interface's headers (openrac/openrac_mod.h, openrac/games/*.h) and
#                               its C++ layer (openrac/mod.hpp, openrac/games/*.hpp)
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

# Sets `out` (in the caller's scope) to the platform key of this build: the system, a dash, then the
# processor the build targets (macos-arm64, linux-x86_64, windows-x86_64, ...). Stops with an error
# for a system or a processor the port does not run on.
function(_openrac_mod_platform out)
  # macOS: the processor is the one architecture the build was told to make.
  if(APPLE)
    set(system macos)
    set(arch "${CMAKE_OSX_ARCHITECTURES}")
    list(LENGTH arch count)
    # A universal build would put two libraries in one file; the archive holds one per platform.
    if(count GREATER 1)
      message(FATAL_ERROR "openrac_add_mod: one architecture per build (CMAKE_OSX_ARCHITECTURES is ${arch}); "
                          "build each and join the archives with package_mod.py --merge")
    endif()
    # No architecture asked for: the build is for the machine it runs on.
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
    elseif(CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
      set(arch "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}")
    else()
      set(arch "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
  # Linux: the processor is the system's.
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(system linux)
    set(arch "${CMAKE_SYSTEM_PROCESSOR}")
  else()
    message(FATAL_ERROR "openrac_add_mod: OpenRAC mods are built for macOS, Linux or Windows, not ${CMAKE_SYSTEM_NAME}")
  endif()
  # Fold the spellings of each processor into the platform keys the manifest and the launcher use.
  string(TOLOWER "${arch}" arch)
  if(arch MATCHES "^(x86_64|amd64|x64)$")
    set(arch x86_64)
  elseif(arch MATCHES "^(arm64|aarch64|armv8.*)$")
    # The key follows each system's own name for the processor: Linux says aarch64, the others arm64.
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

# Builds a mod's shared library from SOURCES (C or C++), and writes its package folder: the manifest
# with its "plugin" set for this platform, the library, and the README and LICENSE files beside the
# manifest. `target` names the library; MANIFEST is the manifest's path (relative to the calling
# CMakeLists.txt); PACKAGE_DIR, when given, replaces <build>/package/<target>. Stops with an error
# when MANIFEST or SOURCES is missing, the manifest has no id, or its "api" is one this SDK does
# not build. Sets the target property OPENRAC_MOD_PACKAGE_DIR.
function(openrac_add_mod target)
  cmake_parse_arguments(PARSE_ARGV 1 MOD "" "MANIFEST;PACKAGE_DIR" "SOURCES")
  # Both are needed: without a manifest there is nothing to package, without sources no library.
  if(NOT MOD_MANIFEST)
    message(FATAL_ERROR "openrac_add_mod(${target}): MANIFEST is required")
  endif()
  if(NOT MOD_SOURCES)
    message(FATAL_ERROR "openrac_add_mod(${target}): SOURCES is required")
  endif()
  get_filename_component(manifest "${MOD_MANIFEST}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  # The usual place unless the caller says where the package goes.
  if(NOT MOD_PACKAGE_DIR)
    set(MOD_PACKAGE_DIR "${CMAKE_BINARY_DIR}/package/${target}")
  endif()

  add_library(${target} SHARED ${MOD_SOURCES})
  target_link_libraries(${target} PRIVATE OpenRAC::mod)
  set_target_properties(${target} PROPERTIES
    PREFIX ""
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    # C++ sources: C++17 (openrac/mod.hpp), with the compiler's own runtime as the port has it
    # (libc++ on macOS, libstdc++ on Linux, the DLL runtime /MD on Windows, CMake's default).
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
  # A mod takes nothing from the program that loads it but the table it is handed: anything left
  # undefined is a mistake, found here rather than when the game opens the library. Apple's and
  # Microsoft's linkers refuse it already (ld64 deprecates saying -undefined error).
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(${target} PRIVATE "LINKER:--no-undefined")
    # Only the entry points are seen from outside: the C++ standard library's template code a mod
    # instantiates (std::function, std::string, ...) would otherwise be exported, weak, on Linux,
    # where its headers give it default visibility whatever the build asks for.
    set(exports "${CMAKE_CURRENT_BINARY_DIR}/${target}.exports")
    file(WRITE "${exports}" "{\n  global: openrac_mod_query; openrac_mod_load; openrac_mod_unload;\n  local: *;\n};\n")
    target_link_options(${target} PRIVATE "LINKER:--version-script=${exports}")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${exports}")
  endif()

  # The manifest as the archive holds it: its plugin entry for this platform names the library.
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${manifest}")
  file(READ "${manifest}" json)
  string(JSON id ERROR_VARIABLE err GET "${json}" id)
  # The port checks the manifest's id against the library's, so a manifest without one is no mod.
  if(err)
    message(FATAL_ERROR "openrac_add_mod(${target}): ${manifest} has no id (${err})")
  endif()
  # The interface the mod is built for is its manifest's "api": a mod that says 1.1 reports 1.1
  # (OPENRAC_MOD_TARGET_MINOR, openrac_mod.h) whatever this SDK's minor, and loads on a port of 1.1.
  string(JSON api ERROR_VARIABLE err GET "${json}" api)
  # A manifest without a usable "api" leaves the library to report this SDK's own minor.
  if(NOT err AND api MATCHES "^([0-9]+)\\.([0-9]+)$")
    set(api_major "${CMAKE_MATCH_1}")
    set(api_minor "${CMAKE_MATCH_2}")
    file(STRINGS "${OPENRAC_MOD_SDK_DIR}/include/openrac/openrac_mod.h" sdk_minor_line
         REGEX "^#define OPENRAC_MOD_API_MINOR [0-9]+")
    string(REGEX REPLACE "^#define OPENRAC_MOD_API_MINOR ([0-9]+).*" "\\1" sdk_minor "${sdk_minor_line}")
    # An interface newer than this SDK's headers describe cannot be built against them.
    if(NOT api_major EQUAL 1 OR api_minor GREATER sdk_minor)
      message(FATAL_ERROR "openrac_add_mod(${target}): its manifest's api ${api} is not one this SDK "
                          "(interface 1.${sdk_minor}) builds")
    endif()
    target_compile_definitions(${target} PRIVATE "OPENRAC_MOD_TARGET_MINOR=${api_minor}")
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
  # Copy each of them as it is.
  foreach(extra IN LISTS extras)
    file(COPY "${extra}" DESTINATION "${MOD_PACKAGE_DIR}")
  endforeach()

  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${target}>" "${MOD_PACKAGE_DIR}/${plugin_path}"
    COMMENT "Packaging ${id} for ${OPENRAC_MOD_PLATFORM} in ${MOD_PACKAGE_DIR}"
    VERBATIM)
  set_target_properties(${target} PROPERTIES OPENRAC_MOD_PACKAGE_DIR "${MOD_PACKAGE_DIR}")
endfunction()

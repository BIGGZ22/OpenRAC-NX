# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# Luau (https://luau.org, MIT): the language of script mods (docs/port/MODS.md, section 11).
# Fetched at a pinned release and built static, its virtual machine and compiler only (Luau.VM,
# Luau.Compiler and the Luau.Ast and Luau.Common they need): no analysis, no code generation, no
# command-line tools. Every program and every system builds it, Android included: script mods are
# what a platform that loads no native mod still runs. Nothing of it runs until a script mod is on.
#
# The release: 0.741 (2026-10-02), commit 2d88f9b5facaa9e4ec91247b39ae8251bd49a244.
# Offline: -DFETCHCONTENT_SOURCE_DIR_LUAU=<a clone at that commit>.

include_guard(GLOBAL)
include(FetchContent)

FetchContent_Declare(luau
  GIT_REPOSITORY https://github.com/luau-lang/luau.git
  GIT_TAG 2d88f9b5facaa9e4ec91247b39ae8251bd49a244 # 0.741
)
set(LUAU_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_WEB OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
FetchContent_GetProperties(luau)
if(NOT luau_POPULATED)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.28)
    FetchContent_MakeAvailable(luau)
  else()
    FetchContent_Populate(luau)
    add_subdirectory("${luau_SOURCE_DIR}" "${luau_BINARY_DIR}" EXCLUDE_FROM_ALL)
  endif()
endif()

if(SWITCH AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # Luau's builtins use LLONG_MIN without including the standard limits header.
  # devkitA64 does not supply it through Luau's other headers.
  target_compile_options(Luau.VM PRIVATE -include climits)
endif()

if(NOT TARGET openrac_luau)
  # Only what the port links is built: Luau's own targets stay out of `all`.
  foreach(t Luau.Ast Luau.Compiler Luau.VM Luau.Analysis Luau.CodeGen Luau.Config Luau.EqSat
            Luau.Require Luau.RequireNavigator Luau.CLI.lib Luau.Repl.CLI Luau.Analyze.CLI
            Luau.Compile.CLI Luau.Reduce.CLI Luau.Bytecode.CLI Luau.Ast.CLI Luau.UnitTest
            Luau.Conformance Luau.CLI.Test isocline)
    if(TARGET ${t})
      set_target_properties(${t} PROPERTIES EXCLUDE_FROM_ALL ON)
      get_target_property(type ${t} TYPE)
      if(NOT type STREQUAL "INTERFACE_LIBRARY")
        if(MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
          target_compile_options(${t} PRIVATE /w)
        else()
          target_compile_options(${t} PRIVATE -w)
        endif()
      endif()
    endif()
  endforeach()
  add_library(openrac_luau INTERFACE)
  target_link_libraries(openrac_luau INTERFACE Luau.Compiler Luau.VM)
  # Its headers as the system's: the port's warnings are not for them.
  target_include_directories(openrac_luau SYSTEM INTERFACE "${luau_SOURCE_DIR}/VM/include"
                             "${luau_SOURCE_DIR}/Compiler/include" "${luau_SOURCE_DIR}/Common/include")
endif()

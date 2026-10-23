# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# The port's test programs, registered one way (docs/CODING_CONVENTIONS.md, section 19). A test of
# subject <subject> is the program <subject>_test and the ctest test <subject>; it is built with
# openrac_warnings like every target of the port and passes when it returns 0 (tests/check.h).
#
#   openrac_add_test(<subject> [SOURCES <file>...] [INCLUDES <dir>...] [DEFINITIONS <def>...]
#                    [LIBS <target>...])
#     A test written in C++. It may compile C beside it (tests/guest_c.c).
#
#   openrac_add_c_test(<subject> [SOURCES <file>...] [INCLUDES <dir>...] [DEFINITIONS <def>...]
#                      [LIBS <target>...] [NO_STRICT_ALIASING] [WRAPV] [NO_FP_CONTRACT] [MATH])
#     A test written in C, most often of the game-side C (the hand-written C, the shared host C),
#     which it compiles with the options that C is built with:
#       NO_STRICT_ALIASING  -fno-strict-aliasing: the game's C reads one memory as several types.
#       WRAPV               -fwrapv: signed arithmetic wraps, as the console's does.
#       NO_FP_CONTRACT      -ffp-contract=off: no fused multiply-add, which rounds otherwise than
#                           the console's multiply then add. openrac_warnings sets it too, but not
#                           under clang-cl.
#       MATH                links the C math library (libm), where there is one (UNIX).
#
# SOURCES defaults to tests/<subject>_test.cpp (openrac_add_test) or tests/<subject>_test.c
# (openrac_add_c_test); relative paths are from the directory that calls. INCLUDES, DEFINITIONS and
# LIBS are the target's PRIVATE include directories, compile definitions and link libraries. The
# target stays open to more: a test that needs a property or a dependency of its own sets it on
# <subject>_test or <subject> after the call (SKIP_RETURN_CODE 77 for a test that can be skipped).
#
# Tests whose shape is their own (a MODULE library loaded at run time, a Python test) stay written
# out in port/CMakeLists.txt. The tests of a module directory, one program per tests/<dir>/*.cpp,
# are cmake/Assets.cmake's openrac_add_module_tests.
#
# Included from port/CMakeLists.txt, after cmake/Warnings.cmake.

include_guard(GLOBAL)

# The work both helpers share: `lang_options` are openrac_add_c_test's game C options, `math` its
# MATH.
function(_openrac_add_test_program subject default_source sources includes definitions libs
         lang_options math)
  set(target "${subject}_test")
  if(NOT sources)
    set(sources "${default_source}")
  endif()
  add_executable(${target} ${sources})
  if(includes)
    target_include_directories(${target} PRIVATE ${includes})
  endif()
  if(definitions)
    target_compile_definitions(${target} PRIVATE ${definitions})
  endif()
  if(lang_options)
    target_compile_options(${target} PRIVATE ${lang_options})
  endif()
  if(libs)
    target_link_libraries(${target} PRIVATE ${libs})
  endif()
  if(math AND UNIX)
    target_link_libraries(${target} PRIVATE m)
  endif()
  openrac_warnings(${target})
  add_test(NAME ${subject} COMMAND ${target})
endfunction()

# A test written in C++: the program <subject>_test, the ctest test <subject>.
function(openrac_add_test subject)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES;INCLUDES;DEFINITIONS;LIBS")
  if(arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "openrac_add_test(${subject}): unknown arguments ${arg_UNPARSED_ARGUMENTS}")
  endif()
  _openrac_add_test_program(${subject} "tests/${subject}_test.cpp" "${arg_SOURCES}"
                            "${arg_INCLUDES}" "${arg_DEFINITIONS}" "${arg_LIBS}" "" OFF)
endfunction()

# A test written in C, with the game C's options it names: the program <subject>_test, the ctest
# test <subject>.
function(openrac_add_c_test subject)
  cmake_parse_arguments(PARSE_ARGV 1 arg "NO_STRICT_ALIASING;WRAPV;NO_FP_CONTRACT;MATH" ""
                        "SOURCES;INCLUDES;DEFINITIONS;LIBS")
  if(arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
            "openrac_add_c_test(${subject}): unknown arguments ${arg_UNPARSED_ARGUMENTS}")
  endif()
  # In this fixed order, whatever order the call names them in, so equal tests compile alike.
  set(options "")
  if(arg_NO_STRICT_ALIASING)
    list(APPEND options -fno-strict-aliasing)
  endif()
  if(arg_WRAPV)
    list(APPEND options -fwrapv)
  endif()
  if(arg_NO_FP_CONTRACT)
    list(APPEND options -ffp-contract=off)
  endif()
  _openrac_add_test_program(${subject} "tests/${subject}_test.c" "${arg_SOURCES}"
                            "${arg_INCLUDES}" "${arg_DEFINITIONS}" "${arg_LIBS}" "${options}"
                            ${arg_MATH})
endfunction()

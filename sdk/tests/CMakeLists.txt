# SPDX-License-Identifier: MIT
# The C++ layer's test (openrac/mod.hpp): built by any C++17 compiler, from a checkout or from the
# unpacked SDK.
#
#   cmake -S sdk/tests -B build/sdk-tests -G Ninja && cmake --build build/sdk-tests && ctest --test-dir build/sdk-tests
cmake_minimum_required(VERSION 3.20)
project(openrac_mod_sdk_tests LANGUAGES CXX)
enable_testing()

add_executable(mod_hpp_test mod_hpp_test.cpp)
target_include_directories(mod_hpp_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set_target_properties(mod_hpp_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
# Warnings are errors, so a header that warns fails the test: the layer is compiled into every mod.
if(MSVC)
  target_compile_options(mod_hpp_test PRIVATE /W4 /EHsc)
else()
  target_compile_options(mod_hpp_test PRIVATE -Wall -Wextra -Werror)
endif()
add_test(NAME mod_hpp COMMAND mod_hpp_test)

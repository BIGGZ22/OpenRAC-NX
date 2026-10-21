# SPDX-License-Identifier: GPL-3.0-or-later
# The frame-counter example (sdk/README.md):
#   cmake -S sdk/examples/frame-counter -B build/frame-counter -G Ninja
#   cmake --build build/frame-counter
cmake_minimum_required(VERSION 3.20)
project(frame_counter LANGUAGES C)

# Inside OpenRAC the SDK is two folders up; elsewhere -DOpenRACMod_DIR=<OpenRAC>/sdk/cmake.
if(NOT OpenRACMod_DIR)
  set(OpenRACMod_DIR "${CMAKE_CURRENT_LIST_DIR}/../../cmake")
endif()
find_package(OpenRACMod 1.0 REQUIRED)

openrac_add_mod(frame_counter MANIFEST openrac-mod.json SOURCES src/frame_counter.c)

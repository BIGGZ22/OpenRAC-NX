# A mod for OpenRAC's native port: copy this folder, rename the target and the id, write your code.
#
#   cmake -S . -B build -G Ninja -DOpenRACMod_DIR=<OpenRAC>/sdk/cmake
#   cmake --build build
#   python3 <OpenRAC>/sdk/tools/package_mod.py build/package/my_mod    # dist/my-mod-0.1.0-<platform>.zip
cmake_minimum_required(VERSION 3.20)
project(my_mod LANGUAGES C)

# The SDK: -DOpenRACMod_DIR=<OpenRAC>/sdk/cmake, or OPENRAC_SDK=<OpenRAC>/sdk.
if(DEFINED OPENRAC_SDK AND NOT OpenRACMod_DIR)
  set(OpenRACMod_DIR "${OPENRAC_SDK}/cmake")
endif()
find_package(OpenRACMod 1.0 REQUIRED)

openrac_add_mod(my_mod MANIFEST openrac-mod.json SOURCES src/mod.c)

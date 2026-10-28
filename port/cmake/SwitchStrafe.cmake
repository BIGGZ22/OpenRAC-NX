# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(GLOBAL)
include(FetchContent)
# Reuse the project's GPL-3.0 lock-strafe, pinned for reproducible console builds.
FetchContent_Declare(switch_strafe
  GIT_REPOSITORY https://github.com/OpenRAC/rac1-mods.git
  GIT_TAG 2c5139c106de03a7ae2d0ef55a703a1d7c290d09
  SOURCE_SUBDIR openrac-download-only)
FetchContent_MakeAvailable(switch_strafe)
set(strafe_dir "${CMAKE_BINARY_DIR}/generated/switch-strafe")
foreach(file lock_strafe.cpp discs.h strafe.h)
  file(READ "${switch_strafe_SOURCE_DIR}/lock-strafe/src/${file}" text)
  if(file STREQUAL "strafe.h")
    set(original "#define RAC1_STRAFE_BUTTONS (RAC1_PAD_L2 | RAC1_PAD_R2)")
    string(FIND "${text}" "${original}" found)
    if(found EQUAL -1)
      message(FATAL_ERROR "Switch lock-strafe: upstream button definition changed")
    endif()
    string(REPLACE "${original}" "#define RAC1_STRAFE_BUTTONS RAC1_PAD_L2 /* Switch: ZL only; ZR remains retail. */" text "${text}")
  endif()
  if(file STREQUAL "lock_strafe.cpp")
    foreach(anchor "void turn_toward(float yaw," "void update_motion_input(Original original) {" "    original(angle);" "    put_float(move_yaw, yaw);")
      string(FIND "${text}" "${anchor}" found)
      if(found EQUAL -1)
        message(FATAL_ERROR "Switch strafe smoothing: upstream hook changed: ${anchor}")
      endif()
    endforeach()
    string(PREPEND text "#include \"platform/switch/strafe_smoothing.h\"\n")
    string(REPLACE "void turn_toward(float yaw," "#include \"platform/switch/strafe_mod.inc\"\nvoid turn_toward(float yaw," text "${text}")
    string(REPLACE "void update_motion_input(Original original) {" "void update_motion_input(Original original) {\n    switch_strafe_begin_tick();" text "${text}")
    string(REPLACE "    put_float(move_yaw, yaw);" "    put_float(move_yaw, switch_strafe_facing(yaw));" text "${text}")
    string(REPLACE "    original(angle);" "    original(angle);\n    switch_strafe_finish_velocity();" text "${text}")
    string(REPLACE "    s_backpedal = 0;" "    s_backpedal = 0;\n    s_switch_smoothing = SwitchStrafeSmoothing{};" text "${text}")
    string(REPLACE "Hold L2 or R2" "Hold ZL" text "${text}")
    # The SDK's desktop base+address view cannot reach Switch's compact stack.
    # Route all three memory helpers through the runtime's guest translator.
    string(PREPEND text "#ifdef __SWITCH__\n#include \"openrac/guest.h\"\n#endif\n")
    string(REPLACE "return s_mod->memory().ref<T>(address);"
      "#ifdef __SWITCH__\n    return *static_cast<T*>(G(address));\n#else\n    return s_mod->memory().ref<T>(address);\n#endif" text "${text}")
    string(REPLACE "return s_mod->memory().read<float>(at);"
      "#ifdef __SWITCH__\n    float value; std::memcpy(&value, G(at), sizeof(value)); return value;\n#else\n    return s_mod->memory().read<float>(at);\n#endif" text "${text}")
    string(REPLACE "s_mod->memory().write<float>(at, v);"
      "#ifdef __SWITCH__\n    std::memcpy(G(at), &v, sizeof(v));\n#else\n    s_mod->memory().write<float>(at, v);\n#endif" text "${text}")
  endif()
  file(CONFIGURE OUTPUT "${strafe_dir}/${file}" CONTENT "${text}" @ONLY)
endforeach()
add_library(openrac_switch_strafe STATIC "${strafe_dir}/lock_strafe.cpp")
target_include_directories(openrac_switch_strafe PRIVATE "${PROJECT_SOURCE_DIR}/../sdk/include"
  "${PROJECT_SOURCE_DIR}/runtime/include" "${PROJECT_SOURCE_DIR}" "${strafe_dir}")
target_compile_definitions(openrac_switch_strafe PRIVATE
  OPENRAC_MOD_TARGET_MINOR=1
  openrac_mod_load=openrac_switch_strafe_load openrac_mod_query=openrac_switch_strafe_query)
target_compile_options(openrac_switch_strafe PRIVATE -ffp-contract=off)
openrac_add_c_test(switch_strafe INCLUDES "${strafe_dir}" MATH)

# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(GLOBAL)

# Independent native probe only. Do not advertise a complete game renderer.
option(OPENRAC_SWITCH_DEKO3D_PROBE "Prepare a separate native Deko3D hardware-test target" OFF)
if(NOT SWITCH OR NOT OPENRAC_SWITCH_DEKO3D_PROBE)
  return()
endif()
find_library(OPENRAC_DEKO3D_LIBRARY NAMES deko3dd REQUIRED)
find_path(OPENRAC_DEKO3D_INCLUDE_DIR deko3d.h REQUIRED)
if(NOT NX_UAM_EXE)
  message(FATAL_ERROR "Deko3D probe requires devkitPro UAM for native shaders")
endif()
add_library(openrac_deko3d_presenter STATIC renderer/deko3d/presenter.cpp renderer/deko3d/level_scene.cpp)
target_include_directories(openrac_deko3d_presenter PUBLIC "${PROJECT_SOURCE_DIR}"
  PRIVATE "${OPENRAC_DEKO3D_INCLUDE_DIR}")
target_link_libraries(openrac_deko3d_presenter PUBLIC "${OPENRAC_DEKO3D_LIBRARY}")

set(deko_romfs "${CMAKE_CURRENT_BINARY_DIR}/generated/deko3d-probe-romfs")
file(MAKE_DIRECTORY "${deko_romfs}/shaders")
set(deko_shaders)
foreach(program probe mesh scene)
 foreach(stage vert frag)
  set(source "${PROJECT_SOURCE_DIR}/renderer/deko3d/shaders/${program}.${stage}")
  set(output "${deko_romfs}/shaders/${program}.${stage}.dksh")
  add_custom_command(OUTPUT "${output}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${deko_romfs}/shaders"
    COMMAND "${NX_UAM_EXE}" -s "${stage}" -o "${output}" "${source}"
    DEPENDS "${source}" VERBATIM)
  list(APPEND deko_shaders "${output}")
 endforeach()
endforeach()
add_custom_target(openrac_deko3d_probe_shaders DEPENDS ${deko_shaders})
add_executable(openrac-switch-deko3d-probe platform/switch/deko3d_probe.cpp renderer/texture.cpp)
target_link_libraries(openrac-switch-deko3d-probe PRIVATE openrac_deko3d_presenter)
add_dependencies(openrac-switch-deko3d-probe openrac_deko3d_probe_shaders)
set_property(TARGET openrac-switch-deko3d-probe APPEND PROPERTY LINK_DEPENDS ${deko_shaders})
set(nacp "${CMAKE_CURRENT_BINARY_DIR}/openrac-switch-deko3d-probe.nacp")
nx_generate_nacp(OUTPUT "${nacp}" NAME "OpenRAC Deko3D hardware test"
  AUTHOR "OpenRAC contributors" VERSION "${OPENRAC_NX_VERSION}")
nx_create_nro(openrac-switch-deko3d-probe NACP "${nacp}" ROMFS "${deko_romfs}")

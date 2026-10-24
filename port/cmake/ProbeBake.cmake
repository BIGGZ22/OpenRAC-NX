# SPDX-License-Identifier: GPL-3.0-or-later
# Embree belongs to the offline tool only; never link it into the game.
option(OPENRAC_PROBE_BAKER "Build the offline Embree 4 probe baker" OFF)
if(OPENRAC_PROBE_BAKER)
  find_package(embree 4 CONFIG REQUIRED)
  find_package(Threads REQUIRED)
  add_library(openrac_probe_bake STATIC
    enhanced/lighting/bake.cpp enhanced/lighting/bake_integration.cpp
    enhanced/lighting/bake_material.cpp enhanced/lighting/bake_scene.cpp
    enhanced/lighting/bake_transport.cpp)
  target_link_libraries(openrac_probe_bake PUBLIC openrac_lighting PRIVATE embree Threads::Threads)
  openrac_warnings(openrac_probe_bake)
  openrac_add_test(probe_bake SOURCES enhanced/tests/probe_bake_test.cpp LIBS openrac_probe_bake)
  openrac_add_test(probe_visibility SOURCES enhanced/tests/probe_visibility_test.cpp LIBS openrac_probe_bake)
  openrac_add_test(probe_lighting SOURCES enhanced/tests/probe_lighting_test.cpp LIBS openrac_probe_bake)
  openrac_add_test(probe_bake_parallel SOURCES enhanced/tests/probe_bake_parallel_test.cpp
                   LIBS openrac_probe_bake)
  openrac_add_test(probe_integration SOURCES enhanced/tests/probe_integration_test.cpp
                   LIBS openrac_probe_bake)
  if(WIN32)
    # The official binary package keeps Embree and its TBB DLLs together.
    set_tests_properties(probe_bake probe_bake_parallel probe_integration
      probe_visibility probe_lighting PROPERTIES
      ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:embree>")
  endif()
endif()

# SPDX-License-Identifier: GPL-3.0-or-later
# Embree belongs to the offline tool only; never link it into the game.
option(OPENRAC_PROBE_BAKER "Build the offline Embree 4 probe baker" OFF)
if(OPENRAC_PROBE_BAKER)
  find_package(embree 4 CONFIG REQUIRED)
  find_package(Threads REQUIRED)
  add_library(openrac_probe_bake STATIC enhanced/lighting/bake.cpp)
  target_link_libraries(openrac_probe_bake PUBLIC openrac_lighting PRIVATE embree Threads::Threads)
  openrac_warnings(openrac_probe_bake)
  add_executable(probe_bake_test enhanced/tests/probe_bake_test.cpp)
  target_link_libraries(probe_bake_test PRIVATE openrac_probe_bake)
  openrac_warnings(probe_bake_test)
  add_test(NAME probe_bake COMMAND probe_bake_test)
  add_executable(probe_bake_parallel_test enhanced/tests/probe_bake_parallel_test.cpp)
  target_link_libraries(probe_bake_parallel_test PRIVATE openrac_probe_bake)
  openrac_warnings(probe_bake_parallel_test)
  add_test(NAME probe_bake_parallel COMMAND probe_bake_parallel_test)
  if(WIN32)
    # The official binary package keeps Embree and its TBB DLLs together.
    set_tests_properties(probe_bake PROPERTIES
      ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:embree>")
    set_tests_properties(probe_bake_parallel PROPERTIES
      ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:embree>")
  endif()
endif()

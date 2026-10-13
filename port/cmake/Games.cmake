# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# The games, each built from its decompilation's C: tools/hostgen writes the C
# again for the port (game memory, calls by code address), the build compiles
# it with the game's own host code and the library replacements every game
# shares, and links it into the game's program, openrac-<id>.
#
# A game is a directory port/game/<id>/ with a hostgen.json (its id, title,
# serial, decompilation, entry...), a libraries.tsv and, if it needs any, host
# C of its own in host/. Every such game whose decompilation is present is
# built; OPENRAC_<ID>_SOURCE (OPENRAC_RAC1_PAL_SOURCE, ...) points one at
# another copy of its decompilation (your own checkout, further along).
#
# The decompilations are only read: hostgen works on copies in the build tree.

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# hostgen uses every core itself: one game's translation at a time (Ninja).
set_property(GLOBAL APPEND PROPERTY JOB_POOLS openrac_hostgen=1)

set(OPENRAC_HOSTGEN "${CMAKE_CURRENT_LIST_DIR}/../tools/hostgen/hostgen.py")
set(OPENRAC_GAME_DIR "${CMAKE_CURRENT_LIST_DIR}/../game")
set(OPENRAC_REPO_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

# How the games' C is compiled: as the console's C expects (wrapping integers,
# signed char, no type-based aliasing, no fused multiply-add).
set(OPENRAC_GAME_C_FLAGS -fno-strict-aliasing -fwrapv -fsigned-char -ffp-contract=off)

# The library replacements every game shares (port/game/common/lib).
file(GLOB openrac_game_common_sources CONFIGURE_DEPENDS "${OPENRAC_GAME_DIR}/common/lib/*.c"
     "${OPENRAC_GAME_DIR}/common/lib/*.cpp")
add_library(openrac_game_common STATIC ${openrac_game_common_sources})
target_include_directories(openrac_game_common PUBLIC "${OPENRAC_GAME_DIR}/common/include")
target_link_libraries(openrac_game_common PUBLIC openrac_runtime)
# The asset readers the library replacements use (the WAD decompressor).
if(TARGET openrac_assets_disc)
  target_link_libraries(openrac_game_common PUBLIC openrac_assets_disc)
endif()
# The sound engine the sound library's replacement drives (lib/sound_engine.cpp).
target_link_libraries(openrac_game_common PUBLIC openrac_audio)
# The moby animation code the blend snapshot runs (lib/moby_snapshot.cpp).
if(TARGET openrac_assets_geometry)
  target_link_libraries(openrac_game_common PUBLIC openrac_assets_geometry)
endif()
target_compile_options(openrac_game_common PRIVATE ${OPENRAC_GAME_C_FLAGS} -Wall -Wextra)

function(openrac_add_game config)
  file(READ "${config}" json)
  string(JSON id GET "${json}" id)
  string(JSON source GET "${json}" source)
  get_filename_component(config_dir "${config}" DIRECTORY)
  string(TOUPPER "${id}" ID)
  string(REPLACE "-" "_" ID "${ID}")
  string(REPLACE "-" "_" target_id "${id}")
  set(OPENRAC_${ID}_SOURCE "${OPENRAC_REPO_DIR}/${source}" CACHE PATH
      "The decompilation openrac-${id} is built from")
  set(game_source "${OPENRAC_${ID}_SOURCE}")
  set(gen "${CMAKE_CURRENT_BINARY_DIR}/games/${id}/gen")

  if(NOT EXISTS "${game_source}")
    message(STATUS "openrac-${id}: not built, no decompilation at ${game_source}")
    return()
  endif()

  # What hostgen will write, known now so the build can depend on it.
  execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${OPENRAC_HOSTGEN}" --game "${config}"
            --source "${game_source}" --out "${gen}" --list
    OUTPUT_VARIABLE listing
    RESULT_VARIABLE status)
  if(NOT status EQUAL 0)
    message(FATAL_ERROR "hostgen could not list the sources of ${id}")
  endif()
  string(REPLACE "\n" ";" listing "${listing}")
  set(outputs "")
  set(sources "")
  foreach(file IN LISTS listing)
    if(file)
      list(APPEND outputs "${gen}/${file}")
      if(file MATCHES "\\.c$")
        list(APPEND sources "${gen}/${file}")
      endif()
    endif()
  endforeach()

  file(GLOB_RECURSE inputs CONFIGURE_DEPENDS
    "${game_source}/src/*.c" "${game_source}/src/*.h" "${game_source}/include/*.h"
    "${game_source}/candidates/*.c" "${game_source}/nonmatching/*.tsv" "${game_source}/nonmatching/*.c")
  # The decompilation's own tables that hostgen.json names ("places", "symbols").
  string(JSON places ERROR_VARIABLE no_places GET "${json}" places)
  if(NOT no_places AND EXISTS "${game_source}/${places}")
    list(APPEND inputs "${game_source}/${places}")
  endif()
  string(JSON symbol_count ERROR_VARIABLE no_symbols LENGTH "${json}" symbols)
  if(NOT no_symbols AND symbol_count GREATER 0)
    math(EXPR last "${symbol_count} - 1")
    foreach(i RANGE ${last})
      string(JSON symbols GET "${json}" symbols ${i})
      if(EXISTS "${game_source}/${symbols}")
        list(APPEND inputs "${game_source}/${symbols}")
      endif()
    endforeach()
  endif()
  # hostgen's own code (in a function, CMAKE_CURRENT_LIST_DIR is the caller's directory, so the
  # tool's is taken from OPENRAC_HOSTGEN, set where this file was read), and the headers it reads
  # the decompilation with.
  get_filename_component(hostgen_dir "${OPENRAC_HOSTGEN}" DIRECTORY)
  file(GLOB tool CONFIGURE_DEPENDS "${hostgen_dir}/*.py" "${hostgen_dir}/include/*.h")
  file(GLOB tables CONFIGURE_DEPENDS "${config_dir}/*.json" "${config_dir}/*.tsv"
       "${config_dir}/hand/*" "${OPENRAC_GAME_DIR}/common/*.tsv")
  # hostgen rewrites only the files whose text changed, so after an edit only
  # those recompile: the stamp says when it last ran, the files are byproducts
  # whose times the build looks at again.
  add_custom_command(
    OUTPUT "${gen}/hostgen.stamp"
    BYPRODUCTS ${outputs}
    COMMAND "${Python3_EXECUTABLE}" "${OPENRAC_HOSTGEN}" --game "${config}"
            --source "${game_source}" --out "${gen}" --quiet
    COMMAND "${CMAKE_COMMAND}" -E touch "${gen}/hostgen.stamp"
    DEPENDS ${inputs} ${tool} ${tables}
    COMMENT "hostgen: ${id} from ${game_source}"
    JOB_POOL openrac_hostgen
    VERBATIM)
  add_custom_target(openrac_${target_id}_hostgen DEPENDS "${gen}/hostgen.stamp")

  # The game: generated C, and the game's own host C if it has any.
  file(GLOB host CONFIGURE_DEPENDS "${config_dir}/host/*.c")
  add_library(openrac_${target_id}_game STATIC ${sources} ${host})
  target_include_directories(openrac_${target_id}_game PUBLIC "${gen}")
  target_link_libraries(openrac_${target_id}_game PUBLIC openrac_game_common)
  if(UNIX)
    target_link_libraries(openrac_${target_id}_game PUBLIC m)  # host maths the games call (fabsf...)
  endif()
  target_compile_options(openrac_${target_id}_game PRIVATE ${OPENRAC_GAME_C_FLAGS})
  add_dependencies(openrac_${target_id}_game openrac_${target_id}_hostgen)
  # The decompilation's C is read as it is: its warnings are its own.
  set_source_files_properties(${sources} PROPERTIES COMPILE_OPTIONS "-w")
  if(host)
    set_source_files_properties(${host} PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
  endif()

  add_executable(openrac-${id} "${OPENRAC_GAME_DIR}/common/main.cpp")
  target_link_libraries(openrac-${id} PRIVATE openrac_${target_id}_game openrac_runtime)
  # The window: the renderer draws what the game holds in memory (game/common/frontend.h).
  if(TARGET openrac_viewer_lib AND TARGET openrac_platform)
    target_sources(openrac-${id} PRIVATE "${OPENRAC_GAME_DIR}/common/frontend.cpp"
                                         "${OPENRAC_GAME_DIR}/common/debug_menu.cpp"
                                         "${OPENRAC_GAME_DIR}/common/options_menu.cpp"
                                         "${OPENRAC_GAME_DIR}/common/settings.cpp")
    target_link_libraries(openrac-${id} PRIVATE openrac_viewer_lib openrac_platform)
    target_compile_definitions(openrac-${id} PRIVATE OPENRAC_FRONTEND=1)
    # The window's icon: OpenRAC's own, the launcher's, embedded as PNG bytes.
    openrac_window_icon(icon_c)
    target_sources(openrac-${id} PRIVATE "${icon_c}")
    if(TARGET openrac_media)
      target_link_libraries(openrac-${id} PRIVATE openrac_media)
      target_compile_definitions(openrac-${id} PRIVATE OPENRAC_MOVIES=1)
    endif()
  endif()
  openrac_warnings(openrac-${id})
endfunction()

# The launcher's 128x128 icon as a C array (openrac_icon_png, openrac_icon_png_size), written once
# into the build folder and again when the picture changes.
function(openrac_window_icon out)
  set(png "${PROJECT_SOURCE_DIR}/../launcher/src-tauri/icons/128x128.png")
  set(c "${CMAKE_BINARY_DIR}/generated/openrac_icon.c")
  set_property(DIRECTORY "${PROJECT_SOURCE_DIR}" APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${png}")
  file(READ "${png}" hex HEX)
  string(LENGTH "${hex}" n)
  math(EXPR size "${n} / 2")
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
  file(CONFIGURE OUTPUT "${c}" CONTENT "/* Generated from launcher/src-tauri/icons/128x128.png by cmake/Games.cmake. */
const unsigned char openrac_icon_png[] = {${bytes}};
const unsigned int openrac_icon_png_size = ${size}u;
")
  set(${out} "${c}" PARENT_SCOPE)
endfunction()

# Every game in port/game/ (the directories with a hostgen.json).
function(openrac_add_games)
  file(GLOB configs CONFIGURE_DEPENDS "${OPENRAC_GAME_DIR}/*/hostgen.json")
  foreach(config IN LISTS configs)
    openrac_add_game("${config}")
  endforeach()
endfunction()

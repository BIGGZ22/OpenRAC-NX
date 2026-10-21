/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/* A game with no binding for script mods (openrac/script_binding.h): its programs refuse them. A
 * game gets one with sdk/bindings/<game>.json and sdk/tools/gen_binding.py, which writes
 * port/game/<game>/script_binding.c in its place (cmake/Games.cmake picks one or the other). */
#include <stddef.h>

#include "openrac/script_binding.h"

const openrac_script_binding* openrac_game_script_binding(void) {
    return NULL;
}

/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The port's rows in the game's options (port/game/common/options_menu.cpp): their texts. */
#include "rac1.h"

/* The text of an id (func_001FE540, matched C in the decompilation: the language's table, or the
 * empty string for an id it does not have). The port's own ids, which the table never has, are
 * answered with the port's strings (libraries.tsv, port = wrap). */
RAC1_TYPE_func_001FE540_result func_001FE540(int id) {
    const gaddr own = openrac_game_text(id);
    return own != 0 ? (RAC1_TYPE_func_001FE540_result)own : func_001FE540__game(id);
}

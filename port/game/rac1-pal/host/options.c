/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The port's rows in the game's options (port/game/common/options_menu.cpp): their texts. */
#include "game_protos.h"
#include "openrac/game_host.h"

/* The text of an id (func_001FE540, matched C in the decompilation: the language's table, or the
 * empty string for an id it does not have). The port's own ids, which the table never has, are
 * answered with the port's strings (libraries.tsv, port = wrap). */
gaddr func_001FE540(int id) {
    const gaddr own = openrac_game_text(id);
    return own != 0 ? own : func_001FE540__game(id);
}

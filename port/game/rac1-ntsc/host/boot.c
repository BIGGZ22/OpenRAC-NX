/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Around the game's own start-up code (libraries.tsv, port = wrap): the
 * NTSC-U counterpart of rac1-pal's host/boot.c, with that file's addresses
 * carried through the version map (README.md). */
#include "game_protos.h"

#include <stdlib.h>
#include "openrac/game_host.h"

/* The level being played, or -1: NTSC-U 0x15ED84 (PAL 0x15EE84). */
#define CURRENT_LEVEL 0x0015ED84u
#define LEVELS 19

static int loaded_level = OPENRAC_OVERLAY_EXE;

/* parse_bin (0x12D8F8): copies the next level's program over the
 * executable's main segment and returns its entry, which main
 * (run_game_main_loop, 0x12D9D8) calls next. From then on a code address in
 * that range means the loaded level's function (the overlay the runtime
 * looks in). */
long long game_FUN_0012d8f8(void) {
    static int relocation_set;
    /* Before the first level is copied in: the executable's code as the
     * pattern its level copies are paired against (guest.h, per-level
     * relocation). The executable's code and data are 0x100000-0x240000;
     * what a level's program moves is its data from 0x15EF00 (resident
     * below: every NTSC-U section from core.lit on sits 0x100 under PAL's)
     * up to the end of the program; $gp is 0x166C00 in every program. */
    if (!relocation_set) {
        relocation_set = 1;
        openrac_guest_set_relocation(0x0015EF00u, 0x00250000u, 0x00166C00u,
                                     (const uint8_t *)G(0x00100000u), 0x00100000u, 0x00140000u);
    }
    const long long entry = game_FUN_0012d8f8__game();
    loaded_level = GREF(int, CURRENT_LEVEL);
    return entry;
}

/* hostgen.json: "overlay_hook": true. */
int openrac_game_loaded_overlay(void) {
    return loaded_level >= 0 && loaded_level < LEVELS ? loaded_level : OPENRAC_OVERLAY_EXE;
}

/* SetupGifPaging (PAL func_001F4630), every frame before the texture paging. The four words at
 * 0x18A2D0 are flags the draw code tests (shrubs at +0, mobys at +8, the effect-texture uploads at
 * +0xC: without that one the font and the title's logo are never uploaded). Retail has all four
 * set to 1 by its first frame; no code in the game stores them by name, so they are set here, once
 * for each program (the executable, then each level, whose copies are elsewhere: OPENRAC_DATA). */
void game_FUN_001f4280(int arg0) {
    static int set_for = -2;
    if (set_for != loaded_level) {
        set_for = loaded_level;
        for (unsigned i = 0; i < 4; ++i) {
            GREF(int, OPENRAC_DATA(0x0018A2D0u + i * 4)) = 1;
        }
    }
    game_FUN_001f4280__game(arg0);
}

/* NewGameInit (PAL func_00209DC0; the two versions' code differs here, so the pairing is by
 * place, not by identical instructions): a new game's state is a fresh save restored from the
 * disc, with the level set to 0 (Veldin); the level's files are read right after. With --level N
 * the new game starts in level N instead. */
void game_FUN_00209370(void) {
    game_FUN_00209370__game();
    if (openrac_game_start_level >= 0 && openrac_game_start_level < LEVELS) {
        GREF(int, CURRENT_LEVEL) = openrac_game_start_level;
    }
}

/*
 * OPENRAC_DIRECT with --level N: straight into the level from the title, as the save page's
 * "continue without saving" starts a new game (NewGameInit, then the menus closed and the start
 * flag at 0x13E05A set), so a test skips the title's wait and the menus.
 */
static void direct_start(unsigned frame) {
    static int done = 0;
    static unsigned title_from = 0;
    (void)frame;
    if (done) {
        return;
    }
    if (openrac_game_start_level < 0 || getenv("OPENRAC_DIRECT") == NULL) {
        done = 1;
        return;
    }
    /* The title world is up once the boot program's camera has a forward row (0x187290). */
    if (GREF(float, 0x00187290u) == 0.0f && GREF(float, 0x00187294u) == 0.0f) {
        return;
    }
    if (title_from == 0) {
        title_from = frame;
    }
    if (frame < title_from + 60) {
        return;
    }
    done = 1;
    game_FUN_00209370();
    game_InitializeGlobalStateEntry(0); /* PAL func_0022F4A0: closes the menus */
    GREF(short, 0x0013E05Au) = 1;
}

/* The player's bolts (PAL 0x15EE98). */
static int bolts(void) {
    return GREF(int, 0x0015ED98u);
}

__attribute__((constructor)) static void install_direct_start(void) {
    openrac_game_on_frame = direct_start;
    openrac_game_bolts = bolts;
}

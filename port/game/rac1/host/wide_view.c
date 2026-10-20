/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: the view tests of the draw callbacks in a picture wider than the game's.
 *
 * Seas, lava and the sand sea (Aridia, Kerwan, Rilgar, Umbris, Batalia, Gaspar, Oltanis: the
 * liquid grid; Novalis's lava and other overlay layers) are drawn by level code, most of it run
 * from the game's four draw callback lists (func_001F4A00, func_001F4BB8, func_001F4A78,
 * func_001F4AF0, each list's runner; callback_lists.c). Each decides what to send from the game's
 * own 4:3 view, read from two places the frame's setup writes:
 *
 * - the camera block (0x187040, func_001F2608): +0x100 a world-to-clip matrix that culls, whose
 *   first terms of each column give x. The liquid grid's cull (func_L02_00250720) tests each 7 x 7
 *   cell block's corners against it: a block whose four corners all have x > |w|, or all x < -|w|,
 *   is not drawn;
 * - the view block (0x18CE00, func_001F3140): its copies of the tangent of half the horizontal
 *   field of view (+0x2C0) and of its secant (+0x2C8, +0x2E0), the clip constants. The bounding
 *   sphere test (func_L00_00200290, FastBSphereCheck) culls a sphere when |x| - sec * r > tan * z;
 *   the overlay layers and other callbacks test with it.
 *
 * The port's picture shows the game's 4:3 view in its middle and opens the horizontal tangent by
 * as much as the picture is wider (frontend.cpp, openrac_game_picture_wider), so what these tests
 * cull at the 4:3 sides is in the picture: tiles near its sides were missing. Here the tests run
 * against values opened by the same amount: the matrix's x terms divided by it, the tangent
 * multiplied by it and the secants made the new tangent's. They are opened for each list's run
 * (for whatever its callbacks test), and again at the two tests themselves when a callback set the
 * view up anew in between (the liquid grid's callback sets its fog first, which builds the
 * matrices again); each time they are put back right after, those still as they were set.
 *
 * The liquid grid's cull only ever decides what is drawn, wherever it is called from. The sphere
 * test also serves the game's own logic (an explosion shakes the camera only when it is in view),
 * so it is opened only while a draw callback list runs, and in the updates whose test only decides
 * what they draw: everything else, the game's logic among it, sees the 4:3 view. At 4:3 nothing
 * changes.
 *
 * Those updates: the nanotech cluster's (class 806, the orb on a nanotech crate, NanotechBallUpdate,
 * func_L00_002D9EA0). Each tick it tests its centre with the sphere test (radius in the sphere's w,
 * draw distance 64) and, only when in view, registers its draw callback (func_L00_002DACC0, the
 * sphere, halo and glass sheen) and spawns its eight glints. Tested against the 4:3 tangent, a crate
 * between the 4:3 edge and the picture's lost its orb, although the crate itself is drawn and
 * "drawn" (moby_view.h). Its state changes (homing in on the hero, the pickup) do not depend on
 * the test.
 *
 * Both are game fixes the player can switch back to the console's 4:3 view (differences.json;
 * openrac_game_fix, game_host.h): fix_wide_liquids for the lists' runs and the liquid grid's cull,
 * fix_wide_nanotech_orbs for the cluster's update. Off, they run as the game wrote them.
 *
 * Not opened: the callbacks' tests that project a point to the screen and compare it with the
 * screen's pixels. The rest of the world is the port's own: it draws every terrain part, tie and
 * shrub and every live moby from the extracted level, whatever the game culled.
 */
#include "rac1.h"
#include "wide_view.h"

#include <stdint.h>
#include <string.h>

#define CAMERA RAC1_ADDR(0x00187040u)
#define VIEW RAC1_ADDR(0x0018CE00u)

enum {
    CULL_MATRIX = 0x100,  /* camera block: the cull matrix */
    CLIP_TAN_X = 0x2C0,   /* view block: the clip constants' tangent ... */
    CLIP_SEC_X = 0x2C8,   /* ... its secant ... */
    CLIP_SCALE_X = 0x2E0, /* ... and the secant again (the sphere test's) */
    SLOTS = 4
};

/* Words opened for a test or a list's run: where, what they were, what they were set to. */
typedef struct {
    gaddr at[SLOTS];
    uint32_t was[SLOTS];
    uint32_t set[SLOTS];
    int count;
} opened;

static float get(gaddr at) {
    float f;
    memcpy(&f, G(at), 4);
    return f;
}

static void put(opened* o, gaddr at, float value) {
    uint32_t bits;
    memcpy(&bits, &value, 4);
    o->at[o->count] = at;
    o->was[o->count] = GREF(uint32_t, at);
    o->set[o->count] = bits;
    GREF(uint32_t, at) = bits;
    o->count++;
}

/* Back as they were, those still as they were set (a test that set the view up again keeps it). */
static void close_view(const opened* o) {
    for (int i = o->count - 1; i >= 0; i--) {
        if (GREF(uint32_t, o->at[i]) == o->set[i]) {
            GREF(uint32_t, o->at[i]) = o->was[i];
        }
    }
}

/* The cull matrix's x terms, divided by `wider`. */
static void open_matrix(opened* o, float wider) {
    const gaddr matrix = OPENRAC_DATA(CAMERA) + CULL_MATRIX;
    o->count = 0;
    for (int column = 0; column < 4; column++) {
        const gaddr at = matrix + (gaddr)column * 16u;
        put(o, at, get(at) / wider);
    }
}

/* The clip constants' horizontal tangent, times `wider`, and its secants. */
static void open_clip(opened* o, float wider) {
    const gaddr view = OPENRAC_DATA(VIEW);
    float tan_x, sec_x;
    wide_clip(get(view + CLIP_TAN_X), wider, &tan_x, &sec_x);
    o->count = 0;
    put(o, view + CLIP_TAN_X, tan_x);
    put(o, view + CLIP_SEC_X, sec_x);
    put(o, view + CLIP_SCALE_X, sec_x);
}

/* Whether every word `o` opened still holds what it was set to. */
static int still_open(const opened* o) {
    for (int i = 0; i < o->count; i++) {
        if (GREF(uint32_t, o->at[i]) != o->set[i]) {
            return 0;
        }
    }
    return o->count > 0;
}

/* How many draw callback lists are running (a callback could run a list); how much wider than 4:3
 * the picture is while they do (asked once a run, not at every test); the words opened for the
 * whole run. */
static int g_lists;
static float g_wider = 1.0f;
static opened g_list_matrix, g_list_clip;

/* How many updates whose sphere test only decides what they draw are running, and how much wider
 * the picture is while they do. */
static int g_effects;
static float g_effect_wider = 1.0f;

/* How much wider than 4:3 the picture is for the game fix `key`: 1 (the game's own view) while
 * the player has it off. `*flag` is its flag, asked for the first time. */
static float wider_for(const unsigned char** flag, const char* key) {
    if (!*flag) {
        *flag = openrac_game_fix(key);
    }
    return **flag ? openrac_game_picture_wider() : 1.0f;
}

static const unsigned char* g_fix_liquids;
static const unsigned char* g_fix_orbs;

static void run_list(void (*run)(void)) {
    if (g_lists > 0) {
        g_lists++;
        run();
        g_lists--;
        return;
    }
    g_wider = wider_for(&g_fix_liquids, "fix_wide_liquids");
    if (!(g_wider > 1.0f)) {
        run();
        return;
    }
    open_matrix(&g_list_matrix, g_wider);
    open_clip(&g_list_clip, g_wider);
    g_lists = 1;
    run();
    g_lists = 0;
    close_view(&g_list_clip);
    close_view(&g_list_matrix);
    g_list_matrix.count = g_list_clip.count = 0;
}

/* ExecuteDrawCallbacks: list 1. */
void func_001F4A00(void) {
    run_list(func_001F4A00__game);
}

/* ExecuteDrawCallbacks2: list 2. */
void func_001F4BB8(void) {
    run_list(func_001F4BB8__game);
}

/* ExecuteDrawCallbacks3: list 3. */
void func_001F4A78(void) {
    run_list(func_001F4A78__game);
}

/* ExecuteDrawCallbacks4: list 4. */
void func_001F4AF0(void) {
    run_list(func_001F4AF0__game);
}

/* The liquid grid's cull: a flag per block into the scratchpad (0: not drawn). */
void func_L02_00250720(gaddr grid) {
    const float wider = g_lists > 0 ? g_wider : wider_for(&g_fix_liquids, "fix_wide_liquids");
    if (!(wider > 1.0f) || (g_lists > 0 && still_open(&g_list_matrix))) {
        func_L02_00250720__game(grid);
        return;
    }
    opened o;
    open_matrix(&o, wider);
    func_L02_00250720__game(grid);
    close_view(&o);
}

/* FastBSphereCheck: 1 in view, 0 in view but across the guard band's edge, -1 culled. */
int func_L00_00200290(gaddr sphere, float far) {
    const float wider = wide_sphere_wider(g_lists, g_wider, g_effects, g_effect_wider);
    if (!(wider > 1.0f) || (g_lists > 0 && still_open(&g_list_clip))) {
        return func_L00_00200290__game(sphere, far);
    }
    opened o;
    open_clip(&o, wider);
    const int result = func_L00_00200290__game(sphere, far);
    close_view(&o);
    return result;
}

/* NanotechBallUpdate: the nanotech cluster (the orb on a nanotech crate) draws only while its
 * sphere test finds it in view; it is run against the picture's sides. */
void func_L00_002D9EA0(gaddr moby) {
    if (g_effects++ == 0) {
        g_effect_wider = wider_for(&g_fix_orbs, "fix_wide_nanotech_orbs");
    }
    func_L00_002D9EA0__game(moby);
    g_effects--;
}

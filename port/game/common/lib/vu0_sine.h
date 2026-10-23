/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * A game's VU0 scalar sine and cosine, for the games whose host C replaces
 * them (rac2-ntsc/host/trig.c, rac4-ntsc/host/trig.c): each describes where
 * its microprogram lies and binds the two entry points to two-line functions
 * that call these.
 *
 * The game's routines move their float argument into VU0 and call a
 * microprogram that a VIF MPG packet of the game loaded. Its cosine entry
 * adds pi/2 and runs on into the sine entry, which folds the angle into
 * [-pi/2, pi/2] with pi - x and -pi - x, then sums the odd series
 * x + c3 x^3 + c5 x^5 + c7 x^7 + c9 x^9. The constants are the immediates of
 * the program's LOI pairs, read from the packet the game itself loaded; the
 * sum is the port's own order of operations, one per game, not a matching
 * claim. */
#ifndef OPENRAC_VU0_SINE_H
#define OPENRAC_VU0_SINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The order in which a game's port sums the series. The two give different
 * roundings, so each game keeps the one its port has always used. */
typedef enum openrac_vu0_sine_order {
    /* x + x x^2 (c3 + x^2 (c5 + x^2 (c7 + x^2 c9))): nested in x^2. */
    OPENRAC_VU0_SINE_NESTED,
    /* (x + c3 x^3) + x^4 ((c5 x + c7 x^3) + c9 x^5): two halves joined by x^4. */
    OPENRAC_VU0_SINE_HALVES,
} openrac_vu0_sine_order;

/* Where a game's sine microprogram lies, and how its port sums the series.
 * An entry is a microprogram address; the constant of an entry is the
 * immediate of the LOI pair there, the pair's lower word. */
typedef struct openrac_vu0_sine_program {
    uint32_t code;                /* Game address of the packet's first instruction pair. */
    uint32_t load;                /* Microprogram address that pair is loaded at. */
    uint32_t cosine_entry;        /* The cosine's entry, whose constant is pi/2. */
    uint32_t sine_entry;          /* The sine's entry, whose constant is pi. */
    uint32_t minus_pi;            /* The entry whose constant is -pi; 0 to negate pi. */
    uint32_t series[4];           /* The entries whose constants are c3, c5, c7, c9. */
    openrac_vu0_sine_order order; /* How the port sums the series. */
} openrac_vu0_sine_program;

/* Returns the sine of `angle` (radians) as `program` computes it. */
float openrac_vu0_sine(const openrac_vu0_sine_program* program, float angle);

/* Returns the cosine of `angle` (radians): the sine of `angle` plus the
 * cosine entry's pi/2. */
float openrac_vu0_cosine(const openrac_vu0_sine_program* program, float angle);

#ifdef __cplusplus
}
#endif

#endif

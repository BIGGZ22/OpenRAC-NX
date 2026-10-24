#ifndef VECTOR_H
#define VECTOR_H

/*
 * The game's two vectors: Vec4f, four floats with 4-byte alignment, and
 * Vec4, the 16-byte vector it moves with lq/sq (one quadword, four floats
 * or four words). One definition for every source file, in place of the
 * private copies candidates carried to land without clashing with their
 * files.
 *
 * Adapted from Lombyte (GPL-3.0), the US decompilation:
 * include/rnc/math/vector.h at 42adb529. See THIRD_PARTY_NOTICES.md.
 */

#include "common.h"

/* A float vector with 4-byte alignment (no 128-bit member). */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f;

/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad __attribute__((mode(TI), aligned(16)));

/* Four floats aligned to 16 bytes, the layout moved by one lq/sq. */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} __attribute__((aligned(16))) Vec4A;

typedef union {
    VecQuad q;
    f32 f[4];
    s32 i[4];
} Vec4;

#endif /* VECTOR_H */

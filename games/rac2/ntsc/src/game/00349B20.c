#include "include_asm.h"

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

/* This unit is built with -ffast-math because the retail clamps the morph progress with the
   R5900's min.s and max.s (0x46020829 and 0x46020828). The approved cc1 only folds a float
   min/max into those instructions under -ffast-math: gcc/config/mips guards the fold with
   flag_fast_math, and the EE target's float format is IEEE. Measured with the approved
   compiler, -funsafe-math-optimizations, -fno-math-errno, -fassociative-math and
   -fno-signed-zeros all still emit c.lt.s. The retail's ordinary 32-bit stores in the parent
   unit are folded, so the flag is specific to the 128-bit and float-clamp shapes. */

extern float func_0034EE78(float, float, float, float, float);

extern s32 func_00283D38(s32, s32, f32);

typedef struct { float x, y, z, w; } MorphVec;

typedef struct {
    float f0;
    float f4;
    float f8;
    float fC;
    float f10;
    int i14;
    float f18;
    int i1C;
    int *p20;
    float *dst[2];
    unsigned char pad2C[0x10];
    MorphVec from[2];
    unsigned char pad5C[0x10];
    MorphVec to[2];
    s32 p8C;
    s32 p90;
} MorphObject;

/* Advances the morph progress by the step, clamps it to its range, lerps the two
   position pairs into the destination vectors and updates the second morph. */
void func_00349B20(u8 *a0)
{
    MorphObject *s = (MorphObject *)a0;
    float t, k;
    int i;

    if (s->i1C != 0) {
        if (s->i14 == 1) {
            float v = s->f10 + s->f18;
            s->f10 = v;
            s->i1C = (v <= 1.0f) ? 1 : 0;
            s->f10 = (v <= 1.0f) ? v : 1.0f;
        } else {
            float v = s->f10 - s->f18;
            s->f10 = v;
            s->i1C = (v >= 0.0f) ? 1 : 0;
            s->f10 = (v >= 0.0f) ? v : 0.0f;
        }
        t = func_0034EE78(s->f10, 0.0f, s->f0, s->f4, 1.0f);
        for (i = 0; i < 2; i++) {
            if (s->dst[i] != 0) {
                k = 1.0f - t;
                s->dst[i][0] = k * s->from[i].x + t * s->to[i].x;
                s->dst[i][1] = k * s->from[i].y + t * s->to[i].y;
                s->dst[i][2] = k * s->from[i].z + t * s->to[i].z;
                s->dst[i][3] = k * s->from[i].w + t * s->to[i].w;
            }
        }
        if (s->p20 != 0) {
            s->p20[0] = func_00283D38(s->p8C, s->p90, func_0034EE78(s->f10, 0.0f, s->f8, s->fC, 1.0f));
        }
    }
}

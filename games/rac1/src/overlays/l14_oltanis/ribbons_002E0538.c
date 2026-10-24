/* Level 14 code: func_L14_002E0538 in a file of its own, built with -fno-gcse
 * (config/file_cflags.txt). The functions of vendor_002E0538.c do not build
 * with it (18 of 36 change), so it was another object in retail
 * (docs/BUILD_FIDELITY.md, rule 2). */
#include "common.h"
#include "records.h"
#include "vector.h"
#include "hero.h"
#include "moby.h"

extern float func_001FA748(float, float);
extern float func_001FA790(float, float);
extern float func_001FA888(int);
extern float func_L00_0025F368(float);
extern float func_001F9FA8(float);
extern float func_L00_00258C80(float, float);
extern float func_001F9F90(float x);
struct Moby;
extern void func_001F49B0(void (*)(struct Moby *), struct Moby *);
void func_L14_002E0858(char *m);

typedef struct RibbonPoint {
    f32 x;                              /* 0x00 */
    union { f32 f; s32 i; } aux;        /* 0x04: zeroed as an int in the ring; the sweep's cosine */
    union { f32 f; s32 i; } height;     /* 0x08: ring height; the sweep's height is zeroed as an int first */
    f32 one;                            /* 0x0C */
} RibbonPoint;

typedef struct RibbonVars {
    RibbonPoint pts[29];                /* 0x000: ring points 0..9; sweep points 10..28 (their fields sit at +0xA0) */
    u8 pad_1D0[0x10];
    f32 ripple_phase;                   /* 0x1E0 */
    f32 sweep_phase_a;                  /* 0x1E4 */
    f32 sweep_phase_b;                  /* 0x1E8 */
    u8 pad_1EC[0x4];
    f32 ring_step;                      /* 0x1F0 */
} RibbonVars;

// Per-frame update for the level 14 moby: refreshes its ring table and hands the table to func_001F49B0.
void animate_ribbon_effect_paths(char *moby)
{
    RibbonVars *d = (RibbonVars *)((struct Moby *)moby)->pvars;
    RibbonPoint *o;
    int i;
    int k;
    int j;
    int n;
    float c1;
    float c16;
    float one;
    float step;
    float f20;
    float f21;
    float c16b;
    float one2;
    float k99;
    float k20;
    float k75;
    float k50;
    float c39;
    float one3;

    c1 = 0.1f;
    c16 = 1.6f;
    one = 1.0f;
    d->ripple_phase = FastAddRots(d->ripple_phase, 0.17453292f);
    d->sweep_phase_a = FastSubRots(d->sweep_phase_a, 0.80285144f);
    d->sweep_phase_b = FastAddRots(d->sweep_phase_b, 0.05236f);
    step = 360.0f / (IntToFloat(10) * 0.5f) * 0.017453292f;

    n = 5;
    o = d->pts;
    for (i = 0; i < 5; i++) {
        IntToFloat(i);
        IntToFloat(10);
        if (i == 0) {
            f20 = 0.0f;
            f21 = f20;
        } else {
            float t = (float)i;
            f21 = FastSin(normalize_angle(d->ripple_phase + t * step)) * c1;
            f21 = f21 + randf_sym(0.0f, 0.2f);
            f20 = t;
        }
        o->height.f = f21;
        o->aux.i = 0;
        o->x = d->ring_step * f20 + c16;
        o->one = one;
        o++;
    }

    c16b = 1.6f;
    one2 = 1.0f;
    k = 5;
    j = n - 1;
    for (; k < 10; k++) {
        d->pts[k].height.f = d->pts[j].height.f;
        d->pts[k].aux.i = 0;
        d->pts[k].x = d->ring_step * (float)k + c16b;
        d->pts[k].one = one2;
        j--;
    }

    k99 = 0.99483764f;
    k20 = 0.20944f;
    k75 = 0.75f;
    k50 = 0.5f;
    c39 = 0.39f;
    one3 = 1.0f;
    i = 0;
    o = d->pts;
    for (; i < 19; i++) {
        float fi = (float)i;
        float x, b, c, s;
        o[10].height.i = 0;
        x = normalize_angle(d->sweep_phase_a + fi * k99);
        b = normalize_angle(d->sweep_phase_b + fi * k20);
        c = FastSin(b) * k75;
        s = FastSin(x);
        o[10].height.f = o[10].height.f + (c * s + randf_sym(0.0f, 0.2f));
        o[10].aux.f = FastCos(x) * k50;
        o[10].one = one3;
        o[10].x = fi * c39;
        o++;
    }

    AddDrawCallback((void (*)(struct Moby *))draw_ribbon_effect, (struct Moby *)moby);
}

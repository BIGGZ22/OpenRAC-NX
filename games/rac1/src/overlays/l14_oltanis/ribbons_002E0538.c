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
void ScrollingRibbonsDraw(char *m);

// Per-frame update for the level 14 moby: refreshes its ring table and hands the table to func_001F49B0.
void func_L14_002E0538(char *moby)
{
    char *d = *(char **)(moby + 0x78);
    char *o;
    char *p;
    int i;
    int k;
    int j;
    int n;
    float c1;
    float c16;
    float one;
    float f20;
    float f21;
    float f22;
    float a;
    float b;
    float c;
    float e;
    float g;
    float one2;
    float c16b;
    float one3;
    float c39;
    float k99;
    float k20;
    float k75;
    float k50;

    c1 = 0.1f;
    c16 = 1.6f;
    one = 1.0f;
    *(float *)(d + 0x1E0) = func_001FA748(*(float *)(d + 0x1E0), 0.17453292f);
    *(float *)(d + 0x1E4) = func_001FA790(*(float *)(d + 0x1E4), 0.80285144f);
    *(float *)(d + 0x1E8) = func_001FA748(*(float *)(d + 0x1E8), 0.05236f);
    f22 = 360.0f / (func_001FA888(10) * 0.5f) * 0.017453292f;

    n = 5;
    o = d;
    for (i = 0; i < 5; i++) {
        func_001FA888(i);
        func_001FA888(10);
        if (i == 0) {
            f20 = 0.0f;
            f21 = f20;
        } else {
            float t = (float)i;
            f21 = func_001F9FA8(func_L00_0025F368(*(float *)(d + 0x1E0) + t * f22)) * c1;
            f21 = f21 + func_L00_00258C80(0.0f, 0.2f);
            f20 = t;
        }
        *(float *)(o + 0x8) = f21;
        *(int *)(o + 0x4) = 0;
        *(float *)o = *(float *)(d + 0x1F0) * f20 + c16;
        *(float *)(o + 0xC) = one;
        o += 0x10;
    }

    c16b = 1.6f;
    one2 = 1.0f;
    k = 5;
    j = n - 1;
    for (; k < 10; k++) {
        *(float *)(d + k * 16 + 0x8) = *(float *)(d + (j * 16 + 8));
        *(int *)(d + k * 16 + 0x4) = 0;
        *(float *)(d + k * 16) = *(float *)(d + 0x1F0) * (float)k + c16b;
        *(float *)(d + k * 16 + 0xC) = one2;
        j--;
    }

    k99 = 0.99483764f;
    k20 = 0.20944f;
    k75 = 0.75f;
    k50 = 0.5f;
    c39 = 0.39f;
    one3 = 1.0f;
    i = 0;
    o = d;
    for (; i < 19; i++) {
        float fi = (float)i;
        float x, c, s;
        *(int *)(o + 0xA8) = 0;
        x = func_L00_0025F368(*(float *)(d + 0x1E4) + fi * k99);
        b = func_L00_0025F368(*(float *)(d + 0x1E8) + fi * k20);
        c = func_001F9FA8(b) * k75;
        s = func_001F9FA8(x);
        *(float *)(o + 0xA8) = *(float *)(o + 0xA8) + (c * s + func_L00_00258C80(0.0f, 0.2f));
        *(float *)(o + 0xA4) = func_001F9F90(x) * k50;
        *(float *)(o + 0xAC) = one3;
        *(float *)(o + 0xA0) = fi * c39;
        o += 0x10;
    }

    func_001F49B0((void (*)(struct Moby *))ScrollingRibbonsDraw, (struct Moby *)moby);
}

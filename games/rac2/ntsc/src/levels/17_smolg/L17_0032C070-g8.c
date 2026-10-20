#include "common.h"

/* This unit is built with -G8: every body here reaches a global through $gp. */

extern float D_L17_001B032C __attribute__((sda));

extern float D_L17_001B0330 __attribute__((sda));

extern float D_L17_001B0334 __attribute__((sda));

extern float D_L17_001B0338 __attribute__((sda));

extern float D_L17_001B033C __attribute__((sda));

extern float D_L17_001B0360 __attribute__((sda));

extern float D_00189EA0[];

extern void func_002AEE70(char *, char *, char *, float, float, float);

extern void func_002AEF28(char *, char *, char *, float, float, float);

extern void func_00282CF0(float *, float *, float *);

extern void func_002836E8(float *, float *);

extern void func_00283098(float *, float *, float *);

extern float func_00282C48(float);

/* Re-aims one record's two angle pairs from the level's shared angle words: the 0x118/0x11C
   pair takes 0.000290888245f (arcseconds to radians) times the shared 0x1B0330 word, with the
   record's own 0x12C float as the third argument in the type-3 arm and -1.0f otherwise; the
   0x120/0x124 pair then takes the other two scaled words. */
void func_L17_003C7018(char *arg0)
{
    char *state = *(char **)(arg0 + 0x68);

    if (*(u8 *)(arg0 + 0x20) == 3) {
        func_002AEE70(arg0, state + 0x11C, state + 0x118, D_L17_001B032C,
                      D_L17_001B0330 * 0.000290888245f,
                      *(f32 *)(state + 0x12C));
    } else {
        func_002AEE70(arg0, state + 0x11C, state + 0x118, D_L17_001B032C,
                      D_L17_001B0330 * 0.000290888245f, -1.0f);
    }
    func_002AEF28(arg0, state + 0x120, state + 0x124, D_L17_001B0334,
                  D_L17_001B0338 * 0.000290888245f,
                  D_L17_001B033C * 0.000290888245f);
}

/* Transforms the level's fixed point through the object's own matrix, then through the
   object's second matrix, and keeps the result when both coordinates stay inside the
   shared bound and the object's own height is within two of the fixed point's. */
int func_L17_003C70E0(char *a0)
{
    float v[4];
    float m[12];

    func_00282CF0(v, D_00189EA0, a0 + 0x10);
    func_002836E8(m, a0 + 0xC0);
    func_00283098(v, v, m);
    if (!(func_00282C48(v[0]) <= D_L17_001B0360))
        return 0;
    if (!(func_00282C48(v[1]) <= D_L17_001B0360))
        return 0;
    if (func_00282C48(D_00189EA0[2] - *(float *)(a0 + 0x18)) < 2.0f)
        return 1;
    return 0;
}

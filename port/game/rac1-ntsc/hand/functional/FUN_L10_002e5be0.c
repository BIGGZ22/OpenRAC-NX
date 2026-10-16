/* Carried from the PAL decompilation's src/overlays/l10_orxon/vendor_002E30F8.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern s32 func_001FA6D0(f32);
extern float func_002132A8(float, float);
extern float func_001F96B0(float);
extern int func_00213260(int);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern float func_001FA580(float, float);
extern short D_L10_00161EF0, D_L10_00161EF4, D_L10_00161EF8, D_L10_00161EFC, D_L10_00161F00, D_L10_00161F04;
extern void func_00214DB0(void *, float, float, float);
extern void func_001F9A68(void *, void *, float);
extern int func_001FA6E0(int, int, float);
extern int func_L00_002738E8(void *, void *, int, float, float, float, int, int, int);



/* Exhaust vent: each frame emits a burst of smoke puffs from a random spot across the vent, blown out
 * along its facing; some are small fast sparks instead. */
void FUN_L10_002e5be0(char *m) {
    float pos[4];
    float vel[4];
    float off[4];
    char *d = *(char **)(m + 0x78);
    int i;
    ((unsigned char *)m)[0x30] = 0x80;
    for (i = 0; i < *(int *)(d + 0xC); i++) {
        int spark = func_00213260(100) < *(int *)&D_L10_00161F04;
        char *mp = m + 0x10;
        float a, b, c;
        float lo, hi;
        float s;
        int c1, c2;
        qcopy(pos, mp);
        off[0] = func_001F9DC8(func_001FA580(*(float *)(m + 0x48), 1.5707964f)) * func_002132A8(-*(float *)(d + 0x10), *(float *)(d + 0x10));
        off[1] = func_001F9DE0(func_001FA580(*(float *)(m + 0x48), 1.5707964f)) * func_002132A8(-*(float *)(d + 0x10), *(float *)(d + 0x10));
        off[2] = 0.0f;
        off[2] = func_002132A8(-*(float *)(d + 0x14), *(float *)(d + 0x14));
        func_001F9A10(pos, pos, off);
        a = func_002132A8(*(float *)&D_L10_00161EFC, *(float *)&D_L10_00161EF8);
        b = func_002132A8(*(float *)&D_L10_00161EFC, *(float *)&D_L10_00161EF8);
        c = func_002132A8(*(float *)&D_L10_00161EFC, *(float *)&D_L10_00161EF8);
        if (a < b) a = b;
        if (a < c) a = c;
        func_00214DB0(vel, a * D_0015ED6C, *(float *)(m + 0x48), -*(float *)(m + 0x44));
        if (spark) {
            func_001F9A68(vel, vel, func_002132A8(0.5f, 1.5f));
            s = *(float *)(d + 4) * func_002132A8(0.24f, 0.36f);
        } else {
            s = *(float *)(d + 4) * func_002132A8(0.8f, 1.2f);
        }
        c1 = func_001FA6E0(*(int *)&D_L10_00161EF0, *(int *)&D_L10_00161EF4, func_002132A8(0.5f, 1.0f));
        c2 = func_001FA6E0(*(int *)&D_L10_00161EF4, *(int *)&D_L10_00161EF4 & 0xFF000000, func_002132A8(0.25f, 0.5f));
        func_L00_002738E8(pos, vel, func_001FA6D0(func_001F96B0(*(float *)(d + 8) * 60.0f)), s, *(float *)d, *(float *)&D_L10_00161F00 * D_0015ED70, c1, c2, spark);
    }
}

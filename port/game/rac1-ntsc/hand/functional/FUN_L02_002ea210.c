/* Carried from the PAL decompilation's nonmatching/shared/func_L02_002EB648.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L02_002ea210 -- src/overlays/shared/vendor_002A5218.c
 * Best so far: SIZE ours 1472 / retail 1468, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped after 10 runs (budget spent). Best candidate p7.c: SIZE 1472 vs 1468, case 3 branch layout and the pro
 *   Unblock: the scheduler's placement of the case 0 sb/lwc1 pair and of the D_L02_00179A10 test in case 4; a diff
 */
extern char D_0013E533[];
extern char D_0013D4E3[];
extern float D_L02_00174468;
extern float D_L02_0015F3FC MACRO_ADDR;
extern int D_L02_0015F5CC MACRO_ADDR;
extern int D_L02_0015F5C4 MACRO_ADDR;
extern int D_L02_0015F640 MACRO_ADDR;
extern int D_L02_00179A10 MACRO_ADDR;
extern char *D_L02_00167400;
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern int func_001F9688(int);
extern void func_L02_002EA7D0(char *moby);
extern float func_00213508(void *, int, float);
extern float func_001FA580(float, float);
extern float func_001FA5C8(float, float);
extern void func_L02_002EA8B0(char *m);
extern float func_001F9B80(void *, void *);
extern float func_001F99C0(float);
extern void func_L00_00298840(int);
extern void func_L00_00260860(int, int);
extern void func_L00_00263D40(int, int);
extern void func_L00_002039A0(void);
extern int func_L00_00203908(int, int);
extern void func_L00_00216F90(void *, void *, int, int);
extern int func_0020B178(int, int);
extern void func_0020C828(void *);
extern float func_00213F38(float *, float *, float, float, float, float);
extern void func_001F9A40(void *, void *, void *, float);
extern void func_L00_002EAAA0(void *, void *, int, int, int);
extern void func_L00_002EA9D8(void *);
extern void func_L00_002EAA30(void *);
extern float func_00213ED8(float *, float, float);
extern void func_L00_002EAC18(int);
typedef int u128_002EB648_c2ea210 __attribute__((mode(TI)));

/* Update function of moby class 1005 (tresspasser) on level 02, class 1016 (hydrodisplacer) on level 06: state machine. */
void FUN_L02_002ea210(char *moby) {
    char *d;
    char *g;
    char *gp;
    unsigned char *q;
    int *e;
    int sel;
    float r;
    float t;
    float e70;
    float e6c;

    sel = 0x16;
    d = *(char **)(moby + 0x78);
    r = 3.1415927f / (float)func_001F9688(0x3C);
    if (*(short *)(moby + 0xA6) == 0x3ED) {
        sel = 0x1A;
    }

    switch (*(unsigned char *)(moby + 0x20)) {
    case 0:
        func_L02_002EA7D0(moby);
        q = (unsigned char *)(D_0013D4E3 + 5);
        if (q[sel] != 0) {
            func_0020C828(moby);
            break;
        }
        func_00213508(moby + 0x10, 0, 0.5f);
        *(float *)(moby + 0x44) = 3.1415927f;
        *(float *)(moby + 0x40) = 1.5707964f;
        moby[0x20] = 1;
        *(float *)(moby + 0x18) = D_L02_00174468 + 0.2f;
        break;
    case 1:
        *(float *)(moby + 0x48) = func_001FA580(*(float *)(moby + 0x48), r);
        func_L02_002EA8B0(moby);
        if (D_L02_0015F5CC % 10 != 0) {
            break;
        }
        g = D_0013E533 + 0xE9D;
        if (!(func_001F9B80(moby + 0x10, g) < 1.0f)) {
            break;
        }
        t = func_001F99C0(*(float *)(moby + 0x18) - *(float *)(g + 0x8));
        if (!(t < 2.0f)) {
            break;
        }
        moby[0x20] = 3;
        *(unsigned short *)(moby + 0x34) |= 1;
        moby[0x31] = 0;
        *(int *)(moby + 0x94) = 0;
        if (*(short *)(moby + 0xA6) == 0x3ED) {
            func_L00_00298840(4);
        } else {
            func_L00_00298840(2);
        }
        break;
    case 2:
        break;
    case 3:
        if (D_L02_0015F5C4 == 2) {
            break;
        }
        if (*(unsigned char *)(D_0013E533 + 0x2EC1) != 0) {
            if (sel == 0x16) {
                func_L00_00260860(0x16, 0);
                D_L02_0015F640 = 0;
            } else {
                func_L00_00260860(sel, 0);
            }
        } else {
            func_L00_00260860(sel, 1);
        }
        if (*(short *)(moby + 0xA6) == 0x3ED) {
            func_L00_00263D40(0x7DF, -1);
            func_L00_002039A0();
            func_L00_00203908(0x7D9, 0x13);
            D_L02_0015F3FC = 1.0f;
            moby[0x20] = 4;
            func_L00_00216F90(D_0013E533 + 0xE9D, D_0013E533 + 0xE9D + 0x10, 0x72, 1);
        } else {
            func_0020B178(0, -1);
            func_0020C828(moby);
        }
        break;
    case 4:
        {
            float a[16];
            float o[4];
            float w[4];
            *(u128_002EB648_c2ea210 *)&a[0] = 0;
            *(u128_002EB648_c2ea210 *)&a[4] = 0;
            *(u128_002EB648_c2ea210 *)&a[8] = 0;
            *(u128_002EB648_c2ea210 *)&a[12] = 0;
            a[0] = 91.2f;
            a[1] = 270.76f;
            a[2] = 64.85f;
            a[4] = 106.98f;
            a[5] = 273.46f;
            a[6] = 62.4f;
            a[9] = 0.33f;
            a[10] = 0.55f;
            a[12] = 0.07f;
            a[13] = 0.43f;
            a[14] = 0.72f;
            a[8] = 0.07f;
            e70 = D_0015ED70 * 0.15f;
            e6c = D_0015ED6C * 0.15f;
            func_00213F38((float *)(d + 0x40), (float *)(d + 0x44), 1.0f, e70, e70, e6c);
            func_001F9A40(o, a, a + 4, *(float *)(d + 0x40));
            t = func_001FA5C8(a[14], a[10]);
            w[2] = func_001FA580(t * *(float *)(d + 0x40), a[10]);
            t = func_001FA5C8(a[13], a[9]);
            w[1] = func_001FA580(t * *(float *)(d + 0x40), a[9]);
            t = func_001FA5C8(a[12], a[8]);
            w[0] = func_001FA580(t * *(float *)(d + 0x40), a[8]);
            gp = D_L02_00167400;
            if (gp != 0 && *(short *)(gp + 0x86) != 5) {
                func_L00_002EAAA0(o, w, 1, 0, 0);
            }
            func_L00_002EA9D8(o);
            func_L00_002EAA30(w);
            e = &D_L02_00179A10;
            if (D_L02_00179A10 == 0 && e[9] == -1 && *(float *)(d + 0x40) > 0.2f) {
                func_00213ED8(&D_L02_0015F3FC, 1.0f, D_0015ED6C * 4.0f);
            } else {
                func_00213ED8(&D_L02_0015F3FC, 0.0f, D_0015ED6C * 4.0f);
            }
            if (D_L02_0015F3FC == 1.0f) {
                g = D_0013E533 + 0xE9D;
                func_L00_00216F90(g, g + 0x10, 0, 0);
                func_L00_002EAC18(0);
                moby[0x20] = 5;
            }
        }
        break;
    case 5:
        func_00213ED8(&D_L02_0015F3FC, 0.0f, D_0015ED6C * 4.0f);
        if (D_L02_0015F3FC == 0.0f) {
            func_0020B178(0, -1);
            func_0020C828(moby);
        }
        break;
    }
}

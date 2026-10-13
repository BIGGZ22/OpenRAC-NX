/* Carried from the PAL decompilation's nonmatching/l14_oltanis/func_L14_00316388.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L14_00314f00 -- src/overlays/l14_oltanis/vendor_002FF358.c
 * Best so far: SIZE ours 892 / retail 908, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Camera activate (camera 20, level 14): clears the record, builds two 8-entry path tables from D_L14_001B0BB0 w
 *   Left: retail keeps the moby pointer in $fp and reloads the count from memory at more points than ours; ours sp
 *   Unblock: which local retail keeps in $fp (the moby) and where it reloads the table count; the spills are the l
 */
extern char *D_L14_0015EF50 MACRO_ADDR;
extern char *D_L14_001B0BB0[];
extern int func_001F9688(int);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B48(void *, void *);
extern float func_001FA6C0(int);
extern char D_L14_0016D140[];
extern char D_L14_00167380[];
extern char *D_L14_001601AC_t __asm__("D_L14_001600EC") MACRO_ADDR;
extern void func_001F99F8(void *);
extern void func_L00_00216F90(void *, void *, int, int);

/* Activate function of camera 20 (level 14): clears the record, builds the path tables and copies the camera vectors. */
void FUN_L14_00314f00(char *cam) {
    float v[4] __attribute__((aligned(16)));
    char *obj;
    char *data;
    char *c20;
    char *ent;
    char *p;
    int i, k, m, cnt;
    float *fo;
    short *so;
    float d, f0, f20, f21;

    obj = *(char **)(D_L14_0015EF50 + *(short *)(cam + 0x84) * 32 + 0x1C);
    obj[0x38] = 3;
    data = *(char **)(cam + 0x70);
    c20 = data + 0x40;
    *(int *)(data + 0x40) = 0;
    *(int *)(c20 + 4) = 0;
    *(int *)(c20 + 8) = 0;
    *(int *)(c20 + 0xC) = 0;
    *(int *)(c20 + 0x10) = 0;
    *(short *)(c20 + 0x14) = 0;
    *(short *)(c20 + 0x16) = func_001F9688(*(short *)(obj + 0x3A));

    for (k = 0; k < 8; k++) {
        *(int *)(data + 0x68 + k * 4) = 0;
        *(short *)(data + 0x58 + k * 2) = -1;
    }

    *(float *)(data + 0x88) = func_L00_001FF8B0(1.0f, *(float *)(D_L14_0016D140 + 0xB0)) * 2.0f;

    if (*(int *)(obj + 0x20) >= 0) {
        ent = D_L14_001B0BB0[*(int *)(obj + 0x20)];
        cnt = *(int *)ent;
        if (cnt > 0) {
            i = 0;
            f20 = 0.0f;
            fo = (float *)(data + 0x68);
            so = (short *)(data + 0x58);
            do {
                f0 = *(float *)(ent + i * 16 + 0x1C);
                if (f20 < f0) {
                    *fo++ = f0;
                    *so++ = i;
                }
                m = i + 1;
                d = func_001F9B48(ent + m * 16, ent + (m % cnt) * 16 + 0x10);
                *(float *)(ent + i * 16 + 0x1C) = d;
                if (i < *(int *)ent - 1) {
                    *(float *)(data + 0x48) = *(float *)(data + 0x48) + d;
                }
                i = m;
            } while (i < *(int *)ent);
        }
    }

    if (*(short *)(obj + 0x3A) > 0) {
        f0 = func_001FA6C0(func_001F9688(*(short *)(obj + 0x3A)));
        *(float *)(obj + 0x3C) = *(float *)(data + 0x48) / f0 * 60.0f;
    }

    if (*(int *)(obj + 0x24) >= 0) {
        ent = D_L14_001B0BB0[*(int *)(obj + 0x24)];
        cnt = *(int *)ent;
        if (cnt > 0) {
            i = 0;
            do {
                m = i + 1;
                d = func_001F9B48(ent + m * 16, ent + (m % *(int *)ent) * 16 + 0x10);
                *(float *)(ent + i * 16 + 0x1C) = d;
                if (i < cnt - 1) {
                    *(float *)(data + 0x4C) = *(float *)(data + 0x4C) + d;
                }
                i = m;
            } while (i < cnt);
        }
    }

    data = *(char **)(cam + 0x70);
    f20 = 0.03f;
    f21 = 0.2f;
    *(float *)(data + 0) = f20;
    *(float *)(data + 4) = f21;
    func_001F99F8(data + 0x10);
    data = *(char **)(cam + 0x70);
    *(float *)(data + 0x20) = f20;
    *(float *)(data + 0x24) = f21;
    func_001F99F8(data + 0x30);

    p = *(char **)(D_L14_00167380 + 0x184);
    qcopy(cam + 0x30, p + 0x30);
    qcopy(cam, p);
    qcopy(cam + 0x10, p + 0x10);
    qcopy(cam + 0x20, p + 0x20);

    qzero(v);
    v[3] = 1.0f;
    func_L00_00216F90(D_L14_001601AC_t + *(int *)(obj + 0x54) * 128 + 0x30, v, 0x72, 0);
}

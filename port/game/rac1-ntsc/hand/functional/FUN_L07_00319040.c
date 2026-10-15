/* Carried from the PAL decompilation's nonmatching/l07_umbris/func_L07_0031A410.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L07_00319040 -- src/overlays/l07_umbris/vendor_00313D28.c
 * Best so far: SIZE ours 1596 / retail 1624, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Homing mine update: a state machine on moby[0x20] (jump table, cases 0-4, 2 falls into 3), a homing/bobbing bl
 *   Left: retail saves f20 for a 1.0f that it keeps across the aim call and reuses (mov.s $f12,$f20 / $f15,$f20); 
 *   Unblock: a source form that keeps 1.0 live in a saved float (retail may read it from a global), then the doubl
 */
typedef int u128_1A410_c319040 __attribute__((mode(TI)));

extern void func_L00_0025FCB8(void *, void *, float);
extern float func_001F9B48(void *, void *);
extern void func_L00_0025A120(void *);
extern char *func_L00_0025A420(void *, int, int);
extern int func_L00_0025A478(void *, void *, void *, int, int *, float *, int, int);
extern void func_001F99F8(void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, int, float, float, int, int, int, int);
extern void func_L00_00257470(void *, int, int);
extern int func_001F9688(int);
extern float func_001FA580(float, float);
extern float func_001F9B80(void *, void *);
extern float func_001F99C0(float);
extern void func_L01_0026E0E0(int list, int state);
extern void func_00212F90(void *, int, int, int);
extern int func_001F9740(int *);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_00259888(void *, void *, int, float, void *);
extern int func_L00_001F2868(float, void *, int, void *, void *);
extern int func_L00_001F0D60(float, void *, int, void *);
extern void func_0020C828(void *);
extern void func_L00_00259FE8(unsigned char *, float);
extern char D_L07_00166E40[];
extern char D_L07_00173EF0[];
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED60 MACRO_ADDR;
extern char *D_L07_00173F58_p __asm__("D_L07_00173ED8");

/* Homing mine update (moby class 1110): per-state homing and bobbing, aims at the target moby. */
void FUN_L07_00319040(char *moby) {
    char *data;
    char *p17;
    char *p38;
    char *pp;
    int ob[24];
    float v70[4];
    float v80[4];
    float vb0[4];
    float vc0[4];
    int r0;
    int st;
    int cls;
    float one;

    if (moby == 0) {
        return;
    }
    data = *(char **)(moby + 0x78);
    if (data == 0) {
        return;
    }
    func_L00_0025FCB8(moby, ob, 8.0f);
    if (*(unsigned char *)(moby + 0x31) != 0) {
        if (func_001F9B48(moby + 0x10, D_L07_00166E40) < 27.0f) {
            func_L00_0025A120(moby);
            *(unsigned char *)(moby + 0x7F) = 0x15;
        }
    }
    p17 = func_L00_0025A420(moby, 0x230000, 0);
    func_L00_0025A478(moby, p17, data + 0x20, 0, &r0, 0, 0, 4);
    if (r0 >= 2 && p17 != 0) {
        cls = *(short *)(*(char **)(p17 + 0x20) + 0xA6);
        if (cls != 0x456 && cls != 0x365 && cls != 0x367) {
            func_001F99F8(v70);
            func_L00_0025E450(moby, v70, 0, 0.0f, 0.0f, 3, 3, 5, 2.0f, 1.0f, 4.0f, 2, 1.0f, 7.0f, 0, 1, -1, 0);
            func_L00_00257470(moby, 0, -1);
            goto aa1c;
        }
    }

    st = *(unsigned char *)(moby + 0x20);
    switch (st) {
    case 0:
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 0x1000;
        {
            char *src = moby + 0x10;
            *(u128_1A410_c319040 *)(data + 0x60) = *(u128_1A410_c319040 *)src;
        }
        *(unsigned char *)(moby + 0x20) = 1;
        *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
        goto aa2c;
    case 1:
        *(int *)(data + 0x78) = func_001F9688(600);
        *(float *)(moby + 0x48) = func_001FA580(*(float *)(moby + 0x48), D_0015ED6C * 6.2831855f);
        p38 = *(char **)(data + 0x38);
        if (p38 != 0) {
            if (func_001F9B80(moby + 0x10, p38 + 0x10) < 20.0f) {
                if (func_001F99C0(*(float *)(moby + 0x18) - *(float *)(p38 + 0x18)) < 3.0f) {
                    *(int *)(data + 0x70) = 0;
                    goto l6b4;
                }
            }
        }
        if (ob[16] == 0) {
            goto aa2c;
        }
        if (!(func_001F9B48(moby + 0x10, ob) < 8.0f)) {
            goto aa2c;
        }
        *(int *)(data + 0x70) = 0;
    l6b4:
        *(int *)(data + 0x78) = func_001F9688(600);
        *(unsigned char *)(moby + 0xBC) = 1;
        *(int *)(data + 0x7C) = func_001F9688(7);
        *(unsigned char *)(moby + 0x20) = 3;
        if (*(int *)(data + 0x74) != -1) {
            func_L01_0026E0E0(*(int *)(data + 0x74), 2);
        }
        *(unsigned char *)(moby + 0x20) = 2;
        goto aa2c;
    case 2:
        if (*(unsigned char *)(moby + 0x53) != 1) {
            func_00212F90(moby, 1, 0, 10);
        }
        *(unsigned char *)(moby + 0x20) = 3;
        /* falls through */
    case 3:
        if (*(unsigned char *)(moby + 0x70) & 2) {
            if (*(unsigned char *)(moby + 0x52) == 1 && *(unsigned char *)(moby + 0x53) != 2) {
                func_00212F90(moby, 2, 0, 10);
            }
        }
        {
            float f2 = D_0015ED70;
            float t = *(float *)(data + 0x70) + f2 * 30.0f;
            float m;
            *(float *)(data + 0x70) = t;
            m = D_0015ED6C * 6.0f;
            if (m < t) {
                *(float *)(data + 0x70) = m;
            }
        }
        if (*(unsigned char *)(moby + 0xBC) == 1) {
            *(float *)(moby + 0x18) = *(float *)(moby + 0x18) + D_0015ED60 * 0.1f;
            if (func_001F9740((int *)(data + 0x7C)) != 0) {
                *(int *)(data + 0x7C) = func_001F9688(7);
                *(unsigned char *)(moby + 0xBC) = 2;
            }
        } else if (*(unsigned char *)(moby + 0xBC) == 2) {
            *(float *)(moby + 0x18) = *(float *)(moby + 0x18) - D_0015ED60 * 0.1f;
            if (func_001F9740((int *)(data + 0x7C)) != 0) {
                *(unsigned char *)(moby + 0xBC) = 0;
            }
        }
        func_001F9A28(v70, ob, moby + 0x10);
        func_L00_001FF500(v70, v70, *(float *)(data + 0x70));
        func_001F9A10(moby + 0x10, moby + 0x10, v70);
        if (64.0f < func_001F9B48(moby + 0x10, data + 0x60)) {
            goto aa1c;
        }
        one = 1.0f;
        func_L00_00259888(v80, moby, 0x10001, one, v70);
        *(u128_1A410_c319040 *)vb0 = *(u128_1A410_c319040 *)(moby + 0x10);
        vb0[2] = vb0[2] + 0.75f;
        if (func_L00_001F2868(0.75f, vb0, 0x10, moby, v80) != 0) {
            pp = D_L07_00173F58_p;
            if (pp != 0) {
                cls = *(short *)(pp + 0xA6);
                if (cls != 0x363 && cls != 0x456 && cls != 0x458 && cls != 0x365 && cls != 0x367) {
                    goto l9c8;
                }
            }
        }
        *(u128_1A410_c319040 *)vb0 = *(u128_1A410_c319040 *)(moby + 0x10);
        vb0[2] = vb0[2] + 0.55f;
        if (func_L00_001F0D60(0.55f, vb0, 0, moby) != 0) {
            func_001F9A28(vc0, D_L07_00173EF0, vb0);
            func_001F9A10(moby + 0x10, moby + 0x10, vc0);
        }
        if (func_001F9740((int *)(data + 0x78)) == 0) {
            goto aa2c;
        }
    l9c8:
        func_L00_0025E450(moby, v70, 0, 0.0f, 0.0f, 3, 3, 5, 2.0f, one, 4.0f, 2, one, 7.0f, 0, 1, -1, 0);
    aa1c:
        func_0020C828(moby);
        return;
    case 4:
    default:
        goto aa2c;
    }
aa2c:
    func_L00_00259FE8((unsigned char *)moby, 0.7f);
}

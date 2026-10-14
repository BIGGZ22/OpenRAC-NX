/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002CC3C0.c (games/rac1/pal) through the version
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
/* What the PAL file_c2caf70 this was written in declares before it. */
extern unsigned char D_0013E05A[];

/* NON_MATCHING FUN_L00_002caf70 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: SIZE ours 1180 / retail 1188, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file_c2caf70
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_002caf70 (1188 bytes): builds a moby's data block from a source moby and a 16-byte position vector, r
 *   Stopped after 5 runs (best p3.c, SIZE 1180 vs 1188). Differences are structural, not one tie: retail keeps a c
 *   Harness note: the candidate prelude already holds a u128 typedef at the end of the file_c2caf70; a u128 typedef in the
 */
typedef int w128_c2caf70 __attribute__((mode(TI)));

extern char *func_0020D348_2CD3B8(int) __asm__("func_0020C4F8");
extern float func_001FA580(float, float);
extern int func_L00_0025C338(char *);
extern void func_001F99F8(void *);
extern int func_001F9688(int);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern void func_00214DB0(void *, float, float, float);
extern float func_002132A8(float, float);
extern void func_L00_001FF660(void *, void *, void *);
extern float func_001F9AF0(void *a);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_00257B90(int, int);
extern char *func_L00_0026DD20(char *pos, char *vel, int c, int d, float f);
extern void func_0020C828(void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B20(void *);
extern char *func_L00_0026DAA0(char *, int, int, int, float);
extern unsigned char *func_L00_00275158(char *a, char *b, int c, float f);
extern float func_001FA6C0(int);
extern float func_L00_00257E20(float, float, float, float, float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern void func_L00_002CBCC0(void *, int);
extern float D_L00_00161894 MACRO_ADDR;
extern char D_L00_00173E60[];
extern unsigned char D_L00_00173E80[];
extern float D_0015ED6C MACRO_ADDR;
extern float D_L00_0015F580[] MACRO_ADDR;
extern short D_L00_00161890;
extern short D_L00_00161888;
extern short D_L00_00161870;
extern short D_L00_00161874;
extern short D_L00_00161878;

// Builds a moby's data block from its source moby and a position vector; returns the new moby or 0.
char *FUN_L00_002caf70(char *a, char *vec, char *b, float f12, float f13, float f14, float f15) {
    w128_c2caf70 tv;
    float lf[4];
    float out[4];
    w128_c2caf70 tmp;
    float v[4];
    char *m;
    char *d;
    char *q;
    unsigned char *p;
    int r;
    int r2;
    int r3;
    int i;
    int *arr;
    float len;
    float ret;
    float fi;
    float f16;

    tv = *(w128_c2caf70 *)vec;
    m = func_0020D348_2CD3B8(0x131);
    if (m == 0) {
        return 0;
    }
    d = *(char **)(m + 0x78);
    *(char **)(d + 0x20) = a;
    *(short *)(m + 0x32) = 0x7F;
    m[0x30] = 0x7F;
    m[0x31] = 1;
    *(long long *)(m + 0x38) = *(long long *)(a + 0x38);
    *(float *)(m + 0x40) = *(float *)&D_L00_00161890;
    *(float *)(m + 0x44) = func_001FA580(f15, D_L00_00161894);
    *(float *)(m + 0x48) = f14;
    *(w128_c2caf70 *)(m + 0x10) = tv;
    if (b != 0) {
        *(w128_c2caf70 *)d = *(w128_c2caf70 *)(b + 0x10);
        r = func_L00_0025C338(b);
        *(float *)(d + 0x8) = *(float *)(d + 0x8) + (r != 0 ? *(float *)(r + 0x10) : 0.5f);
    } else {
        func_001F99F8(d);
    }
    *(float *)(d + 0x18) = f12;
    *(float *)(d + 0x1C) = f13;
    *(int *)(d + 0x10) = 0;
    *(int *)(d + 0x14) = 0;
    r = func_001F9688(0x23);
    p = (unsigned char *)D_0013E05A + 0x4C6;
    *(int *)(d + 0x28) = 0;
    r = r + p[0xF] * 10;
    *(short *)(d + 0x26) = r;
    *(short *)(d + 0x24) = r;
    *(w128_c2caf70 *)lf = *(w128_c2caf70 *)(a + 0x10);
    lf[2] = ((float *)&tv)[2];
    r = func_L00_001EFC70(lf, &tv, 0, (int)a, 0);
    if (r != 0) {
        func_00214DB0(out, D_0015ED6C * 40.0f, *(float *)(m + 0x48), -*(float *)(m + 0x44));
        *(w128_c2caf70 *)(m + 0x10) = *(w128_c2caf70 *)D_L00_00173E60;
        i = 4;
        do {
            *(w128_c2caf70 *)v = 0;
            v[0] = func_002132A8(-1.0f, 1.0f);
            i--;
            v[1] = func_002132A8(-1.0f, 1.0f);
            v[2] = func_002132A8(-1.0f, 1.0f);
            tmp = *(w128_c2caf70 *)v;
            func_L00_001FF660(v, out, D_L00_00173E80);
            len = func_001F9AF0(v);
            func_L00_001FF500(&tmp, &tmp, len * 0.5f);
            func_001F9A10(v, v, &tmp);
            func_L00_001FF500(v, v, func_002132A8(D_0015ED6C * 3.0f, D_0015ED6C * 6.0f));
            r = func_001F9688(10);
            r2 = func_001F9688(15);
            r3 = func_L00_00257B90(r, r2);
            func_L00_0026DD20(m + 0x10, (char *)v, 0x7F2F4F6F, r3, 30000.0f);
        } while (i >= 0);
        p = (unsigned char *)D_0013E05A + 0x4C6;
        if (p[0xF] == 0) {
            func_L00_002CBCC0(m, 0);
            func_0020C828(m);
            return 0;
        }
        *(int *)(d + 0x28) = *(int *)(d + 0x28) + 1;
        if (p[0xF] + 5 < *(int *)(d + 0x28)) {
            func_L00_002CBCC0(m, 1);
            func_0020C828(m);
            return 0;
        }
        func_L00_001FF660(out, out, D_L00_00173E80);
        *(float *)(m + 0x48) = func_L00_001FF8B0(out[0], out[1]);
        len = func_001F9B20(out);
        *(float *)(m + 0x44) = -func_L00_001FF8B0(len, out[2]);
        return m;
    } else {
        arr = (int *)(d + 0x2C);
        func_L00_0026DAA0(m, 0x2F4F7F7F, *(short *)(d + 0x24), -1, *(float *)&D_L00_00161888);
        for (i = 0; i < 17; i++) {
            q = (char *)func_L00_00275158(m + 0x10, (char *)D_L00_0015F580, 1, 0.0f);
            if (q != 0) {
                *(float *)(q + 0xC) = func_002132A8(*(float *)&D_L00_00161870, *(float *)&D_L00_00161874);
                fi = func_001FA6C0(i);
                f16 = fi / func_001FA6C0(16);
                ret = func_L00_00257E20(-1.5f, 0.0f, 1.0f, 0.0f, f16);
                *(float *)(q + 0xC) = *(float *)(q + 0xC) + (*(float *)&D_L00_00161878 - *(float *)(q + 0xC)) * ret;
                r = func_001FA898_r(ret * -64.0f + 128.0f);
                *(int *)(q + 4) = (r << 24) | 0xFFFFFF;
                arr[i] = (int)q;
            } else {
                arr[i] = 0;
            }
        }
    }
    return m;
}

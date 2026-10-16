/* Carried from the PAL decompilation's src/overlays/shared/vendor_002C96D0.c (matched C there) (games/rac1) through the version
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
extern char D_L00_00173E60[];
extern int func_L00_0025C338(char *);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_001FF660(void *, void *, void *);
extern float func_002132A8(float, float);
extern void func_001F99F8(void *);
extern float func_L00_001FF8B0(float, float);
extern int func_001F9688(int);
extern int func_L00_00257B90(int, int);
extern float func_001FA6C0(int);
extern void func_0020C828(void *);
extern unsigned char D_0013E05A[];
typedef int w128_c2caf70 __attribute__((mode(TI)));
extern unsigned char *func_0020C4F8(int);
extern float func_001FA580(float, float);
extern void func_00214DB0(void *, float, float, float);
extern float func_001F9AF0(void *a);
extern void *func_L00_0026DD20(void *, void *, unsigned, int, float);
extern float func_001F9B20(void *);
extern char *func_L00_0026DAA0(char *, int, int, int, float);
extern unsigned char *func_L00_00275158(char *a, char *b, int c, float f);
extern float func_L00_00257E20(float, float, float, float, float);
extern int func_001FA6D0(float);
extern void func_L00_002CBCC0(unsigned char *, int);
extern unsigned char D_L00_00173E80[];
extern float D_0015ED6C MACRO_ADDR;
extern float D_L00_0015F580[] MACRO_ADDR;
extern float D_L00_00161890 MACRO_ADDR;
extern short D_L00_00161888;
extern short D_L00_00161870;
extern short D_L00_00161874;
extern short D_L00_00161878;



// Builds a moby's data block from its source moby and a position vector; returns the new moby or 0.
char *FUN_L00_002caf70(char *a, char *vec, char *b, float f12, float f13, float f14, float f15) {
    float tv[4];
    float lf[4];
    float out[4];
    float tmp[4];
    float v[4];
    unsigned char *m;
    char *pos;
    char *d;
    char *q;
    unsigned char *p;
    int r;
    int r2;
    int r3;
    int i;
    int k;
    int *arr;
    float len;
    float ret;
    float fi;
    float f16;
    float *t;

    t = tv;
    *(w128_c2caf70 *)tv = *(w128_c2caf70 *)vec;
    m = func_0020C4F8(0x131);
    if (m != 0) {
        d = *(char **)(m + 0x78);
        *(char **)(d + 0x20) = a;
        m[0x30] = 0x7F;
        *(short *)(m + 0x32) = 0x7F;
        m[0x31] = 1;
        *(long *)(m + 0x38) = *(long *)(a + 0x38);
        *(float *)(m + 0x40) = ((float *)&D_L00_00161888)[2];
        *(float *)(m + 0x44) = func_001FA580(f15, (&D_L00_00161890)[1]);
        *(float *)(m + 0x48) = f14;
        qcopy(m + 0x10, t);
        if (b != 0) {
            int rr;
            qcopy(d, b + 0x10);
            rr = func_L00_0025C338(b);
            if (rr != 0) {
                *(float *)(d + 0x8) = *(float *)(d + 0x8) + *(float *)(rr + 0x10);
            } else {
                *(float *)(d + 0x8) = *(float *)(d + 0x8) + 0.5f;
            }
        } else {
            func_001F99F8(d);
        }
        *(float *)(d + 0x18) = f12;
        *(float *)(d + 0x1C) = f13;
        *(int *)(d + 0x10) = 0;
        *(int *)(d + 0x14) = 0;
        r2 = func_001F9688(0x23);
        p = (unsigned char *)D_0013E05A + 0x4C6;
        r2 = r2 + p[0xF] * 10;
        *(int *)(d + 0x28) = 0;
        *(short *)(d + 0x24) = r2;
        *(short *)(d + 0x26) = r2;
        *(w128_c2caf70 *)lf = *(w128_c2caf70 *)(a + 0x10);
        lf[2] = t[2];
        r = func_L00_001EFC70(lf, t, 0, (int)a, 0);
        if (r != 0) {
            func_00214DB0(out, D_0015ED6C * 40.0f, *(float *)(m + 0x48), -*(float *)(m + 0x44));
            qcopy(m + 0x10, D_L00_00173E60);
            t = tmp;
            k = 4;
            do {
                *(w128_c2caf70 *)v = 0;
                v[0] = func_002132A8(-1.0f, 1.0f);
                k--;
                v[1] = func_002132A8(-1.0f, 1.0f);
                v[2] = func_002132A8(-1.0f, 1.0f);
                *(w128_c2caf70 *)tmp = *(w128_c2caf70 *)v;
                func_L00_001FF660(v, out, D_L00_00173E80);
                len = func_001F9AF0(v);
                func_L00_001FF500(t, t, len * 0.5f);
                func_001F9A10(v, v, t);
                func_L00_001FF500(v, v, func_002132A8(D_0015ED6C * 3.0f, D_0015ED6C * 6.0f));
                r = func_001F9688(10);
                r2 = func_001F9688(15);
                r3 = func_L00_00257B90(r, r2);
                func_L00_0026DD20(m + 0x10, (char *)v, 0x7F2F4F6F, r3, 30000.0f);
            } while (k >= 0);
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
            func_L00_0026DAA0(m, 0x2F4F7F7F, *(short *)(d + 0x24), -1, *(float *)&D_L00_00161888);
            for (i = 0; i < 17; i++) {
                q = (char *)func_L00_00275158(m + 0x10, (char *)D_L00_0015F580, 1, 0.0f);
                if (q != 0) {
                    *(float *)(q + 0xC) = func_002132A8(*(float *)&D_L00_00161870, *(float *)&D_L00_00161874);
                    fi = func_001FA6C0(i);
                    f16 = fi / func_001FA6C0(16);
                    ret = func_L00_00257E20(-1.5f, 0.0f, 1.0f, 0.0f, f16);
                    *(float *)(q + 0xC) = *(float *)(q + 0xC) + (*(float *)&D_L00_00161878 - *(float *)(q + 0xC)) * ret;
                    r = func_001FA6D0(ret * -64.0f + 128.0f);
                    *(int *)(q + 4) = (r << 24) | 0xFFFFFF;
                    ((int *)(d + 0x2C))[i] = (int)q;
                } else {
                    ((int *)(d + 0x2C))[i] = 0;
                }
            }
        }
    }
    return m;
}

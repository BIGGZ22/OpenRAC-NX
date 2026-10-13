/* Carried from the PAL decompilation's nonmatching/l06_blarg/func_L06_002F71A8.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L06_002f5d78 -- src/overlays/l06_blarg/vendor_002B5990.c
 * Best so far: SIZE ours 1764 / retail 1784, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Blarg moby sway: two loops (20 slots over the data+0x70/+0xC0 pointer arrays, then 10 slots over +0x110/+0x138
 *   Left over: retail saves $fp/$s6..$s1/$ra and f20-f25; ours keeps an extra saved float ($f26) and a different p
 *   Would unblock with a rewording that makes the saved-register set match retail's (six saved floats, $s0-$s7), w
 */
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9CF8(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_00250DF8(void *);
extern void func_0022DA68(int, int, int);
extern short D_L06_00161D70;
extern short D_L06_00161D74;
extern short D_L06_00161D78;
extern short D_L06_00161D7C;
extern short D_L06_00161D80;
extern short D_L06_00161DA0;
extern short D_L06_00161DA4;
extern short D_L06_00161D50;
extern short D_L06_00161D60;

/* Blarg moby sway: updates two sets of 20 and 10 sample slots from the angle curves. */
void FUN_L06_002f5d78(char *moby)
{
    char *data;
    char *c0;
    char *mb;
    char *s1;
    char *s2;
    char **pa;
    char **pb;
    float v[4];
    float wa[4];
    float xa[4];
    char *w;
    char *x;
    int i;
    int j;
    int off;
    int r2;
    float f0, f1, f2, f20, f21, a, b, q;

    w = (char *)wa;
    c0 = moby + 0xC0;
    mb = moby + 0x10;
    x = (char *)xa;
    data = *(char **)(moby + 0x78);
    off = 0;
    for (i = 0; i < 20; i++) {
        s1 = data + 0x70;
        s2 = data + 0xC0;
        f1 = *(float *)&D_L06_00161D78 - (float)i;
        if (f1 < 0.0f) f1 = 0.0f;
        func_L00_001FF500(w, c0, f1);
        func_001F9CF8(v, &D_L06_00161D50, c0);
        func_001F9A10(v, v, moby + 0x10);
        func_001F9A10(v, v, w);
        qcopy(*(char **)(s1 + off) + 0x10, v);
        func_001F9CF8(v, &D_L06_00161D60, c0);
        func_001F9A10(v, v, moby + 0x10);
        func_001F9A10(v, v, w);
        qcopy(*(char **)(s2 + off) + 0x10, v);
        f1 = *(float *)&D_L06_00161D7C * 0.017453292f;
        f0 = (float)(i * 45) * 0.017453292f;
        f1 = f1 - f0;
        if (1.5707964f < f1) f1 = 1.5707964f;
        else if (f1 < 0.0f) f1 = 0.0f;
        *(float *)(*(char **)(s1 + off) + 0x40) = -f1;
        *(float *)(*(char **)(s2 + off) + 0x40) = f1;
        func_L00_00250DF8(*(char **)(s1 + off));
        func_L00_00250DF8(*(char **)(s2 + off));
        off += 4;
    }

    pa = (char **)(data + 0x110);
    pb = (char **)(data + 0x138);
    for (j = 0; j < 10; j++) {
        r2 = j + j;
        f1 = (float)j + (float)j;
        f0 = (float)(4 - j) * *(float *)&D_L06_00161D74;
        a = *(float *)&D_L06_00161D78 - f1;
        b = *(float *)&D_L06_00161D70 + f0;
        q = a;
        if (a < b) q = b;
        func_L00_001FF500(w, c0, q);
        f20 = a - 1.0f;
        if (f20 < b) f20 = b;
        func_L00_001FF500(x, c0, f20);
        func_001F9CF8(v, &D_L06_00161D50, c0);
        func_001F9A10(v, v, moby + 0x10);
        func_001F9A10(v, v, w);
        qcopy(*pa + 0x10, v);
        func_001F9CF8(v, &D_L06_00161D60, c0);
        func_001F9A10(v, v, moby + 0x10);
        func_001F9A10(v, v, x);
        qcopy(*pb + 0x10, v);

        f0 = *(float *)&D_L06_00161D7C * 0.017453292f;
        f1 = (float)r2 * 0.78539819f;
        f2 = *(float *)&D_L06_00161DA0 * 0.017453292f;
        f21 = f0 - f1;
        f0 = f21 - f2;
        if (1.5707964f < f21) f21 = 1.5707964f;
        else if (f21 < 0.0f) f21 = 0.0f;
        if (1.5707964f < f0) f0 = 1.5707964f;
        else if (f0 < 0.0f) f0 = 0.0f;
        if (f21 != 0.0f && f0 == 0.0f) func_0022DA68(1, 0, (int)*pa);

        f1 = *(float *)&D_L06_00161D7C * 0.017453292f;
        f0 = (float)r2 * 0.78539819f;
        f1 = f1 - f0;
        f20 = f1 - 0.78539819f;
        f2 = *(float *)&D_L06_00161DA0 * 0.017453292f;
        f0 = f20 - f2;
        if (1.5707964f < f20) f20 = 1.5707964f;
        else if (f20 < 0.0f) f20 = 0.0f;
        if (1.5707964f < f0) f0 = 1.5707964f;
        else if (f0 < 0.0f) f0 = 0.0f;
        if (f20 != 0.0f && f0 == 0.0f) func_0022DA68(1, 0, (int)*pb);

        *(float *)(*pa + 0x40) = -f21;
        *(float *)(*pb + 0x40) = f20;
        f0 = *(float *)&D_L06_00161D80 * 0.017453292f;
        f1 = *(float *)&D_L06_00161DA4 * 0.017453292f;
        f2 = (float)r2 * 0.78539819f;
        f21 = f0 - f2;
        f0 = f21 - f1;
        if (1.5707964f < f21) f21 = 1.5707964f;
        else if (f21 < 0.0f) f21 = 0.0f;
        if (1.5707964f < f0) f0 = 1.5707964f;
        else if (f0 < 0.0f) f0 = 0.0f;
        if (f21 != 0.0f && f0 == 0.0f) func_0022DA68(2, 0, (int)*pa);

        f1 = *(float *)&D_L06_00161D80 * 0.017453292f;
        f2 = *(float *)&D_L06_00161DA4 * 0.017453292f;
        f0 = (float)r2 * 0.78539819f;
        f1 = f1 - f0;
        f20 = f1 - 0.78539819f;
        f0 = f21 - f2;
        if (1.5707964f < f20) f20 = 1.5707964f;
        else if (f20 < 0.0f) f20 = 0.0f;
        if (1.5707964f < f0) f0 = 1.5707964f;
        else if (f0 < 0.0f) f0 = 0.0f;
        if (f20 != 0.0f && f0 == 0.0f) func_0022DA68(2, 0, (int)*pb);
        *(float *)(*pa + 0x44) = -f21;
        *(float *)(*pb + 0x44) = -f20;
        func_L00_00250DF8(*pa);
        func_L00_00250DF8(*pb);
        pa++;
        pb++;
    }
}

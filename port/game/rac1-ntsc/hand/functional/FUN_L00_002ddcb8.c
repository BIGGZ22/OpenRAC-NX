/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002DF168.c (games/rac1/pal) through the version
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
/* What the PAL file this was written in declares before it. */
extern char D_0013E533[] NOT_SDA;

/* NON_MATCHING FUN_L00_002ddcb8 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: SIZE ours 664 / retail 680, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Camera matrix setup: picks 3 vec4 from the hero/camera by state, builds matrices via func_L00_001FF500, fills 
 *   Left: retail keeps the &s[3] pointer in $s0 plus a copy in $s4 for the loop (daddu $s4,$s0) and loop addresses
 */
typedef int u128_DF168_c2ddcb8 __attribute__((mode(TI)));
extern void func_001F9A68(void *, void *, float);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F99F8(void *);
extern float func_00213308(void);
extern void func_00214890(void *, void *, float, void *);
extern float D_L00_00161B84 MACRO_ADDR;
extern char D_L00_00166FD0[];
extern char D_L00_001E5F40[];
extern short D_L00_001E6008[];
extern char D_L00_001E5F30[];
extern char D_L00_00161B60[];
extern char D_L00_00161B70[];
extern float D_L00_00161B80 MACRO_ADDR;
extern char D_L00_001E4FD0_x[] __asm__("D_L00_001E4C80");
extern short D_L00_00161B40;
extern short D_L00_00161B44;
extern short D_L00_00161B50;
extern int D_L00_00161B54;
extern int D_L00_00161B58;

// Builds the camera matrices and resets the camera effect state.
void FUN_L00_002ddcb8(char *a, char *b) {
    u128_DF168_c2ddcb8 s[7];
    char *g, *v, *h, *p, *q;
    short *w;
    int i, st;
    if (*(int *)&D_L00_00161B40 == 0) return;
    g = D_0013E533 + 0xE1D;
    D_L00_00161B84 = 1.7f;
    *(int *)&D_L00_00161B40 = 0;
    st = *(int *)(g + 0x2084);
    if ((st == 0x1E || st == 1) && (*(unsigned short *)(*(char **)(g + 0x2080) + 0x34) & 1)) {
        qcopy((char *)s, D_L00_00166FD0);
        qcopy((char *)s + 0x10, D_L00_00166FD0 + 0x10);
        v = (char *)s + 0x20;
        qcopy(v, D_L00_00166FD0 + 0x20);
    } else {
        g = D_0013E533 + 0xE1D;
        if (*(short *)(g + 0x22C8) == 1) {
            func_001F9A68(s, a + 0xD0, -1.0f);
            qcopy((char *)s + 0x10, a + 0xC0);
            v = (char *)s + 0x20;
            qcopy(v, a + 0xE0);
        } else {
            qcopy((char *)s, g + 0x640);
            qcopy((char *)s + 0x10, g + 0x650);
            v = (char *)s + 0x20;
            qcopy(v, g + 0x660);
        }
    }
    h = (char *)s + 0x30;
    func_L00_001FF500(h, s, D_L00_00161B84);
    func_L00_001FF500((char *)s + 0x40, v, 1.0f);
    qcopy(D_L00_001E5F40, b);
    D_L00_001E6008[0] = 0;
    p = D_L00_001E5F40 + 0x10;
    q = D_L00_001E5F30 + 0x10;
    w = D_L00_001E6008 + 1;
    for (i = 8; i >= 0; i--) {
        func_001F9A10(p, q, h);
        q += 0x10;
        p += 0x10;
        *w = 0;
        w++;
    }
    func_001F99F8(D_L00_00161B60);
    s[5] = 0;
    s[6] = 0;
    *(float *)((char *)s + 0x6C) = 1.0f;
    *(float *)((char *)s + 0x58) = 0.1f;
    *(float *)((char *)s + 0x5C) = 1.0f;
    *(float *)((char *)s + 0x60) = 1.0f;
    {
        float f = func_00213308();
        func_00214890(D_L00_00161B70, (char *)s + 0x50, f, (char *)s + 0x60);
        D_L00_00161B80 = f;
    }
    *(float *)&D_L00_00161B50 = 0.006f;
    D_L00_00161B54 = 0;
    D_L00_00161B58 = 0;
    if (*(int *)&D_L00_00161B44 != 0) {
        *(int *)&D_L00_00161B44 = 0;
        for (i = 199; i >= 0; i--) *(int *)(D_L00_001E4FD0_x + i * 0x18 + 0x14) = 0;
    }
}

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
extern int func_001F9688(int);
extern float func_002132A8(float, float);
extern float func_001F96B0(float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern short D_L10_00161E74;
extern short D_L10_00161E78;
extern short D_L10_00161E7C;
extern short D_L10_00161E80;
extern short D_L10_00161E84;
extern short D_L10_00161E88;
extern short D_L10_00161E8C;
extern short D_L10_00161E90;
extern short D_L10_00161E94;
extern short D_L10_00161E98;
extern short D_L10_00161E9C;
extern short D_L10_00161EA0;
extern short D_L10_00161EA4;
extern short D_L10_00161EA8;
extern short D_L10_00161EAC;
extern short D_L10_00161EB0;
extern short D_L10_00161EB4;
extern short D_L10_00161EB8;
extern short D_L10_00161EBC;
extern void func_L00_0025F8E0(float *v, float s);
extern float func_00213308(void);
extern void func_001F9A68(void *, void *, float);
extern int func_001FA6E0(int, int, float);
extern char *func_00218888(void *, void *, void *, int, int, int, int, int, int);



// Per-frame update of the moby in arg0: builds two vectors from its position and the trig helpers, then calls the level's helpers for each of its entries.
void FUN_L10_002e3a90(void *p) {
    char *arg0 = p;
    float v[4];
    float a[4];
    float b[4];
    float o[4];
    char *d = *(char **)(arg0 + 0x78);
    float f20;
    float f21;
    float f22;
    float r;
    int n = *(int *)&D_L10_00161E74;
    int i;
    int j;
    int k;
    int x16, x17, x18, x19, r7;
    for (i = 0; i < n; i++) {
        { float *dst = v; char *src = arg0 + 0x10; float *d2 = dst; qcopy_nc(d2, src); }
        func_L00_0025F8E0(v, (*(float *)&D_L10_00161E7C));
        v[2] = v[2] + 0.55f;
        f22 = func_00213308();
        f20 = func_002132A8((*(float *)&D_L10_00161E88), (*(float *)&D_L10_00161E8C));
        f21 = func_002132A8((*(float *)&D_L10_00161E90), (*(float *)&D_L10_00161E94));
        if (((unsigned char)arg0[0x20]) == 0xB) {
            f20 = f20 * 0.55f;
            f21 = f21 * 0.55f;
            v[2] = v[2] - 0.2f;
        }
        r = func_001F9DC8(f22);
        a[0] = r * (f20 * D_0015ED6C);
        r = func_001F9DE0(f22);
        a[1] = r * (f20 * D_0015ED6C);
        a[2] = (f21 * D_0015ED6C) + 0.0f;
        func_001F9A68(o, d + 0x70, 0.5f);
        o[2] = 0.0f;
        func_001F9A10(a, a, o);
        qcopy(b, a);
        k = func_001F9688((*(int *)&D_L10_00161EB8));
        b[2] = b[2] - ((*(float *)&D_L10_00161E98) * D_0015ED70) * (float)k;
        for (j = 0; j < (*(int *)&D_L10_00161E78); j++) {
            r = func_002132A8(0.5f, 1.5f);
            a[3] = r * *(float *)&D_L10_00161E9C;
            b[3] = r * *(float *)&D_L10_00161EA0;
            func_L00_0025F8E0(v, (*(float *)&D_L10_00161E80));
            func_L00_0025F8E0(b, (*(float *)&D_L10_00161E84) * D_0015ED6C);
            r = func_002132A8(0.0f, 1.0f);
            x19 = func_001FA6E0((*(int *)&D_L10_00161EA4), (*(int *)&D_L10_00161EAC), r);
            r = func_002132A8(0.0f, 1.0f);
            x18 = func_001FA6E0((*(int *)&D_L10_00161EA8), (*(int *)&D_L10_00161EB0), r);
            x17 = func_001F9688((*(int *)&D_L10_00161EB4));
            x16 = func_001F9688((*(int *)&D_L10_00161EB8));
            r = func_002132A8((float)(*(int *)&D_L10_00161EBC) * 0.5f, (float)(*(int *)&D_L10_00161EBC) * 2.5f);
            r = func_001F96B0(r);
            r7 = func_001FA6D0(r);
            func_00218888(v, a, b, x19, x18, x17, x16, r7, -1);
        }
    }
}

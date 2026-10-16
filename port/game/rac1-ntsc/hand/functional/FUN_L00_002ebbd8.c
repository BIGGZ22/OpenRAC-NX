/* Carried from the PAL decompilation's src/overlays/shared/vendor_002EB0D8.c (matched C there) (games/rac1) through the version
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
/* clears the 7 words of the state block, calls the helper, clears halfword 0x7E. Adapted from Lombyte (MIT) for PAL: overlays/shared/gameplay_vendor_002e9010.c, FUN_L00_002e9ef0. */
void func_L00_002E9EF0(p)
    char *p;
void func_L00_001FF500(void *, void *, float);
typedef struct { char pad[0x254]; int p; char pad2[0x18]; short s; char pad3[1]; char c; char pad4[0x14]; float f0; char pad5[8]; float f1; } S_c2ebbd8;
extern S_c2ebbd8 D_L00_00166C80;
extern void func_L00_002E9EF0(char *);
extern void func_001F9A28(void *, void *, void *);
extern void func_001F99F8(void *);
extern float func_L00_001FF8B0(float, float);
extern char D_0013E533[];
extern int D_0015ED84 MACRO_ADDR;
extern int func_001F9688(int);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);



/* Sets up the camera activation moby: tuning floats, then the pose copied from the current camera. */
void FUN_L00_002ebbd8(void *arg) {
    char *m = arg;
    char *g;
    int lvl0 = D_0015ED84;
    int lvl;
    char *p = *(char **)(m + 0x70) + 0x40;
    float f;
    char *t, *q, *r, *w, *s19, *src;
    char *gg, *gg0, *h;
    *(int *)(p + 0x40) = 0;
    f = 1.5f;
    if (lvl0 == 0xE && (h = D_0013E533 + 0xE1D, (unsigned)(*(int *)(h + 0x2084) - 0x2C) < 2))
        *(float *)(p + 0x4C) = 2.0f;
    else
        *(float *)(p + 0x4C) = 1.5f;
    g = D_0013E533 + 0xE1D;
    *(float *)(p + 0x48) = 0.20943951606750488f;
    *(float *)(p + 0x58) = 0.001745329238474369f;
    *(float *)(p + 0x5C) = -0.004363323096185923f;
    *(int *)(p + 0x44) = *(int *)(g + 0x2080);
    {
        char *q0 = *(char **)(m + 0x70);
        *(float *)(q0 + 0xA0) = 4.639999866485596f;
        q0 += 0xA0;
        *(float *)(q0 + 4) = 0.5f;
        *(float *)(q0 + 0x10) = 2.0f;
        *(float *)(q0 + 0x14) = 0.05f;
    }
    r = *(char **)(m + 0x70);
    *(int *)(r + 0x18) = 0;
    *(float *)(r + 0x10) = 0.01f;
    *(float *)(r + 0x14) = 0.3f;
    *(short *)(r + 0x1E) = func_001F9688(0x78);
    *(short *)(r + 0x1C) = 0;
    func_001F99F8(r);
    q = *(char **)(m + 0x70);
    t = q + 0x30;
    *(float *)(t + 4) = 0.02f;
    *(float *)(t + 8) = 0.2f;
    *(int *)(t + 0xC) = 0;
    *(int *)(q + 0x30) = 0;
    s19 = *(char **)(m + 0x70);
    w = s19 + 0xB8;
    *(int *)(w + 4) = 0;
    gg0 = (char *)&D_L00_00166C80;
    src = *(char **)(gg0 + 0x184);
    qcopy(m + 0x30, src + 0x30);
    qcopy(m, src);
    qcopy(m + 0x10, src + 0x10);
    qcopy(m + 0x20, src + 0x20);
    qcopy(m + 0x40, m);
    *(short *)(m + 0x7E) = 0;
    lvl = *(int *)(g + 0x2084);
    if ((unsigned)(lvl - 0x2C) < 2) {
        *(float *)(p + 0x30) = func_001F9DC8(*(float *)(g + 0x9AC));
        *(float *)(p + 0x34) = func_001F9DE0(*(float *)(g + 0x9AC));
        *(int *)(p + 0x38) = 0;
        *(float *)(s19 + 0xB8) = *(float *)(g + 0x9AC);
        *(int *)(w + 8) = *(int *)(g + 0x994);
    } else {
        func_001F9A28(p + 0x30, *(char **)(g + 0x964) + 0x10, m + 0x30);
        *(int *)(p + 0x38) = 0;
        *(int *)(p + 0x3C) = 0;
        func_L00_001FF500(p + 0x30, p + 0x30, 1.0f);
        *(float *)(s19 + 0xB8) = func_L00_001FF8B0(*(float *)(p + 0x30), *(float *)(p + 0x34));
        *(int *)(w + 8) = *(int *)(g + 0x964);
    }
    gg = (char *)&D_L00_00166C80;
    *(char *)(gg + 0x273) = 2;
    *(short *)(gg + 0x270) = 1;
    *(int *)(gg + 0x2F4) = func_001F9688(0x3C);
}

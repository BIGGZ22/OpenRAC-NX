/* Carried from the PAL decompilation's src/overlays/shared/partupd_0026A130.c (matched C there) (games/rac1) through the version
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
extern int func_001FA6E0(int, int, float);
extern void func_001F9A10(void *, void *, void *);
extern int func_001F9770(void *);
extern int func_001F9688(int);
extern void func_L00_00267A08(void *);
extern float func_001FA6C0(int);
extern void func_L00_001FF500(void *, void *, float);
extern float D_0015ED70 MACRO_ADDR;
extern short D_L00_001601B8;
extern short D_L00_001601D8;
/* NON_MATCHING FUN_L00_0026b500 -- src/overlays/shared/partupd_0026A130.c
 * Best so far: SIZE ours 660 / retail 656, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_0026b500 (PartType12Update): per-frame update of a particle; fades/scales it from a blend ratio = FA8
 *   p3.c is one instruction over (660 vs 656): retail keeps the blend ratio in $f12 (div.s $f12,$f20,$f0) with lo 
 *   Tried: ratio inline vs via locals a/b, lo reassigned vs a separate x, declaration orders. Would need whatever 
 */
extern unsigned char D_0013E05A[];
extern float D_0015ED6C MACRO_ADDR;
extern short D_L00_001601E8;
extern short D_L00_001601EC;
extern short D_L00_001601DC;
extern short D_L00_001601BC;
extern float func_001F9AF0(void *);
extern float func_00213F38(float *, float *, float, float, float, float);



/* per-frame update of a spinning particle: fades, scales and kills it when out of range */
void FUN_L00_0026b500(unsigned char *m) {
    unsigned char *p = m + 0x20;
    float r;
    float w = 1.0f;
    float ratio;
    m[8] = m[8] + p[0x1E];
    if (p[0x1F] & 4) {
        w = func_001FA6C0(D_0013E05A[0x4D6]) * 0.5f + w;
    }
    func_001F9A10(m + 0x10, m + 0x10, p);
    r = -func_001F9AF0(p);
    func_00213F38((float *)(m + 0x30), &r, 0.0f, 0.0f, *(float *)&D_L00_001601E8 * D_0015ED70, r);
    func_L00_001FF500(p, p, -r);
    *(float *)(m + 0x18) = *(float *)(m + 0x18) + *(float *)&D_L00_001601EC * D_0015ED6C;
    ratio = func_001FA6C0(*(short *)(m + 0xA)) / func_001FA6C0(p[0x1D]);
    if (p[0x1F] & 1) {
        float lo = *(float *)&D_L00_001601DC * w;
        *(float *)(m + 0xC) = ((*(float *)&D_L00_001601D8 - lo) * ratio + lo) * 210000.0f;
    } else {
        float lo = *(float *)&D_L00_001601BC * w;
        *(float *)(m + 0xC) = ((*(float *)&D_L00_001601B8 - lo) * ratio + lo) * 210000.0f;
    }
    *(int *)(m + 4) = func_001FA6E0(*(int *)(p + 0x18), *(int *)(p + 0x14), ratio);
    if (*(short *)(m + 0xA) < func_001F9688(15)) {
        float q = func_001FA6C0(*(short *)(m + 0xA));
        float t = q / func_001FA6C0(func_001F9688(10));
        *(int *)(m + 4) = func_001FA6E0(*(int *)(m + 4) & 0xFFFFFF, *(int *)(m + 4), t);
    }
    if (*(float *)(m + 0x10) < 2.0f || *(float *)(m + 0x10) > 1021.0f ||
        *(float *)(m + 0x14) < 2.0f || *(float *)(m + 0x14) > 1021.0f ||
        *(float *)(m + 0x18) < 2.0f || *(float *)(m + 0x18) > 1021.0f ||
        func_001F9770(m + 0xA) != 0) {
        func_L00_00267A08(m);
    } else if (*(float *)(p + 0x10) < 0.01f) {
        *(short *)(m + 0xA) = *(short *)(m + 0xA) - 1;
    }
}

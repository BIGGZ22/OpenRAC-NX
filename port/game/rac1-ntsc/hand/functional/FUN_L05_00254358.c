/* Carried from the PAL decompilation's src/overlays/shared/help_00237B00.c (matched C there) (games/rac1) through the version
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
extern unsigned char D_0013E533[] NOT_SDA;
extern float func_001F99C0(float);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern float func_001F9B80(void *, void *);
extern int D_0015EE84_2523C8 __asm__("D_0015ED84") MACRO_ADDR;
extern char *D_L05_001B0CB0_254B38[] __asm__("D_L05_001B0930");
extern float func_00213508(void *, int, float);
extern void func_001F99F8(void *);



/* Finds the nearest path node ahead of the hero (behind the facing test) and returns its position and heading. */
void FUN_L05_00254358(float *out, float *rot) {
    char *g = (char *)D_0013E533 + 0xE1D;
    char *m = *(char **)(g + 0x86C);
    char *path;
    float distance;
    int i, best;
    float tmp[4] __attribute__((aligned(16)));

    if (m == 0) {
        qcopy(out, g + 0x80);
        qcopy(rot, g + 0x90);
        return;
    }
    path = D_L05_001B0CB0_254B38[*(int *)(*(char **)(m + 0x78) + 0x24)];
    distance = 9999999.0f;
    best = 0;
    for (i = 0; i < *(int *)path; i += 2) {
            char *h = (char *)D_0013E533 + 0xE1D;
            char *p = path + i * 16;
            float limit = 4.0f;
            float angle = func_L00_001FF8B0(*(float *)(p + 0x10) - *(float *)(h + 0x80), *(float *)(p + 0x14) - *(float *)(h + 0x84));
            int n2 = *(int *)path + 2;
            int next = (i + n2) % *(int *)path;
            char *q = path + next * 16;
            if (func_001FA688(angle, func_L00_001FF8B0(*(float *)(q + 0x10) - *(float *)(p + 0x10), *(float *)(q + 0x14) - *(float *)(p + 0x14))) > 1.5707964f) {
                float d;
                if (D_0015EE84_2523C8 == 0x10 && *(char **)(h + 0x8B4) != 0) {
                    char *verts = *(char **)(h + 0x8B4);
                    char *v1 = verts + *(short *)(h + 0x8C8) * 16;
                    char *v2 = verts + *(short *)(h + 0x898) * 16;
                    float z1 = *(float *)(v1 + 0x18);
                    float z2 = *(float *)(v2 + 0x18);
                    if (z1 < z2) {
                        if (func_001F99C0(z2 - *(float *)(p + 0x18)) > limit)
                            goto next_point;
                    } else {
                        if (func_001F99C0(z1 - *(float *)(p + 0x18)) > limit)
                            goto next_point;
                    }
                }
                d = func_001F9B80(D_0013E533 + 0xE9D, path + i * 16 + 0x10);
                if (d < distance) {
                    distance = d;
                    best = i;
                }
            }
        next_point:;
    }
    qcopy(out, path + best * 16 + 0x10);
    qcopy(tmp, out);
    tmp[2] += 2.0f;
    out[2] = func_00213508(tmp, 0, 0.5f);
    func_001F99F8(rot);
    {
        float *a = (float *)(path + best * 16);
        float *b = (float *)(path + (best + 1) * 16);
        rot[2] = func_L00_001FF8B0(b[4] - a[4], b[5] - a[5]);
    }
}

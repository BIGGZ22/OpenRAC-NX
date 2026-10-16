/* Carried from the PAL decompilation's nonmatching/shared/func_L08_00318468.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L08_00316fa8 -- src/overlays/shared/vendor_002D3DF8.c
 * Best so far: SIZE ours 644 / retail 648, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 */
typedef int Q_318468_c316fa8 __attribute__((mode(TI)));
extern char D_L08_00167500_w[] __asm__("D_L08_00167480");
extern char D_L08_00167640_w[] __asm__("D_L08_001675C0");
extern short D_L08_001625E8;
extern float D_0015EE7C_w __asm__("D_0015ED7C") MACRO_ADDR;
extern char D_0013E633_w[] __asm__("D_0013E533");
extern void func_00214890(void *dst, void *vec, void *axis, float angle);
extern void func_001F9A10(void *, void *, void *);
extern float func_L00_00263468(void *);
extern int func_L00_0028DF38(int, int);
extern int func_L00_0028DDD8(int, int, int, int);

/* Wind ambience: probes 20 points around the camera; the more of them are below the camera (open
 * terrain) the louder the wind loop gets (eased volume 0..1), keeping the loop playing at the camera. */
void FUN_L08_00316fa8(char *m) {
    float p[4];
    float off[4];
    float axis[4];
    char *pos = D_L08_00167500_w + 0x140;
    char *d = *(char **)(m + 8);
    float ang = 0.0f;
    float open = ang;
    off[0] = 4.0f;
    axis[2] = 1.0f;
    off[1] = open;
    off[2] = open;
    off[3] = 1.0f;
    axis[0] = open;
    axis[1] = open;
    do {
        func_00214890(p, off, axis, ang);
        func_001F9A10(p, pos, p);
        if (func_L00_00263468(p) < *(float *)(D_L08_00167500_w + 0x148)) open += 0.05f;
        ang += 0.31415927f;
    } while (ang < 6.2831855f);
    if (*(float *)&D_L08_001625E8 < open) {
        *(float *)&D_L08_001625E8 += D_0015EE7C_w * 0.2f;
    } else {
        *(float *)&D_L08_001625E8 -= D_0015EE7C_w * 0.75f;
    }
    if (1.0f < *(float *)&D_L08_001625E8) {
        *(float *)&D_L08_001625E8 = 1.0f;
    } else if (*(float *)&D_L08_001625E8 < 0.0f) {
        *(float *)&D_L08_001625E8 = 0.0f;
    }
    {
        char *e = D_0013E633_w + 0x1D + *(int *)(d + 8) * 0x70;
        if (*(char **)(e + 0x8C) == m && ((unsigned char *)e)[0x74] != 0) {
            func_L00_0028DF38(*(int *)(d + 8),
                              (int)((*(float *)(d + 4) + *(float *)&D_L08_001625E8) / (*(float *)(d + 4) + 1.0f) * 1024.0f));
        }
    }
    qcopy(D_0013E633_w + 0xAD + *(int *)(d + 8) * 0x70, pos);
    {
        char *e = D_0013E633_w + 0x1D + *(int *)(d + 8) * 0x70;
        if (*(char **)(e + 0x8C) != m || ((unsigned char *)e)[0x74] == 0) {
            *(int *)(d + 8) = func_L00_0028DDD8(*(int *)d, 0x15, (int)m, 0x400);
        }
    }
}

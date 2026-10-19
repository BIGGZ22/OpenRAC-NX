/* Carried from the PAL decompilation's src/overlays/shared/vendor_002D3DF8.c (matched C there) (games/rac1) through the version
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
extern void vadd_3DF8(void *, void *, void *) __asm__("func_001F9A10");
extern char hero_pos_3DF8[] __asm__("D_L08_001675C0");
extern char D_0013E533[];
extern float D_0015ED7C MACRO_ADDR;
typedef struct { char p0[0x148]; float f148; char p14C[8]; float f154; float f158; } Cam_3091B0_c316fa8;
extern float func_L00_002644E0_3091B0(void *) __asm__("func_L00_00263468");
extern short D_L08_001625E8;
extern void func_00214890(void *dst, void *vec, void *axis, float angle);
/* NTSC-U calls 0x296230 here (jal at 0x31718C): FUN_L01_002a1968, the setter of the entry's word 0x80, not
 * FUN_L00_0028df38 (0x296250 in this level, word 0x84). */
extern int func_L01_002A1968(int, int);
extern int func_L00_0028DDD8(int a, int b, int c, int d);



/* Wind ambience: probes 20 points around the hero; the more of them are below the camera, the louder the loop. */
void FUN_L08_00316fa8(char *m) {
    float p[4];
    float off[4];
    float axis[4];
    char *d = *(char **)(m + 8);
    float ang = 0.0f;
    float open = ang;
    off[0] = 4.0f;
    off[1] = open;
    off[2] = open;
    off[3] = 1.0f;
    axis[0] = open;
    axis[1] = open;
    axis[2] = 1.0f;
    do {
        func_00214890(p, off, axis, ang);
        vadd_3DF8(p, hero_pos_3DF8, p);
        if (func_L00_002644E0_3091B0(p) < ((Cam_3091B0_c316fa8 *)(hero_pos_3DF8 - 0x140))->f148) open += 0.05f;
        ang += 0.31415927f;
    } while (ang < 6.2831855f);
    if (*(float *)&D_L08_001625E8 < open) {
        *(float *)&D_L08_001625E8 += D_0015ED7C * 0.2f;
    } else {
        *(float *)&D_L08_001625E8 -= D_0015ED7C * 0.75f;
    }
    if (1.0f < *(float *)&D_L08_001625E8) {
        *(float *)&D_L08_001625E8 = 1.0f;
    } else if (*(float *)&D_L08_001625E8 < 0.0f) {
        *(float *)&D_L08_001625E8 = 0.0f;
    }
    {
        char *e = D_0013E533 + 0x1D + *(int *)(d + 8) * 0x70;
        if (*(char **)(e + 0x8C) == m && ((unsigned char *)e)[0x74] != 0) {
            func_L01_002A1968(*(int *)(d + 8),
                              (int)((*(float *)(d + 4) + *(float *)&D_L08_001625E8) / (*(float *)(d + 4) + 1.0f) * 1024.0f));
        }
    }
    qcopy_nc(D_0013E533 + 0xAD + *(int *)(d + 8) * 0x70, hero_pos_3DF8);
    {
        char *e = D_0013E533 + 0x1D + *(int *)(d + 8) * 0x70;
        if (*(char **)(e + 0x8C) != m || ((unsigned char *)e)[0x74] == 0) {
            *(int *)(d + 8) = func_L00_0028DDD8(*(int *)d, 0x15, (int)m, 0x400);
        }
    }
}

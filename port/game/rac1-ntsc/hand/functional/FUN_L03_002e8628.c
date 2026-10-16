/* Carried from the PAL decompilation's src/overlays/shared/vendor_00292AC0.c (matched C there) (games/rac1) through the version
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
extern int D_L03_0015EF50 MACRO_ADDR;
extern void func_001F97B0(void);
extern void func_001FA050(void *, void *);
typedef struct { char pad0[0x40]; int x40; char pad1[8]; float x4C; } CamSub_c2e8628;
typedef struct { char pad0[4]; float f4; float f8; int fC; CamSub_c2e8628 sub; char pad1[0x20]; int x80; } CamInfo_c2e8628;



void FUN_L03_002e8628(char *cam) {
    float v[4] __attribute__((aligned(16)));
    float mat[16] __attribute__((aligned(16)));
    CamSub_c2e8628 *a = &(*(CamInfo_c2e8628 **)(cam + 0x70))->sub;
    CamInfo_c2e8628 *d;
    char *ent;
    int idx;
    char *p;
    a->x4C = 1.5f;
    a->x40 = 0;
    (*(CamInfo_c2e8628 **)(cam + 0x70))->x80 = 0;
    d = *(CamInfo_c2e8628 **)(cam + 0x70);
    d->f4 = 0.02f;
    d->f8 = 0.1f;
    d->fC = 0;
    idx = *(short *)(cam + 0x84);
    ent = (char *)D_L03_0015EF50 + (idx << 5);
    p = *(char **)(ent + 0x1C);
    if (idx < 0) {
        func_001F97B0();
    } else {
        a->x4C = *(float *)(p + 0x18);
    }
    qcopy(cam + 0x30, ent);
    qcopy(v, ent + 0x10);
    func_001FA050(mat, v);
    qcopy(cam, mat);
    qcopy(cam + 0x10, mat + 4);
    qcopy(cam + 0x20, mat + 8);
    qcopy(cam + 0x40, cam);
    *(short *)(cam + 0x7E) = 0;
}

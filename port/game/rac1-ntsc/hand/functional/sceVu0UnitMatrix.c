/* Carried from the PAL decompilation's nonmatching/core/func_00125358.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * The two versions' code differs here: func_00125358 (called) was paired by place; read it against NTSC-U before trusting it. */
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
/* Stores four 16-byte rows at m: the (0,0,0,1) vector in vf4 and its
 * vmr32 rotations (y,z,w,x) in vf5, vf6, vf7. Row 3 (+0x30) is written
 * first, row 0 (+0x00) last. */
void sceVu0UnitMatrix(float *m)
{
    char *p = (char *)m;
    float *r;

    r = (float *)(p + 0x30);
    r[0] = 0.0f; r[1] = 0.0f; r[2] = 0.0f; r[3] = 1.0f;
    r = (float *)(p + 0x20);
    r[0] = 0.0f; r[1] = 0.0f; r[2] = 1.0f; r[3] = 0.0f;
    r = (float *)(p + 0x10);
    r[0] = 0.0f; r[1] = 1.0f; r[2] = 0.0f; r[3] = 0.0f;
    r = (float *)(p + 0x00);
    r[0] = 1.0f; r[1] = 0.0f; r[2] = 0.0f; r[3] = 0.0f;
}

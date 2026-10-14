/* Carried from the PAL decompilation's src/game/draw.c (matched C there) (games/rac1/pal) through the version
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
extern int D_0018CD00[];
extern int D_0013E500[];
extern float func_001FA6C0(int);
extern void func_001F2D98(void);

void FUN_001f33b8(int w, int h, float a2, float a3, float a4, float a5, float a6) {
    float *ctx = (float *)D_0018CD00;
    int hw = w >> 1;
    int hh = h >> 1;
    float fh;

    D_0013E500[0] = w;
    D_0013E500[1] = h;
    D_0013E500[2] = hw;
    D_0013E500[3] = hh;
    D_0013E500[4] = (0x800 - hw) << 4;
    D_0013E500[5] = (0x800 - hh) << 4;
    D_0013E500[6] = (hw + 0x800) << 4;
    D_0013E500[7] = (hh + 0x800) << 4;
    ctx[0x2C] = a2;
    ctx[0x28] = 32.0f;
    ctx[0x29] = 524288.0f;
    ctx[0x80] = func_001FA6C0(w) * 0.5f;
    fh = func_001FA6C0(h) * 0.5f;
    ctx[0x81] = fh;
    ctx[0x82] = ctx[0x80] * 4.0f;
    ctx[0x83] = fh * 4.0f;
    ctx[0x86] = a3;
    ctx[0x87] = a4;
    ctx[0x8A] = a5;
    ctx[0x8B] = a6;
    func_001F2D98();
}

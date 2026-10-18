/* functional: high. Checked against the US assembly (vclipw.xyz, then cfc2 CLIPFLAG & 0x3F): the same six
 * strict tests against |w|, no change. Bit order as the header states. */
/* Carried from the PAL decompilation's nonmatching/game/func_001F9B20.c (games/rac1) through the version
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
/* Clip test of the vector at a0 against |w| (vclipw.xyz vf1, vf1w), returning
 * the six flag bits of that test in the order +x, -x, +y, -y, +z, -z (the low
 * six bits of CLIPFLAG, which cfc2 vi18 & 0x3F reads). */
s32 FUN_001f9958(f32 *v)
{
    f32 w = v[3];
    s32 flags = 0;

    if (w < 0.0f)
        w = -w;
    if (v[0] > w)
        flags |= 1;
    if (v[0] < -w)
        flags |= 2;
    if (v[1] > w)
        flags |= 4;
    if (v[1] < -w)
        flags |= 8;
    if (v[2] > w)
        flags |= 16;
    if (v[2] < -w)
        flags |= 32;
    return flags;
}

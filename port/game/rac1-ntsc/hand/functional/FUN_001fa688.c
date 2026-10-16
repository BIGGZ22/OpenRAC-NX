/* Carried from the PAL decompilation's nonmatching/game/func_001FA850.c (games/rac1) through the version
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
/*
 * The unsigned angle between two angles a and b (radians): |a - b|, or 2*pi - |a - b|
 * when that is the shorter way round (|a - b| >= pi).
 */
float FUN_001fa688(float a, float b)
{
    float d;

    d = __builtin_fabsf(a - b);
    if (!(d < 3.14159265f)) {
        d = (3.14159265f + 3.14159265f) - d;
    }
    return d;
}

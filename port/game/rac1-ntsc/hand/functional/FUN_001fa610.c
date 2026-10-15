/* Carried from the PAL decompilation's nonmatching/game/func_001FA7D8.c (games/rac1) through the version
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
/* Wraps an angle in radians into [-pi, pi): subtracts or adds 2*pi
   (two single-precision steps of pi each) until the value is in range. */
float FUN_001fa610(float x)
{
    const float pi = 3.14159274f;

    if (!(x < pi)) {
        do {
            x = x - pi;
            x = x - pi;
        } while (!(x < pi));
    }
    if (x < -pi) {
        do {
            x = x + pi;
            x = x + pi;
        } while (x < -pi);
    }
    return x;
}

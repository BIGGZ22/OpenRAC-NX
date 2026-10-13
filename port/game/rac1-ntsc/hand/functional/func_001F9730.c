/* Carried from the PAL decompilation's nonmatching/game/func_001F98F8.c (games/rac1/pal) through the version
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
/* Scale helper: returns D_0015ED70 * x (one float global, gp-relative). */
extern float D_0015ED70;

float func_001F9730(float x) {
    return D_0015ED70 * x;
}

/* Carried from the PAL decompilation's nonmatching/game/func_001F98B0.c (games/rac1) through the version
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
/* Scales its float argument by the small-data float at gp-0x7E98 (D_0015ED68). */
extern float D_0015ED68 MACRO_ADDR;

float func_001F96E8(float x)
{
    return D_0015ED68 * x;
}

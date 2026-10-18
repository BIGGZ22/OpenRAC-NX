/* functional: high. Checked against the US assembly (one min.s $f0, $f12, $f13, word 0x460D6029): same operands, same result. Equal inputs (only +0 and -0 can be equal and differ) are not told apart; no value changes. Carried from the PAL decompilation's nonmatching/game/func_001F9B98.c (games/rac1) through the version
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
/* Returns the smaller of its two float arguments (EE min.s). */
float FUN_001f99d0(float a, float b)
{
    return a < b ? a : b;
}

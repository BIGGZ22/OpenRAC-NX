/* functional: high. Checked against the US assembly (one max.s $f0, $f12, $f13, word 0x460D6028): same operands, same result. Equal inputs (only +0 and -0 can be equal and differ) are not told apart; no value changes. Carried from the PAL decompilation's nonmatching/game/func_001F9B90.c (games/rac1) through the version
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
/* Returns the larger of its two float arguments (EE max.s). */
float FUN_001f99c8(float a, float b)
{
    return a > b ? a : b;
}

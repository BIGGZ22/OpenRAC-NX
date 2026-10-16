/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001F39B0.c (games/rac1) through the version
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
/* functional: high. Returns 0 in $v0. Retail uses a trapping `addi $2, $0, 0x0`; the trap cannot fire for a constant, so plain C returning 0 behaves the same. */
int FUN_L08_002025d0(void) {
    return 0;
}

/* Carried from the PAL decompilation's nonmatching/game/func_L00_0024EBF8.c (games/rac1/pal) through the version
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
/* Spins while bit 0x100 of the word at the third argument (register $3 in retail: a flag word the caller points to) is set; returns when clear. */
void FUN_L00_0024dbc0(void *a0, void *a1, unsigned int *flag) {
    while ((*flag & 0x100) != 0) {
    }
}

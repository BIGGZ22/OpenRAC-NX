/* Carried from the PAL decompilation's nonmatching/game/func_L00_001F3988.c (games/rac1) through the version
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
/* FUN_L00_001f3608: footstep sound class (bits 5-6) of the last collision hit (CollOutput+0x1c); 0 when negative or class 3. */
extern int D_L00_00173E40[];
int FUN_L00_001f3608(void) {
    int t = D_L00_00173E40[7];
    if (t < 0) {
        return 0;
    }
    if ((t & 0x60) == 0x60) {
        return 0;
    }
    return (t & 0x60) >> 5;
}

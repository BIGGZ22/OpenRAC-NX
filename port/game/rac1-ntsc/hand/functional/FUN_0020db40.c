/* Carried from the PAL decompilation's nonmatching/game/func_L00_00251A78.c (games/rac1) through the version
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
extern char D_L00_001AB7E0[];

/* Clears n bits of the bitmap at D_L00_001AB7E0, from bit idx upward, one bit
   at a time (at least one). Retail checks each bit is set first and traps
   (teq) when it is already clear, storing nothing for that bit; this returns
   at that point instead. */
void FUN_0020db40(unsigned int idx, int n) {
    do {
        unsigned char *p = (unsigned char *)D_L00_001AB7E0 + (idx >> 3);
        unsigned int bit = 1u << (idx & 7);
        unsigned int old = *p;
        unsigned int nw = old & ~bit;
        if (nw == old) {
            return;
        }
        idx++;
        n--;
        *p = (unsigned char)nw;
    } while (n > 0);
}

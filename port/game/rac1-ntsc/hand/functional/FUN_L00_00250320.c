/* functional: high. Checked against the US assembly 2026-10-17: three bytes of the 64-bit word at 0x38 (bits 32, 40, 48) stored as 32-bit words, as the C does. */
/* Carried from the PAL decompilation's nonmatching/shared/func_L00_00251358.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L00_00250320 -- src/overlays/shared/mobyfunc_0024FD50.c
 * Best so far: SIZE ours 44 / retail 48, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_00250320: unpacks three bytes (bits 32-39, 40-47, 48-55) of the 64-bit word at moby+0x38 into three i
 *   Wall: retail uses $at ($1) as an ordinary register for dsrl32 results, which gcc never allocates; the original
 */
/* unpacks three bytes from the upper half of the moby's 64-bit word at 0x38 */
void FUN_L00_00250320(char *m, int *a, int *b, int *c) {
    unsigned long v = *(unsigned long *)(m + 0x38);
    *a = (unsigned char)(v >> 32);
    *b = (unsigned char)(v >> 40);
    *c = (unsigned char)(v >> 48);
}

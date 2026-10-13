/* Carried from the PAL decompilation's nonmatching/game/func_001FA480.c (games/rac1/pal) through the version
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
/* Copies three quadwords (48 bytes) from arg1 to arg0: three 128-bit loads, then three stores. */
void FUN_001fa2b8(void *arg0, void *arg1) {
    *(unsigned long long *)((char *)arg0 + 0x00) = *(unsigned long long *)((char *)arg1 + 0x00);
    *(unsigned long long *)((char *)arg0 + 0x08) = *(unsigned long long *)((char *)arg1 + 0x08);
    *(unsigned long long *)((char *)arg0 + 0x10) = *(unsigned long long *)((char *)arg1 + 0x10);
    *(unsigned long long *)((char *)arg0 + 0x18) = *(unsigned long long *)((char *)arg1 + 0x18);
    *(unsigned long long *)((char *)arg0 + 0x20) = *(unsigned long long *)((char *)arg1 + 0x20);
    *(unsigned long long *)((char *)arg0 + 0x28) = *(unsigned long long *)((char *)arg1 + 0x28);
}

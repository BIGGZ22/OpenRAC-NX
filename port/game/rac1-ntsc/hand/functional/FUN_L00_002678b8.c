/* functional: high. Checked against the US assembly 2026-10-17: a tail jump to FUN_L00_002678c0 with a1 = 0; that callee writes a1 before it reads anything, so dropping the zero changes nothing. */
/* Carried from port/game/rac1-pal/hand/game_particle_allocate.c through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* PAL's entry 8 bytes in (func_00218930) is this level's FUN_L00_002678c0 here. */
char *FUN_L00_002678c0(int type);



void* FUN_L00_002678b8(int type) {
    return FUN_L00_002678c0(type);
}

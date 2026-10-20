#include "include_asm.h"

extern char *D_L12_001BDB00;

extern float D_L12_001AD230;

extern float D_L12_001AD234;

INCLUDE_ASM("asm/levels", func_L12_002CF7B0);

INCLUDE_ASM("asm/levels", func_L12_002FFF88);

/* Refresh the two tuning factors, or zero them on the two states that ask for it. */
INCLUDE_ASM("asm/levels", func_L12_00300138);

INCLUDE_ASM("asm/levels", func_L12_00300410);

INCLUDE_ASM("asm/levels", func_L12_0035A838);

/* func_L12_00388038 is in the sibling file L12_002CF7B0-g8.c: its body
   reads a small-data global through $gp. */

INCLUDE_ASM("asm/levels", func_L12_00388090);

INCLUDE_ASM("asm/levels", func_L12_00389988);

INCLUDE_ASM("asm/levels", func_L12_00389BC0);

INCLUDE_ASM("asm/levels", func_L12_00389F80);

INCLUDE_ASM("asm/levels", func_L12_00393458);

INCLUDE_ASM("asm/levels", func_L12_00393870);

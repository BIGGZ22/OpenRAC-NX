#include "include_asm.h"

/* Built with -G8: the spawn counter at 0x1AC4D8 and the object table base at 0x1B251C are
   reached through $gp in retail, so those two bodies live here. */

extern int D_L05_001AC4D8 __attribute__((sda));

extern char *D_L05_001B251C __attribute__((sda));

extern int D_L05_00225DD0[];

extern int D_L05_00221D10[];

extern int D_L05_001FB220[];

extern char *func_379CA0(char *object);

INCLUDE_ASM("asm/levels", func_L05_0037AFF8);

/* Finds the free entry of the level list that belongs to id and spawns it there. */
INCLUDE_ASM("asm/levels", func_L05_0037B440);

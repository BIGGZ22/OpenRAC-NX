#include "include_asm.h"

extern int D_L12_001A8F00 __attribute__((nosda));

extern int D_L12_001BDB30 __attribute__((nosda));

extern char *D_L12_001B2EA8 __attribute__((sda));

/* Clear the two flag bits on every object of the chain while a mode gate holds.
   Near miss: nonmatching/levels/func_L12_00388038.c */
INCLUDE_ASM("asm/levels", func_L12_00388038);

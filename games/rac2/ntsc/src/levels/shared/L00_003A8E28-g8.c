#include "include_asm.h"

/* Small-data sibling of L00_003A8E28.c: the body below reaches the level's
   globals through $gp (the file gets -G8 by its name, see config/cflags.txt). */

extern char D_L00_0023BB78[];

extern unsigned char D_L00_001AF810 __attribute__((sda));

extern unsigned char D_L00_001AF811 __attribute__((sda));

extern void func_2EDE88();

/* Resets the two slot cursors, then releases the 256-entry slot table. */
void func_L00_003B09F8(void)
{
    D_L00_001AF811 = 0;
    D_L00_001AF810 = 0;
    func_2EDE88(D_L00_0023BB78, 0x1C00);
}

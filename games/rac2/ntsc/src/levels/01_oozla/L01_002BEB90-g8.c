#include "include_asm.h"

extern int D_L01_001B2088 __attribute__((sda));

extern int D_L01_001AEC38 __attribute__((sda));

extern unsigned char D_L01_001A7A6F;

extern int D_L01_001A907C;

extern void func_3558E0(int, int, int);

extern void func_31BEC0(int, int);

/* Fires the Oozla intro once the level's mode and counter allow it. */
INCLUDE_ASM("asm/levels", func_L01_00378A80);

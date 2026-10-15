#include "include_asm.h"

extern char D_00189E20[];

extern int D_001A79F0;

extern int D_001A73A0;

extern unsigned char D_00139568[];

extern char D_L22_002645F0[];

extern char D_L22_001B1920[];

extern void func_L00_002D24B8(void);

extern void func_L22_002C9898(int mode, int flag);

extern void func_L00_003B09F8(void);

extern int func_L00_002ADEA0(int index);

/* Loads the level's tuning constants and starts the two subsystems. */
INCLUDE_ASM("asm/levels", func_L22_002A8B98);

/* Picks the shop entry the level's table names for the current progress state. */
INCLUDE_ASM("asm/levels", func_L22_002A8CE0);

INCLUDE_ASM("asm/levels", func_L22_002AA480);

INCLUDE_ASM("asm/levels", func_L22_002AA678);

INCLUDE_ASM("asm/levels", func_L22_002AAA30);

INCLUDE_ASM("asm/levels", func_L22_002B70F8);

INCLUDE_ASM("asm/levels", func_L22_002B7328);

INCLUDE_ASM("asm/levels", func_L22_002B7C70);

INCLUDE_ASM("asm/levels", func_L22_002B8E10);

INCLUDE_ASM("asm/levels", func_L22_002C8610);

INCLUDE_ASM("asm/levels", func_L22_002C9898);

INCLUDE_ASM("asm/levels", func_L22_002CE468);

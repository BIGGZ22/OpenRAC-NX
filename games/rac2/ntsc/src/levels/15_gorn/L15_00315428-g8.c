#include "include_asm.h"

#include "common.h"

/* Small-data unit: the bodies whose state lives at $gp-relative offsets. */

extern int D_L15_001B1530 __attribute__((sda));
extern int D_L15_001B1534 __attribute__((sda));
extern int D_L15_001B15C4 __attribute__((sda));

extern unsigned char D_L15_00221E10[];
extern unsigned char D_L15_0029FD80[];

extern int func_80381310(int a0);
extern void func_80380770(int a0, void *a1);
extern void func_80383FB8(void);
extern void func_803807B0(void);

/* Publishes the level's frame budget: seeds the counters and points the stage at
   the record the factory returned, then runs the three stage hooks. */
INCLUDE_ASM("asm/levels", func_L15_0042FD40);

extern int D_L15_001B1520 __attribute__((sda));
extern int D_L15_001B1524;
extern int D_L15_001B1528;
extern int D_L15_001B152C;
extern u8 D_L15_001395B8[];

/* Copies the four clamped channel bytes into the render block's colour slots. */
INCLUDE_ASM("asm/levels", func_L15_00432008);

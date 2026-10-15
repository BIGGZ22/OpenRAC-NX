#include "include_asm.h"
#include "common.h"

extern void func_L00_00325AE8(void *, void *, float, float);

extern void *D_L02_001B17E8 __attribute__((sda));

extern float D_L02_001B17E4 __attribute__((sda));

extern float D_L02_001B17E0 __attribute__((sda));

extern float D_L02_001B17EC __attribute__((sda));

/* Runs the kind's placement helper with the kind's own angles. */
void func_L02_00402440(char *a0) {
    if (*(unsigned char *)(a0 + 0x20) == 3)
        func_L00_00325AE8(a0 + 0x10, D_L02_001B17E8, D_L02_001B17E4, D_L02_001B17E0);
    else if (*(unsigned char *)(a0 + 0x20) == 2)
        func_L00_00325AE8(a0 + 0x10, D_L02_001B17E8, D_L02_001B17EC, D_L02_001B17E0);
}

/* Near miss (best attempt in work/wave9-20261013/w01/drafts/func_L02_003D28E0.c): one
   instruction over retail, the `nop` this assembler puts after `mfc1`, plus a rotated
   register allocation for the two comparison operands. 240 bytes against 236. */
INCLUDE_ASM("asm/levels", func_L02_003D28E0);

/* Near miss (best attempt in work/wave9-20261013/w01/drafts/func_L02_004013C0.c): retail keeps
   a dead load of the second scalar on the taken side of the branch and a dead stack store;
   this compiler drops them (172 bytes against 188). */
INCLUDE_ASM("asm/levels", func_L02_004013C0);

extern char *D_L02_001B1190[1] __attribute__((sda));

extern float func_00283CE0(int);

extern int func_00283CF0(float);

extern char *func_L02_003F9B70(int);

extern float func_00288D58(void);

/* Near miss (best attempt in work/wave9-20261013/w01/drafts/func_L02_003FCDF8.c): the
   `$gp` base is reached with a one-element array declaration under -G8 (268 bytes against
   272, one instruction short) — retail keeps a second copy of the base (`daddu $s4,$s0`)
   and materialises the 0x1395B8 counter base once (`lui $s3,0x14`) across the loop, where
   this compiler re-uses the base register and rematerialises inside the loop. */
INCLUDE_ASM("asm/levels", func_L02_003FCDF8);

/* Near miss: same body with the other entry parameter (draft beside this one). */
INCLUDE_ASM("asm/levels", func_L02_003FCF08);

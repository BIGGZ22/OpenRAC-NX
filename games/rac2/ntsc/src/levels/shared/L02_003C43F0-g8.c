#include "include_asm.h"
#include "common.h"

extern void func_L00_00325AE8(void *, void *, float, float);

extern void *D_L02_001B17E8 __attribute__((sda));

extern float D_L02_001B17E4 __attribute__((sda));

extern float D_L02_001B17E0 __attribute__((sda));

extern float D_L02_001B17EC __attribute__((sda));

extern char *D_L02_001B1190[1] __attribute__((sda));

extern float func_00283CE0(int);

extern int func_00283CF0(float);

extern int func_L02_003F9B70(int);

extern int func_00288D58(void);

extern unsigned char D_001395B8[0x100];

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

/* The kind's table gives a count; the kind's own helper gives the kind's id. */

/* The byte table the count is read from, at its address in the boot's resident memory. */

/* Scales the kind's own angle at +0xC down by 0.3 once per count the kind's byte table
   gives, keeps it inside the kind's own range, scales it by ten when the global flag is
   set, and hands it to the angle setter. */
int func_L02_003FCDF8(int a0) {
    float f20;
    int i = 0;

    f20 = func_00283CE0(*(int *)(D_L02_001B1190[a0] + 0xC));
    while (i < D_001395B8[func_L02_003F9B70(a0)]) {
        f20 *= 0.3f;
        i++;
    }
    {
        float f0 = (float)*(int *)(D_L02_001B1190[a0] + 0xC);

        if (f0 < f20)
            f20 = f0;
        else if (f20 < 500.0f)
            f20 = 500.0f;
    }
    if (func_00288D58() != 0)
        f20 *= 10.0f;
    return func_00283CF0(f20);
}

/* Near miss: same body with the other entry parameter (draft beside this one). */
int func_L02_003FCF08(int a0) {
    float f20;
    int i = 0;

    f20 = func_00283CE0(*(int *)(D_L02_001B1190[a0] + 0x10));
    while (i < D_001395B8[func_L02_003F9B70(a0)]) {
        f20 *= 0.33333f;
        i++;
    }
    {
        float f0 = (float)*(int *)(D_L02_001B1190[a0] + 0x10);

        if (f0 < f20)
            f20 = f0;
        else if (f20 < 5.0f)
            f20 = 5.0f;
    }
    return func_00283CF0(f20) << 5;
}

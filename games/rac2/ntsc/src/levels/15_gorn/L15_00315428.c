#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L15_00315428);

extern void *func_803804E8(int a0);

/* Stores the second argument into the record the lookup returned. */
void func_L15_003804B8(int a0, void *a1)
{
    void *p = func_803804E8(a0);

    if (p != 0) {
        *(void **)((char *)p + 0x158) = a1;
    }
}

/* func_L15_00381CF0: attempt kept in drafts (232 bytes, 6 instructions over). */
INCLUDE_ASM("asm/levels", func_L15_00381CF0);

INCLUDE_ASM("asm/levels", func_L15_003AF878);

INCLUDE_ASM("asm/levels", func_L15_003DBB08);

INCLUDE_ASM("asm/levels", func_L15_0041F2F8);

INCLUDE_ASM("asm/levels", func_L15_0042DCE0);

INCLUDE_ASM("asm/levels", func_L15_0042E1B0);

INCLUDE_ASM("asm/levels", func_L15_0042EFE0);

/* func_L15_0042FD40: attempt kept in drafts (148 bytes, +0x4 differs). */
/* func_L15_0042FD40 is defined in L15_00315428-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L15_0042FFB8);

INCLUDE_ASM("asm/levels", func_L15_00430168);

INCLUDE_ASM("asm/levels", func_L15_00430590);

INCLUDE_ASM("asm/levels", func_L15_004307E0);

INCLUDE_ASM("asm/levels", func_L15_00430B30);

INCLUDE_ASM("asm/levels", func_L15_00430C80);

INCLUDE_ASM("asm/levels", func_L15_00430DF0);

INCLUDE_ASM("asm/levels", func_L15_00431590);

/* func_L15_00432008: attempt kept in drafts (176 bytes, 4 instructions over). */
/* func_L15_00432008 is defined in L15_00315428-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L15_004320B0);

extern unsigned char D_L15_004328E8[];

extern void func_L15_00432210(void *a0);
extern void func_802FFC98(void *a0, void *a1);
extern int func_803068A0(void *a0);
extern void func_80329048(void *a0);

/* Runs the level's per-frame update once the record is in its armed state. */
void func_L15_004321A8(char *p)
{
    char *q = *(char **)(p + 0x68);

    if (*(unsigned char *)(p + 0x20) == 1) {
        func_L15_00432210(p);
        func_802FFC98(D_L15_004328E8, p);
        if (func_803068A0(q + 0x54) != 0) {
            func_80329048(p);
        }
    }
}

INCLUDE_ASM("asm/levels", func_L15_00432210);

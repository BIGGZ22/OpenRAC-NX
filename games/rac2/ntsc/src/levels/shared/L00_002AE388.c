#include "include_asm.h"

typedef int s32;

extern void *func_L00_002A4E48(void *p);

extern void func_L00_002D2168(void *a, void *b, float f);

extern unsigned char D_00189E20_26[];

INCLUDE_ASM("asm/levels", func_L00_002AE388);

INCLUDE_ASM("asm/levels", func_L00_002AE5F0);

INCLUDE_ASM("asm/levels", func_L00_002AE888);

/* maps values one, two and three to identifiers 0x72, 0x70 and 0x6e */
s32 func_L00_002AEAC0(s32 value) {
 switch(value) {case 1:return 0x72;case 2:return 0x70;case 3:return 0x6e;default:return 0;}
}

INCLUDE_ASM("asm/levels", func_L00_002AEB18);

INCLUDE_ASM("asm/levels", func_L00_002AED68);

INCLUDE_ASM("asm/levels", func_L00_002AF3D8);

INCLUDE_ASM("asm/levels", func_L00_002AF568);

/* guards a fixed data block on an enable byte, takes one of two early exits (setting a pending word and a flag, or skipping), calls two leaves with a scaled value when a byte field matches a threshold word, then clears a byte pair of the block */
void func_L00_002AF7D0(void)
{
    if (D_00189E20_26[8888] == 0)
        return;
    if (*(int *)(D_00189E20_26 + 3736) != 0) {
        *(int *)(D_00189E20_26 + 3748) = 1;
    } else if (*(int *)(D_00189E20_26 + 8852) == 0) {
        if (*(unsigned char *)(*(int *)(D_00189E20_26 + 8848) + 67) == *(int *)(D_00189E20_26 + 9392)) {
            func_L00_002D2168(func_L00_002A4E48(0), 0, -2.0f);
        }
    }
    D_00189E20_26[8888] = 0;
    D_00189E20_26[8890] = 0;
}

INCLUDE_ASM("asm/levels", func_L00_002AF860);

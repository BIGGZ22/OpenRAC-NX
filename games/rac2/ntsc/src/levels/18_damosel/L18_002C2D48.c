#include "include_asm.h"

typedef unsigned char u8;
typedef short s16;

extern char D_00189E20[];
extern u8 D_00139568[];
extern char D_0026BCF0[];
extern int func_002CB1C8(int);

typedef struct {
    u8 pad[0x28];
    int v;
} WeaponEntry;

/* Screen-id lookup: a finished player mode wins, then a running screen whose two
   gates are clear returns the armed item's word, otherwise the health/state pair
   picks the fallback id. */
int func_L18_002C2D48(char *a0)
{
    char *s0 = D_00189E20;

    if (*(u8 *)(s0 + 0x22B4) == 4)
        return 0xD;
    if (a0 != 0 && *(u8 *)(s0 + 0x22BA) != 0 && *(s16 *)(s0 + 0x24EC) == 0) {
        int idx = func_002CB1C8(0);
        return ((WeaponEntry *)(D_0026BCF0 + D_00139568[idx] * 224))->v;
    }
    a0 = D_00189E20;
    if (*(int *)(a0 + 0x24CC) != 1)
        return *(u8 *)(a0 + 0x1492) ? 0x67 : 0;
    return 0x51;
}

INCLUDE_ASM("asm/levels", func_L18_002CA350);

INCLUDE_ASM("asm/levels", func_L18_002D01B0);

INCLUDE_ASM("asm/levels", func_L18_002D0850);

INCLUDE_ASM("asm/levels", func_L18_002D1E38);

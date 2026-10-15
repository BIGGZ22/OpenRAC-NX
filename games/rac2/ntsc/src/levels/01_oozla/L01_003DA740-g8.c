#include "include_asm.h"
#include "common.h"

extern char *D_L01_001B255C __attribute__((sda));

extern u16 *D_L01_001FB020[];

/* Raises the measured flag on every record of the table the object's selector names. */
void func_L01_003DC838(s32 a0) {
    s32 base;
    s16 *p;
    s16 *q;

    if (*(u8 *)(a0 + 0x21) == 0xFF)
        return;
    q = D_L01_001FB020[*(u8 *)(a0 + 0x21)];
    if (q == 0)
        return;
    p = q;
    base = (s32)D_L01_001B255C;
    do {
        u32 v = (*(u16 *)p & 0x7FFF) << 8;

        if (*(s8 *)(base + v + 0x20) >= 0) {
            if (*(s16 *)(base + v + 0xAA) == 0xB2B)
                *(u16 *)(*(u32 *)(base + v + 0x68) + 0x3DA) = 1;
        }
    } while (*p++ >= 0);
}

/* func_L01_00416A50 is defined in L01_003DA740.c (this unit has no $gp access for it). */

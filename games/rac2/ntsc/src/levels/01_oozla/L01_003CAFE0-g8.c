#include "common.h"

extern u8 *D_L01_001B255C;

extern u32 D_L01_001FB020[];

/* Walks the 0x7FFF-masked id list at table[index] and flags class 0xB2B entries. */
void func_L01_003D87B8(s32 index)
{
    u16 *p;
    u32 slot;
    u8 *base;
    u8 *e;

    if (index == -1)
        return;
    slot = D_L01_001FB020[index];
    if (slot == 0)
        return;
    p = (u16 *)slot;
    base = D_L01_001B255C;
    do {
        e = base + ((*p & 0x7FFF) << 8);
        if (*(s8 *)(e + 0x20) >= 0 && *(s16 *)(e + 0xAA) == 0xB2B)
            *(u16 *)(*(u8 **)(e + 0x68) + 0x3DA) = 1;
    } while (*(s16 *)p++ >= 0);
}


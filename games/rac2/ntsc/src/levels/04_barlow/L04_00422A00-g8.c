#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;

extern u8 *D_L04_001FB1E0[];
extern u8 *D_L04_001B24DC __attribute__((sda));

/* walks the entry's two-byte id list and flags each live object of one class */
void func_L04_0044D7E0(u8 *a0) {
    u8 *base;
    u8 *p;
    u16 *q;
    char *e;

    if (a0[0x21] == 0xFF)
        return;
    p = D_L04_001FB1E0[a0[0x21]];
    if (p == 0)
        return;
    q = (u16 *)p;
    base = D_L04_001B24DC;
    for (;;) {
        e = (char *)(base + (*q & 0x7FFF) * 256);
        if ((s8)e[0x20] >= 0) {
            if (*(s16 *)(e + 0xAA) == 0xFB6) {
                *(u16 *)(*(u32 *)(e + 0x68) + 0x306) = 1;
            }
        }
        if ((s16)*q < 0)
            break;
        q++;
    }
}

/* same walk, one class flag and a visit counter instead */
void func_L04_004510C8(u8 *a0) {
    u8 *base;
    u8 *p;
    u16 *q;
    char *e;

    if (a0[0x21] == 0xFF)
        return;
    p = D_L04_001FB1E0[a0[0x21]];
    if (p == 0)
        return;
    q = (u16 *)p;
    base = D_L04_001B24DC;
    for (;;) {
        e = (char *)(base + (*q & 0x7FFF) * 256);
        if ((s8)e[0x20] >= 0) {
            if (*(s16 *)(e + 0xAA) == 0xFF8) {
                char *o = *(char **)(e + 0x68);
                *(u16 *)(o + 0x276) = 1;
                *(u32 *)(o + 0x574) += 1;
            }
        }
        if ((s16)*q < 0)
            break;
        q++;
    }
}

#include "include_asm.h"

typedef unsigned char u8;

/* These bodies reach D_L03_001B2A1C with `lw $a2,0x3A2C($gp)`, which only a
 * -G8 unit emits; the same symbol is reached with an absolute `lui`/`lw` pair
 * inside func_L03_003F66D8 and func_L03_0041D128, so those stay in the -G0 file. */
extern u8 *D_L03_001B2A1C __attribute__((sda));

/* The stream table itself is `lui $v0,0x20 ; addiu $v0,$v0,-0x4A60` (absolute). */
extern u8 *D_L03_001FB5A0[];

/* Walks the entity's stream, storing 1 at +0x2D6 of every record whose halfword
 * at +0xAA is 0xFFC.  Twin of func_L01_003B5290 (same file layout, level 1). */
void func_L03_0041D0A8(u8 *entity) {
    u8 *list;
    u8 *p;
    u8 *base;

    if (entity[0x21] == 0xFF)
        return;
    list = D_L03_001FB5A0[entity[0x21]];
    if (list == 0)
        return;
    p = list;
    base = D_L03_001B2A1C;
    for (;;) {
        u8 *q = base + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0xFF3) {
                *(short *)(*(u8 **)(q + 0x68) + 0x2D6) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}

/* Same walk as func_L03_0041D0A8 after seeding two state halfwords. */
void func_L03_0041E240(u8 *entity) {
    u8 *state = *(u8 **)(entity + 0x68);
    u8 *list;
    u8 *p;
    u8 *base;

    *(short *)(state + 0x336) = 1;
    *(short *)(state + 0x330) = 0xF0;
    if (entity[0x21] == 0xFF)
        return;
    list = D_L03_001FB5A0[entity[0x21]];
    if (list == 0)
        return;
    p = list;
    base = D_L03_001B2A1C;
    for (;;) {
        u8 *q = base + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0xFFC) {
                *(short *)(*(u8 **)(q + 0x68) + 0x336) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}

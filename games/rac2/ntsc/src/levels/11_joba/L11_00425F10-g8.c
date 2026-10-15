#include "common.h"

extern void *D_L11_001FC420[];

/* Reached with `lw $a2,0x45AC($gp)`, so this unit is built -G8. */
extern u8 *D_L11_001B359C;

/* Flags every record of the id list whose class halfword is 0xD07. */
void func_L11_00431DB0(u8 *arg0) {
    void *list;
    u16 *p;
    u8 *base;

    if (arg0[0x21] == 0xFF)
        return;
    list = D_L11_001FC420[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    base = D_L11_001B359C;
    do {
        u8 *e = base + ((*p & 0x7FFF) << 8);

        if (*(s8 *)(e + 0x20) >= 0 && *(s16 *)(e + 0xAA) == 0xD07)
            *(s16 *)(*(u8 **)(e + 0x68) + 0x276) = 1;
    } while (*(s16 *)p++ >= 0);
}

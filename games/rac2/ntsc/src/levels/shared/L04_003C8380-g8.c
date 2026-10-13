#include "include_asm.h"

extern unsigned int D_L04_001B24DC;

extern int func_00282948(void *a0);

extern void func_L04_003F8FA0(unsigned char *a0);

extern void func_L04_003F9078(unsigned char *a0);

void func_L04_003F9270(unsigned char *a0) {
    unsigned char *obj = *(unsigned char **)(a0 + 0x68);
    int idx = *(int *)(obj + 8);
    unsigned char *entry;

    if (idx != -1) {
        entry = (unsigned char *)((idx << 8) + D_L04_001B24DC);
        if ((unsigned int)(entry[0x20] - 2) >= 2) {
            return;
        }
    }
    if (a0[0x20] == 0) {
        *(int *)obj = 0;
        a0[0x30] = 0xFF;
        *(unsigned short *)(a0 + 0x32) = 0x7F;
        a0[0x20] = 1;
        func_L04_003F8FA0(a0);
    }
    if (func_00282948(obj) != 0) {
        func_L04_003F9078(a0);
        *(unsigned short *)(a0 + 0x34) &= 0xFFBE;
    } else {
        *(unsigned short *)(a0 + 0x34) |= 0x41;
    }
}

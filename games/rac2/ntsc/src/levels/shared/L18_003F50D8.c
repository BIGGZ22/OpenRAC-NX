#include "include_asm.h"

extern int D_L18_001A79F0;
extern int D_L18_001A8F00;
extern void func_L18_003F5198(void *);
extern void func_L18_003F53D8(void *);
extern void func_L18_003F5650(void *);
extern void func_L18_003F6CA8(void *);
extern void func_L18_003F7198(void *);

void func_L18_003F50D8(char *a0)
{
    char *s0 = a0;
    unsigned char v;

    if (D_L18_001A79F0 == 0x13) {
        if (D_L18_001A8F00 == 2) {
            *(int *)(s0 + 0x98) = 0;
            *(unsigned short *)(s0 + 0x34) = (*(unsigned short *)(s0 + 0x34) | 0x41) & 0xEFFF;
        } else {
            char *p = *(char **)(s0 + 0x24);

            *(unsigned short *)(s0 + 0x34) = (*(unsigned short *)(s0 + 0x34) & 0xFFBE) | 0x1000;
            *(int *)(s0 + 0x98) = *(int *)(p + 0x10);
        }
    }
    v = *(unsigned char *)(s0 + 0x20);
    if (v == 0) {
        func_L18_003F5198(s0);
    } else if (v != 1 && v != 0x13) {
        func_L18_003F53D8(s0);
        func_L18_003F5650(s0);
        func_L18_003F6CA8(s0);
        func_L18_003F7198(s0);
    }
}

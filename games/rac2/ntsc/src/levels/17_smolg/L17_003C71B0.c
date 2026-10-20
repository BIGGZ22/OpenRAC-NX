#include "include_asm.h"

typedef char *ptr;
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;

extern s32 func_L03_003268C0(void *, s32, s32, s32, s32, s32, s32, void *);
extern s32 func_002E5CF8(s32, s32, void *);
extern char D_L17_001A8F20[];

void func_L17_003C71B0(char *a0)
{
    char *s0 = a0;

    if (*(s16 *)(s0 + 0xAA) == 0x929 && *(u8 *)(s0 + 0x20) == 0) {
        func_L03_003268C0(s0, 0x931, 0, 0x931, 4, 6, 0, D_L17_001A8F20);
        func_002E5CF8(0, 0, s0);
        *(u16 *)(s0 + 0x34) = *(u16 *)(s0 + 0x34) | 0x41;
        *(s32 *)(s0 + 0x98) = 0;
        if (s0 != 0) {
            if (*(u8 *)(s0 + 0x20) != 0xFE) {
                if (*(u8 *)(s0 + 0x20) != 0xFD)
                    *(u8 *)(s0 + 0x20) = 1;
            }
        }
    }
}

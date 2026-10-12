#include "common.h"

extern void func_002A7750(u8 *arg0, s32 arg1, s32 arg2);

extern s32 func_002A77E0(s32 arg0);

extern s32 func_L11_00440C80(s32 arg0);

extern s32 D_L11_001B056C __attribute__((sda));

extern s32 D_L11_001B058C __attribute__((sda));

/* Retires the object and copies two measured state bytes into its state block. */
void func_L11_003F2D38(u8 *arg0) {
    u8 *s0 = arg0;
    s32 v1 = *(s32 *)(s0 + 0x68);
    if (*(u16 *)(v1 + 0x278) >= 2)
        return;
    if (*(u16 *)(v1 + 0x27A) >= 2)
        return;
    if (func_002A77E0(D_L11_001B058C) != 0
        && func_L11_00440C80(*(s32 *)(s0 + 0xB8)) < D_L11_001B056C) {
        func_002A7750(s0, 7, -1);
    } else {
        func_002A7750(s0, 6, -1);
    }
}

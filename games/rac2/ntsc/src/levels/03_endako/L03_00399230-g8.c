#include "common.h"

extern f32  D_L03_001B0340 __attribute__((sda));
extern void func_002A7750(char *a0, s32 a1, s32 a2);
extern s32  func_31BA30(s32 a0, s32 a1);
extern s32  func_31B9F0(s32 a0);
extern f32  func_2F0000(f32 a0, f32 a1);

void func_L03_003CA938(char *a0)
{
    if (*(u8 *)(a0 + 0x31) == 0) {
        return;
    }
    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(u8 *)(a0 + 0xBC) = (u8)func_31BA30(5, 10);
        if (func_31B9F0(2) != 0) {
            *(u8 *)(a0 + 0xBC) += 15;
        }
        func_002A7750(a0, 1, -1);
        break;
    case 1:
        *(f32 *)(a0 + 0xF8) = func_2F0000(*(f32 *)(a0 + 0xF8),
            D_L03_001B0340 * 0.000290888245f * (f32)(*(u8 *)(a0 + 0xBC) - 15));
        break;
    }
}

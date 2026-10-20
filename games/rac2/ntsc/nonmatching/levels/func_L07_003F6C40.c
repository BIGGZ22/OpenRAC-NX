/* func_L07_003F6C40, 172 bytes, 1 placement.  Nearest: 168 bytes, two words.
   Arms the level-7 object: stores the integer argument at a0 + 0xB8, copies the 16 bytes at
   a2 to a0 + 0x10, clears the 16 bytes at s1 + 0x50 (s1 = *(a0 + 0x68)), converts s1's +0x24
   short to a float at s1 + 0x20, publishes the float argument at s1 + 0x60, scales the
   func_2A78D8(a0 + 0x10, D_001B0FD8, D_001B0FDC) result by 0.017453292f (0x3C8EFA36) into
   s1 + 0x64, sets the 0x1000 bit of a0 + 0x34 masking 0x0041 off, and copies the 0x10 field of
   the *(a0 + 0x24) object to a0 + 0x98 before func_3109C8(a0, 2, -1).
   The two TI128 stores both fold their constant offset into the sq here
   (sq $v0,0x10($s0), sq $zero,0x50($s1)) where retail materialises $a0 = s0 + 0x10 and
   $v1 = s1 + 0x50 and stores at offset 0 - the same wall as func_L11_003973F8 and
   func_L04_0033D0C0.  Note retail reaches 0x001B0FD8 through $gp and 0x001B0FDC absolutely,
   so this body belongs in the -g8 sibling. */

#include "include_asm.h"
#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern f32 D_L07_001B0FD8 __attribute__((sda));

extern f32 D_L07_001B0FDC __attribute__((sda));

extern f32 func_002A78D8(char *a, f32 b, f32 c);

extern void func_3109C8(u8 *a, s32 b, s32 c);

void func_L07_003F6C40(char *a0, s32 a1, TI128 *a2, f32 f12)
{
    char *s1 = *(char **)(a0 + 0x68);

    *(s32 *)(a0 + 0xB8) = a1;
    *(TI128 *)(a0 + 0x10) = *a2;
    *(TI128 *)(s1 + 0x50) = 0;
    *(f32 *)(s1 + 0x20) = (f32)*(s16 *)(s1 + 0x24);
    *(f32 *)(s1 + 0x60) = f12;
    *(f32 *)(s1 + 0x64) =
        func_002A78D8(a0 + 0x10, D_L07_001B0FD8, D_L07_001B0FDC) * 0.017453292f;
    *(u16 *)(a0 + 0x34) = (*(u16 *)(a0 + 0x34) & 0xFFBE) | 0x1000;
    *(s32 *)(a0 + 0x98) = *(s32 *)(*(char **)(a0 + 0x24) + 0x10);
    func_3109C8((u8 *)a0, 2, -1);
}

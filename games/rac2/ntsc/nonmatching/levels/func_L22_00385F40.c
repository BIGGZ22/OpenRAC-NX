/* func_L22_00385F40, 220 bytes, 1 placement.  Nearest: exact 220 bytes, ONE differing word
   at +0xac.
   Switch on the object's 0x20 byte: case 0 sets +0x30 = 0xFF and calls func_31FBB8(a0,1,-1);
   case 1 walks the D_L22_001B2128 list for a node whose +0xAA short is 0xD3D with a +0x20 byte
   other than 3, and, when none is found and D_L22_001A7A78 is clear, sets it to 1, calls
   func_362E98(1,0,0) and func_32AE80(0x1233,-1) and stores 1 to D_L22_001A903C.
   The single gap: retail emits the second argument's `daddu $a1,$zero,$zero` one slot before
   the lui/sb of the D_L22_001A7A78 store, this cc1 emits it after it.  Three spellings (a
   named zero for both call arguments, one named byte feeding the store and the argument, and
   the guard as `found == 0 && ...`) give the same word. */

#include "include_asm.h"

typedef unsigned char u8;
typedef signed short s16;
typedef int s32;

extern s32 D_L22_001B2128 __attribute__((sda));

extern u8 D_L22_001A7A78 __attribute__((sda));

extern s32 D_L22_001A903C __attribute__((sda));

extern void func_31FBB8();

extern void func_362E98(s32, s32, s32);

extern void func_32AE80(s32, s32);

void func_L22_00385F40(s32 a0)
{
    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(u8 *)(a0 + 0x30) = 0xFF;
        func_31FBB8(a0, 1, -1);
        break;
    case 1: {
        s32 found = 0;
        s32 node = D_L22_001B2128;

        while (node != 0) {
            if (*(s16 *)(node + 0xAA) == 0xD3D) {
                if (*(u8 *)(node + 0x20) != 3)
                    found = 1;
            }
            node = *(s32 *)(node + 0x28);
        }
        if (found != 0)
            break;
        if (D_L22_001A7A78 == 0) {
            D_L22_001A7A78 = 1;
            func_362E98(1, 0, 0);
            func_32AE80(0x1233, -1);
            D_L22_001A903C = 1;
        }
        break;
    }
    }
}

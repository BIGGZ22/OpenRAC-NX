/* Alternative source form for func_L08_00425568.
   The same `if (byte31 == 0) { store 0; increment }` reading with the READ marked
   `volatile` - the only spelling measured that stops cc1 forwarding the dead store, so the
   unit reaches retail's 192 bytes. The cost is the annulled branch: the block becomes
   `bnez $v0,...` with a `lw` in the delay slot where retail has `bnel $v0,$zero,...` with
   the store itself in the delay slot, and every word after it shifts.
   Kept beside the primary near-miss (levels/func_L08_00425568.c).
   Naming/extraction only: this form has not been recompiled or validated. */
#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
typedef float f32;

extern void func_002F9A90(char *a);

void func_L08_00425568(char *a0)
{
    char *s0 = a0;
    char *s1;

    if (*(u16 *)(s0 + 0x96) < 0x7FFD)
        *(u16 *)(s0 + 0x96) = *(u16 *)(s0 + 0x96) + 1;
    s1 = *(char **)(s0 + 0x68);
    if (*(u8 *)(s0 + 0x31) == 0) {
        *(u16 *)(s1 + 0x1BC) = 0;
        *(u16 *)(s1 + 0x1BC) = *(volatile u16 *)(s1 + 0x1BC) + 1;
    }
    func_002F9A90(s1 + 0x1B0);
    if (*(u8 *)(s0 + 0x20) >= 3) {
        if (*(s16 *)(s1 + 0x1BE) < 0x78)
            *(u16 *)(s1 + 0x1BE) = *(u16 *)(s1 + 0x1BE) + 1;
    }
    if (*(u8 *)(s0 + 0x20) >= 2) {
        *(f32 *)(s0 + 0x74) = 2.0f;
        *(u8 *)(s0 + 0x6F) = *(u8 *)(s0 + 0x32);
        *(f32 *)(s0 + 0x70) = *(f32 *)(s1 + 0x1C4) - *(f32 *)(s0 + 0x18);
        *(u8 *)(s0 + 0xBD) = 0;
    }
}

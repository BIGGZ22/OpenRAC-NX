/* Alternative source form for func_L08_00425568.
   Both statements under `if (byte31 == 0)` - the reading retail's own `bnel` delay slot
   implies. This cc1 forwards the dead `= 0` store into the following load and folds the
   pair, so the unit comes out 184 bytes against retail's 192, short by exactly the
   `sh $zero,0x1BC($s1)` and its `lhu`.
   Kept beside the primary near-miss (levels/func_L08_00425568.c), which records the
   bnel/no-fold conflict and the remaining allocation differences.
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
        *(u16 *)(s1 + 0x1BC) = *(u16 *)(s1 + 0x1BC) + 1;
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

/* 0 of 5 places match - last line: 04_barlow: 124 bytes, expected 132.
   Complete: both signed power-of-two divisions with their `slt`/`movn` pairs,
   the masked tag loop (`& 0xC0FFFFFF`, `| (value & 0x3F) << 24`) and the guard
   all reproduce.  Differences: the tag stays in $a1 where retail moves it to
   $t1, the state pointer goes to $a2 against retail's $v1, and the retail loop
   carries an extra `daddu` copy plus a padding `nop` (eight words) this cc1 does
   not emit for the seven-word loop. */

#include "include_asm.h"

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

/* tags the masked words of the sub-array the state block points at */
void func_L04_003A14B8(u8 *object, u32 value) {
    u32 tag = (value & 0x3F) << 24;
    u8 *state = *(u8 **)(object + 0x98);
    u32 *p = (u32 *)(state + *(s32 *)(state + 0x4) + 0x10 +
                     (*(s32 *)(state + 0xC) / 8) * 8);
    s32 count = *(s32 *)(state + 0x8) / 4;

    if (count > 0) {
        do {
            *p = (*p & 0xC0FFFFFF) | tag;
            p++;
            count--;
        } while (count != 0);
    }
}

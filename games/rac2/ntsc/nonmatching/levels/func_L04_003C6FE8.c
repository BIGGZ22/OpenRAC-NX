/* 0 of 3 places match - last line: 04_barlow: 228 bytes, expected 224
   Two-state timer on the record's byte 0x20 (a `switch`, like the matched sibling
   func_L09_003C8CD8 uses): state 0 zeroes the resident word, stamps the record, loads the
   frame duration 0x3C into three words and derives both the reciprocal step (1.0f / ret) and
   the 1/60 blend; state 1 counts the phase down by 0.0025 and wraps it by 8.
   With the switch the dispatch matches exactly.  The four words at 0x001B2E70..0xE84 need a
   second, `nosda` name: 0x001B2E84 is written absolutely here and through $gp by the matched
   func_L04_003C70C8.  What stays: this cc1 copies the record pointer to $a1 at entry (retail
   keeps it in $a0 and only loads $a0 with 0x3C at the call), one instruction over. */

#include "include_asm.h"

extern float func_00283CE0(int);
extern u32 D_L04_001B2E84_1 __attribute__((nosda));
extern int D_L04_001B2E70 __attribute__((nosda));
extern int D_L04_001B2E7C __attribute__((nosda));
extern float D_L04_001B2E80 __attribute__((nosda));
extern int ref_L04_003C6FE8_00 __attribute__((sda));
extern float ref_L04_003C6FE8_01 __attribute__((sda));
extern float ref_L04_003C6FE8_02 __attribute__((sda));

void func_L04_003C6FE8(u8 *a0)
{
    int v = 0x3C;

    switch (a0[0x20]) {
    case 0:
        D_L04_001B2E84_1 = 0;
        a0[0x20] = 1;
        a0[0x30] = 0xFF;
        D_L04_001B2E70 = v;
        ref_L04_003C6FE8_00 = v;
        ref_L04_003C6FE8_01 = 1.0f / func_00283CE0(v);
        D_L04_001B2E7C = v;
        D_L04_001B2E80 = 0.016666668f;
        break;
    case 1:
        {
            float t = ref_L04_003C6FE8_02 - 0.0025f;

            if (t <= -8.0f)
                t += 8.0f;
            ref_L04_003C6FE8_02 = t;
        }
        break;
    }
}

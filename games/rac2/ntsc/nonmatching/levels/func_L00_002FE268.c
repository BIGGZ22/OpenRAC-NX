/* 0 of 27 places match - last line: 00_aranos_tutorial: 112 bytes, expected 108.
   Complete: both 64-bit constants reach retail's exact dli sequences
   (`ori 0x8000 / dsll 20 / ori 0x8000 / dsll 19 / ori 0x4000` and
   `ori 0xFFA0 / dsll32 0 / ori 0xE0`), the resident latch read, the three-word
   block stores and the call all reproduce.  One instruction too many: this cc1
   computes the block address into a spare register and copies it into $a3 for
   the call, where retail materialises it straight into $a3
   (`addiu $a3,$v1,-0x6E40`) and stores through it.
   The third call argument is a one-instruction `addiu $a2,$gp` load, so the unit
   must be -G8 (measured in a dedicated file; the canonical L00_002FC7A0.c is
   -G0). */

#include "include_asm.h"

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;

extern u8 D_L00_001A9520[4] __attribute__((sda));

extern u64 D_001A72C8;
extern u32 D_001A72C0;
extern u32 D_L00_001BF1B4;
extern u64 D_L00_001D91C0[3];

extern void func_002FF0C0(u32, u32, void *, void *);

/* latches the incoming word, seeds the three-word block from the resident latch
   and hands both to the shared routine */
void func_L00_002FE268(u32 value)
{
    u64 previous = D_001A72C8;
    u64 *block = D_L00_001D91C0;

    D_001A72C0 = value;
    block[0] = previous;
    block[2] = 0x0040000400004000ULL;
    block[1] = 0xFFA0000000E0ULL;
    func_002FF0C0(D_L00_001BF1B4, 0, D_L00_001A9520, block);
}
